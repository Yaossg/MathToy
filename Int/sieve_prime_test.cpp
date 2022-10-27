#include "sieve_prime.cpp"

#include <iostream>

int main() {
    SievePrimeEngine engine;
    uintmax_t n = 1111177;
    engine.computeBelow(n);
    std::cout << std::boolalpha << engine.is_prime(n) << std::endl;
    std::cout << engine.get_split(n) << std::endl;
}
