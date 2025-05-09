#include "engine.hpp"

swroo::Engine::Engine(const std::filesystem::path& p_ProdKeys, const std::filesystem::path& p_TitleKeys)
    : m_KeyManager(p_ProdKeys, p_TitleKeys)
{
}

swroo::filesys::PFS swroo::Engine::loadFPS0(const std::filesystem::path& p_Path)
{
    FileReader* l_MainFile = new MainFileReader(p_Path);
    return filesys::PFS(l_MainFile, this);
}
