#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

void encodeVariant(std::vector<uint8_t> &buffer, uint64_t value);
uint64_t decodeVariant(const std::vector<uint8_t> &buffer, size_t &offset);

#endif
