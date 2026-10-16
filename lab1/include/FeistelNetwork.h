/**
 * Файл содержит описание класса FeistelNetwork, реализующего интерфейс ISymmetricCipher
 */

#pragma once

#include "ISymmetricCipher.h"
#include "IRoundFunction.h"
#include "IKeyExpansion.h"
#include <cstdint>
#include <vector>
#include <cstddef>
#include <memory>

enum class KeyOrder {
    forward,  // ENCRYPTION - ключи для шифрования берутся в прямом порядке
    reverse // DECRYPTION - ключи для расшифрования берутся в обратном порядке
};

class FeistelNetwork : public ISymmetricCipher {
    public:
        FeistelNetwork(std::shared_ptr<IKeyExpansion> keyExpansion, 
            std::shared_ptr<IRoundFunction> roundFunction, 
            size_t blockSize);

        std::vector<uint8_t> encrypt(const std::vector<uint8_t>& block) const override;
        std::vector<uint8_t> decrypt(const std::vector<uint8_t>& block) const override;
        void takeKey(const std::vector<uint8_t>& key) override;
        size_t getBlockSize() const override;


    private:
        // Указатель на расширение ключа
        std::shared_ptr<IKeyExpansion> keyExpansion_;

        // Указатель на раундовую функцию
        std::shared_ptr<IRoundFunction> roundFunction_;

        // Размер блока
        size_t blockSize_;

        // Раундовые ключи
        std::vector<std::vector<uint8_t>> roundKeys_;

        // Метод прогона раундов 
        std::vector<uint8_t> runRounds(const std::vector<uint8_t>& block,
            KeyOrder order) const;
        
};