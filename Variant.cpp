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

uint64_t decodeVariant(const std::vector<uint8_t> &buffer, size_t &offset) {
    uint64_t result = 0;
    int shift = 0;
    while ((buffer[offset] & 128) != 0 && offset < buffer.size()) {
        uint8_t byte = buffer[offset++];
        byte &= 127;
        const uint64_t cast = static_cast<uint64_t>(byte) << shift;
        result |= cast;
        shift += 7;
    }
    uint8_t byte = buffer[offset++] & 127;
    const uint64_t cast = static_cast<uint64_t>(byte) << shift;
    result |= cast;
    return result;
}
