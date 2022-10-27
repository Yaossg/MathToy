cmake_minimum_required(VERSION 3.21)
project(MathToy)

set(CMAKE_CXX_STANDARD 20)


add_executable(matrix Matrix/test.cpp yao_math.h Matrix/matrix.cpp)
add_executable(matrix-expr Matrix/test-expr.cpp yao_math.h Matrix/matrix.cpp Expr/expr.cpp)
add_executable(expr Expr/test.cpp Expr/expr.cpp)
add_executable(rational Int/rational.cpp Int/ratioanl_test.cpp)
add_executable(prime-benchmark Int/prime_benchmark.cpp)
add_executable(linear-prime Int/linear_prime.cpp Int/linear_prime_test.cpp)
add_executable(sieve-prime Int/sieve_prime.cpp Int/sieve_prime_test.cpp)
add_executable(wide-int Int/wide_int.cpp Int/int_base.cpp Int/fpbits.cpp Int/wide_int_test.cpp)
