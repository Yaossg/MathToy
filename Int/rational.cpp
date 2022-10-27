#include <stdexcept>
#include <cmath>
#include <compare>
#include "../yao_math.h"

namespace yao_math {
class BadRational : public std::domain_error {
public:
    explicit BadRational() : std::domain_error("bad rational: zero denominator") {}
};

template<typename IntType>
class Rational {
    IntType num, den;
public:
    typedef IntType int_type;
    constexpr Rational(): num(0), den(1) {}
    constexpr Rational(IntType n): num(n), den(1) {}
    constexpr Rational(IntType n, IntType d): num(n), den(d) { normalize(); }

    constexpr Rational(Rational const&) = default;
    constexpr Rational(Rational&&) = default;

    constexpr void numerator(IntType n) {
        assgin(n, den);
    }

    constexpr IntType numerator()const {
        return num;
    }

    constexpr void denominator(IntType d) {
        assgin(num, d);
    }

    constexpr IntType denominator() const {
        return den;
    }

    constexpr void assign(IntType n) {
        num = n;
        den = 1;
    }

    constexpr void assign(IntType n, IntType d) {
        num = n;
        den = d;
        normalize();
    }

    constexpr Rational<IntType> inverse()const {
        if (den == IntType(0)) throw BadRational();
        return {den, num};
    }
    
    constexpr Rational<IntType> operator+()const {
        return *this;
    }

    constexpr Rational<IntType> operator-()const {
        return {-num, den};
    }

    friend constexpr Rational<IntType> operator+(Rational<IntType> const& x, 
            Rational<IntType> const& y) {
        return {x.num * lcm(x.den, y.den) / x.den + y.num * lcm(x.den, y.den) / y.den, lcm(x.den, y.den)};
    }

    friend constexpr Rational<IntType> operator-(Rational<IntType> const& x, 
            Rational<IntType> const& y) {
        return x + -y;
    }

    friend constexpr Rational<IntType> operator*(Rational<IntType> const& x, 
            Rational<IntType> const& y) {
        return {x.num * y.num, x.den * y.den};
    }

    friend constexpr Rational<IntType> operator/(Rational<IntType> const& x, 
            Rational<IntType> const& y) {
        return x * y.inverse();
    }

    constexpr Rational<IntType> operator+=(Rational<IntType> const& y) {
        return *this = *this + y;
    }

    constexpr Rational<IntType> operator-=(Rational<IntType> const& y) {
        return *this = *this - y;
    }

    constexpr Rational<IntType> operator*=(Rational<IntType> const& y) {
        return *this = *this * y;
    }

    constexpr Rational<IntType> operator/=(Rational<IntType> const& y) {
        return *this = *this / y;
    }
    
    friend constexpr bool operator==(Rational<IntType> const& x, 
                                     Rational<IntType> const& y) = default;
    
    friend constexpr std::strong_ordering operator<=>(Rational<IntType> const& x, 
                                                      Rational<IntType> const& y) {
        return x.num * lcm(x.den, y.den) / x.den <=> y.num * lcm(x.den, y.den) / y.den;
    }
    
    friend constexpr Rational<IntType> abs(Rational<IntType> const& x) {
        return {std::abs(x.num), x.den};
    }

    constexpr void normalize() {
        IntType zero(0);
        if (den == zero)
            throw BadRational();
        if (num == zero) {
            den = IntType(1);
            return;
        }
        IntType g = gcd(num, den);
        num /= g;
        den /= g;
        if (den < zero) {
            num = -num;
            den = -den;
        }
    }
};

}
