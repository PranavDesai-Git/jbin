#include "Variant.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    size_t offset = 0;
    std::vector<uint8_t> buffer;
    encodeVariant(buffer, 999999999999999999);
    uint64_t decode = decodeVariant(buffer, offset);
    std::cout << "Decoded:" << decode << std::endl;
    return 0;
}
