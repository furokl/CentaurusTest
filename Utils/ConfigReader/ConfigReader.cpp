#include "ConfigReader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * Конструктор класса ConfigReader.
 * @param filePath_ Путь к конфигурационному файлу.
 */
ConfigReader::ConfigReader(std::string filePath_)
    : filePath(std::move(filePath_))
{
    loadConfig();
}

/**
 * Загружает конфигурацию из файла, заполняет map.
 */
void ConfigReader::loadConfig() {
    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("Unable to open configuration file.");

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            configMap[key] = value;
        }
    }
}

/**
 * Получает значение конфигурационного параметра по ключу.
 * @param key Ключ параметра.
 * @return Значение параметра или пустую строку, если ключ не найден.
 */
std::string ConfigReader::getValue(const std::string& key) const
{
    auto it = configMap.find(key);
    return (it != configMap.end())
        ? it->second
        : "";
}

/**
 * Получает IP-адрес сервера из конфигурации.
 * @return IP-адрес сервера (строка).
 */
std::string ConfigReader::getServerIP() const
{
    return getValue("server-ip");
}

/**
 * Получает порт сервера из конфигурации.
 * @return Порт сервера (строка).
 */
std::string ConfigReader::getServerPort() const
{
    return getValue("server-port");
}
