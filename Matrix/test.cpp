#include "matrix.cpp"

#include <fstream>

using namespace std;
using namespace yao_math;





int main() {
    ifstream ifs("input.txt");
    ofstream ofs("output.txt");
    int m1, n1;
    ifs >> m1 >> n1;
    ofs << "A (" << m1 << ", " << n1 << ") =" << endl;
    matrix<double> a(m1, n1);
    ifs >> a;
    ofs << a;
    int m2, n2;
    ifs >> m2 >> n2;
    ofs << "B (" << m2 << ", " << n2 << ") =" << endl;
    matrix<double> b(m2, n2);
    ifs >> b;
    ofs << b;
    try {
        matrix<double> c = a.trans();
        ofs << "A^T (" << c.row() << ", " << c.col() << ") =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        double c = a.det();
        ofs << "|A| =" << endl;
        ofs << c << endl;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a.normalizeRow();
        ofs << "a.normalizeRow() =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a.normalizeCol();
        ofs << "a.normalizeCol() =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a.adjoint();
        ofs << "A^* =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a.inverse();
        ofs << "A^-1 =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a + b;
        ofs << "A + B (" << c.row() << ", " << c.col() << ") =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a - b;
        ofs << "A - B (" << c.row() << ", " << c.col() << ") =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        matrix<double> c = a * b;
        ofs << "A * B (" << c.row() << ", " << c.col() << ") =" << endl;
        ofs << c;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
    try {
        string c = toTex(a);
        ofs << "toTex(A) =" << endl;
        ofs << c << endl;
    } catch (invalid_matrix const& e) {
        ofs << e.what() << endl;
    }
}