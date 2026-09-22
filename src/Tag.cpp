#include "Tag.hpp"
#include "Varint.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

void encodeTag(std::vector<uint8_t> &buffer, uint32_t fieldNumber,
               wiretype wiretype) {
    uint64_t val = (static_cast<uint64_t>(fieldNumber) << 2);
    val |= static_cast<uint64_t>(wiretype);
    encodeVariant(buffer, val);
}

void decodeTag(const std::vector<uint8_t> &buffer, size_t &offset,
               uint32_t &outFieldNumber, wiretype &outType) {
    uint64_t val = decodeVariant(buffer, offset);
    outType = static_cast<wiretype>(val & 3);
    outFieldNumber = val >> 2;
}

void encodeString(std::vector<uint8_t> &buffer, uint32_t fieldNumber,
                  const std::string &text) {
    encodeTag(buffer, fieldNumber, wiretype::Delimited);
    encodeVariant(buffer, text.size());
    buffer.insert(buffer.end(), text.begin(), text.end());
}

std::string decodeString(const std::vector<uint8_t> &buffer, size_t &offset) {
    uint64_t size = decodeVariant(buffer, offset);
    std::string result =
        std::string(buffer.begin() + offset, buffer.begin() + offset + size);
    offset += size;
    return result;
}
