#include "Server.h"
#include "ConfigReader.h"
#include "Constants.h"

int main()
{
    ConfigReader config(Centaurus::configPath.string());

    u_short serverPort = std::stoi(config.getServerPort());
    
    Server server(serverPort);
    server.start();
}