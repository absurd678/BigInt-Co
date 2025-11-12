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

void testBasicPrimes() {
    cout << "=== БАЗОВАЯ ПРОВЕРКА ПРОСТЫХ ЧИСЕЛ ===" << endl << endl;
    
    vector<pair<string, string>> testCases = {
        {"1009", "Малое простое"},
        {"10007", "Простое среднего размера"},
        {"104729", "10000-е простое"},
        {"1000003", "Простое > 10^6"}
    };
    
    for (const auto& testCase : testCases) {
        string numStr = testCase.first;
        string description = testCase.second;
        BigInt num(numStr);
        
        cout << "Тест: " << description << endl;
        cout << "Число: " << num << endl;
        
        bool result;
        
        // ECPP метод
        long long ecppTime = measureTime([&]() {
            return num.isPrimeECPP(10);
        }, result);
        
        cout << "ECPP:        " << (result ? "простое" : "составное") 
             << " (время: " << ecppTime << " мс)" << endl;
        
        // Стандартный метод
        long long standardTime = measureTime([&]() {
            return BigInt::isPrimeStandard(num);
        }, result);
        
        cout << "Стандартный: " << (result ? "простое" : "составное")
             << " (время: " << standardTime << " мс)" << endl;
        
        cout << string(40, '-') << endl;
    }
}

void testCompositeNumbers() {
    cout << "=== ПРОВЕРКА СОСТАВНЫХ ЧИСЕЛ ===" << endl << endl;
    
    vector<string> composites = {
        "1001", "10001", "100001", "999999"
    };
    
    for (const auto& numStr : composites) {
        BigInt num(numStr);
        cout << "Число: " << num << endl;
        
        bool result;
        long long time = measureTime([&]() {
            return num.isPrimeECPP(5);
        }, result);
        
        cout << "Результат: " << (result ? "простое" : "составное") 
             << " (время: " << time << " мс)" << endl;
        
        if (!result) {
            auto factors = BigInt::factorize(num, 3);
            cout << "Факторы: ";
            for (size_t i = 0; i < factors.size(); ++i) {
                cout << factors[i];
                if (i < factors.size() - 1) cout << " × ";
            }
            cout << endl;
        }
        cout << endl;
    }
}

void testMersennePrimes() {
    cout << "=== ЧИСЛА МЕРСЕННА ===" << endl << endl;
    
    vector<int> exponents = {2, 3, 5, 7, 13};
    
    for (int exp : exponents) {
        cout << "2^" << exp << "-1:" << endl;
        
        bool result;
        long long llTime = measureTime([&]() {
            return BigInt::lucasLehmerTest(exp);
        }, result);
        
        cout << "Люка-Лемера: " << (result ? "простое" : "составное")
             << " (" << llTime << " мс)" << endl;
        
        if (exp <= 13) {
            BigInt mersenne = (BigInt(2) ^ BigInt(exp)) - BigInt(1);
            long long ecppTime = measureTime([&]() {
                return mersenne.isPrimeECPP(5);
            }, result);
            cout << "ECPP:        " << (result ? "простое" : "составное")
                 << " (" << ecppTime << " мс)" << endl;
        }
        cout << endl;
    }
}

void testPerformance() {
    cout << "=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << endl << endl;
    
    vector<string> testNumbers = {
        "1009", "7919", "10007", "104729", "1000003"
    };
    
    cout << setw(10) << "Число" << setw(8) << "Цифр" 
         << setw(10) << "ECPP(мс)" << setw(12) << "Станд(мс)" << endl;
    cout << string(40, '-') << endl;
    
    for (const auto& numStr : testNumbers) {
        BigInt num(numStr);
        bool result1, result2;
        
        long long ecppTime = measureTime([&]() { return num.isPrimeECPP(5); }, result1);
        long long standardTime = measureTime([&]() { return BigInt::isPrimeStandard(num); }, result2);
        
        cout << setw(10) << numStr << setw(8) << numStr.length()
             << setw(10) << ecppTime << setw(12) << standardTime << endl;
    }
}

int main() {
    cout << "ПРАКТИЧЕСКАЯ РАБОТА №5 - ОПТИМИЗИРОВАННАЯ ECPP" << endl;
    cout << "=============================================" << endl << endl;
    
    testBasicPrimes();
    testCompositeNumbers();
    testMersennePrimes();
    testPerformance();
    
    cout << "КОМПИЛЯЦИЯ И ЗАПУСК:" << endl;
    cout << "g++ -std=c++11 -O2 -o primality_test bigint.cpp main.cpp" << endl;
    cout << "./primality_test" << endl;
    
    return 0;
}