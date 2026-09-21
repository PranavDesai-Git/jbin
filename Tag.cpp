#include "Variant.hpp"
#include <cstdint>
#include <vector>
enum class wiretype : uint8_t {
    Varint = 0,
    Fixed32 = 1,
    Fixed64 = 2,
    Delimited = 3
};
