// LongArithmeticCPP.cpp : Defines the entry point for the console application.
//
#include "LongArithVect.h"
#include "LongArithUnion.h"
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
    for (size_t i = 0; i < 1000000; ++i)
        ++a;
    garbage << a;
    arith b = arith::from_string("1654984191651891");
    for (size_t i = 0; i < 1000000; ++i)
        ++b;
    garbage << b;

    a = arith(100000);
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
    const size_t begin = rdtsc();
    std::vector<arith> t;
    arith M(1000000);
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
    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
    for (auto& tmp : t)
        garbage << tmp;
}


int main()
{
    using namespace  std;

    const char vect_label[] = "Vector impl\n", union_label[] = "Only Union impl\n";
    tout << vect_label;
    simple_operation_benchmark<LongArithVect>();
    tout << union_label;
    simple_operation_benchmark<LongArithUnion>();
    
    tout << endl;

    tout << vect_label;
    vector_op_benchmark<LongArithVect>();
    tout << union_label;
    vector_op_benchmark<LongArithUnion>();


    //fast_division_benchmark();
    out << tout.str();
    ofstream("tmp.txt") << garbage_marker << garbage.str();

    
    return 0;
}

