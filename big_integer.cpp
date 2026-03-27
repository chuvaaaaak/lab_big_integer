#include "big_integer.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
BigInteger::BigInteger() : digits_({0}), negative_(false) {}

BigInteger::BigInteger(int value) : BigInteger(static_cast<long long>(value)) {}

BigInteger::BigInteger(long long value) {
    if (value == 0) {
        digits_.push_back(0);
        negative_ = false;
        return;
    }
    
    negative_ = (value < 0);
    long long abs_value = negative_ ? -value : value;
    
    while (abs_value > 0) {
        digits_.push_back(abs_value % 10);
        abs_value /= 10;
    }
}

BigInteger::BigInteger(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty string");
    }
    
    size_t start = 0;
    negative_ = false;
    
    if (str[0] == '-') {
        negative_ = true;
        start = 1;
    } else if (str[0] == '+') {
        start = 1;
    }
    
    if (start >= str.length()) {
        throw std::invalid_argument("String contains only sign");
    }
    
    // Find first non-zero digit
    size_t first_non_zero = str.length();
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) {
            throw std::invalid_argument("Invalid character");
        }
        if (str[i] != '0' && first_non_zero == str.length()) {
            first_non_zero = i;
        }
    }
    
    // All zeros
    if (first_non_zero == str.length()) {
        digits_.push_back(0);
        negative_ = false;
        return;
    }
    
    // Store digits in reverse order (least significant first)
    for (size_t i = str.length(); i-- > first_non_zero; ) {
        digits_.push_back(str[i] - '0');
    }
}

BigInteger BigInteger::operator+(const BigInteger& rhs) const {
    if (negative_ == rhs.negative_) {
        // Same sign - add absolute values
        BigInteger result;
        result.negative_ = negative_;
        int carry = 0;
        size_t max_size = std::max(digits_.size(), rhs.digits_.size());
        
        for (size_t i = 0; i < max_size || carry; ++i) {
            int sum = carry;
            if (i < digits_.size()) sum += digits_[i];
            if (i < rhs.digits_.size()) sum += rhs.digits_[i];
            result.digits_.push_back(sum % 10);
            carry = sum / 10;
        }
        return result;
    }
    
    // Different signs - convert to subtraction
    if (negative_) {
        return rhs - (-(*this));
    } else {
        return *this - (-rhs);
    }
}

BigInteger BigInteger::operator-(const BigInteger& rhs) const {
    if (negative_ != rhs.negative_) {
        // Different signs - convert to addition
        BigInteger rhs_abs = rhs;
        rhs_abs.negative_ = !rhs_abs.negative_;
        return *this + rhs_abs;
    }
    
    // Same sign - subtract absolute values
    bool result_negative;
    const BigInteger *larger, *smaller;
    
    if (BigInteger::abs_compare(*this, rhs) >= 0) {
        larger = this;
        smaller = &rhs;
        result_negative = negative_;
    } else {
        larger = &rhs;
        smaller = this;
        result_negative = !negative_;
    }
    
    BigInteger result;
    result.negative_ = result_negative;
    int borrow = 0;
    
    for (size_t i = 0; i < larger->digits_.size(); ++i) {
        int diff = larger->digits_[i] - borrow;
        if (i < smaller->digits_.size()) {
            diff -= smaller->digits_[i];
        }
        
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits_.push_back(diff);
    }
    
    result.remove_leading_zeros();
    return result;
}

BigInteger BigInteger::operator*(const BigInteger& rhs) const {
    if (is_zero() || rhs.is_zero()) {
        return BigInteger();
    }
    
    BigInteger result;
    result.negative_ = negative_ != rhs.negative_;
    
    // Temporary array for intermediate products
    std::vector<int> temp(digits_.size() + rhs.digits_.size(), 0);
    
    // Multiply each digit
    for (size_t i = 0; i < digits_.size(); ++i) {
        for (size_t j = 0; j < rhs.digits_.size(); ++j) {
            temp[i + j] += digits_[i] * rhs.digits_[j];
        }
    }
    
    // Handle carries
    int carry = 0;
    for (size_t i = 0; i < temp.size(); ++i) {
        int sum = temp[i] + carry;
        result.digits_.push_back(sum % 10);
        carry = sum / 10;
    }
    
    // Add remaining carry
    while (carry > 0) {
        result.digits_.push_back(carry % 10);
        carry /= 10;
    }
    
    result.remove_leading_zeros();
    return result;
}

BigInteger BigInteger::operator/(const BigInteger& rhs) const {
    if (rhs.is_zero()) {
        throw std::runtime_error("Division by zero");
    }
    
    if (BigInteger::abs_compare(*this, rhs) < 0) {
        return BigInteger();
    }
    
    BigInteger dividend = *this;
    BigInteger divisor = rhs;
    dividend.negative_ = false;
    divisor.negative_ = false;
    
    BigInteger quotient;
    quotient.digits_.resize(dividend.digits_.size(), 0);
    BigInteger remainder;
    
    for (int i = static_cast<int>(dividend.digits_.size()) - 1; i >= 0; --i) {
        // Add next digit to remainder
        remainder.digits_.insert(remainder.digits_.begin(), dividend.digits_[i]);
        remainder.remove_leading_zeros();
        
        // Binary search for digit
        int digit = 0;
        int low = 0, high = 9;
        
        while (low <= high) {
            int mid = (low + high) / 2;
            BigInteger product = divisor * BigInteger(mid);
            
            if (BigInteger::abs_compare(product, remainder) <= 0) {
                digit = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        
        quotient.digits_[i] = digit;
        remainder = remainder - (divisor * BigInteger(digit));
    }
    
    quotient.negative_ = negative_ != rhs.negative_;
    quotient.remove_leading_zeros();
    
    if (quotient.is_zero()) {
        quotient.negative_ = false;
    }
    
    return quotient;
}

BigInteger BigInteger::operator%(const BigInteger& rhs) const {
    if (rhs.is_zero()) {
        throw std::runtime_error("Modulo by zero");
    }
    
    BigInteger remainder = *this - (*this / rhs) * rhs;
    
    // Adjust remainder sign to match dividend (C++ standard)
    if (!is_zero() && !rhs.is_zero()) {
        if (is_negative() && remainder > BigInteger(0)) {
            remainder = remainder - rhs;
        } else if (!is_negative() && remainder < BigInteger(0)) {
            remainder = remainder + rhs;
        }
    }
    
    return remainder;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs) {
    *this = *this + rhs;
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& rhs) {
    *this = *this - rhs;
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& rhs) {
    *this = *this * rhs;
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& rhs) {
    *this = *this / rhs;
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& rhs) {
    *this = *this % rhs;
    return *this;
}

BigInteger BigInteger::operator-() const {
    BigInteger result = *this;
    if (!result.is_zero()) {
        result.negative_ = !result.negative_;
    }
    return result;
}

BigInteger& BigInteger::operator++() {
    *this = *this + BigInteger(1);
    return *this;
}

BigInteger BigInteger::operator++(int) {
    BigInteger temp = *this;
    ++(*this);
    return temp;
}

BigInteger& BigInteger::operator--() {
    *this = *this - BigInteger(1);
    return *this;
}

BigInteger BigInteger::operator--(int) {
    BigInteger temp = *this;
    --(*this);
    return temp;
}

bool BigInteger::operator==(const BigInteger& rhs) const {
    if (negative_ != rhs.negative_) return false;
    if (digits_.size() != rhs.digits_.size()) return false;
    return digits_ == rhs.digits_;
}

bool BigInteger::operator!=(const BigInteger& rhs) const {
    return !(*this == rhs);
}

bool BigInteger::operator<(const BigInteger& rhs) const {
    if (negative_ != rhs.negative_) {
        return negative_;
    }
    
    int cmp = BigInteger::abs_compare(*this, rhs);
    if (cmp != 0) {
        return negative_ ? (cmp > 0) : (cmp < 0);
    }
    return false;
}

bool BigInteger::operator>(const BigInteger& rhs) const {
    return rhs < *this;
}

bool BigInteger::operator<=(const BigInteger& rhs) const {
    return !(rhs < *this);
}

bool BigInteger::operator>=(const BigInteger& rhs) const {
    return !(*this < rhs);
}

std::string BigInteger::to_string() const {
    if (is_zero()) {
        return "0";
    }
    
    std::string result;
    if (negative_) {
        result += '-';
    }
    
    for (int i = static_cast<int>(digits_.size()) - 1; i >= 0; --i) {
        result += static_cast<char>('0' + digits_[i]);
    }
    return result;
}

bool BigInteger::is_zero() const {
    return digits_.size() == 1 && digits_[0] == 0;
}

bool BigInteger::is_negative() const {
    return negative_;
}

BigInteger::operator bool() const {
    return !is_zero();
}

std::ostream& operator<<(std::ostream& os, const BigInteger& value) {
    os << value.to_string();
    return os;
}

std::istream& operator>>(std::istream& is, BigInteger& value) {
    std::string str;
    is >> str;
    value = BigInteger(str);
    return is;
}

void BigInteger::remove_leading_zeros() {
    while (digits_.size() > 1 && digits_.back() == 0) {
        digits_.pop_back();
    }
    if (is_zero()) {
        negative_ = false;
    }
}

int BigInteger::abs_compare(const BigInteger& a, const BigInteger& b) {
    if (a.digits_.size() != b.digits_.size()) {
        return a.digits_.size() < b.digits_.size() ? -1 : 1;
    }
    
    for (int i = static_cast<int>(a.digits_.size()) - 1; i >= 0; --i) {
        if (a.digits_[i] != b.digits_[i]) {
            return a.digits_[i] < b.digits_[i] ? -1 : 1;
        }
    }
    return 0;
}
