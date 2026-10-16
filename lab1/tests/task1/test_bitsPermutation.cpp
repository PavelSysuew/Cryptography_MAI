#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "bitsPermutation.h"
#include "test_bitsPermutation.h"

using namespace std;


// Вход для большинства тестов: 10110001 01001100
//   номера битов слева направо (с 1):  1  2  3  4  5  6  7  8 |  9 10 11 12 13 14 15 16
//   номера битов справа налево (с 1):  8  7  6  5  4  3  2  1 | 16 15 14 13 12 11 10  9
const vector<uint8_t> INPUT = {0b10110001, 0b01001100};

/**
 * Перестановка должна сработать!
 */



// Слева направо, с 1: сначала чётные биты, потом нечётные.
void test1() {
    checkPermutation(INPUT, {2, 4, 6, 8, 10, 12, 14, 16, 1, 3, 5, 7, 9, 11, 13, 15},
                     bitOrder::leftToRight, 1,
                     {0b01011010, 0b11000010});
}

// Слева направо, с 1: все 16 бит в обратном порядке.
void test2() {
    checkPermutation(INPUT, {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
                     bitOrder::leftToRight, 1,
                     {0b00110010, 0b10001101});
}

// Слева направо, с 0: как test1, номера на 1 меньше - результат тот же.
void test3() {
    checkPermutation(INPUT, {1, 3, 5, 7, 9, 11, 13, 15, 0, 2, 4, 6, 8, 10, 12, 14},
                     bitOrder::leftToRight, 0,
                     {0b01011010, 0b11000010});
}

// Слева направо, с 0: как test2, номера на 1 меньше - результат тот же.
void test4() {
    checkPermutation(INPUT, {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                     bitOrder::leftToRight, 0,
                     {0b00110010, 0b10001101});
}

// Справа налево, с 1: сначала чётные биты, потом нечётные.
void test5() {
    checkPermutation(INPUT, {2, 4, 6, 8, 10, 12, 14, 16, 1, 3, 5, 7, 9, 11, 13, 15},
                     bitOrder::rightToLeft, 1,
                     {0b00110100, 0b10100101});
}

// Справа налево, с 0: как test5, номера на 1 меньше - результат тот же.
void test6() {
    checkPermutation(INPUT, {1, 3, 5, 7, 9, 11, 13, 15, 0, 2, 4, 6, 8, 10, 12, 14},
                     bitOrder::rightToLeft, 0,
                     {0b00110100, 0b10100101});
}

// Сжатие 16 -> 8 бит, слева направо: только чётные биты.
void test7() {
    checkPermutation(INPUT, {2, 4, 6, 8, 10, 12, 14, 16},
                     bitOrder::leftToRight, 1,
                     {0b01011010});
}

// Сжатие 16 -> 8 бит, справа налево: только чётные биты.
void test8() {
    checkPermutation(INPUT, {2, 4, 6, 8, 10, 12, 14, 16},
                     bitOrder::rightToLeft, 1,
                     {0b00110100});
}

// Сжатие 16 -> 4 бита: результат - один байт, младшие 4 бита нулевые.
void test9() {
    checkPermutation(INPUT, {1, 5, 9, 13},
                     bitOrder::leftToRight, 1,
                     {0b10010000});
}

// Сжатие 16 -> 1 бит.
void test10() {
    checkPermutation(INPUT, {1},
                     bitOrder::leftToRight, 1,
                     {0b10000000});
}

// Расширение 8 -> 20 бит, слева направо: каждый бит дважды, потом ещё биты 1..4.
void test11() {
    checkPermutation({0b10110001}, {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 1, 2, 3, 4},
                     bitOrder::leftToRight, 1,
                     {0b11001111, 0b00000011, 0b10110000});
}

// Расширение 8 -> 20 бит, справа налево: каждый бит дважды, потом ещё биты 1..4.
void test12() {
    checkPermutation({0b10110001}, {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 1, 2, 3, 4},
                     bitOrder::rightToLeft, 1,
                     {0b11000000, 0b11110011, 0b10000000});
}

/**
 * Перестановка должна выдать ошибку!
 */



// Пустой pBlock.
void test13() {
    checkPermutationError(INPUT, {}, bitOrder::leftToRight, 0);
}

// Пустой вход: ни одного бита нет.
void test14() {
    checkPermutationError({}, {0}, bitOrder::leftToRight, 0);
}

// Нумерация с 0: во входе биты 0..15, бита 16 нет.
void test15() {
    checkPermutationError(INPUT, {16}, bitOrder::leftToRight, 0);
}

// Нумерация с 1: во входе биты 1..16, бита 17 нет.
void test16() {
    checkPermutationError(INPUT, {17}, bitOrder::rightToLeft, 1);
}

// Несуществующий бит в середине pBlock.
void test17() {
    checkPermutationError(INPUT, {1, 99, 2}, bitOrder::leftToRight, 1);
}

// Нумерация с 1: бита с номером 0 нет.
void test18() {
    checkPermutationError(INPUT, {0}, bitOrder::leftToRight, 1);
}

// Огромный номер бита (так выглядит -1, приведённое к size_t).
void test19() {
    checkPermutationError(INPUT, {SIZE_MAX}, bitOrder::leftToRight, 1);
}

// firstElement может быть только 0 или 1.
void test20() {
    checkPermutationError(INPUT, {1}, bitOrder::leftToRight, 2);
}

// ============================================================================
//                                Запуск тестов
// ============================================================================

void (*const TESTS[])() = {
    test1,  test2,  test3,  test4,  test5,  test6,  test7,  test8,  test9,  test10,
    test11, test12, test13, test14, test15, test16, test17, test18, test19, test20,
};
const int TEST_COUNT = 20;

void runTest(int number) {
    cout << "Тест " << number << endl;
    try {
        TESTS[number - 1]();
    } catch (const exception& e) {
        allPassed = false;
        cout << "  ОШИБКА: неожиданное исключение: " << e.what() << endl;
    }
    cout << endl;
}

// Без аргументов - все тесты, с аргументом N - только тест N (так их запускает ctest).
int main(int argc, char** argv) {
    if (argc > 1) {
        int number = atoi(argv[1]);     // для нечисла вернёт 0
        if (number < 1 || number > TEST_COUNT) {
            cout << "Нет теста с номером " << argv[1] << endl;
            return 2;
        }
        runTest(number);
    } else {
        for (int i = 1; i <= TEST_COUNT; ++i) {
            runTest(i);
        }
    }

    cout << (allPassed ? "Все проверки пройдены" : "ЕСТЬ ОШИБКИ") << endl;
    return allPassed ? 0 : 1;           // ctest считает тест упавшим, если код возврата не 0
}
