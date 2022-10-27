#include "rational.cpp"

#include <iostream>

using namespace std;

using namespace yao_math;


constexpr Rational<int> a{1, 2}, b{3, 4};
static_assert(a / b == Rational<int>{2, 3});


int main() {
    cout << toTex(a / b) << endl;
}
