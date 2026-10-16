/*
* Интерфейс,  предоставляющий  описание  функционала  по  выполнению  шифрующего 
* преобразования
*/

#pragma once

#include <cstdint>
#include <vector>


class IRoundFunction {
    public:
        virtual ~IRoundFunction() = default;

        /* Раундовая функция шифрования
         * @param input - половина блока шифрования в сети Фейстеля
         * @param roundKey - ключ раунда
         * @return вектор, содержащий результат преобразования. 
         * Выходной блок равен так же половине целого блока шифрования в сети Фейстеля
         */
        virtual std::vector<uint8_t> roundFunctionTransform(const std::vector<uint8_t>& input,
            const std::vector<uint8_t>& roundKey) const = 0;
        
};