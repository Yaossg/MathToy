cmake_minimum_required(VERSION 3.21)
project(MathToy)

set(CMAKE_CXX_STANDARD 20)


add_executable(matrix Matrix/test.cpp yao_math.h Matrix/matrix.cpp)
add_executable(matrix-expr Matrix/test-expr.cpp yao_math.h Matrix/matrix.cpp Expr/expr.cpp)
add_executable(expr Expr/test.cpp Expr/expr.cpp)
