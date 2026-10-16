#pragma once

/**
 * Режимы шифрования
 */
enum class CipherMode {
    ECB,        
    CBC,         
    PCBC,     
    CFB,          
    OFB,       
    CTR,         
    RandomDelta,  
};
