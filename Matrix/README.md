# Matrix

```C++
#include "../yao_math.h"
```

前置头文件，引入`yao_math::toTex` 和 `yao_math::pow`

```C++
namespace yao_math
```

本头文件定义的类均在此命名空间下


```C++
struct invalid_matrix: std::invalid_argument
```

矩阵相关的异常


```C++
template<typename E>
class matrix
```

矩阵类模板

## 矩阵类模板

### 模板参数 `E`

`E` 为矩阵的元素类型，至少需要满足下列要求

- 不是`bool`
- `E(0)`合法
- `std::is_copy_assignable_v<E> == true`
- `std::is_copy_constructible_v<E> == true`
- 可以完成加减乘除四则运算

部分函数的特殊要求见下面得详细介绍

注意：矩阵的行列下标均从 0 开始

### 公共成员和友元函数

```C++
using element_t = E;
using size_t = std::size_t;
```
成员类型
```C++
explicit matrix(size_t m, std::function<E(size_t, size_t)> gen = zero);
```
构造一个`m`阶方阵，等价于`matrix(m, m, gen)`
```C++
matrix(size_t m, size_t n, std::function<E(size_t, size_t)> gen = zero);
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
size_t sz();
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
matrix map(std::function<E(E)> mapper) const;
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
matrix const& operator*=(E coe);
matrix operator*(E coe) const;
friend matrix operator*(E coe, matrix that);
```
矩阵数乘
```C++
matrix operator*(matrix that) const;
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
E reduce(std::function<E(E, E)> fold) const;
E reduceRow(size_t row, std::function<E(E, E)> fold) const;
E reduceCol(size_t col, std::function<E(E, E)> fold) const;
```
归约所有元素，或某一行某一列。

归约顺序由小到大，归约所有元素时`at(i, j)`迭代下标递增是先`j`后`i`

```C++
matrix normalizeRow() const;
matrix normalizeCol() const;
```
按行或按列归一化

`std::sqrt(std::declval<E>())`必须有意义才能使用该函数

```C++
friend std::string toTex(matrix const& t);
```
将矩阵转换为Tex（可以用于如 LaTeX）

`yao_math::toTex(std::declval<E>())`必须有意义才能使用该函数

```C++
friend std::istream& operator>>(std::istream& is, matrix& that);
friend std::ostream& operator<<(std::ostream& os, matrix const& that);
```
从 `iostream` 中输入输出矩阵，需要元素实现相关函数重载。

## 示例程序

[两个实数矩阵输入输出各类运算示例](./test.cpp)

[五阶行列式展开LaTeX生成器](./test-expr.cpp)

