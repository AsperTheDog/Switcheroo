#include "aes.hpp"

#include <mbedtls/cipher.h>


bool swroo::crypto::AES::decryptXTS(const u8* p_In, u8* p_Out, const usize p_Size, const u8* p_Key, const TweakCallback& p_TweakProvider, const usize p_SectorSize)
{
    if (p_Size % p_SectorSize != 0) 
        return false; // must be a whole number of sectors

    mbedtls_cipher_context_t ctx;
    mbedtls_cipher_init(&ctx);

    const mbedtls_cipher_info_t* cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_XTS);
    if (!cipher_info || mbedtls_cipher_setup(&ctx, cipher_info) != 0) 
    {
        mbedtls_cipher_free(&ctx);
        return false;
    }

    if (mbedtls_cipher_setkey(&ctx, p_Key, 256, MBEDTLS_DECRYPT) != 0) 
    {
        mbedtls_cipher_free(&ctx);
        return false;
    }

    const usize numSectors = p_Size / p_SectorSize;

    for (usize sector = 0; sector < numSectors; ++sector) 
    {
        ByteArray<16> tweak = p_TweakProvider(sector);

        if (mbedtls_cipher_set_iv(&ctx, tweak.data(), tweak.size()) != 0 || mbedtls_cipher_reset(&ctx) != 0) 
        {
            mbedtls_cipher_free(&ctx);
            return false;
        }

        usize out_len = 0;
        if (mbedtls_cipher_update(&ctx, p_In + sector * p_SectorSize, p_SectorSize, p_Out + sector * p_SectorSize, &out_len) != 0 || out_len != p_SectorSize) 
        {
            mbedtls_cipher_free(&ctx);
            return false;
        }
    }

    mbedtls_cipher_free(&ctx);
    return true;
}
