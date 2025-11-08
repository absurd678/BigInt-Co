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

public:
    // Конструкторы
    BigInt();
    BigInt(const std::string& str);
    BigInt(long long num);
    BigInt(const BigInt& other);
    BigInt(int numDigits, std::mt19937& gen);

    // Операторы присваивания
    BigInt& operator=(const BigInt& other);
    BigInt& operator=(long long num);

    // Арифметические операторы
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
    BigInt operator^(const BigInt& exponent) const;

    // Операторы сравнения
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    // Унарные операторы
    BigInt operator-() const;
    BigInt operator+() const;

    // Методы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const BigInt& num);
    friend std::istream& operator>>(std::istream& is, BigInt& num);

    // Вспомогательные методы
    std::string toString() const;
    bool isZero() const;
    BigInt abs() const;
    size_t getDigitCount() const { return digits.size(); }
    
    // Математические функции
    static BigInt gcd(BigInt a, BigInt b);
    static BigInt lcm(const BigInt& a, const BigInt& b);
    
    // Методы проверки простоты для практической работы №2
    static bool isPrimeStandard(const BigInt& n);
    static bool isPrimeEratosthenes(const BigInt& n, int limit = 1000000);
    static bool isPrimeAtkin(const BigInt& n, int limit = 1000000);
    static bool lucasLehmerTest(int p);
    
    // Вспомогательные методы для проверки простоты
    static BigInt sqrt(const BigInt& n);
    static BigInt modPow(const BigInt& base, const BigInt& exponent, const BigInt& mod);
    
    // Генерация простых чисел
    static BigInt generateRandomPrime(int numDigits, std::mt19937& gen);
    bool isPrime(int iterations = 10) const;
};

#endif