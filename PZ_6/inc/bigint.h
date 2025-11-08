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
#include "mt19937lab.h" // Своя реализация вихря Мерсенна
#include "entropy.h"

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
    void fill_random_decimal_digits(size_t numDigits, MT19937lab& mt); // Для вихря Мерсенна
public:
    // ==================== КОНСТРУКТОРЫ ====================

    /**
     * Конструктор по умолчанию - создает число 0
     */
    BigInt();

    /**
     * Конструктор из строки
     * @param str - строка, представляющая число (может содержать знак + или -)
     */
    BigInt(const std::string& str);

    /**
     * Конструктор из целого числа
     * @param num - целое число
     */
    BigInt(long long num);

    /**
     * Конструктор копирования
     * @param other - копируемое число
     */
    BigInt(const BigInt& other);

    /**
     * Конструктор для генерации случайного числа заданной длины
     * @param numDigits - количество цифр в числе
     * @param gen - генератор случайных чисел
     */
    BigInt(int numDigits, std::mt19937& gen);

    // ==================== ОПЕРАТОРЫ ПРИСВАИВАНИЯ ====================

    /**
     * Оператор присваивания для BigInt
     */
    BigInt& operator=(const BigInt& other);

    /**
     * Оператор присваивания для long long
     */
    BigInt& operator=(long long num);

    // ==================== АРИФМЕТИЧЕСКИЕ ОПЕРАТОРЫ ====================

    /**
     * Оператор сложения
     */
    BigInt operator+(const BigInt& other) const;

    /**
     * Оператор вычитания
     */
    BigInt operator-(const BigInt& other) const;

    /**
     * Оператор умножения (алгоритм "в столбик")
     */
    BigInt operator*(const BigInt& other) const;

    /**
     * Оператор деления (алгоритм деления "в столбик")
     */
    BigInt operator/(const BigInt& other) const;

    /**
     * Оператор взятия остатка от деления
     */
    BigInt operator%(const BigInt& other) const;

    /**
     * Оператор возведения в степень (бинарное возведение)
     * Поддерживает только неотрицательные показатели степени
     */
    BigInt operator^(const BigInt& exponent) const;

    // ==================== ОПЕРАТОРЫ СРАВНЕНИЯ ====================

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    // ==================== УНАРНЫЕ ОПЕРАТОРЫ ====================

    /**
     * Унарный минус - меняет знак числа
     */
    BigInt operator-() const;

    /**
     * Унарный плюс - возвращает копию числа
     */
    BigInt operator+() const;

    // ==================== МЕТОДЫ ВВОДА/ВЫВОДА ====================

    friend std::ostream& operator<<(std::ostream& os, const BigInt& num);
    friend std::istream& operator>>(std::istream& is, BigInt& num);

    // ==================== ВСПОМОГАТЕЛЬНЫЕ ПУБЛИЧНЫЕ МЕТОДЫ ====================

    /**
     * Преобразование число в строку
     */
    std::string toString() const;

    /**
     * Проверка на ноль
     */
    bool isZero() const;

    /**
     * Возвращает абсолютное значение числа
     */
    BigInt abs() const;

    /**
     * Возвращает количество цифр в числе
     */
    size_t getDigitCount() const { return digits.size(); }

    // ==================== НОВЫЕ МЕТОДЫ ДЛЯ ДОСТУПА К ЦИФРАМ ====================
    
    /**
     * Возвращает цифру по индексу (младшие разряды в начале)
     * @param index - индекс цифры (0 - младший разряд)
     * @return цифра в указанной позиции
     */
    int getDigitAt(size_t index) const {
        if (index < digits.size()) {
            return digits[index];
        }
        return 0;
    }
    
    /**
     * Возвращает младшую цифру числа (последнюю цифру в десятичном представлении)
     */
    int getLastDigit() const {
        return digits.empty() ? 0 : digits[0];
    }
    
    /**
     * Возвращает все цифры числа (только для чтения)
     */
    const std::vector<int>& getDigits() const {
        return digits;
    }
    
    /**
     * Проверяет, является ли число четным
     */
    bool isEven() const {
        return digits.empty() ? true : (digits[0] % 2 == 0);
    }
    
    /**
     * Проверяет, является ли число нечетным
     */
    bool isOdd() const {
        return !isEven();
    }
    
    /**
     * Преобразует число в двоичное представление (для тестов)
     * @return вектор битов (младшие биты в начале)
     */
    std::vector<int> toBinary() const {
        std::vector<int> binary;
        BigInt temp = this->abs();
        
        while (!temp.isZero()) {
            binary.push_back(temp.getLastDigit() % 2);
            temp = temp / BigInt(2);
        }
        
        if (binary.empty()) {
            binary.push_back(0);
        }
        
        return binary;
    }
    
    /**
     * Возвращает знак числа
     * @return 1 если положительное, -1 если отрицательное, 0 если ноль
     */
    int sign() const {
        if (isZero()) return 0;
        return isNegative ? -1 : 1;
    }

    // ==================== СТАТИЧЕСКИЕ МАТЕМАТИЧЕСКИЕ ФУНКЦИИ ====================

    /**
     * Вычисление наибольшего общего делителя (алгоритм Евклида)
     * @param a, b - числа для вычисления НОД
     * @return НОД(a, b)
     */
    static BigInt gcd(BigInt a, BigInt b);

    /**
     * Вычисление наименьшего общего кратного
     * @param a, b - числа для вычисления НОК
     * @return НОК(a, b) = (a * b) / НОД(a, b)
     */
    static BigInt lcm(const BigInt& a, const BigInt& b);

    /**
     * Генерация случайного простого числа заданной длины
     * @param numDigits - количество цифр в числе
     * @param gen - генератор случайных чисел
     * @return случайное простое число
     */
    static BigInt generateRandomPrime(int numDigits, std::mt19937& gen);

    /**
     * Проверка числа на простоту (тест Миллера-Рабина)
     * @param iterations - количество итераций теста
     * @return true если число вероятно простое, false если составное
     */
    bool isPrime(int iterations = 10) const;
    
    /**
     * Преобразует BigInt в int с проверкой переполнения
     * @return значение как int, если оно в пределах диапазона
     * @throws std::overflow_error если значение слишком велико/мало для int
     */
    int toInt() const;


    // ==================== МЕТОДЫ ПРОВЕРКИ ПРОСТОТЫ ДЛЯ ПРАКТИЧЕСКОЙ РАБОТЫ №2 ====================

    /**
     * Стандартный метод проверки простоты
     */
    static bool isPrimeStandard(const BigInt& n);

    /**
     * Проверка простоты с помощью решета Эратосфена
     */
    static bool isPrimeEratosthenes(const BigInt& n, int limit = 1000000);

    /**
     * Проверка простоты с помощью решета Аткина
     */
    static bool isPrimeAtkin(const BigInt& n, int limit = 1000000);

    /**
     * Тест Люка-Лемера для чисел Мерсенна
     */
    static bool lucasLehmerTest(int p);
    
    /**
     * Вычисление квадратного корня (бинарный поиск)
     */
    static BigInt sqrt(const BigInt& n);
    
    /**
     * Модульное возведение в степень
     */
    static BigInt modPow(const BigInt& base, const BigInt& exponent, const BigInt& mod);

    /**
     * Приближенное вычисление натурального логарифма
     * Используется для AKS теста
     */
    static BigInt log(const BigInt& n);

        // ==================== ГЕНЕРАЦИЯ (LCG) ====================

    /**
     * Инициализировать это BigInt значением линейного конгруэнтного генератора:
     * x = (a * seed + c) mod 10^{numDigits}.
     * Гарантируется, что число имеет ровно numDigits цифр (старшая не ноль).
     * @param numDigits  число десятичных цифр результата (m = 10^{numDigits})
     * @param seed       начальное значение (x_n)
     * @param a          множитель LCG (по умолчанию 1664525)
     * @param c          приращение LCG (по умолчанию 1013904223)
     * @return ссылка на это число (*this)
     */
    BigInt& initLCG(size_t numDigits,
                    const BigInt& seed,
                    const BigInt& a = BigInt(1664525),
                    const BigInt& c = BigInt(1013904223));

    /**
     * То же, но seed берётся из текущего времени (наносекунды).
     * Удобно для «быстрой» инициализации без явного сида.
     */
    BigInt& initLCG(size_t numDigits);

    // ==================== ГЕНЕРАЦИЯ (Mersenne Twister — собственная реализация) ====================

    /**
     * Инициализировать это BigInt случайным числом из MT19937lab (собственная реализация),
     * ровно numDigits десятичных цифр, старшая цифра != 0.
     * @param numDigits число цифр
     * @param seed32    32-битный сид
     * @return *this
     */
    BigInt& initMT(size_t numDigits, uint32_t seed32);

    /**
     * То же, но сид берётся из времени (nanoseconds since epoch).
     */
    BigInt& initMT(size_t numDigits);
    


    // ==================== ГЕНЕРАЦИЯ (Энтропия из курсора — Linux/X11) ====================

    /**
     * То же самое, если вы уже собрали координаты (x,y) и хотите использовать их как источник.
     */
    BigInt& initFromMouseEntropy(size_t numDigits,
                                const std::vector<std::pair<int,int>>& coords);
    
    /**
     * Сгенерировать число, накапливая энтропию из положения курсора мыши (Linux/X11).
     * Активно опрашивает курсор duration_ms миллисекунд с частотой poll_hz,
     * «выбеляет» шум и инициализирует собственный MT19937lab; затем заполняет BigInt.
     * Гарантируются ровно numDigits цифр, старшая != 0.
     */
    BigInt& initFromMouseEntropy(size_t numDigits, int duration_ms = 500, int poll_hz = 500);


};

#endif