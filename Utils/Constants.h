#pragma once
#include <filesystem>
#include <string>

namespace Centaurus
{
    const std::string
            configFileName = "server.properties";
    
    const std::filesystem::path
            contentPath = std::filesystem::current_path().parent_path().parent_path(),
            configPath = contentPath / configFileName;

    namespace cmd
    {
        const std::string
            connect = "/connect";
    }
}
