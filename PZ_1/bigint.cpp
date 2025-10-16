#include "bigint.h"

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

// Вычисление квадратного корня (бинарный поиск)
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

// Модульное возведение в степень
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

// 1. Стандартный метод проверки простоты
bool BigInt::isPrimeStandard(const BigInt& n) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2)) return true;
    
    // Проверка на четность
    if (n.digits[0] % 2 == 0) return false;
    
    // Проверка деления на 5
    if (n.digits[0] == 5 || n.digits[0] == 0) return n == BigInt(5);
    
    // Проверка суммы цифр на делимость на 3
    int sum = 0;
    for (int digit : n.digits) sum += digit;
    if (sum % 3 == 0) return n == BigInt(3);
    
    // Проверка всех чисел до квадратного корня
    BigInt i(3);
    BigInt limit = sqrt(n) + BigInt(1);
    
    while (i <= limit) {
        if (n % i == BigInt(0)) return false;
        i = i + BigInt(2);
    }
    
    return true;
}

// 2. Решето Эратосфена
bool BigInt::isPrimeEratosthenes(const BigInt& n, int limit) {
    if (n < BigInt(2)) return false;
    
    // Если число маленькое, используем обычное решето
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
        // Число слишком большое для long long
    }
    
    // Для больших чисел используем комбинацию методов
    return isPrimeStandard(n);
}

// 3. Решето Аткина
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
        // Число слишком большое
    }
    
    return isPrimeStandard(n);
}

// 4. Тест Люка-Лемера для чисел Мерсенна
bool BigInt::lucasLehmerTest(int p) {
    if (p < 2) return false;
    if (p == 2) return true;
    
    // Вычисляем число Мерсенна M_p = 2^p - 1
    BigInt mersenne = (BigInt(2) ^ BigInt(p)) - BigInt(1);
    BigInt s(4);
    
    for (int i = 0; i < p - 2; ++i) {
        s = (s * s - BigInt(2)) % mersenne;
    }
    
    return s.isZero();
}

// Упрощенная проверка простоты для демонстрации
bool BigInt::isPrime(int iterations) const {
    return isPrimeStandard(*this);
}

BigInt BigInt::generateRandomPrime(int numDigits, mt19937& gen) {
    if (numDigits <= 0) {
        throw invalid_argument("Invalid number of digits");
    }

    BigInt candidate(numDigits, gen);
    
    // Убедимся, что число нечетное
    if (candidate.digits[0] % 2 == 0) {
        candidate.digits[0] += 1;
    }

    // Простой поиск следующего простого числа
    while (!candidate.isPrime()) {
        candidate = candidate + BigInt(2);
    }

    return candidate;
}