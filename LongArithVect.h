#pragma once
#include <ostream>
#include <vector>
#include <algorithm>

#include <unordered_map>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

class LongArithVect
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
	LongArithVect(compute_t default_value, size_t default_capacity);

	// Change the value by abs(change)
	// bool argument added to prevent innecessary copy of change when we need to get -change
	//void increase(const LongArithVect &change, bool is_same_sign);
	//void increase(LongArithVect&&change, bool is_same_sign);

    // make negative zero normal
	inline void check_zero()
    {
        if (negative && equals_zero())
            negative = false;
    }

    // below zero if left more right, more zero if left less rigth and 0 otherwise
    // Complexity: if they has differen sizes - const; otherwise O(n)
    static signed short compare_absolute_values(const LongArithVect &left, const LongArithVect &rigth);


public:

    inline void swap(LongArithVect& other)& {
        if (&other != this) {
            std::swap(this->storage, other.storage);
            std::swap(this->negative, other.negative);
        }
    }

	LongArithVect(const LongArithVect &original);

	LongArithVect(LongArithVect &&temporary);

	LongArithVect(compute_t default_value) : LongArithVect(default_value, DEFAULT_DIGIT_CAPACITY) { }

    // Constructor. Initiate with zero
	LongArithVect() : LongArithVect(0) { }

    // \brief Converts string in decimal format
	std::string to_string() const;

    // \brief Builds long number from decimal string
    // \detailed Builds long number from string, which can begin from '-'
    //           or '+' and can contain only decimal symbols
	static LongArithVect from_string(std::string s);

    // \brief Returns sign of number
    // \return Returns -1, if negative; 0, if 0; 1 if positive
    // \detailed Calculate sign of number.
    //             complexity is O(1)
	int sign() const;

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithVect, LongArithVect> fraction_and_remainder(const LongArithVect& dividable, const LongArithVect& divider);

    // \brief Divide dividend by divider, returns fraction and remainder
    // \return Pair of fraction (first) and remainder (second)
    static std::pair<LongArithVect, long> fraction_and_remainder(const LongArithVect& dividable, const long divider);

    // \return true, if value can be stored in compute_t
    inline bool plain_convertable()const {
        return storage.size() <= 2;
    }
    // \return value equal to this in plain version
    compute_t to_plain_int()const;
    
    //***************** OPERATORS ***************


	// \brief Arithmetic plus. Make copy of first argument. Complexity is O(n)
    // \detailed Plus. If 
	friend LongArithVect operator+(LongArithVect a, const LongArithVect &b) {
        return std::move(a += b);
    };

	friend LongArithVect operator+(LongArithVect a, LongArithVect &&b) {
        return std::move(b += std::move(a));
    }

	friend LongArithVect operator-(LongArithVect left, const LongArithVect &rigth);

	friend LongArithVect operator *(const LongArithVect& a, const LongArithVect& b);
    friend LongArithVect operator /(const LongArithVect& a, const LongArithVect& b) {
        return LongArithVect::fraction_and_remainder(a, b).first;
    }

    friend LongArithVect operator %(const LongArithVect& a, const LongArithVect& b) {
        return LongArithVect::fraction_and_remainder(a, b).second;
    }

	// unary minus
	friend LongArithVect operator-(const LongArithVect& original) {
        LongArithVect result(original);
        result.negative = !original.negative;
        return std::move(result);
    }
	friend LongArithVect operator-(LongArithVect&& original) {
        original.negative = !original.negative;
        return std::move(original);
    }

	// Assignment with arithmetical
	LongArithVect & operator+=(const LongArithVect &change)&;
			    
	LongArithVect & operator+=(LongArithVect &&change)&;
			    
	LongArithVect & operator+=(long change)&;
			    
	// I created only prefix increment, because it faster and enough
	LongArithVect & operator++()&;
			    
	LongArithVect & operator-=(const LongArithVect &change)&;
			    
	LongArithVect & operator-=(LongArithVect &&change)&;
			    
	// I created only prefix decrement, because it faster and enough
	LongArithVect & operator--()&;
	LongArithVect & operator-=(long change)&;
	
    LongArithVect & operator*=(const LongArithVect& multiplier)&;
	LongArithVect & operator*=(long multiplier)&;

    LongArithVect & operator/=(const LongArithVect& divider)& {
        return (*this = LongArithVect::fraction_and_remainder(*this, divider).first);
    }

    LongArithVect & operator%=(const LongArithVect& divider)& {
        return (*this = LongArithVect::fraction_and_remainder(*this, divider).second);
    }


	//Logic
	bool operator<(const LongArithVect &other) const;

	bool operator<=(const LongArithVect &other) const;

	bool operator>(const LongArithVect &other) const;

	bool operator>=(const LongArithVect &other) const;

	bool operator==(const LongArithVect &other) const;

	bool operator!=(const LongArithVect &other) const;

    // \brief true, if zero, false otherwise
	bool equals_zero() const;

	// other
	LongArithVect &operator=(const LongArithVect &other)&;

	LongArithVect &operator=(LongArithVect &&temp)&;

	friend std::ostream &operator<<(std::ostream &os, const LongArithVect &obj);

	friend std::istream &operator >> (std::istream &is, LongArithVect& obj);
};

namespace std {
    template<>
    inline void swap<LongArithVect>(LongArithVect& a, LongArithVect& b) {
        a.swap(b);
    }
}


