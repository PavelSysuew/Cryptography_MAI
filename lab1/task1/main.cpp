#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitsPermutation.h"

using namespace std;

// Печать массива байтов в двоичном и шестнадцатеричном виде.
void printBytes(const string& title, const vector<uint8_t>& data) {
    cout << title;
    for (uint8_t b : data) {
        cout << bitset<8>(b) << ' ';
    }
    cout << "(";
    const char* hex = "0123456789ABCDEF";
    for (uint8_t b : data) {
        cout << hex[b >> 4] << hex[b & 0x0F];
    }
    cout << ")" << endl;
}

int main() {
    vector<uint8_t> b = {0b10110001};
    vector<size_t> reverse = {1, 2, 3, 1, 2, 3, 0, 7};
    try {
        printBytes("reverse:            ", bitsPermutation(b, reverse, bitOrder::rightToLeft, 0));
    } catch (const invalid_argument& e){
        cout << "Ошибка: " << e.what() << endl;
    }

    try {
        printBytes("reverse:            ", bitsPermutation(b, reverse, bitOrder::leftToRight, 0));
    } catch (const invalid_argument& e){
        cout << "Ошибка: " << e.what() << endl;
    }

    return 0;
}