#pragma once
#include "../util/common.hpp"

#include <unordered_map>
#include <filesystem>

namespace swroo::filesys
{
    struct KeyData
    {
        enum KeySize : u8 {K128, K256, KVAR};

        enum K128Type : u8
        {
            MASTER,
            PACKAGE_1,
            PACKAGE_2,
            TITLE_KEK,
            ETICKET_RSA_KEK,
            KEY_AREA,
            SD_SEED,
            TITLE_KEY,
            SOURCE,
            KEY_BLOB,
            KEY_BLOB_MAC,
            TSEC,
            SECURE_BOOT,
            BIS,
            HEADER_KEK,
            SD_KEK,
            RSA_KEK,
        };

        enum K256Type : u8
        {
            SD_KEY,
            HEADER,
            SD_KEY_SOURCE,
            HEADER_SOURCE,
        };
        
        u8 keySize;
        u8 keyType;
        u64 first;
        u64 second;

        bool operator==(const KeyData& other) const
        {
            return keySize == other.keySize && keyType == other.keyType && first == other.first && second == other.second;
        }
    };
}

//KeyData hash implementation
template <>
struct std::hash<swroo::filesys::KeyData>
{
    size_t operator()(const swroo::filesys::KeyData& key) const noexcept
    {
        return std::hash<u8>()(key.keySize)
             ^ std::hash<u8>()(key.keyType)
             ^ std::hash<u64>()(key.first)
             ^ std::hash<u64>()(key.second);
    }
};

namespace swroo::filesys
{
    class KeyManager
    {
    public:
        explicit KeyManager(const std::filesystem::path& p_ProdKeys, const std::filesystem::path& p_TitleKeys);

        [[nodiscard]] const ByteArray<32>& getKey(KeyData::KeySize p_Size, u8 p_KeyType, u64 p_First = 0, u64 p_Second = 0) const;
        [[nodiscard]] bool hasKey(KeyData::KeySize p_Size, u8 p_KeyType, u64 p_First = 0, u64 p_Second = 0) const;
        [[nodiscard]] const ByteArray<144>& getKeyblob(u32 p_KeyblobID) const;
        [[nodiscard]] const ByteArray<176>& getEncryptedKeyblob(u32 p_KeyblobID) const;
        [[nodiscard]] const ByteArray<576>& getExtendedETicket() const;

    private:
        static std::unordered_map<std::string, KeyData> m_KeyNames;

        std::unordered_map<KeyData, ByteArray<32>> m_Keys{};
        std::unordered_map<u32, ByteArray<144>> m_Keyblobs{};
        std::unordered_map<u32, ByteArray<176>> m_EncryptedKeyblobs{};

        ByteArray<576> m_ExtendedETicket;
    };
}
