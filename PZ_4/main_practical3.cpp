//   g++ -std=c++11 -O2 main_practical3.cpp bigint.cpp primality_tests.cpp -lgmp -lgmpxx -o practical3
//  ./practical3


#include "bigint.h"
#include "primality_tests.h"
#include <gmpxx.h>
#include <iostream>
#include <iomanip>

using namespace std;

void demonstrateSingleNumber() {
    cout << "==========================================" << endl;
    cout << "ДЕМОНСТРАЦИЯ НА ОДНОМ ЧИСЛЕ" << endl;
    cout << "==========================================" << endl;
    
    BigInt test_prime("1000000007");
    
    // 1. Тест Миллера-Рабина (100 раз)
    PrimalityTests::millerRabinStatistics(test_prime, 100);
    
    // 2. Тест Люка (100 раз)
    PrimalityTests::lucasStrongStatistics(test_prime, 100);
    
    // 3. Тест BPSW (100 раз)
    PrimalityTests::bpswStatistics(test_prime, 100);
    
    // 4. Сравнение всех тестов
    PrimalityTests::compareAllTests(test_prime, 100);
}

void demonstrateMultipleNumbers() {
    cout << "==========================================" << endl;
    cout << "ТЕСТИРОВАНИЕ НА РАЗНЫХ ЧИСЛАХ" << endl;
    cout << "==========================================" << endl;
    
    vector<BigInt> test_numbers = {
        BigInt("101"),           // Маленькое простое
        BigInt("1009"),          // Среднее простое
        BigInt("1000003"),       // Большое простое
        BigInt("1000000007"),    // Очень большое простое
        BigInt("10000000019")    // Гигантское простое
    };
    
    for (const auto& number : test_numbers) {
        cout << "ЧИСЛО: " << number.toString() << " (" 
             << number.getDigitCount() << " цифр)" << endl;
        PrimalityTests::compareAllTests(number, 50);
    }
}

void verifyWithGMP() {
    cout << "==========================================" << endl;
    cout << "ПРОВЕРКА С ИСПОЛЬЗОВАНИЕМ GMP" << endl;
    cout << "==========================================" << endl;
    
    vector<string> gmp_test_numbers = {
        "1000000007",
        "10000000019", 
        "100000000003",
        "1000000000000000003"
    };
    
    for (const auto& num_str : gmp_test_numbers) {
        mpz_class gmp_num(num_str);
        BigInt our_num(num_str);
        
        cout << "Число: " << num_str << endl;
        
        // Проверка GMP
        auto start = chrono::high_resolution_clock::now();
        int gmp_result = mpz_probab_prime_p(gmp_num.get_mpz_t(), 25);
        auto end = chrono::high_resolution_clock::now();
        auto gmp_duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        // Проверка нашим BPSW
        start = chrono::high_resolution_clock::now();
        bool our_result = PrimalityTests::bpswTest(our_num, 10);
        end = chrono::high_resolution_clock::now();
        auto our_duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        cout << "  GMP: " << (gmp_result ? "простое" : "составное") 
             << " (" << gmp_duration.count() << " мс)" << endl;
        cout << "  BPSW: " << (our_result ? "простое" : "составное")
             << " (" << our_duration.count() << " мс)" << endl;
        cout << "  Совпадение: " << (our_result == (gmp_result > 0) ? "✓" : "✗") << endl << endl;
    }
}

int main() {
    setlocale(LC_ALL, "ru");
    
    try {        
        // Задание 1-4: Демонстрация на одном числе
        demonstrateSingleNumber();
        
        // Дополнительно: тестирование на разных числах
        demonstrateMultipleNumbers();
        
        // Проверка с GMP
        verifyWithGMP();
        
        // Комплексный анализ
        PrimalityTests::runComprehensiveAnalysis();
        
        cout << "==========================================" << endl;
        cout << "ВЫВОДЫ:" << endl;
        cout << "1. Тест Миллера-Рабина - быстрый, но вероятностный" << endl;
        cout << "2. Тест Люка - дополняет Миллера-Рабина" << endl;
        cout << "3. BPSW - наиболее надежный комбинированный тест" << endl;
        cout << "4. Все тесты показывают 100% для простых чисел" << endl;
        cout << "5. Для практики рекомендуется использовать BPSW" << endl;
        cout << "==========================================" << endl;
        
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}