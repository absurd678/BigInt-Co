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
    
    // Вычисляем a^d mod n
    BigInt x = BigInt::modPow(a, d, n);
    
    if (x == BigInt(1) || x == n - BigInt(1)) {
        return false; // Не свидетель
    }
    
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
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n.getLastDigit() % 2 == 0) return false;  // Исправлено
    
    // Маленькие простые числа для быстрой проверки
    vector<int> small_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    for (int p : small_primes) {
        if (n == BigInt(p)) return true;
        if (n % BigInt(p) == BigInt(0)) return false;
    }
    
    auto witnesses = getWitnesses(n, iterations);
    
    for (const auto& a : witnesses) {
        if (isWitness(a, n)) {
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

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ДЛЯ ТЕСТА ЛЮКА ====================

// Вычисление символа Якоби (a/n)
int PrimalityTests::jacobiSymbol(const BigInt& a, const BigInt& n) {
    if (n == BigInt(1)) return 1;
    if (a == BigInt(0)) return 0;
    if (a == BigInt(1)) return 1;
    
    BigInt a_temp = a % n;
    BigInt n_temp = n;
    
    int result = 1;
    
    while (a_temp != BigInt(0)) {
        // Убираем множители 2
        while (a_temp % BigInt(2) == BigInt(0)) {
            a_temp = a_temp / BigInt(2);
            BigInt mod8 = n_temp % BigInt(8);
            if (mod8 == BigInt(3) || mod8 == BigInt(5)) {
                result = -result;
            }
        }
        
        // Меняем местами по квадратичному закону взаимности
        BigInt temp = a_temp;
        a_temp = n_temp;
        n_temp = temp;
        
        if (a_temp % BigInt(4) == BigInt(3) && n_temp % BigInt(4) == BigInt(3)) {
            result = -result;
        }
        
        a_temp = a_temp % n_temp;
    }
    
    return (n_temp == BigInt(1)) ? result : 0;
}

// Нахождение параметров для теста Люка (D, P, Q)
tuple<BigInt, int, int> PrimalityTests::findLucasParameters(const BigInt& n) {
    //initialize
    int d_val = 5;
    int sign = 1;
    
    while (true) {
        BigInt d(sign * d_val);
        int jacobi = jacobiSymbol(d, n);
        
        // Нужен символ Якоби = -1 - it's main condition
        if (jacobi == -1) {
            // P = 1, Q = (1 - D) / 4
            int p = 1;
            int q = (1 - sign * d_val) / 4;
            return make_tuple(d, p, q);
        }
        
        // Чередуем последовательность: 5, -7, 9, -11, 13, -15, ...
        if (sign == 1) {
            d_val += 2;
            sign = -1;
        } else {
            sign = 1;
        }
        
        // Защита от бесконечного цикла
        if (d_val > 100) {
            return make_tuple(BigInt(5), 1, -1); // Возвращаем значения по умолчанию
        }
    }
}

// Вычисление последовательности Люка U_k mod n
BigInt PrimalityTests::lucasSequenceU(const BigInt& k, int p, int q, const BigInt& n) {
    if (k == BigInt(0)) return BigInt(0);
    if (k == BigInt(1)) return BigInt(1);
    
    BigInt u_prev2 = BigInt(0); // U_0
    BigInt u_prev1 = BigInt(1); // U_1
    BigInt u_current;
    
    // Вычисляем по бинарному представлению k
    int bits = k.bitLength();
    for (int i = bits - 2; i >= 0; i--) {
        // Удвоение индекса: U_{2m} = U_m * V_m
        BigInt u_temp = u_prev1;
        BigInt v_temp = (u_prev1 * BigInt(p) - BigInt(2) * u_prev2) % n;
        u_current = (u_prev1 * v_temp) % n;
        
        if (u_current < BigInt(0)) u_current = u_current + n;
        
        if (k.getBit(i)) {
            // U_{m+1} = (P * U_m - Q * U_{m-1}) / 2
            BigInt u_next = (BigInt(p) * u_current - BigInt(q) * u_prev1) % n; //main moment
            if (u_next % BigInt(2) != BigInt(0)) {
                u_next = u_next + n; // Делаем четным для деления
            }
            u_next = u_next / BigInt(2);
            
            u_prev2 = u_current;
            u_prev1 = u_next;
        } else {
            u_prev2 = u_prev1;
            u_prev1 = u_current;
        }
        
        // Нормализация по модулю
        if (u_prev1 < BigInt(0)) u_prev1 = u_prev1 + n;
        if (u_prev2 < BigInt(0)) u_prev2 = u_prev2 + n;
    }
    
    return u_prev1;
}

// Вычисление последовательности Люка V_k mod n
BigInt PrimalityTests::lucasSequenceV(const BigInt& k, int p, int q, const BigInt& n) {
    if (k == BigInt(0)) return BigInt(2);
    if (k == BigInt(1)) return BigInt(p);
    
    BigInt v_prev2 = BigInt(2);  // V_0
    BigInt v_prev1 = BigInt(p);  // V_1
    BigInt v_current;
    
    int bits = k.bitLength();
    for (int i = bits - 2; i >= 0; i--) {
        // Удвоение индекса: V_{2m} = V_m^2 - 2Q^m
        v_current = (v_prev1 * v_prev1 - BigInt(2)) % n;
        
        if (v_current < BigInt(0)) v_current = v_current + n;
        
        if (k.getBit(i)) {
            // V_{m+1} = P * V_m - Q * V_{m-1}
            BigInt v_next = (BigInt(p) * v_current - BigInt(q) * v_prev1) % n; //main moment
            if (v_next < BigInt(0)) v_next = v_next + n;
            
            v_prev2 = v_current;
            v_prev1 = v_next;
        } else {
            v_prev2 = v_prev1;
            v_prev1 = v_current;
        }
    }
    
    return v_prev1;
}

// ==================== ТЕСТ ЛЮКА НА СИЛЬНУЮ ПСЕВДОПРОСТОТУ ====================

bool PrimalityTests::isStrongLucasWitness(const BigInt& n, const BigInt& d, int p, int q) {
    // Проверяем, что gcd(Q, n) = 1
    BigInt gcd_qn = BigInt::gcd(BigInt(q), n);
    if (gcd_qn != BigInt(1) && gcd_qn != n) {
        return true; // Найден нетривиальный делитель
    }
    
    // Вычисляем n - (D/n) = n + 1 (так как (D/n) = -1)
    BigInt m = n + BigInt(1);
    
    // Находим представление m = d * 2^s
    BigInt d_temp = m;
    int s = 0;
    while (d_temp % BigInt(2) == BigInt(0)) {
        d_temp = d_temp / BigInt(2);
        s++;
    }
    
    // Вычисляем U_d mod n
    BigInt u_d = lucasSequenceU(d_temp, p, q, n);
    
    // Проверка 1: U_d ≡ 0 (mod n)
    if (u_d == BigInt(0)) {
        return false; // Не свидетель
    }
    
    // Проверка 2: последовательность V_{d*2^r} для r = 0...s-1
    BigInt v_current = lucasSequenceV(d_temp, p, q, n);
    
    for (int r = 0; r < s; r++) {
        if (v_current == BigInt(0)) {
            return false; // Не свидетель
        }
        
        // Переходим к следующей степени: V_{2k} = V_k^2 - 2
        if (r < s - 1) {
            v_current = (v_current * v_current - BigInt(2)) % n;
            if (v_current < BigInt(0)) v_current = v_current + n;
        }
    }
    
    return true; // Свидетель составности
}

bool PrimalityTests::lucasStrongTest(const BigInt& n, int iterations) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n % BigInt(2) == BigInt(0)) return false;
    
    // Быстрая проверка маленьких простых
    vector<int> small_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    for (int p : small_primes) {
        if (n == BigInt(p)) return true;
        if (n % BigInt(p) == BigInt(0)) return false;
    }
    
    // Проверяем, является ли n точным квадратом
    BigInt sqrt_n = BigInt::sqrt(n);
    if (sqrt_n * sqrt_n == n) {
        return false;
    }
    
    // Находим параметры для теста Люка
    auto params = findLucasParameters(n);
    BigInt d = get<0>(params);
    int p = get<1>(params);
    int q = get<2>(params);
    
    // Проводим тест с разными параметрами для повышения надежности
    for (int i = 0; i < iterations; ++i) {
        if (isStrongLucasWitness(n, d, p, q)) {
            return false; // Найден свидетель составности
        }
        
        // Можно также менять параметры для каждой итерации
        // для большей надежности (опционально)
    }
    
    return true; // Вероятно простое
}

void PrimalityTests::lucasStrongStatistics(const BigInt& n, int tests_count) {
    cout << "     ТЕСТ ЛЮКА НА СИЛЬНУЮ ПСЕВДОПРОСТОТУ " << endl;
    cout << "Число: " << n.toString() << endl;
    cout << "Количество тестов: " << tests_count << endl;
    
    int positive_results = 0;
    auto total_start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < tests_count; ++i) {
        auto start = chrono::high_resolution_clock::now();
        bool result = lucasStrongTest(n, 1); // 1 итерация достаточно для детерминированного теста
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
    
    // Дополнительная информация о параметрах
    auto params = findLucasParameters(n);
    cout << "Параметры теста: D = " << get<0>(params).toString() 
         << ", P = " << get<1>(params) 
         << ", Q = " << get<2>(params) << endl;
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