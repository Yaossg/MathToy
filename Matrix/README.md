# Matrix

## 概述

```C++
#include "../yao_math.h"
```

前置头文件，引入`yao_math::toTex` 和 `yao_math::pow`

```C++
namespace yao_math
```

本头文件定义的类均在此命名空间下


```C++
struct invalid_matrix : std::invalid_argument
```

矩阵相关的异常


```C++
template<typename E>
class matrix
```

矩阵类模板

## 矩阵类模板

```C++
matrix(size_t m, size_t n);
```
构造一个`m * n`矩阵，并将每个元素默认初始化。
```C++
matrix(size_t m, size_t n, std::function<E(size_t, size_t)> gen);
```
构造一个`m * n`矩阵，将`i`行`j`列并初始化为`gen(i, j)`
```C++
matrix(matrix const& that) = default;
matrix(matrix&&) = default;
```
默认复制/移动构造器
```C++
size_t row();
size_t col();
size_t size();
bool is_square() const;
bool is_singleton() const;
bool is_vector() const;
bool is_row_vector() const;
bool is_col_vector() const;
```
返回矩阵行数列数，元素个数
判断元素是否为方阵，是否仅有一个元素，是否为向量、行向量或列向量
注意一个元素不算做向量
```C++
matrix& operator=(matrix const& that) = default;
matrix& operator=(matrix&& that) = default;
```
默认复制/移动赋值运算符
```C++
E& at(size_t i, size_t j);
E const& at(size_t i, size_t j) const;
```
访问`i`行`j`列元素
```C++
matrix map(std::function<E(E const&)> mapper) const;
```
对所有元素进行映射
返回矩阵的`i`行`j`列元素为`mapper(this->at(i, j))`
```C++
matrix operator+() const;
matrix operator-() const;
matrix const& operator+=(matrix const& that);
matrix operator+(matrix that) const;
matrix const& operator-=(matrix const& that);
matrix operator-(matrix that) const;
```
矩阵的线性计算，注意参与计算的矩阵必须同形
```C++
matrix const& operator*=(E const& coe);
matrix operator*(E const& coe) const;
friend matrix operator*(E const& coe, matrix that);
```
矩阵数乘
```C++
matrix operator*(matrix const& that) const;
```
矩阵乘法，注意前一个矩阵的列数必须等于后一个矩阵的行数
```C++
matrix trans() const;
```
转置
```C++
matrix submat(size_t z, size_t w, size_t u, size_t v) const;
```
子矩阵从`z`行`w`列开始截取一个`u * v`的矩阵。
```C++
E cofactor(size_t u, size_t v) const;
E algebraic_cofactor(size_t u, size_t v) const;
```
返回除去`u`行`v`列的余子式和代数余子式
注意必须是方阵才能执行此函数
```C++
E det() const;
```
行列式
采用拉普拉斯按第一行展开
注意必须是方阵才能执行此函数

`(bool)std::declval<E>()`必须有意义才能使用该函数

```C++
matrix adjoint() const;
matrix inverse() const;
```
伴随矩阵和逆矩阵
注意必须是方阵才能执行此函数

```C++
E trace() const;
```
矩阵的迹
注意必须是方阵才能执行此函数

```C++
friend std::string toTex(matrix const& t);
```
将矩阵转换为 Tex

`yao_math::toTex(std::declval<E>())`必须有意义才能使用该函数

```C++
friend std::istream& operator>>(std::istream& is, matrix& that);
friend std::ostream& operator<<(std::ostream& os, matrix const& that);
```
从 `iostream` 中输入输出矩阵，需要元素实现相关函数重载。

## 示例程序

[两个实数矩阵输入输出各类运算示例](./test.cpp)

[五阶行列式展开、直线对称矩阵 TeX 生成器](./test-expr.cpp)

