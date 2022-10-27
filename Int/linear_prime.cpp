#include <vector>
#include <cstdint>
#include <cmath>

struct LinearPrimeEngine {
    using int_t = std::uintmax_t;
    std::vector<int_t> primes;
    std::vector<bool> is_not_prime = {1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1};
    LinearPrimeEngine(int_t L) {
        if (L <= 10) {
            primes = {2, 3, 5, 7};
            return;
        }
        is_not_prime.resize(L + 1);
        double ln = std::log(L);
        size_t estimate = L / (ln - 1.1);
        primes.reserve(estimate);
        for (int_t i = 2; i <= L; ++i) {
            if (!is_not_prime[i]) primes.push_back(i);
            for (int_t j : primes) {
                if (i * j > L) break;
                is_not_prime[i * j] = true;
                if (i % j == 0) break;
            }
        }
    }

    inline bool is_prime(int_t u) const {
        return !is_not_prime[u];
    }

};
