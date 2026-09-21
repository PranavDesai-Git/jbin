#include "Tag.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    size_t offset = 0;
    std::vector<uint8_t> buffer;
    encodeTag(buffer, 1, 2);
    return 0;
}
