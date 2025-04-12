#pragma once
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
        u64 offset;
        u64 size;
        u32 strtabOffset;
    };

    struct PFSEntry {
        FSEntry fsEntry;
        ZERO_PADDING(0x4);
    };

    struct HFSEntry {
        FSEntry fsEntry;
        u32 hashSize;
        ZERO_PADDING(0x8);
        ByteArray<0x20> hash;
    };
#pragma pack(pop)

    class PFS
    {
    public:
        explicit PFS(FileReader* p_File, Engine* p_Engine, bool p_ShouldOwnFile = true);
        PFS(const PFS&) = delete;
        PFS(PFS&& other) noexcept;
        ~PFS();

    private:
        FileReader* m_File;
        bool m_FileOwned = true;

        struct Header {
            enum MagicType: u8 { PFS0, HFS0, INVALID };

            u32 magic;
            u32 numEntries;
            u32 strTabSize;
            ZERO_PADDING(0x4);

            [[nodiscard]] MagicType getMagicType() const;
            [[nodiscard]] const char* getMagicString() const;
        } m_Header{};

        std::vector<NCA> m_NCAs;

        Engine* m_Engine{ nullptr };
    };
}


