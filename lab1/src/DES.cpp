#include "DES.h"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "DesKeyExpansion.h"
#include "DesRoundFunction.h"
#include "bitsPermutation.h"
#include "desTables.h"

namespace {

const size_t DES_BLOCK_SIZE = 8;

void checkBlockSize(const std::vector<uint8_t>& block) {
    if (block.size() != DES_BLOCK_SIZE) {
        throw std::invalid_argument("Блок DES должен быть длиной 8 байт (64 бита)");
    }
}

}  // namespace

// Сеть Фейстеля собирается из реализаций интерфейсов 2.1 и 2.2 для DES.
DES::DES()
    : FeistelNetwork(std::make_shared<DesKeyExpansion>(),
                     std::make_shared<DesRoundFunction>(),
                     DES_BLOCK_SIZE) {}

// ЛК-2, п. 3: IP перед раундами, IP^-1 после.
std::vector<uint8_t> DES::encrypt(const std::vector<uint8_t>& block) const {
    checkBlockSize(block);
    std::vector<uint8_t> permuted = bitsPermutation(block, des::IP, bitOrder::leftToRight, 1);
    std::vector<uint8_t> rounds = FeistelNetwork::encrypt(permuted);
    return bitsPermutation(rounds, des::IP_INV, bitOrder::leftToRight, 1);
}

// Расшифрование: те же IP и IP^-1, раунды с ключами в обратном порядке
// ("IP -> E -> IP^-1 -> IP -> D -> IP^-1", ЛК-2, п. 3).
std::vector<uint8_t> DES::decrypt(const std::vector<uint8_t>& block) const {
    checkBlockSize(block);
    std::vector<uint8_t> permuted = bitsPermutation(block, des::IP, bitOrder::leftToRight, 1);
    std::vector<uint8_t> rounds = FeistelNetwork::decrypt(permuted);
    return bitsPermutation(rounds, des::IP_INV, bitOrder::leftToRight, 1);
}
