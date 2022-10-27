#include <iostream>
#include <string>
#include <functional>
#include <cmath>
#include <map>
#include <chrono>
#include <iomanip>

bool is_prime_odd(unsigned long long n) {
	if (n < 2) return false;
	unsigned long long limit = std::sqrt(n);
	for (unsigned long long i = 2; i <= limit; ++i) {
		if (n % i == 0) return false;
	}
	return true;
}
bool is_prime_even(unsigned long long n) {
	if (n < 2 || (n & 1) == 0) return false;
	unsigned long long limit = std::sqrt(n);
	for (unsigned long long i = 3; i <= limit; i += 2) {
		if (n % i == 0) return false;
	}
	return true;
}
bool is_prime_hexa(unsigned long long n) {
	if (n < 2) return false;
	switch (n) case 2: case 3: case 5: case 7: case 11: return true;
	switch (n % 6) case 0: case 2: case 3: case 4: return false;
	unsigned long long limit = std::sqrt(n);
	for (unsigned long long i = 5; i <= limit; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0) return false;
	}
	return true;
}


void measure(std::function<bool(unsigned long long)> test, std::string what) {
    auto start = std::chrono::system_clock::now();
    for (unsigned long long i = 0; i < 10'000'000; ++i)
    	test(i);
    auto stop = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> time = stop - start;
    std::cout << time.count() << "ms for " << what << std::endl;
}

int main() {
	measure(is_prime_odd, "is_prime_odd");
	measure(is_prime_even, "is_prime_even");
	measure(is_prime_hexa, "is_prime_hexa");
}