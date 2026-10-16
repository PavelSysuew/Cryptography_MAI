#pragma once

// Вспомогательные функции для тестов задания 1 (перестановка битов).

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "bitsPermutation.h"
#include "testUtils.h"      // allPassed, printBits, checkEqual

// Печатает входные данные теста.
inline void printParams(const std::vector<uint8_t>& input, const std::vector<size_t>& pBlock,
                        bitOrder order, unsigned int firstElement) {
    std::cout << "  вход:             "; printBits(input);

    std::cout << "  pBlock:           {";
    for (size_t i = 0; i < pBlock.size(); ++i) {
        std::cout << (i == 0 ? "" : ", ") << pBlock[i];
    }
    std::cout << "}" << std::endl;

    std::cout << "  порядок:          "
              << (order == bitOrder::leftToRight ? "leftToRight" : "rightToLeft")
              << ", первый бит: " << firstElement << std::endl;
}

// Тест, где перестановка должна вернуть expected.
inline void checkPermutation(const std::vector<uint8_t>& input, const std::vector<size_t>& pBlock,
                             bitOrder order, unsigned int firstElement,
                             const std::vector<uint8_t>& expected) {
    printParams(input, pBlock, order, firstElement);
    std::vector<uint8_t> result = bitsPermutation(input, pBlock, order, firstElement);
    checkEqual(result, expected);
}

// Тест, где перестановка должна бросить invalid_argument.
inline void checkPermutationError(const std::vector<uint8_t>& input, const std::vector<size_t>& pBlock,
                                  bitOrder order, unsigned int firstElement) {
    printParams(input, pBlock, order, firstElement);
    std::cout << "  правильный ответ: исключение invalid_argument" << std::endl;
    try {
        std::vector<uint8_t> result = bitsPermutation(input, pBlock, order, firstElement);
        std::cout << "  результат:        "; printBits(result);
        allPassed = false;
        std::cout << "  ОШИБКА: исключения не было" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "  результат:        исключение invalid_argument: " << e.what() << std::endl;
        std::cout << "  OK" << std::endl;
    }
}
