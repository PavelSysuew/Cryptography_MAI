#include "bitsPermutation.h"

#include <stdexcept>
#include <string>
#include <algorithm>

using namespace std;

namespace {

/**
 * Создаёт маску для бита: байт, в котором единица стоит только на позиции нужного бита.
 * @param k     Номер бита во всём массиве, нумерация с нуля.
 * @param order Порядок нумерации битов.
 * @return      Маска для бита внутри его байта
 */
uint8_t bitMask(size_t k, bitOrder order) {
    size_t posFromRight;  // позиция бита внутри байта, считая справа

    if (order == bitOrder::rightToLeft) {
        posFromRight = k % 8;
    } else {
        posFromRight = 7 - k % 8;
    }

    return static_cast<uint8_t>(1 << posFromRight);
}

/**
 * Возвращает значение бита.
 * @param data  Массив байтов.
 * @param k     Номер бита во всём массиве, нумерация с нуля.
 * @param order Порядок нумерации битов.
 * @return      true, если бит равен 1, иначе false.
 */
bool getBit(const vector<uint8_t>& data, size_t k, bitOrder order) {
    uint8_t byte = data[k / 8];
    uint8_t mask = bitMask(k, order);
    return (byte & mask) != 0;
}

/**
 * Устанавливает значение бита.
 * @param data  Массив байтов (изменяется).
 * @param k     Номер бита во всём массиве, нумерация с нуля.
 * @param order Порядок нумерации битов.
 * @param value Новое значение бита.
 */
void setBit(vector<uint8_t>& data, size_t k, bitOrder order, bool value) {
    uint8_t mask = bitMask(k, order);

    if (value) {
        data[k / 8] |= mask; // включить бит
    }
}

}

vector<uint8_t> bitsPermutation(const vector<uint8_t>& input, const vector<size_t>& pBlock,
                                bitOrder order, unsigned int firstElement) {
    if (firstElement > 1) {
        throw invalid_argument("firstElement must be 0 or 1");
    }

    const size_t totalBits = input.size() * 8;
    vector<uint8_t> result((pBlock.size() + 7) / 8, 0);

    if (pBlock.size() == 0) {
        throw invalid_argument("pBlock не должен быть пустым");
    }

    

    for (size_t i = 0; i < pBlock.size(); ++i) {
        if (pBlock[i] < firstElement || pBlock[i] - firstElement >= totalBits) {
            throw invalid_argument("pBlock[" + to_string(i) + "] = " + to_string(pBlock[i]) +
                                   " is out of range");
        }

        size_t source = pBlock[i] - firstElement; 
        bool bit = getBit(input, source, order);
        setBit(result, i, order, bit);              
    }


    if (order == bitOrder::rightToLeft) {
        std::reverse(result.begin(), result.end());
        return result;
    }
    return result;
}