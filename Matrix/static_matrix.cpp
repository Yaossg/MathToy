#include <cstddef>
#include <vector>

#include "../yao_math.h"

namespace yao_math {

template<typename E, std::size_t m, std::size_t n>
class matrix {
    static_assert(!std::is_same<E, bool>::value, "matrix<bool> is forbidden");
public:
    using element_t = E;
    using size_t = std::size_t;
    constexpr static size_t row() { return m; }
    constexpr static size_t col() { return n; }

};



}

