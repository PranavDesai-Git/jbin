#include "Variant.hpp"
#include <cstdint>
#include <vector>
enum class wiretype : uint8_t {
    Varint = 0,
    Fixed32 = 1,
    Fixed64 = 2,
    Delimited = 3
};

void encodetag(std::vector<uint8_t> &buffer, uint32_t fieldnumber,
               wiretype wiretype) {
    uint64_t val = (static_cast<uint64_t>(fieldnumber) << 2);
    val |= static_cast<uint64_t>(wiretype);
    encodeVariant(buffer, val);
}
