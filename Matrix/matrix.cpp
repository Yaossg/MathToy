#include <cstddef>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>

namespace yao_math {

#ifndef YAO_MATH_TO_TEX_GENERIC
#define YAO_MATH_TO_TEX_GENERIC

template<typename T>
std::string toTex(T const& t) {
	return std::to_string(t);
}

#endif

using std::size_t;
using std::invalid_argument;
using std::function;

struct invalid_matrix: invalid_argument {
    explicit invalid_matrix(const char* message) : invalid_argument(message) {}
};

// attention: indices range from 0 to n - 1
template<typename E>
class matrix {
    static_assert(!std::is_const<E>::value, "matrix<E const> is forbidden, use matrix<E> const instead");
    static_assert(!std::is_volatile<E>::value, "matrix<E volatile> is forbidden");
    static_assert(!std::is_same<E, bool>::value, "matrix<bool> is forbidden");
    std::vector<E> e;
    size_t m, n;
    void alloc() {
        e.resize(m * n);
    }
    void clone(matrix const& that) {
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) at(i, j) = that.at(i, j);
    }
    void assert_identical_size(matrix const& that) const {
        if (m != that.m || n != that.n) throw invalid_matrix("linear operations on matrix can only be applied on matrices with an identical size");
    }
    void assert_square_matrix() const {
        if (!is_square()) throw invalid_matrix("determinant of matrix is available only if it is a squre one");
    }
    static E zero(...) { return 0; }
public:
    explicit matrix(size_t m): matrix(m, m) {}
    matrix(size_t m, size_t n, function<E(size_t, size_t)> gen = zero): m(m), n(n) {
        if (!m || !n) throw invalid_matrix("empty matrix is forbidden"); 
        alloc(); 
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) at(i, j) = gen(i, j);
    }
    matrix(matrix const& that) = default;
    matrix(matrix&&) = default;
    size_t row() const { return m; }
    size_t col() const { return n; }
    size_t sz() const { return m * n; }
    matrix& operator=(matrix const& that) = default;
    matrix& operator=(matrix&& that) = default;
    E& at(size_t i, size_t j) { return e[i * m + j]; }
    E const& at(size_t i, size_t j) const { return e[i * m + j]; }

    matrix map(function<E(E)> mapper) const {
        matrix that(m, n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) that.at(i, j) = mapper(at(i, j));
        return that;
    }

    matrix operator+() const { return *this; }
    
    matrix operator-() const { return map(std::negate<E>()); }

    matrix const& operator+=(matrix const& that) {
        assert_identical_size(that);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) at(i, j) += that.at(i, j);
        return *this;
    }

    matrix operator+(matrix that) const { return that += *this; }
    matrix const& operator-=(matrix const& that) { return *this += -that; }
    matrix operator-(matrix that) const { return *this + -that; }

    matrix const& operator*=(E coe) { return *this = *this * coe; }
    matrix operator*(E coe) const {
        return map(std::bind(std::multiplies<E>(), coe, std::placeholders::_1));
    }
    friend matrix operator*(E coe, matrix that) {
        return that *= coe;
    }
    
    matrix operator*(matrix that) const {
        size_t p = that.m;
        if (n != p) throw invalid_matrix("matrix multiplication can be applied only if the row() of first matrix equals col() of second one");
        matrix product(m, that.n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) for (size_t k = 0; k < p; ++k) product.at(i, j) += at(i, k) * that.at(k, j);
        return product;
    }

    matrix trans() const {
        matrix that(n, m);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) that.at(j, i) = at(i, j);
        return that;
    }

    bool is_square() const { return m == n; }
    bool is_singleton() const { return m == 1 && n == 1; }
    bool is_vector() const { return (m == 1) != (n == 1); }
    bool is_row_vector() const { return m == 1 && n != 1; }
    bool is_col_vector() const { return m != 1 && n == 1; }

    E cofactor(size_t u, size_t v) const {
        matrix that(m - 1, n - 1);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) {
            if (i == u || j == v) continue;
            size_t x = i, y = j;
            if (i > u) --x;
            if (j > v) --y;
            that.at(x, y) = at(i, j);
        }
        return that.det();
    }

    E algebraic_cofactor(size_t u, size_t v) const { 
        return ((u + v) % 2 ? -1 : 1) * cofactor(u, v);
    }

    E det() const {
        assert_square_matrix();
        switch (m) { // shortcut
            case 1: return at(0, 0);
            case 2: return at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0);
            case 3: ; return 
                + at(0, 0) * at(1, 1) * at(2, 2) 
                + at(1, 0) * at(2, 1) * at(0, 2)
                + at(2, 0) * at(0, 1) * at(1, 2)
                - at(0, 2) * at(1, 1) * at(2, 0)
                - at(1, 2) * at(2, 1) * at(0, 0)
                - at(2, 2) * at(0, 1) * at(1, 0);
            default: ; // fallback
        }
        E ret = 0;
        for (size_t j = 0; j < n; ++j) {
            ret += at(0, j) * algebraic_cofactor(0, j);
        }
        return ret;
    }

    matrix adjugate() const {
        assert_square_matrix();
        matrix that(m, n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) that.at(i, j) = algebraic_cofactor(i, j);
        return that;
    }

    matrix inverse() const {
        assert_square_matrix();
        E det_ = det();
        if (det_ == 0) throw invalid_matrix("matrix whose determinant is zero has no inverse matrix");
        return 1 / det_ * adjugate();
    }

    friend std::string toTex(matrix t) {
        std::string r = "\\left[ \\begin{array}{" + std::string(t.n, 'c') + '}';
        for (size_t i = 0; i < t.m; ++i) {
            bool first = true;
            for (size_t j = 0; j < t.n; ++j) {
                if (first) first = false; else r += " & ";
                r += toTex(t.at(i, j));
            }
            if (i + 1 < t.m)
                r += " \\\\ ";
        }
        return r + "\\end{array} \\right]";
    }

};

}
