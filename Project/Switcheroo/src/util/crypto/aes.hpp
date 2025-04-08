#pragma once
#include <functional>

#include "../common.hpp"

namespace swroo::crypto
{
    class AES
    {
    public:
        using TweakCallback = std::function<ByteArray<16>(u64 sector)>;

        static bool decryptXTS(const u8* p_In, u8* p_Out, usize p_Size, const u8* p_Key, const TweakCallback& p_TweakProvider, usize p_SectorSize);
    };
}
