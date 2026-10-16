#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CipherContext.h"
#include "DES.h"
#include "bytesUtils.h"
#include "testUtils.h"      // allPassed, printBits, checkEqual, checkNotEqual, checkThrows

using namespace std;

// Тесты задания 2.4, шаг 2: режимы шифрования (ЛК-3, п. 2-8) на DES.
//
// Тесты 1-9 сверяют результат контекста с формулой режима, посчитанной вручную
// через DES на двух блоках. Набивка Zeros, данные ровно 16 байт - набивка не добавляется.

const vector<uint8_t> KEY = {0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1};
const vector<uint8_t> IV  = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

const vector<uint8_t> M1 = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
const vector<uint8_t> M2 = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

// Данные из двух блоков: M1 || M2.
vector<uint8_t> twoBlocks(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    vector<uint8_t> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

// DES с ключом KEY - для вычисления эталонов по формулам.
DES makeDes() {
    DES des;
    des.takeKey(KEY);
    return des;
}

// Шифрует через контекст и ждёт результата.
vector<uint8_t> encryptWith(CipherMode mode, PaddingMode padding, const vector<uint8_t>& data,
                            const vector<uint8_t>& key = KEY) {
    CipherContext context(make_shared<DES>(), key, mode, padding, IV);
    vector<uint8_t> result;
    context.encrypt(data, result).get();
    return result;
}

// Дешифрует через контекст и ждёт результата.
vector<uint8_t> decryptWith(CipherMode mode, PaddingMode padding, const vector<uint8_t>& data,
                            const vector<uint8_t>& key = KEY) {
    CipherContext context(make_shared<DES>(), key, mode, padding, IV);
    vector<uint8_t> result;
    context.decrypt(data, result).get();
    return result;
}

// Шифрует и дешифрует обратно, результат должен совпасть с исходными данными.
void checkRoundTrip(CipherMode mode, PaddingMode padding, const vector<uint8_t>& data) {
    vector<uint8_t> cipherText = encryptWith(mode, padding, data);
    cout << "  шифртекст:        "; printBits(cipherText);
    checkEqual(decryptWith(mode, padding, cipherText), data);
}

// 20 байт: 2 полных блока + 4 байта (набивка дополнит до 24).
const vector<uint8_t> TEXT = {'S', 'y', 's', 'u', 'e', 'v', ' ', 'P', 'a', 'v',
                              'e', 'l', ',', ' ', 'M', 'A', 'I', ' ', '8', '!'};

// ============================================================================
//                   Режимы: результат совпадает с формулой
// ============================================================================

// ECB: c_i = E(m_i).
void test1() {
    DES des = makeDes();
    checkEqual(encryptWith(CipherMode::ECB, PaddingMode::Zeros, twoBlocks(M1, M2)),
               twoBlocks(des.encrypt(M1), des.encrypt(M2)));
}

// CBC: c_1 = E(m_1 xor IV), c_2 = E(m_2 xor c_1).
void test2() {
    DES des = makeDes();
    vector<uint8_t> c1 = des.encrypt(xorBytes(M1, IV));
    vector<uint8_t> c2 = des.encrypt(xorBytes(M2, c1));
    checkEqual(encryptWith(CipherMode::CBC, PaddingMode::Zeros, twoBlocks(M1, M2)), twoBlocks(c1, c2));
}

// PCBC: c_1 = E(m_1 xor IV), c_2 = E(m_2 xor m_1 xor c_1).
void test3() {
    DES des = makeDes();
    vector<uint8_t> c1 = des.encrypt(xorBytes(M1, IV));
    vector<uint8_t> c2 = des.encrypt(xorBytes(xorBytes(M2, M1), c1));
    checkEqual(encryptWith(CipherMode::PCBC, PaddingMode::Zeros, twoBlocks(M1, M2)), twoBlocks(c1, c2));
}

// CFB: c_1 = m_1 xor E(IV), c_2 = m_2 xor E(c_1).
void test4() {
    DES des = makeDes();
    vector<uint8_t> c1 = xorBytes(M1, des.encrypt(IV));
    vector<uint8_t> c2 = xorBytes(M2, des.encrypt(c1));
    checkEqual(encryptWith(CipherMode::CFB, PaddingMode::Zeros, twoBlocks(M1, M2)), twoBlocks(c1, c2));
}

// OFB: s_1 = E(IV), s_2 = E(s_1), c_i = m_i xor s_i.
void test5() {
    DES des = makeDes();
    vector<uint8_t> s1 = des.encrypt(IV);
    vector<uint8_t> s2 = des.encrypt(s1);
    checkEqual(encryptWith(CipherMode::OFB, PaddingMode::Zeros, twoBlocks(M1, M2)),
               twoBlocks(xorBytes(M1, s1), xorBytes(M2, s2)));
}

// CTR: c_1 = m_1 xor E(IV), c_2 = m_2 xor E(IV + 1).
void test6() {
    DES des = makeDes();
    vector<uint8_t> ivPlusOne = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x81};
    checkEqual(encryptWith(CipherMode::CTR, PaddingMode::Zeros, twoBlocks(M1, M2)),
               twoBlocks(xorBytes(M1, des.encrypt(IV)), xorBytes(M2, des.encrypt(ivPlusOne))));
}

// Random Delta: delta = правая половина IV = 50 60 70 80, младший бит -> 1: 50 60 70 81.
// c_1 = m_1 xor E(IV), c_2 = m_2 xor E(IV + delta), IV + delta = 10 20 30 40 A0 C0 E1 01.
void test7() {
    DES des = makeDes();
    vector<uint8_t> ivPlusDelta = {0x10, 0x20, 0x30, 0x40, 0xA0, 0xC0, 0xE1, 0x01};
    checkEqual(encryptWith(CipherMode::RandomDelta, PaddingMode::Zeros, twoBlocks(M1, M2)),
               twoBlocks(xorBytes(M1, des.encrypt(IV)), xorBytes(M2, des.encrypt(ivPlusDelta))));
}

// ECB: одинаковые блоки открытого текста -> одинаковые блоки шифртекста (ЛК-3, п. 2).
void test8() {
    vector<uint8_t> c = encryptWith(CipherMode::ECB, PaddingMode::Zeros, twoBlocks(M1, M1));
    checkEqual(vector<uint8_t>(c.begin() + 8, c.end()), vector<uint8_t>(c.begin(), c.begin() + 8));
}

// CBC: одинаковые блоки открытого текста -> разные блоки шифртекста (ЛК-3, п. 3).
void test9() {
    vector<uint8_t> c = encryptWith(CipherMode::CBC, PaddingMode::Zeros, twoBlocks(M1, M1));
    checkNotEqual(vector<uint8_t>(c.begin() + 8, c.end()), vector<uint8_t>(c.begin(), c.begin() + 8));
}

// ============================================================================
//            Шифрование + дешифрование возвращает исходные данные
// ============================================================================

void test10() { checkRoundTrip(CipherMode::ECB,         PaddingMode::PKCS7, TEXT); }
void test11() { checkRoundTrip(CipherMode::CBC,         PaddingMode::PKCS7, TEXT); }
void test12() { checkRoundTrip(CipherMode::PCBC,        PaddingMode::PKCS7, TEXT); }
void test13() { checkRoundTrip(CipherMode::CFB,         PaddingMode::PKCS7, TEXT); }
void test14() { checkRoundTrip(CipherMode::OFB,         PaddingMode::PKCS7, TEXT); }
void test15() { checkRoundTrip(CipherMode::CTR,         PaddingMode::PKCS7, TEXT); }
void test16() { checkRoundTrip(CipherMode::RandomDelta, PaddingMode::PKCS7, TEXT); }

// Другие набивки (в режиме CBC).
void test17() { checkRoundTrip(CipherMode::CBC, PaddingMode::Zeros,    TEXT); }
void test18() { checkRoundTrip(CipherMode::CBC, PaddingMode::AnsiX923, TEXT); }
void test19() { checkRoundTrip(CipherMode::CBC, PaddingMode::ISO10126, TEXT); }

// Пустые данные: после набивки PKCS7 - один блок, после дешифрования - снова пусто.
void test20() { checkRoundTrip(CipherMode::CBC, PaddingMode::PKCS7, {}); }

// ============================================================================
//                     Неправильное использование -> ошибка
// ============================================================================

// CBC без IV -> invalid_argument.
void test21() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        CipherContext context(make_shared<DES>(), KEY, CipherMode::CBC, PaddingMode::PKCS7);
    });
}

// IV длиной 7 байт вместо 8 -> invalid_argument.
void test22() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        CipherContext context(make_shared<DES>(), KEY, CipherMode::CBC, PaddingMode::PKCS7,
                              vector<uint8_t>{1, 2, 3, 4, 5, 6, 7});
    });
}

// Шифртекст 13 байт (не кратен блоку) -> invalid_argument из future.get().
void test23() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        decryptWith(CipherMode::CBC, PaddingMode::PKCS7, vector<uint8_t>(13, 0xAB));
    });
}

// Дешифрование другим ключом: на выходе мусор, набивка PKCS7 не сходится -> invalid_argument.
void test24() {
    vector<uint8_t> cipherText = encryptWith(CipherMode::CBC, PaddingMode::PKCS7, TEXT);
    vector<uint8_t> wrongKey = {0x0E, 0x32, 0x92, 0x32, 0xEA, 0x6D, 0x0D, 0x73};
    checkThrows<invalid_argument>("invalid_argument", [&] {
        decryptWith(CipherMode::CBC, PaddingMode::PKCS7, cipherText, wrongKey);
    });
}

// ============================================================================
//         Большие данные: параллельная обработка (от 512 блоков)
// ============================================================================

// 64 КБ = 8192 блока DES - блоки делятся между потоками.
vector<uint8_t> bigData() {
    vector<uint8_t> data(64 * 1024);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i * 31 + 7);
    }
    return data;
}

// ECB параллельно == поблочно по порядку (потоки не перепутали блоки).
void test25() {
    vector<uint8_t> data = bigData();
    DES des = makeDes();
    vector<uint8_t> expected;
    for (size_t offset = 0; offset < data.size(); offset += 8) {
        vector<uint8_t> block = des.encrypt(vector<uint8_t>(data.begin() + offset, data.begin() + offset + 8));
        expected.insert(expected.end(), block.begin(), block.end());
    }
    vector<uint8_t> result = encryptWith(CipherMode::ECB, PaddingMode::Zeros, data);
    cout << "  блоков: " << data.size() / 8 << endl;
    checkEqual(vector<uint8_t>(result.end() - 16, result.end()), vector<uint8_t>(expected.end() - 16, expected.end()));
    cout << "  весь шифртекст совпадает: " << (result == expected ? "да" : "НЕТ") << endl;
    if (result != expected) allPassed = false;
}

// CTR параллельно == последовательно: счётчик увеличивается на 1 от блока к блоку.
void test26() {
    vector<uint8_t> data = bigData();
    DES des = makeDes();
    vector<uint8_t> counter = IV;
    vector<uint8_t> expected;
    for (size_t offset = 0; offset < data.size(); offset += 8) {
        vector<uint8_t> block = xorBytes(vector<uint8_t>(data.begin() + offset, data.begin() + offset + 8),
                                         des.encrypt(counter));
        expected.insert(expected.end(), block.begin(), block.end());
        for (size_t i = counter.size(); i-- > 0 && ++counter[i] == 0;) {}   // counter += 1
    }
    vector<uint8_t> result = encryptWith(CipherMode::CTR, PaddingMode::Zeros, data);
    checkEqual(vector<uint8_t>(result.end() - 16, result.end()), vector<uint8_t>(expected.end() - 16, expected.end()));
    cout << "  весь шифртекст совпадает: " << (result == expected ? "да" : "НЕТ") << endl;
    if (result != expected) allPassed = false;
}

// Шифрование + параллельное дешифрование большого объёма (печатаем только последние 16 байт).
void checkBigRoundTrip(CipherMode mode) {
    vector<uint8_t> data = bigData();
    vector<uint8_t> result = decryptWith(mode, PaddingMode::PKCS7, encryptWith(mode, PaddingMode::PKCS7, data));
    checkEqual(vector<uint8_t>(result.end() - 16, result.end()), vector<uint8_t>(data.end() - 16, data.end()));
    cout << "  все данные совпадают: " << (result == data ? "да" : "НЕТ") << endl;
    if (result != data) allPassed = false;
}

void test27() { checkBigRoundTrip(CipherMode::CBC); }
void test28() { checkBigRoundTrip(CipherMode::CFB); }
void test29() { checkBigRoundTrip(CipherMode::RandomDelta); }

// ============================================================================
//                                   Файлы
// ============================================================================

// Файл -> зашифрованный файл -> расшифрованный файл: содержимое совпадает с исходным.
void test30() {
    const string plainPath = "test_context_plain.bin";
    const string cipherPath = "test_context_cipher.bin";
    const string decryptedPath = "test_context_decrypted.bin";

    vector<uint8_t> data = bigData();
    data.resize(10000);                         // не кратно 8 - нужна набивка
    {
        ofstream file(plainPath, ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    }

    CipherContext context(make_shared<DES>(), KEY, CipherMode::CBC, PaddingMode::PKCS7, IV);
    context.encrypt(plainPath, cipherPath).get();
    context.decrypt(cipherPath, decryptedPath).get();

    ifstream file(decryptedPath, ios::binary);
    vector<uint8_t> decrypted((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    checkEqual(vector<uint8_t>(decrypted.end() - 8, decrypted.end()), vector<uint8_t>(data.end() - 8, data.end()));
    cout << "  размер: " << data.size() << " -> " << decrypted.size()
         << ", файл совпадает: " << (decrypted == data ? "да" : "НЕТ") << endl;
    if (decrypted != data) allPassed = false;

    remove(plainPath.c_str());
    remove(cipherPath.c_str());
    remove(decryptedPath.c_str());
}

// Несуществующий входной файл -> runtime_error из future.get().
void test31() {
    CipherContext context(make_shared<DES>(), KEY, CipherMode::CBC, PaddingMode::PKCS7, IV);
    checkThrows<runtime_error>("runtime_error", [&] {
        context.encrypt(string("no_such_file.bin"), string("out.bin")).get();
    });
}

// ============================================================================
//                                Запуск тестов
// ============================================================================

void (*const TESTS[])() = {test1,  test2,  test3,  test4,  test5,  test6,  test7,  test8,
                           test9,  test10, test11, test12, test13, test14, test15, test16,
                           test17, test18, test19, test20, test21, test22, test23, test24,
                           test25, test26, test27, test28, test29, test30, test31};
const int TEST_COUNT = 31;

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
