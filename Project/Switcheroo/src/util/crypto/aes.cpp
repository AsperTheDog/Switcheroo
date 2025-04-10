#include "aes.hpp"

#include <stdexcept>
#include <mbedtls/cipher.h>


swroo::crypto::AES::AES(const u8* p_Key)
{
    mbedtls_cipher_init(&m_Ctx);

    const mbedtls_cipher_info_t* l_CipherInfo = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_XTS);
    if (!l_CipherInfo || mbedtls_cipher_setup(&m_Ctx, l_CipherInfo) != 0)
        throw std::runtime_error("Failed to setup AES cipher context");

    if (mbedtls_cipher_setkey(&m_Ctx, p_Key, 256, MBEDTLS_DECRYPT) != 0)
        throw std::runtime_error("Failed to set AES key");
}

swroo::crypto::AES::~AES()
{
    mbedtls_cipher_free(&m_Ctx);
}

bool swroo::crypto::AES::decryptXTS(const u8* p_In, u8* p_Out, const usize p_Size, const TweakCallback& p_TweakProvider, const usize p_SectorSize, const usize p_SectorOffset)
{
    if (p_Size % p_SectorSize != 0) 
        return false; // must be a whole number of sectors

    const usize l_NumSectors = p_Size / p_SectorSize;
    for (usize l_Sector = 0; l_Sector < l_NumSectors; ++l_Sector) 
    {
        ByteArray<16> l_Tweak = p_TweakProvider(l_Sector + p_SectorOffset);

        if (mbedtls_cipher_set_iv(&m_Ctx, l_Tweak.data(), l_Tweak.size()) != 0 || mbedtls_cipher_reset(&m_Ctx) != 0) 
            return false;

        usize out_len = 0;
        if (mbedtls_cipher_update(&m_Ctx, p_In + l_Sector * p_SectorSize, p_SectorSize, p_Out + l_Sector * p_SectorSize, &out_len) != 0 || out_len != p_SectorSize) 
            return false;
    }

    return true;
}
