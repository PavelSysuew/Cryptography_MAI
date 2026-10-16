#include "DesRoundFunction.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "bitsPermutation.h"
#include "bytesUtils.h"
#include "desTables.h"

namespace {

/**
 * Подстановка через 8 S-блоков
 */
std::vector<uint8_t> substitute(const std::vector<uint8_t>& bytes48) {
    // Складываем 6 байт в одно число: первый байт - в старших битах
    uint64_t bits = 0;
    for (uint8_t b : bytes48) {
        bits = (bits << 8) | b;
    }

    uint32_t out = 0;
    for (size_t k = 0; k < 8; ++k) {
        uint8_t group = static_cast<uint8_t>((bits >> (42 - 6 * k)) & 0x3F);

        uint8_t row = static_cast<uint8_t>(((group >> 4) & 0b10) | (group & 0b01));  // b1 b6
        uint8_t col = static_cast<uint8_t>((group >> 1) & 0x0F);                     // b2 b3 b4 b5

        out = (out << 4) | des::S[k][row][col];
    }

    return {
        static_cast<uint8_t>(out >> 24),
        static_cast<uint8_t>(out >> 16),
        static_cast<uint8_t>(out >> 8),
        static_cast<uint8_t>(out),
    };
}

}

std::vector<uint8_t> DesRoundFunction::roundFunctionTransform(const std::vector<uint8_t>& input,
    const std::vector<uint8_t>& roundKey) const {
    if (input.size() != 4) {
        throw std::invalid_argument("Входной блок должен быть длиной 4 байта (32 бита)");
    }
    if (roundKey.size() != 6) {
        throw std::invalid_argument("Ключ раунда должен быть длиной 6 байт (48 бит)");
    }

    // Шаг 1: расширение E, 32 -> 48 бит (часть битов повторяется).
    std::vector<uint8_t> expanded = bitsPermutation(input, des::E, bitOrder::leftToRight, 1);

    // Шаг 2: XOR с раундовым ключом.
    std::vector<uint8_t> mixed = xorBytes(expanded, roundKey);

    // Шаг 3: S-блоки, 48 -> 32 бита.
    std::vector<uint8_t> substituted = substitute(mixed);

    // Шаг 4: перестановка P, 32 -> 32 бита.
    return bitsPermutation(substituted, des::P, bitOrder::leftToRight, 1);
}
