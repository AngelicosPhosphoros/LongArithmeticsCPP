/**
Copyright (c) 2018 AngelicosPhosphoros
https://github.com/AngelicosPhosphoros/LongArithmeticsCPP/
*/
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS 

#include "LongArith.h"
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <deque>
#include <tuple>
#include <cstring>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif


struct internal_accessor :public LongArith {
    using container_type = LongArith::container_type;
    using container_union = LongArith::container_union;
};
using compute_t = LongArith::compute_t;
using digit_t = LongArith::digit_t;
constexpr compute_t DigitBase = LongArith::DigitBase;
constexpr size_t DigitStringLength = LongArith::DigitStringLength;


using container_type = internal_accessor::container_type;

// Type asserts
static_assert(std::is_nothrow_move_assignable<LongArith>::value && std::is_nothrow_move_constructible<LongArith>::value, "Nothrow guarantee check for LongArith failed");
static_assert(std::numeric_limits<compute_t>::max() >= DigitBase*DigitBase 
    && std::numeric_limits<compute_t>::min() <= -DigitBase*DigitBase
    && std::numeric_limits<compute_t>::max() / DigitBase >= DigitBase
    && std::numeric_limits<compute_t>::min() / DigitBase <= -DigitBase
    ,
    "Checks for sizes of compute_t failed");
static_assert(std::numeric_limits<digit_t>::max() >= DigitBase, "digit_t have not enough range");

// Runtime checks
#if 0

compute_t cast_digit_to_compute(const digit_t val)
{
    return static_cast<compute_t>(val);
}

digit_t cast_compute_to_digit(const compute_t val)
{
    if (val < 0 || val >= static_cast<compute_t>(DigitBase))
        throw std::logic_error("Cast error on digit_t -> compute_t");
    return static_cast<digit_t>(val);
}


#define TO_COMPUTE_T(val) cast_digit_to_compute(val)
#define TO_DIGIT_T(val) cast_compute_to_digit(val)

#define LA_ASSERT(x) assert(x)

#else
#define TO_COMPUTE_T(val) static_cast<compute_t>(val)
#define TO_DIGIT_T(val) static_cast<digit_t>(val)

#define LA_ASSERT(x) 
#endif


#pragma region Internal Static Code
//****************** SIMPLE INTERNAL UTILS **********************

// how many digits we need to storage it
static size_t get_digit_count(digit_t val)
{
    LA_ASSERT(val < DigitBase);
    if (val < 10)
        return 1;
    if (val < 100)
        return 2;
    if (val < 1000)
        return 3;
    if (val < 10000)
        return 4;
    if (val < 100000UL)
        return 5;
    if (val < 1000000UL)
        return 6;
    if (val < 10000000UL)
        return 7;
    if (val < 100000000UL)
        return 8;
    if (val < 1000000000UL)
        return 9;
    size_t res = 9;
    val /= 1000000000UL;
    while (val)
    {
        val /= 10;
        res++;
    }
    return res;
}

// Checks that string is correct integer
static bool check_string(const std::string& s)
{
    using namespace std;
    //constexpr char digits[] = "0123456789";
    if (s.empty())
        return false;
    auto iterator = s.cbegin();
    // Check symbols
    if (s[0] == '+' || s[0] == '-')
    {
        ++iterator;
        if (s.length() == 1)
            return false;
    }
    for (; iterator != s.cend(); ++iterator)
    {
        //if (!binary_search(begin(digits), end(digits), *iterator))
        if (*iterator<'0' || *iterator>'9')
        {
            return false;
        }
    }
    return true;
}

// Remove all zeros at end except vect[0]
static void clean_leading_zeros(container_type& vect)
{
    if (vect.size() > 1)
    {
        size_t index = vect.size() - 1;
        while (index > 0 && !vect[index])
            --index;
        vect.resize(index + 1);
    }
}

//****************** ARRAY OPERATING UTILS **********************

// Compares absolute values of encoded numbers in vectors
// -1 if left>right, 1 if left<right, 0 otherwise
static inline signed short compare_absolute_vectors(const container_type& left, const container_type& right)
{
    if (left.size() > right.size())
        return -1;
    if (left.size() < right.size())
        return 1;
    size_t index = left.size() - 1;
    while (index > 0 && left[index] == right[index])
        index--;
    if (left[index] == right[index])
        return 0;
    return (left[index] > right[index]) ? -1 : 1;
}


// Sum and diff

// Implementation of add_array
// Assume original and addition is different
static void unchecked_internal_add_array(container_type& original, const container_type& addition, const size_t shift)
{
    LA_ASSERT(&original != &addition);
    // when we work with addition, we keep in mind "virtual" digits
    const size_t addition_size = addition.size();
    if (addition_size + shift > (original.capacity() << 1))
    {
        original.reserve(addition_size + shift + 1);
    }
        
    if (shift >= original.size())
    {
        const size_t old_size = original.size();
        original.resize(shift + 1);
        std::fill(original.begin() + old_size, original.end(), 0);
    }

    const size_t original_size = original.size();
    
    compute_t sum = 0;
    const size_t less = std::min(addition_size + shift, original_size);
    size_t index = shift;
    // common part
    while (index < less)
    {
        sum += original[index];
        sum += addition[index - shift];
        original[index] = sum % DigitBase;
        sum /= DigitBase;
        index++;
    }
    // This will work if original shorter than addition
    while (index < addition_size + shift)
    {
        sum += addition[index - shift];
        original.push_back(sum % DigitBase);
        sum /= DigitBase;
        index++;
    }
    // This is addition of sum to storage
    while (sum && index < original_size)
    {
        sum += original[index];
        original[index] = sum % DigitBase;
        sum /= DigitBase;
        index++;
    }
    // Now we push sum to empty positions
    while (sum)
    {
        original.push_back(sum % DigitBase);
        sum /= DigitBase;
    }
}

// increment original by addition and change original
// sum started from original[shift] and addition[0] (addition is shifted to left)
// \param original is changing vector
// \param addition is change
// \param shift is first index of original to change
inline static void add_array(container_type &original, const container_type &addition, const size_t shift)
{
    // to prevent errors
    if (&original == &addition)
    {
        container_type add_copy(addition);
        unchecked_internal_add_array(original, add_copy, shift);
    }
    else
    {
        unchecked_internal_add_array(original, addition, shift);
    }
}


// decrement bigger by less and store data in bigger
// \param bigger is changing digits
// \param less is value which decreased left
static void substract_array(container_type &bigger, const container_type &less)
{
    LA_ASSERT(bigger.size() >= less.size());
    const size_t bigger_size = bigger.size(), less_size = less.size();
    compute_t to_del = 0;
    for (size_t i = 0; i < less_size; i++)
    {
        to_del += less[i];
        if (to_del > bigger[i])
        {
            bigger[i] = TO_DIGIT_T(DigitBase - (to_del - bigger[i]));
            to_del = 1;
        }
        else
        {
            bigger[i] = TO_DIGIT_T(bigger[i] - to_del);
            to_del = 0;
        }
    }
    for (size_t i = less_size; i < bigger_size && to_del; i++)
    {
        if (to_del > bigger[i])
        {
            bigger[i] = TO_DIGIT_T(DigitBase - (to_del - bigger[i]));
            to_del = 1;
        }
        else
        {
            bigger[i] = TO_DIGIT_T(bigger[i] - to_del);
            to_del = 0;
        }
    }

    clean_leading_zeros(bigger);
}
// Working with absolute value

// Increase num by 1
static void inline inc1_array(container_type &num)
{
    bool cont = true;
    size_t index = 0;
    for(size_t index = 0, num_size = num.size(); cont && index<num_size; ++index)
    {
        cont = ++num[index] == DigitBase;
        if (cont)
        {
            num[index] = 0;
        }
    }
    if (cont)
    {
        num.push_back(1);
    }
}

// Decrease num by 1
static void inline dec1_array(container_type &num)
{
    // We cannot decrement zero
    LA_ASSERT(!(num.size() == 1 && num.back() == 0));
    bool cont = true;
    size_t index = 0;
    for (size_t index = 0, num_size = num.size(); cont && index<num_size; ++index)
    {
        if (num[index] == 0)
        {
            num[index] = DigitBase - 1;
        }
        else
        {
            --num[index];
            cont = false;
        }
    }
    clean_leading_zeros(num);
    LA_ASSERT(!cont);
}


// This is always increase absolute value of array
// \param change must be positive
static void increment_array(container_type &arr, compute_t change)
{
    LA_ASSERT(change >= 0);
    switch (change)
    {
    case 0: return;
    case 1:
        inc1_array(arr);
        return;
    default:
        compute_t sum = change;
        for (size_t index = 0, arr_size = arr.size(); sum && index < arr_size; index++)
        {
            sum += arr[index];
            arr[index] = sum % DigitBase;
            sum /= DigitBase;
        }
        while (sum)
        {
            arr.push_back(sum % DigitBase);
            sum /= DigitBase;
        }
    }
}
// This will decrease absolute value of array.
// \param change must be lower than DigitBase squared
// \return if arr>change return true else false (means change digit)
static bool decrement_array(container_type &arr, digit_t change)
{
    LA_ASSERT(change >= 0);
    LA_ASSERT(change < DigitBase*DigitBase);

    compute_t current_val = (arr.size() > 1) ?
        (arr[0] + arr[1] * DigitBase) : arr[0];

    if (change > current_val)
    {
        // zero lower digits
        for (size_t i = 0; i < std::min<size_t>(2, arr.size());++i)
        {
            arr[i] = 0;
        }
        change -= TO_DIGIT_T(current_val);

        if (arr.size() > 2)
        {
            change -= 1;
            dec1_array(arr);
            arr[0] -= TO_DIGIT_T(change % DigitBase);
            arr[1] -= TO_DIGIT_T(change / DigitBase);
            return true;
        }
        else // We changed sign
        {
            increment_array(arr, change);
            return false;
        }
    }
    else
    {
        const compute_t comp_result = current_val - change;
        arr[0] = TO_DIGIT_T(comp_result % DigitBase);
        if (arr.size() > 1)
            arr[1] = TO_DIGIT_T(comp_result / DigitBase);
        return false;
    }
}

//======= Mult

// Multiplication of big value on small
// Complexity O(n), memory O(1)
static void mult_small(container_type& big_number, const compute_t multiplicator)
{
    switch (multiplicator)
    {
    case 0:
        big_number.resize(1);
        big_number[0] = 0;
        return;
    case 1:
        return;
    default:
        LA_ASSERT(multiplicator < TO_COMPUTE_T(DigitBase)*DigitBase);
        compute_t trans_product = 0, mult = multiplicator;
        for (size_t i = 0, big_number_size = big_number.size(); i < big_number_size; i++)
        {
            trans_product = trans_product + mult * TO_COMPUTE_T(big_number[i]);
            big_number[i] = trans_product % DigitBase;
            trans_product = trans_product / DigitBase;
        }
        while (trans_product)
        {
            big_number.push_back(trans_product % DigitBase);
            trans_product /= DigitBase;
        }
    }
}

// Multiplication of two long numbers
// Complexity is O(m1.size()*m2.size())
static container_type mult_big(const container_type& m1, const container_type& m2)
{
    container_type result(m1.size()+m2.size());
    result.push_back(0);
    const container_type& bigger = (m1.size() > m2.size()) ? m1 : m2;
    const container_type& smaller = (m1.size() > m2.size()) ? m2 : m1;
    container_type trans_product;
    for (size_t i = 0, smaller_size = smaller.size(); i < smaller_size;i++)
    {
        trans_product = bigger;
        mult_small(trans_product, smaller[i]);
        add_array(result, trans_product, i);
    }
    return result;
}

// Division

// returns fraction in result & put remainder into dividend
// Size of dividend_and_remainder must be bigger than divider only by one digit
// Complexity: O(n^2) in mean, O(log(DigitBase)*n^2) in worst case
// \param dividend_and_remainder must begin from nonzero digit if longer than divider and have size in [divider.size(), divider.size()+1]
// \param divider is simple divider, must begin from nonzero character
digit_t divide_almost_same_len_vectors(container_type& dividend_and_remainder, const container_type& divider)
{
    LA_ASSERT(dividend_and_remainder.size() && divider.size());
    LA_ASSERT(dividend_and_remainder.size() >= divider.size() && dividend_and_remainder.size() <= divider.size() + 1);
    LA_ASSERT(divider.back());
    LA_ASSERT(dividend_and_remainder.size() == divider.size() || (dividend_and_remainder.back() && dividend_and_remainder.size() == divider.size() + 1));
    // Check simple cases
    // They have complexity of O(n)
    const auto abs_cmp = compare_absolute_vectors(dividend_and_remainder, divider);
    if (abs_cmp > 0)
        return 0;
    if (abs_cmp == 0)
    {
        std::fill(dividend_and_remainder.begin(), dividend_and_remainder.end(), digit_t(0));
        return 1;
    }

    // if (abs_cmp < 0)

    // Divide first digits to get result
    compute_t dividend_beg, divider_beg;
    if (dividend_and_remainder.size() > divider.size())
    {
        dividend_beg = TO_COMPUTE_T(dividend_and_remainder.back())*DigitBase + *(dividend_and_remainder.end() - 2);
    }
    else
    {
        dividend_beg = dividend_and_remainder.back();
    }
    divider_beg = divider.back();

    // Try to divide most significant digits
    compute_t main_div = dividend_beg / divider_beg;
    container_type multiplicated(divider);
    mult_small(multiplicated, main_div);
    auto cmp = compare_absolute_vectors(dividend_and_remainder, multiplicated);
    if (cmp <= 0) // if multiplicated less or equal to dividend
    {
        substract_array(dividend_and_remainder, multiplicated);
        return TO_DIGIT_T(main_div);
    }

    // Start to search good divider, because 1th digit/
    compute_t lower_div = dividend_beg / (divider_beg + 1);
    // check lower_div
    multiplicated = divider;
    mult_small(multiplicated, lower_div);
    LA_ASSERT(compare_absolute_vectors(multiplicated, dividend_and_remainder) > 0);
    substract_array(dividend_and_remainder, multiplicated);
    if (compare_absolute_vectors(dividend_and_remainder, divider) > 0)
    {
        return TO_DIGIT_T(lower_div);
    }
    else
    {
        add_array(dividend_and_remainder, multiplicated, 0);
    }

    // Binary search of multiplier
    // O(log2 of DigitBase multiplications)
    compute_t upper = main_div; // upper is too big multiplier
    compute_t lower = lower_div; // lower is too small multiplier
    while (1)
    {
        if (upper - lower <= 5)
        {
            compute_t mult;
            for (mult = lower; mult <= upper; ++mult)
            {
                multiplicated = divider;
                mult_small(multiplicated, mult);
                const auto cmp = compare_absolute_vectors(dividend_and_remainder, multiplicated);
                if (cmp == 0)
                {
                    std::fill(dividend_and_remainder.begin(), dividend_and_remainder.end(), digit_t(0));
                    return TO_DIGIT_T(mult);
                }
                if (cmp > 0)
                {
                    substract_array(multiplicated, divider);
                    substract_array(dividend_and_remainder, multiplicated);
                    mult--;
                    return TO_DIGIT_T(mult);
                }
            }
            throw std::runtime_error("Something wrong in division part");
        }
        else
        {
            compute_t middle = lower + ((upper - lower) >> 1);
            multiplicated = divider;
            mult_small(multiplicated, middle);
            const auto cmp = compare_absolute_vectors(dividend_and_remainder, multiplicated);
            if (cmp == 0) // if exact match 
            {
                std::fill(dividend_and_remainder.begin(), dividend_and_remainder.end(), digit_t(0));
                return TO_DIGIT_T(middle);
            }
            bool middle_less = cmp < 0;
            if (middle_less)
            {
                lower = middle;
            }
            else
            {
                upper = middle;
            }
        }
    }

    throw std::runtime_error("Something wrong in division part");
}

// Divide dividable by divider
// Complexity O(divider.size()^2)*dividable.size()/divider.size() = O(m*n)
// \param dividable must be bigger than divider
// \return pair of fraction and remainder
std::pair<container_type, container_type> divide_vectors(const container_type& dividable, const container_type& divider)
{
    LA_ASSERT(dividable.size() >= divider.size());
    std::deque<digit_t> fraction; // because we insert from begin, use deque

    container_type current_part(dividable.end() - divider.size(), dividable.end());
    size_t not_checked_len = dividable.size() - divider.size();
    bool last = !not_checked_len;
    while (not_checked_len || last)
    {
        digit_t r = divide_almost_same_len_vectors(current_part, divider);
        fraction.push_front(r);

        if (current_part.size() != divider.size())
        {
            const size_t old = current_part.size();
            current_part.resize(divider.size());
            std::fill(&current_part[old], &current_part[old] + (divider.size() - old), 0);
        }

        size_t lead_zero_count = 0;
        for (size_t i = current_part.size(); i && !current_part[i - 1];--i)
        {
            lead_zero_count++;
        }

        if (lead_zero_count) // We can skip zeros
        {
            if (not_checked_len)
            {
                const size_t next_shift = (std::min)(lead_zero_count, not_checked_len);
                const size_t remain_count = divider.size() - next_shift;

                digit_t* begin_ptr = &current_part[0];
                // Move remainder
                memmove(begin_ptr + next_shift, begin_ptr, remain_count * sizeof(digit_t));
                // Copy new data in tail
                memcpy(begin_ptr, &dividable[0] + not_checked_len - next_shift, next_shift * sizeof(digit_t));
                not_checked_len -= next_shift;

                for (size_t i = 1; i < next_shift; ++i) // We add zeros in places of skiped positions
                {
                    fraction.push_front(0);
                }
            }
        }
        else // if all digits continue to be occupied
        {
            if (not_checked_len)
            {
                const size_t new_size = divider.size() + 1;
                current_part.resize(new_size);

                digit_t* begin_ptr = &current_part[0];
                // Move remainder
                memmove(begin_ptr + 1, begin_ptr, (new_size - 1) * sizeof(digit_t));

                // Copy new data in tail
                current_part[0] = dividable[not_checked_len - 1];
                not_checked_len--;
            }
            else
            {
                LA_ASSERT(last); // This can happen only if last
            }
        }

        if (!not_checked_len) // If it here, we end
            last = !last; // !last for handle not_checked_len==0 in first time, false in all other
    }
    container_type fr(fraction.begin(), fraction.end());
    clean_leading_zeros(fr);
    clean_leading_zeros(current_part);
    return std::make_pair(std::move(fr), std::move(current_part));
}



#pragma endregion


#pragma region Class Methods


//***************** CLASS INTERFACE IMPLEMENTATIONS ****************************

// -1 if left>right, 1 if left<right, 0 otherwise
signed short LongArith::compare_absolute_values(const LongArith& left, const LongArith& rigth)
{
    return compare_absolute_vectors(left.storage, rigth.storage);
}


// Plus

LongArith& LongArith::operator+=(const LongArith& change)&
{
    if (get_negative() == change.get_negative())
    {
        add_array(storage, change.storage, 0);
    }
    else
    {
        bool res_negative;
        if (compare_absolute_values(*this, change) <= 0)
        {
            res_negative = this->get_negative();
            substract_array(storage, change.storage);
        }
        else
        {
            res_negative = change.get_negative();
            container_type tmp(storage);
            storage = container_type(change.storage);
            substract_array(storage, tmp);\
        }
        set_negative(res_negative);
    }
    check_zero();
    return *this;
}

LongArith& LongArith::operator+=(LongArith&& change)&
{
    if (change.get_negative() == get_negative())
    {
        add_array(storage, change.storage, 0);
    }
    else
    {
        bool res_negative;
        if (compare_absolute_values(*this, change) <= 0)
        {
            res_negative = this->get_negative();
            substract_array(storage, change.storage);
        }
        else
        {
            res_negative = change.get_negative();
            std::swap(change.storage, storage);
            substract_array(storage, change.storage);
        }
        set_negative(res_negative);
    }
    check_zero();
    return *this;
}

LongArith& LongArith::operator+=(long change)&
{
    // Handle extreme values safely
    if (std::numeric_limits<compute_t>::max() < std::numeric_limits<long>::max() || change > DigitBase*DigitBase || change < -DigitBase*DigitBase)
    {
        return *this += LongArith(change);
    }

    if (get_negative() == change < 0)
    {
        compute_t change_b(change); // to avoid integer overflow
        increment_array(storage, change_b < 0 ? -change_b : change_b);
    }
    else
    {
        const bool old_negative = get_negative();
        digit_t add = (change < 0) ? -change : change;
        const bool not_changed_sign = decrement_array(storage, add);
        set_negative(not_changed_sign == old_negative);
    }
    check_zero();
    return *this;
}


LongArith& LongArith::operator++()&
{
    // this variant runs 1.4x faster than this+=1
    check_zero();
    if (get_negative())
    {
        dec1_array(storage);
    }
    else
    {
        inc1_array(storage);
    }
    check_zero();
    return *this;
}

LongArith& LongArith::operator-=(long change)&
{
    if (std::numeric_limits<long>::min() == change)
    {
        ++*this;
        ++change;
    }
    return this->operator+=(-change);
}



LongArith& LongArith::operator--()&
{
    // this variant runs 1.4x faster than this-=1
    if (!get_negative() && equals_zero())
        set_negative(true);
    if (get_negative())
    {
        inc1_array(storage);
    }
    else
    {
        dec1_array(storage);
    }
    check_zero();
    return *this;
}

// Multiplication
LongArith operator*(const LongArith& a, const LongArith& b)
{
    LongArith res(0);
    if (!(a.equals_zero() || b.equals_zero()))
    {
        res.storage = mult_big(a.storage, b.storage);
        res.set_negative(a.get_negative() != b.get_negative());
    }
    return res;
}


LongArith& LongArith::operator*=(long multiplier)&
{
    const bool calculated_negative = get_negative() != (multiplier < 0);
    if (multiplier < 0)
    {
        if (multiplier == std::numeric_limits<long>::min())
        {
            return *this *= LongArith(multiplier);
        }
        multiplier = -multiplier;
    }
    mult_small(storage, multiplier);
    set_negative(calculated_negative);
    return *this;
}


std::pair<LongArith, LongArith> LongArith::fraction_and_remainder(const LongArith& dividable, const LongArith& divider)
{
    typedef std::pair<LongArith, LongArith> t_result;
    // Argument check
    if (divider.equals_zero())
    {
        throw std::logic_error("Division by zero");
    }

    // Simple Cases
    if (dividable.equals_zero())
    {
        return t_result(0, 0);
    }
    const int abs_compare = LongArith::compare_absolute_values(dividable, divider);
    if (abs_compare == 0)
    {
        return t_result(LongArith((dividable.get_negative() == divider.get_negative()) ? 1 : -1),
            LongArith(0));
    }
    if (abs_compare > 0)
    {
        return t_result(0, dividable);
    }

    // Here divider is always lower than dividable

    LongArith fraction, remainder;
    std::tie(fraction.storage, remainder.storage) = divide_vectors(dividable.storage, divider.storage);
    fraction.set_negative(dividable.get_negative() != divider.get_negative());
    remainder.set_negative(dividable.get_negative());

    return t_result(std::move(fraction), std::move(remainder));
}

std::pair<LongArith, long> LongArith::fraction_and_remainder(const LongArith & dividable, const long divider)
{
    // Argument check
    if (!divider)
    {
        throw std::logic_error("Division by zero");
    }

    typedef std::pair<LongArith, long> t_result;

    if (divider == 1)
    {
        return t_result(dividable, 0);
    }

    if (divider == -1)
    {
        return t_result(-dividable, 0);
    }

    if (std::numeric_limits<long>::min() == divider)
    {
        auto res = fraction_and_remainder(dividable, LongArith(divider));
        return t_result(std::move(res.first), static_cast<long>(res.second.to_plain_int()));
    }

    const unsigned long u_div = (divider >= 0) ? divider : -divider;

    LongArith fraction;
    unsigned long remainder = 0;
    for (size_t i1 = dividable.storage.size(); i1 > 0; --i1)
    {
        const size_t i = i1 - 1;
        const compute_t value = dividable.storage[i] + remainder*DigitBase;
        fraction.storage.push_back(TO_DIGIT_T(value / u_div));
        remainder = value % u_div;
    }

    std::reverse(fraction.storage.begin(), fraction.storage.end());
    clean_leading_zeros(fraction.storage);

    fraction.set_negative(divider < 0 != dividable.get_negative());
    const signed long signed_remainder = dividable.get_negative() ? -static_cast<signed long>(remainder) : remainder;

    return t_result(std::move(fraction), signed_remainder);
}

// Utility for constant calculation
namespace hidden
{
    inline static size_t len_10_in_DIGIT_BASE()
    {
        size_t v = DigitBase;
        size_t r = 0;
        while (v > 1)
        {
            r++;
            v /= 10;
        }
        return r;
    }
}
const static size_t DecimalDigitLen = hidden::len_10_in_DIGIT_BASE();

LongArith LongArith::fast_divide_by_10(const size_t power) const
{
    if (!power)
        return *this;

    const size_t digits_skipped = power / DecimalDigitLen;
    const size_t remain = power % DecimalDigitLen;
    digit_t remain_div = 1;
    for (size_t i = 0; i < remain; ++i)
        remain_div *= 10;

    if (digits_skipped >= storage.size() || (digits_skipped == storage.size() - 1 && remain_div > storage.back()))
        return LongArith(0);

    LongArith result;
    result.storage.resize(storage.size() - digits_skipped);
    memcpy(&result.storage[0], &storage[digits_skipped], sizeof(digit_t)*(storage.size() - digits_skipped));

    if (remain_div == 1)
    {
        result.set_negative(get_negative());
        result.check_zero();
        return result;
    }

    for (size_t i = 0; i + 1 < result.storage.size(); ++i)
    {
        compute_t next_digit_summed = TO_COMPUTE_T(result.storage[i + 1]) * DigitBase + result.storage[i];
        result.storage[i] = TO_DIGIT_T((next_digit_summed / remain_div) % DigitBase);
    }
    if (result.storage.back() >= remain_div)
    {
        result.storage[result.storage.size() - 1] = result.storage.back() / remain_div;
    }
    else
    {
        result.storage.pop_back();
    }

    result.set_negative(get_negative());
    result.check_zero();
    return result;
}

LongArith LongArith::fast_remainder_by_10(const size_t power) const
{
    if (!power)
        return LongArith(0);

    const size_t digits_skipped = power / DecimalDigitLen;
    const size_t remain = power % DecimalDigitLen;
    digit_t remain_div = 1;
    for (size_t i = 0; i < remain; ++i)
        remain_div *= 10;

    const size_t copy_size = (digits_skipped >= storage.size()) ? storage.size() : (digits_skipped + ((remain > 0) ? 1 : 0));
    LongArith result;
    result.storage.resize(copy_size);
    memcpy(&result.storage[0], &storage[0], copy_size * sizeof(digit_t));

    if (remain > 0 && storage.size() > digits_skipped)
    {
        const size_t i = result.storage.size() - 1;
        result.storage[i] = result.storage[i] % remain_div;
    }

    clean_leading_zeros(result.storage);

    result.set_negative(get_negative());
    result.check_zero();
    return result;
}

compute_t LongArith::to_plain_int() const
{
    if (!plain_convertable())
        throw std::logic_error("Cannot convert to plain!");
    compute_t res = storage[0];
    if (storage.size() == 2)
        res += storage[1] * DigitBase;
    return get_negative() ? -res : res;
}


// Comparison

bool operator<(const LongArith& left, const LongArith &right)
{
    if (&left == &right)
        return false;
    if (left.get_negative() && !right.get_negative())
        return true;
    if (right.get_negative() && !left.get_negative())
        return false;
    if (left.get_negative())
        return LongArith::compare_absolute_values(left, right) < 0;
    else
        return LongArith::compare_absolute_values(left, right) > 0;
}


bool operator>(const LongArith& left, const LongArith &right)
{
    if (&left == &right)
        return false;
    if (left.get_negative() && !right.get_negative())
        return false;
    if (right.get_negative() && !left.get_negative())
        return true;
    if (left.get_negative())
        return LongArith::compare_absolute_values(left, right) > 0;
    else
        return LongArith::compare_absolute_values(left, right) < 0;
}


bool operator==(const LongArith& left, const LongArith &right)
{
    return (&left == &right) || (left.get_negative() == right.get_negative() && LongArith::compare_absolute_values(left, right) == 0);
}

bool operator<=(const LongArith& left, const LongArith &right)
{
    return !(left>right);
}


bool operator>=(const LongArith& left, const LongArith &right)
{
    return !(left<right);
}

bool operator!=(const LongArith& left, const LongArith &right)
{
    return !(left == right);
}

bool LongArith::equals_zero() const noexcept
{
    return storage.size() == 1 && storage[0] == 0;
}

int LongArith::sign() const noexcept
{
    if (get_negative())
        return -1;
    if (equals_zero())
        return 0;
    return 1;
}


std::ostream& operator<<(std::ostream& os, const LongArith& obj)
{
    return os << obj.to_string();
}

std::istream& operator >> (std::istream& is, LongArith& obj)
{
    std::string s;
    std::istream& r = is >> s;

    try
    {
        obj = LongArith::from_string(s);
    }
    catch (std::invalid_argument&)
    {
        obj = 0;
    }

    return r;
}

LongArith::LongArith(long default_value) :storage()
{
    const bool negativity = default_value < 0;
    // Avoiding integer overflow
    if (default_value == std::numeric_limits<long>::min())
    {
        default_value += 2;
        storage.push_back(2);
    }
    else
    {
        storage.push_back(0);
    }

    if (default_value == 0)
    {
        set_negative(false);
        return;
    }

    if (default_value < 0)
    {
        default_value = -default_value;
    }

    if (std::numeric_limits<long>::max() > std::numeric_limits<compute_t>::max() && default_value > static_cast<long>(std::numeric_limits<compute_t>::max()))
    {
        container_type temp;
        while (default_value)
        {
            temp.push_back(default_value % DigitBase);
            default_value /= DigitBase;
        }
        add_array(storage, temp, 0);
    }
    else if (static_cast<compute_t>(default_value) > std::numeric_limits<compute_t>::max() - 3)
    {
        increment_array(storage, default_value / 2);
        increment_array(storage, default_value - (default_value / 2));
    }
    else
    {
        increment_array(storage, default_value);
    }

    set_negative(negativity);
}

LongArith::LongArith() :storage()
{
    storage.push_back(0);
}

std::string LongArith::to_string() const
{
    std::string res;
    res.reserve(storage.size()*LongArith::DigitStringLength + int(get_negative()));

    if (get_negative() && !equals_zero())
        res += '-';
    res += std::to_string(storage.back());
    for (size_t index = storage.size() - 1; index; --index)
    {
        const size_t i = index - 1;
        size_t digits = get_digit_count(storage[i]);
        for (int j = 0; j < int(DigitStringLength) - digits; ++j)
        {
            res += '0';
        }
        res += std::to_string(storage[i]);
    }

    return res;
}


LongArith LongArith::from_string(const std::string& arg)
{
    using namespace std;
    // Trim beginning
    size_t i = 0;
    while (i < arg.length() && (arg[i] == ' ' || arg[i] == '\t'))
        i++;
    if (i == arg.size() || !check_string(arg))
        throw invalid_argument("Invalid string");

    bool negative = false;
    if (arg[i] == '-' || arg[i] == '+')
    {
        negative = arg[i] == '-';
        ++i;
    }

    std::string copied;
    if (i)
    {
        copied = std::string(arg.begin() + i, arg.end()); // Copy only if need to erase beginning
    }

    const std::string& s = (i ? copied : arg);

    // Working with digits
    LongArith result;
    result.set_negative(negative);
    container_type& digits = result.storage;
    digits.clear();
    digits.reserve(s.length() / DigitStringLength + 1);
    for (i = 0; i < s.length() / DigitStringLength + (s.length() % DigitStringLength > 0 ? 1 : 0); ++i)
    {
        ssize_t left = s.length() - (i + 1) * DigitStringLength;
        left = left >= 0 ? left : 0;
        ssize_t right = s.length() - i * DigitStringLength;
        digit_t tmp(0);
        for (; left < right; left++)
        {
            tmp = tmp * 10 + static_cast<int>(s[left] - '0');
        }
        digits.push_back(tmp);
    }
    return result;
}


#pragma endregion



#pragma region Definition of internal container

LongArith::container_union::container_union() noexcept : is_local(true), is_negative(false), local_size(0)
{
    data_pointer = local_data;
}

LongArith::container_union::container_union(const container_union & other)
{
    if (other.is_local)
    {
        memcpy(this, &other, sizeof(*this));
        data_pointer = local_data;
    }
    else
    {
        if (other.heap_data.size > local_capacity) // If cannot store locally
        {
            is_local = false;
            // Allocate only needed memory in heap
            heap_data.capacity = heap_data.size = other.heap_data.size;
            data_pointer = new digit_t[heap_data.size];
        }
        else // Copy from heap to local
        {
            is_local = true;
            local_size = static_cast<uint8_t>(other.heap_data.size);
            data_pointer = local_data;
        }
        is_negative = other.is_negative;
        memcpy(data_pointer, other.data_pointer, sizeof(digit_t)*other.heap_data.size);
    }
}

LongArith::container_union::container_union(container_union && tmp) noexcept
{
    memcpy(this, &tmp, sizeof(*this));
    if (is_local)
    {
        data_pointer = local_data;
    }
    else if (heap_data.size > local_capacity)
    {
        // remove pointer from tmp
        tmp.data_pointer = tmp.local_data;
        tmp.is_local = true;
        tmp.local_size = 0;
    }
    else // Copy data to local and let tmp to deallocate
    {
        is_local = true;
        data_pointer = local_data;
        local_size = static_cast<uint8_t>(tmp.heap_data.size);
        memcpy(local_data, tmp.data_pointer, tmp.heap_data.size * sizeof(digit_t));
    }
}

template<>
LongArith::container_union::container_union(const digit_t* beg, const digit_t* end)
{
    const size_t requested_size = end - beg;
    if (!requested_size)
    {
        new(this)container_union();
    }
    else if (requested_size <= local_capacity)
    {
        new(this)container_union();
        memcpy(data_pointer, beg, requested_size * sizeof(digit_t));
        local_size = static_cast<uint8_t>(requested_size);
    }
    else
    {
        is_local = false;
        is_negative = false;
        data_pointer = new digit_t[requested_size];
        heap_data.capacity = heap_data.size = requested_size;
        memcpy(data_pointer, beg, requested_size * sizeof(digit_t));
    }
}

LongArith::container_union::container_union(const size_t initial_capacity):container_union()
{
    if (initial_capacity > local_capacity)
    {
        digit_t* allocated = new digit_t[initial_capacity];
        is_local = false;
        data_pointer = allocated;
        heap_data.size = 0;
        heap_data.capacity = initial_capacity;
    }
}

LongArith::container_union::~container_union()
{
    if (!is_local)
    {
        delete[] data_pointer;
    }
}

LongArith::container_union & LongArith::container_union::operator=(const container_union & other)
{
    if (this != &other)
    {
        const size_t sz = other.size();
        resize(sz);
        memcpy(data_pointer, other.data_pointer, sizeof(digit_t)*sz);
        is_negative = other.is_negative;
    }
    return *this;
}

LongArith::container_union & LongArith::container_union::operator=(container_union && tmp) noexcept
{
    if (this != &tmp)
    {
        this->~container_union();
        new (this)container_union(std::move(tmp));
    }
    return *this;
}

void LongArith::container_union::swap(container_union & other)& noexcept
{
    if (this == &other)
        return;
    const bool l_stack = is_local, r_stack = other.is_local;
    if (l_stack && r_stack)
    {
        // Construct copies
        container_union tmp(*this);
        new(this)container_union(other);
        new(&other)container_union(tmp);
    }
    else if (!l_stack && !r_stack)
    {
        // Simple swap local bytes and switch pointers to dynamic memory
        constexpr size_t sz = sizeof(*this);
        char tmp[sz];
        memcpy(tmp, this, sz);
        memcpy(this, &other, sz);
        memcpy(&other, tmp, sz);
    }
    else // Copy local object to stack than move bytes of heap object to new place
        if (l_stack && !r_stack)
        {
            container_union loc_copy(*this);
            memcpy(this, &other, sizeof(*this)); // Just copy data
            new(&other)container_union(loc_copy); // We don't need to clean up actually
        }
        else // if (!l_stack && r_stack)
        {
            container_union loc_copy(other);
            memcpy(&other, this, sizeof(*this));
            new(this)container_union(loc_copy);
        }
}

// Public interface

digit_t & LongArith::container_union::operator[](const size_t index)
{
    return data_pointer[index];
}

const digit_t & LongArith::container_union::operator[](const size_t index) const
{
    return data_pointer[index];
}

size_t LongArith::container_union::size() const noexcept
{
    return is_local ? local_size : heap_data.size;
}

size_t LongArith::container_union::capacity() const noexcept
{
    return is_local ? local_capacity : heap_data.capacity;
}

void LongArith::container_union::resize(const size_t new_size)
{
    reserve(new_size);
    if (is_local)
    {
        local_size = static_cast<uint8_t>(new_size);
    }
    else
    {
        heap_data.size = new_size;
    }
}

void LongArith::container_union::reserve(const size_t new_capacity)
{
    if (new_capacity > local_capacity) // If data in heap it always have more capacity than local
    {
        if (is_local)
        {
            switch_to_heap(new_capacity);
        }
        else if (heap_data.capacity < new_capacity)
        {
            reallocate(new_capacity);
        }
    }
}

void LongArith::container_union::switch_to_heap(const size_t reserve_amount)
{
    LA_ASSERT(is_local && reserve_amount > local_capacity);
    data_pointer = new digit_t[reserve_amount];
    is_local = false;
    memcpy(data_pointer, local_data, local_capacity * sizeof(digit_t));
    heap_data.size = local_size;
    heap_data.capacity = reserve_amount;
}

void LongArith::container_union::reallocate(const size_t new_capacity)
{
    LA_ASSERT(!is_local && new_capacity > heap_data.capacity);
    digit_t* allocated = new digit_t[new_capacity];
    memcpy(allocated, data_pointer, heap_data.size * sizeof(digit_t));
    digit_t* old = data_pointer;
    data_pointer = allocated;
    heap_data.capacity = new_capacity;
    delete[] old;
}

// Does not deallocate memory
void LongArith::container_union::clear()
{
    local_size = 0;
    heap_data.size = 0; // This change local array values but as they not used we don't need them
}

void LongArith::container_union::push_back(const digit_t val)
{
    const size_t old_size = size();
    if (old_size == capacity())
    {
        reserve(old_size << 1);
    }
    if (is_local)
    {
        data_pointer[local_size++] = val;
    }
    else
    {
        data_pointer[heap_data.size++] = val;
    }
}


// UB if size()==0. Return last value
digit_t LongArith::container_union::back() const
{
    LA_ASSERT(size() > 0);
    return data_pointer[size() - 1];
}

// UB if size()==0. Remove last value
void LongArith::container_union::pop_back()
{
    LA_ASSERT(size() > 0);
    if (is_local)
    {
        --local_size;
    }
    else
    {
        --heap_data.size;
    }
}

digit_t* LongArith::container_union::begin() noexcept
{
    return data_pointer;
}

digit_t* LongArith::container_union::end() noexcept
{
    return begin() + size();
}

const digit_t* LongArith::container_union::begin() const noexcept
{
    return data_pointer;
}

const digit_t* LongArith::container_union::end() const noexcept
{
    return begin() + size();
}

#pragma endregion