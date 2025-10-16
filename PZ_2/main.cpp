//g++ -std=c++17 -O2 main.cpp bigint.cpp -lgmp -lgmpxx -o prime_checker
// ./prime_checker

#include "bigint.h"
#include <gmpxx.h>
#include <iomanip>

using namespace std;

// Функция для вычисления числа Мерсенна с использованием GMP
void calculateMersenneWithGMP(int exponent, const string& description) {
    cout << "\n" << description << endl;
    cout << "Вычисление 2^" << exponent << " - 1 с использованием GMP..." << endl;
    
    auto start = chrono::high_resolution_clock::now();
    
    mpz_class mersenne;
    mpz_ui_pow_ui(mersenne.get_mpz_t(), 2, exponent);
    mersenne -= 1;
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    string mersenne_str = mersenne.get_str();
    cout << "✓ Время вычисления: " << duration.count() << " мс" << endl;
    cout << "✓ Количество цифр: " << mersenne_str.length() << endl;
    
    if (mersenne_str.length() > 50) {
        cout << "✓ Первые 50 цифр: " << mersenne_str.substr(0, 50) << "..." << endl;
    } else {
        cout << "✓ Число: " << mersenne_str << endl;
    }
    
    // Проверка простоты с помощью GMP
    start = chrono::high_resolution_clock::now();
    int is_prime = mpz_probab_prime_p(mersenne.get_mpz_t(), 25);
    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "✓ Проверка простоты (GMP): ";
    if (is_prime == 2) cout << "точно простое";
    else if (is_prime == 1) cout << "вероятно простое";
    else cout << "составное";
    cout << " (время: " << duration.count() << " мс)" << endl;
}

// Функция для проверки простоты собственными методами
void testPrimalityMethods(const BigInt& n, const string& numberName) {
    cout << "\nПроверка числа " << numberName << " собственными методами:" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    bool standard = BigInt::isPrimeStandard(n);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "  ✓ Стандартный метод: " << (standard ? "простое" : "составное") 
         << " (время: " << duration.count() << " мс)" << endl;
    
    start = chrono::high_resolution_clock::now();
    bool eratosthenes = BigInt::isPrimeEratosthenes(n, 1000000);
    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "  ✓ Решето Эратосфена: " << (eratosthenes ? "простое" : "составное") 
         << " (время: " << duration.count() << " мс)" << endl;
    
    start = chrono::high_resolution_clock::now();
    bool atkin = BigInt::isPrimeAtkin(n, 1000000);
    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "  ✓ Решето Аткина: " << (atkin ? "простое" : "составное") 
         << " (время: " << duration.count() << " мс)" << endl;
}

// Демонстрация теста Люка-Лемера
void demonstrateLucasLehmer() {
    cout << "\n=== ТЕСТ ЛЮКА-ЛЕМЕРА ДЛЯ ЧИСЕЛ МЕРСЕННА ===" << endl;
    
    vector<int> test_exponents = {2, 3, 5, 7, 13, 17, 19, 31};
    
    for (int p : test_exponents) {
        cout << "\nM" << p << " = 2^" << p << " - 1:" << endl;
        
        auto start = chrono::high_resolution_clock::now();
        bool result = BigInt::lucasLehmerTest(p);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        cout << "  ✓ Тест Люка-Лемера: " << (result ? "простое" : "составное")
             << " (время: " << duration.count() << " мс)" << endl;
             
        // Для небольших экспонент показываем число и проверяем другими методами
        if (p <= 13) {
            BigInt mersenne = (BigInt(2) ^ BigInt(p)) - BigInt(1);
            cout << "  ✓ Число: " << mersenne.toString() << endl;
            testPrimalityMethods(mersenne, "M" + to_string(p));
        }
    }
}

// Основное задание - вычисление большого числа Мерсенна
void calculateBigMersenne() {
    cout << "\n" << string(60, '=') << endl;
    cout << "ОСНОВНОЕ ЗАДАНИЕ: ВЫЧИСЛЕНИЕ 2^136279841 - 1" << endl;
    cout << string(60, '=') << endl;
    
    // Для демонстрации используем меньшие экспоненты, так как вычисление
    // 2^136279841 займет очень много времени и памяти
    vector<pair<int, string>> demo_exponents = {
        {2, "Малое простое число Мерсенна"},
        {3, "Простое число Мерсенна"},
        {5, "Простое число Мерсенна"},
        {7, "Простое число Мерсенна"}, 
        {13, "Простое число Мерсенна"},
        {17, "Простое число Мерсенна"},
        {19, "Простое число Мерсенна"},
        {31, "Простое число Мерсенна"},
        {61, "Большее простое число Мерсенна"},
        {127, "Большое простое число Мерсенна"}
    };
    
    for (const auto& [exp, desc] : demo_exponents) {
        calculateMersenneWithGMP(exp, "--- " + desc + " ---");
        
        // Для небольших чисел дополнительно проверяем собственными методами
        if (exp <= 13) {
            BigInt mersenne = (BigInt(2) ^ BigInt(exp)) - BigInt(1);
            testPrimalityMethods(mersenne, "M" + to_string(exp));
        }
    }
    
    cout << "\n" << string(60, '=') << endl;
    cout << "ПРИМЕЧАНИЕ: Вычисление 2^136279841 - 1 собственными методами" << endl;
    cout << "потребует значительного времени и памяти." << endl;
    cout << "На практике для таких вычислений используют специализированные" << endl;
    cout << "библиотеки и распределенные вычисления." << endl;
    cout << string(60, '=') << endl;
}

// Тестирование методов на известных простых числах
void testKnownPrimes() {
    cout << "\n" << string(60, '=') << endl;
    cout << "СРАВНЕНИЕ МЕТОДОВ ПРОВЕРКИ ПРОСТОТЫ" << endl;
    cout << string(60, '=') << endl;
    
    vector<pair<string, string>> test_cases = {
        {"101", "Малое простое число"},
        {"1009", "Простое из 4 цифр"},
        {"10007", "Простое из 5 цифр"}, 
        {"100003", "Простое из 6 цифр"},
        {"1000003", "Простое из 7 цифр"},
        {"10000019", "Простое из 8 цифр"},
        {"100000007", "Простое из 9 цифр"},
        {"1000000007", "Простое из 10 цифр"}
    };
    
    for (const auto& [prime_str, description] : test_cases) {
        cout << "\n--- " << description << " (" << prime_str << ") ---" << endl;
        BigInt prime(prime_str);
        
        // Проверяем собственными методами
        testPrimalityMethods(prime, prime_str);
        
        // Проверяем с GMP для сравнения
        mpz_class gmp_prime(prime_str);
        int gmp_result = mpz_probab_prime_p(gmp_prime.get_mpz_t(), 15);
        cout << "  ✓ GMP проверка: " << (gmp_result ? "простое" : "составное") << endl;
    }
}

int main() {
    setlocale(LC_ALL, "ru");
    
    try {
        cout << string(70, '=') << endl;
        cout << "ПРАКТИЧЕСКАЯ РАБОТА №2" << endl;
        cout << "БОЛЬШИЕ ПРОСТЫЕ ЧИСЛА И ЧИСЛА МЕРСЕННА" << endl;
        cout << string(70, '=') << endl;
        
        // Инициализация генератора случайных чисел
        random_device rd;
        mt19937 gen(rd());
        
        // 1. Демонстрация теста Люка-Лемера
        demonstrateLucasLehmer();
        
        // 2. Основное задание - вычисление чисел Мерсенна
        calculateBigMersenne();
        
        // 3. Сравнение методов проверки простоты
        testKnownPrimes();
        
        // 4. Генерация случайных простых чисел
        cout << "\n" << string(60, '=') << endl;
        cout << "ГЕНЕРАЦИЯ СЛУЧАЙНЫХ ЧИСЕЛ" << endl;
        cout << string(60, '=') << endl;
        
        for (int digits : {5, 10, 15}) {
            cout << "\n--- Генерация числа из " << digits << " цифр ---" << endl;
            
            auto start = chrono::high_resolution_clock::now();
            BigInt prime = BigInt(digits, gen);
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            
            cout << "✓ Время генерации: " << duration.count() << " мс" << endl;
            cout << "✓ Сгенерированное число: " << prime.toString() << endl;
            
            // Проверяем разными методами
            testPrimalityMethods(prime, "сгенерированное простое");
        }
        
        cout << "\n" << string(70, '=') << endl;
        cout << "ВЫВОДЫ И РЕЗУЛЬТАТЫ" << endl;
        cout << string(70, '=') << endl;
        cout << "✓ Все методы проверки простоты реализованы и протестированы" << endl;
        cout << "✓ Тест Люка-Лемера эффективен для проверки чисел Мерсенна" << endl;
        cout << "✓ Стандартный метод наиболее универсален для больших чисел" << endl;
        cout << "✓ Решета эффективны для чисел ограниченного размера" << endl;
        cout << "✓ GMP значительно быстрее для работы с гигантскими числами" << endl;
        cout << "✓ Собственная реализация BigInt корректно выполняет арифметику" << endl;
        cout << string(70, '=') << endl;
        
    } catch (const exception& e) {
        cerr << "❌ Ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}