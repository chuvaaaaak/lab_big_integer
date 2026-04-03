#include "big_integer.h"
#include <utility>
#include <algorithm>
BigInteger::BigInteger(int value): negative_(value < 0){
    long long val = std::abs(static_cast<long long>(value));
    do {
        digits_.push_back(mod(val, 10));
        val /= 10;
    } while (val > 0);
}
long long BigInteger::mod(long long a, long long b) const{
    return ((a % b) + b) % b;
}
BigInteger::BigInteger(long long value): negative_(value < 0){
    value = std::abs(value);
    do {
        digits_.push_back(mod(value, 10));
        value /= 10;
    } while (value > 0);
}
BigInteger::BigInteger(): negative_(0){
    digits_.push_back(0);
}

BigInteger::BigInteger(const std::string& str): negative_(false) {
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+'){
        negative_ = (str[0] == '-');
        start = 1;
    }
    while (start + 1 < str.size() && str[start] == '0'){
        ++start;
    }
    for (size_t i = str.size(); i > start; --i){
        digits_.push_back(str[i - 1] - '0');
    }
}
std::ostream& operator<<(std::ostream& os, const BigInteger& value){
    if (value.is_negative() && !value.is_zero()){
        os << '-';
    }
    for (int i = value.digits_.size() - 1; i > -1; --i){
        os << value.digits_[i];
    }
    return os;
}
std::istream& operator>>(std::istream& is, BigInteger& value){
    value.digits_.clear();
    std::string res;
    size_t start = 0;
    if (!(is >> res)) return is;
    if (res[0] == '-') {
        value.negative_ = true;
        ++start;
    } else if (res[0] == '+') {
        ++start;
    }
    while (start < res.size() - 1 && res[start] == '0') {
        ++start;
    }
    for (size_t i = res.size(); i > start; --i) {
        value.digits_.push_back(res[i - 1] - '0');
    }
    return is;
}
bool BigInteger::is_negative() const{
    return negative_;
}
BigInteger BigInteger::operator-() const{
    BigInteger a = *this;
    if (!a.is_zero()) a.negative_ = !a.negative_;
    return a;
}
bool BigInteger::operator<(const BigInteger& rhs) const{
    if (is_zero() && rhs.is_zero()) return 0;
    if (is_negative() && !rhs.is_negative()){
        return 1;
    }
    if (!is_negative() && rhs.is_negative()){
        return 0;
    }
    if (digits_.size() < rhs.digits_.size()){
        return !is_negative();
    }
    if (digits_.size() > rhs.digits_.size()){
        return is_negative();
    }
    for (int i = digits_.size() - 1; i > -1; --i){
        if (digits_[i] < rhs.digits_[i]) return !is_negative();
        if (digits_[i] > rhs.digits_[i]) return is_negative();
    }
    return 0;
}
bool BigInteger::operator<=(const BigInteger& rhs) const{
    return !(rhs < *this);
}
bool BigInteger::operator>(const BigInteger& rhs) const{
    return (rhs < *this);
}
bool BigInteger::operator>=(const BigInteger& rhs) const{
    return !(*this < rhs);
}
bool BigInteger::operator==(const BigInteger& rhs) const{
    return !(*this < rhs || rhs < *this);
}
bool BigInteger::operator!=(const BigInteger& rhs) const{
    return !(*this == rhs);
}
void BigInteger::absSum(const BigInteger& other){
    size_t end = std::max(digits_.size(), other.digits_.size());
    size_t a_s = digits_.size();
    size_t b_s = other.digits_.size();
    int carry = 0;
    for (size_t i = 0; i < end; ++i){
        int a = (i >= a_s ? 0 : digits_[i]);
        int b = (i >= b_s ? 0 : other.digits_[i]);
        if (i < digits_.size()){
            digits_[i] = mod(a + b + carry, 10);
        } else {
            digits_.push_back(mod(a + b + carry, 10));
        }
        carry = (a + b + carry) / 10;
    }
    if (carry) digits_.push_back(1);
}
void BigInteger::absSub(const BigInteger& other){
    size_t end = std::max(digits_.size(), other.digits_.size());
    size_t a_s = digits_.size();
    size_t b_s = other.digits_.size();
    int borrow = 0;
    for (size_t i = 0; i < end; ++i){
        int a = (i >= a_s ? 0 : digits_[i]);
        int b = (i >= b_s ? 0 : other.digits_[i]);
        if (i < digits_.size()){
            digits_[i] = mod(a - b - borrow, 10);
        } else {
            digits_.push_back(mod(a - b - borrow, 10));
        }
        borrow = (a - b - borrow) < 0;
    }
    for (int i = digits_.size() - 1; i > 0; --i){
        if (digits_[i] == 0){
            digits_.pop_back();
        } else {
            break;
        }
    }
}
void BigInteger::absMul(const BigInteger& other){
    int n = digits_.size();
    int m = other.digits_.size();
    std::vector<int> res(n + m, 0);
    for (int i = 0; i < m; ++i){
        int carry = 0;
        for (int j = 0; j < n; ++j){
            int cur = res[i + j] + digits_[j] * other.digits_[i] + carry;
            res[i + j] = mod(cur, 10);
            carry = cur / 10;
        }
        if (carry) res[i + n] += carry;
    }
    while (res.size() > 1 && res.back() == 0){
        res.pop_back();
    }
    digits_ = res;
}
void BigInteger::absDiv(const BigInteger& other){
    int n = digits_.size();
    std::vector<int> res(n, 0);
    BigInteger rem{"0"};
    for (int i = n - 1; i >= 0; --i){
        rem.digits_.insert(rem.digits_.begin(), digits_[i]);
        while (rem.digits_.size() > 1 && rem.digits_.back() == 0){
            rem.digits_.pop_back();
        }
        int q = 0;
        BigInteger temp = rem;
        while(!(temp.abs() < other.abs())){
            temp.absSub(other);
            ++q;
        }
        rem = temp;
        res[i] = q;
    }
    while (res.size() > 1 && res.back() == 0){
        res.pop_back();
    }
    digits_ = res;
}
BigInteger& BigInteger::operator+=(const BigInteger& rhs){
    if (negative_ == rhs.negative_){
        absSum(rhs);
    } else if (abs() < rhs.abs()){
        BigInteger a = rhs;
        std::swap(a, *this);
        absSub(a);
        negative_ = rhs.negative_;
    } else {
        absSub(rhs);
    }
    if (is_zero()) negative_ = false;
    return *this;
}
BigInteger& BigInteger::operator-=(const BigInteger& rhs){
    *this += (-rhs);
    if (is_zero()) negative_ = false;
    return *this;
}
BigInteger& BigInteger::operator*=(const BigInteger& rhs){
    if (is_zero() || rhs.is_zero()){
        *this = BigInteger("0");
        return *this;
    }
    absMul(rhs);
    negative_ = (negative_ != rhs.negative_);
    return *this;
}
BigInteger& BigInteger::operator/=(const BigInteger& rhs){
    absDiv(rhs);
    negative_ = (negative_ != rhs.negative_);
    if (is_zero()) negative_ = false;
    return *this;
}
BigInteger& BigInteger::operator%=(const BigInteger& rhs){
    *this = *this - (*this / rhs) * rhs;
    if (is_zero()) negative_ = false;
    return *this;
}
BigInteger BigInteger::operator+(const BigInteger& rhs) const{
    BigInteger a = *this;
    a += rhs;
    return a;
}
BigInteger BigInteger::operator-(const BigInteger& rhs) const{
    BigInteger a = *this;
    a -= rhs;
    return a;
}
BigInteger BigInteger::operator*(const BigInteger& rhs) const {
    BigInteger a = *this;
    a *= rhs;
    return a;
}
BigInteger BigInteger::operator/(const BigInteger& rhs) const {
    BigInteger a = *this;
    a /= rhs;
    return a;
}
BigInteger BigInteger::operator%(const BigInteger& rhs) const {
    BigInteger a = *this;
    a %= rhs;
    return a;
}
BigInteger& BigInteger::operator++(){
    *this += BigInteger(1);
    return *this;
}
BigInteger BigInteger::operator++(int){
    BigInteger temp = *this;
    *this += BigInteger(1);
    return temp;
}
BigInteger& BigInteger::operator--(){
    *this -= BigInteger(1);
    return *this;
}
BigInteger BigInteger::operator--(int){
    BigInteger temp = *this;
    *this -= BigInteger(1);
    return temp;
}
BigInteger BigInteger::abs() const {
    BigInteger a = *this;
    a.negative_ = 0;
    return a;
}
bool BigInteger::is_zero() const{
    return digits_.size() == 1 && digits_[0] == 0;
}
std::string BigInteger::to_string() const {
    std::string ans = "";
    if (negative_ && !is_zero()) ans += "-";
    for (int i = (int)digits_.size() - 1; i > -1; --i){
        ans += (digits_[i] + '0');
    }
    return ans;
}

BigInteger::operator bool() const {
    return (*this != BigInteger("0"));
}
