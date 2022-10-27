#include <iostream>
using namespace std;

#include "expr.cpp"

using namespace yao_math;

int main() {
	IntExpr<int> x("x", 1);
	
	auto e1 = (x+1)*(x+2)*(x+3);
	cout << toTex(e1) << endl;
    auto e2 = e1.eval("x", -1);
	cout << toTex(e2) << endl;
}
