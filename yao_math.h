// common header for Yaossg's math toys

#include <type_traits>
#include <string>

#ifndef YAO_MATH
#define YAO_MATH

namespace yao_math {

// generic constexpr recursive implementation of gcd and lcm
template<typename Int> requires std::is_integral_v<Int>
constexpr Int gcd(Int x, Int y) {
	return y == 0 ? x : gcd(y, x % y);
}

template<typename Int> requires std::is_integral_v<Int>
constexpr Int lcm(Int x, Int y) {
    return x / gcd(x, y) * y;
}

// fallback implementation for toTex
template<typename T> requires std::is_fundamental_v<T>
std::string toTex(T t) {
	return std::to_string(t);
}

// generic implementation of fast power with the integral exponent
template<typename Base>
constexpr Base pow(Base a, size_t n) {
	Base r = 1;
	while(n & 1 && (r *= a, 0), n && (a *= a, 0), n >>= 1);
	return r;
}

}

#endif