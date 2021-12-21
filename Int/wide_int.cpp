// note: in this header, int[N] should be treated as a single little-endian integer

#include <cstddef>
#include <array>
#include <vector>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include "../yao_math.h"
#include "int_conv.cpp"

namespace yao_math {
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

#define REQUIRES(cond) ::std::enable_if_t<cond, bool> = true

namespace yao_math {

using byte = unsigned char;
using uint = unsigned int;

template<size_t N, bool S>
struct wide_int {
    static_assert(N, "N must not be 0");
    static_assert((N & 7) == 0, "N should be a multiplier of 8");
    constexpr static bool SIGNED = S;
    constexpr static size_t BITS = N;
    constexpr static size_t BYTES = BITS >> 3;

    byte bytes[BYTES];

    struct div_t {
        wide_int quot, rem;
        div_t negative() {
            quot.negative();
            rem.negative();
            return *this; 
        }
    };

    wide_int() noexcept {
        std::memset(bytes, 0, BYTES);
    }

    explicit operator bool() const noexcept {
        for (size_t i = 0; i < BYTES; ++i) {
            if (bytes[i]) return true;
        }
        return false;
    }

    template<typename Unsigned, REQUIRES(std::is_unsigned_v<Unsigned>)>
    wide_int(Unsigned u) noexcept : wide_int() {
        if (!u) return; 
        size_t n = std::min(BYTES, sizeof u);
        for (size_t i = 0; i < n; ++i) {
            bytes[i] = u & 0xFF;
            u >>= 8;
            if (!u) break;
        }
    }
    
    template<typename Unsigned, REQUIRES(std::is_unsigned_v<Unsigned>)>
    wide_int& operator=(Unsigned u) noexcept {
        return *new (this) wide_int(u);
    }

    template<typename Signed, REQUIRES(std::is_signed_v<Signed>)>
    wide_int(Signed s) noexcept : wide_int(static_cast<std::make_unsigned_t<Signed>>(s < 0 ? -s : s)) {
        if (s < 0) negative();
    }

    template<typename Signed, REQUIRES(std::is_signed_v<Signed>)>
    wide_int& operator=(Signed s) noexcept {
        return *new (this) wide_int(s);
    }

    template<size_t M, bool R>
    wide_int(wide_int<M, R> const& rhs) noexcept : wide_int() {
        if (!rhs) return; 
        size_t n = std::min(BYTES, rhs.BYTES);
        std::memcpy(bytes, rhs.bytes, n);
        if (rhs.is_negative() && BYTES > rhs.BYTES) {
            std::memset(bytes + rhs.BYTES, 0xFF, BYTES - rhs.BYTES);
        }
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

    bool highest_bit() const noexcept {
        return bytes[BYTES - 1] >> 7;
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

#define SCALE_OP(op, ex) \
wide_int& operator op##=(wide_int const& rhs) noexcept(ex) { \
    return *this = *this op rhs; \
}

SCALE_OP(*, true)
SCALE_OP(/, false)
SCALE_OP(%, false)

#undef SCALE_OP

    void shiftLeftBytes(size_t rhs) noexcept {
        if (!rhs || !*this) return;
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
        if (!rhs || !*this) return;
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

    int compare(wide_int const& rhs) const noexcept {
        if (int cmp = is_negative() - rhs.is_negative()) return -cmp;
        for (size_t i = BYTES - 1; /* i >= 0 */ ~i; --i) {
            if (int cmp = bytes[i] - rhs.bytes[i]) return cmp;
        }
        return 0;
    }
    
    size_t log2() const {

const static char table[] = 
"-011222233333333444444444444444455555555555555555555555555555555"
"6666666666666666666666666666666666666666666666666666666666666666"
"7777777777777777777777777777777777777777777777777777777777777777"
"7777777777777777777777777777777777777777777777777777777777777777"
;
        for (size_t i = BYTES - 1; /* i >= 0 */ ~i; --i) {
            if (bytes[i]) {
                return (i << 3) + table[bytes[i]] - '0';
            }
        }
        throw std::invalid_argument("0.log2() is invalid");
    }

    wide_int abs() const noexcept {
        if (is_negative())  return operator-();
        else                return operator+();
    }

    div_t div(wide_int rhs) const {
        if (!rhs) throw std::invalid_argument("divided by 0");
        if (!*this) return {0, 0};
        switch (is_negative() + rhs.is_negative()) {
            case 1: return abs().div(rhs.abs()).negative();
            case 2: return abs().div(rhs.abs());
        }
        size_t rbits = rhs.log2();
        wide_int rem = *this;
        wide_int qout;
        while (rem > rhs) {
            size_t bits = rem.log2();
            size_t overhead = bits - rbits;
            wide_int q = rhs << overhead;
            if (q > rem && overhead) {
                --overhead;
                q >>= 1;
            }
            rem -= q;
            qout += wide_int(1) << overhead;
        }
        if (rem == rhs) {
            ++qout;
            rem = 0;
        }
        return {qout, rem};
    }

    byte to_byte() const noexcept {
        return bytes[0];
    }

    std::string to_string(uint base = 10) const {
        if (base < 2 || base > 36) throw std::invalid_argument("invalid base");
        std::string buf;
        div_t d = {*this, 0};
        do {
            d = d.quot.div(base);
            buf += int_conv::base::to_char(d.rem.abs().to_byte(), base);
        } while (d.quot);
        if (is_negative()) buf += '-';
        return {buf.rbegin(), buf.rend()};
    }

    static wide_int from_string(std::string s, uint base = 10) {
        if (base < 2 || base > 36) throw std::invalid_argument("invalid base");
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

template<size_t N, bool S, size_t M, bool R>
typename std::common_type_t<wide_int<N, S>, wide_int<M, R>>::div_t
    div(wide_int<N, S> lhs, wide_int<M, R> rhs) {
    std::common_type_t<wide_int<N, S>, wide_int<M, R>> ret = lhs;
    return ret.div(rhs);
}

using int128 = wide_int<128, true>;
using int256 = wide_int<256, true>;
using int512 = wide_int<512, true>;
using int1024 = wide_int<1024, true>;
using uint128 = wide_int<128, false>;
using uint256 = wide_int<256, false>;
using uint512 = wide_int<512, false>;
using uint1024 = wide_int<1024, false>;

inline namespace literals {
inline namespace wide_int_literals {

#define LITERAL(type) \
type operator"" _##type(const char* s) { \
    return type::from_string(s); \
}

LITERAL(int128)
LITERAL(int256)
LITERAL(int512)
LITERAL(int1024)

LITERAL(uint128)
LITERAL(uint256)
LITERAL(uint512)
LITERAL(uint1024)

#undef LITERAL

}
}
}

#undef REQUIRES

#include <iostream>

int main() {
    using namespace std;
    using namespace yao_math;
    using yao_math::byte;
    using namespace yao_math::wide_int_literals;
    cout << (111111111111111111111111111111111111111111111111111111111111111111111_uint256).to_string(16);
}