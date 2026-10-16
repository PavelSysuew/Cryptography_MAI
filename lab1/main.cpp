/*
 * Задание 5: демонстрация шифрования и дешифрования алгоритмом DES в разных режимах.
 *
 * Запуск:
 *   ./build/lab1                          - псевдослучайные данные + файлы по умолчанию
 *   ./build/lab1 song.mp3 video.mp4 ...   - те же демо на своих файлах
 *
 * Результаты (зашифрованные и расшифрованные файлы, картинки) - в папке demo_output
 * в корне проекта. Большие файлы (видео) быстрее шифруются в Release-сборке:
 *   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
 */

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "DES.h"
#include "demo.h"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

using namespace std;

int main(int argc, char** argv) {
    const string root = PROJECT_ROOT;
    const string outputDir = root + "/demo_output";
    filesystem::create_directories(outputDir);

    DemoCipher des{
        "DES",
        [] { return make_shared<DES>(); },
        {0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1},
    };

    const string imagePath = createDemoImage(outputDir);

    // Файлы: из командной строки или по умолчанию (текст, исходный код, PDF, картинка).
    vector<string> files;
    for (int i = 1; i < argc; ++i) {
        files.push_back(argv[i]);
    }
    if (files.empty()) {
        files = {
            root + "/docs/lectures/ЛК-3.md",
            root + "/lab1/tests/task2/test_CipherContext.cpp",
            root + "/docs/Лабораторная № 1.pdf",
            imagePath,
        };
    }

    try {
        demoRandomData(des);
        demoFiles(des, files, outputDir);
        demoImageModes(des, imagePath, outputDir);
    } catch (const exception& e) {
        cout << "Ошибка: " << e.what() << endl;
        return 1;
    }
    return 0;
}
