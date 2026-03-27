#include "big_integer.h"
#include <algorithm>
#include <stdexcept>
namespace {
void normalize(BigInteger& n) {
    while (n.digits_.size() > 1 && n.digits_.back() == 0) n.digits_.pop_back();

    if (n.digits_.size() == 1 && n.digits_[0] == 0) n.negative_ = false;
}

int abs_compare(const BigInteger& a, const BigInteger& b) {
    if (a.digits_.size() != b.digits_.size()) return (a.digits_.size() < b.digits_.size()) ? -1 : 1;

    for (int i = (int)a.digits_.size() - 1; i >= 0; --i) {
        if (a.digits_[i] != b.digits_[i]) return (a.digits_[i] < b.digits_[i]) ? -1 : 1;
    }
    return 0;
}
BigInteger abs_add(const BigInteger& a, const BigInteger& b) {
    BigInteger res;
    res.digits_.clear();

    int carry = 0;
    size_t n = std::max(a.digits_.size(), b.digits_.size());

    for (size_t i = 0; i < n || carry; ++i) {
        int sum = carry;
        if (i < a.digits_.size()) sum += a.digits_[i];
        if (i < b.digits_.size()) sum += b.digits_[i];

        res.digits_.push_back(sum % 10);
        carry = sum / 10;
    }
    return res;
}
BigInteger abs_sub(const BigInteger& a, const BigInteger& b) {
    // |a| >= |b|
    BigInteger res;
    res.digits_.clear();

    int carry = 0;
    for (size_t i = 0; i < a.digits_.size(); ++i) {
        int cur = a.digits_[i] - carry - (i < b.digits_.size() ? b.digits_[i] : 0);

        if (cur < 0) {
            cur += 10;
            carry = 1;
        } else
            carry = 0;

        res.digits_.push_back(cur);
    }

    normalize(res);
    return res;
}

}  // namespace
BigInteger::BigInteger() : digits_{0}, negative_(false) {}

BigInteger::BigInteger(int value) : BigInteger((long long)value) {}

BigInteger::BigInteger(long long value) {
    negative_ = value < 0;
    if (value < 0) value = -value;

    if (value == 0)
        digits_ = {0};
    else {
        while (value) {
            digits_.push_back(value % 10);
            value /= 10;
        }
    }
}

BigInteger::BigInteger(const std::string& str) {
    digits_.clear();
    negative_ = false;

    size_t pos = 0;
    if (str[pos] == '-') {
        negative_ = true;
        ++pos;
    } else if (str[pos] == '+') {
        ++pos;
    }

    for (size_t i = str.size(); i > pos; --i) {
        if (!isdigit(str[i - 1])) throw std::invalid_argument("Invalid number");
        digits_.push_back(str[i - 1] - '0');
    }

    normalize(*this);
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

std::string BigInteger::to_string() const {
    std::string s;
    if (negative_) s.push_back('-');

    for (int i = (int)digits_.size() - 1; i >= 0; --i) s.push_back(char('0' + digits_[i]));

    return s;
}
bool BigInteger::operator==(const BigInteger& rhs) const {
    return negative_ == rhs.negative_ && digits_ == rhs.digits_;
}

bool BigInteger::operator!=(const BigInteger& rhs) const {
    return !(*this == rhs);
}

bool BigInteger::operator<(const BigInteger& rhs) const {
    if (negative_ != rhs.negative_) return negative_;

    int cmp = abs_compare(*this, rhs);
    return negative_ ? (cmp > 0) : (cmp < 0);
}

bool BigInteger::operator>(const BigInteger& rhs) const {
    return rhs < *this;
}
bool BigInteger::operator<=(const BigInteger& rhs) const {
    return !(*this > rhs);
}
bool BigInteger::operator>=(const BigInteger& rhs) const {
    return !(*this < rhs);
}

BigInteger BigInteger::operator+(const BigInteger& rhs) const {
    BigInteger res;

    if (negative_ == rhs.negative_) {
        res = abs_add(*this, rhs);
        res.negative_ = negative_;
    } else {
        int cmp = abs_compare(*this, rhs);
        if (cmp >= 0) {
            res = abs_sub(*this, rhs);
            res.negative_ = negative_;
        } else {
            res = abs_sub(rhs, *this);
            res.negative_ = rhs.negative_;
        }
    }

    normalize(res);
    return res;
}

BigInteger BigInteger::operator-(const BigInteger& rhs) const {
    return *this + (-rhs);
}

BigInteger BigInteger::operator-() const {
    BigInteger r = *this;
    if (!is_zero()) r.negative_ = !negative_;
    return r;
}

BigInteger BigInteger::operator*(const BigInteger& rhs) const {
    BigInteger res;
    res.digits_.assign(digits_.size() + rhs.digits_.size(), 0);

    for (size_t i = 0; i < digits_.size(); ++i) {
        int carry = 0;
        for (size_t j = 0; j < rhs.digits_.size() || carry; ++j) {
            long long cur = res.digits_[i + j] +
                            digits_[i] * (j < rhs.digits_.size() ? rhs.digits_[j] : 0) + carry;

            res.digits_[i + j] = int(cur % 10);
            carry = int(cur / 10);
        }
    }

    res.negative_ = negative_ != rhs.negative_;
    normalize(res);
    return res;
}

BigInteger BigInteger::operator/(const BigInteger& rhs) const {
    if (rhs.is_zero()) throw std::runtime_error("Division by zero");

    BigInteger a = *this;
    BigInteger b = rhs;
    a.negative_ = b.negative_ = false;

    BigInteger cur;
    BigInteger result;
    result.digits_.assign(a.digits_.size(), 0);

    for (int i = (int)a.digits_.size() - 1; i >= 0; --i) {
        cur.digits_.insert(cur.digits_.begin(), a.digits_[i]);
        normalize(cur);

        int x = 0;
        while (abs_compare(cur, b) >= 0) {
            cur = abs_sub(cur, b);
            ++x;
        }

        result.digits_[i] = x;
    }

    result.negative_ = negative_ != rhs.negative_;
    normalize(result);
    return result;
}

BigInteger BigInteger::operator%(const BigInteger& rhs) const {
    BigInteger q = *this / rhs;
    return *this - q * rhs;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs) {
    return *this = *this + rhs;
}
BigInteger& BigInteger::operator-=(const BigInteger& rhs) {
    return *this = *this - rhs;
}
BigInteger& BigInteger::operator*=(const BigInteger& rhs) {
    return *this = *this * rhs;
}
BigInteger& BigInteger::operator/=(const BigInteger& rhs) {
    return *this = *this / rhs;
}
BigInteger& BigInteger::operator%=(const BigInteger& rhs) {
    return *this = *this % rhs;
}

BigInteger& BigInteger::operator++() {
    return *this += 1;
}
BigInteger BigInteger::operator++(int) {
    BigInteger t = *this;
    ++(*this);
    return t;
}
BigInteger& BigInteger::operator--() {
    return *this -= 1;
}
BigInteger BigInteger::operator--(int) {
    BigInteger t = *this;
    --(*this);
    return t;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& value) {
    return os << value.to_string();
}

std::istream& operator>>(std::istream& is, BigInteger& value) {
    std::string s;
    is >> s;
    value = BigInteger(s);
    return is;
}
