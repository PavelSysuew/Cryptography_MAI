#pragma once

// Общие функции для тестов всех заданий лабораторной.

#include <bitset>
#include <cstdint>
#include <iostream>
#include <vector>

// Становится false, если хотя бы одна проверка не прошла.
inline bool allPassed = true;

// Печатает байты в двоичном виде: 10110001 01001100
inline void printBits(const std::vector<uint8_t>& bytes) {
    for (uint8_t b : bytes) {
        std::cout << std::bitset<8>(b) << ' ';
    }
    std::cout << std::endl;
}

// Сравнивает результат с правильным ответом, печатает оба и итог (OK / ОШИБКА).
inline void checkEqual(const std::vector<uint8_t>& result, const std::vector<uint8_t>& expected) {
    std::cout << "  правильный ответ: "; printBits(expected);
    std::cout << "  результат:        "; printBits(result);
    if (result == expected) {
        std::cout << "  OK" << std::endl;
    } else {
        allPassed = false;
        std::cout << "  ОШИБКА" << std::endl;
    }
}
