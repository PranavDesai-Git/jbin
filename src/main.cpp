#include "Tag.hpp"
#include "Variant.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
    size_t offset = 0;
    std::vector<uint8_t> buffer;
    encodeString(buffer, 1, "Pranav");
    encodeTag(buffer, 2, wiretype::Varint);
    encodeVariant(buffer, 100);

    while (offset < buffer.size()) {
        uint32_t fieldNumber;
        wiretype type;
        decodeTag(buffer, offset, fieldNumber, type);
        switch (type) {
        case wiretype::Varint: {
            uint64_t val = decodeVariant(buffer, offset);
            std::cout << "Read Field " << fieldNumber << " (Varint): " << val
                      << std::endl;
            break;
        }
        case wiretype::Delimited: {
            std::string text = decodeString(buffer, offset);
            std::cout << "Read Field " << fieldNumber << " (String): " << text
                      << std::endl;
            break;
        }
        default:
            std::cerr << "Unknown wire type!" << std::endl;
            return 1;
        }
    }

    return 0;
}
