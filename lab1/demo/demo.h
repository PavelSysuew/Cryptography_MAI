#pragma once

/**
 * Демонстрация шифрования (задание 5; для задания 7 - тот же код с DEAL).
 * Все функции работают с любым шифром через интерфейс 2.3 и контекст 2.4.
 */

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ISymmetricCipher.h"

// Какой шифр демонстрировать.
struct DemoCipher {
    std::string name;                                        // для вывода: "DES"
    std::function<std::shared_ptr<ISymmetricCipher>()> make; // создаёт новый объект шифра
    std::vector<uint8_t> key;
};

// 1. Псевдослучайные последовательности байтов: все режимы, разные длины, все набивки.
void demoRandomData(const DemoCipher& cipher);

// 2. Файлы: каждый файл шифруется и дешифруется во всех режимах, результат сравнивается
//    с исходным. В outputDir остаются зашифрованный и расшифрованный файлы режима CBC.
void demoFiles(const DemoCipher& cipher, const std::vector<std::string>& paths,
               const std::string& outputDir);

// Создаёт картинку BMP с крупными одноцветными фигурами, возвращает путь к ней.
std::string createDemoImage(const std::string& outputDir);

// 3. Картинка: шифруются только пиксели (заголовок BMP сохраняется), поэтому
//    зашифрованный файл можно открыть и увидеть разницу между ECB и CBC/CTR.
void demoImageModes(const DemoCipher& cipher, const std::string& imagePath,
                    const std::string& outputDir);
