#pragma once
#include "../../util/swap.hpp"
#include "../../util/common.hpp"

#include <filesystem>

#include "nca.hpp"
#include "../file.hpp"

namespace swroo
{
    class Engine;
}

namespace swroo::filesys
{
#pragma pack(push, 1)
    struct FSEntry {
        u64_le offset;
        u64_le size;
        u32_le strtabOffset;
    };

    struct PFSEntry {
        FSEntry fsEntry;
        ZERO_PADDING(0x4);
    };

    struct HFSEntry {
        FSEntry fsEntry;
        u32_le hashSize;
        ZERO_PADDING(0x8);
        ByteArray<0x20> hash;
    };
#pragma pack(pop)

    class PFS
    {
    public:
        explicit PFS(const std::filesystem::path& p_File, Engine* p_Engine);

    private:
        MainFileReader m_File;

        struct Header {
            enum MagicType: u8 { PFS0, HFS0, INVALID };

            u32_le magic;
            u32_le numEntries;
            u32_le strTabSize;
            ZERO_PADDING(0x4);

            [[nodiscard]] MagicType getMagicType() const;
            [[nodiscard]] const char* getMagicString() const;
        } m_Header;

        std::vector<NCA> m_NCAs;

        Engine* m_Engine{ nullptr };
    };
}


