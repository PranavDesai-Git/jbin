#include <cstddef>
#include <cstdint>
#include <vector>

void encodeVariant(std::vector<uint8_t> &buffer, uint64_t value) {
    while (value >= 128) {
        uint8_t lower = value & 127; // 127 = 01111111
        lower |= 128;
        buffer.push_back(lower);
        value >>= 7;
    }
    uint8_t lower = value & 127;
    buffer.push_back(lower);
}

