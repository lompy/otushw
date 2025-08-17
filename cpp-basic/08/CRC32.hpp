#pragma once
#include <cstddef>
#include <cstdint>
#include <istream>

// CRC-32-IEEE 802.3 (0xEDB88320).
uint32_t crc32(const char *bytes, size_t bytesCount, uint32_t prev = ~0u);
uint32_t crc32(std::istream& in, uint32_t prev = ~0u);
