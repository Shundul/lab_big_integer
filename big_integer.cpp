#include "big_integer.h"
#include <algorithm>
#include <stdexcept>
#include <climits>



BigInteger::BigInteger() : digits_{0}, negative_(false) {}

BigInteger::BigInteger(int value) : BigInteger((long long)value) {}

BigInteger::BigInteger(long long value) {
    negative_ = (value < 0);
    unsigned long long uval;
    if (value == LLONG_MIN)
        uval = (unsigned long long)LLONG_MAX + 1ULL;
    else if (negative_)
        uval = (unsigned long long)(-value);
    else
        uval = (unsigned long long)value;

    if (uval == 0) {
        digits_.push_back(0);
    } else {
        while (uval > 0) {
            digits_.push_back((int)(uval % BASE));
            uval /= BASE;
        }
    }
}

BigInteger::BigInteger(const std::string& str) {
    if (str.empty()) { digits_.push_back(0); negative_ = false; return; }
    size_t start = 0;
    negative_ = false;
    if (str[0] == '-') { negative_ = true; start = 1; }
    else if (str[0] == '+') { start = 1; }

    
    while (start < str.size() - 1 && str[start] == '0') ++start;

    
    int i = (int)str.size();
    while (i > (int)start) {
        int from = std::max((int)start, i - CHUNK);
        int val = std::stoi(str.substr(from, i - from));
        digits_.push_back(val);
        i = from;
    }
    if (digits_.empty()) digits_.push_back(0);
    trim();
    if (is_zero()) negative_ = false;
}



void BigInteger::trim() {
    while (digits_.size() > 1 && digits_.back() == 0)
        digits_.pop_back();
}

int BigInteger::cmp_abs(const BigInteger& a, const BigInteger& b) {
    if (a.digits_.size() != b.digits_.size())
        return a.digits_.size() < b.digits_.size() ? -1 : 1;
    for (int i = (int)a.digits_.size() - 1; i >= 0; --i) {
        if (a.digits_[i] != b.digits_[i])
            return a.digits_[i] < b.digits_[i] ? -1 : 1;
    }
    return 0;
}

BigInteger BigInteger::add_abs(const BigInteger& a, const BigInteger& b) {
    BigInteger res;
    res.digits_.clear();
    int carry = 0;
    for (size_t i = 0; i < std::max(a.digits_.size(), b.digits_.size()) || carry; ++i) {
        long long cur = carry;
        if (i < a.digits_.size()) cur += a.digits_[i];
        if (i < b.digits_.size()) cur += b.digits_[i];
        res.digits_.push_back((int)(cur % BASE));
        carry = (int)(cur / BASE);
    }
    return res;
}

BigInteger BigInteger::sub_abs(const BigInteger& a, const BigInteger& b) {
    BigInteger res;
    res.digits_.clear();
    int borrow = 0;
    for (size_t i = 0; i < a.digits_.size(); ++i) {
        long long cur = (long long)a.digits_[i] - borrow;
        if (i < b.digits_.size()) cur -= b.digits_[i];
        if (cur < 0) { cur += BASE; borrow = 1; }
        else borrow = 0;
        res.digits_.push_back((int)cur);
    }
    res.trim();
    return res;
}

std::pair<BigInteger, BigInteger> BigInteger::divmod(const BigInteger& a, const BigInteger& b) {
    if (b.is_zero()) throw std::domain_error("Division by zero");

    bool q_neg = a.negative_ != b.negative_;
    bool r_neg = a.negative_;

    BigInteger dividend = a; dividend.negative_ = false;
    BigInteger divisor  = b; divisor.negative_  = false;

    if (cmp_abs(dividend, divisor) < 0) {
        BigInteger r = dividend;
        r.negative_ = r_neg && !r.is_zero();
        return {BigInteger(0), r};
    }

    BigInteger quotient;
    quotient.digits_.assign(dividend.digits_.size(), 0);
    BigInteger cur;

    for (int i = (int)dividend.digits_.size() - 1; i >= 0; --i) {
        cur.digits_.insert(cur.digits_.begin(), dividend.digits_[i]);
        cur.trim();
        int lo = 0, hi = BASE - 1, q = 0;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            BigInteger t = divisor * BigInteger(mid);
            if (cmp_abs(t, cur) <= 0) { q = mid; lo = mid + 1; }
            else hi = mid - 1;
        }
        quotient.digits_[i] = q;
        cur = sub_abs(cur, divisor * BigInteger(q));
    }

    quotient.trim();
    quotient.negative_ = q_neg && !quotient.is_zero();
    cur.negative_ = r_neg && !cur.is_zero();
    return {quotient, cur};
}

BigInteger BigInteger::operator+(const BigInteger& rhs) const {
    if (negative_ == rhs.negative_) {
        BigInteger res = add_abs(*this, rhs);
        res.negative_ = negative_;
        if (res.is_zero()) res.negative_ = false;
        return res;
    }
    int cmp = cmp_abs(*this, rhs);
    if (cmp == 0) return BigInteger(0);
    BigInteger res;
    if (cmp > 0) {
        res = sub_abs(*this, rhs);
        res.negative_ = negative_;
    } else {
        res = sub_abs(rhs, *this);
        res.negative_ = rhs.negative_;
    }
    if (res.is_zero()) res.negative_ = false;
    return res;
}

BigInteger BigInteger::operator-(const BigInteger& rhs) const {
    BigInteger neg_rhs = rhs;
    if (!neg_rhs.is_zero()) neg_rhs.negative_ = !rhs.negative_;
    return *this + neg_rhs;
}

BigInteger BigInteger::operator*(const BigInteger& rhs) const {
    BigInteger res;
    res.digits_.assign(digits_.size() + rhs.digits_.size(), 0);
    for (size_t i = 0; i < digits_.size(); ++i) {
        long long carry = 0;
        for (size_t j = 0; j < rhs.digits_.size() || carry; ++j) {
            long long cur = (long long)res.digits_[i + j] + carry;
            if (j < rhs.digits_.size())
                cur += (long long)digits_[i] * rhs.digits_[j];
            res.digits_[i + j] = (int)(cur % BASE);
            carry = cur / BASE;
        }
    }
    res.trim();
    res.negative_ = negative_ != rhs.negative_;
    if (res.is_zero()) res.negative_ = false;
    return res;
}

BigInteger BigInteger::operator/(const BigInteger& rhs) const {
    auto [q, r] = divmod(*this, rhs);
    return q;
}

BigInteger BigInteger::operator%(const BigInteger& rhs) const {
    auto [q, r] = divmod(*this, rhs);
    return r;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs) { return *this = *this + rhs; }
BigInteger& BigInteger::operator-=(const BigInteger& rhs) { return *this = *this - rhs; }
BigInteger& BigInteger::operator*=(const BigInteger& rhs) { return *this = *this * rhs; }
BigInteger& BigInteger::operator/=(const BigInteger& rhs) { return *this = *this / rhs; }
BigInteger& BigInteger::operator%=(const BigInteger& rhs) { return *this = *this % rhs; }


BigInteger BigInteger::operator-() const {
    BigInteger res = *this;
    if (!res.is_zero()) res.negative_ = !res.negative_;
    return res;
}

BigInteger& BigInteger::operator++() { *this += BigInteger(1); return *this; }
BigInteger  BigInteger::operator++(int) { BigInteger tmp = *this; ++(*this); return tmp; }
BigInteger& BigInteger::operator--() { *this -= BigInteger(1); return *this; }
BigInteger  BigInteger::operator--(int) { BigInteger tmp = *this; --(*this); return tmp; }


bool BigInteger::operator==(const BigInteger& rhs) const {
    return negative_ == rhs.negative_ && digits_ == rhs.digits_;
}
bool BigInteger::operator!=(const BigInteger& rhs) const { return !(*this == rhs); }
bool BigInteger::operator<(const BigInteger& rhs) const {
    if (negative_ != rhs.negative_) return negative_;
    if (negative_) return cmp_abs(*this, rhs) > 0;
    return cmp_abs(*this, rhs) < 0;
}
bool BigInteger::operator>(const BigInteger& rhs) const  { return rhs < *this; }
bool BigInteger::operator<=(const BigInteger& rhs) const { return !(rhs < *this); }
bool BigInteger::operator>=(const BigInteger& rhs) const { return !(*this < rhs); }

std::string BigInteger::to_string() const {
    std::string res;
    if (negative_) res += '-';
    res += std::to_string(digits_.back());
    for (int i = (int)digits_.size() - 2; i >= 0; --i) {
        std::string chunk = std::to_string(digits_[i]);
        res += std::string(CHUNK - (int)chunk.size(), '0') + chunk;
    }
    return res;
}

bool BigInteger::is_zero() const { return digits_.size() == 1 && digits_[0] == 0; }
bool BigInteger::is_negative() const { return negative_; }
BigInteger::operator bool() const { return !is_zero(); }

std::ostream& operator<<(std::ostream& os, const BigInteger& value) {
    return os << value.to_string();
}

std::istream& operator>>(std::istream& is, BigInteger& value) {
    std::string s;
    is >> s;
    value = BigInteger(s);
    return is;
}