#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class wiretype : uint8_t {
    Varint = 0,
    Fixed32 = 1,
    Fixed64 = 2,
    Delimited = 3
};

void encodeTag(std::vector<uint8_t> &buffer, uint32_t fieldnumber,
               wiretype wiretype);

void decodeTag(const std::vector<uint8_t> &buffer, size_t &offset,
               uint32_t &outFieldNumber, wiretype &outType);

void encodeString(std::vector<uint8_t> &buffer, uint32_t fieldNumber,
                  const std::string &text);

std::string decodeString(const std::vector<uint8_t> &buffer, size_t &offset);
