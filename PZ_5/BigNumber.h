#ifndef BIGNUMBER_H
#define BIGNUMBER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define BASE 10

typedef struct {
    int *digits;
    int length;
    int sign; // 1 — положительное, -1 — отрицательное
} BigNum;

BigNum CreateFromInt(long long value);
BigNum MakeBigNum(const char *str);
void   FreeBigNum(BigNum *n);
int    Compare(BigNum a, BigNum b);
bool   IsZero(BigNum n);
bool   IsEven(BigNum n);
BigNum Add(BigNum a, BigNum b);
BigNum Sub(BigNum a, BigNum b);
BigNum Mul(BigNum a, BigNum b);
BigNum Mod(BigNum a, BigNum m);
BigNum DivByTwo(BigNum n);
void   get_number_preview(BigNum n, char *buf, int bufsize);

int  MillerTest(BigNum n);
int  ECPP_IsPrime(BigNum n);
void ECPP_RunAndPrint(BigNum n);
BigNum CopyBigNum(BigNum src);

#endif
