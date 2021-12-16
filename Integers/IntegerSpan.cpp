// note: in this header, int[N] should be treated as a single little-endian integer

#include <cstddef>
#include <array>
#include <vector>

namespace yao_math::integer::span {
    template<typename T>
    struct Span {
        T* ptr;
        size_t size;
    };

    template<typename T, size_t N>
    Span<T> from_array(std::array<T, N> const& a) {
        return {a.data(), N};
    }
    
    template<typename T>
    Span<T> from_vector(std::vector<T> const& a) {
        return {a.data(), a.size()};
    }

    // note: code below without generalization for entertainment is without further support guaranteed

    struct BIS { // based int span
        Span<int> span;
        int base = 10;
    };


    void conv(BIS src, BIS dst) {


    }


}

int main() {
    using namespace std;
    

}