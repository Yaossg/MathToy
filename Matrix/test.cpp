#include "matrix.cpp"

#include <iostream>
#include <fstream>

using namespace std;
using namespace yao_math;

template<typename E>
istream& operator>>(istream& is, matrix<E>& that) {
    for (size_t i = 0; i < that.row(); ++i) for (size_t j = 0; j < that.col(); ++j) is >> that.at(i, j);
    return is;
}

template<typename E>
ostream& operator<<(ostream& os, matrix<E> const& that) {
    for (size_t i = 0; i < that.row(); ++i) {
        bool first = true;
        for (size_t j = 0; j < that.col(); ++j) { 
            if (first) first = false; else os << ' ';
            os << that.at(i, j);
        }
        os << endl;
    } 
    return os;
}



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