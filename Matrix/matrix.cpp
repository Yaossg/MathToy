#ifndef YAO_MATH_MATRIX
#define YAO_MATH_MATRIX

#include <cstddef>
#include <utility>
#include <stdexcept>
#include <vector>
#include <functional>

#include <cmath>
#include <iostream>

#include "../yao_math.h"


namespace yao_math {

struct invalid_matrix: std::invalid_argument {
    explicit invalid_matrix(const char* message) : invalid_argument(message) {}
};

#define ASSERT_SQUARE(func) do if (!is_square()) throw invalid_matrix(func " is available only for square matrices"); while (false)

// attention: indices range from 0 to n - 1
template<typename E>
class matrix {
    static_assert(!std::is_same<E, bool>::value, "matrix<bool> is forbidden");
    
    static E zero(...) { return 0; }

    std::vector<E> e;
    size_t m, n;

public:
    using element_t = E;
    explicit matrix(size_t m, std::function<E(size_t, size_t)> gen = zero): matrix(m, m, gen) {}
    explicit matrix(size_t m, size_t n, std::function<E(size_t, size_t)> gen = zero): m(m), n(n) {
        if (!m || !n) throw invalid_matrix("empty matrix is forbidden"); 
        e.resize(m * n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) at(i, j) = gen(i, j);
    }
    matrix(matrix const& that) = default;
    matrix(matrix&&) = default;
    size_t row() const { return m; }
    size_t col() const { return n; }
    size_t sz() const { return m * n; }

    bool is_square() const { return m == n; }
    bool is_singleton() const { return m == 1 && n == 1; }
    bool is_vector() const { return (m == 1) != (n == 1); }
    bool is_row_vector() const { return m == 1 && n != 1; }
    bool is_col_vector() const { return m != 1 && n == 1; }

    matrix& operator=(matrix const& that) = default;
    matrix& operator=(matrix&& that) = default;
    E& at(size_t i, size_t j) { return e[i * m + j]; }
    E const& at(size_t i, size_t j) const { return e[i * m + j]; }

    matrix map(std::function<E(E)> mapper) const {
        matrix that(m, n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) that.at(i, j) = mapper(at(i, j));
        return that;
    }

    matrix operator+() const { return *this; }
    
    matrix operator-() const { return map(std::negate<E>()); }

    matrix const& operator+=(matrix const& that) {
        if (m != that.m || n != that.n) throw invalid_matrix("addition can be applied only on matrices with an identical size");
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
    
    matrix operator*(matrix const& that) const {
        size_t p = that.m;
        if (n != p) throw invalid_matrix("multiplication can be applied only if the row() of first matrix equals col() of second one");
        matrix product(m, that.n);
        for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) for (size_t k = 0; k < p; ++k) product.at(i, j) += at(i, k) * that.at(k, j);
        return product;
    }

    matrix trans() const {
        return matrix(n, m, [this] (size_t i, size_t j) { return at(j, i); });
    }

    matrix submat(size_t z, size_t w, size_t u, size_t v) const {
        return matrix(u, v, [this, z, w] (size_t i, size_t j) { return at(z + i, w + j); });
    }

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
        ASSERT_SQUARE("det");
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
            E e = at(0, j);
            if (e)
                ret += e * algebraic_cofactor(0, j);
        }
        return ret;
    }

    matrix adjoint() const {
        ASSERT_SQUARE("adjoint");
        return matrix(m, n, [this] (size_t i, size_t j) { return algebraic_cofactor(j, i); });
    }

    matrix inverse() const {
        ASSERT_SQUARE("inverse");
        E det_ = det();
        if (det_ == 0) throw invalid_matrix("matrix whose determinant is zero has no inverse matrix");
        return 1 / det_ * adjoint();
    }

    E trace() const {
        ASSERT_SQUARE("trace");
        E trace_ = 0;
        for (size_t i = 0; i < m; ++i) trace_ += at(i, i);
        return trace_;
    }

    E reduce(std::function<E(E, E)> fold) const {
        E acc = e[0];
        for (size_t i = 1; i < sz(); ++i) acc = fold(acc, e[i]);
        return acc;
    }

    E reduceRow(size_t row, std::function<E(E, E)> fold) const {
        E acc = at(row, 0);
        for (size_t j = 1; j < n; ++j) acc = fold(acc, at(row, j));
        return acc;
    }

    E reduceCol(size_t col, std::function<E(E, E)> fold) const {
        E acc = at(0, col);
        for (size_t i = 1; i < m; ++i) acc = fold(acc, at(i, col));
        return acc;
    }

    matrix normalizeRow() const {
        matrix squared = map(std::bind(std::multiplies<E>(), std::placeholders::_1, std::placeholders::_1));
        std::vector<E> length(m);
        for (size_t i = 0; i < m; ++i) length[i] = std::sqrt(squared.reduceRow(i, std::plus<E>()));
        return matrix(m, n, [this, &length] (size_t i, size_t j) { return at(i, j) / length[i]; });
    }

    matrix normalizeCol() const {
        matrix squared = map(std::bind(std::multiplies<E>(), std::placeholders::_1, std::placeholders::_1));
        std::vector<E> length(n);
        for (size_t j = 0; j < n; ++j) length[j] = std::sqrt(squared.reduceCol(j, std::plus<E>()));
        return matrix(m, n, [this, &length] (size_t i, size_t j) { return at(i, j) / length[j]; });
    }

    friend std::string toTex(matrix const& t) {
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
    
    friend std::istream& operator>>(std::istream& is, matrix& that) {
        for (size_t i = 0; i < that.m; ++i) for (size_t j = 0; j < that.n; ++j) is >> that.at(i, j);
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, matrix const& that) {
        for (size_t i = 0; i < that.m; ++i) {
            bool first = true;
            for (size_t j = 0; j < that.n; ++j) { 
                if (first) first = false; else os << ' ';
                os << that.at(i, j);
            }
            os << std::endl;
        } 
        return os;
    }
};

#undef ASSERT_SQUARE

}

#endif