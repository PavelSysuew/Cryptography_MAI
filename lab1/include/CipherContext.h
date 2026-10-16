#pragma once

/**
 * Контекст симметричного шифрования (задание 2.4).
 *
 * Шифрует и дешифрует данные произвольной длины блочным шифром (реализацией
 * интерфейса 2.3) в заданном режиме (ЛК-3, п. 1-8) с заданной набивкой (ЛК-3, п. 9).
 *
 *   шифрование:    данные -> набивка -> блоки -> режим -> шифртекст
 *   дешифрование:  шифртекст -> блоки -> режим -> снятие набивки -> данные
 *
 * Все операции асинхронные (возвращают std::future). Блоки обрабатываются
 * параллельно там, где режим это позволяет: ECB (оба направления),
 * CBC и CFB (дешифрование), CTR и Random Delta (оба направления).
 */

#include <any>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CipherMode.h"
#include "ISymmetricCipher.h"
#include "PaddingMode.h"

class CipherContext {
    public:
        /**
         * @param cipher       Блочный шифр (например, DES). Контекст вызывает у него takeKey(key).
         * @param key          Ключ шифрования.
         * @param mode         Режим шифрования.
         * @param padding      Режим набивки.
         * @param iv           Вектор инициализации длиной в блок. Обязателен для всех режимов,
         *                     кроме ECB (в ECB не используется).
         *                     CTR, Random Delta: IV (как big-endian число) - начальный счётчик.
         *                     Random Delta: шаг delta - правая половина IV с младшим битом 1.
         * @param extraParams  Дополнительные параметры режима (коллекция переменной длины).
         *                     Текущим режимам не нужны, сохраняются для расширения.
         * @throws std::invalid_argument если шифр пустой, IV не задан или неверной длины.
         */
        CipherContext(std::shared_ptr<ISymmetricCipher> cipher,
                      const std::vector<uint8_t>& key,
                      CipherMode mode,
                      PaddingMode padding,
                      std::optional<std::vector<uint8_t>> iv = std::nullopt,
                      std::vector<std::any> extraParams = {});

        /**
         * Асинхронно шифрует input и записывает результат в output.
         * Возвращает future: вызовите .get(), чтобы дождаться окончания (и получить
         * исключение, если оно было). До этого input, output и сам контекст должны жить.
         */
        std::future<void> encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) const;

        /** Асинхронно дешифрует input в output. Условия - как у encrypt. */
        std::future<void> decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) const;

        /**
         * Асинхронно шифрует файл inputPath и записывает результат в outputPath.
         * Файлы читаются и пишутся в двоичном режиме (подходит для любых файлов).
         * Ошибки (файл не открылся и т.п.) - std::runtime_error из .get().
         * Контекст должен жить до .get().
         */
        std::future<void> encrypt(const std::string& inputPath, const std::string& outputPath) const;

        /** Асинхронно дешифрует файл inputPath в outputPath. Условия - как у encrypt для файлов. */
        std::future<void> decrypt(const std::string& inputPath, const std::string& outputPath) const;

    private:
        std::shared_ptr<ISymmetricCipher> cipher_;
        CipherMode mode_;
        PaddingMode padding_;
        std::vector<uint8_t> iv_;               // пустой для ECB
        std::vector<std::any> extraParams_;

        // Синхронные версии: вся работа здесь, encrypt/decrypt только запускают их асинхронно.
        std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data) const;
        std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data) const;
};
