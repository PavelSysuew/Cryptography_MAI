#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "FeistelNetwork.h"
#include "IKeyExpansion.h"
#include "IRoundFunction.h"
#include "testUtils.h"      // allPassed, printBits, checkEqual, checkNotEqual, checkThrows

using namespace std;

// Тесты задания 3: сеть Фейстеля.
// Сеть обратима при ЛЮБОЙ функции F, поэтому здесь используются игрушечные
// реализации интерфейсов 2.1 и 2.2, а не DES.

// ============================================================================
//                      Игрушечные реализации интерфейсов
// ============================================================================

// Игрушечная F (интерфейс 2.2): out[i] = half[i] * 3 + key[i % длина ключа] + 1.
// Арифметика по модулю 256 (uint8_t переполняется сам). Длина выхода = длина входа.
// F НЕ обязана быть обратимой - сеть Фейстеля обратима и без этого.
class ToyRoundFunction : public IRoundFunction {
public:
    vector<uint8_t> roundFunctionTransform(const vector<uint8_t>& half,
                                           const vector<uint8_t>& roundKey) const override {
        vector<uint8_t> out(half.size());
        for (size_t i = 0; i < half.size(); ++i) {
            out[i] = static_cast<uint8_t>(half[i] * 3 + roundKey[i % roundKey.size()] + 1);
        }
        return out;
    }
};

// Игрушечное расширение ключа (интерфейс 2.1): rounds РАЗНЫХ ключей,
// в раунде r каждый байт ключа увеличен на 17 * r.
// Ключи обязательно разные: при одинаковых ключах тест не заметил бы,
// что decrypt берёт ключи не в обратном порядке.
class ToyKeyExpansion : public IKeyExpansion {
public:
    explicit ToyKeyExpansion(size_t rounds) : rounds_(rounds) {}

    vector<vector<uint8_t>> expandKey(const vector<uint8_t>& key) const override {
        vector<vector<uint8_t>> roundKeys;
        for (size_t r = 0; r < rounds_; ++r) {
            vector<uint8_t> roundKey = key;
            for (uint8_t& b : roundKey) {
                b = static_cast<uint8_t>(b + 17 * r);
            }
            roundKeys.push_back(roundKey);
        }
        return roundKeys;
    }

private:
    size_t rounds_;
};

// Создаёт сеть с игрушечными F и расширением ключа и сразу задаёт ключ.
FeistelNetwork makeNetwork(size_t blockSize, size_t rounds, const vector<uint8_t>& key) {
    FeistelNetwork net(make_shared<ToyKeyExpansion>(rounds), make_shared<ToyRoundFunction>(), blockSize);
    net.takeKey(key);
    return net;
}

// ============================================================================
//                        Тесты: шифрование и расшифрование
// ============================================================================

// Расшифрование возвращает исходный блок. Блок 8 байт (как в DES), 4 раунда.
void test1() {
    FeistelNetwork net = makeNetwork(8, 4, {1, 2, 3, 4});
    vector<uint8_t> plain = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

    vector<uint8_t> cipher = net.encrypt(plain);
    cout << "  шифртекст:        "; printBits(cipher);
    checkEqual(net.decrypt(cipher), plain);
}

// Расшифрование возвращает исходный блок. Блок 16 байт (как в DEAL), 8 раундов.
void test2() {
    FeistelNetwork net = makeNetwork(16, 8, {9, 8, 7, 6, 5, 4, 3, 2});
    vector<uint8_t> plain = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                             0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    vector<uint8_t> cipher = net.encrypt(plain);
    cout << "  шифртекст:        "; printBits(cipher);
    checkEqual(net.decrypt(cipher), plain);
}

// Шифрование действительно меняет блок: encrypt(x) != x.
void test3() {
    FeistelNetwork net = makeNetwork(8, 4, {1, 2, 3, 4});
    vector<uint8_t> plain = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

    checkNotEqual(net.encrypt(plain), plain);
}

// Эталон, посчитанный вручную. Блок 2 байта: L = 0x12, R = 0x34. 1 раунд, ключ {0x05}.
//   F(R, k) = 0x34 * 3 + 0x05 + 1 = 0x9C + 0x06 = 0xA2
//   новая R = L xor F = 0x12 xor 0xA2 = 0xB0,  новая L = старая R = 0x34
//   выход = R || L (без финального обмена) = 0xB0 0x34
// Проверяет и формулу раунда, и порядок половин на выходе.
void test4() {
    FeistelNetwork net = makeNetwork(2, 1, {0x05});

    checkEqual(net.encrypt({0x12, 0x34}), {0xB0, 0x34});
}

// Обратная проверка эталона из test4: decrypt(0xB0 0x34) = 0x12 0x34.
void test5() {
    FeistelNetwork net = makeNetwork(2, 1, {0x05});

    checkEqual(net.decrypt({0xB0, 0x34}), {0x12, 0x34});
}

// ============================================================================
//                     Тесты: неправильное использование -> ошибка
// ============================================================================

// encrypt без takeKey: раундовых ключей нет -> logic_error.
void test6() {
    FeistelNetwork net(make_shared<ToyKeyExpansion>(4), make_shared<ToyRoundFunction>(), 8);
    vector<uint8_t> plain = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

    checkThrows<logic_error>("logic_error", [&] { net.encrypt(plain); });
}

// Блок неверной длины (7 байт вместо 8) -> invalid_argument.
void test7() {
    FeistelNetwork net = makeNetwork(8, 4, {1, 2, 3, 4});
    vector<uint8_t> shortBlock = {1, 2, 3, 4, 5, 6, 7};

    checkThrows<invalid_argument>("invalid_argument", [&] { net.encrypt(shortBlock); });
}

// Конструктор: пустой указатель на расширение ключа -> invalid_argument.
void test8() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        FeistelNetwork net(nullptr, make_shared<ToyRoundFunction>(), 8);
    });
}

// Конструктор: пустой указатель на раундовую функцию -> invalid_argument.
void test9() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        FeistelNetwork net(make_shared<ToyKeyExpansion>(4), nullptr, 8);
    });
}

// Конструктор: нечётный размер блока (не делится пополам) -> invalid_argument.
void test10() {
    checkThrows<invalid_argument>("invalid_argument", [&] {
        FeistelNetwork net(make_shared<ToyKeyExpansion>(4), make_shared<ToyRoundFunction>(), 7);
    });
}

// ============================================================================
//                                Запуск тестов
// ============================================================================

void (*const TESTS[])() = {test1, test2, test3, test4, test5, test6, test7, test8, test9, test10};
const int TEST_COUNT = 10;

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
