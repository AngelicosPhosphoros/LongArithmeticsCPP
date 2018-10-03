/**
Copyright (c) 2018 AngelicosPhosphoros
https://github.com/AngelicosPhosphoros/LongArithmeticsCPP/
*/

#pragma once
#include <ostream>
#include <algorithm>
#include <type_traits>


class LongArith
{
public:
	//****************** CONSTS **********************

	// this is 4-byte type, which max value is 4,294,967,295
	// Digits of our big number
	typedef unsigned long digit_t;
	// this is for intermediate computation
	// must be able to store DigitBase**2
	typedef signed long long compute_t;

	// base of our numeral system
	static constexpr compute_t DigitBase = 1000ULL * 1000 * 1000;
	static constexpr size_t DigitStringLength = 9; // how long string of one digit

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
	static signed short compare_absolute_values(const LongArith &left, const LongArith &rigth);


public:

	inline void swap(LongArith& other)& noexcept {
		if (&other != this) {
			this->storage.swap(other.storage);
		}
	}

	// Constructor. Initiate with zero
	LongArith();

	// With initial value
	LongArith(long default_value);

	LongArith(const LongArith &original) = default;

	LongArith(LongArith &&temporary) = default;


	// \brief Converts string in decimal format
	std::string to_string() const;

	// \brief Builds long number from decimal string
	// \detailed Builds long number from string, which can begin from '-'
	//           or '+' and can contain only decimal symbols
	static LongArith from_string(const std::string& s);

	// \brief Returns sign of number
	// \return Returns -1, if negative; 0, if 0; 1 if positive
	// \detailed Calculate sign of number.
	//             complexity is O(1)
	int sign() const noexcept;

	// \brief Divide dividend by divider, returns fraction and remainder
	// \detailed This function is provided to use in cases when user need both division and modulus results
	//              it calculate it with complexity O(m*n*log(DigitBase)) in worst case
	// \return Pair of fraction (first) and remainder (second)
	static std::pair<LongArith, LongArith> fraction_and_remainder(const LongArith& dividable, const LongArith& divider);

	// \brief Divide dividend by divider, returns fraction and remainder
	// \detailed This function is provided to use in cases when user need both division and modulus results when divider is plain number
	//           It works significantly faster than version that get LongArith divider
	//           Complexity of method is O(n)
	// \return Pair of fraction (first) and remainder (second)
	static std::pair<LongArith, long> fraction_and_remainder(const LongArith& dividable, const long divider);

	// \brief Divide value by 10^power
	// \param power - exponent of 10
	// \return value/10^power
	LongArith fast_divide_by_10(const size_t power) const;

	// \brief Divide value by 10^power
	// \param power - exponent of 10
	// \return value%10^power
	LongArith fast_remainder_by_10(const size_t power) const;


	// \return true, if value can be stored in compute_t
	inline bool plain_convertable()const {
		return storage.size() <= 2;
	}
	// \return value equal to this in plain version
	compute_t to_plain_int()const;

	//***************** OPERATORS ***************


	// \brief Arithmetic plus. Make copy of first argument. Complexity is O(n)
	friend LongArith operator+(LongArith a, const LongArith &b) {
		return std::move(a += b);
	};

	friend LongArith operator+(LongArith a, LongArith &&b) {
		return std::move(b += std::move(a));
	}

	friend LongArith operator+(LongArith a, const long b) {
		return a += b;
	}

	friend LongArith operator+(const long a, LongArith b) {
		return b += a;
	}

	// \brief Arithmetic substraction. Make copy of both arguments. Complexity is O(n)
	friend LongArith operator-(LongArith left, const LongArith &rigth) {
		return std::move(left -= rigth);
	}

	// \brief Arithmetic substraction. Make copy of first argument and change other. Complexity is O(n)
	friend LongArith operator-(LongArith left, LongArith&& rigth)
	{
		return std::move(left -= std::move(rigth));
	}

	friend LongArith operator-(LongArith left, const long rigth) {
		return std::move(left -= rigth);
	}

	friend LongArith operator-(const long left, LongArith rigth) {
		return -std::move(rigth -= left);
	}

	friend LongArith operator *(const LongArith& a, const LongArith& b);
	friend LongArith operator *(LongArith a, const long b) {
		return std::move(a *= b);
	}
	friend LongArith operator *(const long a, LongArith b) {
		return std::move(b *= a);
	}

	friend LongArith operator /(const LongArith& a, const LongArith& b) {
		return LongArith::fraction_and_remainder(a, b).first;
	}

	friend LongArith operator /(const LongArith& a, const long b) {
		return LongArith::fraction_and_remainder(a, b).first;
	}

	friend long operator%(const LongArith& a, const long b) {
		return LongArith::fraction_and_remainder(a, b).second;
	}

	friend LongArith operator %(const LongArith& a, const LongArith& b) {
		return LongArith::fraction_and_remainder(a, b).second;
	}

	// unary minus
	friend LongArith operator-(const LongArith& original) {
		LongArith result(original);
		result.set_negative(!original.get_negative());
		return result;
	}
	friend LongArith operator-(LongArith&& original) {
		original.set_negative(!original.get_negative());
		return std::move(original);
	}

	// Assignment with arithmetical
	LongArith & operator+=(const LongArith &change)&;

	LongArith & operator+=(LongArith &&change)&;

	LongArith & operator+=(long change)&;

	// I created only prefix increment, because it faster and enough
	LongArith & operator++()&;

	LongArith & operator-=(const LongArith &change)& {
		return *this += -change;
	}

	LongArith & operator-=(LongArith &&change)& {
		return (*this) += -std::move(change);
	}

	// I created only prefix decrement, because it faster and enough
	LongArith & operator--()&;
	LongArith & operator-=(long change)&;

	LongArith & operator*=(const LongArith& multiplier)& {
		return (*this = (*this)*multiplier);
	}

	LongArith & operator*=(long multiplier)&;

	LongArith & operator/=(const LongArith& divider)& {
		return (*this = LongArith::fraction_and_remainder(*this, divider).first);
	}

	LongArith & operator%=(const LongArith& divider)& {
		return (*this = LongArith::fraction_and_remainder(*this, divider).second);
	}

	//Compare
	friend bool operator<(const LongArith& left, const LongArith &right);

	friend bool operator<=(const LongArith& left, const LongArith &right);

	friend bool operator>(const LongArith& left, const LongArith &right);

	friend bool operator>=(const LongArith& left, const LongArith &right);

	friend  bool operator==(const LongArith& left, const LongArith &right);

	friend bool operator!=(const LongArith& left, const LongArith &right);

	// \brief true, if zero, false otherwise
	bool equals_zero() const noexcept;

	// other
	LongArith &operator=(const LongArith &other)& = default;

	LongArith &operator=(LongArith &&temp)& = default;

	friend std::ostream &operator<<(std::ostream &os, const LongArith &obj);

	friend std::istream &operator >> (std::istream &is, LongArith& obj);

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
		container_union(const size_t initial_capacity);
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
	inline void swap<LongArith>(LongArith& a, LongArith& b) noexcept {
		a.swap(b);
	}
}

template<typename Iter1, typename Iter2>
inline LongArith::container_union::container_union(Iter1 beg, Iter2 end) :container_union()
{
	Iter1 bg = beg;
	reserve(end - beg);
	for (; bg != end; ++bg)
		push_back(*bg);
}