// note: in this header, int[N] should be treated as a single little-endian integer

#include <array>
#include <vector>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <iostream>

#include "../yao_math.h"
#include "int_conv.cpp"

namespace yao_math {
    using byte = unsigned char;
    using uint = unsigned int;

    uint log2byte(byte b) {
        const static char table[] = 
            "-011222233333333444444444444444455555555555555555555555555555555"
            "6666666666666666666666666666666666666666666666666666666666666666"
            "7777777777777777777777777777777777777777777777777777777777777777"
            "7777777777777777777777777777777777777777777777777777777777777777"
            ;
        return table[b] - '0';
    }

    template<typename T>
    constexpr bool is_uint_v = std::is_integral_v<T> && std::is_unsigned_v<T>;
    template<typename T>
    constexpr bool is_int_v = std::is_integral_v<T> && std::is_signed_v<T>;

    template<size_t N, bool S>
    struct wide_int;
}

namespace std {
template <size_t N, bool S, size_t M, bool R>
struct common_type<yao_math::wide_int<N, S>,
                   yao_math::wide_int<M, R>> {
    using type = yao_math::wide_int<max(M, N), (S == R ? S : S ? N > M : N < M)>;
};
}

#define REQUIRES(cond) std::enable_if_t<cond, bool> = true

namespace yao_math {


template<size_t N, bool S>
struct wide_int {
    static_assert(N, "N must not be 0");
    static_assert((N & 7) == 0, "N should be a multiplier of 8");
    constexpr static bool SIGNED = S;
    constexpr static size_t BITS = N;
    constexpr static size_t BYTES = BITS >> 3;

    byte bytes[BYTES];

    wide_int() noexcept {
        std::memset(bytes, 0, BYTES);
    }

    template<typename Unsigned, REQUIRES(is_uint_v<Unsigned>)>
    wide_int(Unsigned u) noexcept : wide_int() {
        std::memcpy(bytes, &u, std::min(BYTES, sizeof u));
    }

    template<typename Signed, REQUIRES(is_int_v<Signed>)>
    wide_int(Signed s) noexcept : 
        wide_int(static_cast<std::make_unsigned_t<Signed>>(s)) {
        size_t size = sizeof s;
        if (s < 0 && BYTES > size) {
            std::memset(bytes + size, 0xFF, BYTES - size);
        }
    }

    template<size_t M, bool R>
    wide_int(wide_int<M, R> const& rhs) noexcept : wide_int() {
        std::memcpy(bytes, rhs.bytes, std::min(BYTES, rhs.BYTES));
        if (rhs.is_negative() && BYTES > rhs.BYTES) {
            std::memset(bytes + rhs.BYTES, 0xFF, BYTES - rhs.BYTES);
        }
    }

    template<typename Unsigned, REQUIRES(is_uint_v<Unsigned>)>
    wide_int& operator=(Unsigned u) noexcept {
        return *new (this) wide_int(u);
    }

    template<typename Signed, REQUIRES(is_int_v<Signed>)>
    wide_int& operator=(Signed s) noexcept {
        return *new (this) wide_int(s);
    }

    template<size_t M, bool R>
    wide_int& operator=(wide_int<M, R> const& rhs) noexcept {
        return *new (this) wide_int(rhs);
    }

    wide_int& complement() noexcept {
        for (size_t i = 0; i < BYTES; ++i) {
            bytes[i] = ~bytes[i];
        }
        return *this;
    }
    
    wide_int operator~() const noexcept {
        wide_int ret = *this;
        return ret.complement();
    }

    wide_int& negative() noexcept {
        return complement().operator++();
    }

    wide_int operator+() const noexcept {
        return *this;
    }

    wide_int operator-() const noexcept {
        wide_int ret = *this;
        return ret.negative();
    }

    bool is_negative() const noexcept {
        return SIGNED && highest_bit();
    }

    template<typename Integral, REQUIRES(std::is_integral_v<Integral>)>
    wide_int& operator+=(Integral rhs) noexcept {
        return operator+=(wide_int<sizeof rhs << 3, std::is_signed_v<Integral>>(rhs));
    }

    wide_int& operator+=(wide_int const& rhs) noexcept {
        uint carry = 0;
        for (size_t i = 0; i < BYTES; ++i) {
            carry += bytes[i] + rhs.bytes[i];
            bytes[i] = carry & 0xFF;
            carry >>= 8;
        }
        return *this;
    }

    wide_int& operator-=(wide_int const& rhs) noexcept {
        return operator+=(-rhs);
    }

    wide_int& operator++() noexcept {
        return operator+=(1);
    }

    wide_int operator++(int) noexcept {
        wide_int ret = *this;
        operator++();
        return ret;
    }

    wide_int& operator--() noexcept {
        return operator-=(1);
    }

    wide_int operator--(int) noexcept {
        wide_int ret = *this;
        operator--();
        return ret;
    }

#define MUL_DIV_OP(op, ex) \
wide_int& operator op##=(wide_int const& rhs) ex { \
    return *this = *this op rhs; \
}

MUL_DIV_OP(*, noexcept)
MUL_DIV_OP(/, )
MUL_DIV_OP(%, )

#undef MUL_DIV_OP

#define BIT_OP(op) \
wide_int& operator op##=(wide_int const& rhs) noexcept { \
    for (size_t i = 0; i < BYTES; ++i) \
        bytes[i] op##= rhs.bytes[i]; \
    return *this; \
}

BIT_OP(&)
BIT_OP(|)
BIT_OP(^)

#undef BIT_OP

    void shiftLeftBytes(size_t rhs) noexcept {
        if (!rhs) return;
        rhs = std::min(rhs, BYTES);
        memmove(bytes + rhs, bytes, BYTES - rhs);
        memset(bytes, 0, rhs);
    }

    wide_int& operator<<=(size_t rhs) noexcept {
        size_t rbyte = rhs >> 3;
        size_t rbit = rhs & 7;
        shiftLeftBytes(rbyte);
        if (!rbit || !*this) return *this;
        for (size_t i = BYTES - 1; i >= rbyte && /* i >= 0 */ ~i; --i) {
            byte rem = bytes[i] >> (8 - rbit);
            bytes[i] <<= rbit;
            if (i + 1 < BYTES) {
                bytes[i + 1] |= rem;
            }
        }
        return *this;
    }

    void shiftRightBytes(size_t rhs) noexcept {
        if (!rhs) return;
        rhs = std::min(rhs, BYTES);
        memmove(bytes, bytes + rhs, BYTES - rhs);
        memset(bytes + BYTES - rhs, is_negative() ? 0xFF : 0, rhs);
    }

    wide_int& operator>>=(size_t rhs) noexcept {
        size_t rbyte = rhs >> 3;
        size_t rbit = rhs & 7;
        shiftRightBytes(rbyte);
        if (!rbit || !*this) return *this;
        const byte mask = (1 << rbit) - 1;
        for (size_t i = 0; i < BYTES - rbyte; ++i) {
            byte rem = bytes[i] & mask;
            bytes[i] >>= rbit;
            if (i > 0) {
                bytes[i - 1] |= rem << (8 - rbit);
            }
        }
        if(is_negative()) {
            bytes[BYTES - rbyte - 1] |= mask << (8 - rbit);
        }
        return *this;
    }

#define SHIFT_OP(op) \
wide_int operator op(size_t rhs) const noexcept { \
    wide_int ret = *this; \
    return ret op##= rhs; \
}

SHIFT_OP(<<)
SHIFT_OP(>>)

#undef SHIFT_OP

    wide_int abs() const noexcept {
        if (is_negative())  return operator-();
        else                return operator+();
    }
    
    static wide_int exp2(size_t n) {
        wide_int ret;
        if (n < ret.BITS) {
            ret.bytes[n >> 3] = 1 << (n & 7);
        }
        return ret;
    }
    
    size_t log2() const {
        for (size_t i = BYTES - 1; /* i >= 0 */ ~i; --i) {
            if (bytes[i]) {
                return (i << 3) + log2byte(bytes[i]);
            }
        }
        throw std::invalid_argument("0.log2() is invalid");
    }

    struct div_t {
        wide_int quot, rem;
    };

    div_t div(wide_int rhs) const {
        if (!rhs) throw std::invalid_argument("divided by 0");
        if (!*this) return {0, 0};
        if (SIGNED) {
            if (is_negative()) {
                if (rhs.is_negative()) {
                    div_t d = operator-().div(-rhs);
                    d.rem.negative();
                    return d;
                } else {
                    div_t d = operator-().div(rhs);
                    d.quot.negative();
                    d.rem.negative();
                    return d;
                }
            } else if (rhs.is_negative()) {
                div_t d = div(-rhs);
                d.quot.negative();
                return d;
            }
        }
        size_t rbits = rhs.log2();
        wide_int rem = *this;
        wide_int qout;
        int cmp;
        while ((cmp = rem.compare(rhs)) > 0 /*rem > rhs*/) {
            size_t bits = rem.log2();
            size_t shift = bits - rbits;
            wide_int q = rhs << shift;
            if (q > rem && shift) {
                --shift;
                q >>= 1;
            }
            rem -= q;
            qout += exp2(shift);
        }
        if (cmp == 0 /*rem == rhs*/) {
            ++qout;
            rem = 0;
        }
        return {qout, rem};
    }

    explicit operator bool() const noexcept {
        for (size_t i = 0; i < BYTES; ++i) {
            if (bytes[i]) return true;
        }
        return false;
    }
    
    bool highest_bit() const noexcept {
        return bytes[BYTES - 1] >> 7;
    }

    byte to_byte() const noexcept {
        return bytes[0];
    }

    static wide_int from_byte(byte b) noexcept {
        wide_int ret;
        ret.bytes[0] = b;
        return ret;
    }

    template<typename Integral, REQUIRES(std::is_integral_v<Integral>)>
    Integral to_integral() const noexcept {
        static_assert(!std::is_same_v<Integral, bool>, "use operator bool() instead");
        Integral integral;
        memcpy(&integral, bytes, std::min(BYTES, sizeof integral));
        return integral;
    }

    wide_int<BITS, false> to_unsigned() const noexcept {
        return operator+();
    }

    wide_int<BITS, true> to_signed() const noexcept {
        return operator+();
    }

    std::string to_string(uint base = 10, bool uppercase = false) const {
		int_conv::base::assertValid(base);
        std::string buf;
        div_t d = {abs(), 0};
        do {
            d = d.quot.div(base);
            buf += int_conv::base::to_char(d.rem.to_byte(), base, uppercase);
        } while (d.quot);
        if (is_negative()) buf += '-';
        return {buf.rbegin(), buf.rend()};
    }

    static wide_int from_string(std::string s, uint base = 10) {
		int_conv::base::assertValid(base);
        if (s.length() == 0) throw std::invalid_argument("invalid string");
        if (s[0] == '+') s = s.substr(1);
        if (s[0] == '-') return from_string(s.substr(1), base).negative();
        wide_int ret;
        for (char ch : s) {
            int num = int_conv::base::from_char(ch, base);
            if (num < 0) throw std::invalid_argument("invalid string");
            ret *= base;
            ret += (uint) num;
        }
        return ret;
    }

    friend std::ostream& operator<<(std::ostream& os, wide_int rhs) noexcept {
        int base = 10;
        std::ios::fmtflags f = os.flags();
        if (f & std::ios::oct) base = 8;
        if (f & std::ios::dec) base = 10;
        if (f & std::ios::hex) base = 16;
        bool sb = false;
        if (f & std::ios::showbase) sb = true; 
        if (base == 8) {
            if (sb && rhs) os << '0'; 
            return os << rhs.to_unsigned().to_string(base);
        }
        if (base == 16) {
            if (sb) os << "0x";
            return os << rhs.to_unsigned().to_string(base, f & std::ios::uppercase);
        }
        if ((f & std::ios::showpos) && !rhs.is_negative()) os << '+';
        return os << rhs.to_string(base);
    }

    friend std::istream& operator>>(std::istream& is, wide_int& rhs) noexcept {
        int base = 10;
        std::ios::fmtflags f = is.flags() & std::ios::basefield;
        if (f & std::ios::oct) base = 8;
        if (f & std::ios::dec) base = 10;
        if (f & std::ios::hex) base = 16;
        bool neg = false;
        wide_int ret;
        if (is) {
            char ch = is.get();
            switch (ch) {
                case '-': neg = true;
                case '+': break;
                default: is.unget();
            }
        }
        if (is && base == 16) {
            char ch1 = is.get();
            if (is && ch1 == '0') {
                char ch2 = is.get();
                if (!(ch2 == 'x' || ch2 == 'X')) {
                    is.unget();
                }
            } else {
                is.unget();
            }
        }
        size_t read = 0;
        while (is) {
            char ch = is.get();
            int num = int_conv::base::from_char(ch, base);
            if (num < 0) break;
            ++read;
            ret *= base;
            ret += (uint) num;
        }
        if (read) {
            if (neg) ret.negative();
            rhs = ret;    
        } else {
            is.setstate(std::ios::failbit);
        }
        return is;
    }

    int compare(wide_int const& rhs) const noexcept {
        if (int cmp = is_negative() - rhs.is_negative()) return -cmp;
        for (size_t i = BYTES - 1; /* i >= 0 */ ~i; --i) {
            if (int cmp = bytes[i] - rhs.bytes[i]) return cmp;
        }
        return 0;
    }
};

#define BINARY_OP(op) \
template<size_t N, bool S, size_t M, bool R> \
std::common_type_t<wide_int<N, S>, wide_int<M, R>> \
    operator op(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept { \
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs; \
    return ret op##= rhs; \
}

BINARY_OP(+)
BINARY_OP(-)
BINARY_OP(&)
BINARY_OP(|)
BINARY_OP(^)

#undef BINARY_OP

template<size_t N, bool S, size_t M, bool R>
std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator*(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret;
    for (size_t i = 0; i < lhs.BYTES; ++i) {
        uint carry = 0;
        for (size_t j = 0; j < rhs.BYTES && (i + j) < ret.BYTES; ++j) {
            carry += ret.bytes[i + j] + lhs.bytes[i] * rhs.bytes[j];
            ret.bytes[i + j] = carry & 0xFF;
            carry >>= 8;
        }
    }
    return ret;
}

template<size_t N, bool S, size_t M, bool R>
typename std::common_type_t<wide_int<N, S>, wide_int<M, R>>::div_t
    div(wide_int<N, S> lhs, wide_int<M, R> rhs) {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs;
    return ret.div(rhs);
}

template<size_t N, bool S, size_t M, bool R>
std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator/(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) {
    return div(lhs, rhs).quot;
}

template<size_t N, bool S, size_t M, bool R>
std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator%(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) {
    return div(lhs, rhs).rem;
}

template<size_t N, bool S, size_t M, bool R>
int compare(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs;
    return ret.compare(rhs);
}

#define COMPARE_OP(op) \
template<size_t N, bool S, size_t M, bool R> \
bool operator op(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept { \
    return compare(lhs, rhs) op 0; \
}

COMPARE_OP(==)
COMPARE_OP(!=)
COMPARE_OP(<)
COMPARE_OP(>)
COMPARE_OP(<=)
COMPARE_OP(>=)

#undef COMPARE_OP

#define ALIAS_SIGNED(N) \
using int##N = wide_int<N, true>; \
inline namespace literals { \
inline namespace wide_int_literals { \
int##N operator"" _int##N(const char* s) { \
    return int##N::from_string(s); \
} \
} \
} 

ALIAS_SIGNED(128)
ALIAS_SIGNED(256)
ALIAS_SIGNED(512)
ALIAS_SIGNED(1024)
ALIAS_SIGNED(2048)
ALIAS_SIGNED(4096)

#undef ALIAS_SIGNED

#define ALIAS_UNSIGNED(N) \
using uint##N = wide_int<N, false>; \
inline namespace literals { \
inline namespace wide_int_literals { \
uint##N operator"" _uint##N(const char* s) { \
    return uint##N::from_string(s); \
} \
} \
} 

ALIAS_UNSIGNED(128)
ALIAS_UNSIGNED(256)
ALIAS_UNSIGNED(512)
ALIAS_UNSIGNED(1024)
ALIAS_UNSIGNED(2048)
ALIAS_UNSIGNED(4096)

#undef ALIAS_UNSIGNED

}

#undef REQUIRES

#include <iostream>
#include <iomanip>

int main() {
    using namespace std;
    using namespace yao_math;
    using yao_math::byte;
    using namespace yao_math::wide_int_literals;
    cout << hex << (111111111111111111111111111111111111111111111111111111111111111111111_uint256) << endl;
    cout << hex << uppercase << wide_int<128, true>((int)0x8000'0000) << endl;
    int128(false);
}