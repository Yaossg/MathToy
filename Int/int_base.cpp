#include <stdexcept>
#include <cstdint>

namespace yao_math::int_base {
	constexpr int min = 2, bin = 2, oct = 8, dec = 10, hex = 16, max = 36;
	constexpr bool isInvalid(int base) { 
		return base < min || base > max; 
	}
	constexpr void assertValid(int base) {
		if (isInvalid(base)) throw std::invalid_argument("invalid base");
	}
	constexpr int from_char_raw(char ch, int base = 10) {
		int num;
		if ('0' <= ch && ch <= '9') num = ch - '0';
		else if ('A' <= ch && ch <= 'Z') num = ch - 'A' + 10;
		else if ('a' <= ch && ch <= 'z') num = ch - 'a' + 10;
		else return -1;
		if (num < base) return num;
		else return -1;
	}
	constexpr int from_char(char ch, int base = 10) {
		assertValid(base);
		int ret = from_char_raw(ch, base);
		if (ret < 0) throw std::invalid_argument("invalid num");
		return ret;
	}
	constexpr char to_char_raw(int num, int base = 10, bool uppercase = false) {
		if (num < 10) return num + '0';
		else return num - 10 + (uppercase ? 'A' : 'a');
	}
	constexpr char to_char(int num, int base = 10, bool uppercase = false) {
		assertValid(base);
		if (num < 0 || num >= base) throw std::invalid_argument("invalid num");
		return to_char_raw(num, base, uppercase);
	}
}