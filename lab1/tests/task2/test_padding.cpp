#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "padding.h"
#include "testUtils.h"      // allPassed, printBits, checkEqual, checkThrows

using namespace std;

// Тесты задания 2.4, шаг 1: набивки (ЛК-3, п. 9). Блок 8 байт.

const size_t BLOCK = 8;
const vector<uint8_t> FIVE  = {0x01, 0x02, 0x03, 0x04, 0x05};                    // не хватает 3 байт
const vector<uint8_t> EIGHT = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};  // ровно блок

// ============================================================================
//                              Добавление набивки
// ============================================================================

// Zeros: 5 байт -> дописываются 3 нуля.
void test1() {
    checkEqual(addPadding(FIVE, BLOCK, PaddingMode::Zeros),
               {0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x00});
}

// Zeros: ровно блок -> ничего не дописывается.
void test2() {
    checkEqual(addPadding(EIGHT, BLOCK, PaddingMode::Zeros), EIGHT);
}

// ANSI X.923: 5 байт -> 00 00 03.
void test3() {
    checkEqual(addPadding(FIVE, BLOCK, PaddingMode::AnsiX923),
               {0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x03});
}

// ANSI X.923: ровно блок -> целый блок набивки 00 .. 00 08.
void test4() {
    checkEqual(addPadding(EIGHT, BLOCK, PaddingMode::AnsiX923),
               {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08});
}

// PKCS7: 5 байт -> 03 03 03.
void test5() {
    checkEqual(addPadding(FIVE, BLOCK, PaddingMode::PKCS7),
               {0x01, 0x02, 0x03, 0x04, 0x05, 0x03, 0x03, 0x03});
}

// PKCS7: ровно блок -> целый блок 08 08 .. 08.
void test6() {
    checkEqual(addPadding(EIGHT, BLOCK, PaddingMode::PKCS7),
               {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08});
}

// PKCS7: пустые данные -> один блок 08 08 .. 08.
void test7() {
    checkEqual(addPadding({}, BLOCK, PaddingMode::PKCS7),
               {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08});
}

// ISO 10126: 5 байт -> ?? ?? 03. Средние байты случайные, поэтому проверяем
// данные (первые 5 байт) и последний байт; середину заменяем нулями.
void test8() {
    vector<uint8_t> padded = addPadding(FIVE, BLOCK, PaddingMode::ISO10126);
    if (padded.size() == BLOCK) {
        padded[5] = 0x00;
        padded[6] = 0x00;
    }
    checkEqual(padded, {0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x03});
}

// ============================================================================
//                               Снятие набивки
// ============================================================================

// Zeros: нули в конце отрезаются.
void test9() {
    checkEqual(removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x00}, BLOCK, PaddingMode::Zeros),
               FIVE);
}

// ANSI X.923: 00 00 03 отрезается.
void test10() {
    checkEqual(removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x03}, BLOCK, PaddingMode::AnsiX923),
               FIVE);
}

// PKCS7: 03 03 03 отрезается.
void test11() {
    checkEqual(removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x03, 0x03, 0x03}, BLOCK, PaddingMode::PKCS7),
               FIVE);
}

// PKCS7: целый блок набивки отрезается, остаются исходные 8 байт.
void test12() {
    checkEqual(removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                              0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, BLOCK, PaddingMode::PKCS7),
               EIGHT);
}

// ISO 10126: случайные байты + длина отрезаются (добавили и сразу сняли).
void test13() {
    vector<uint8_t> padded = addPadding(FIVE, BLOCK, PaddingMode::ISO10126);
    checkEqual(removePadding(padded, BLOCK, PaddingMode::ISO10126), FIVE);
}

// ============================================================================
//                       Повреждённая набивка -> ошибка
// ============================================================================

// PKCS7: последний байт 03, но перед ним не 03 -> invalid_argument.
void test14() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x03, 0x07, 0x03}, BLOCK, PaddingMode::PKCS7);
    });
}

// ANSI X.923: последний байт 03, но перед ним не нули -> invalid_argument.
void test15() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x07, 0x03}, BLOCK, PaddingMode::AnsiX923);
    });
}

// Длина набивки в последнем байте больше блока (09 > 8) -> invalid_argument.
void test16() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09}, BLOCK, PaddingMode::PKCS7);
    });
}

// Длина данных не кратна блоку (7 байт) -> invalid_argument.
void test17() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        removePadding({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}, BLOCK, PaddingMode::PKCS7);
    });
}

// ============================================================================
//                                Запуск тестов
// ============================================================================

void (*const TESTS[])() = {test1,  test2,  test3,  test4,  test5,  test6,  test7,  test8,  test9,
                           test10, test11, test12, test13, test14, test15, test16, test17};
const int TEST_COUNT = 17;

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
