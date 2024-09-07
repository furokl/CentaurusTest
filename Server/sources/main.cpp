#include <iostream>

#include "Server.h"
#include "ConfigReader.h"
#include "Constants.h"

int main()
{
    try
    {
        ConfigReader config(CONFIG_PATH);
        u_short serverPort = std::stoi(config.getServerPort());
        Server server(serverPort);
        server.start();
    }
    catch (const std::exception& e)
    {
        MessageBox(NULL, e.what(), "Server", MB_OK);
    }
}