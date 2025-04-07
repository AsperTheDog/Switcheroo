#pragma once
#include "filesys/loader/pfs.hpp"
#include "filesys/loader/key_manager.hpp"

namespace swroo
{
    class Engine
    {
    public:
        Engine(const std::filesystem::path& p_ProdKeys, const std::filesystem::path& p_TitleKeys);

        [[nodiscard]] filesys::PFS loadFPS0(const std::filesystem::path& p_Path);

        filesys::KeyManager& getKeyManager() { return m_KeyManager; }

    private:
        filesys::KeyManager m_KeyManager;
    };
}

