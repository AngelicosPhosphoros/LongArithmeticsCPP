// LongArithmeticCPP.cpp : Defines the entry point for the console application.
//
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

//#include <fstream>
std::ostream& out = std::cout;//std::ofstream("out1.txt");

int main()
{
    using namespace  std;
    size_t very_begin = rdtsc();
    //    LongArith::test();
    LongArith a(5);
    LongArith b(0);
    a += LongArith(100);
    out << a << "\n";
    out << b << "\n";
    b = -a;
    out << b << "\n";
    b = -(LongArith::fromString("15465342342342347489719841234234878") + a + b + LongArith(1000));
    b = -b;
    b -= a;
    b -= LongArith(a);
    b = -std::move(b);
    b = b;
    out << b << "\n";
    b += b;
    out << b << "\n";
    out << "Hello, World!" << std::endl;
    ++a;
    out << ++a << ++a;
    out << --a << --a << endl;
    int n;

    out << (b = LongArith::fromString("+154654654879498415984984189491941987489719841")) << endl;
    out << a << endl;
    //cin >> a;
    out << a << endl;
    out << a - b << endl;
    out << b - a << endl;
    out << a + b << endl;
    a = LongArith::fromString("+154654654879498415984984189491941987489719841");
    b = LongArith::fromString("468789871987198719871919000000000");
    a *= b;
    out << "Check zero\n";
    a = 0;
    b = LongArith::fromString("468789871987198719871919");
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
    //    c = LongArith::fromString(LongArith::gen2(50, 12))
    //       *LongArith::fromString(LongArith::gen2(40, 7));
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
    std::tie(fraction, rem) = LongArith::FractionAndRemainder(dividable, divider);
    out << "\t= " << fraction << " ; " << rem << "\n";
    out << LongArith::fromString("654897491581065498498719467981567498") / LongArith::fromString("49879871") << " ; " <<
        LongArith::fromString("654897491581065498498719467981567498") % LongArith::fromString("49879871") << "\n";
    LongArith fr = LongArith::fromString("654897491581065498498719467981567498");
    fr /= LongArith::fromString("49879871");
    {
        std::vector<LongArith> vect; vect.reserve(1000);
        out << "CHECK VECTOR TIME: ";
        const size_t vect_c = rdtsc();
        for (size_t i = 0; i < 1000; ++i)
        {
            vect.push_back(LongArith::fromString("468787974191"));
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
            vect.push_back(LongArith::fromString("468787974191"));
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
    system("pause");
    return 0;
}

