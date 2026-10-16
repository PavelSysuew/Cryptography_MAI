#include "DesKeyExpansion.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "bitsPermutation.h"
#include "desTables.h"

namespace {

// Половины C и D - по 28 бит.
const size_t HALF_BITS = 28;

// P-блок для циклического сдвига влево обеих половин C и D на shift бит
std::vector<size_t> leftShiftBlock(size_t shift) {
    std::vector<size_t> pBlock;
    // Половина C: биты 1..28
    for (size_t j = 0; j < HALF_BITS; ++j) {
        pBlock.push_back((j + shift) % HALF_BITS + 1);
    }
    // Половина D: биты 29..56
    for (size_t j = 0; j < HALF_BITS; ++j) {
        pBlock.push_back(HALF_BITS + (j + shift) % HALF_BITS + 1);
    }
    return pBlock;
}

}

std::vector<std::vector<uint8_t>> DesKeyExpansion::expandKey(
    const std::vector<uint8_t>& key) const {
    if (key.size() != 8) {
        throw std::invalid_argument("Ключ DES должен быть длиной 8 байт (64 бита)");
    }

    // Шаг 1-2: PC-1 убирает биты чётности и переставляет 56 бит.
    // Результат - 7 байт: первые 28 бит - C0, следующие 28 - D0.
    std::vector<uint8_t> cd = bitsPermutation(key, des::PC1, bitOrder::leftToRight, 1);

    std::vector<std::vector<uint8_t>> roundKeys;
    for (size_t round = 0; round < des::SHIFTS.size(); ++round) {
        // Шаг 3: циклический сдвиг C и D влево на 1 или 2 бита.
        cd = bitsPermutation(cd, leftShiftBlock(des::SHIFTS[round]), bitOrder::leftToRight, 1);

        // Шаг 4: PC-2 выбирает 48 бит из 56 - это раундовый ключ.
        roundKeys.push_back(bitsPermutation(cd, des::PC2, bitOrder::leftToRight, 1));
    }

    return roundKeys;
}
