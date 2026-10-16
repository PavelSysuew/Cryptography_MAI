#include "CipherContext.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <future>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "bytesUtils.h"
#include "padding.h"

namespace {

using Block = std::vector<uint8_t>;
using Blocks = std::vector<Block>;

// ============================================================================
//                         Разбиение на блоки и склейка
// ============================================================================

Blocks splitBlocks(const std::vector<uint8_t>& data, size_t blockSize) {
    if (data.size() % blockSize != 0) {
        throw std::invalid_argument("Длина данных должна быть кратна размеру блока");
    }
    Blocks blocks;
    for (size_t offset = 0; offset < data.size(); offset += blockSize) {
        blocks.emplace_back(data.begin() + offset, data.begin() + offset + blockSize);
    }
    return blocks;
}

std::vector<uint8_t> joinBlocks(const Blocks& blocks) {
    std::vector<uint8_t> data;
    for (const Block& block : blocks) {
        data.insert(data.end(), block.begin(), block.end());
    }
    return data;
}

// ============================================================================
//                       Параллельная обработка блоков
// ============================================================================

// Меньше этого числа блоков на поток не даём: запуск потока дороже, чем DES на паре блоков.
const size_t MIN_BLOCKS_PER_THREAD = 256;

/**
 * Вызывает processBlock(i) для всех i от 0 до count - 1, разбивая номера на
 * непрерывные куски по числу ядер; каждый кусок - в своём потоке (std::async).
 * processBlock(i) должна зависеть только от блока i и уже известных данных
 * и писать только в свой out[i] - тогда потоки не мешают друг другу.
 * Исключение из любого потока пробрасывается дальше через future.get().
 */
template <typename ProcessBlock>
void forEachBlockParallel(size_t count, ProcessBlock processBlock) {
    size_t cores = std::max<size_t>(1, std::thread::hardware_concurrency());
    size_t threads = std::min(cores, std::max<size_t>(1, count / MIN_BLOCKS_PER_THREAD));
    size_t chunk = (count + threads - 1) / threads;       // округление вверх

    std::vector<std::future<void>> tasks;
    for (size_t begin = 0; begin < count; begin += chunk) {
        size_t end = std::min(count, begin + chunk);
        tasks.push_back(std::async(std::launch::async, [begin, end, &processBlock] {
            for (size_t i = begin; i < end; ++i) {
                processBlock(i);
            }
        }));
    }
    for (std::future<void>& task : tasks) {
        task.get();                                        // ждём всех и пробрасываем исключения
    }
}

// ============================================================================
//                     Счётчик для CTR и Random Delta
// ============================================================================

// a = a + b по модулю 2^(8 * размер). Числа big-endian: старший байт - первый.
void addBigEndian(Block& a, const Block& b) {
    unsigned carry = 0;
    for (size_t i = a.size(); i-- > 0;) {
        unsigned sum = a[i] + b[i] + carry;
        a[i] = static_cast<uint8_t>(sum);
        carry = sum >> 8;
    }
}

// Значение счётчика для блока номер index (с 0): start + index * delta (по модулю 2^n).
// Умножение на index заменено сложениями «удвоением»: так любой блок можно
// посчитать сразу, не перебирая предыдущие (пригодится для параллельности).
Block counterAt(const Block& start, const Block& delta, size_t index) {
    Block result = start;
    Block step = delta;                 // delta * 2^k
    while (index > 0) {
        if (index & 1) {
            addBigEndian(result, step);
        }
        addBigEndian(step, step);       // step *= 2
        index >>= 1;
    }
    return result;
}

// Шаг счётчика: CTR - 1 (ЛК-3, п. 8);
// Random Delta - правая половина IV с младшим битом 1 (ЛК-3, п. 7).
Block counterDelta(CipherMode mode, const Block& iv) {
    Block delta(iv.size(), 0);
    if (mode == CipherMode::CTR) {
        delta.back() = 1;
    } else {
        size_t half = iv.size() / 2;
        for (size_t i = half; i < iv.size(); ++i) {
            delta[i] = iv[i];
        }
        delta.back() |= 1;              // нечётное delta взаимно просто с 2^n
    }
    return delta;
}

// ============================================================================
//                                   Режимы
// ============================================================================

// ECB (ЛК-3, п. 2): c_i = E(m_i), m_i = D(c_i).
// Блоки независимы - параллельно в обе стороны.
Blocks ecb(const ISymmetricCipher& cipher, const Blocks& in, bool encrypting) {
    Blocks out(in.size());
    forEachBlockParallel(in.size(), [&](size_t i) {
        out[i] = encrypting ? cipher.encrypt(in[i]) : cipher.decrypt(in[i]);
    });
    return out;
}

// CBC (ЛК-3, п. 3): c_i = E(m_i xor c_{i-1}), c_0 = IV.
Blocks cbcEncrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out;
    Block previous = iv;
    for (const Block& m : in) {
        previous = cipher.encrypt(xorBytes(m, previous));
        out.push_back(previous);
    }
    return out;
}

// CBC: m_i = D(c_i) xor c_{i-1}.
// Все c_i уже известны - дешифрование параллельно (ЛК-3, п. 3).
Blocks cbcDecrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out(in.size());
    forEachBlockParallel(in.size(), [&](size_t i) {
        const Block& previous = (i == 0) ? iv : in[i - 1];
        out[i] = xorBytes(cipher.decrypt(in[i]), previous);
    });
    return out;
}

// PCBC (ЛК-3, п. 4): c_i = E(m_i xor m_{i-1} xor c_{i-1}), для первого блока - E(m_1 xor IV).
// feedback хранит m_{i-1} xor c_{i-1} (для первого блока - IV).
Blocks pcbcEncrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out;
    Block feedback = iv;
    for (const Block& m : in) {
        Block c = cipher.encrypt(xorBytes(m, feedback));
        feedback = xorBytes(m, c);
        out.push_back(c);
    }
    return out;
}

// PCBC: m_i = D(c_i) xor m_{i-1} xor c_{i-1}.
Blocks pcbcDecrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out;
    Block feedback = iv;
    for (const Block& c : in) {
        Block m = xorBytes(cipher.decrypt(c), feedback);
        feedback = xorBytes(m, c);
        out.push_back(m);
    }
    return out;
}

// CFB (ЛК-3, п. 5): c_i = m_i xor E(c_{i-1}), c_0 = IV.
Blocks cfbEncrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out;
    Block previous = iv;
    for (const Block& m : in) {
        previous = xorBytes(m, cipher.encrypt(previous));
        out.push_back(previous);
    }
    return out;
}

// CFB: m_i = c_i xor E(c_{i-1}). При дешифровании тоже вызывается E.
// Все c_i уже известны - дешифрование параллельно.
Blocks cfbDecrypt(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out(in.size());
    forEachBlockParallel(in.size(), [&](size_t i) {
        const Block& previous = (i == 0) ? iv : in[i - 1];
        out[i] = xorBytes(in[i], cipher.encrypt(previous));
    });
    return out;
}

// OFB (ЛК-3, п. 6): s_0 = IV, s_i = E(s_{i-1}), c_i = m_i xor s_i.
// Шифрование и дешифрование совпадают.
Blocks ofb(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv) {
    Blocks out;
    Block state = iv;
    for (const Block& block : in) {
        state = cipher.encrypt(state);
        out.push_back(xorBytes(block, state));
    }
    return out;
}

// CTR (ЛК-3, п. 8) и Random Delta (ЛК-3, п. 7): c_j = m_j xor E(IV + (j - 1) * delta).
// Шифрование и дешифрование совпадают. Счётчик любого блока считается сразу
// (counterAt) - параллельно в обе стороны.
Blocks counterMode(const ISymmetricCipher& cipher, const Blocks& in, const Block& iv, CipherMode mode) {
    Block delta = counterDelta(mode, iv);
    Blocks out(in.size());
    forEachBlockParallel(in.size(), [&](size_t j) {
        out[j] = xorBytes(in[j], cipher.encrypt(counterAt(iv, delta, j)));
    });
    return out;
}

// ============================================================================
//                                   Файлы
// ============================================================================

std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл для чтения: " + path);
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void writeFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + path);
    }
    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!file) {
        throw std::runtime_error("Ошибка записи в файл: " + path);
    }
}

}  // namespace

// ============================================================================
//                                  Контекст
// ============================================================================

CipherContext::CipherContext(std::shared_ptr<ISymmetricCipher> cipher,
                             const std::vector<uint8_t>& key,
                             CipherMode mode,
                             PaddingMode padding,
                             std::optional<std::vector<uint8_t>> iv,
                             std::vector<std::any> extraParams)
    : cipher_(std::move(cipher)),
      mode_(mode),
      padding_(padding),
      extraParams_(std::move(extraParams)) {
    if (!cipher_) {
        throw std::invalid_argument("Шифр не задан (nullptr)");
    }

    if (mode_ != CipherMode::ECB) {
        if (!iv) {
            throw std::invalid_argument("Для этого режима нужен вектор инициализации (IV)");
        }
        if (iv->size() != cipher_->getBlockSize()) {
            throw std::invalid_argument("Длина IV должна быть равна размеру блока шифра");
        }
        iv_ = std::move(*iv);
    }

    cipher_->takeKey(key);
}

std::vector<uint8_t> CipherContext::encryptData(const std::vector<uint8_t>& data) const {
    const size_t blockSize = cipher_->getBlockSize();
    Blocks in = splitBlocks(addPadding(data, blockSize, padding_), blockSize);
    Blocks out;

    switch (mode_) {
        case CipherMode::ECB:         out = ecb(*cipher_, in, true); break;
        case CipherMode::CBC:         out = cbcEncrypt(*cipher_, in, iv_); break;
        case CipherMode::PCBC:        out = pcbcEncrypt(*cipher_, in, iv_); break;
        case CipherMode::CFB:         out = cfbEncrypt(*cipher_, in, iv_); break;
        case CipherMode::OFB:         out = ofb(*cipher_, in, iv_); break;
        case CipherMode::CTR:
        case CipherMode::RandomDelta: out = counterMode(*cipher_, in, iv_, mode_); break;
    }

    return joinBlocks(out);
}

std::vector<uint8_t> CipherContext::decryptData(const std::vector<uint8_t>& data) const {
    const size_t blockSize = cipher_->getBlockSize();
    Blocks in = splitBlocks(data, blockSize);
    Blocks out;

    switch (mode_) {
        case CipherMode::ECB:         out = ecb(*cipher_, in, false); break;
        case CipherMode::CBC:         out = cbcDecrypt(*cipher_, in, iv_); break;
        case CipherMode::PCBC:        out = pcbcDecrypt(*cipher_, in, iv_); break;
        case CipherMode::CFB:         out = cfbDecrypt(*cipher_, in, iv_); break;
        case CipherMode::OFB:         out = ofb(*cipher_, in, iv_); break;
        case CipherMode::CTR:
        case CipherMode::RandomDelta: out = counterMode(*cipher_, in, iv_, mode_); break;
    }

    return removePadding(joinBlocks(out), blockSize, padding_);
}

std::future<void> CipherContext::encrypt(const std::vector<uint8_t>& input,
                                         std::vector<uint8_t>& output) const {
    return std::async(std::launch::async, [this, &input, &output] {
        output = encryptData(input);
    });
}

std::future<void> CipherContext::decrypt(const std::vector<uint8_t>& input,
                                         std::vector<uint8_t>& output) const {
    return std::async(std::launch::async, [this, &input, &output] {
        output = decryptData(input);
    });
}

// Пути копируются в лямбду (по значению): строки вызывающего могут исчезнуть раньше,
// чем закончится фоновая работа.
std::future<void> CipherContext::encrypt(const std::string& inputPath,
                                         const std::string& outputPath) const {
    return std::async(std::launch::async, [this, inputPath, outputPath] {
        writeFile(outputPath, encryptData(readFile(inputPath)));
    });
}

std::future<void> CipherContext::decrypt(const std::string& inputPath,
                                         const std::string& outputPath) const {
    return std::async(std::launch::async, [this, inputPath, outputPath] {
        writeFile(outputPath, decryptData(readFile(inputPath)));
    });
}
