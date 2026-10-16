/*
* Интерфейс,  предоставляющий  описание  функционала  по  
* выполнению  шифрования  и  дешифрования  симметричным  алгоритмом
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>


class ISymmetricCipher {
    public: 
        virtual ~ISymmetricCipher() = default;

        /* Функция шифрования
         * @param block - блок данных для шифрования
         * @param key - ключ шифрования
         * @return вектор, содержащий зашифрованные данные
         */
        virtual std::vector<uint8_t> encrypt(
            const std::vector<uint8_t>& block) const = 0;

        /* Функция дешифрования
         * @param block - блок данных для дешифрования
         * @param key - ключ дешифрования
         * @return вектор, содержащий дешифрованные данные
         */
        virtual std::vector<uint8_t> decrypt(
            const std::vector<uint8_t>& block) const = 0;
        
        virtual void takeKey(
            const std::vector<uint8_t>& key) = 0;

        virtual size_t getBlockSize() const = 0;
};