/*
g++ -std=c++11 -O2 -o primality_test bigint.cpp main.cpp
./primality_test
*/

#include "bigint.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;
using namespace std::chrono;

template<typename Func>
long long measureTime(Func func, bool& result) {
    auto start = high_resolution_clock::now();
    result = func();
    auto end = high_resolution_clock::now();
    return duration_cast<milliseconds>(end - start).count();
}

void testOptimizedECPP() {
    cout << "=== ОПТИМИЗИРОВАННАЯ ECPP ===" << endl << endl;
    
    vector<pair<string, string>> testCases = {
        {"1000003", "Малое простое"},
        {"1000000007", "Простое < 2^32"},
        {"2147483647", "2^31-1 (Мерсенна)"},
        {"999999000001", "Большое простое (>2^64)"}
    };
    
    for (const auto& testCase : testCases) {
        string numStr = testCase.first;
        string description = testCase.second;
        BigInt num(numStr);
        cout << "Тест: " << description << endl;
        cout << "Число: " << num << " (" << num.toString().length() << " цифр)" << endl;
        
        bool result;
        
        // Оптимизированный ECPP
        long long ecppTime = measureTime([&]() {
            return num.isPrimeECPP(20);
        }, result);
        
        cout << "ECPP:        " << (result ? "простое" : "составное") 
             << " (время: " << setw(6) << ecppTime << " мс)" << endl;
        
        // Стандартный метод для сравнения
        long long standardTime = measureTime([&]() {
            return BigInt::isPrimeStandard(num);
        }, result);
        
        cout << "Стандартный: " << (result ? "простое" : "составное")
             << " (время: " << setw(6) << standardTime << " мс)" << endl;
        
        if (ecppTime > 0 && standardTime > 0) {
            cout << "Ускорение:   " << fixed << setprecision(1) 
                 << (double)standardTime / max(ecppTime, 1LL) << "x" << endl;
        }
        cout << string(50, '-') << endl;
    }
}

void testMersenneWithECPP() {
    cout << "=== ЧИСЛА МЕРСЕННА С ECPP ===" << endl << endl;
    
    vector<int> exponents = {2, 3, 5, 7, 13, 17, 19, 31};
    
    for (int exp : exponents) {
        cout << "2^" << exp << "-1:" << endl;
        
        bool result;
        
        // Тест Люка-Лемера
        long long llTime = measureTime([&]() {
            return BigInt::lucasLehmerTest(exp);
        }, result);
        cout << "Люка-Лемера: " << (result ? "простое" : "составное")
             << " (" << llTime << " мс)" << endl;
        
        // ECPP
        if (exp <= 19) {
            BigInt mersenne = (BigInt(2) ^ BigInt(exp)) - BigInt(1);
            long long ecppTime = measureTime([&]() {
                return mersenne.isPrimeECPP(10);
            }, result);
            cout << "ECPP:        " << (result ? "простое" : "составное")
                 << " (" << ecppTime << " мс)" << endl;
        }
        cout << endl;
    }
}

void testFactorization() {
    cout << "=== ТЕСТ ФАКТОРИЗАЦИИ ===" << endl << endl;
    
    vector<string> numbersToFactor = {
        "1001", "10001", "100001", "999999"
    };
    
    for (const auto& numStr : numbersToFactor) {
        BigInt num(numStr);
        cout << "Факторизация " << num << ":" << endl;
        
        auto start = high_resolution_clock::now();
        auto factors = BigInt::factorize(num);
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "Множители: ";
        for (size_t i = 0; i < factors.size(); ++i) {
            cout << factors[i];
            if (i < factors.size() - 1) cout << " × ";
        }
        cout << " (" << time << " мс)" << endl << endl;
    }
}

void testRange() {
    cout << "=== ТЕСТ ДИАПАЗОНА 1000...100000000 ===" << endl << endl;
    
    vector<string> rangeNumbers = {
        "1009", "7919", "10007", "104729", "1000003", "10000019", "99999989"
    };
    
    for (const auto& numStr : rangeNumbers) {
        BigInt num(numStr);
        cout << "Число: " << numStr << endl;
        
        bool result;
        long long ecppTime = measureTime([&]() {
            return num.isPrimeECPP(10);
        }, result);
        
        cout << "ECPP: " << (result ? "простое" : "составное")
             << " (" << ecppTime << " мс)" << endl;
        
        long long standardTime = measureTime([&]() {
            return BigInt::isPrimeStandard(num);
        }, result);
        
        cout << "Стандарт: " << (result ? "простое" : "составное")
             << " (" << standardTime << " мс)" << endl;
        cout << "------------------------" << endl;
    }
}

int main() {
    cout << "ПРАКТИЧЕСКАЯ РАБОТА №5 - ОПТИМИЗИРОВАННАЯ ECPP" << endl;
    cout << "Алгоритм Аткина-Морейна на эллиптических кривых" << endl;
    cout << "=============================================" << endl;
    
    testRange();
    testOptimizedECPP();
    testMersenneWithECPP();
    testFactorization();
    
    cout << "Проверка чисел из задания:" << endl;
    cout << "- 2^82589933-1: используйте GIMPS для проверки" << endl;
    cout << "- 2^136279841-1: требует распределенных вычислений" << endl;
    cout << "ECPP оптимизирован для чисел до 100 цифр" << endl;
    
    return 0;
}