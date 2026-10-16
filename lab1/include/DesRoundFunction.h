#pragma once

/**
 * Раундовая функция F алгоритма DES (реализация интерфейса 2.2).
 * Теория: ЛК-2, п. 5.
 */

#include <cstdint>
#include <vector>

#include "IRoundFunction.h"

class DesRoundFunction : public IRoundFunction {
    public:
        /**
         * F(R, K) = P( S( E(R) xor K ) )
         * @param input     Половина блока: ровно 4 байта (32 бита).
         * @param roundKey  Раундовый ключ: ровно 6 байт (48 бит).
         * @return          4 байта (32 бита).
         * @throws std::invalid_argument если размеры входа или ключа неверные.
         */
        std::vector<uint8_t> roundFunctionTransform(const std::vector<uint8_t>& input,
            const std::vector<uint8_t>& roundKey) const override;
};
