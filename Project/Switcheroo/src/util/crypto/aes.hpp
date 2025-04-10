#pragma once
#include <functional>
#include <mbedtls/cipher.h>

#include "../common.hpp"

namespace swroo::crypto
{
    class AES
    {
    public:
        explicit AES(const u8* p_Key);
        ~AES();

        using TweakCallback = std::function<ByteArray<16>(u64 sector)>;

        bool decryptXTS(const u8* p_In, u8* p_Out, usize p_Size, const TweakCallback& p_TweakProvider, usize p_SectorSize, usize p_SectorOffset = 0);

    private:
        mbedtls_cipher_context_t m_Ctx;
    };
}
