#pragma once

/**
 * Расширение ключа DES (реализация интерфейса 2.1).
 * Теория: ЛК-2, п. 4.
 */

#include <cstdint>
#include <vector>

#include "IKeyExpansion.h"

class DesKeyExpansion : public IKeyExpansion {
    public:
        /**
         * Строит 16 раундовых ключей DES из 64-битного ключа.
         * @param key  Ключ: ровно 8 байт (64 бита, из них 56 значащих, 8 - биты чётности).
         * @return     16 раундовых ключей по 6 байт (48 бит), в порядке раундов 1..16.
         * @throws std::invalid_argument если ключ не 8 байт.
         */
        std::vector<std::vector<uint8_t>> expandKey(
            const std::vector<uint8_t>& key) const override;
};
