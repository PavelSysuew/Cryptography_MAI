/*
 * Интерфейс для расширения ключа
*/

#pragma once

#include <cstdint>
#include <vector>

class IKeyExpansion {
    public:
        virtual ~IKeyExpansion() = default;

        /* Функция расширения ключа.
         * @param key - исходный ключ
         * @return вектор, содержащий расширенные ключи для каждого раунда
         */
        virtual std::vector<std::vector<uint8_t>> expandKey(
            const std::vector<uint8_t>& key) const = 0;
};