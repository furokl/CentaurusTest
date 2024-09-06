#include "Client.h"
#include "ConfigReader.h"
#include "Constants.h"

int main()
{
    ConfigReader config(Centaurus::configPath.string());

    u_short serverPort = std::stoi(config.getServerPort());
    
    Client client(config.getServerIP().c_str(), serverPort);
    client.start();
}
