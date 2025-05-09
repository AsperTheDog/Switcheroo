#include "pfs.hpp"

#include "../file.hpp"
#include <iostream>

swroo::filesys::PFS::Header::MagicType swroo::filesys::PFS::Header::getMagicType() const
{
    if (magic == utils::MagicFromChars('P', 'F', 'S', '0'))
        return MagicType::PFS0;
    if (magic == utils::MagicFromChars('H', 'F', 'S', '0'))
        return MagicType::HFS0;
    return MagicType::INVALID;
}

const char* swroo::filesys::PFS::Header::getMagicString() const
{
    return reinterpret_cast<const char*>(&magic);
}

swroo::filesys::PFS::PFS(FileReader* p_File, Engine* p_Engine, const bool p_ShouldOwnFile)
    : m_File(p_File), m_FileOwned(p_ShouldOwnFile), m_Engine(p_Engine)
{
    std::cout << "\nLoading PFS0 from: " << p_File << '\n';

    m_File->read(m_Header);

    const Header::MagicType l_MagicType = m_Header.getMagicType();
    if (l_MagicType == Header::MagicType::INVALID)
    {
        throw std::runtime_error("Invalid magic type: " + std::string(m_Header.getMagicString()));
    }

    std::cout << "\tMagic: " << m_Header.getMagicString() << '\n';
    std::cout << "\tNumber of entries: " << m_Header.numEntries << '\n';
    std::cout << "\tString table size: " << m_Header.strTabSize << '\n';

    constexpr usize l_EntriesOffset = sizeof(Header);

    const usize l_EntrySize = l_MagicType == Header::MagicType::PFS0 ? sizeof(PFSEntry) : sizeof(HFSEntry);
    const usize l_MetadataSize = sizeof(Header) + (m_Header.numEntries * l_EntrySize) + m_Header.strTabSize;
    const usize l_StrTabOffset = l_EntriesOffset + (m_Header.numEntries * l_EntrySize);
    const usize l_ContentOffset = l_StrTabOffset + m_Header.strTabSize;

    std::vector<u8> l_Metadata;
    l_Metadata.resize(l_MetadataSize);
    m_File->readData(l_Metadata.data(), l_MetadataSize, 0);
    
    if (l_Metadata.size() != l_MetadataSize)
    {
        throw std::runtime_error("Failed to read metadata from file: " + p_File->getFilePath().string());
    }

    std::vector<FSEntry> l_Entries(m_Header.numEntries);
    std::vector<std::string> l_Strings(m_Header.numEntries);

    for (usize i = 0; i < m_Header.numEntries; ++i)
    {
        const usize l_EntryOffset = l_EntriesOffset + (i * l_EntrySize);
        m_File->read(l_Entries[i], l_EntryOffset);
        const usize l_StrOffset = l_StrTabOffset + l_Entries[i].strtabOffset;
        const usize l_StrSize = std::strlen(reinterpret_cast<const char*>(&l_Metadata[l_StrOffset]));
        if (l_StrSize > 0)
        {
            l_Strings[i].resize(l_StrSize);
            std::memcpy(l_Strings[i].data(), l_Metadata.data() + l_StrOffset, l_StrSize);
        }

        std::cout << "\tEntry " << i << ": " << l_Strings[i] << ", Offset: " << l_Entries[i].offset << ", Size: " << l_Entries[i].size << '\n';

        FileReader* l_SubFile = new SubFileReader(*m_File, l_ContentOffset + l_Entries[i].offset, l_Entries[i].size);
        m_NCAs.emplace_back(l_SubFile, m_Engine);
    }
}

swroo::filesys::PFS::PFS(PFS&& other) noexcept
    : m_File(other.m_File), m_FileOwned(other.m_FileOwned), m_Header(other.m_Header), m_NCAs(std::move(other.m_NCAs)), m_Engine(other.m_Engine)
{
    other.m_File = nullptr;
    other.m_FileOwned = false;
}

swroo::filesys::PFS::~PFS()
{
    if (m_FileOwned)
        delete m_File;
}
