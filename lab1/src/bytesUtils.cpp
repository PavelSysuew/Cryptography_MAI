#include <cstdint>
#include <vector>
#include <stdexcept>
#include "bytesUtils.h"
#include <cstddef>


std::vector<uint8_t> xorBytes(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Вектора должны быть одинаковой длины для операции XOR");
    }

    std::vector<uint8_t> result(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        result[i] = a[i] ^ b[i];
    }
    return result;
}