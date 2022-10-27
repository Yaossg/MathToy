#include "linear_prime.cpp"

#include <iostream>
using namespace std;

int main() {
    LinearPrimeEngine lp(1000 * 1000);
    for (LinearPrimeEngine::int_t p : lp.primes) {
        cout << p <<", ";
    }
}
