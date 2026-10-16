#pragma once

// Общие функции для тестов всех заданий лабораторной.

#include <bitset>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
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

// Проверяет, что результат НЕ совпадает с notExpected.
inline void checkNotEqual(const std::vector<uint8_t>& result, const std::vector<uint8_t>& notExpected) {
    std::cout << "  не должно быть:   "; printBits(notExpected);
    std::cout << "  результат:        "; printBits(result);
    if (result != notExpected) {
        std::cout << "  OK" << std::endl;
    } else {
        allPassed = false;
        std::cout << "  ОШИБКА" << std::endl;
    }
}

// Проверяет, что action() бросает исключение типа Expected (или его наследника).
//   action - любая функция без параметров; удобно передавать лямбду:
//   checkThrows<std::invalid_argument>("invalid_argument", [&] { net.encrypt(block); });
template <typename Expected, typename Action>
void checkThrows(const std::string& expectedName, Action action) {
    std::cout << "  правильный ответ: исключение " << expectedName << std::endl;
    try {
        action();
    } catch (const Expected& e) {
        std::cout << "  результат:        исключение " << expectedName << ": " << e.what() << std::endl;
        std::cout << "  OK" << std::endl;
        return;
    } catch (const std::exception& e) {
        allPassed = false;
        std::cout << "  результат:        исключение другого типа: " << e.what() << std::endl;
        std::cout << "  ОШИБКА" << std::endl;
        return;
    }
    allPassed = false;
    std::cout << "  результат:        исключения не было" << std::endl;
    std::cout << "  ОШИБКА" << std::endl;
}

// Заглушка для теста, который ещё не написан: тест считается непройденным.
inline void notWrittenYet() {
    allPassed = false;
    std::cout << "  ТЕСТ ЕЩЁ НЕ НАПИСАН" << std::endl;
}
