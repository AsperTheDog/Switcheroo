#include "nca.hpp"

#include <iostream>

#include "../../engine.hpp"
#include "../../util/crypto/aes.hpp"

#include <mbedtls/cipher.h>

static ByteArray<16> getNintendoTweak(u64 p_SectorNumber) {
    ByteArray<16> l_Tweak{};
    for (usize i = 15; i <= 15; --i) {
        l_Tweak[i] = static_cast<u8>(p_SectorNumber & 0xFF);
        p_SectorNumber >>= 8;
    }
    return l_Tweak;
}

swroo::filesys::NCA::Header::MagicType swroo::filesys::NCA::Header::getMagicType() const
{
    if (magic == MagicFromChars('N', 'C', 'A', '3'))
        return MagicType::NCA3;
    if (magic == MagicFromChars('N', 'C', 'A', '2'))
        return MagicType::NCA2;
    if (magic == MagicFromChars('N', 'C', 'A', '0'))
        return MagicType::NCA0;
    return MagicType::INVALID;
}

swroo::filesys::NCA::NCA(MainFileReader& p_MainFile, const u32 p_Offset, const u32 p_Size, Engine* p_Engine)
    : m_SubFile(p_MainFile, p_Offset, p_Size), m_Engine(p_Engine)
{
    m_SubFile.read(m_Header);
    if (!decryptHeader())
    {
        throw std::runtime_error("Unsupported or invalid NCA");
    }
}

bool swroo::filesys::NCA::decryptHeader()
{
    Header::MagicType l_MagicType = m_Header.getMagicType();
    if (l_MagicType == Header::MagicType::NCA3)
        return true;
    if (l_MagicType != Header::MagicType::INVALID)
        return false;
    
    Header l_DecryptedHeader;
    const ByteArray<32> l_HeaderKey = m_Engine->getKeyManager().getKey(KeyData::K256, KeyData::K256Type::HEADER);
    if (!crypto::AES::decryptXTS(reinterpret_cast<const u8*>(&m_Header), reinterpret_cast<u8*>(&l_DecryptedHeader), 
        sizeof(Header), l_HeaderKey.data(), getNintendoTweak, 0x200))
    {
        return false;
    }

    m_Header = l_DecryptedHeader;
    l_MagicType = m_Header.getMagicType();
    if (l_MagicType == Header::MagicType::INVALID)
        return false;
    return true;
}
