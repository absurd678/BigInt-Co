#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>  
#include <vector>    
#include <cstdint>
#include "Prime.h"
using namespace std;

// Преобразует число в двоичную строку (младший бит первый)
inline int BianaryTransform(BigInt num, string &bin_num) {
    int i = 0;  
    bin_num.clear();
    while (num != "0") {
        BigInt mod = num % "2";                // Получаем остаток от деления на 2
        bin_num += char(mod[0] + 48);         // Добавляем бит в строку
        num = num / 2;                        // Делим на 2
        i++;
    }
    return i;
}

// Модульное возведение в степень: a^e mod n (бинарный алгоритм)
inline string Modular_Exonentiation(BigInt a, BigInt e, BigInt n) {
    string bin_num;
    BigInt res("1");
    int k = BianaryTransform(e, bin_num) - 1;  // Преобразуем степень в двоичный вид

    // Проходим по битам степени справа налево
    for (int i = k; i >= 0; --i) {
        res = (res * res) % n;          // Квадрат результата
        if (bin_num[i] == '1')          // Если бит установлен
            res = (res * a) % n;        // Умножаем на основание
    }

    // Преобразуем результат в строку
    string out;
    for (int i = Length(res) - 1; i >= 0; --i)
        out += char(res[i] + 48);
    
    if (out.empty()) out = "0";
    return out;
}

// Вычисляет наибольший общий делитель (НОД) рекурсивно
inline BigInt func_gcd(BigInt e, BigInt On) {
    if (On != "0") {
        BigInt temp = e % On;           // Остаток от деления
        return func_gcd(On, temp);      // Рекурсивный вызов
    }
    return e;                         
}

// Вычитание по модулю без отрицательных: (A - B) mod M
inline BigInt sub_mod(const BigInt& A, const BigInt& B, const BigInt& M) {
    if (A >= B) return A - B;           // Если A >= B, просто вычитаем
    BigInt diff = B - A;                // Иначе вычисляем разность (B - A)
    BigInt r = M - (diff % M);          // Находим остаток по модулю
    return (r == M) ? BigInt("0") : r;  // Если r == M, возвращаем 0
}

// Модульная обратная: d = a^{-1} mod m (расширенный алгоритм Евклида)
inline BigInt modInverse(BigInt a, const BigInt& m) {
    if (m == "1") return BigInt("0");
    
    BigInt m0 = m;
    BigInt x0("0"), x1("1");  // Коэффициенты для вычисления обратного
    
    // Нормализация a в диапазоне [0, m)
    a = a % m0;

    while (a > "1") {
        BigInt q = a / m0;  // Частное от деления
        BigInt t = m0; 
        m0 = a % m0;        // Остаток - новый модуль
        a = t;              // Старый модуль - новое a

        // Обновление коэффициентов: x0 = (x1 - q*x0) mod m
        t = x0;
        BigInt qx0 = q * x0;
        x0 = sub_mod(x1, qx0, m);  // Всегда в диапазоне [0, m)
        x1 = t;
    }
    
    // x1 - обратный элемент с точностью до модуля
    x1 = x1 % m;
    return x1;
}

// Инициализация RSA ключей из простых чисел p и q
inline void RSA_Initialize_FromPQ(const BigInt& p, const BigInt& q, 
                                 BigInt &n, BigInt &e, BigInt &d) {
    n = p * q;                                      // Модуль: произведение простых чисел
    BigInt phi = (p - "1") * (q - "1");             // Функция Эйлера: phi(n) = (p-1)*(q-1)
    
    cout << "p = " << p << endl;
    cout << "q = " << q << endl;
    cout << "n = " << n << endl;
    cout << "phi = " << phi << endl;

    e = "65537";                                    // Публичная экспонента
    if (func_gcd(e, phi) != "1") {                  // Проверяем взаимную простоту
        e = "3";                                    // Если 65537 не подходит, начинаем с 3
        while (func_gcd(e, phi) != "1") e += "2";   // Ищем следующее нечётное
    }

    cout << "e = " << e << endl;
    d = modInverse(e, phi);                         // Приватная экспонента
    cout << "d = " << d << endl;
}

// Шифрование текста: каждый символ преобразуется в c = m^e mod n
inline void RSA_Encrypt_FromKeys(const string& plaintext, 
                                 const BigInt& n, const BigInt& e, 
                                 vector<string>& out) {
    out.clear(); 
    out.reserve(plaintext.size());
    cout << "Encrypting " << plaintext.size() << " characters..." << endl;
    
    for (size_t i = 0; i < plaintext.size(); ++i) {
        unsigned char ch = plaintext[i];
        BigInt m((int)ch);                              // Преобразуем символ в число
        cout << "Char " << i << ": '" << ch << "' = " << (int)ch << endl;
        
        string encrypted = Modular_Exonentiation(m, e, n);  // Шифруем: m^e mod n
        out.push_back(encrypted);
        
        cout << "Encrypted to: " << encrypted << endl;
    }
}

// Преобразует число в вектор битов 
inline std::vector<uint8_t> ToBitsBE(BigInt e) {
    std::vector<uint8_t> le;
    while (e != "0") {
        BigInt r = e % "2";                 // Получаем младший бит
        le.push_back((r == "1") ? 1u : 0u);
        e = e / 2;                          // Делим на 2
    }
    if (le.empty()) le.push_back(0);
    std::reverse(le.begin(), le.end());     // Переворачиваем для MSB-first
    return le;
}

// Быстрое модульное возведение в степень с методом скользящего окна
inline BigInt ModExp_WindowBits(BigInt base, const std::vector<uint8_t>& bits, 
                                const BigInt& mod, int w = 5) {
    if (mod == "1") return BigInt("0");
    base = base % mod;  // Нормализация основания
    if (base == "0") return BigInt("0");

    // Предвычисление таблицы нечётных степеней: g^1, g^3, ..., g^(2^w-1)
    int tblSize = 1 << (w - 1);
    std::vector<BigInt> T(tblSize);
    T[0] = base;  // g^1
    BigInt g2 = (base * base) % mod;  // g^2
    for (int i = 1; i < tblSize; ++i)
        T[i] = (T[i-1] * g2) % mod;  // g^(2i+1) = предыдущее * g^2

    BigInt result("1");
    size_t i = 0, n = bits.size();

    // Проход по битам экспоненты с использованием окна
    while (i < n) {
        if (bits[i] == 0) {
            result = (result * result) % mod;  // Квадрат при нулевом бите
            ++i;
        } else {
            // Формируем окно из следующих w битов
            int width = 1;
            int val = 1;  // Уже учли текущую единицу
            size_t j = i + 1;
            while (j < n && width < w) {
                val = (val << 1) | bits[j];  // Сдвиг и добавление бита
                ++width; ++j;
            }
            // Откатываем хвостовые нули для получения нечётного значения
            while ((val & 1) == 0) { val >>= 1; --width; --j; }

            // Возведение в квадрат width раз
            for (int k = 0; k < width; ++k)
                result = (result * result) % mod;

            // Умножение на предвычисленную степень из таблицы
            int idx = (val - 1) >> 1;  // Индекс в таблице (1->0, 3->1, ...)
            result = (result * T[idx]) % mod;

            i = j;  // Переход к следующему окну
        }
    }
    return result;
}

// Дешифрование одного сообщения 
inline BigInt RSA_Decrypt_One_CRT(const BigInt& c,
                                  const BigInt& p, const BigInt& q,
                                  const BigInt& d) {
    BigInt one("1");
    // Вычисляем d по модулю (p-1) и (q-1) для ускорения (разбиваем на 2 степени)
    BigInt dp = d % (p - one);  // d mod (p-1)
    BigInt dq = d % (q - one);  // d mod (q-1)
    BigInt qInv = modInverse(q, p);  // Обратный к q по модулю p

    // Преобразуем экспоненты в биты для быстрого возведения в степень
    std::vector<uint8_t> bits_dp = ToBitsBE(dp);
    std::vector<uint8_t> bits_dq = ToBitsBE(dq);

    // Вычисляем остатки зашифрованного сообщения
    BigInt c_p = c % p;  // c mod p
    BigInt c_q = c % q;  // c mod q

    // Возводим в степень по модулю p и q отдельно (быстрее)
    BigInt m1 = ModExp_WindowBits(c_p, bits_dp, p, 5);  // c^dp mod p
    BigInt m2 = ModExp_WindowBits(c_q, bits_dq, q, 5);  // c^dq mod q

    // Восстанавливаем исходное сообщение по формуле Гарнера
    BigInt diff = sub_mod(m1, m2, p);  // (m1 - m2) mod p
    BigInt h = (qInv * diff) % p;  // Коэффициент для восстановления
    BigInt m = m2 + h * q;  // Итоговое сообщение: m mod n
    return m;
}

// Быстрое дешифрование массива зашифрованных сообщений с CRT
inline void RSA_Decrypt_FromKeys_CRT_Fast(const std::vector<std::string>& cipher,
                                          const BigInt& n, const BigInt& d,
                                          const BigInt& p, const BigInt& q,
                                          std::vector<std::string>& out) {
    out.clear(); 
    out.reserve(cipher.size());
    cout << "Decrypting " << cipher.size() << " ciphertexts..." << endl;
    
    for (size_t i = 0; i < cipher.size(); ++i) {
        const std::string& cs = cipher[i];
        BigInt C(cs);                                   // Преобразуем строку в число
        cout << "Ciphertext " << i << ": " << cs << endl;
        
        BigInt M = RSA_Decrypt_One_CRT(C, p, q, d);     // Дешифруем с CRT
        
        // Преобразуем результат обратно в строку
        std::string s; 
        s.reserve(Length(M));
        for (int j = Length(M) - 1; j >= 0; --j) 
            s.push_back(char(M[j] + 48));
        
        if (s.empty()) s = "0";
        out.push_back(s);
        
        cout << "Decrypted to: " << s << endl;
    }
}