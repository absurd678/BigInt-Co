// gcc -O3 -std=c11 -Wno-alloc-size-larger-than -o lab5 main.c BigNumber.c ECPPLite.c -lm
// ./lab5
#include "BigNumber.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    BigNum n1 = MakeBigNum("162259276829213363391578010288127");        // 2^107 - 1 (простое)
    BigNum n2 = MakeBigNum("170141183460469231731687303715884105727");  // 2^127 - 1 (простое)
    BigNum n3 = MakeBigNum("536870911");                                // 2^29 - 1 (составное)
    BigNum n4 = MakeBigNum("147573952589676412927");                    // 2^67 - 1 (составное)

    // BigNum n1 = MakeBigNum("2147483647");                // 2^31 - 1 (простое)
    // BigNum n2 = MakeBigNum("147573952589676412927");     // 2^67 - 1 (составное)
    // BigNum n3 = MakeBigNum("536870911");                 // 2^29 - 1 (составное)
    // BigNum n4 = MakeBigNum("2305843009213693951");       // 2^61 - 1 (простое)

    ECPP_RunAndPrint(n1);
    ECPP_RunAndPrint(n2);
    ECPP_RunAndPrint(n3);
    ECPP_RunAndPrint(n4);

    return 0;
}
