#include "wide_int.cpp"

#include <iostream>

constexpr yao_math::uint256 foo = yao_math::pow(yao_math::uint256(3), 100);
constexpr yao_math::uint256 bar = yao_math::uint256::from_string("123456");
static_assert(foo > bar);

int main() {
    using namespace std;
    using namespace yao_math;
    using yao_math::byte;
    using namespace yao_math::wide_int_literals;
    cout << hash<uint256>{}(1000000000000000000000000000000000000000000000000000000000_uL256) << endl;
}