#ifndef DETERMINISTIC_PRIMALITY_H
#define DETERMINISTIC_PRIMALITY_H

#include "bigint.h"
#include "polynomial.h"
#include <vector>
#include <cmath>
#include <chrono>

class DeterministicPrimality {
private:
    // Вспомогательные методы для AKS
    static bool isPerfectPower(const BigInt& n);
    static BigInt findSmallestR(const BigInt& n);
    static BigInt multiplicativeOrder(const BigInt& a, const BigInt& n);
    static BigInt eulerTotient(const BigInt& n);
    static bool checkPolynomialAKS(const BigInt& n, const BigInt& r);
    
    // Вспомогательные методы для Миллера
    static void factorOutTwos(BigInt n_minus_one, BigInt& d, int& s);
    
    // Утилиты
    static BigInt log2Approx(const BigInt& n);
    static BigInt sqrt(const BigInt& n);

public:
    /**
     * Детерминированный тест Агравала-Каяла-Саксены (AKS) - ПОЛНАЯ РЕАЛИЗАЦИЯ
     */
    static bool aksTest(const BigInt& n);
    
    /**
     * Детерминированный тест Миллера - ПОЛНАЯ РЕАЛИЗАЦИЯ
     */
    static bool millerTest(const BigInt& n);
    
    /**
     * Упрощенная версия AKS для демонстрации
     */
    static bool aksTestSimple(const BigInt& n);
    
    /**
     * Измерение скорости выполнения теста
     */
    static std::pair<bool, double> measureTestSpeed(bool (*testFunc)(const BigInt&), 
                                                   const BigInt& n, 
                                                   const std::string& testName);
    
    /**
     * Сравнение двух тестов на одном числе
     */
    static void compareTests(const BigInt& n);
    
    /**
     * Комплексное тестирование на известных простых числах
     */
    static void runComprehensiveBenchmark();
};

#endif