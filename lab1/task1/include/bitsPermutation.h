#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Порядок нумерации битов ВХОДА (номера в pBlock).
 * rightToLeft: бит №0 (или №1) - младший (самый правый) бит input[0],
 *              дальше нумерация идёт справа налево, потом переходит на input[1] и т.д.
 * leftToRight: бит №0 (или №1) - старший (самый левый) бит input[0],
 *              дальше нумерация идёт слева направо, потом переходит на input[1] и т.д.
 * Байты массива всегда идут слева направо: input[0], input[1], ...
 * Результат от порядка не зависит: он всегда заполняется в порядке pBlock слева направо.
 */
enum class bitOrder {
    rightToLeft,
    leftToRight
};

/**
 * Переставляет биты массива input в соответствии с блоком перестановки pBlock.
 * @param input        Входной массив байтов.
 * @param pBlock       Блок перестановки: pBlock[i] - номер бита input, который
 *                     окажется на позиции i результата. Позиция 0 - старший бит
 *                     result[0], позиция 8 - старший бит result[1] и т.д. Размер
 *                     pBlock задаёт число бит результата, поэтому блок может быть
 *                     короче или длиннее входа, а номера в нём могут повторяться.
 * @param order        Порядок нумерации битов входа (по умолчанию rightToLeft).
 * @param firstElement Номер первого бита: 0 или 1 (по умолчанию 0). Относится
 *                     к числам в pBlock.
 * @return             Массив байтов длиной ceil(pBlock.size() / 8). Неиспользуемые
 *                     биты последнего байта равны нулю.
 * @throws std::invalid_argument если firstElement не 0 и не 1 или номер в pBlock
 *                               выходит за границы входа.
 */
std::vector<uint8_t> bitsPermutation(const std::vector<uint8_t>& input,
                                     const std::vector<size_t>& pBlock,
                                     bitOrder order = bitOrder::rightToLeft,
                                     unsigned int firstElement = 0);