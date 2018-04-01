#pragma once
#include <ostream>
#include <vector>
#include <algorithm>

#include <unordered_map>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

class LongArith
{
public:
	//****************** CONSTS **********************

	// this is 4-byte type, which max value is 4,294,967,295
	// Digits of our big number
	typedef unsigned long digit_t;
	// this is for intermediate computation
	// must be able to store DIGIT_BASE**2
	typedef signed long long compute_t;
    //****************** INTERNAL TYPES ****************************
    typedef std::vector<digit_t> container_type;

	// Default capacity of internal vector
	static const size_t DEFAULT_DIGIT_CAPACITY = 1;
	// base of our numeral system
	static constexpr compute_t DIGIT_BASE = 1000ULL * 1000 * 1000;
	static constexpr size_t DIGIT_STRING_LENGTH = 9; // how long string of one digit
	static constexpr digit_t MINUS_ONE = 0xFFFFFFFF; // This is value which indicates, that we delete from zero

private:
    
	//****************** INTERNAL DATA FIELDS **********************
	std::vector<digit_t> storage;
	bool negative;

	

	// internal constructors
	LongArith(compute_t default_value, size_t default_capacity);

	// Change the value by abs(change)
	// bool argument added to prevent innecessary copy of change when we need to get -change
	//void increase(const LongArith &change, bool is_same_sign);
	//void increase(LongArith&&change, bool is_same_sign);

    // make negative zero normal
	inline void check_zero()
    {
        if (negative && equalsZero())
            negative = false;
    }

    // below zero if left more right, more zero if left less rigth and 0 otherwise
    // Complexity: if they has differen sizes - const; otherwise O(n)
    static signed short compare_absolute_values(const LongArith &left, const LongArith &rigth);


public:

    inline void swap(LongArith& other)& {
        if (&other != this) {
            std::swap(this->storage, other.storage);
            std::swap(this->negative, other.negative);
        }
    }

	LongArith(const LongArith &original);

	LongArith(LongArith &&temporary);

	LongArith(compute_t default_value) : LongArith(default_value, DEFAULT_DIGIT_CAPACITY) { }

    // Constructor. Initiate with zero
	LongArith() : LongArith(0) { }

    // \brief Converts string in decimal format
	std::string toString() const;

    // \brief Builds long number from decimal string
    // \detailed Builds long number from string, which can begin from '-'
    //           or '+' and can contain only decimal symbols
	static LongArith fromString(std::string s);

    // \brief Returns sign of number
    // \return Returns -1, if negative; 0, if 0; 1 if positive
    // \detailed Calculate sign of number.
    //             complexity is O(1)
	int sign() const;

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArith, LongArith> FractionAndRemainder(const LongArith& dividable, const LongArith& divider);

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArith, long> FractionAndRemainder(const LongArith& dividable, const long divider);

    // \return true, if value can be stored in compute_t
    inline bool plain_convertable()const {
        return storage.size() <= 2;
    }
    // \return value equal to this in plain version
    compute_t to_plain_int()const;
    
    //***************** OPERATORS ***************


	// \brief Arithmetic plus. Make copy of first argument. Complexity is O(n)
    // \detailed Plus. If 
	friend LongArith operator+(LongArith a, const LongArith &b) {
        return std::move(a += b);
    };

	friend LongArith operator+(LongArith a, LongArith &&b) {
        return std::move(b += std::move(a));
    }

	friend LongArith operator-(LongArith left, const LongArith &rigth);

	friend LongArith operator *(const LongArith& a, const LongArith& b);
    friend LongArith operator /(const LongArith& a, const LongArith& b) {
        return LongArith::FractionAndRemainder(a, b).first;
    }

    friend LongArith operator %(const LongArith& a, const LongArith& b) {
        return LongArith::FractionAndRemainder(a, b).second;
    }

	// unary minus
	friend LongArith operator-(const LongArith& original) {
        LongArith result(original);
        result.negative = !original.negative;
        return std::move(result);
    }
	friend LongArith operator-(LongArith&& original) {
        original.negative = !original.negative;
        return std::move(original);
    }

	// Assignment with arithmetical
	LongArith & operator+=(const LongArith &change)&;
			    
	LongArith & operator+=(LongArith &&change)&;
			    
	LongArith & operator+=(long change)&;
			    
	// I created only prefix increment, because it faster and enough
	LongArith & operator++()&;
			    
	LongArith & operator-=(const LongArith &change)&;
			    
	LongArith & operator-=(LongArith &&change)&;
			    
	// I created only prefix decrement, because it faster and enough
	LongArith & operator--()&;
	LongArith & operator-=(long change)&;
	
    LongArith & operator*=(const LongArith& multiplier)&;
	LongArith & operator*=(long multiplier)&;

    LongArith & operator/=(const LongArith& divider)& {
        return (*this = LongArith::FractionAndRemainder(*this, divider).first);
    }

    LongArith & operator%=(const LongArith& divider)& {
        return (*this = LongArith::FractionAndRemainder(*this, divider).second);
    }


	//Logic
	bool operator<(const LongArith &other) const;

	bool operator<=(const LongArith &other) const;

	bool operator>(const LongArith &other) const;

	bool operator>=(const LongArith &other) const;

	bool operator==(const LongArith &other) const;

	bool operator!=(const LongArith &other) const;

    // \brief true, if zero, false otherwise
	bool equalsZero() const;

	// other
	LongArith &operator=(const LongArith &other)&;

	LongArith &operator=(LongArith &&temp)&;

	friend std::ostream &operator<<(std::ostream &os, const LongArith &obj);

	friend std::istream &operator >> (std::istream &is, LongArith& obj);
};

namespace std {
    template<>
    inline void swap<LongArith>(LongArith& a, LongArith& b) {
        a.swap(b);
    }
}


