#include "demo.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "CipherContext.h"
#include "CipherMode.h"
#include "PaddingMode.h"

namespace fs = std::filesystem;
using namespace std;

namespace {

const vector<CipherMode> ALL_MODES = {
    CipherMode::ECB, CipherMode::CBC, CipherMode::PCBC, CipherMode::CFB,
    CipherMode::OFB, CipherMode::CTR, CipherMode::RandomDelta,
};

const vector<PaddingMode> ALL_PADDINGS = {
    PaddingMode::Zeros, PaddingMode::AnsiX923, PaddingMode::PKCS7, PaddingMode::ISO10126,
};

// Режим, файлы которого остаются в папке результата (остальные удаляются, чтобы не занимать место).
const CipherMode KEPT_MODE = CipherMode::CBC;

string modeName(CipherMode mode) {
    switch (mode) {
        case CipherMode::ECB:         return "ECB";
        case CipherMode::CBC:         return "CBC";
        case CipherMode::PCBC:        return "PCBC";
        case CipherMode::CFB:         return "CFB";
        case CipherMode::OFB:         return "OFB";
        case CipherMode::CTR:         return "CTR";
        case CipherMode::RandomDelta: return "RandomDelta";
    }
    return "?";
}

string paddingName(PaddingMode padding) {
    switch (padding) {
        case PaddingMode::Zeros:    return "Zeros";
        case PaddingMode::AnsiX923: return "ANSI X.923";
        case PaddingMode::PKCS7:    return "PKCS7";
        case PaddingMode::ISO10126: return "ISO 10126";
    }
    return "?";
}

// Первые maxBytes байт в hex: "0A 1B 2C ..."
string toHex(const vector<uint8_t>& data, size_t maxBytes = 16) {
    ostringstream out;
    for (size_t i = 0; i < data.size() && i < maxBytes; ++i) {
        out << hex << uppercase << setw(2) << setfill('0') << static_cast<int>(data[i]) << ' ';
    }
    if (data.size() > maxBytes) {
        out << "...";
    }
    return out.str();
}

// Псевдослучайные байты. Генератор с фиксированным зерном - запуски повторяемы.
vector<uint8_t> randomBytes(size_t size, mt19937& generator) {
    uniform_int_distribution<int> distribution(0, 255);
    vector<uint8_t> data(size);
    for (uint8_t& b : data) {
        b = static_cast<uint8_t>(distribution(generator));
    }
    return data;
}

// Случайный вектор инициализации (ЛК-3, п. 3: IV не секретен, но должен быть непредсказуемым).
vector<uint8_t> randomIv(size_t blockSize) {
    random_device device;
    vector<uint8_t> iv(blockSize);
    for (uint8_t& b : iv) {
        b = static_cast<uint8_t>(device());
    }
    return iv;
}

// Время выполнения action() в миллисекундах.
template <typename Action>
double measureMs(Action action) {
    auto start = chrono::steady_clock::now();
    action();
    return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

vector<uint8_t> readFile(const string& path) {
    ifstream file(path, ios::binary);
    return vector<uint8_t>(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
}

void writeFile(const string& path, const vector<uint8_t>& data) {
    ofstream file(path, ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
}

string sizeText(uintmax_t bytes) {
    ostringstream out;
    if (bytes >= 1024 * 1024) {
        out << fixed << setprecision(1) << bytes / (1024.0 * 1024.0) << " МБ";
    } else if (bytes >= 1024) {
        out << fixed << setprecision(1) << bytes / 1024.0 << " КБ";
    } else {
        out << bytes << " Б";
    }
    return out.str();
}

void printTitle(const string& title) {
    cout << endl << "=== " << title << " ===" << endl << endl;
}

}  // namespace

// ============================================================================
//                    1. Псевдослучайные последовательности
// ============================================================================

void demoRandomData(const DemoCipher& cipher) {
    printTitle(cipher.name + ": псевдослучайные последовательности байтов");

    const size_t blockSize = cipher.make()->getBlockSize();
    const vector<uint8_t> iv = randomIv(blockSize);
    mt19937 generator(2026);

    cout << "Ключ: " << toHex(cipher.key, 32) << endl;
    cout << "IV:   " << toHex(iv, 32) << endl << endl;

    // --- Все режимы на данных разной длины (набивка PKCS7) ---
    const vector<size_t> sizes = {0, 1, 7, 8, 9, 1000, 64 * 1024};
    cout << "Все режимы, набивка PKCS7, длины данных: 0, 1, 7, 8, 9, 1000, 65536 байт" << endl;
    // Заголовки выровнены пробелами вручную: setw считает байты, а русская буква в UTF-8 - 2 байта.
    cout << "Режим        шифр. 64 КБ, мс   дешифр. 64 КБ, мс   все длины совпали" << endl;

    for (CipherMode mode : ALL_MODES) {
        CipherContext context(cipher.make(), cipher.key, mode, PaddingMode::PKCS7, iv);
        bool allMatch = true;
        double encryptMs = 0;
        double decryptMs = 0;

        for (size_t size : sizes) {
            vector<uint8_t> data = randomBytes(size, generator);
            vector<uint8_t> encrypted;
            vector<uint8_t> decrypted;
            double e = measureMs([&] { context.encrypt(data, encrypted).get(); });
            double d = measureMs([&] { context.decrypt(encrypted, decrypted).get(); });
            if (size == sizes.back()) {
                encryptMs = e;
                decryptMs = d;
            }
            allMatch = allMatch && (decrypted == data);
        }

        cout << left << setw(13) << modeName(mode) << setw(18) << fixed << setprecision(1) << encryptMs
             << setw(20) << decryptMs << (allMatch ? "да" : "НЕТ") << endl;
    }

    // --- Все набивки (режим CBC, 13 байт - до двух блоков не хватает) ---
    cout << endl << "Все набивки, режим CBC, 13 байт:" << endl;
    vector<uint8_t> data13 = randomBytes(13, generator);
    for (PaddingMode padding : ALL_PADDINGS) {
        CipherContext context(cipher.make(), cipher.key, CipherMode::CBC, padding, iv);
        vector<uint8_t> encrypted;
        vector<uint8_t> decrypted;
        context.encrypt(data13, encrypted).get();
        context.decrypt(encrypted, decrypted).get();
        cout << "  " << left << setw(12) << paddingName(padding) << "13 Б -> шифртекст " << encrypted.size()
             << " Б -> " << decrypted.size() << " Б, совпадает: " << (decrypted == data13 ? "да" : "НЕТ") << endl;
    }

    // --- ECB против CBC на двух одинаковых блоках (ЛК-3, п. 2-3) ---
    cout << endl << "Два одинаковых блока открытого текста:" << endl;
    vector<uint8_t> block = randomBytes(blockSize, generator);
    vector<uint8_t> twoBlocks = block;
    twoBlocks.insert(twoBlocks.end(), block.begin(), block.end());
    cout << "  открытый текст: " << toHex(twoBlocks, 32) << endl;
    for (CipherMode mode : {CipherMode::ECB, CipherMode::CBC}) {
        CipherContext context(cipher.make(), cipher.key, mode, PaddingMode::Zeros, iv);
        vector<uint8_t> encrypted;
        context.encrypt(twoBlocks, encrypted).get();
        cout << "  " << left << setw(16) << modeName(mode) << toHex(encrypted, 32) << endl;
    }
    cout << "  В ECB блоки шифртекста одинаковые - структура текста видна; в CBC разные." << endl;
}

// ============================================================================
//                                 2. Файлы
// ============================================================================

void demoFiles(const DemoCipher& cipher, const vector<string>& paths, const string& outputDir) {
    printTitle(cipher.name + ": файлы");

    const vector<uint8_t> iv = randomIv(cipher.make()->getBlockSize());
    cout << "Набивка PKCS7. Результаты: " << outputDir << endl;
    cout << "(зашифрованный и расшифрованный файлы остаются только для режима " << modeName(KEPT_MODE) << ")"
         << endl;

    for (const string& path : paths) {
        cout << endl;
        if (!fs::exists(path)) {
            cout << "Нет файла, пропускаю: " << path << endl;
            continue;
        }

        fs::path source(path);
        cout << source.filename().string() << " (" << sizeText(fs::file_size(source)) << ")" << endl;
        cout << "  Режим        шифр., мс   дешифр., мс   совпадает" << endl;

        vector<uint8_t> original = readFile(path);

        for (CipherMode mode : ALL_MODES) {
            CipherContext context(cipher.make(), cipher.key, mode, PaddingMode::PKCS7, iv);

            string base = (fs::path(outputDir) / source.stem()).string() + "." + cipher.name + "." + modeName(mode);
            string encryptedPath = base + ".enc";
            string decryptedPath = base + ".dec" + source.extension().string();

            double encryptMs = measureMs([&] { context.encrypt(path, encryptedPath).get(); });
            double decryptMs = measureMs([&] { context.decrypt(encryptedPath, decryptedPath).get(); });
            bool match = (readFile(decryptedPath) == original);

            cout << "  " << left << setw(13) << modeName(mode) << setw(12) << fixed << setprecision(1)
                 << encryptMs << setw(14) << decryptMs << (match ? "да" : "НЕТ") << endl;

            if (mode != KEPT_MODE) {
                fs::remove(encryptedPath);
                fs::remove(decryptedPath);
            }
        }
    }
}

// ============================================================================
//                      3. Картинка: ECB против CBC и CTR
// ============================================================================

namespace {

const int IMAGE_WIDTH = 240;              // 240 * 3 байта = 720, кратно 4 - строки без выравнивания
const int IMAGE_HEIGHT = 160;
const size_t BMP_HEADER_SIZE = 54;

void putLE(vector<uint8_t>& out, uint32_t value, int bytes) {
    for (int i = 0; i < bytes; ++i) {
        out.push_back(static_cast<uint8_t>(value >> (8 * i)));
    }
}

}  // namespace

string createDemoImage(const string& outputDir) {
    const uint32_t pixelBytes = IMAGE_WIDTH * IMAGE_HEIGHT * 3;

    // Заголовок BMP (24 бита на пиксель, без сжатия).
    vector<uint8_t> bmp = {'B', 'M'};
    putLE(bmp, BMP_HEADER_SIZE + pixelBytes, 4);  // размер файла
    putLE(bmp, 0, 4);                             // зарезервировано
    putLE(bmp, BMP_HEADER_SIZE, 4);               // где начинаются пиксели
    putLE(bmp, 40, 4);                            // размер второй части заголовка
    putLE(bmp, IMAGE_WIDTH, 4);
    putLE(bmp, IMAGE_HEIGHT, 4);
    putLE(bmp, 1, 2);                             // число плоскостей
    putLE(bmp, 24, 2);                            // бит на пиксель
    putLE(bmp, 0, 4);                             // без сжатия
    putLE(bmp, pixelBytes, 4);
    putLE(bmp, 2835, 4);                          // разрешение (72 dpi)
    putLE(bmp, 2835, 4);
    putLE(bmp, 0, 4);
    putLE(bmp, 0, 4);

    // Пиксели (порядок цветов в BMP: синий, зелёный, красный): фон и три фигуры.
    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        for (int x = 0; x < IMAGE_WIDTH; ++x) {
            uint8_t r = 245, g = 245, b = 245;
            int dx = x - 70, dy = y - 50;
            if (x >= 20 && x < 110 && y >= 95 && y < 145) {
                r = 220; g = 30; b = 30;               // красный прямоугольник
            } else if (x >= 135 && x < 220 && y >= 20 && y < 130) {
                r = 30; g = 60; b = 200;               // синий прямоугольник
            } else if (dx * dx + dy * dy < 30 * 30) {
                r = 30; g = 160; b = 60;               // зелёный круг
            }
            bmp.push_back(b);
            bmp.push_back(g);
            bmp.push_back(r);
        }
    }

    string path = (fs::path(outputDir) / "image.bmp").string();
    writeFile(path, bmp);
    return path;
}

void demoImageModes(const DemoCipher& cipher, const string& imagePath, const string& outputDir) {
    printTitle(cipher.name + ": картинка, зашифрованная в разных режимах");

    vector<uint8_t> bmp = readFile(imagePath);
    if (bmp.size() <= BMP_HEADER_SIZE) {
        cout << "Нет картинки: " << imagePath << endl;
        return;
    }

    vector<uint8_t> header(bmp.begin(), bmp.begin() + BMP_HEADER_SIZE);
    vector<uint8_t> pixels(bmp.begin() + BMP_HEADER_SIZE, bmp.end());
    const vector<uint8_t> iv = randomIv(cipher.make()->getBlockSize());

    cout << "Шифруются только пиксели (" << pixels.size() << " Б, кратно блоку - набивка Zeros ничего не "
         << "добавляет), заголовок BMP сохраняется, поэтому результат открывается как картинка." << endl;
    cout << "Исходная: " << imagePath << endl;

    for (CipherMode mode : {CipherMode::ECB, CipherMode::CBC, CipherMode::CTR}) {
        CipherContext context(cipher.make(), cipher.key, mode, PaddingMode::Zeros, iv);
        vector<uint8_t> encrypted;
        context.encrypt(pixels, encrypted).get();

        vector<uint8_t> result = header;
        result.insert(result.end(), encrypted.begin(), encrypted.end());
        string path = (fs::path(outputDir) / ("image." + cipher.name + "." + modeName(mode) + ".bmp")).string();
        writeFile(path, result);
        cout << "  " << left << setw(6) << modeName(mode) << path << endl;
    }

    cout << "В ECB одинаковые блоки пикселей шифруются одинаково - контуры фигур видны (ЛК-3, п. 2)." << endl;
    cout << "В CBC и CTR картинка превращается в шум." << endl;
}
