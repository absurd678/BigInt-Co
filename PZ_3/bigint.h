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

    // В раздел публичных методов в bigint.h добавьте:

/**
 * Возвращает количество битов в двоичном представлении числа
 */
int bitLength() const;

/**
 * Возвращает значение бита в указанной позиции (0 - младший бит)
 * @param pos - позиция бита
 * @return значение бита (0 или 1)
 */
bool getBit(int pos) const;
};

#endif