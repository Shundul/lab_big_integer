#pragma once
#include <iostream>
#include <string>
#include <vector>

class BigInteger {
public:
    BigInteger();
    BigInteger(int value);
    BigInteger(long long value);
    explicit BigInteger(const std::string& str);

    BigInteger(const BigInteger& other)            = default;
    BigInteger& operator=(const BigInteger& other) = default;

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

    BigInteger  operator-() const;
    BigInteger& operator++();
    BigInteger  operator++(int);
    BigInteger& operator--();
    BigInteger  operator--(int);

    bool operator==(const BigInteger& rhs) const;
    bool operator!=(const BigInteger& rhs) const;
    bool operator< (const BigInteger& rhs) const;
    bool operator> (const BigInteger& rhs) const;
    bool operator<=(const BigInteger& rhs) const;
    bool operator>=(const BigInteger& rhs) const;

    std::string to_string() const;
    bool is_zero() const;
    bool is_negative() const;
    explicit operator bool() const;

    friend std::ostream& operator<<(std::ostream& os, const BigInteger& value);
    friend std::istream& operator>>(std::istream& is, BigInteger& value);

private:
    static const int BASE  = 1'000'000'000;
    static const int CHUNK = 9;

    std::vector<int> digits_;
    bool negative_ = false;

    void trim();
    static int cmp_abs(const BigInteger& a, const BigInteger& b);
    static BigInteger add_abs(const BigInteger& a, const BigInteger& b);
    static BigInteger sub_abs(const BigInteger& a, const BigInteger& b);
    static std::pair<BigInteger, BigInteger> divmod(const BigInteger& a, const BigInteger& b);
};