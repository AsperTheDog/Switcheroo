
#include <filesystem>
#include <iostream>

#include "engine.hpp"
#include "filesys/loader/pfs.hpp"

i32 main(const i32 argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <path_to_pfs> <path_to_key_folder>" << '\n';
        return 1;
    }

    const std::filesystem::path l_FilePath = argv[1];
    // Check if the file exists
    if (!std::filesystem::exists(l_FilePath))
    {
        std::cerr << "File does not exist: " << l_FilePath << '\n';
        return 1;
    }
    // Generate keys
    std::filesystem::path l_ProdKeysPath = argv[2];
    l_ProdKeysPath /= "prod.keys";
    std::filesystem::path l_TitleKeysPath = argv[2];
    l_TitleKeysPath /= "title.keys";

    swroo::Engine l_Engine(l_ProdKeysPath, l_TitleKeysPath);
    swroo::filesys::PFS l_PFS = l_Engine.loadFPS0(l_FilePath);

    std::cout << "PFS0 loaded successfully!" << '\n';
    return 0;
}
