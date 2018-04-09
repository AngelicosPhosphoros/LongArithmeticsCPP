#pragma once
#include <ostream>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <unordered_map>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

class LongArithUnion
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
    static signed short compare_absolute_values(const LongArithUnion &left, const LongArithUnion &rigth);


public:

    inline void swap(LongArithUnion& other)& noexcept {
        if (&other != this) {
            this->storage.swap(other.storage);
        }
    }

    // Constructor. Initiate with zero
    LongArithUnion();

    // With initial value
    LongArithUnion(long default_value);

    LongArithUnion(const LongArithUnion &original) = default;

    LongArithUnion(LongArithUnion &&temporary) = default;


    // \brief Converts string in decimal format
    std::string to_string() const;

    // \brief Builds long number from decimal string
    // \detailed Builds long number from string, which can begin from '-'
    //           or '+' and can contain only decimal symbols
    static LongArithUnion from_string(const std::string& s);

    // \brief Returns sign of number
    // \return Returns -1, if negative; 0, if 0; 1 if positive
    // \detailed Calculate sign of number.
    //             complexity is O(1)
    int sign() const noexcept;

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithUnion, LongArithUnion> fraction_and_remainder(const LongArithUnion& dividable, const LongArithUnion& divider);

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithUnion, long> fraction_and_remainder(const LongArithUnion& dividable, const long divider);

    // \brief Divide value by 10^power
    // \param power - exponent of 10
    // \return value/10^power
    LongArithUnion fast_divide_by_10(const size_t power) const;

    // \brief Divide value by 10^power
    // \param power - exponent of 10
    // \return value%10^power
    LongArithUnion fast_remainder_by_10(const size_t power) const;


    // \return true, if value can be stored in compute_t
    inline bool plain_convertable()const {
        return storage.size() <= 2;
    }
    // \return value equal to this in plain version
    compute_t to_plain_int()const;

    //***************** OPERATORS ***************


    // \brief Arithmetic plus. Make copy of first argument. Complexity is O(n)
    // \detailed Plus. If 
    friend LongArithUnion operator+(LongArithUnion a, const LongArithUnion &b) {
        return std::move(a += b);
    };

    friend LongArithUnion operator+(LongArithUnion a, LongArithUnion &&b) {
        return std::move(b += std::move(a));
    }

    friend LongArithUnion operator-(LongArithUnion left, const LongArithUnion &rigth);

    friend LongArithUnion operator *(const LongArithUnion& a, const LongArithUnion& b);
    friend LongArithUnion operator /(const LongArithUnion& a, const LongArithUnion& b) {
        return LongArithUnion::fraction_and_remainder(a, b).first;
    }

    friend LongArithUnion operator %(const LongArithUnion& a, const LongArithUnion& b) {
        return LongArithUnion::fraction_and_remainder(a, b).second;
    }

    // unary minus
    friend LongArithUnion operator-(const LongArithUnion& original) {
        LongArithUnion result(original);
        result.set_negative(!original.get_negative());
        return result;
    }
    friend LongArithUnion operator-(LongArithUnion&& original) {
        original.set_negative(!original.get_negative());
        return std::move(original);
    }

    // Assignment with arithmetical
    LongArithUnion & operator+=(const LongArithUnion &change)&;

    LongArithUnion & operator+=(LongArithUnion &&change)&;

    LongArithUnion & operator+=(long change)&;

    // I created only prefix increment, because it faster and enough
    LongArithUnion & operator++()&;

    LongArithUnion & operator-=(const LongArithUnion &change)&;

    LongArithUnion & operator-=(LongArithUnion &&change)&;

    // I created only prefix decrement, because it faster and enough
    LongArithUnion & operator--()&;
    LongArithUnion & operator-=(long change)&;

    LongArithUnion & operator*=(const LongArithUnion& multiplier)&;
    LongArithUnion & operator*=(long multiplier)&;

    LongArithUnion & operator/=(const LongArithUnion& divider)& {
        return (*this = LongArithUnion::fraction_and_remainder(*this, divider).first);
    }

    LongArithUnion & operator%=(const LongArithUnion& divider)& {
        return (*this = LongArithUnion::fraction_and_remainder(*this, divider).second);
    }

    //Logic
    bool operator<(const LongArithUnion &other) const;

    bool operator<=(const LongArithUnion &other) const;

    bool operator>(const LongArithUnion &other) const;

    bool operator>=(const LongArithUnion &other) const;

    bool operator==(const LongArithUnion &other) const;

    bool operator!=(const LongArithUnion &other) const;

    // \brief true, if zero, false otherwise
    bool equals_zero() const noexcept;

    // other
    LongArithUnion &operator=(const LongArithUnion &other)& = default;

    LongArithUnion &operator=(LongArithUnion &&temp)& = default;

    friend std::ostream &operator<<(std::ostream &os, const LongArithUnion &obj);

    friend std::istream &operator >> (std::istream &is, LongArithUnion& obj);

protected:

    //****************** INTERNAL TYPES ****************************
    //typedef std::vector<digit_t> container_type;
    // This is container that work same way as vector but keep small storages directly in stack
    union container_union {
        // types
        struct local_dt {
            bool is_local : 1;
            bool negative : 1;
            unsigned short size : 4;
            // Minimum of max value of 4 bit size and vector memory amount
            constexpr static size_t container_capacity = std::min<size_t>(0xF, sizeof(std::vector<digit_t>) / sizeof(digit_t));
            digit_t data[container_capacity];
            local_dt()noexcept : is_local(true), size(0), negative(false) {}
            local_dt(const bool is_local, const bool is_negative, size_t size) noexcept;
        };
        struct heap_dt {
            bool is_local : 1;
            bool negative : 1;
            std::vector<digit_t> vdata;
            heap_dt(): is_local(false), vdata() {}
        };

        // Checks
        static_assert(std::is_nothrow_move_assignable<std::vector<digit_t>>::value && std::is_nothrow_move_constructible<std::vector<digit_t>>::value, "nothrow guarantee check failed");
        static_assert(std::is_nothrow_move_assignable<local_dt>::value && std::is_nothrow_move_constructible<local_dt>::value, "nothrow guarantee check failed");
        static_assert(std::is_nothrow_move_assignable<heap_dt>::value && std::is_nothrow_move_constructible<heap_dt>::value, "nothrow guarantee check failed");

        container_union();
        container_union(const container_union& other);
        container_union(container_union&& tmp) noexcept;
        template<typename Iter1, typename Iter2>
        container_union(Iter1 beg, Iter2 end);
        template<>
        container_union(const digit_t * beg, const digit_t * end);
        ~container_union();
        inline container_union& operator= (const container_union& other);
        inline container_union& operator= (container_union&& tmp) noexcept;
        void swap(container_union& other)& noexcept;
    public:
        inline bool negative()const noexcept {
            return is_local ? local_data.negative : heap_data.negative;
        }
        inline void set_negative(const bool v) noexcept {
            if (is_local)
                local_data.negative = v;
            else
                heap_data.negative = v;
        }
        inline  digit_t& operator[](const size_t index);
        inline const digit_t& operator[](const size_t index)const;
        inline size_t size()const noexcept;
        inline size_t capacity() const noexcept;
        inline void resize(const size_t new_size);
        inline void reserve(const size_t new_capacity);
        inline void clear();
        inline void push_back(const digit_t val);
        inline digit_t back() const;
        inline void pop_back();

        // Iterators
        inline digit_t* begin() noexcept;
        inline digit_t* end() noexcept;

        inline const digit_t* begin() const noexcept;
        inline const digit_t* end()const noexcept;

    private:
        // Union data
        bool is_local : 1; // same as local_data.is_local and heap_data.is_local
        local_dt local_data;
        heap_dt heap_data;
        static void copy_heap_to_stack(local_dt & dest, const heap_dt & source) noexcept;
        void switch_to_heap(const size_t reserve_amount);
    };
    typedef container_union container_type;
    static_assert(std::is_nothrow_move_assignable<container_type>::value && std::is_nothrow_move_constructible<container_type>::value, "nothrow guarantee check failed");
private:

    //****************** INTERNAL DATA FIELDS **********************
    container_type storage;
};

namespace std {
    template<>
    inline void swap<LongArithUnion>(LongArithUnion& a, LongArithUnion& b) noexcept {
        a.swap(b);
    }
}

template<typename Iter1, typename Iter2>
inline LongArithUnion::container_union::container_union(Iter1 beg, Iter2 end) :container_union()
{
    Iter1 bg = beg;
    reserve(end - beg);
    for (;bg != end;++bg)
        push_back(*bg);
}