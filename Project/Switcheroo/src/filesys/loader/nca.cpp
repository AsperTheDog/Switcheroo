#include "nca.hpp"

#include "../../engine.hpp"
#include "../../util/crypto/aes.hpp"

static ByteArray<0x10> getNintendoTweak(u64 p_SectorNumber) {
    ByteArray<0x10> l_Tweak{};
    for (i32 i = 0xF; i >= 0; i--) 
    {
        l_Tweak[i] = static_cast<u8>(p_SectorNumber & 0xFF);
        p_SectorNumber >>= 8;
    }
    return l_Tweak;
}

swroo::filesys::NCA::Header::MagicType swroo::filesys::NCA::Header::getMagicType() const
{
    if (magic == utils::MagicFromChars('N', 'C', 'A', '3'))
        return MagicType::NCA3;
    if (magic == utils::MagicFromChars('N', 'C', 'A', '2'))
        return MagicType::NCA2;
    if (magic == utils::MagicFromChars('N', 'C', 'A', '0'))
        return MagicType::NCA0;
    return MagicType::INVALID;
}

u8 swroo::filesys::NCA::Header::getEntryCount() const
{
    u8 l_Count = 0;
    for (const auto& l_Entry : entries)
    {
        if (l_Entry.isValid())
            ++l_Count;
    }
    return l_Count;
}

swroo::filesys::NCA::NCA(MainFileReader& p_MainFile, const usize p_Offset, const usize p_Size, Engine* p_Engine)
    : m_SubFile(p_MainFile, p_Offset, p_Size), m_Engine(p_Engine)
{
    ByteArray<0xC00> l_InitialData;
    m_SubFile.readBytes(l_InitialData.data(), l_InitialData.size(), 0);

    const ByteArray<0x20> l_HeaderKey = m_Engine->getKeyManager().getKey(KeyData::K256, KeyData::K256Type::HEADER);
    crypto::AES l_AES(l_HeaderKey.data());

    const utils::DecryptResult l_HeaderResult = decryptHeader(l_InitialData, l_AES);
    if (l_HeaderResult == utils::DecryptResult::FAILURE)
        throw std::runtime_error("Invalid NCA Header");

    if (m_MagicType == Header::MagicType::NCA0)
        throw std::runtime_error("NCA0 is not implemented yet"); // TODO?

    const utils::DecryptResult l_EntriesResult = decryptEntries(l_InitialData, l_AES, l_HeaderResult != utils::DecryptResult::NOT_ENCRYPTED);
    if (l_EntriesResult == utils::DecryptResult::FAILURE)
        throw std::runtime_error("Failed to decrypt NCA entries");
}

swroo::utils::DecryptResult swroo::filesys::NCA::decryptHeader(const ByteArray<0xC00>& p_RawData, crypto::AES& p_AES)
{
    m_MagicType = reinterpret_cast<const Header*>(&p_RawData)->getMagicType();
    if (m_MagicType != Header::MagicType::INVALID)
        return utils::DecryptResult::NOT_ENCRYPTED;
    
    Header l_DecryptedHeader;
    if (!p_AES.decryptXTS(reinterpret_cast<const u8*>(&p_RawData), reinterpret_cast<u8*>(&l_DecryptedHeader), sizeof(Header), getNintendoTweak, 0x200))
        return utils::DecryptResult::FAILURE;

    m_Header = l_DecryptedHeader;
    m_MagicType = m_Header.getMagicType();
    if (m_MagicType == Header::MagicType::INVALID)
        return utils::DecryptResult::FAILURE;
    return utils::DecryptResult::SUCCESS;
}

swroo::utils::DecryptResult swroo::filesys::NCA::decryptEntries(const ByteArray<0xC00>& p_RawData, crypto::AES& p_AES, const bool p_IsHeaderEnctrypted)
{
    if (!p_IsHeaderEnctrypted)
        return utils::DecryptResult::NOT_ENCRYPTED;

    const u8* l_EntryPtr = p_RawData.data() + sizeof(Header);
    if (m_MagicType == Header::MagicType::NCA3)
    {
        std::array<FSEntry, 4> l_Entries{};
        if (!p_AES.decryptXTS(l_EntryPtr, reinterpret_cast<u8*>(l_Entries.data()), 
            sizeof(FSEntry) * m_Entries.size(), getNintendoTweak, 0x200, 2))
        {
            return utils::DecryptResult::FAILURE;
        }
        m_Entries = l_Entries;
    }
    else if (m_MagicType == Header::MagicType::NCA2)
    {
        for (FSEntry& l_Entry : m_Entries)
        {
            FSEntry l_DecEntry{};
            if (!p_AES.decryptXTS(l_EntryPtr, reinterpret_cast<u8*>(&l_DecEntry), 
                sizeof(FSEntry), getNintendoTweak, 0x200, 0))
            {
                return utils::DecryptResult::FAILURE;
            }
            l_Entry = l_DecEntry;
            l_EntryPtr += sizeof(FSEntry);
        }
    }

    return utils::DecryptResult::SUCCESS;
}
