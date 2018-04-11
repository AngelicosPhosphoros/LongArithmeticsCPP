// LongArithmeticCPP.cpp : Defines the entry point for the console application.
//
#include "LongArithVect.h"
#include "LongArithUnion.h"
#include "LongArithLast.h"
#include "LongArith.h"
#include <iostream>
#include <sstream>
#include <list>

#ifdef _WIN32
#ifndef _MSC_VER
#include <x86intrin.h>
#endif

uint64_t rdtsc() {
    return __rdtsc();
}
//  Linux/GCC
#else

uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif

#include <fstream>
std::ofstream out("out.txt");
//std::ostream& out = std::cout;
std::stringstream tout = std::stringstream();
std::ostringstream garbage;

const char garbage_marker[] = "garbage out: ",
begin_marker[] = "BENCH_BEGIN: ",
end_marker[] = "BENCH_END in time: ";
using std::endl;

#define NOINLINE __declspec(noinline)

template<typename arith>
NOINLINE void simple_operation_benchmark()
{
    tout << begin_marker << "Simple" << endl;
    const size_t begin = rdtsc();

    arith a;
    for (size_t i = 0; i < 10000000; ++i)
        ++a;
    garbage << a;
    arith b = arith::from_string("1654984191651891");
    for (size_t i = 0; i < 10000000; ++i)
        ++b;
    garbage << b;

    a = arith(1000000);
    for (arith i = 0; i < a; ++i)
        b += arith(15057);
    garbage << b;

    tout << "\tPlus: " << rdtsc() - begin << endl;
    const size_t t = rdtsc();
    a /= 10;
    b = 1;
    for (arith i = 0; i < a; ++i)
        b = b * 2;
    garbage << b;
    tout << "\tMult: " << rdtsc() - t << endl;
    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
}


template<typename arith>
NOINLINE void vector_op_benchmark()
{
    tout << begin_marker << "VECT" << endl;
    tout << "Mixed sized\n";
    const size_t begin = rdtsc();
    std::vector<arith> t;
    arith M(4000000);
    for (arith i = 0; i < M; ++i)
    {
        if (rand() % 100 > 95)
        {
            t.push_back(rand() % 1000);
        }
        else
        {
            arith tmp = rand() % 1000;
            for (size_t i = 0; i < 10;++i)
                tmp *= rand() % 1000;
            t.push_back(tmp);
        }
    }
    tout << rdtsc() - begin << endl;
    for (auto& tmp : t)
        garbage << tmp;
    tout << "Dealloc\n";

    size_t beg = rdtsc();
    t.clear();
    tout << rdtsc() - beg << endl;

    tout << "Long\n";
    beg = rdtsc();
    for (arith i = 0; i < M; ++i)
    {
        arith tmp = rand() % 1000;
        for (size_t i = 0; i < 40;++i)
            tmp *= rand() % 1000;
        t.push_back(tmp);

    }
    tout << rdtsc() - beg << endl;

    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
    for (auto& tmp : t)
        garbage << tmp;
}

template<typename arith>
void factorial_op_benchmark_long()
{
    tout << begin_marker << "Factorial Long" << endl;
    const size_t begin = rdtsc();
    arith M(200 * 1000);
    arith fact = 1;
    for (arith i = 1; i <= M; ++i)
    {
        fact *= i;
    }
    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
    garbage << endl << fact << endl;
}

template<typename arith>
void factorial_op_benchmark_plain()
{
    tout << begin_marker << "Factorial" << endl;
    const size_t begin = rdtsc();
    long M(200 * 1000);
    arith fact = 1;
    for (long i = 1; i <= M; ++i)
    {
        fact *= i;
    }
    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
    garbage << endl << fact << endl;
}


int main()
{
    using namespace  std;

    const char vect_label[] = "Vector impl\n", union_label[] = "Only Union impl\n", union2_label[] = "Union 2 impl\n",
        last_label[] = "Current version\n";
    tout << vect_label;
    simple_operation_benchmark<LongArithVect>();
    tout << union_label;
    simple_operation_benchmark<LongArithUnion>();
    tout << union2_label;
    simple_operation_benchmark<LongArithLast>();
    tout << last_label;
    simple_operation_benchmark<LongArith>();

    tout << endl;

    tout << vect_label;
    vector_op_benchmark<LongArithVect>();
    tout << union_label;
    vector_op_benchmark<LongArithUnion>();
    tout << union2_label;
    vector_op_benchmark<LongArithLast>();
    tout << last_label;
    vector_op_benchmark<LongArith>();

    tout << endl;

    tout << vect_label;
    factorial_op_benchmark_long<LongArithVect>();
    tout << union_label;
    factorial_op_benchmark_long<LongArithUnion>();
    tout << union2_label;
    factorial_op_benchmark_long<LongArithLast>();
    tout << last_label;
    factorial_op_benchmark_long<LongArith>();

    tout << endl;

    tout << vect_label;
    factorial_op_benchmark_plain<LongArithVect>();
    tout << union_label;
    factorial_op_benchmark_plain<LongArithUnion>();
    tout << union2_label;
    factorial_op_benchmark_plain<LongArithLast>();
    tout << last_label;
    factorial_op_benchmark_plain<LongArith>();
    
    out << tout.str();
    ofstream("tmp.txt") << garbage_marker << garbage.str();
    ofstream("tmp.txt").clear();

    return 0;
}

