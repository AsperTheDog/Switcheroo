#pragma once
#include "../file.hpp"

namespace swroo
{
    namespace crypto
    {
        class AES;
    }

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

            struct FSEntry {
                u32 beginOffset;
                u32 endOffset;
                ZERO_PADDING(0x8);

                [[nodiscard]] bool isValid() const { return beginOffset > 0; }
            };

            ByteArray<0x100> RSASign1;
            ByteArray<0x100> RSASign2;
            u32 magic;
            u8 distType;
            ContentType contentType;
            u8 KeyGenOld;
            u8 keyIndex;
            u64 size;
            u64 titleID;
            ZERO_PADDING(0x4);
            u32 sdkVersion;
            u8 KeyGen;
            ZERO_PADDING(0xE);
            ByteArray<0x10> rightsID;
            std::array<FSEntry, 0x4> entries;
            std::array<ByteArray<0x20>, 0x4> hashTables;
            ByteArray<0x40> keyArea;
            ZERO_PADDING(0xC0);


            [[nodiscard]] MagicType getMagicType() const;

            [[nodiscard]] u8 getEntryCount() const;
        };

        struct FSEntry
        {
            struct Header
            {
                PADDING(0x2);
                u8 partitionType;
                u8 fsFype;
                u8 cryptType;
                PADDING(0x3);
            };

            struct IVFCHeader
            {
                struct IVFCLevel {
                    u64 offset;
                    u64 size;
                    u32 blockSize;
                    u32 reserved;
                };

                u32 magic;
                u32 magicNumber;
                PADDING(0x8);
                std::array<IVFCLevel, 0x6> levels;
                PADDING(0x40);
            };

            struct PFS0SuperBlock
            {
                ByteArray<0x20> masterHash;
                u32 size;
                PADDING(0x4);
                u64 hashOffset;
                u64 hashSize;
                u64 pfsOffset;
                u64 pfsSize;
                PADDING(0x1B0);
            };

            struct RomFSuperBlock
            {
                IVFCHeader ivfcHeader;
                PADDING(0x118);
            };

            struct BKRTSuperBlock
            {
                struct Header {
                    u64 offset;
                    u64 size;
                    u32 magic;
                    PADDING(0x4);
                    u32 number_entries;
                    PADDING(0x4);
                };
                IVFCHeader ivfcHeader;
                PADDING(0x18);
                Header relocationHeader;
                Header subsectionHeader;
                PADDING(0xC0);
            };

            Header header;
            union
            {
                PFS0SuperBlock pfs0;
                RomFSuperBlock romfs;
                BKRTSuperBlock bkrts;
            };
        };

    public:
        explicit NCA(MainFileReader& p_MainFile, usize p_Offset, usize p_Size, Engine* p_Engine);

        utils::DecryptResult decryptHeader(const ByteArray<3072>& p_RawData, crypto::AES& p_AES);
        utils::DecryptResult decryptEntries(const ByteArray<3072>& p_RawData, crypto::AES& p_AES, bool p_IsHeaderEnctrypted);

    private:
        SubFileReader m_SubFile;

        Header m_Header;
        Header::MagicType m_MagicType{ Header::MagicType::INVALID };

        std::array<FSEntry, 4> m_Entries{};

        Engine* m_Engine{ nullptr };
    };
}


