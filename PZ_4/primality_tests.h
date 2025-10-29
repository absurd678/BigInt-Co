#ifndef PRIMALITY_TESTS_H
#define PRIMALITY_TESTS_H

#include "bigint.h"
#include <random>
#include <vector>
#include <map>

class PrimalityTests {
private:
    static std::vector<BigInt> getWitnesses(const BigInt& n, int count);
    static bool isWitness(const BigInt& a, const BigInt& n);
    static bool isStrongLucasWitness(const BigInt& n, const BigInt& d, int p, int q);
    static BigInt findD(const BigInt& n);
    
public:
    // 1. Тест Миллера-Рабина
    static bool millerRabinTest(const BigInt& n, int iterations = 10);
    static void millerRabinStatistics(const BigInt& n, int tests_count = 100);
    
    // 2. Тест Люка на сильную псевдопростоту
    static bool lucasStrongTest(const BigInt& n, int iterations = 10);
    static void lucasStrongStatistics(const BigInt& n, int tests_count = 100);
    
    // 3. Тест Бейли-Померанца-Селфриджа-Уогстаффа (BPSW)
    static bool bpswTest(const BigInt& n, int iterations = 10);
    static void bpswStatistics(const BigInt& n, int tests_count = 100);
    
    // 4. Сравнение всех тестов
    static void compareAllTests(const BigInt& n, int tests_count = 100);
    
    // Вспомогательные методы
    static std::vector<BigInt> generateTestNumbers(int min_digits, int max_digits, int count);
    static void runComprehensiveAnalysis();
};

#endif