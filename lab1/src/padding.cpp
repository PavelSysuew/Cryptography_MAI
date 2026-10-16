#include "padding.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

void checkBlockSize(size_t blockSize) {
    if (blockSize == 0 || blockSize > 255) {
        throw std::invalid_argument("Размер блока для набивки должен быть от 1 до 255 байт");
    }
}

// Случайный байт для ISO 10126.
uint8_t randomByte() {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 255);
    return static_cast<uint8_t>(distribution(generator));
}

}  // namespace

std::vector<uint8_t> addPadding(const std::vector<uint8_t>& data, size_t blockSize, PaddingMode mode) {
    checkBlockSize(blockSize);

    // Сколько байт добавить: от 1 до blockSize (если данные кратны блоку - целый блок).
    size_t padLength = blockSize - data.size() % blockSize;

    std::vector<uint8_t> result = data;

    switch (mode) {
        case PaddingMode::Zeros:
            // Для нулей целый лишний блок не нужен: его всё равно нельзя отличить от данных.
            if (padLength == blockSize) {
                padLength = 0;
            }
            result.insert(result.end(), padLength, 0x00);
            break;

        case PaddingMode::AnsiX923:
            // 00 00 ... 00 N
            result.insert(result.end(), padLength - 1, 0x00);
            result.push_back(static_cast<uint8_t>(padLength));
            break;

        case PaddingMode::PKCS7:
            // N N ... N
            result.insert(result.end(), padLength, static_cast<uint8_t>(padLength));
            break;

        case PaddingMode::ISO10126:
            // ?? ?? ... ?? N
            for (size_t i = 0; i + 1 < padLength; ++i) {
                result.push_back(randomByte());
            }
            result.push_back(static_cast<uint8_t>(padLength));
            break;
    }

    return result;
}

std::vector<uint8_t> removePadding(const std::vector<uint8_t>& data, size_t blockSize, PaddingMode mode) {
    checkBlockSize(blockSize);
    if (data.size() % blockSize != 0) {
        throw std::invalid_argument("Длина данных с набивкой должна быть кратна размеру блока");
    }

    if (mode == PaddingMode::Zeros) {
        // Отрезаем нули в конце, но не дальше начала последнего блока.
        size_t end = data.size();
        size_t lastBlockStart = data.empty() ? 0 : data.size() - blockSize;
        while (end > lastBlockStart && data[end - 1] == 0x00) {
            --end;
        }
        return std::vector<uint8_t>(data.begin(), data.begin() + end);
    }

    if (data.empty()) {
        throw std::invalid_argument("Нет данных: набивка должна занимать хотя бы 1 байт");
    }

    // Для AnsiX923, PKCS7, ISO10126 длина набивки записана в последнем байте.
    size_t padLength = data.back();
    if (padLength == 0 || padLength > blockSize) {
        throw std::invalid_argument("Повреждённая набивка: неверная длина в последнем байте");
    }

    size_t dataEnd = data.size() - padLength;

    // Проверяем остальные байты набивки (кроме последнего).
    for (size_t i = dataEnd; i + 1 < data.size(); ++i) {
        if (mode == PaddingMode::AnsiX923 && data[i] != 0x00) {
            throw std::invalid_argument("Повреждённая набивка ANSI X.923: ожидались нули");
        }
        if (mode == PaddingMode::PKCS7 && data[i] != padLength) {
            throw std::invalid_argument("Повреждённая набивка PKCS7: все байты должны быть равны длине");
        }
        // ISO10126: байты случайные, проверять нечего.
    }

    return std::vector<uint8_t>(data.begin(), data.begin() + dataEnd);
}
