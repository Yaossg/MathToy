#include <iostream>
using namespace std;

#include "expr.cpp"

using namespace yao_math;

void int_expr() {
    IntExpr<int> x("x", 1);

    auto e1 = (x+1)*(x+2)*(x+3);
    cout << toTex(e1) << endl;
    auto e2 = e1.eval("x", -1);
    cout << toTex(e2) << endl;
}

void ratio_expr() {
    IntExpr<int> x("x", 1);
    IntExpr<int> y("y", 1);

    RatioExpr e1 = (3*(x*x+2*x+1))/(6*(x+1));
    cout << toTex(e1) << endl;
    RatioExpr e2 = e1.eval("x", y - 1);
    cout << toTex(e2) << endl;
    cout << toTex(e2.eval("y", 6)) << endl;
}

void substitute() {
    IntExpr<int> x("x", 1);
    IntExpr e1 = x * x * x + 2 * x + 1;
    cout << toTex(e1) << endl;
    IntExpr e2 = e1.eval({{{{"x", 2}}}, 1}, IntExpr(1));
    cout << toTex(e2) << endl;
}

int main() {
    int_expr();
    ratio_expr();
    substitute();
}
