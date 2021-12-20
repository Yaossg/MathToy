#include <string>
#include <stdexcept>
#include <cstdint>
#include <cctype>

namespace yao_math::int_conv {
	namespace base {
		constexpr int min = 2, bin = 2, oct = 8, dec = 10, hex = 16, max = 36;
		constexpr bool check(int base) 
		{ return base < min || base > max; }
		int from_char(char ch, int base = 10) {
			if (check(base)) throw std::invalid_argument("invalid base");
			int num;
			if (isdigit(ch)) num = ch - '0';
			else if (isupper(ch)) num = ch - 'A';
			else if (islower(ch)) num = ch - 'a';
			else return -1;
			if (num < base) return num;
			else return -1;
		}
		char to_char(int num, int base = 10) {
			if (check(base)) throw std::invalid_argument("invalid base");
			if (num < 0 || num >= base) throw std::invalid_argument("invalid num");
			if (num < 10) return num + '0';
			else return num + 'A' - 10;
		}
	}
	template<typename UIntT, std::enable_if_t<std::is_unsigned_v<UIntT>, bool> = true>
	std::string uint_to_str(UIntT integer, int base = 10) {
		if (base::check(base)) throw std::invalid_argument("invalid integer base");
		if (integer == 0) return "0";
		std::string ret;
		while (integer > 0) {
			ret += base::to_char(integer % base, base);
			integer /= base;
		}
		return ret;
	}
		
	template<typename UIntT, std::enable_if_t<std::is_unsigned_v<UIntT>, bool> = true>
	UIntT str_to_uint(char const * cp, int base = 10) {
		if (base::check(base)) throw std::invalid_argument("invalid integer base");
		if (*cp == 0) throw std::invalid_argument("invalid string");
		UIntT ret = 0;
		while (char ch = *cp++) {
			int num = base::from_char(ch, base);
			if (num < 0) throw std::invalid_argument("invalid string");
			ret *= base;
			ret += num;
		}
		return ret;
	}
}