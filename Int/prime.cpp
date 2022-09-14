#include <cmath>
#include <cstdio>


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



int main() {
	int i = 1;
	printf("is_prime(%d)=%s", i, is_prime_hexa(i) ? "true" : "false");	
}