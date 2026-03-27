#pragma once

#include <iostream>
#include <string>
#include <vector>

class BigInteger {
public:
    // ======================== Constructors ========================

    BigInteger();                          // Default (value 0)
    BigInteger(int value);                 // From int
    BigInteger(long long value);           // From long long
    explicit BigInteger(const std::string& str);  // From string, e.g. "123456789"

    // =================== Copy =====================================

    BigInteger(const BigInteger& other)            = default;
    BigInteger& operator=(const BigInteger& other) = default;

    // ==================== Arithmetic ==============================

    BigInteger  operator+ (const BigInteger& rhs) const;
    BigInteger  operator- (const BigInteger& rhs) const;
    BigInteger  operator* (const BigInteger& rhs) const;
    BigInteger  operator/ (const BigInteger& rhs) const;
    BigInteger  operator% (const BigInteger& rhs) const;

    BigInteger& operator+=(const BigInteger& rhs);
    BigInteger& operator-=(const BigInteger& rhs);
    BigInteger& operator*=(const BigInteger& rhs);
    BigInteger& operator/=(const BigInteger& rhs);
    BigInteger& operator%=(const BigInteger& rhs);

    // ====================== Unary ================================

    BigInteger  operator-() const;         // Unary minus

    BigInteger& operator++();              // Prefix increment
    BigInteger  operator++(int);           // Postfix increment
    BigInteger& operator--();              // Prefix decrement
    BigInteger  operator--(int);           // Postfix decrement

    // ======================= Comparison ==========================

    bool operator==(const BigInteger& rhs) const;
    bool operator!=(const BigInteger& rhs) const;
    bool operator< (const BigInteger& rhs) const;
    bool operator> (const BigInteger& rhs) const;
    bool operator<=(const BigInteger& rhs) const;
    bool operator>=(const BigInteger& rhs) const;

    // ======================== Misc ===============================

    std::string to_string() const;         // Convert to string
    bool is_zero() const;                  // Check if zero
    bool is_negative() const;              // Check sign

    explicit operator bool() const;        // true if != 0

    // ======================== I/O ================================

    friend std::ostream& operator<<(std::ostream& os, const BigInteger& value);
    friend std::istream& operator>>(std::istream& is, BigInteger& value);

private:
    std::vector<int> digits_;
    bool negative_ = false;
    void remove_leading_zeros();
    static int abs_compare(const BigInteger& a, const BigInteger& b);
};
