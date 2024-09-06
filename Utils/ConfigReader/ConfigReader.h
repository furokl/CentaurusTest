#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

class ConfigReader {
public:
    ConfigReader(std::string filePath_);
    std::string getValue(const std::string& key) const;
    std::string getServerIP() const;
    std::string getServerPort() const;

private:
    void loadConfig();

    std::string filePath;
    std::unordered_map<std::string, std::string> configMap;
};
