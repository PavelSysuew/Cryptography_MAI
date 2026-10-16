#pragma once

/**
 * Набивка (padding) - дополнение данных до длины, кратной размеру блока (ЛК-3, п. 9).
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include "PaddingMode.h"

/**
 * Дополняет данные до длины, кратной blockSize.
 *
 * AnsiX923, PKCS7, ISO10126: добавляется от 1 до blockSize байт. Если длина уже
 * кратна блоку, добавляется целый блок набивки - иначе при снятии нельзя
 * отличить набивку от данных.
 * Zeros: добавляется от 0 до blockSize - 1 нулей.
 *
 * @throws std::invalid_argument если blockSize == 0 или blockSize > 255
 *         (длина набивки должна помещаться в один байт).
 */
std::vector<uint8_t> addPadding(const std::vector<uint8_t>& data, size_t blockSize, PaddingMode mode);

/**
 * Убирает набивку, добавленную addPadding с тем же режимом.
 *
 * Zeros: отрезаются нули в конце последнего блока. Режим неоднозначный:
 * если данные сами кончались нулями, они тоже будут отрезаны.
 *
 * @throws std::invalid_argument если длина данных не кратна blockSize, данные пустые
 *         (кроме Zeros) или набивка повреждена (например, неверный ключ при расшифровании).
 */
std::vector<uint8_t> removePadding(const std::vector<uint8_t>& data, size_t blockSize, PaddingMode mode);
