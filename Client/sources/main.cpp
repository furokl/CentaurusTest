#include "Client.h"
#include "ConfigReader.h"
#include "Constants.h"

int main()
{
    try
    {
        ConfigReader config(CONFIG_PATH);
        u_short serverPort = std::stoi(config.getServerPort());
        Client client(config.getServerIP().c_str(), serverPort);
        client.start();
    }
    catch (const std::exception& e)
    {
        MessageBox(NULL, e.what(), "Client", MB_OK);
    }
}
