#define YAO_MATH_MATRIX_NO_ARITHMETIC
#include "matrix.cpp"
#include "../Expr/expr.cpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace yao_math;

int main() {

    matrix<IntExpr<int>> a(5, 5, [](size_t i, size_t j) {
        return "a_{" + to_string(i + 1) + to_string(j + 1) + '}';
    });
    cout << "\\det " << toTex(a) << " = ";
    cout << endl << endl;
    cout << toTex(a.det());

}