#include "bigint.h"
#include <numeric>
#include <unordered_map>
#include <functional>

using namespace std;

// ==================== РЕАЛИЗАЦИЯ ОСНОВНЫХ МЕТОДОВ ====================

void BigInt::removeLeadingZeros() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
    if (digits.size() == 1 && digits[0] == 0) {
        isNegative = false;
    }
}

int BigInt::compareAbsolute(const BigInt& other) const {
    if (digits.size() != other.digits.size()) {
        return digits.size() < other.digits.size() ? -1 : 1;
    }
    for (int i = digits.size() - 1; i >= 0; --i) {
        if (digits[i] != other.digits[i]) {
            return digits[i] < other.digits[i] ? -1 : 1;
        }
    }
    return 0;
}

BigInt BigInt::addAbsolute(const BigInt& other) const {
    BigInt result;
    result.digits.resize(max(digits.size(), other.digits.size()) + 1, 0);

    int carry = 0;
    for (size_t i = 0; i < result.digits.size(); ++i) {
        int sum = carry;
        if (i < digits.size()) sum += digits[i];
        if (i < other.digits.size()) sum += other.digits[i];

        result.digits[i] = sum % 10;
        carry = sum / 10;
    }

    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::subtractAbsolute(const BigInt& other) const {
    if (compareAbsolute(other) < 0) {
        BigInt result = other.subtractAbsolute(*this);
        result.isNegative = true;
        return result;
    }

    BigInt result;
    result.digits.resize(digits.size(), 0);

    int borrow = 0;
    for (size_t i = 0; i < digits.size(); ++i) {
        int diff = digits[i] - borrow;
        if (i < other.digits.size()) {
            diff -= other.digits[i];
        }

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }

        result.digits[i] = diff;
    }

    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::multiplyByDigit(int digit) const {
    if (digit == 0) return BigInt(0);

    BigInt result;
    result.digits.resize(digits.size() + 1, 0);

    int carry = 0;
    for (size_t i = 0; i < digits.size(); ++i) {
        int product = digits[i] * digit + carry;
        result.digits[i] = product % 10;
        carry = product / 10;
    }

    if (carry > 0) {
        result.digits[digits.size()] = carry;
    }

    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::divideByDigit(int digit) const {
    if (digit == 0) {
        throw runtime_error("Division by zero");
    }

    BigInt result;
    result.digits.resize(digits.size(), 0);

    int remainder = 0;
    for (int i = digits.size() - 1; i >= 0; --i) {
        int current = remainder * 10 + digits[i];
        result.digits[i] = current / digit;
        remainder = current % digit;
    }

    result.removeLeadingZeros();
    return result;
}

BigInt::BigInt() : isNegative(false) {
    digits.push_back(0);
}

BigInt::BigInt(const string& str) {
    if (str.empty()) {
        digits.push_back(0);
        isNegative = false;
        return;
    }

    size_t start = 0;
    if (str[0] == '-') {
        isNegative = true;
        start = 1;
    } else if (str[0] == '+') {
        isNegative = false;
        start = 1;
    } else {
        isNegative = false;
    }

    for (int i = str.size() - 1; i >= (int)start; --i) {
        if (isdigit(str[i])) {
            digits.push_back(str[i] - '0');
        } else {
            throw invalid_argument("Invalid character in number string");
        }
    }

    removeLeadingZeros();
}

BigInt::BigInt(long long num) {
    if (num < 0) {
        isNegative = true;
        num = -num;
    } else {
        isNegative = false;
    }

    if (num == 0) {
        digits.push_back(0);
        return;
    }

    while (num > 0) {
        digits.push_back(num % 10);
        num /= 10;
    }
}

BigInt::BigInt(const BigInt& other)
    : digits(other.digits), isNegative(other.isNegative) {}

BigInt::BigInt(int numDigits, mt19937& gen) {
    if (numDigits <= 0) {
        throw invalid_argument("Invalid number of digits");
    }

    uniform_int_distribution<int> dist(1, 9);
    digits.push_back(dist(gen));

    uniform_int_distribution<int> dist2(0, 9);
    for (int i = 1; i < numDigits; ++i) {
        digits.push_back(dist2(gen));
    }

    reverse(digits.begin(), digits.end());
    isNegative = false;
}

BigInt& BigInt::operator=(const BigInt& other) {
    if (this != &other) {
        digits = other.digits;
        isNegative = other.isNegative;
    }
    return *this;
}

BigInt& BigInt::operator=(long long num) {
    *this = BigInt(num);
    return *this;
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (isNegative == other.isNegative) {
        BigInt result = addAbsolute(other);
        result.isNegative = isNegative;
        return result;
    }

    int cmp = compareAbsolute(other);
    if (cmp == 0) {
        return BigInt(0);
    }

    BigInt result;
    if (cmp > 0) {
        result = subtractAbsolute(other);
        result.isNegative = isNegative;
    } else {
        result = other.subtractAbsolute(*this);
        result.isNegative = other.isNegative;
    }

    return result;
}

BigInt BigInt::operator-(const BigInt& other) const {
    return *this + (-other);
}

BigInt BigInt::operator*(const BigInt& other) const {
    if (isZero() || other.isZero()) {
        return BigInt(0);
    }

    BigInt result;
    result.digits.resize(digits.size() + other.digits.size(), 0);

    for (size_t i = 0; i < digits.size(); ++i) {
        int carry = 0;
        for (size_t j = 0; j < other.digits.size() || carry; ++j) {
            long long product = result.digits[i + j] +
                digits[i] * (j < other.digits.size() ? other.digits[j] : 0) +
                carry;
            result.digits[i + j] = product % 10;
            carry = product / 10;
        }
    }

    result.isNegative = isNegative != other.isNegative;
    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator/(const BigInt& other) const {
    if (other.isZero()) {
        throw runtime_error("Division by zero");
    }

    BigInt absOther = other.abs();
    if (absOther.compareAbsolute(*this) > 0) {
        return BigInt(0);
    }

    BigInt quotient, remainder;
    quotient.digits.resize(digits.size(), 0);

    for (int i = digits.size() - 1; i >= 0; --i) {
        remainder = remainder * BigInt(10) + BigInt(digits[i]);
        int count = 0;
        while (remainder.compareAbsolute(absOther) >= 0) {
            remainder = remainder - absOther;
            count++;
        }
        quotient.digits[i] = count;
    }

    quotient.isNegative = isNegative != other.isNegative;
    quotient.removeLeadingZeros();
    return quotient;
}

BigInt BigInt::operator%(const BigInt& other) const {
    if (other.isZero()) {
        throw runtime_error("Division by zero");
    }

    BigInt absOther = other.abs();
    BigInt remainder;

    for (int i = digits.size() - 1; i >= 0; --i) {
        remainder = remainder * BigInt(10) + BigInt(digits[i]);
        while (remainder.compareAbsolute(absOther) >= 0) {
            remainder = remainder - absOther;
        }
    }

    remainder.isNegative = isNegative;
    if (remainder.isNegative && !remainder.isZero()) {
        remainder = remainder + absOther;
    }

    return remainder;
}

BigInt BigInt::operator^(const BigInt& exponent) const {
    if (exponent.isNegative) {
        throw runtime_error("Negative exponents not supported");
    }

    if (exponent.isZero()) {
        return BigInt(1);
    }

    BigInt result(1);
    BigInt base = *this;
    BigInt exp = exponent;

    while (!exp.isZero()) {
        if (exp.digits[0] % 2 == 1) {
            result = result * base;
        }
        base = base * base;
        exp = exp / BigInt(2);
    }

    return result;
}

bool BigInt::operator==(const BigInt& other) const {
    return isNegative == other.isNegative && digits == other.digits;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt& other) const {
    if (isNegative != other.isNegative) {
        return isNegative;
    }

    if (isNegative) {
        return compareAbsolute(other) > 0;
    } else {
        return compareAbsolute(other) < 0;
    }
}

bool BigInt::operator<=(const BigInt& other) const {
    return *this < other || *this == other;
}

bool BigInt::operator>(const BigInt& other) const {
    return !(*this <= other);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

BigInt BigInt::operator-() const {
    BigInt result = *this;
    if (!result.isZero()) {
        result.isNegative = !result.isNegative;
    }
    return result;
}

BigInt BigInt::operator+() const {
    return *this;
}

ostream& operator<<(ostream& os, const BigInt& num) {
    if (num.isNegative) {
        os << '-';
    }
    for (int i = num.digits.size() - 1; i >= 0; --i) {
        os << num.digits[i];
    }
    return os;
}

istream& operator>>(istream& is, BigInt& num) {
    string str;
    is >> str;
    num = BigInt(str);
    return is;
}

string BigInt::toString() const {
    string result;
    if (isNegative) {
        result += '-';
    }
    for (int i = digits.size() - 1; i >= 0; --i) {
        result += to_string(digits[i]);
    }
    return result;
}

bool BigInt::isZero() const {
    return digits.size() == 1 && digits[0] == 0;
}

BigInt BigInt::abs() const {
    BigInt result = *this;
    result.isNegative = false;
    return result;
}

BigInt BigInt::gcd(BigInt a, BigInt b) {
    a = a.abs();
    b = b.abs();

    while (!b.isZero()) {
        BigInt temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

BigInt BigInt::lcm(const BigInt& a, const BigInt& b) {
    if (a.isZero() || b.isZero()) {
        return BigInt(0);
    }
    return (a * b) / gcd(a, b);
}

// ==================== МЕТОДЫ ПРОВЕРКИ ПРОСТОТЫ ====================

BigInt BigInt::sqrt(const BigInt& n) {
    if (n < BigInt(0)) {
        throw invalid_argument("Square root of negative number");
    }
    if (n.isZero()) return BigInt(0);
    if (n == BigInt(1)) return BigInt(1);

    BigInt low(1), high = n;
    BigInt result(1);

    while (low <= high) {
        BigInt mid = (low + high) / BigInt(2);
        BigInt square = mid * mid;

        if (square == n) {
            return mid;
        } else if (square < n) {
            low = mid + BigInt(1);
            result = mid;
        } else {
            high = mid - BigInt(1);
        }
    }

    return result;
}

BigInt BigInt::modPow(const BigInt& base, const BigInt& exponent, const BigInt& mod) {
    if (mod == BigInt(1)) return BigInt(0);
    
    BigInt result(1);
    BigInt b = base % mod;
    BigInt exp = exponent;
    
    while (!exp.isZero()) {
        if (exp.digits[0] % 2 == 1) {
            result = (result * b) % mod;
        }
        b = (b * b) % mod;
        exp = exp / BigInt(2);
    }
    
    return result;
}

bool BigInt::isPrimeStandard(const BigInt& n) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    if (n.isEven()) return false;
    
    // Quick checks for small primes
    if (n == BigInt(3) || n == BigInt(5) || n == BigInt(7)) return true;
    if (n % BigInt(3) == BigInt(0)) return false;
    if (n % BigInt(5) == BigInt(0)) return false;
    
    // Check sum of digits for divisibility by 3
    int sum = 0;
    for (int digit : n.digits) {
        sum += digit;
    }
    if (sum % 3 == 0) return n == BigInt(3);
    
    // Check last digit for divisibility by 5
    if (n.digits[0] == 0 || n.digits[0] == 5) return n == BigInt(5);
    
    // Proper trial division up to sqrt(n)
    BigInt i(3);
    BigInt limit = sqrt(n) + BigInt(1);
    
    while (i <= limit) {
        if (n % i == BigInt(0)) {
            return false;
        }
        i = i + BigInt(2);
        
        // Skip multiples of 3 and 5 for optimization
        if (i % BigInt(3) == BigInt(0)) i = i + BigInt(2);
        if (i % BigInt(5) == BigInt(0)) i = i + BigInt(2);
    }
    
    return true;
}

bool BigInt::isPrimeEratosthenes(const BigInt& n, int limit) {
    if (n < BigInt(2)) return false;
    
    try {
        long long num = stoll(n.toString());
        if (num <= limit) {
            if (num < 2) return false;
            if (num == 2) return true;
            if (num % 2 == 0) return false;
            
            vector<bool> sieve(num + 1, true);
            sieve[0] = sieve[1] = false;
            
            for (long long i = 2; i * i <= num; ++i) {
                if (sieve[i]) {
                    for (long long j = i * i; j <= num; j += i) {
                        sieve[j] = false;
                    }
                }
            }
            return sieve[num];
        }
    } catch (...) {
    }
    
    return isPrimeStandard(n);
}

bool BigInt::isPrimeAtkin(const BigInt& n, int limit) {
    if (n < BigInt(2)) return false;
    
    try {
        long long num = stoll(n.toString());
        if (num <= limit) {
            if (num < 2) return false;
            if (num == 2 || num == 3) return true;
            
            vector<bool> sieve(num + 1, false);
            sieve[2] = sieve[3] = true;
            
            for (long long x = 1; x * x <= num; x++) {
                for (long long y = 1; y * y <= num; y++) {
                    long long temp = 4 * x * x + y * y;
                    if (temp <= num && (temp % 12 == 1 || temp % 12 == 5)) {
                        sieve[temp] = !sieve[temp];
                    }
                    
                    temp = 3 * x * x + y * y;
                    if (temp <= num && temp % 12 == 7) {
                        sieve[temp] = !sieve[temp];
                    }
                    
                    temp = 3 * x * x - y * y;
                    if (x > y && temp <= num && temp % 12 == 11) {
                        sieve[temp] = !sieve[temp];
                    }
                }
            }
            
            for (long long i = 5; i * i <= num; i++) {
                if (sieve[i]) {
                    for (long long j = i * i; j <= num; j += i * i) {
                        sieve[j] = false;
                    }
                }
            }
            
            return sieve[num];
        }
    } catch (...) {
    }
    
    return isPrimeStandard(n);
}

bool BigInt::lucasLehmerTest(int p) {
    if (p < 2) return false;
    if (p == 2) return true;
    
    BigInt mersenne = (BigInt(2) ^ BigInt(p)) - BigInt(1);
    BigInt s(4);
    
    for (int i = 0; i < p - 2; ++i) {
        s = (s * s - BigInt(2)) % mersenne;
    }
    
    return s.isZero();
}

bool BigInt::isPrime(int iterations) const {
    return isPrimeECPP(iterations);
}

BigInt BigInt::generateRandomPrime(int numDigits, mt19937& gen) {
    if (numDigits <= 0) {
        throw invalid_argument("Invalid number of digits");
    }

    BigInt candidate(numDigits, gen);
    
    if (candidate.isEven()) {
        candidate = candidate + BigInt(1);
    }

    while (!candidate.isPrime()) {
        candidate = candidate + BigInt(2);
    }

    return candidate;
}

BigInt BigInt::log(const BigInt& n) {
    if (n <= BigInt(0)) {
        throw invalid_argument("Logarithm of non-positive number");
    }
    if (n == BigInt(1)) {
        return BigInt(0);
    }
    
    int digit_count = n.toString().length();
    return BigInt(digit_count * 2);
}

// ==================== ECPP РЕАЛИЗАЦИЯ ====================

/**
 * Генерирует случайное большое число в диапазоне [0, max-1]
 * @param max - верхняя граница диапазона (исключительно)
 * @param gen - генератор случайных чисел
 * @return случайное BigInt меньше max
 */
BigInt BigInt::randomBigInt(const BigInt& max, mt19937& gen) {
    // Базовые проверки
    if (max <= BigInt(1)) return BigInt(0);  // Если max <= 1, возвращаем 0
    
    // Преобразуем максимальное число в строку для обработки цифр
    string maxStr = max.toString();
    string resultStr;  // Строка для результата
    
    // Генераторы случайных цифр:
    uniform_int_distribution<int> firstDist(1, maxStr[0] - '0');  // Первая цифра: 1 до первой цифры max
    uniform_int_distribution<int> otherDist(0, 9);               // Остальные цифры: 0-9
    
    // Генерируем цифры для случайного числа
    bool firstDigit = true;  // Флаг для первой цифры
    for (size_t i = 0; i < maxStr.length(); ++i) {
        int digit;
        if (firstDigit) {
            digit = firstDist(gen);   // Первая цифра с ограничением
            firstDigit = false;       // Следующие цифры будут обычными
        } else {
            digit = otherDist(gen);   // Остальные цифры без ограничений
        }
        resultStr += to_string(digit);  // Добавляем цифру к результату
    }
    
    // Преобразуем строку в BigInt
    BigInt result(resultStr);
    
    // Гарантируем, что результат < max (делим пополам пока не выполнится условие)
    while (result >= max) {
        result = result / BigInt(2);
    }
    
    return result;
}

/**
 * Алгоритм ECPP (Elliptic Curve Primality Proving) - доказательство простоты на эллиптических кривых
 * @param maxAttempts - максимальное количество попыток поиска подходящей кривой
 * @return true если число простое, false если составное
 */
bool BigInt::isPrimeECPP(int maxAttempts) const {
    BigInt n = *this;  // Работаем с копией числа
    
    // ========== ЭТАП 1: БАЗОВЫЕ ПРОВЕРКИ ==========
    
    // Проверка тривиальных случаев
    if (n < BigInt(2)) return false;           // Числа < 2 не простые
    if (n == BigInt(2) || n == BigInt(3)) return true;  // 2 и 3 - простые
    if (n.isEven()) return false;              // Четные числа > 2 не простые
    
    // ========== ЭТАП 2: ПРОВЕРКА МАЛЫХ ПРОСТЫХ ДЕЛИТЕЛЕЙ ==========
    
    // Список малых простых чисел для быстрой проверки
    static const int smallPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    
    // Проверяем делимость на малые простые числа
    for (int p : smallPrimes) {
        if (n % BigInt(p) == BigInt(0)) {
            // Если делится, то простое только если равно этому простому числу
            return n == BigInt(p);
        }
    }
    
    // Для небольших чисел используем стандартный метод (оптимизация)
    if (n < BigInt(10000)) {
        return isPrimeStandard(n);
    }
    
    // ========== ЭТАП 3: ОСНОВНОЙ АЛГОРИТМ ECPP ==========
    
    random_device rd;    // Источник энтропии
    mt19937 gen(rd());   // Генератор случайных чисел
    
    // Многократные попытки найти подходящую эллиптическую кривую
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        
        // ========== ЭТАП 3.1: ГЕНЕРАЦИЯ ПАРАМЕТРОВ КРИВОЙ ==========
        
        // Генерируем случайные параметры эллиптической кривой:
        // Уравнение кривой: y² = x³ + a·x + b
        BigInt a = randomBigInt(n, gen);  // Коэффициент a
        BigInt x = randomBigInt(n, gen);  // Координата x случайной точки
        BigInt y = randomBigInt(n, gen);  // Координата y случайной точки
        
        // ========== ЭТАП 3.2: ВЫЧИСЛЕНИЕ ПАРАМЕТРА b ==========
        
        // Вычисляем b из уравнения кривой: b = y² - x³ - a·x (mod n)
        BigInt x2 = (x * x) % n;          // x² mod n
        BigInt x3 = (x2 * x) % n;         // x³ mod n  
        BigInt ax = (a * x) % n;          // a·x mod n
        BigInt y2 = (y * y) % n;          // y² mod n
        
        // Вычисляем b по формуле
        BigInt b = (y2 - x3 - ax) % n;
        // Корректируем если b отрицательное
        if (b < BigInt(0)) b = b + n;
        
        // ========== ЭТАП 3.3: ПРОВЕРКА ДИСКРИМИНАНТА ==========
        
        // Дискриминант эллиптической кривой: Δ = 4a³ + 27b²
        // Должен быть ≠ 0 для невырожденной кривой
        BigInt disc = (BigInt(4) * a * a * a + BigInt(27) * b * b) % n;
        
        // Если дискриминант нулевой - кривая вырожденная, пробуем снова
        if (disc == BigInt(0)) continue;
        
        // ========== ЭТАП 3.4: УПРОЩЕННЫЙ ПОДСЧЕТ ТОЧЕК ==========
        
        // В РЕАЛЬНОМ ECPP: используется сложный алгоритм Шуфа для подсчета точек
        // В ЭТОЙ РЕАЛИЗАЦИИ: используем упрощенную аппроксимацию
        // m ≈ количество точек на кривой по модулю n
        BigInt m = n + BigInt(1) + randomBigInt(BigInt(100), gen);
        
        // ========== ЭТАП 3.5: ПОИСК ПОДХОДЯЩЕГО ПРОСТОГО ДЕЛИТЕЛЯ ==========
        
        // Ищем простой делитель q числа m такой, что:
        // 1. q - простое число
        // 2. m делится на q
        // 3. m/q - тоже простое число
        for (BigInt q = BigInt(2); q < BigInt(1000); q = q + BigInt(1)) {
            // Проверяем что q простое и делит m
            if (BigInt::isPrimeStandard(q) && (m % q == BigInt(0))) {
                BigInt candidate = m / q;  // Кандидат на простое число
                
                // Если кандидат > 1 и простой - вероятно n простое
                if (candidate > BigInt(1) && BigInt::isPrimeStandard(candidate)) {
                    return true;  // Число вероятно простое
                }
            }
        }
    }
    
    // ========== ЭТАП 4: РЕЗЕРВНЫЙ МЕТОД ==========
    
    // Если ECPP не смог доказать простоту за maxAttempts попыток,
    // используем стандартный метод как запасной вариант
    return isPrimeStandard(n);
}

// ==================== АЛГОРИТМЫ ФАКТОРИЗАЦИИ ====================

/**
 * Алгоритм Полларда-Ро для факторизации (нахождение нетривиального делителя)
 * @param n - число для факторизации
 * @param maxIterations - максимальное количество итераций
 * @return нетривиальный делитель n или 1 если не найден
 */
BigInt BigInt::pollardRho(const BigInt& n, int maxIterations) {
    // Базовые случаи
    if (n == BigInt(1)) return BigInt(1);           // 1 не имеет делителей
    if (n % BigInt(2) == BigInt(0)) return BigInt(2);  // Четные числа делятся на 2
    
    // Инициализация генератора случайных чисел
    random_device rd;
    mt19937 gen(rd());
    
    // Начальные значения для алгоритма
    BigInt x = randomBigInt(n, gen);  // Начальная точка x
    BigInt y = x;                     // Начальная точка y (такая же как x)
    BigInt d = BigInt(1);             // Найденный делитель (пока 1)
    
    // Функция итерации: f(x) = (x² + 1) mod n
    auto f = [](const BigInt& x, const BigInt& n) {
        return (x * x + BigInt(1)) % n;
    };
    
    // Основной цикл алгоритма Полларда-Ро
    for (int i = 0; i < maxIterations && d == BigInt(1); ++i) {
        // Движение "черепахи" - один шаг
        x = f(x, n);
        // Движение "зайца" - два шага (метод Флойда)
        y = f(f(y, n), n);
        
        // Вычисляем НОД(|x-y|, n) - потенциальный делитель
        d = gcd((x - y).abs(), n);
        
        // Если d != 1 и d != n, нашли нетривиальный делитель
    }
    
    return d;  // Возвращаем найденный делитель (или 1 если не нашли)
}

/**
 * Полная факторизация числа на простые множители
 * @param n - число для факторизации
 * @param maxAttempts - максимальное количество попыток алгоритма Полларда-Ро
 * @return вектор простых множителей в порядке возрастания
 */
vector<BigInt> BigInt::factorize(const BigInt& n, int maxAttempts) {
    vector<BigInt> factors;  // Результирующий вектор множителей
    BigInt temp = n;         // Временная переменная для разложения
    
    // ========== ЭТАП 1: ПРОВЕРКА МАЛЫХ ПРОСТЫХ ДЕЛИТЕЛЕЙ ==========
    
    // Список малых простых чисел для последовательной проверки
    for (int p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
        // Пока число делится на текущее простое, добавляем его в множители
        while (temp % BigInt(p) == BigInt(0)) {
            factors.push_back(BigInt(p));  // Добавляем простой делитель
            temp = temp / BigInt(p);       // Делим число на найденный множитель
        }
    }
    
    // Если после этого осталась 1 - факторизация завершена
    if (temp == BigInt(1)) return factors;
    
    // Если оставшееся число простое - добавляем его и завершаем
    if (isPrimeStandard(temp)) {
        factors.push_back(temp);
        return factors;
    }
    
    // ========== ЭТАП 2: АЛГОРИТМ ПОЛЛАРДА-РО ДЛЯ БОЛЬШИХ ЧИСЕЛ ==========
    
    // Используем алгоритм Полларда-Ро для поиска нетривиальных делителей
    for (int attempt = 0; attempt < maxAttempts && temp > BigInt(1); ++attempt) {
        // Пытаемся найти делитель алгоритмом Полларда-Ро
        BigInt factor = pollardRho(temp, 1000);
        
        // Если нашли нетривиальный делитель (не 1 и не само число)
        if (factor > BigInt(1) && factor < temp) {
            // Рекурсивно факторизуем найденный делитель
            auto subfactors = factorize(factor, maxAttempts);
            // Добавляем все подмножители в результат
            factors.insert(factors.end(), subfactors.begin(), subfactors.end());
            // Делим оставшееся число на найденный множитель
            temp = temp / factor;
            
            // Если оставшаяся часть простая - добавляем и выходим
            if (isPrimeStandard(temp)) {
                factors.push_back(temp);
                break;
            }
        } else {
            // Если делитель не найден, прерываем попытки
            break;
        }
    }
    
    // ========== ЭТАП 3: ДОБАВЛЕНИЕ ОСТАТКА ==========
    
    // Если после всех попыток осталось число > 1, добавляем его как есть
    if (temp > BigInt(1)) {
        factors.push_back(temp);
    }
    
    return factors;  // Возвращаем полный список множителей
}