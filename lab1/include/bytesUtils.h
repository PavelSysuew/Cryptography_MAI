#pragma once

#include <cstdint>
#include <vector>


// XOR двух векторов одинаковой длины
std::vector<uint8_t> xorBytes(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);