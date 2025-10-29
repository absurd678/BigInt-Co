#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include "bigint.h"
#include <vector>
#include <iostream>

class Polynomial {
private:
    std::vector<BigInt> coefficients; // coefficients[0] = constant term
    
public:
    Polynomial();
    Polynomial(const BigInt& constant);
    Polynomial(const std::vector<BigInt>& coeffs);
    
    // Основные операции
    int degree() const;
    bool isZero() const;
    BigInt getCoefficient(int index) const;
    void setCoefficient(int index, const BigInt& value);
    
    // Арифметические операции
    Polynomial operator+(const Polynomial& other) const;
    Polynomial operator-(const Polynomial& other) const;
    Polynomial operator*(const Polynomial& other) const;
    Polynomial operator*(const BigInt& scalar) const;
    
    // Операции по модулю
    Polynomial mod(const Polynomial& modulus, const BigInt& n) const;
    Polynomial pow(const BigInt& exponent, const Polynomial& modulus, const BigInt& n) const;
    
    // Специальные операции для AKS
    static Polynomial xPower(int k);
    bool isCongruent(const Polynomial& other, const Polynomial& modulus, const BigInt& n) const;
    
    // Утилиты
    void trim();
    std::string toString() const;
    
    friend std::ostream& operator<<(std::ostream& os, const Polynomial& p);
};

#endif