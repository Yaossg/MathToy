#include <string>
#include <stdexcept>
#include <cstdint>
#include <cctype>

namespace yao_math::int_conv {
	namespace base {
		constexpr int min = 2, bin = 2, oct = 8, dec = 10, hex = 16, max = 36;
		constexpr bool isInvalid(int base) { 
			return base < min || base > max; 
		}
		void assertValid(int base) {
			if (isInvalid(base)) throw std::invalid_argument("invalid base");
		}
		int from_char(char ch, int base = 10) {
			assertValid(base);
			int num;
			if (isdigit(ch)) num = ch - '0';
			else if (isupper(ch)) num = ch - 'A';
			else if (islower(ch)) num = ch - 'a';
			else return -1;
			if (num < base) return num;
			else return -1;
		}
		char to_char(int num, int base = 10) {
			assertValid(base);
			if (num < 0 || num >= base) throw std::invalid_argument("invalid num");
			if (num < 10) return num + '0';
			else return num + 'A' - 10;
		}
	}

	template<typename UIntT, std::enable_if_t<std::is_unsigned_v<UIntT>, bool> = true>
	void to_string(char* cp, UIntT val, int base = 10) {
		if (cp == nullptr) throw std::invalid_argument("nullptr");
		base::assertValid(base);
		if (val == 0) *cp = '0';
		while (val > 0) {
			*cp++ = base::to_char(val % base, base);
			val /= base;
		}
	}
		
	template<typename UIntT, std::enable_if_t<std::is_unsigned_v<UIntT>, bool> = true>
	UIntT from_string(const char* cp, int base = 10) {
		if (cp == nullptr) throw std::invalid_argument("nullptr");
		base::assertValid(base);
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