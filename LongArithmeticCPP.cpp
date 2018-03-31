// LongArithmeticCPP.cpp : Defines the entry point for the console application.
//
#include "LongArith.h"
#include <iostream>
#include <sstream>


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



int main()
{
    using namespace  std;
//    LongArith::test();
    LongArith a(5);
    LongArith b(0);
    a += LongArith(100);
    std::cout << a << "\n";
    std::cout << b << "\n";
    b = -a;
    std::cout << b << "\n";
    b = -(LongArith::fromString("15465342342342347489719841234234878") + a + b + LongArith(1000));
    b = -b;
    b -= a;
    b -= LongArith(a);
    b = -std::move(b);
    b = b;
    std::cout << b << "\n";
    b += b;
    std::cout << b << "\n";
    std::cout << "Hello, World!" << std::endl;
    ++a;
    cout << ++a << ++a;
    cout << --a << --a << endl;
    int n;

    cout << (b = LongArith::fromString("+154654654879498415984984189491941987489719841")) << endl;
    cout << a << endl;
    //cin >> a;
    cout << a << endl;
    cout << a - b << endl;
    cout << b - a << endl;
    cout << a + b << endl;
    a = LongArith::fromString("+154654654879498415984984189491941987489719841");
    b = LongArith::fromString("468789871987198719871919000000000");
    a *= b;
    cout << "Check zero\n";
    a = 0;
    b = LongArith::fromString("468789871987198719871919");
    cout << a*b << "\n";

    cout << "Time test\n";
    cout << "Increment test" << endl;
    LongArith check_count = 1000;
    LongArith iter(1);
    size_t time = rdtsc();
    for (LongArith checks = 1; checks <= check_count; ++checks)
    {
        iter *= checks;
    }
    cout << rdtsc() - time << "\n";
    cout << iter << "\n";
    a = 999999999L;
    std::cout << "Iterations:\n";
    for (size_t i = 0; i < 5;++i)
    {
        std::cout << a << endl;
        ++a;
    }
    for (size_t i = 0; i < 10;++i)
    {
        std::cout << a << endl;
        --a;
    }
    LongArith t = 5;

    for (size_t i = 0; i < 10;++i)
    {
        std::cout << t << endl;
        --t;
    }
    std::cout << ++t << "\n";


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
    cout << rdtsc() - time << "\n";
    cout << a << "\n";
    cout << "Comparing\n";
    LongArith ten(10);
    for (LongArith i = -ten; i < ten; ++i)
        for (LongArith j = -ten; j < ten; ++j)
            cout << i << ">" << j << "==" << (i > j) << "\n";
    int pt = 5;
    cout << pt;
    
    cout << LongArith(1234567890LL) << " - \n" <<
        LongArith(1111111111LL) << " = \n" <<
        LongArith(1234567890LL) - LongArith(1111111111LL) << "\n";

    LongArith dividable = LongArith(1111111111LL);
    cout << dividable << " * " << dividable << " = ";
    LongArith divider = 1000LL * 1000 * 1000;
    divider = dividable;
    dividable *= dividable;
    cout << dividable << "\n";
    cout << "======================\n"<<
        "Division begins...\n";
    LongArith fraction, rem;
    cout << dividable << " / " << divider<<"\n";
    std::tie(fraction, rem) = LongArith::FractionAndRemainder(dividable, divider);
    cout << "\t= " << fraction << " ; " << rem<<"\n";
    cout << LongArith::fromString("654897491581065498498719467981567498") / LongArith::fromString("49879871") << " ; " <<
        LongArith::fromString("654897491581065498498719467981567498") % LongArith::fromString("49879871") <<"\n";
    LongArith fr = LongArith::fromString("654897491581065498498719467981567498");
    fr /=  LongArith::fromString("49879871");
    cout << "FINISHED\n";
    system("pause");
    return 0;
}

