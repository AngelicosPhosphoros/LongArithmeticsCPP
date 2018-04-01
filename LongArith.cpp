#include "LongArith.h"
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <deque>
#include <cstring>

using compute_t = LongArith::compute_t;
using digit_t = LongArith::digit_t;
constexpr digit_t DIGIT_BASE = LongArith::DIGIT_BASE;
constexpr size_t DIGIT_STRING_LENGTH = LongArith::DIGIT_STRING_LENGTH;
constexpr digit_t MINUS_ONE = LongArith::MINUS_ONE;

using container_type = LongArith::container_type;

#pragma region Internal Static Code
//****************** SIMPLE INTERNAL UTILS **********************

// how many digits we need to storage it
static size_t get_digit_count(digit_t val)
{
    assert(val < DIGIT_BASE);
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
template<typename dt>
static inline signed short compare_absolute_vectors(const std::vector<dt>& left, const std::vector<dt>& right)
{
    if (left.size() > right.size())
        return -1;
    if (left.size() < right.size())
        return 1;
    ssize_t index = left.size() - 1;
    while (index > 0 && left[index] == right[index])
        index--;
    if (left[index] == right[index])
        return 0;
    return (left[index] > right[index]) ? -1 : 1;
}


// Sum and diff

// Implementation of add_array
// Assume original and addition is different
inline void unchecked_internal_add_array(container_type& original, const container_type& addition, const size_t& shift)
{
    assert(&original != &addition);
    // when we work with addition, we keep in mind "virtual" digits
    if (addition.size() + shift > (original.capacity() << 1))
        original.reserve(addition.size() + shift + 1);
    compute_t sum = 0;
    size_t less = std::min(addition.size() + shift, original.size());
    size_t index = shift;
    // common part
    while (index < less)
    {
        sum += original[index];
        sum += addition[index - shift];
        original[index] = sum % DIGIT_BASE;
        sum /= DIGIT_BASE;
        index++;
    }
    // This will work if original shorter than addition
    while (index - shift < addition.size())
    {
        sum += addition[index - shift];
        original.push_back(sum % DIGIT_BASE);
        sum /= DIGIT_BASE;
        index++;
    }
    // This is addition of sum to storage
    while (index < original.size() && sum)
    {
        sum += original[index];
        original[index] = sum % DIGIT_BASE;
        sum /= DIGIT_BASE;
        index++;
    }
    // Now we push sum to empty positions
    while (sum)
    {
        original.push_back(sum % DIGIT_BASE);
        sum /= DIGIT_BASE;
    }
}

// increment original by addition and change original
// sum started from original[shift] and addition[0] (addition is shifted to left)
// \param original is changing vector
// \param addition is change
// \param shift is first index of original to change
static void add_array(container_type &original, const container_type &addition, const size_t& shift)
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
    assert(bigger.size() >= less.size());
    compute_t to_del = 0;
    for (size_t i = 0; i < less.size(); i++)
    {
        to_del += less[i];
        if (to_del > bigger[i])
        {
            bigger[i] = static_cast<digit_t>(DIGIT_BASE - (to_del - bigger[i]));
            to_del = 1;
        }
        else
        {
            bigger[i] = static_cast<digit_t>(bigger[i] - to_del);
            to_del = 0;
        }
    }
    for (size_t i = less.size(); i < bigger.size() && to_del; i++)
    {
        if (to_del > bigger[i])
        {
            bigger[i] = static_cast<digit_t>(DIGIT_BASE - (to_del - bigger[i]));
            to_del = 1;
        }
        else
        {
            bigger[i] = static_cast<digit_t>(bigger[i] - to_del);
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
    int index = 0;
    while (cont && index < num.size())
    {
        cont = ++num[index] == DIGIT_BASE;
        if (cont)
        {
            num[index] = 0;
        }
        ++index;
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
    assert(!(num.size() == 1 && num.back() == 0));
    bool cont = true;
    size_t index = 0;
    while (cont && index < num.size())
    {
        if (num[index] == 0)
        {
            num[index] = DIGIT_BASE - 1;
        }
        else
        {
            --num[index];
            cont = false;
        }
        ++index;
    }
    if (num.size() > 1 && index == num.size() && num.back() == 0)
    {
        num.pop_back();
    }
    assert(!cont);
}


// This is always increase absolute value of array
// \param change must be positive
static void increment_array(container_type &arr, compute_t change)
{
    assert(change >= 0);
    switch (change)
    {
    case 0: return;
    case 1:
        inc1_array(arr);
        return;
    default:
        compute_t sum = change;
        for (size_t index = 0; sum && index < arr.size(); index++)
        {
            sum += arr[index];
            arr[index] = sum % DIGIT_BASE;
            sum /= DIGIT_BASE;
        }
        while (sum)
        {
            arr.push_back(sum % DIGIT_BASE);
            sum /= DIGIT_BASE;
        }
    }
}
// This will decrease absolute value of array.
// \param change must be lower than DIGIT_BASE
// \return if arr>change return true else false (means change digit)
static bool decrement_array(container_type &arr, digit_t change)
{
    assert(change >= 0);
    assert(change < DIGIT_BASE);
    compute_t comp_result = static_cast<compute_t>(arr[0]) - change;
    if (comp_result >= 0)
    {
        arr[0] = static_cast<digit_t>(comp_result);
        return true;
    }
    if (arr.size() == 1)
    {
        arr[0] = static_cast<digit_t>(-comp_result);
        return false;
    }
    else
    {
        assert(DIGIT_BASE + comp_result >= 0);
        arr[0] = 0;
        dec1_array(arr);
        arr[0] = static_cast<digit_t>(DIGIT_BASE + comp_result);
        return true;
    }
}

//======= Mult

// Multiplication of big value on small
// Complexity O(n), memory O(1)
static void mult_small(container_type& big_number, const digit_t& multiplicator)
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
        compute_t trans_product = 0, mult = static_cast<compute_t>(multiplicator);
        for (size_t i = 0; i < big_number.size(); i++)
        {
            trans_product = trans_product + mult * static_cast<compute_t>(big_number[i]);
            big_number[i] = trans_product % DIGIT_BASE;
            trans_product = trans_product / DIGIT_BASE;
        }
        while (trans_product)
        {
            big_number.push_back(trans_product % DIGIT_BASE);
            trans_product /= DIGIT_BASE;
        }
    }
}

// Multiplication of two long numbers
// Complexity is O(m1.size()*m2.size())
static container_type mult_big(const container_type& m1, const container_type& m2)
{
    using namespace std;
    container_type result = { 0 };
    result.reserve(m1.size() + m2.size());
    const container_type& bigger = (m1.size() > m2.size()) ? m1 : m2;
    const container_type& smaller = (m1.size() > m2.size()) ? m2 : m1;
    container_type trans_product;
    for (size_t i = 0; i < smaller.size();i++)
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
// Complexity: O(n^2) in mean, O(log(DIGIT_BASE)*n^2) in worst case
// \param dividend_and_remainder must begin from nonzero digit and have size in [divider.size(), divider.size()+1]
// \param divider is simple divider, must begin from nonzero character
digit_t divide_almost_same_len_vectors(container_type& dividend_and_remainder, const container_type& divider)
{
    assert(dividend_and_remainder.size() && divider.size());
    assert(dividend_and_remainder.size() >= divider.size() && dividend_and_remainder.size() <= divider.size() + 1);
    assert(dividend_and_remainder.back() && divider.back());
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
        dividend_beg = static_cast<compute_t>(dividend_and_remainder.back())*DIGIT_BASE + *(dividend_and_remainder.end() - 2);
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
        return main_div;
    }

    // Start to search good divider, because 1th digit/
    digit_t lower_div = dividend_beg / (divider_beg + 1);
    // check lower_div
    multiplicated = divider;
    mult_small(multiplicated, lower_div);
    assert(compare_absolute_vectors(multiplicated, dividend_and_remainder) > 0);
    substract_array(dividend_and_remainder, multiplicated);
    if (compare_absolute_vectors(dividend_and_remainder, divider) > 0)
    {
        return lower_div;
    }
    else
    {
        add_array(dividend_and_remainder, multiplicated, 0);
    }

    // Binary search of multiplier
    // O(log2 of DIGIT_BASE multiplications)
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
                    return mult;
                }
                if (cmp > 0)
                {
                    substract_array(multiplicated, divider);
                    substract_array(dividend_and_remainder, multiplicated);
                    mult--;
                    return mult;
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
                return middle;
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
    assert(dividable.size() >= divider.size());
    std::deque<digit_t> fraction; // because we insert from begin, use deque

    container_type current_part(dividable.end() - divider.size(), dividable.end());
    size_t not_checked_len = dividable.size() - divider.size();
    bool last = !not_checked_len;
    while (not_checked_len || last)
    {
        digit_t r = divide_almost_same_len_vectors(current_part, divider);
        current_part.resize(divider.size());
        fraction.push_front(r);

        size_t lead_zero_count = 0;
        for (auto iter = current_part.crbegin(); iter != current_part.crend() && !*iter;++iter)
        {
            lead_zero_count++;
        }

        if (lead_zero_count) // We move to begin, than
        {
            if (not_checked_len)
            {
                const size_t next_shift = (std::min)(lead_zero_count, not_checked_len);
                const size_t remain_count = divider.size() - next_shift;
                if (std::is_pod<digit_t>::value)
                {
                    digit_t* begin_ptr = &current_part[0];
                    // Move remainder
                    memmove(begin_ptr + next_shift, begin_ptr, remain_count * sizeof(digit_t));
                    // Copy new data in tail
                    memcpy(begin_ptr, &dividable[0] + not_checked_len - next_shift, next_shift * sizeof(digit_t));
                }
                else
                {
                    // Move remainder
                    std::copy_backward(current_part.begin(), current_part.begin() + remain_count, current_part.begin() + (next_shift + remain_count));
                    // Copy new data in tail
                    std::copy(dividable.begin() + not_checked_len - next_shift, dividable.begin() + not_checked_len, current_part.begin());
                }
                not_checked_len -= next_shift;
            }
        }
        else // if all digits continue to be occupied
        {
            if (not_checked_len)
            {
                const size_t new_size = divider.size() + 1;
                current_part.resize(new_size);

                if (std::is_pod<digit_t>::value)
                {
                    digit_t* begin_ptr = &current_part[0];
                    // Move remainder
                    memmove(begin_ptr + 1, begin_ptr, (new_size - 1) * sizeof(digit_t));
                }
                else
                {
                    // Move remainder
                    std::copy_backward(current_part.begin(), current_part.begin() + (new_size - 1), current_part.end());
                }
                // Copy new data in tail
                current_part[0] = dividable[not_checked_len - 1];
                not_checked_len--;
            }
        }

        if (!not_checked_len) // If it here, we end
            last = !last; // !last for handle not_checked_len==0 in first time, false in all other
    }
    return std::make_pair(container_type(fraction.begin(), fraction.end()), std::move(current_part));
}



#pragma endregion


#pragma region Class Methods


//***************** CLASS INTERFACE IMPLEMENTATIONS ****************************

LongArith::LongArith(compute_t default_value, size_t default_capacity)
{
    storage.reserve(default_capacity);
    negative = default_value < 0;
    // Avoiding integer overflow
    if (default_value == std::numeric_limits<compute_t>::min())
    {
        ++default_value;
        storage.push_back(1);
    }
    else
    {
        storage.push_back(0);
    }
    compute_t rest = static_cast<compute_t>(default_value);
    increment_array(storage, negative ? -rest : rest);
}

LongArith::LongArith(const LongArith& original) : storage(original.storage)
{
    negative = original.negative;
}

LongArith::LongArith(LongArith&& temporary) : storage(std::move(temporary.storage))
{
    negative = temporary.negative;
}


LongArith& LongArith::operator=(const LongArith& other)&
{
    if (this != &other)
    {
        if (other.storage.size() <= this->storage.capacity())
        {
            storage = other.storage;
            negative = other.negative;
        }
        else
        {
            LongArith tmp(other);
            this->swap(tmp);
        }
    }
    return *this;
}

LongArith& LongArith::operator=(LongArith&& temp)&
{
    std::swap(temp.storage, storage);
    negative = temp.negative;
    return *this;
}


// -1 if left>right, 1 if left<right, 0 otherwise
signed short LongArith::compare_absolute_values(const LongArith& left, const LongArith& rigth)
{
    return compare_absolute_vectors(left.storage, rigth.storage);
}


// Plus

LongArith& LongArith::operator+=(const LongArith& change)&
{
    if (negative == change.negative)
    {
        add_array(storage, change.storage, 0);
    }
    else
    {
        if (compare_absolute_values(*this, change) <= 0)
        {
            substract_array(storage, change.storage);
        }
        else
        {
            negative = change.negative;
            container_type tmp(storage);
            storage = container_type(change.storage);
            substract_array(storage, tmp);
        }
    }
    check_zero();
    return *this;
}

LongArith& LongArith::operator+=(LongArith&& change)&
{
    if (change.negative == negative)
    {
        add_array(storage, change.storage, 0);
    }
    else
    {
        if (compare_absolute_values(*this, change) <= 0)
        {
            substract_array(storage, change.storage);
        }
        else
        {
            negative = change.negative;
            std::swap(change.storage, storage);
            substract_array(storage, change.storage);
        }
    }
    check_zero();
    return *this;
}

LongArith& LongArith::operator+=(long change)&
{
    if (std::numeric_limits<compute_t>::max() < std::numeric_limits<long>::max())
    {
        return *this += LongArith(change);
    }
    if (negative == change < 0)
    {
        compute_t change_b(change); // to avoid integer overflow
        if (change_b == std::numeric_limits<compute_t>::min())
        {
            inc1_array(storage);
            change_b++;
        }
        increment_array(storage, change < 0 ? -change_b : change_b);
    }
    else
    {
        if (change>=DIGIT_BASE || change<=-static_cast<compute_t>(DIGIT_BASE))
        {
            (*this) += LongArith(change);
        }
        else
        {
            digit_t add = (change < 0) ? -change : change;
            bool not_changed_sign = decrement_array(storage, add);
            negative = (not_changed_sign == negative);
        }
    }
    check_zero();
    return *this;
}


LongArith& LongArith::operator++()&
{
    // this variant runs 1.4x faster than this+=1
    check_zero();
    if (negative)
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

// Minus

LongArith& LongArith::operator-=(const LongArith& change)&
{
    return *this += -change;
}

LongArith& LongArith::operator-=(LongArith&& change)&
{
    return (*this) += -std::move(change);
}

LongArith operator-(LongArith left, const LongArith& rigth)
{
    return std::move(left -= rigth);
}

LongArith operator-(LongArith left, LongArith&& rigth)
{
    return std::move(left -= std::move(rigth));
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
    if (!negative && equalsZero())
        negative = true;
    if (negative)
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
    if (!(a.equalsZero() || b.equalsZero()))
    {
        res.negative = a.negative != b.negative;
        res.storage = mult_big(a.storage, b.storage);
    }
    return res;
}

LongArith & LongArith::operator*=(const LongArith & multiplier)&
{
    return (*this = (*this)*multiplier);
}

LongArith& LongArith::operator*=(long multiplier)&
{
    if (multiplier < 0)
    {
        if (multiplier == std::numeric_limits<long>::min())
        {
            *this *= LongArith(multiplier);
        }
        this->negative = !this->negative;
        multiplier = -multiplier;
    }
    mult_small(storage, multiplier);
    return *this;
}


std::pair<LongArith, LongArith> LongArith::FractionAndRemainder(const LongArith& dividable, const LongArith& divider)
{
    typedef std::pair<LongArith, LongArith> t_result;
    // Argument check
    if (divider.equalsZero())
    {
        throw std::logic_error("Division by zero");
    }

    // Simple Cases
    if (dividable.equalsZero())
    {
        return t_result(0, 0);
    }
    const int abs_compare = LongArith::compare_absolute_values(dividable, divider);
    if (abs_compare == 0)
    {
        return t_result(LongArith((dividable.negative == divider.negative) ? 1 : -1),
            LongArith(0));
    }
    if (abs_compare > 0)
    {
        return t_result(0, dividable);
    }

    // Here divider is always lower than dividable

    LongArith fraction, remainder;
    std::tie(fraction.storage, remainder.storage) = divide_vectors(dividable.storage, divider.storage);
    fraction.negative = dividable.negative != divider.negative;
    remainder.negative = dividable.negative;

    return t_result(std::move(fraction), std::move(remainder));
}

std::pair<LongArith, long> LongArith::FractionAndRemainder(const LongArith & dividable, const long divider)
{
    // Argument check
    if (!divider)
    {
        throw std::logic_error("Division by zero");
    }

    typedef std::pair<LongArith, long> t_result;
    if (std::numeric_limits<long>::min() == divider)
    {
        auto res = FractionAndRemainder(dividable, LongArith(divider));
        return t_result(std::move(res.first), res.second.to_plain_int());
    }
    //static_assert(false, "Not finished");
    return std::pair<LongArith, long>();
}

compute_t LongArith::to_plain_int() const
{
    if (!plain_convertable())
        throw std::logic_error("Cannot convert to plain!");
    compute_t res = storage[0];
    if (storage.size() == 2)
        res += storage[1] * DIGIT_BASE;
    return res;
}


// Comparison

bool LongArith::operator<(const LongArith& other) const
{
    if (this == &other)
        return false;
    if (negative && !other.negative)
        return true;
    if (other.negative && !negative)
        return false;
    if (negative)
        return compare_absolute_values(*this, other) < 0;
    else
        return compare_absolute_values(*this, other) > 0;
}


bool LongArith::operator>(const LongArith& other) const
{
    if (this == &other)
        return false;
    if (negative && !other.negative)
        return false;
    if (other.negative && !negative)
        return true;
    if (negative)
        return compare_absolute_values(*this, other) > 0;
    else
        return compare_absolute_values(*this, other) < 0;
}


bool LongArith::operator==(const LongArith& other) const
{
    return (this == &other) || (negative == other.negative && compare_absolute_values(*this, other) == 0);
}

bool LongArith::operator<=(const LongArith& other) const
{
    return !this->operator>(other);
}


bool LongArith::operator>=(const LongArith& other) const
{
    return !this->operator<(other);
}

bool LongArith::operator!=(const LongArith& other) const
{
    return !this->operator==(other);
}

bool LongArith::equalsZero() const
{
    return storage.size() == 1 && storage[0] == 0;
}

int LongArith::sign() const
{
    if (negative)
        return -1;
    if (equalsZero())
        return 0;
    return 1;
}


std::ostream& operator<<(std::ostream& os, const LongArith& obj)
{
    return os << obj.toString();
}

std::istream& operator >> (std::istream& is, LongArith& obj)
{
    std::string s;
    std::istream& r = is >> s;
    obj = LongArith::fromString(s);
    return r;
}

std::string LongArith::toString() const
{
    std::stringstream res;
    if (negative && !equalsZero())
        res << "-";
    res << storage.back();
    for (ssize_t i = storage.size() - 2; i >= 0; --i)
    {
        int digits = get_digit_count(storage[i]);
        for (int j = 0; j < DIGIT_STRING_LENGTH - digits; ++j)
        {
            res << '0';
        }
        res << storage[i];
    }
    return res.str();
}


LongArith LongArith::fromString(std::string s)
{
    using namespace std;
    // Trim beginning
    ssize_t i = 0;
    while (i < s.length() && (s[i] == ' ' || s[i] == '\t'))
        i++;
    s.erase(0, i);
    if (!check_string(s))
        throw invalid_argument("Invalid string");
    bool negative = false;
    if (s[0] == '-' || s[0] == '+')
    {
        negative = s[0] == '-';
        s.erase(0, 1);;
    }
    // Working with digits
    LongArith result;
    result.negative = negative;
    container_type& digits = result.storage;
    digits.clear();
    digits.reserve(s.length() / DIGIT_STRING_LENGTH + 1);
    for (i = 0; i < s.length() / DIGIT_STRING_LENGTH + (s.length() % DIGIT_STRING_LENGTH > 0 ? 1 : 0); ++i)
    {
        ssize_t left = s.length() - (i + 1) * DIGIT_STRING_LENGTH;
        left = left >= 0 ? left : 0;
        ssize_t right = s.length() - i * DIGIT_STRING_LENGTH;
        digit_t tmp(0);
        for (; left < right; left++)
        {
            tmp = tmp * 10 + static_cast<int>(s[left] - '0');
        }
        digits.push_back(tmp);
    }
    return move(result);
}


#pragma endregion

