#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <random>
#include <bit>
#include <array>

#include "../yao_math.h"
#include "int_base.cpp"
#include "fpbits.cpp"

namespace yao_math {
    using byte = unsigned char;

    constexpr size_t log2byte(byte b) {
        if (b == 0) throw std::invalid_argument("log2byte(0) is invalid");
        else return
            " 011222233333333444444444444444455555555555555555555555555555555"
            "6666666666666666666666666666666666666666666666666666666666666666"
            "7777777777777777777777777777777777777777777777777777777777777777"
            "7777777777777777777777777777777777777777777777777777777777777777"
            [b] - '0';
    }

    constexpr void memset(byte *dst, byte val, size_t size) noexcept {
        if constexpr (std::is_constant_evaluated()) {
            for (size_t i = 0; i < size; ++i) dst[i] = val;
        } else {
            std::memset(dst, val, size);
        }
    }

    constexpr void memcpy(byte *dst, const byte *src, size_t size) noexcept {
        if constexpr (std::is_constant_evaluated()) {
            for (size_t i = 0; i < size; ++i) dst[i] = src[i];
        } else {
            std::memcpy(dst, src, size);
        }
    }

    template<typename T>
    constexpr bool is_sint_v = std::is_integral_v<T> && std::is_signed_v<T>;
    template<typename T>
    constexpr bool is_uint_v = std::is_integral_v<T> && std::is_unsigned_v<T>;

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

namespace yao_math {

template<size_t N, bool S>
struct wide_int {
    static_assert(N, "N must not be 0");
    static_assert((N & 7) == 0, "N should be a multiplier of 8");
    constexpr static bool SIGNED = S;
    constexpr static size_t BITS = N;
    constexpr static size_t BYTES = BITS >> 3;

    byte bytes[BYTES];

    constexpr wide_int() noexcept {
        memset(bytes, 0, BYTES);
    }

    template<typename Unsigned> requires is_uint_v<Unsigned>
    constexpr wide_int(Unsigned u) noexcept : wide_int() {
        auto buf = std::bit_cast<std::array<byte, std::min(BYTES, sizeof u)>>(u);
        memcpy(bytes, buf.data(), buf.size());
    }

    template<typename Signed> requires is_sint_v<Signed>
    constexpr wide_int(Signed s) noexcept : 
        wide_int(static_cast<std::make_unsigned_t<Signed>>(s)) {
        size_t size = sizeof s;
        if (s < 0 && BYTES > size) {
            memset(bytes + size, 0xFF, BYTES - size);
        }
    }

    template<size_t M, bool R>
    constexpr wide_int(wide_int<M, R> const& rhs) noexcept : wide_int() {
        memcpy(bytes, rhs.bytes, std::min(BYTES, rhs.BYTES));
        if (rhs.is_negative() && BYTES > rhs.BYTES) {
            memset(bytes + rhs.BYTES, 0xFF, BYTES - rhs.BYTES);
        }
    }

    template<typename Unsigned> requires is_uint_v<Unsigned>
    constexpr wide_int& operator=(Unsigned u) noexcept {
        return *new (this) wide_int(u);
    }

    template<typename Signed> requires is_sint_v<Signed>
    constexpr wide_int& operator=(Signed s) noexcept {
        return *new (this) wide_int(s);
    }

    template<size_t M, bool R>
    constexpr wide_int& operator=(wide_int<M, R> const& rhs) noexcept {
        return *new (this) wide_int(rhs);
    }

    constexpr wide_int& complement() noexcept {
        for (size_t i = 0; i < BYTES; ++i) {
            bytes[i] = ~bytes[i];
        }
        return *this;
    }
    
    constexpr wide_int operator~() const noexcept {
        wide_int ret = *this;
        return ret.complement();
    }

    constexpr wide_int& negative() noexcept {
        return complement().operator++();
    }

    constexpr wide_int operator+() const noexcept {
        return *this;
    }

    constexpr wide_int operator-() const noexcept {
        wide_int ret = *this;
        return ret.negative();
    }

    constexpr bool is_negative() const noexcept {
        return SIGNED && highest_bit();
    }

    constexpr wide_int& operator+=(wide_int const& rhs) noexcept {
        unsigned carry = 0;
        for (size_t i = 0; i < BYTES; ++i) {
            carry += bytes[i] + rhs.bytes[i];
            bytes[i] = carry & 0xFF;
            carry >>= 8;
        }
        return *this;
    }

    constexpr wide_int& operator-=(wide_int const& rhs) noexcept {
        return operator+=(-rhs);
    }

    constexpr wide_int& operator++() noexcept {
        return operator+=(1);
    }

    constexpr wide_int operator++(int) noexcept {
        wide_int ret = *this;
        operator++();
        return ret;
    }

    constexpr wide_int& operator--() noexcept {
        return operator-=(1);
    }

    constexpr wide_int operator--(int) noexcept {
        wide_int ret = *this;
        operator--();
        return ret;
    }

    constexpr wide_int& operator *=(wide_int const& rhs) noexcept {
        return *this = *this * rhs;
    }

    constexpr wide_int& operator /=(wide_int const& rhs) {
        return *this = div(rhs).qout;
    }
    
    constexpr wide_int& operator %=(wide_int const& rhs) {
        return *this = div(rhs).rem;
    }

#define BIT_OP(op) \
constexpr wide_int& operator op##=(wide_int const& rhs) noexcept { \
    for (size_t i = 0; i < BYTES; ++i) \
        bytes[i] op##= rhs.bytes[i]; \
    return *this; \
}

BIT_OP(&)
BIT_OP(|)
BIT_OP(^)

#undef BIT_OP

    constexpr wide_int& shiftLeftBytes(size_t rhs) noexcept {
        if (rhs) {
            rhs = std::min(rhs, BYTES);
            memmove(bytes + rhs, bytes, BYTES - rhs);
            memset(bytes, 0, rhs);
        }
        return *this;
    }

    constexpr wide_int& operator<<=(size_t rhs) noexcept {
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

    constexpr wide_int& shiftRightBytes(size_t rhs) noexcept {
        if (rhs) {
            rhs = std::min(rhs, BYTES);
            memmove(bytes, bytes + rhs, BYTES - rhs);
            memset(bytes + BYTES - rhs, is_negative() ? 0xFF : 0, rhs);
        }
        return *this;
    }

    constexpr wide_int& operator>>=(size_t rhs) noexcept {
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

    constexpr wide_int& shift(int rhs) noexcept {
        if (rhs) {
            if (rhs > 0) operator<<=(rhs);
            if (rhs < 0) operator>>=(-rhs);
        }
        return *this;
    }

#define SHIFT_OP(op) \
constexpr wide_int operator op(size_t rhs) const noexcept { \
    wide_int ret = *this; \
    return ret op##= rhs; \
}

SHIFT_OP(<<)
SHIFT_OP(>>)

#undef SHIFT_OP

    constexpr wide_int abs() const noexcept {
        if (is_negative())  return operator-();
        else                return operator+();
    }
    
    static constexpr wide_int exp2(size_t n) {
        wide_int ret;
        if (n < ret.BITS) {
            ret.bytes[n >> 3] = 1 << (n & 7);
        }
        return ret;
    }
    
    constexpr size_t log2() const {
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

    constexpr div_t div(wide_int rhs) const {
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

    explicit constexpr operator bool() const noexcept {
        for (size_t i = 0; i < BYTES; ++i) {
            if (bytes[i]) return true;
        }
        return false;
    }
    
    constexpr bool highest_bit() const noexcept {
        return bytes[BYTES - 1] >> 7;
    }

    constexpr byte to_byte() const noexcept {
        return bytes[0];
    }

    static constexpr wide_int from_byte(byte b) noexcept {
        wide_int ret;
        ret.bytes[0] = b;
        return ret;
    }

    template<typename Integral> requires std::is_integral_v<Integral>
    constexpr Integral to_integral() const noexcept {
        static_assert(!std::is_same_v<Integral, bool>, "use operator bool() instead");
        std::array<byte, std::min(BYTES, sizeof(Integral))> buf;
        memcpy(buf.data(), bytes, buf.size());
        return std::bit_cast<Integral>(buf);
    }

    template<typename FP> requires std::is_floating_point_v<FP>
    static constexpr wide_int from_float(FP fp) noexcept {
        FPBits<FP> fpbits(fp);
        wide_int ret = fpbits.full_fraction();
        if (ret) {
            ret.shift(fpbits.partial_log2());
            if (fpbits.sign) ret.negative();
        }
        return ret;
    }

    constexpr wide_int<BITS, false> to_unsigned() const noexcept {
        return operator+();
    }

    constexpr wide_int<BITS, true> to_signed() const noexcept {
        return operator+();
    }

    std::string to_string(int base = 10, bool uppercase = false) const {
		int_base::assertValid(base);
        std::string buf;
        div_t d = {abs(), 0};
        do {
            d = d.quot.div(base);
            buf += int_base::to_char_raw(d.rem.to_byte(), base, uppercase);
        } while (d.quot);
        if (is_negative()) buf += '-';
        return {buf.rbegin(), buf.rend()};
    }

    friend std::string toTex(wide_int w) {
        return w.to_string();
    }

    template<typename FP> requires std::is_floating_point_v<FP>
    constexpr FP to_float() const noexcept {
        FPBits<FP> fpbits;
        int exp = log2();
        fpbits.exp2(exp);
        fpbits.fraction = operator+().shift(-exp + fpbits.FRACTION).template to_integral<bitfield>();
        fpbits.sign = is_negative();
        return fpbits;
    }

    template<typename G>
    static wide_int random(G& g, wide_int upper) {
        int cmp = upper.compare(0);
        if (cmp < 0) throw std::invalid_argument("negative upper");
        if (cmp == 0) return 0;
        wide_int ret;
        size_t bits = upper.log2();
        size_t bytes = bits >> 3;
        size_t rbits = (bits & 7) + 1;
        size_t mask = (1 << rbits) - 1;
        using dist = std::uniform_int_distribution<byte>;
        dist bd{0, 255};
        dist md{0, (byte)mask};
        do {
            for (size_t i = 0; i < bytes; ++i) {
                ret.bytes[i] = bd(g);
            }
            if (rbits) {
                ret.bytes[bytes] = md(g);
            }
        } while (ret > upper);
        return ret;
    }

    template<typename G>
    static wide_int random(G g, wide_int lower, wide_int upper) {
        return lower + random(g, upper - lower);
    }

    static constexpr wide_int from_string(const char* cp, int base = 10) {
		int_base::assertValid(base);
        if (cp == 0 || *cp == 0) throw std::invalid_argument("invalid string");
        bool neg = false;
        switch (cp[0]) {
            case '-': neg = true;
            case '+': ++cp;
        }
        wide_int ret;
        while (char ch = *cp++) {
            int num = int_base::from_char_raw(ch, base);
            if (num < 0) throw std::invalid_argument("invalid string");
            ret *= base;
            ret += (unsigned) num;
        }
        if (neg) ret.negative();
        return ret;
    }

    static constexpr wide_int from_string_based(const char* cp) {
        if (cp == 0 || *cp == 0) throw std::invalid_argument("invalid string");
        bool neg = false;
        switch (cp[0]) {
            case '-': neg = true;
            case '+': ++cp;
        }
        int base = 10;
        if (char ch1 = *cp++) {
            if (ch1 == '0') {
                switch (*cp++) {
                    case 'b': case 'B': base = 2; break;
                    case 'x': case 'X': base = 16; break;
                    default: --cp;
                }
                if (base == 10) base = 8;
            } else --cp;
        } else --cp;
        wide_int ret;
        while (char ch = *cp++) {
            int num = int_base::from_char_raw(ch, base);
            if (num < 0) throw std::invalid_argument("invalid string");
            ret *= base;
            ret += (unsigned) num;
        }
        if (neg) ret.negative();
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
            switch (is.get()) {
                case '-': neg = true;
                case '+': break;
                default: is.unget();
            }
        }
        if (is && base == 16) {
            if (is.get() == '0') {
                char ch = is.get();
                if (!(ch == 'x' || ch == 'X')) is.unget();
            } else is.unget();
        }
        bool read = false;
        while (is) {
            char ch = is.get();
            int num = int_base::from_char_raw(ch, base);
            if (num < 0) break;
            read = true;
            ret *= base;
            ret += (unsigned) num;
        }
        if (read) {
            if (neg) ret.negative();
            rhs = ret;    
        } else {
            is.setstate(std::ios::failbit);
        }
        return is;
    }

    constexpr int compare(wide_int const& rhs) const noexcept {
        if (int cmp = is_negative() - rhs.is_negative()) return -cmp;
        for (size_t i = BYTES - 1; /* i >= 0 */ ~i; --i) {
            if (int cmp = bytes[i] - rhs.bytes[i]) return cmp;
        }
        return 0;
    }
};

template<typename Integral> requires std::is_integral_v<Integral>
wide_int(Integral) -> wide_int<sizeof(Integral) << 3, std::is_signed_v<Integral>>;

#define BINARY_OP(op) \
template<size_t N, bool S, size_t M, bool R> \
constexpr std::common_type_t<wide_int<N, S>, wide_int<M, R>> \
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
constexpr std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator*(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret;
    for (size_t i = 0; i < lhs.BYTES; ++i) {
        unsigned carry = 0;
        for (size_t j = 0; j < rhs.BYTES && (i + j) < ret.BYTES; ++j) {
            carry += ret.bytes[i + j] + lhs.bytes[i] * rhs.bytes[j];
            ret.bytes[i + j] = carry & 0xFF;
            carry >>= 8;
        }
    }
    return ret;
}

template<size_t N, bool S, size_t M, bool R>
constexpr typename std::common_type_t<wide_int<N, S>, wide_int<M, R>>::div_t
    div(wide_int<N, S> lhs, wide_int<M, R> rhs) {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs;
    return ret.div(rhs);
}

template<size_t N, bool S, size_t M, bool R>
constexpr std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator/(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) {
    return div(lhs, rhs).quot;
}

template<size_t N, bool S, size_t M, bool R>
constexpr std::common_type_t<wide_int<N, S>, wide_int<M, R>> 
    operator%(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) {
    return div(lhs, rhs).rem;
}

template<size_t N, bool S, size_t M, bool R>
constexpr int compare(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs;
    return ret.compare(rhs);
}

#define COMPARE_OP(op) \
template<size_t N, bool S, size_t M, bool R> \
constexpr bool operator op(wide_int<N, S> const& lhs, wide_int<M, R> const& rhs) noexcept { \
    return compare(lhs, rhs) op 0; \
}

COMPARE_OP(==)
COMPARE_OP(!=)
COMPARE_OP(<)
COMPARE_OP(>)
COMPARE_OP(<=)
COMPARE_OP(>=)

#undef COMPARE_OP

#define ALIAS(N) \
using sint##N = wide_int<N, true>; \
using uint##N = wide_int<N, false>; \
inline namespace literals { \
inline namespace wide_int_literals { \
constexpr sint##N operator"" _s##N(unsigned long long ull) { \
    return ull; \
} \
constexpr sint##N operator"" _sL##N(const char* s) { \
    return sint##N::from_string_based(s); \
} \
constexpr uint##N operator"" _u##N(unsigned long long ull) { \
    return ull; \
} \
constexpr uint##N operator"" _uL##N(const char* s) { \
    return uint##N::from_string_based(s); \
} \
} \
}

ALIAS(128)
ALIAS(256)
ALIAS(512)
ALIAS(1024)
ALIAS(2048)
ALIAS(4096)
ALIAS(8192)

#undef ALIAS

}

template<size_t N, bool S>
struct std::hash<yao_math::wide_int<N, S>> {
    constexpr size_t operator()(yao_math::wide_int<N, S> const& rhs) const noexcept {
        return rhs.template to_integral<size_t>();
    }
};
