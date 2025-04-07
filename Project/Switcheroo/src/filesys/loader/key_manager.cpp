#include "key_manager.hpp"

#include <fstream>
#include <iostream>

std::unordered_map<std::string, swroo::filesys::KeyData> swroo::filesys::KeyManager::m_KeyNames{
    {"eticket_rsa_kek_source",          {KeyData::K128, KeyData::K128Type::SOURCE,          11, 0}},
    {"eticket_rsa_kekek_source",        {KeyData::K128, KeyData::K128Type::SOURCE,          12, 0}},
    {"rsa_oaep_kek_generation_source",  {KeyData::K128, KeyData::K128Type::SOURCE,          3,  0}},
    {"sd_card_kek_source",              {KeyData::K128, KeyData::K128Type::SOURCE,          0,  0}},
    {"aes_kek_generation_source",       {KeyData::K128, KeyData::K128Type::SOURCE,          1,  0}},
    {"aes_key_generation_source",       {KeyData::K128, KeyData::K128Type::SOURCE,          2,  0}},
    {"package2_key_source",             {KeyData::K128, KeyData::K128Type::SOURCE,          8,  0}},
    {"master_key_source",               {KeyData::K128, KeyData::K128Type::SOURCE,          4,  0}},
    {"header_kek_source",               {KeyData::K128, KeyData::K128Type::SOURCE,          9,  0}},
    {"key_area_key_application_source", {KeyData::K128, KeyData::K128Type::SOURCE,          6,  0}},
    {"key_area_key_ocean_source",       {KeyData::K128, KeyData::K128Type::SOURCE,          6,  1}},
    {"key_area_key_system_source",      {KeyData::K128, KeyData::K128Type::SOURCE,          6,  2}},
    {"titlekek_source",                 {KeyData::K128, KeyData::K128Type::SOURCE,          7,  0}},
    {"keyblob_mac_key_source",          {KeyData::K128, KeyData::K128Type::SOURCE,          10, 0}},
    {"rsa_kek_mask_0",                  {KeyData::K128, KeyData::K128Type::RSA_KEK,         0,  0}},
    {"rsa_kek_seed_3",                  {KeyData::K128, KeyData::K128Type::RSA_KEK,         1,  0}},
    {"eticket_rsa_kek",                 {KeyData::K128, KeyData::K128Type::ETICKET_RSA_KEK, 0,  0}},
    {"tsec_key",                        {KeyData::K128, KeyData::K128Type::TSEC,            0,  0}},
    {"secure_boot_key",                 {KeyData::K128, KeyData::K128Type::SECURE_BOOT,     0,  0}},
    {"sd_seed",                         {KeyData::K128, KeyData::K128Type::SD_SEED,         0,  0}},
    {"bis_key_0_crypt",                 {KeyData::K128, KeyData::K128Type::BIS,             0,  0}},
    {"bis_key_0_tweak",                 {KeyData::K128, KeyData::K128Type::BIS,             0,  1}},
    {"bis_key_1_crypt",                 {KeyData::K128, KeyData::K128Type::BIS,             1,  0}},
    {"bis_key_1_tweak",                 {KeyData::K128, KeyData::K128Type::BIS,             1,  1}},
    {"bis_key_2_crypt",                 {KeyData::K128, KeyData::K128Type::BIS,             2,  0}},
    {"bis_key_2_tweak",                 {KeyData::K128, KeyData::K128Type::BIS,             2,  1}},
    {"bis_key_3_crypt",                 {KeyData::K128, KeyData::K128Type::BIS,             3,  0}},
    {"bis_key_3_tweak",                 {KeyData::K128, KeyData::K128Type::BIS,             3,  1}},
    {"header_kek",                      {KeyData::K128, KeyData::K128Type::HEADER_KEK,      0,  0}},
    {"sd_card_kek",                     {KeyData::K128, KeyData::K128Type::SD_KEK,          0,  0}},
    {"key_area_key_application_",       {KeyData::K128, KeyData::K128Type::KEY_AREA,        0,  0}},
    {"key_area_key_ocean_",             {KeyData::K128, KeyData::K128Type::KEY_AREA,        0,  1}},
    {"key_area_key_system_",            {KeyData::K128, KeyData::K128Type::KEY_AREA,        0,  2}},
    {"header_key",                      {KeyData::K256, KeyData::K256Type::HEADER,          0,  0}},
    {"sd_card_save_key_source",         {KeyData::K256, KeyData::K256Type::SD_KEY_SOURCE,   0,  0}},
    {"sd_card_nca_key_source",          {KeyData::K256, KeyData::K256Type::SD_KEY_SOURCE,   1,  0}},
    {"header_key_source",               {KeyData::K256, KeyData::K256Type::HEADER_SOURCE,   0,  0}},
    {"sd_card_save_key",                {KeyData::K256, KeyData::K256Type::SD_KEY,          0,  0}},
    {"sd_card_nca_key",                 {KeyData::K256, KeyData::K256Type::SD_KEY,          1,  0}},
    {"master_key_",                     {KeyData::KVAR, KeyData::K128Type::MASTER,          0,  0}},
    {"package1_key_",                   {KeyData::KVAR, KeyData::K128Type::PACKAGE_1,       0,  0}},
    {"package2_key_",                   {KeyData::KVAR, KeyData::K128Type::PACKAGE_2,       0,  0}},
    {"title_kek_",                      {KeyData::KVAR, KeyData::K128Type::TITLE_KEK,       0,  0}},
    {"keyblob_key_source_",             {KeyData::KVAR, KeyData::K128Type::SOURCE,          5,  0}},
    {"keyblob_key_",                    {KeyData::KVAR, KeyData::K128Type::KEY_BLOB,        0,  0}},
    {"keyblob_mac_key_",                {KeyData::KVAR, KeyData::K128Type::KEY_BLOB_MAC,    0,  0}},

};

template<u32 Size>
ByteArray<Size> HexStringToByteArray(const std::string& p_Key)
{
    if (p_Key.size() % 2 != 0)
        throw std::invalid_argument("Hex string must have an even number of digits");

    ByteArray<Size> l_Bytes{};

    for (usize l_CharIdx = 0; l_CharIdx < p_Key.size(); l_CharIdx += 2) 
    {
        std::string l_ByteString = p_Key.substr(l_CharIdx, 2);
        l_Bytes[l_CharIdx / 2] = static_cast<u8>(std::strtoul(l_ByteString.c_str(), nullptr, 16));
    }

    return l_Bytes;
}

static bool isStringNumber(const std::string_view p_Str, const u32 p_Offset, const u32 p_Size)
{
    for (u32 l_Idx = p_Offset; l_Idx < p_Offset + p_Size; ++l_Idx)
    {
        if (!std::isdigit(p_Str[l_Idx]))
            return false;
    }
    return true;
}

swroo::filesys::KeyManager::KeyManager(const std::filesystem::path& p_ProdKeys, const std::filesystem::path& p_TitleKeys)
{

    // The file is prod.keys. It contains the keys used by the Switch.

    auto l_ExtractKeyValue = [](const std::string& p_Line) -> std::pair<std::string, std::string>
    {
        if (p_Line.empty() || p_Line[0] == '#')
            return { "", "" };

        const usize l_EqualPos = p_Line.find('=');
        if (l_EqualPos == std::string::npos)
            return { "", "" };

        const std::string l_KeyName = p_Line.substr(0, l_EqualPos);
        const std::string l_KeyValue = p_Line.substr(l_EqualPos + 1);
        return { l_KeyName, l_KeyValue };
    };

    // prod.keys
    {
        std::cout << "\nLoading keys from: " << p_ProdKeys << '\n';
        std::ifstream l_File(p_ProdKeys);
        if (!l_File.is_open())
            throw std::runtime_error("Failed to open key file: " + p_ProdKeys.string());

        std::string l_Line;
        while (std::getline(l_File, l_Line))
        {
            auto [l_KeyName, l_KeyValue] = l_ExtractKeyValue(l_Line);

            if (l_KeyName.empty() || l_KeyValue.empty())
                continue;

            std::erase_if(l_KeyValue, ::isspace);
            std::erase_if(l_KeyName, ::isspace);
            std::ranges::transform(l_KeyName, l_KeyName.begin(), ::tolower);

            if (m_KeyNames.contains(l_KeyName))
            {
                const KeyData& l_KeyData = m_KeyNames[l_KeyName];
                std::cout << "\tKey: " << l_KeyName << " (" << l_KeyData.first << ", " << l_KeyData.second << ") -> " << l_KeyValue << '\n';
                m_Keys[l_KeyData] = HexStringToByteArray<32>(l_KeyValue);
            }
            else if (l_KeyName.starts_with("eticket_extended_kek"))
            {
                std::cout << "\tKey: " << l_KeyName << " -> " << l_KeyValue << '\n';
                m_ExtendedETicket = HexStringToByteArray<576>(l_KeyValue);
            }
            else if (l_KeyName.starts_with("encrypted_keyblob_"))
            {
                u32 l_KeyblobID = std::stoi(l_KeyName.substr(18, 2), nullptr, 16);

                std::cout << "\tKey: " << l_KeyName << " (" << l_KeyblobID << ") -> " << l_KeyValue << '\n';
                m_EncryptedKeyblobs[l_KeyblobID] = HexStringToByteArray<176>(l_KeyValue);
            }
            else if (l_KeyName.starts_with("keyblob_") && !l_KeyName.starts_with("keyblob_k"))
            {
                std::string l_KeyNameNumber = l_KeyName.substr(8, 2);
                if (!isStringNumber(l_KeyName, 8, 2))
                    continue;

                u32 l_KeyblobID = std::stoi(l_KeyNameNumber, nullptr, 16);

                std::cout << "\tKey: " << l_KeyName << " (" << l_KeyblobID << ") -> " << l_KeyValue << '\n';
                m_Keyblobs[l_KeyblobID] = HexStringToByteArray<144>(l_KeyValue);
            }
            else
            {
                std::string l_KeyNameNoNumber = l_KeyName.substr(0, l_KeyName.size() - 2);
                if (m_KeyNames.contains(l_KeyNameNoNumber) && isStringNumber(l_KeyName, l_KeyName.size() - 2, 2))
                {
                    const KeyData& l_VarKeyData = m_KeyNames[l_KeyNameNoNumber];
                    const u32 l_VarKeyID = std::stoi(l_KeyName.substr(l_KeyName.size() - 2, 2), nullptr, 16);
                    KeyData l_KeyData{
                        .keySize = l_VarKeyData.keySize,
                        .keyType = l_VarKeyData.keyType,
                        .first =   l_VarKeyData.first == 0 ? l_VarKeyID : l_VarKeyData.first,
                        .second =  l_VarKeyData.first == 0 ? 0          : l_VarKeyID
                    };
                    
                    if (l_VarKeyData.keyType == KeyData::K128Type::KEY_AREA)
                        l_KeyData.second = l_VarKeyData.second;

                    
                    std::cout << "\tKey: " << l_KeyName << " (" << l_KeyData.first << ", " << l_KeyData.second << ") -> " << l_KeyValue << '\n';
                    m_Keys[l_KeyData] = HexStringToByteArray<32>(l_KeyValue);
                }
            }
        }
    }

    // title.keys
    {
        std::cout << "\nLoading keys from: " << p_TitleKeys << '\n';
        std::ifstream l_File(p_TitleKeys);
        if (!l_File.is_open())
            throw std::runtime_error("Failed to open key file: " + p_TitleKeys.string());
        
        std::string l_Line;
        while (std::getline(l_File, l_Line))
        {
            auto [l_KeyName, l_KeyValue] = l_ExtractKeyValue(l_Line);

            if (l_KeyName.empty() || l_KeyValue.empty())
                continue;

            std::erase_if(l_KeyValue, ::isspace);
            std::erase_if(l_KeyName, ::isspace);

            ByteArray<16> l_KeyID = HexStringToByteArray<16>(l_KeyName);
            ByteArray<32> l_Key = HexStringToByteArray<32>(l_KeyValue);

            KeyData l_KeyData{
                .keySize = KeyData::K128,
                .keyType = KeyData::K128Type::TITLE_KEY,
                .first = *reinterpret_cast<u64*>(l_KeyID.data()),
                .second = *reinterpret_cast<u64*>(l_KeyID.data() + 8)
            };

            std::cout << "\tKey: " << l_KeyName << " (" << l_KeyData.first << ", " << l_KeyData.second << ") -> " << l_KeyValue << '\n';
            m_Keys[l_KeyData] = l_Key;
        }
    }
}

const ByteArray<32>& swroo::filesys::KeyManager::getKey(const KeyData::KeySize p_Size, const u8 p_KeyType, const u64 p_First, const u64 p_Second) const
{
    const KeyData l_KeyData{
        .keySize = p_Size,
        .keyType = p_KeyType,
        .first = p_First,
        .second = p_Second
    };
    if (m_Keys.contains(l_KeyData))
    {
        return m_Keys.at(l_KeyData);
    }
    throw std::runtime_error("Key not found: " + std::to_string(p_Size) + ", " + std::to_string(p_KeyType) + ", " + std::to_string(p_First) + ", " + std::to_string(p_Second));
}

bool swroo::filesys::KeyManager::hasKey(const KeyData::KeySize p_Size, const u8 p_KeyType, const u64 p_First, const u64 p_Second) const
{
    const KeyData l_KeyData{
        .keySize = p_Size,
        .keyType = p_KeyType,
        .first = p_First,
        .second = p_Second
    };
    return m_Keys.contains(l_KeyData);
}

const ByteArray<144>& swroo::filesys::KeyManager::getKeyblob(const u32 p_KeyblobID) const
{
    if (m_Keyblobs.contains(p_KeyblobID))
    {
        return m_Keyblobs.at(p_KeyblobID);
    }
    throw std::runtime_error("Keyblob not found: " + std::to_string(p_KeyblobID));
}

const ByteArray<176>& swroo::filesys::KeyManager::getEncryptedKeyblob(const u32 p_KeyblobID) const
{
    if (m_EncryptedKeyblobs.contains(p_KeyblobID))
    {
        return m_EncryptedKeyblobs.at(p_KeyblobID);
    }
    throw std::runtime_error("Encrypted keyblob not found: " + std::to_string(p_KeyblobID));
}

const ByteArray<576>& swroo::filesys::KeyManager::getExtendedETicket() const
{
    return m_ExtendedETicket;
}
