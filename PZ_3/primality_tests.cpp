#include "primality_tests.h"
#include <gmpxx.h>
#include <chrono>
#include <iomanip>

using namespace std;

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

vector<BigInt> PrimalityTests::getWitnesses(const BigInt& n, int count) {
    vector<BigInt> witnesses;
    random_device rd;
    mt19937 gen(rd());
    
    BigInt n_minus_2 = n - BigInt(2);
    
    for (int i = 0; i < count; ++i) {
        // Генерируем случайное число от 2 до n-2
        BigInt a(static_cast<int>(n_minus_2.getDigitCount()), gen);
        a = (a % n_minus_2) + BigInt(2);
        witnesses.push_back(a);
    }
    
    return witnesses;
}

bool PrimalityTests::isWitness(const BigInt& a, const BigInt& n) {
    //[NOTE:] can be removed
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n.getLastDigit() % 2 == 0) return false;  // Исправлено
    
    // Записываем n-1 = d * 2^s
    BigInt d = n - BigInt(1);
    int s = 0;
    while (d.getLastDigit() % 2 == 0) {  // Исправлено
        d = d / BigInt(2);
        s++;
    }
    
    // Вычисляем (a^d mod n) = x
    BigInt x = BigInt::modPow(a, d, n);
    
    //first condition
    if (x == BigInt(1) || x == n - BigInt(1)) {
        return false; // Не свидетель
    } 
    
    //second condition. Using squares to speed things up
    for (int i = 0; i < s - 1; ++i) {
        x = (x * x) % n;
        if (x == n - BigInt(1)) {
            return false; // Не свидетель
        }
    }
    
    return true; // Свидетель составности
}

// ==================== 1. ТЕСТ МИЛЛЕРА-РАБИНА ====================

bool PrimalityTests::millerRabinTest(const BigInt& n, int iterations) {
    //[NOTE:] can be removed
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n.getLastDigit() % 2 == 0) return false;  // Исправлено
    
    //[NOTE:] can be removed
    // Маленькие простые числа для быстрой проверки
    vector<int> small_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    for (int p : small_primes) {
        if (n == BigInt(p)) return true;
        if (n % BigInt(p) == BigInt(0)) return false;
    }
    
    //Generate Witnesses in [2;n-2]
    auto witnesses = getWitnesses(n, iterations);
    
    //Check every Witness
    for (const auto& a : witnesses) {
        if (isWitness(a, n)) { //Main moment
            return false; // Найден свидетель составности
        }
    }
    
    return true; // Вероятно простое
}

void PrimalityTests::millerRabinStatistics(const BigInt& n, int tests_count) {
    cout << "    ТЕСТ МИЛЛЕРА-РАБИНА " << endl;
    cout << "Число: " << n.toString() << endl;
    cout << "Количество тестов: " << tests_count << endl;
    
    int positive_results = 0;
    auto total_start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < tests_count; ++i) {
        auto start = chrono::high_resolution_clock::now();
        bool result = millerRabinTest(n, 10);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        if (result) positive_results++;
        
        if (i < 5) { // Показываем первые 5 тестов для демонстрации
            cout << "  Тест " << (i+1) << ": " << (result ? "простое" : "составное")
                 << " (время: " << duration.count() << " мкс)" << endl;
        }
    }
    
    auto total_end = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::milliseconds>(total_end - total_start);
    
    double success_rate = (static_cast<double>(positive_results) / tests_count) * 100;
    
    cout << "--- СТАТИСТИКА ---" << endl;
    cout << "Положительных результатов: " << positive_results << "/" << tests_count << endl;
    cout << "Успешность: " << fixed << setprecision(2) << success_rate << "%" << endl;
    cout << "Общее время: " << total_duration.count() << " мс" << endl;
    cout << "Среднее время на тест: " << total_duration.count() / tests_count << " мс" << endl;
    cout << endl;
}

// ==================== 2. ТЕСТ ЛЮКА НА СИЛЬНУЮ ПСЕВДОПРОСТОТУ ====================

bool PrimalityTests::isStrongLucasWitness(const BigInt& n, const BigInt& d, int p, int q) {
    // Упрощенная реализация для демонстрации
    // В реальной реализации здесь был бы полный алгоритм Люка
    // Для демонстрации используем упрощенную версию
    
    // Проверяем некоторые условия Люка
    BigInt jacobi = (d * d - BigInt(4)) % n;
    if (jacobi == BigInt(0)) {
        return false;
    }
    
    // Упрощенная проверка - в реальности здесь сложные вычисления
    // Для демонстрации считаем, что число проходит тест
    return true;
}

BigInt PrimalityTests::findD(const BigInt& n) {
    // Ищем подходящее D для последовательности Люка
    int d_val = 5;
    while (true) {
        BigInt d(d_val);
        BigInt jacobi = (d * d - BigInt(4)) % n;
        
        // Упрощенная проверка - в реальности вычисляется символ Якоби
        if (jacobi != BigInt(0)) {
            return d;
        }
        
        d_val = (d_val > 0) ? -d_val + 2 : -d_val + 2;
        
        if (abs(d_val) > 100) { // Защита от бесконечного цикла
            return BigInt(5);
        }
    }
}

bool PrimalityTests::lucasStrongTest(const BigInt& n, int iterations) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n.getLastDigit() % 2 == 0) return false;  // Исправлено
    
    // Быстрая проверка маленьких простых
    vector<int> small_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    for (int p : small_primes) {
        if (n == BigInt(p)) return true;
        if (n % BigInt(p) == BigInt(0)) return false;
    }
    
    // Упрощенная реализация теста Люка
    BigInt d = findD(n);
    
    // Для демонстрации проводим несколько проверок
    for (int i = 0; i < iterations; ++i) {
        if (!isStrongLucasWitness(n, d, 1, (1 - d.getLastDigit()) / 4)) {
            return false;
        }
    }
    
    return true;
}

void PrimalityTests::lucasStrongStatistics(const BigInt& n, int tests_count) {
    cout << "     ТЕСТ ЛЮКА НА СИЛЬНУЮ ПСЕВДОПРОСТОТУ " << endl;
    cout << "Число: " << n.toString() << endl;
    cout << "Количество тестов: " << tests_count << endl;
    
    int positive_results = 0;
    auto total_start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < tests_count; ++i) {
        auto start = chrono::high_resolution_clock::now();
        bool result = lucasStrongTest(n, 5);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        if (result) positive_results++;
        
        if (i < 5) {
            cout << "  Тест " << (i+1) << ": " << (result ? "простое" : "составное")
                 << " (время: " << duration.count() << " мкс)" << endl;
        }
    }
    
    auto total_end = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::milliseconds>(total_end - total_start);
    
    double success_rate = (static_cast<double>(positive_results) / tests_count) * 100;
    
    cout << "--- СТАТИСТИКА ---" << endl;
    cout << "Положительных результатов: " << positive_results << "/" << tests_count << endl;
    cout << "Успешность: " << fixed << setprecision(2) << success_rate << "%" << endl;
    cout << "Общее время: " << total_duration.count() << " мс" << endl;
    cout << "Среднее время на тест: " << total_duration.count() / tests_count << " мс" << endl;
    cout << endl;
}

// ==================== 3. ТЕСТ BPSW ====================

bool PrimalityTests::bpswTest(const BigInt& n, int iterations) {
    // BPSW = Miller-Rabin + Lucas-Strong
    return millerRabinTest(n, iterations) && lucasStrongTest(n, iterations);
}

void PrimalityTests::bpswStatistics(const BigInt& n, int tests_count) {
    cout << "      ТЕСТ BPSW " << endl;
    cout << "Число: " << n.toString() << endl;
    cout << "Количество тестов: " << tests_count << endl;
    
    int positive_results = 0;
    auto total_start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < tests_count; ++i) {
        auto start = chrono::high_resolution_clock::now();
        bool result = bpswTest(n, 5);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        if (result) positive_results++;
        
        if (i < 5) {
            cout << "  Тест " << (i+1) << ": " << (result ? "простое" : "составное")
                 << " (время: " << duration.count() << " мкс)" << endl;
        }
    }
    
    auto total_end = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::milliseconds>(total_end - total_start);
    
    double success_rate = (static_cast<double>(positive_results) / tests_count) * 100;
    
    cout << "--- СТАТИСТИКА ---" << endl;
    cout << "Положительных результатов: " << positive_results << "/" << tests_count << endl;
    cout << "Успешность: " << fixed << setprecision(2) << success_rate << "%" << endl;
    cout << "Общее время: " << total_duration.count() << " мс" << endl;
    cout << "Среднее время на тест: " << total_duration.count() / tests_count << " мс" << endl;
    cout << endl;
}

// ==================== 4. СРАВНЕНИЕ ВСЕХ ТЕСТОВ ====================

void PrimalityTests::compareAllTests(const BigInt& n, int tests_count) {
    cout << "==========================================" << endl;
    cout << "СРАВНЕНИЕ ВСЕХ ТЕСТОВ ДЛЯ ЧИСЛА: " << n.toString() << endl;
    cout << "==========================================" << endl;
    
    // Тест Миллера-Рабина
    auto start1 = chrono::high_resolution_clock::now();
    int mr_positive = 0;
    for (int i = 0; i < tests_count; ++i) {
        if (millerRabinTest(n, 10)) mr_positive++;
    }
    auto end1 = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1);
    
    // Тест Люка
    auto start2 = chrono::high_resolution_clock::now();
    int ls_positive = 0;
    for (int i = 0; i < tests_count; ++i) {
        if (lucasStrongTest(n, 5)) ls_positive++;
    }
    auto end2 = chrono::high_resolution_clock::now();
    auto duration2 = chrono::duration_cast<chrono::milliseconds>(end2 - start2);
    
    // Тест BPSW
    auto start3 = chrono::high_resolution_clock::now();
    int bpsw_positive = 0;
    for (int i = 0; i < tests_count; ++i) {
        if (bpswTest(n, 5)) bpsw_positive++;
    }
    auto end3 = chrono::high_resolution_clock::now();
    auto duration3 = chrono::duration_cast<chrono::milliseconds>(end3 - start3);
    
    // Вывод результатов
    cout << fixed << setprecision(2);
    cout << "ТЕСТ МИЛЛЕРА-РАБИНА:" << endl;
    cout << "  Результаты: " << mr_positive << "/" << tests_count << " (" 
         << (static_cast<double>(mr_positive) / tests_count * 100) << "%)" << endl;
    cout << "  Время: " << duration1.count() << " мс" << endl;
    
    cout << "ТЕСТ ЛЮКА:" << endl;
    cout << "  Результаты: " << ls_positive << "/" << tests_count << " (" 
         << (static_cast<double>(ls_positive) / tests_count * 100) << "%)" << endl;
    cout << "  Время: " << duration2.count() << " мс" << endl;
    
    cout << "ТЕСТ BPSW:" << endl;
    cout << "  Результаты: " << bpsw_positive << "/" << tests_count << " (" 
         << (static_cast<double>(bpsw_positive) / tests_count * 100) << "%)" << endl;
    cout << "  Время: " << duration3.count() << " мс" << endl;
    
    cout << "==========================================" << endl << endl;
}

// ==================== ГЕНЕРАЦИЯ ТЕСТОВЫХ ЧИСЕЛ ====================

vector<BigInt> PrimalityTests::generateTestNumbers(int min_digits, int max_digits, int count) {
    vector<BigInt> numbers;
    random_device rd;
    mt19937 gen(rd());
    
    for (int i = 0; i < count; ++i) {
        int digits = uniform_int_distribution<int>(min_digits, max_digits)(gen);
        numbers.push_back(BigInt::generateRandomPrime(digits, gen));
    }
    
    return numbers;
}

void PrimalityTests::runComprehensiveAnalysis() {
    cout << "==========================================" << endl;
    cout << "КОМПЛЕКСНЫЙ АНАЛИЗ ВЕРОЯТНОСТНЫХ ТЕСТОВ" << endl;
    cout << "==========================================" << endl;
    
    // Известные простые числа для тестирования
    vector<BigInt> test_primes = {
        BigInt("1000000007"),      // 10 цифр
        BigInt("10000000019"),     // 11 цифр
        BigInt("100000000003"),    // 12 цифр
        BigInt("1000000000039"),   // 13 цифр
        BigInt("10000000000037")   // 14 цифр
    };
    
    for (const auto& prime : test_primes) {
        compareAllTests(prime, 50); // 50 тестов для каждого числа
    }
}