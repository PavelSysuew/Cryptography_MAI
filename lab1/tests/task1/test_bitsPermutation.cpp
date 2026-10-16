#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitsPermutation.h"

using namespace std;

// Минимальный тестовый каркас (сторонние библиотеки не подключаем).
namespace {

int failedChecks = 0;

string toHex(const vector<uint8_t>& data) {
    const char* hex = "0123456789ABCDEF";
    string s = "{";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i != 0) {
            s += ' ';
        }
        s += hex[data[i] >> 4];
        s += hex[data[i] & 0x0F];
    }
    return s + "}";
}

void checkEqual(const vector<uint8_t>& actual, const vector<uint8_t>& expected,
                const string& what) {
    if (actual != expected) {
        ++failedChecks;
        cout << "    [FAIL] " << what << ": ожидалось " << toHex(expected)
             << ", получено " << toHex(actual) << endl;
    }
}

// Проверяет, что вызов бросает std::invalid_argument.
template <typename F>
void checkThrows(F call, const string& what) {
    try {
        call();
    } catch (const invalid_argument&) {
        return;
    } catch (...) {
        ++failedChecks;
        cout << "    [FAIL] " << what << ": брошено исключение другого типа" << endl;
        return;
    }
    ++failedChecks;
    cout << "    [FAIL] " << what << ": исключение не брошено" << endl;
}

// Проверяет, что вызов НЕ бросает исключение.
template <typename F>
void checkNoThrow(F call, const string& what) {
    try {
        call();
    } catch (const exception& e) {
        ++failedChecks;
        cout << "    [FAIL] " << what << ": неожиданное исключение: " << e.what() << endl;
    }
}

// P-блок «чётные, затем нечётные биты» для n бит (n чётное), если считать с единицы:
// {2, 4, ..., n, 1, 3, ..., n-1}. Номера сдвинуты так, чтобы отсчёт шёл с first.
vector<size_t> evenThenOddBlock(size_t n, size_t first) {
    vector<size_t> p;
    for (size_t i = 1; i < n; i += 2) p.push_back(i + first);
    for (size_t i = 0; i < n; i += 2) p.push_back(i + first);
    return p;
}

// Общий вход для тестов 1-4 и 6-7: 1011 0001 0100 1100.
const vector<uint8_t> sampleInput = {0xB1, 0x4C};

// ---------------------------------------------------------------------------

// 1. Обычный leftToRight, нумерация с единицы.
void testLeftToRightFromOne() {
    // Бит №1 - старший бит input[0]. Чётные биты: 0101 1010, нечётные: 1100 0010.
    checkEqual(bitsPermutation(sampleInput, evenThenOddBlock(16, 1), bitOrder::leftToRight, 1),
               {0x5A, 0xC2}, "чётные и нечётные биты");
    // Разворот всех 16 бит: 1011 0001 0100 1100 -> 0011 0010 1000 1101
    vector<size_t> reversed;
    for (size_t i = 16; i >= 1; --i) reversed.push_back(i);
    checkEqual(bitsPermutation(sampleInput, reversed, bitOrder::leftToRight, 1), {0x32, 0x8D},
               "разворот 16 бит");
}

// 2. Обычный leftToRight, нумерация с нуля.
void testLeftToRightFromZero() {
    // Те же перестановки, номера на 1 меньше - результат тот же.
    checkEqual(bitsPermutation(sampleInput, evenThenOddBlock(16, 0), bitOrder::leftToRight, 0),
               {0x5A, 0xC2}, "чётные и нечётные биты");
    vector<size_t> reversed;
    for (size_t i = 16; i >= 1; --i) reversed.push_back(i - 1);
    checkEqual(bitsPermutation(sampleInput, reversed, bitOrder::leftToRight, 0), {0x32, 0x8D},
               "разворот 16 бит");
}

// 3. Обычный rightToLeft, нумерация с единицы (в исходном списке пункты 3 и 4 повторяли
//    1 и 2, поэтому здесь и в пункте 4 проверяется rightToLeft).
void testRightToLeftFromOne() {
    // Бит №1 - младший бит input[0]. Чётные биты: 0011 0100, нечётные: 1010 0101.
    checkEqual(bitsPermutation(sampleInput, evenThenOddBlock(16, 1), bitOrder::rightToLeft, 1),
               {0x34, 0xA5}, "чётные и нечётные биты");
}

// 4. Обычный rightToLeft, нумерация с нуля.
void testRightToLeftFromZero() {
    checkEqual(bitsPermutation(sampleInput, evenThenOddBlock(16, 0), bitOrder::rightToLeft, 0),
               {0x34, 0xA5}, "чётные и нечётные биты");
}

// 5. Пустой pBlock.
void testEmptyPBlock() {
    for (bitOrder order : {bitOrder::rightToLeft, bitOrder::leftToRight}) {
        for (unsigned int first : {0u, 1u}) {
            string mode = "firstElement = " + to_string(first);
            checkThrows([&] { bitsPermutation(sampleInput, vector<size_t>{}, order, first); },
                        "вход из 2 байт, " + mode);
            checkThrows([&] { bitsPermutation(vector<uint8_t>{}, vector<size_t>{}, order, first); },
                        "пустой вход, " + mode);
        }
    }
}

// 6. Обращение к несуществующему биту.
void testNonexistentBit() {
    for (bitOrder order : {bitOrder::rightToLeft, bitOrder::leftToRight}) {
        // Вход 16 бит: с нуля допустимы 0..15, с единицы 1..16.
        checkNoThrow([&] { bitsPermutation(sampleInput, {15}, order, 0); }, "с нуля: 15");
        checkThrows([&] { bitsPermutation(sampleInput, {16}, order, 0); }, "с нуля: 16");
        checkNoThrow([&] { bitsPermutation(sampleInput, {16}, order, 1); }, "с единицы: 16");
        checkThrows([&] { bitsPermutation(sampleInput, {17}, order, 1); }, "с единицы: 17");
        // Несуществующий бит в начале, в середине и в конце блока.
        checkThrows([&] { bitsPermutation(sampleInput, {99, 1, 2}, order, 1); }, "в начале");
        checkThrows([&] { bitsPermutation(sampleInput, {1, 99, 2}, order, 1); }, "в середине");
        checkThrows([&] { bitsPermutation(sampleInput, {1, 2, 99}, order, 1); }, "в конце");
        // Во входе нет ни одного бита.
        checkThrows([&] { bitsPermutation(vector<uint8_t>{}, {0}, order, 0); }, "пустой вход");
    }
}

// 7. Блок сжимается: битов в результате меньше, чем во входе.
void testCompression() {
    // 16 бит -> 8: чётные биты (с единицы).
    vector<size_t> even = {2, 4, 6, 8, 10, 12, 14, 16};
    checkEqual(bitsPermutation(sampleInput, even, bitOrder::leftToRight, 1), {0x5A},
               "16 -> 8, leftToRight");
    checkEqual(bitsPermutation(sampleInput, even, bitOrder::rightToLeft, 1), {0x34},
               "16 -> 8, rightToLeft");
    // 16 бит -> 4: биты 1, 5, 9, 13; результат один байт, младшие 4 бита нулевые.
    vector<size_t> four = {1, 5, 9, 13};
    checkEqual(bitsPermutation(sampleInput, four, bitOrder::leftToRight, 1), {0x90},
               "16 -> 4, leftToRight");
    checkEqual(bitsPermutation(sampleInput, four, bitOrder::rightToLeft, 1), {0xC0},
               "16 -> 4, rightToLeft");
    // 16 бит -> 1 бит.
    checkEqual(bitsPermutation(sampleInput, {1}, bitOrder::leftToRight, 1), {0x80}, "16 -> 1");
}

// 8. Блок расширяется: битов в результате больше, чем во входе.
void testExpansion() {
    // 8 бит -> 20: каждый бит дважды (16 бит), затем ещё раз биты 1..4.
    vector<uint8_t> in = {0xB1};
    vector<size_t> p = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 1, 2, 3, 4};
    // leftToRight: биты 1,0,1,1,0,0,0,1 -> 1100 1111 | 0000 0011 | 1011 0000
    checkEqual(bitsPermutation(in, p, bitOrder::leftToRight, 1), {0xCF, 0x03, 0xB0},
               "8 -> 20, leftToRight");
    // rightToLeft: биты 1,0,0,0,1,1,0,1 -> 1100 0000 | 1111 0011 | 1000 0000
    checkEqual(bitsPermutation(in, p, bitOrder::rightToLeft, 1), {0xC0, 0xF3, 0x80},
               "8 -> 20, rightToLeft");
    // Один и тот же бит много раз: 1 бит -> 16 бит.
    checkEqual(bitsPermutation({0x80}, vector<size_t>(16, 0), bitOrder::leftToRight, 0),
               {0xFF, 0xFF}, "один бит 16 раз");
}

// 9. В pBlock переданы недопустимые значения.
void testInvalidValues() {
    const size_t negative = static_cast<size_t>(-1);  // «отрицательный» номер
    for (bitOrder order : {bitOrder::rightToLeft, bitOrder::leftToRight}) {
        // Ноль при нумерации с единицы: такого бита нет.
        checkThrows([&] { bitsPermutation(sampleInput, {0}, order, 1); }, "0 при firstElement = 1");
        checkThrows([&] { bitsPermutation(sampleInput, {0, 1, 2}, order, 1); }, "0 в начале");
        checkThrows([&] { bitsPermutation(sampleInput, {1, 0, 2}, order, 1); }, "0 в середине");
        checkThrows([&] { bitsPermutation(sampleInput, {1, 2, 0}, order, 1); }, "0 в конце");
        // Отрицательное число, приведённое к size_t, и предельные значения.
        checkThrows([&] { bitsPermutation(sampleInput, {negative}, order, 0); }, "-1, firstElement = 0");
        checkThrows([&] { bitsPermutation(sampleInput, {negative}, order, 1); }, "-1, firstElement = 1");
        checkThrows([&] { bitsPermutation(sampleInput, {SIZE_MAX / 2}, order, 0); }, "SIZE_MAX / 2");
        checkThrows([&] { bitsPermutation(sampleInput, {SIZE_MAX}, order, 1); }, "SIZE_MAX, firstElement = 1");
        // Неверный firstElement.
        checkThrows([&] { bitsPermutation(sampleInput, {1}, order, 2); }, "firstElement = 2");
        checkThrows([&] { bitsPermutation(sampleInput, {1}, order, static_cast<unsigned int>(-1)); },
                    "firstElement = -1");
    }
}

// ---------------------------------------------------------------------------

struct TestCase {
    const char* name;
    void (*func)();
};

const TestCase tests[] = {
    {"1. leftToRight, нумерация с единицы", testLeftToRightFromOne},
    {"2. leftToRight, нумерация с нуля", testLeftToRightFromZero},
    {"3. rightToLeft, нумерация с единицы", testRightToLeftFromOne},
    {"4. rightToLeft, нумерация с нуля", testRightToLeftFromZero},
    {"5. пустой pBlock", testEmptyPBlock},
    {"6. обращение к несуществующему биту", testNonexistentBit},
    {"7. блок сжимается", testCompression},
    {"8. блок расширяется", testExpansion},
    {"9. недопустимые значения в pBlock", testInvalidValues},
};

}  // namespace

// Без аргументов запускаются все тесты, с аргументом N - только тест номер N
// (так каждый тест регистрируется в CTest отдельно и виден во вкладке «Тестирование»).
int main(int argc, char** argv) {
    const int count = static_cast<int>(sizeof(tests) / sizeof(tests[0]));
    int only = 0;
    if (argc > 1) {
        try {
            only = stoi(argv[1]);
        } catch (const exception&) {
            only = -1;
        }
        if (only < 1 || only > count) {
            cerr << "Использование: " << argv[0] << " [номер теста 1.." << count << "]" << endl;
            return 2;
        }
    }

    int failedTests = 0;
    int run = 0;
    for (int i = 1; i <= count; ++i) {
        if (only != 0 && i != only) {
            continue;
        }
        ++run;
        const TestCase& t = tests[i - 1];
        int before = failedChecks;
        cout << "[ RUN  ] " << t.name << endl;
        try {
            t.func();
        } catch (const exception& e) {
            ++failedChecks;
            cout << "    [FAIL] неожиданное исключение: " << e.what() << endl;
        }
        if (failedChecks == before) {
            cout << "[  OK  ] " << t.name << endl;
        } else {
            ++failedTests;
            cout << "[ FAIL ] " << t.name << endl;
        }
    }

    cout << endl << "Пройдено тестов: " << run - failedTests << " из " << run << endl;
    return failedTests == 0 ? 0 : 1;
}
