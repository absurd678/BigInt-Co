//  g++ -std=c++11 -O2 main_practical4.cpp bigint.cpp deterministic_primality.cpp polynomial.cpp -lgmp -lgmpxx -o practical4
//  ./practical4


#include "bigint.h"
#include "deterministic_primality.h"
#include <iostream>

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");
    
    try {
        cout << "ПРАКТИЧЕСКАЯ РАБОТА №4" << endl;
        cout << "Детерминированные методы проверки на простоту" << endl;
        cout << "ПОЛНЫЕ РЕАЛИЗАЦИИ AKS И ТЕСТА МИЛЛЕРА" << endl;
        cout << "==========================================" << endl;
        
        // Запуск комплексного тестирования
        DeterministicPrimality::runComprehensiveBenchmark();
        
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}