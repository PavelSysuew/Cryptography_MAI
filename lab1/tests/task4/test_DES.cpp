#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "DES.h"
#include "DesKeyExpansion.h"
#include "DesRoundFunction.h"
#include "testUtils.h"      // allPassed, printBits, checkEqual, checkThrows

using namespace std;

// Тесты задания 4: DES.
// Эталонные значения 1-8 - из статьи J. Orlin Grabbe «The DES Algorithm Illustrated»
// (ключ 133457799BBCDFF1, открытый текст 0123456789ABCDEF) и известного вектора
// ключ 0E329232EA6D0D73, текст 8787878787878787 -> шифртекст 0000000000000000.

const vector<uint8_t> KEY1   = {0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1};
const vector<uint8_t> PLAIN1 = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
const vector<uint8_t> CIPHER1 = {0x85, 0xE8, 0x13, 0x54, 0x0F, 0x0A, 0xB4, 0x05};

const vector<uint8_t> KEY2   = {0x0E, 0x32, 0x92, 0x32, 0xEA, 0x6D, 0x0D, 0x73};
const vector<uint8_t> PLAIN2 = {0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87};
const vector<uint8_t> CIPHER2 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Создаёт DES и сразу задаёт ключ.
DES makeDes(const vector<uint8_t>& key) {
    DES des;
    des.takeKey(key);
    return des;
}

// ============================================================================
//                    Части DES по отдельности (2.1 и 2.2)
// ============================================================================

// Расширение ключа: первый раундовый ключ K1.
void test1() {
    vector<vector<uint8_t>> keys = DesKeyExpansion().expandKey(KEY1);
    checkEqual(keys[0], {0x1B, 0x02, 0xEF, 0xFC, 0x70, 0x72});
}

// Расширение ключа: последний (16-й) раундовый ключ K16.
void test2() {
    vector<vector<uint8_t>> keys = DesKeyExpansion().expandKey(KEY1);
    checkEqual(keys[15], {0xCB, 0x3D, 0x8B, 0x0E, 0x17, 0xF5});
}

// Раундовая функция: F(R0, K1), где R0 - правая половина после IP.
void test3() {
    vector<uint8_t> result = DesRoundFunction().roundFunctionTransform(
        {0xF0, 0xAA, 0xF0, 0xAA}, {0x1B, 0x02, 0xEF, 0xFC, 0x70, 0x72});
    checkEqual(result, {0x23, 0x4A, 0xA9, 0xBB});
}

// ============================================================================
//                         Полный DES на эталонных векторах
// ============================================================================

// Вектор 1: шифрование.
void test4() {
    checkEqual(makeDes(KEY1).encrypt(PLAIN1), CIPHER1);
}

// Вектор 1: расшифрование.
void test5() {
    checkEqual(makeDes(KEY1).decrypt(CIPHER1), PLAIN1);
}

// Вектор 2: шифрование.
void test6() {
    checkEqual(makeDes(KEY2).encrypt(PLAIN2), CIPHER2);
}

// Вектор 2: расшифрование.
void test7() {
    checkEqual(makeDes(KEY2).decrypt(CIPHER2), PLAIN2);
}

// decrypt(encrypt(x)) == x на других данных.
void test8() {
    DES des = makeDes({0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18});
    vector<uint8_t> plain = {'C', 'r', 'y', 'p', 't', 'o', '!', '!'};
    checkEqual(des.decrypt(des.encrypt(plain)), plain);
}

// Слабый ключ 0101010101010101 (ЛК-2, п. 6): все 16 раундовых ключей одинаковы,
// поэтому шифрование - инволюция: encrypt(encrypt(x)) == x.
void test9() {
    DES des = makeDes({0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01});
    checkEqual(des.encrypt(des.encrypt(PLAIN1)), PLAIN1);
}

// ============================================================================
//                     Неправильное использование -> ошибка
// ============================================================================

// Ключ 7 байт вместо 8 -> invalid_argument.
void test10() {
    DES des;
    checkThrows<invalid_argument>("invalid_argument",
                                  [&] { des.takeKey({1, 2, 3, 4, 5, 6, 7}); });
}

// Блок 9 байт вместо 8 -> invalid_argument.
void test11() {
    DES des = makeDes(KEY1);
    checkThrows<invalid_argument>("invalid_argument",
                                  [&] { des.encrypt({1, 2, 3, 4, 5, 6, 7, 8, 9}); });
}

// encrypt без takeKey -> logic_error.
void test12() {
    DES des;
    checkThrows<logic_error>("logic_error", [&] { des.encrypt(PLAIN1); });
}

// Раундовая функция: половина 5 байт вместо 4 -> invalid_argument.
void test13() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        DesRoundFunction().roundFunctionTransform({1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 6});
    });
}

// ============================================================================
//                                Запуск тестов
// ============================================================================

void (*const TESTS[])() = {test1, test2, test3,  test4,  test5,  test6, test7,
                           test8, test9, test10, test11, test12, test13};
const int TEST_COUNT = 13;

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
