#pragma once
#include <ostream>
#include <algorithm>
#include <type_traits>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

class LongArithLast
{
public:
    //****************** CONSTS **********************

    // this is 4-byte type, which max value is 4,294,967,295
    // Digits of our big number
    typedef unsigned long digit_t;
    // this is for intermediate computation
    // must be able to store DIGIT_BASE**2
    typedef signed long long compute_t;

    // base of our numeral system
    static constexpr compute_t DIGIT_BASE = 1000ULL * 1000 * 1000;
    static constexpr size_t DIGIT_STRING_LENGTH = 9; // how long string of one digit

    // assertions
    static_assert(sizeof(digit_t) <= sizeof(compute_t), "compute_t must be bigger than digit_t");
    static_assert(std::is_pod<digit_t>::value && std::is_pod<compute_t>::value, "compute_t and digit_t must be POD types");
    static_assert(std::is_integral<digit_t>::value && std::is_integral<compute_t>::value, "compute_t and digit_t must be integral types");
    static_assert(std::is_signed<compute_t>::value, "compute_t must be signed");

private:

    // getters and setters
    inline bool get_negative()const noexcept
    {
        return storage.negative();
    }
    inline void set_negative(const bool neg) noexcept
    {
        storage.set_negative(neg);
    }

    // make negative zero normal
    inline void check_zero()
    {
        if (get_negative() && equals_zero())
            set_negative(false);
    }

    // below zero if left more right, more zero if left less rigth and 0 otherwise
    // Complexity: if they has differen sizes - const; otherwise O(n)
    static signed short compare_absolute_values(const LongArithLast &left, const LongArithLast &rigth);


public:

    inline void swap(LongArithLast& other)& noexcept {
        if (&other != this) {
            this->storage.swap(other.storage);
        }
    }

    // Constructor. Initiate with zero
    LongArithLast();

    // With initial value
    LongArithLast(long default_value);

    LongArithLast(const LongArithLast &original) = default;

    LongArithLast(LongArithLast &&temporary) = default;


    // \brief Converts string in decimal format
    std::string to_string() const;

    // \brief Builds long number from decimal string
    // \detailed Builds long number from string, which can begin from '-'
    //           or '+' and can contain only decimal symbols
    static LongArithLast from_string(const std::string& s);

    // \brief Returns sign of number
    // \return Returns -1, if negative; 0, if 0; 1 if positive
    // \detailed Calculate sign of number.
    //             complexity is O(1)
    int sign() const noexcept;

    // \brief Divide dividend by divider, returns fraction and remainder
    // \detailed This function is provided to use in cases when user need both division and modulus results
    //              it calculate it with complexity O(m*n*log(DIGIT_BASE)) in worst case
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithLast, LongArithLast> fraction_and_remainder(const LongArithLast& dividable, const LongArithLast& divider);

    // \brief Divide dividend by divider, returns fraction and remainder
    // \detailed This function is provided to use in cases when user need both division and modulus results when divider is plain number
    //           It works significantly faster than version that get LongArithLast divider
    //           Complexity of method is O(n)
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithLast, long> fraction_and_remainder(const LongArithLast& dividable, const long divider);

    // \brief Divide value by 10^power
    // \param power - exponent of 10
    // \return value/10^power
    LongArithLast fast_divide_by_10(const size_t power) const;

    // \brief Divide value by 10^power
    // \param power - exponent of 10
    // \return value%10^power
    LongArithLast fast_remainder_by_10(const size_t power) const;


    // \return true, if value can be stored in compute_t
    inline bool plain_convertable()const {
        return storage.size() <= 2;
    }
    // \return value equal to this in plain version
    compute_t to_plain_int()const;

    //***************** OPERATORS ***************


    // \brief Arithmetic plus. Make copy of first argument. Complexity is O(n)
    // \detailed Plus. If 
    friend LongArithLast operator+(LongArithLast a, const LongArithLast &b) {
        return std::move(a += b);
    };

    friend LongArithLast operator+(LongArithLast a, LongArithLast &&b) {
        return std::move(b += std::move(a));
    }

    friend LongArithLast operator-(LongArithLast left, const LongArithLast &rigth);

    friend LongArithLast operator *(const LongArithLast& a, const LongArithLast& b);
    friend LongArithLast operator /(const LongArithLast& a, const LongArithLast& b) {
        return LongArithLast::fraction_and_remainder(a, b).first;
    }

    friend LongArithLast operator %(const LongArithLast& a, const LongArithLast& b) {
        return LongArithLast::fraction_and_remainder(a, b).second;
    }

    // unary minus
    friend LongArithLast operator-(const LongArithLast& original) {
        LongArithLast result(original);
        result.set_negative(!original.get_negative());
        return result;
    }
    friend LongArithLast operator-(LongArithLast&& original) {
        original.set_negative(!original.get_negative());
        return std::move(original);
    }

    // Assignment with arithmetical
    LongArithLast & operator+=(const LongArithLast &change)&;

    LongArithLast & operator+=(LongArithLast &&change)&;

    LongArithLast & operator+=(long change)&;

    // I created only prefix increment, because it faster and enough
    LongArithLast & operator++()&;

    LongArithLast & operator-=(const LongArithLast &change)&;

    LongArithLast & operator-=(LongArithLast &&change)&;

    // I created only prefix decrement, because it faster and enough
    LongArithLast & operator--()&;
    LongArithLast & operator-=(long change)&;

    LongArithLast & operator*=(const LongArithLast& multiplier)&;
    LongArithLast & operator*=(long multiplier)&;

    LongArithLast & operator/=(const LongArithLast& divider)& {
        return (*this = LongArithLast::fraction_and_remainder(*this, divider).first);
    }

    LongArithLast & operator%=(const LongArithLast& divider)& {
        return (*this = LongArithLast::fraction_and_remainder(*this, divider).second);
    }

    //Logic
    bool operator<(const LongArithLast &other) const;

    bool operator<=(const LongArithLast &other) const;

    bool operator>(const LongArithLast &other) const;

    bool operator>=(const LongArithLast &other) const;

    bool operator==(const LongArithLast &other) const;

    bool operator!=(const LongArithLast &other) const;

    // \brief true, if zero, false otherwise
    bool equals_zero() const noexcept;

    // other
    LongArithLast &operator=(const LongArithLast &other)& = default;

    LongArithLast &operator=(LongArithLast &&temp)& = default;

    friend std::ostream &operator<<(std::ostream &os, const LongArithLast &obj);

    friend std::istream &operator >> (std::istream &is, LongArithLast& obj);

protected:

    //****************** INTERNAL TYPES ****************************
    //typedef std::vector<digit_t> container_type;
    // This is container that work same way as vector but keep small storages directly in stack
    struct container_union {

        // Local data
        unsigned short is_local : 1;
        unsigned short is_negative : 1;
        uint8_t local_size;
        digit_t* data_pointer;

        struct heap_dt {
            size_t size;
            size_t capacity;
        };
        // Minimum of max value of local size and heap_dt memory amount
        constexpr static size_t local_capacity = std::min<size_t>(std::numeric_limits<uint8_t>::max(),
            sizeof(heap_dt) / sizeof(digit_t));
        // various data
        union
        {
            digit_t local_data[local_capacity];
            heap_dt heap_data;
        };

        // Constructors
        container_union() noexcept;
        container_union(const container_union& other);
        container_union(container_union&& tmp) noexcept;
        template<typename Iter1, typename Iter2>
        container_union(Iter1 beg, Iter2 end);
        template<>
        container_union(const digit_t * beg, const digit_t * end);
        ~container_union();
        container_union& operator= (const container_union& other);
        container_union& operator= (container_union&& tmp) noexcept;
        void swap(container_union& other)& noexcept;
    public:
        inline bool negative()const noexcept {
            return is_negative;
        }
        inline void set_negative(const bool v) noexcept {
            is_negative = v;
        }
        inline  digit_t& operator[](const size_t index);
        inline const digit_t& operator[](const size_t index)const;
        inline size_t size()const noexcept;
        inline size_t capacity() const noexcept;
        void resize(const size_t new_size);
        void reserve(const size_t new_capacity);
        inline void clear();
        void push_back(const digit_t val);
        digit_t back() const;
        void pop_back();

        // Iterators
        inline digit_t* begin() noexcept;
        inline digit_t* end() noexcept;

        inline const digit_t* begin() const noexcept;
        inline const digit_t* end()const noexcept;

    private:
        inline void switch_to_heap(const size_t reserve_amount);
        inline void reallocate(const size_t new_capacity);
    };
    typedef container_union container_type;
    static_assert(std::is_nothrow_move_assignable<container_type>::value && std::is_nothrow_move_constructible<container_type>::value, "nothrow guarantee check failed");
private:

    //****************** INTERNAL DATA FIELDS **********************
    container_type storage;
};

namespace std {
    template<>
    inline void swap<LongArithLast>(LongArithLast& a, LongArithLast& b) noexcept {
        a.swap(b);
    }
}

template<typename Iter1, typename Iter2>
inline LongArithLast::container_union::container_union(Iter1 beg, Iter2 end) :container_union()
{
    Iter1 bg = beg;
    reserve(end - beg);
    for (;bg != end;++bg)
        push_back(*bg);
}