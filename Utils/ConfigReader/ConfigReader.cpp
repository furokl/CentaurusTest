#include "ConfigReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

ConfigReader::ConfigReader(std::string filePath_)
    : filePath(std::move(filePath_))
{
    loadConfig();
}

void ConfigReader::loadConfig() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open configuration file.");
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            configMap[key] = value;
        }
    }
}

std::string ConfigReader::getValue(const std::string& key) const {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    return "";
}

std::string ConfigReader::getServerIP() const
{
    return getValue("server-ip");
}

std::string ConfigReader::getServerPort() const
{
    return getValue("server-port");
}
