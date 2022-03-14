#include <stdexcept>
#include <cstdint>
#include <cctype>

namespace yao_math::int_base {
	constexpr int min = 2, bin = 2, oct = 8, dec = 10, hex = 16, max = 36;
	constexpr bool isInvalid(int base) { 
		return base < min || base > max; 
	}
	void assertValid(int base) {
		if (isInvalid(base)) throw std::invalid_argument("invalid base");
	}
	int from_char_raw(char ch, int base = 10) {
		int num;
		if (isdigit(ch)) num = ch - '0';
		else if (isupper(ch)) num = ch - 'A' + 10;
		else if (islower(ch)) num = ch - 'a' + 10;
		else return -1;
		if (num < base) return num;
		else return -1;
	}
	int from_char(char ch, int base = 10) {
		assertValid(base);
		int ret = from_char_raw(ch, base);
		if (ret < 0) throw std::invalid_argument("invalid num");
		return ret;
	}
	char to_char_raw(int num, int base = 10, bool uppercase = false) {
		if (num < 10) return num + '0';
		else return num - 10 + (uppercase ? 'A' : 'a');
	}
	char to_char(int num, int base = 10, bool uppercase = false) {
		assertValid(base);
		if (num < 0 || num >= base) throw std::invalid_argument("invalid num");
		return to_char_raw(num, base, uppercase);
	}
}