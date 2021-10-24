#include <type_traits>
#include <string>

#ifndef YAO_MATH
#define YAO_MATH

namespace yao_math {

template<typename Int>
constexpr Int gcd(Int x, Int y) {
	return y == 0 ? x : gcd(y, x % y);
}
    
template<typename T>
std::enable_if_t<std::is_fundamental_v<T>, std::string> toTex(T t) {
	return std::to_string(t);
}


template<typename Base, typename Exp>
std::enable_if_t<std::is_integral_v<Exp>, Base> pow(Base a, Exp n) {
	Base r = 1;
	while(n & 1 && (r *= a, 0), n && (a *= a, 0), n >>= 1);
	return r;
}

}

#endif