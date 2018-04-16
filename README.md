Long Arithmetics C++
======================

This is a optimized realization of signed integer long arithmetics on C++

# How to use

Just include LongArith.h and LongArith.cpp into your C++ project

# Operations

All operations can get plain `long` arguments as well as `LongArith`. Plain versions work faster (around five times). Have fast division methods for number that powers of 10.

`-`: Unary minus. O(n) if argument is lvalue, O(1) if argument is rvalue.

`++`, `--`: Prefix in- and decrements. I provide only prefix versions to eliminate copying.

`+`, `-`: Simple addition and substraction. Complexity is O(n), memory usage O(n). If possible, it is better to be replaced by `+=` and `-=`, because this methods just create copy of long object than call assignment on the copy.

`+=`: Arithmetic assignment. Complexity is O(n), memory usage O(1) in best cases and O(n) on worse.<br>
`-=`: Arithmetic assignment. Complexity is O(n), memory usage O(n).

`*`: Multiplication. Complexity is O(n\*m), memory usage O(n\*m). Must be preferred if both operands are LongArith.<br>
`*=`: Multiplication assignment. Must be preferred if rigth operand is `long`, else `a*=b` is same as `a = a*b`.

Static method `fraction_and_remainder(a,b)`: `a` must be LongArith, `b` can be `long`, can be LongArith. Returns `std::pair`, which `first` is division result and `second` is remainder. Complexity is O(n\*n) in mean, O(n\*n\*log(DigitBase)) in worse cases.

`/`, `%`, `/=`, `%=`: Just calls `fraction_and_remainder` and returns needed result.

`fast_divide_by_10` and `fast_remainder_by_10` use internal representation of long number to divide more fast. Their argument is power of 10 (If you need to divide by `10^n`, you need to call `fast_divide_by_10(n)`). Complexity O(power) for remainder, O(len - power) for division.

`from_string` build LongArith from std::string.<br>
`to_string` convert LongArith to std::string.

`>>` and `<<` is standart stream input and output operations. They use `from_string` and `to_string` internally.

`equals_zero` returns `true` if equals zero<br>
`sign` returns -1 if negative, 0 if equals zero and 1 if positive.

# Internal representation and optimizations
Digits of long number is encoded with notation of 1000 000 000 (`DigitBase` constant) to decrease number of operations. It uses 10-power base to enhance perfomance of string conversion.

Move semantics used everywhere where it can be used.

Internal representation is special struct that keep small numbers direcly in local space without allocation of memory on heap (likely to *Small string optimization*). This improve speed of working with a lot of LongArith in vectors, etc. by eliminating of cache errors. On MS VS x64 numbers lower than 10^36 can be stored locally.

If capacity of local storage is exceeded, it allocate memory in heap and use local space to keep capacity and size of heap data.
