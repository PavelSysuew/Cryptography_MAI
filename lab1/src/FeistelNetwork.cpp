#include "FeistelNetwork.h"
#include "bytesUtils.h"
#include <stdexcept>
#include <utility>


FeistelNetwork::FeistelNetwork(std::shared_ptr<IKeyExpansion> keyExpansion,
    std::shared_ptr<IRoundFunction> roundFunction,
    size_t blockSize) : 
    keyExpansion_(std::move(keyExpansion)), 
    roundFunction_(std::move(roundFunction)), blockSize_(blockSize) 
{
    if (blockSize == 0) {
        throw std::invalid_argument("Размер блока должен быть больше 0");
    }
    if (blockSize % 2 != 0) {
        throw std::invalid_argument("Размер блока должен быть четным");
    }
    if (!keyExpansion_) {
        throw std::invalid_argument("Указатель на расширение ключа не должен быть пустым");
    }
    if (!roundFunction_) {
        throw std::invalid_argument("Указатель на раундовую функцию не должен быть пустым");
    }

}


void FeistelNetwork::takeKey(const std::vector<uint8_t>& key) {
    roundKeys_ = keyExpansion_->expandKey(key);
    if (roundKeys_.empty()) {
        throw std::invalid_argument("Расширение ключа вернуло пустой вектор");
    }
}

size_t FeistelNetwork::getBlockSize() const {
    return blockSize_;
}

std::vector<uint8_t> FeistelNetwork::encrypt(const std::vector<uint8_t>& block) const {
    return runRounds(block, KeyOrder::forward);
}
std::vector<uint8_t> FeistelNetwork::decrypt(const std::vector<uint8_t>& block) const {
    return runRounds(block, KeyOrder::reverse);
}

std::vector<uint8_t> FeistelNetwork::runRounds(const std::vector<uint8_t>& block,
    KeyOrder order) const {
    if (roundKeys_.empty()) {
        throw std::logic_error("Ключ не задан: вызовите takeKey");
    }

    if (block.size() != blockSize_) {
        throw std::invalid_argument("Размер блока не соответствует размеру, заданному при создании объекта");
    }

    size_t halfSize = blockSize_ / 2;
    std::vector<uint8_t> left(block.begin(), block.begin() + halfSize);
    std::vector<uint8_t> right(block.begin() + halfSize, block.end());

    // Число раундов определяется количеством раундовых ключей
    size_t rounds = roundKeys_.size();
    std::vector<uint8_t> newRight;

    for (size_t i = 0; i < rounds; i++) {
        // Определяем индекс раундового ключа в зависимости от порядка (прямой или обратный)
        size_t roundIndex = (order == KeyOrder::forward) ? i : (rounds - 1 - i);

        const std::vector<uint8_t>& roundKey = roundKeys_[roundIndex];

        std::vector<uint8_t> f = roundFunction_->roundFunctionTransform(right, roundKey);
        if (f.size() != halfSize) {
            throw std::logic_error("Размер выходного блока раундовой функции не соответствует половине размера блока");
        }
        newRight = xorBytes(left, f);

        left = std::move(right);
        right = std::move(newRight);
    }

    // Склеим половины обратно в один блок
    std::vector<uint8_t> result = right;
    result.insert(result.end(), left.begin(), left.end());
    
    return result;
}