#include "matrix.cpp"
#include "../Expr/expr.cpp"

#include <fstream>

using namespace std;
using namespace yao_math;

void det() {
    matrix<IntExpr<int>> a(5, 5, [](size_t i, size_t j) {
        return "a_{" + to_string(i + 1) + to_string(j + 1) + '}';
    });
    cout << "\\det " << toTex(a) << " = " << endl;
    cout << toTex(a.det()) << endl;

}

// compute reflection matrix against y=mx+b
void sub() {
    RatioExpr m = IntExpr<int>("m", 1);
    RatioExpr b = IntExpr<int>("b", 1);
    RatioExpr p = IntExpr<int>("p", 1);
    // p = sqrt(1+m^2)
    RatioExpr cos = IntExpr(1) / p;
    RatioExpr sin = -m / p;

    matrix<RatioExpr<int>> t2(3, 3),r2(3, 3),x(3, 3),r1(3, 3),t1(3, 3);

    t2.at(0, 0) = 1;
    t2.at(1, 1) = 1;
    t2.at(2, 2) = 1;
    t2.at(1, 2) = b;

    r2.at(0, 0) = cos;
    r2.at(0, 1) = sin;
    r2.at(1, 0) = -sin;
    r2.at(1, 1) = cos;
    r2.at(2, 2) = 1;
    
    x.at(0, 0) = 1;
    x.at(1, 1) = -1;
    x.at(2, 2) = 1;

    r1.at(0, 0) = cos;
    r1.at(0, 1) = -sin;
    r1.at(1, 0) = sin;
    r1.at(1, 1) = cos;
    r1.at(2, 2) = 1;

    t1.at(0, 0) = 1;
    t1.at(1, 1) = 1;
    t1.at(2, 2) = 1;
    t1.at(1, 2) = -b;

    auto M = t2*r2*x*r1*t1;
    M = M.map([m](RatioExpr<int> const& e){return e.eval({{{{"p", 2}}}, 1}, m * m + 1);});
    cout << toTex(M) << endl;
}

int main() {
    det();
    sub();
}