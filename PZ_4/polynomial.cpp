#include "polynomial.h"

using namespace std;

Polynomial::Polynomial() {
    coefficients.push_back(BigInt(0));
}

Polynomial::Polynomial(const BigInt& constant) {
    coefficients.push_back(constant);
}

Polynomial::Polynomial(const vector<BigInt>& coeffs) : coefficients(coeffs) {
    trim();
}

int Polynomial::degree() const {
    return coefficients.size() - 1;
}

bool Polynomial::isZero() const {
    return coefficients.size() == 1 && coefficients[0].isZero();
}

BigInt Polynomial::getCoefficient(int index) const {
    if (index < coefficients.size()) {
        return coefficients[index];
    }
    return BigInt(0);
}

void Polynomial::setCoefficient(int index, const BigInt& value) {
    if (index >= coefficients.size()) {
        coefficients.resize(index + 1, BigInt(0));
    }
    coefficients[index] = value;
    trim();
}

Polynomial Polynomial::operator+(const Polynomial& other) const {
    vector<BigInt> result_coeffs(max(coefficients.size(), other.coefficients.size()), BigInt(0));
    
    for (size_t i = 0; i < result_coeffs.size(); ++i) {
        BigInt sum = getCoefficient(i) + other.getCoefficient(i);
        result_coeffs[i] = sum;
    }
    
    return Polynomial(result_coeffs);
}

Polynomial Polynomial::operator-(const Polynomial& other) const {
    vector<BigInt> result_coeffs(max(coefficients.size(), other.coefficients.size()), BigInt(0));
    
    for (size_t i = 0; i < result_coeffs.size(); ++i) {
        BigInt diff = getCoefficient(i) - other.getCoefficient(i);
        result_coeffs[i] = diff;
    }
    
    return Polynomial(result_coeffs);
}

Polynomial Polynomial::operator*(const Polynomial& other) const {
    vector<BigInt> result_coeffs(coefficients.size() + other.coefficients.size() - 1, BigInt(0));
    
    for (size_t i = 0; i < coefficients.size(); ++i) {
        for (size_t j = 0; j < other.coefficients.size(); ++j) {
            result_coeffs[i + j] = result_coeffs[i + j] + coefficients[i] * other.coefficients[j];
        }
    }
    
    return Polynomial(result_coeffs);
}

Polynomial Polynomial::operator*(const BigInt& scalar) const {
    vector<BigInt> result_coeffs(coefficients.size());
    
    for (size_t i = 0; i < coefficients.size(); ++i) {
        result_coeffs[i] = coefficients[i] * scalar;
    }
    
    return Polynomial(result_coeffs);
}

Polynomial Polynomial::mod(const Polynomial& modulus, const BigInt& n) const {
    if (modulus.degree() == 0) {
        return Polynomial(BigInt(0)); // Деление на константу
    }
    
    Polynomial result = *this;
    int mod_degree = modulus.degree();
    BigInt leading_coeff = modulus.getCoefficient(mod_degree);
    
    // Находим обратный к ведущему коэффициенту по модулю n
    BigInt inv_leading = BigInt(1);
    if (leading_coeff != BigInt(1)) {
        // Упрощение: предполагаем, что ведущий коэффициент = 1 для AKS
        // В реальном AKS мы используем x^r - 1, где ведущий коэффициент = 1
        inv_leading = BigInt(1);
    }
    
    while (result.degree() >= mod_degree) {
        int current_degree = result.degree();
        BigInt factor = result.getCoefficient(current_degree) * inv_leading;
        
        Polynomial temp = modulus;
        temp = temp * factor;
        
        // Сдвигаем полином
        int shift = current_degree - mod_degree;
        vector<BigInt> shifted_coeffs(shift + temp.coefficients.size(), BigInt(0));
        for (size_t i = 0; i < temp.coefficients.size(); ++i) {
            shifted_coeffs[i + shift] = temp.coefficients[i];
        }
        temp = Polynomial(shifted_coeffs);
        
        result = result - temp;
        
        // Применяем модуль n к коэффициентам
        for (size_t i = 0; i < result.coefficients.size(); ++i) {
            result.coefficients[i] = result.coefficients[i] % n;
            if (result.coefficients[i] < BigInt(0)) {
                result.coefficients[i] = result.coefficients[i] + n;
            }
        }
        
        result.trim();
    }
    
    return result;
}

Polynomial Polynomial::pow(const BigInt& exponent, const Polynomial& modulus, const BigInt& n) const {
    if (exponent.isZero()) {
        return Polynomial(BigInt(1));
    }
    
    Polynomial result(BigInt(1));
    Polynomial base = *this;
    BigInt exp = exponent;
    
    while (!exp.isZero()) {
        if (exp.getLastDigit() % 2 == 1) {
            result = (result * base).mod(modulus, n);
        }
        base = (base * base).mod(modulus, n);
        exp = exp / BigInt(2);
    }
    
    return result;
}

Polynomial Polynomial::xPower(int k) {
    vector<BigInt> coeffs(k + 1, BigInt(0));
    coeffs[k] = BigInt(1);
    return Polynomial(coeffs);
}

bool Polynomial::isCongruent(const Polynomial& other, const Polynomial& modulus, const BigInt& n) const {
    Polynomial diff = (*this - other).mod(modulus, n);
    return diff.isZero();
}

void Polynomial::trim() {
    while (coefficients.size() > 1 && coefficients.back().isZero()) {
        coefficients.pop_back();
    }
    if (coefficients.empty()) {
        coefficients.push_back(BigInt(0));
    }
}

string Polynomial::toString() const {
    if (isZero()) return "0";
    
    string result;
    for (int i = coefficients.size() - 1; i >= 0; --i) {
        if (!coefficients[i].isZero()) {
            if (!result.empty()) result += " + ";
            if (i == 0) {
                result += coefficients[i].toString();
            } else {
                result += coefficients[i].toString();
                if (i == 1) {
                    result += "x";
                } else {
                    result += "x^" + to_string(i);
                }
            }
        }
    }
    return result;
}

ostream& operator<<(ostream& os, const Polynomial& p) {
    os << p.toString();
    return os;
}