// LongArithmeticCPP.cpp : Defines the entry point for the console application.
//
#include "LongArith.h"
#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <tuple>

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
//using arith = LongArith;
template<typename arith>
void simple_operation_benchmark()
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
void vector_op_benchmark()
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

void fast_division_benchmark()
{
    tout << begin_marker << "FAST DIV" << endl;
    const size_t begin = rdtsc();

    std::vector<LongArith> t;
    constexpr size_t N = 100000;
    LongArith M(N);
    for (LongArith i = 0; i < M; ++i)
    {
        LongArith tmp = rand() % 1000;
        for (size_t i = 0; i < 100;++i)
            tmp *= rand() % 1000;
        t.push_back(tmp);
    }

    std::vector<LongArith> dividers = { 1 };
    for (LongArith i = 1; i < M; ++i)
    {
        dividers.emplace_back(dividers.back() * 10);
    }

    tout << "Set generated in " << rdtsc() - begin << endl;

    std::vector<std::pair<LongArith, LongArith>> result1, result2;
    result1.reserve(M.to_plain_int()); result2.reserve(M.to_plain_int());



    const size_t first_begin = rdtsc();
    for (size_t i = 0; i < N; ++i)
    {
        auto r = LongArith::fraction_and_remainder(t[i], dividers[i]);
        result1.emplace_back(std::move(r));
    }
    const size_t first_end = rdtsc();

    const size_t second_begin = rdtsc();
    for (size_t i = 0; i < N; ++i)
    {
        result2.emplace_back(
            t[i].fast_divide_by_10(i),
            t[i].fast_remainder_by_10(i)
        );
    }
    const size_t second_end = rdtsc();

    std::ostringstream eout;

    bool correct = true;
    for (size_t i = 0; i < result1.size(); ++i)
    {
        if (result1[i] != result2[i])
        {
            correct = false;

            eout << i << ":" <<
                "\n\t" << t[i] << " / " << dividers[i] <<
                "\n\t=" << result1[i].first << "\n\t=" << result2[i].first <<
                "\n\t%" << result1[i].second << "\n\t%" << result2[i].second << endl;
        }
    }

    if (!correct)
    {
        std::ofstream("division_errors.txt") << eout.str();
    }

    tout << "Simple division time: " << first_end - first_begin << endl;
    tout << "Fast division time: " << second_end - second_begin << endl;
    tout << "Is correct: " << correct << endl;

    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
}


void compare_long_and_simple_div()
{
    tout << begin_marker << "PLAIN DIV" << endl;
    const size_t begin = rdtsc();

    std::vector<LongArith> t;
    constexpr size_t N = 100000;
    LongArith M(N);
    for (LongArith i = 0; i < M; ++i)
    {
        LongArith tmp = rand() % 1000;
        for (size_t i = 0; i < 100;++i)
            tmp *= rand() % 1000;
        t.push_back(tmp);
    }

    std::vector<int> dividers;
    for (LongArith i = 0; i < M; ++i)
    {
        int v;
        do {
            v = rand();
        } while (v == 0);
        dividers.emplace_back(v);
    }

    for (auto v : dividers)
    {
        if (v == 0)
            throw "Unexpected";
    }

    tout << "Set generated in " << rdtsc() - begin << endl;

    std::vector<std::pair<LongArith, LongArith>> result1;
    std::vector<std::pair<LongArith, long>> result2;
    result1.reserve(N); result2.reserve(N);



    const size_t first_begin = rdtsc();
    for (size_t i = 0; i < N; ++i)
    {
        auto r = LongArith::fraction_and_remainder(t[i], LongArith(dividers[i]));
        result1.emplace_back(std::move(r));
    }
    const size_t first_end = rdtsc();

    const size_t second_begin = rdtsc();
    for (size_t i = 0; i < N; ++i)
    {
        auto r = LongArith::fraction_and_remainder(t[i], dividers[i]);
        result2.emplace_back(std::move(r));
    }
    const size_t second_end = rdtsc();

    std::ostringstream eout;

    bool correct = true;
    for (size_t i = 0; i < result1.size(); ++i)
    {
        if (result1[i].first != result2[i].first || result1[i].second.to_plain_int() != result2[i].second)
        {
            correct = false;

            eout << i << ":" <<
                "\n\t" << t[i] << " / " << dividers[i] <<
                "\n\t=" << result1[i].first << "\n\t=" << result2[i].first <<
                "\n\t%" << result1[i].second << "\n\t%" << result2[i].second << endl;
        }
    }

    if (!correct)
    {
        std::ofstream("division_errors.txt") << eout.str();
    }

    tout << "LongArith division time: " << first_end - first_begin << endl;
    tout << "Plain division time: " << second_end - second_begin << endl;
    tout << "Is correct: " << correct << endl;

    const size_t end = rdtsc();
    tout << end_marker << end - begin << endl;
}

int main()
{
    using namespace  std;

    const char old_label[] = "Old impl\n", new_label[] = "New impl\n";


    factorial_op_benchmark_plain<LongArith>();

    factorial_op_benchmark_long<LongArith>();

    // tout << old_label;
    // simple_operation_benchmark<LA_Old>();
    tout << new_label;
    simple_operation_benchmark<LongArith>();

    //tout << old_label;
    //vector_op_benchmark<LA_Old>();
    tout << new_label;
    vector_op_benchmark<LongArith>();


    fast_division_benchmark();

    compare_long_and_simple_div();


    out << tout.str();
    ofstream("tmp.txt") << garbage_marker << garbage.str();


    size_t very_begin = rdtsc();

    LongArith tt = 11;
    tt *= LongArith::DigitBase;
    tt *= LongArith::DigitBase;
    tt += 11;
    out << tt / 11 << endl;
    out << (tt - 11) / 11 << endl;

    //    LongArith::test();
    LongArith a(5);
    LongArith b(0);
    a += LongArith(100);
    out << a << "\n";
    out << b << "\n";
    b = -a;
    out << b << "\n";
    b = -(LongArith::from_string("15465342342342347489719841234234878") + a + b + LongArith(1000));
    out << b << "\n";
    b = -b;
    out << b << "\n";
    b -= a;
    out << b << "\n";
    b -= LongArith(a);
    out << b << "\n";
    b = -std::move(b);
    out << b << "\n";
    b = b;
    out << b << "\n";
    b += b;
    out << b << "\n";
    out << "Hello, World!" << std::endl;
    ++a;
    out << ++a << ++a;
    out << --a << --a << endl;

    out << (b = LongArith::from_string("+154654654879498415984984189491941987489719841")) << endl;
    out << a << endl;
    //cin >> a;
    out << a << endl;
    out << a - b << endl;
    out << b - a << endl;
    out << a + b << endl;
    a = LongArith::from_string("+154654654879498415984984189491941987489719841");
    b = LongArith::from_string("468789871987198719871919000000000");
    a *= b;
    out << "Check zero\n";
    a = 0;
    b = LongArith::from_string("468789871987198719871919");
    out << a*b << "\n";

    out << "Time test\n";
    out << "Increment test" << endl;
    LongArith check_count = 1000;
    LongArith iter(1);
    size_t time = rdtsc();
    for (LongArith checks = 1; checks <= check_count; ++checks)
    {
        iter *= checks;
    }
    out << rdtsc() - time << "\n";
    out << iter << "\n";
    a = 999999999L;
    out << "Iterations:\n";
    for (size_t i = 0; i < 5;++i)
    {
        out << a << endl;
        ++a;
    }
    for (size_t i = 0; i < 10;++i)
    {
        out << a << endl;
        --a;
    }
    LongArith t = 5;

    for (size_t i = 0; i < 10;++i)
    {
        out << t << endl;
        --t;
    }
    out << ++t << "\n";


    LongArith c = a*b;
    //    c = LongArith::from_string(LongArith::gen2(50, 12))
    //       *LongArith::from_string(LongArith::gen2(40, 7));
    c = c * 5;
    c *= 5;
    a = 1;
    time = rdtsc();
    for (LongArith checks = 0; checks < LongArith(1000); ++checks)
    {
        a += a;
    }
    out << rdtsc() - time << "\n";
    out << a << "\n";
    out << "Comparing\n";
    LongArith ten(10);
    for (LongArith i = -ten; i < ten; ++i)
        for (LongArith j = -ten; j < ten; ++j)
            out << i << ">" << j << "==" << (i > j) << "\n";
    int pt = 5;
    out << pt;

    out << LongArith(1234567890LL) << " - \n" <<
        LongArith(1111111111LL) << " = \n" <<
        LongArith(1234567890LL) - LongArith(1111111111LL) << "\n";

    LongArith dividable = LongArith(1111111111LL);
    out << dividable << " * " << dividable << " = ";
    LongArith divider = 1000LL * 1000 * 1000;
    divider = dividable;
    dividable *= dividable;
    out << dividable << "\n";
    out << "======================\n" <<
        "Division begins...\n";
    LongArith fraction, rem;
    out << dividable << " / " << divider << "\n";
    std::tie(fraction, rem) = LongArith::fraction_and_remainder(dividable, divider);
    out << "\t= " << fraction << " ; " << rem << "\n";
    out << LongArith::from_string("654897491581065498498719467981567498") / LongArith::from_string("49879871") << " ; " <<
        LongArith::from_string("654897491581065498498719467981567498") % LongArith::from_string("49879871") << "\n";
    LongArith fr = LongArith::from_string("654897491581065498498719467981567498");
    fr /= LongArith::from_string("49879871");
    {
        std::vector<LongArith> vect; vect.reserve(1000);
        out << "CHECK VECTOR TIME: ";
        const size_t vect_c = rdtsc();
        for (size_t i = 0; i < 1000; ++i)
        {
            vect.push_back(LongArith::from_string("468787974191"));
        }
        out << rdtsc() - vect_c << " ";
        for (auto& v : vect)
        {
            v *= v;
        }
        out << rdtsc() - vect_c << "\n";
    }
    {
        std::list<LongArith> vect;
        out << "CHECK LIST TIME: ";
        const size_t vect_c = rdtsc();
        for (size_t i = 0; i < 1000; ++i)
        {
            vect.push_back(LongArith::from_string("468787974191"));
        }
        out << rdtsc() - vect_c << " ";
        for (auto& v : vect)
        {
            v *= v;
        }
        out << rdtsc() - vect_c << "\n";
    }
    out << "FINISHED\n";
    out << sizeof(LongArith) << "\n";
    out << "Total ticks " << rdtsc() - very_begin;
    // system("pause");
    return 0;
}

