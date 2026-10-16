#pragma once

/**
 * Алгоритм DES на базе сети Фейстеля
 */

#include <cstdint>
#include <vector>

#include "FeistelNetwork.h"

class DES : public FeistelNetwork {
    public:
        // Блок 8 байт, ключ (передаётся в takeKey) - 8 байт.
        DES();

        // IP -> шифрование сетью Фейстеля -> IP^-1
        std::vector<uint8_t> encrypt(const std::vector<uint8_t>& block) const override;

        // IP -> расшифрование сетью Фейстеля -> IP^-1
        std::vector<uint8_t> decrypt(const std::vector<uint8_t>& block) const override;
};
