#ifndef BIGINT_H
#define BIGINT_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <cmath>
#include <map>

class BigInt {
private:
    std::vector<int> digits;
    bool isNegative;

    void removeLeadingZeros();
    int compareAbsolute(const BigInt& other) const;
    BigInt addAbsolute(const BigInt& other) const;
    BigInt subtractAbsolute(const BigInt& other) const;
    BigInt multiplyByDigit(int digit) const;
    BigInt divideByDigit(int digit) const;

    // Вспомогательные методы для факторизации
    static BigInt pollardRho(const BigInt& n, int maxIterations = 10000);
    static BigInt pollardPMinus1(const BigInt& n, int maxB = 10000);

public:
    // ==================== КОНСТРУКТОРЫ ====================
    BigInt();
    BigInt(const std::string& str);
    BigInt(long long num);
    BigInt(const BigInt& other);
    BigInt(int numDigits, std::mt19937& gen);

    // ==================== ОПЕРАТОРЫ ПРИСВАИВАНИЯ ====================
    BigInt& operator=(const BigInt& other);
    BigInt& operator=(long long num);

    // ==================== АРИФМЕТИЧЕСКИЕ ОПЕРАТОРЫ ====================
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
    BigInt operator^(const BigInt& exponent) const;

    // ==================== ОПЕРАТОРЫ СРАВНЕНИЯ ====================
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    // ==================== УНАРНЫЕ ОПЕРАТОРЫ ====================
    BigInt operator-() const;
    BigInt operator+() const;

    // ==================== МЕТОДЫ ВВОДА/ВЫВОДА ====================
    friend std::ostream& operator<<(std::ostream& os, const BigInt& num);
    friend std::istream& operator>>(std::istream& is, BigInt& num);

    // ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================
    std::string toString() const;
    bool isZero() const;
    BigInt abs() const;
    size_t getDigitCount() const { return digits.size(); }
    
    int getDigitAt(size_t index) const {
        if (index < digits.size()) {
            return digits[index];
        }
        return 0;
    }
    
    int getLastDigit() const {
        return digits.empty() ? 0 : digits[0];
    }
    
    const std::vector<int>& getDigits() const {
        return digits;
    }
    
    bool isEven() const {
        return digits.empty() ? true : (digits[0] % 2 == 0);
    }
    
    bool isOdd() const {
        return !isEven();
    }
    
    int sign() const {
        if (isZero()) return 0;
        return isNegative ? -1 : 1;
    }

    // ==================== СТАТИЧЕСКИЕ МАТЕМАТИЧЕСКИЕ ФУНКЦИИ ====================
    static BigInt gcd(BigInt a, BigInt b);
    static BigInt lcm(const BigInt& a, const BigInt& b);
    static BigInt generateRandomPrime(int numDigits, std::mt19937& gen);
    bool isPrime(int iterations = 10) const;

    // ==================== МЕТОДЫ ПРОВЕРКИ ПРОСТОТЫ ====================
    static bool isPrimeStandard(const BigInt& n);
    static bool isPrimeEratosthenes(const BigInt& n, int limit = 1000000);
    static bool isPrimeAtkin(const BigInt& n, int limit = 1000000);
    static bool lucasLehmerTest(int p);
    static BigInt sqrt(const BigInt& n);
    static BigInt modPow(const BigInt& base, const BigInt& exponent, const BigInt& mod);
    static BigInt log(const BigInt& n);

    // ==================== ECPP МЕТОДЫ ====================
    bool isPrimeECPP(int maxAttempts = 100) const;
    
    // Вспомогательные методы для ECPP
    static bool findEllipticCurve(const BigInt& n, BigInt& a, BigInt& b, BigInt& x, BigInt& y, int maxAttempts);
    static BigInt countPointsOnCurve(const BigInt& a, const BigInt& b, const BigInt& p);
    static bool verifyPrimalityConditions(const BigInt& n, const BigInt& m, const BigInt& q);
    static BigInt randomBigInt(const BigInt& max, std::mt19937& gen);
    
    // Методы факторизации (сделаем публичными для тестирования)
    static std::vector<BigInt> factorize(const BigInt& n, int maxAttempts = 10);
};

#endif