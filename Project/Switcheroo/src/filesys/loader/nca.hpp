#pragma once
#include "../file.hpp"

#include "../../util/swap.hpp"

namespace swroo
{
    class Engine;
}

namespace swroo::filesys
{
    class NCA
    {
        struct Header {
            enum MagicType: u8 { NCA3, NCA2, NCA0, INVALID };

            enum class ContentType : u8 {
                PROGRAM,
                METADATA,
                CONTROL,
                MANUAL,
                DATA,
                PUBLIC_DATA,
            };

            struct TableEntry {
                u32_le media_offset;
                u32_le media_end_offset;
                INSERT_PADDING_BYTES(0x8);
            };

            ByteArray<0x100> RSASign1;
            ByteArray<0x100> RSASign2;
            u32_le magic;
            u8 distType;
            ContentType contentType;
            u8 KeyGenOld;
            u8 keyIndex;
            u64_le size;
            u64_le titleID;
            INSERT_PADDING_BYTES(0x4);
            u32_le sdkVersion;
            u8 KeyGen;
            INSERT_PADDING_BYTES(0xE);
            ByteArray<0x10> rightsID;
            std::array<TableEntry, 0x4> entries;
            std::array<ByteArray<0x20>, 0x4> hashTables;
            ByteArray<0x40> keyArea;

            MagicType getMagicType() const;
        };

    public:
        explicit NCA(MainFileReader& p_MainFile, u32 p_Offset, u32 p_Size, Engine* p_Engine);

        bool decryptHeader();

    private:
        SubFileReader m_SubFile;

        Header m_Header;

        Engine* m_Engine{ nullptr };
    };
}


