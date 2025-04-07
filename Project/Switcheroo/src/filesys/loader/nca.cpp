#include "nca.hpp"

#include "../../engine.hpp"

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
    const Header::MagicType l_MagicType = m_Header.getMagicType();
    if (l_MagicType == Header::MagicType::NCA3)
        return true;
    if (l_MagicType != Header::MagicType::INVALID)
        return false;
    
    Header l_DecryptedHeader;
    ByteArray<32> l_HeaderKey = m_Engine->getKeyManager().getKey(KeyData::K128, KeyData::K256Type::HEADER);

}
