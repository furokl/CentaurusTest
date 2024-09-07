#pragma once

#include <string>

namespace Centaurus
{
    namespace cmd
    {
        const std::string
            help = "/help",
            connect = "/connect",
            screenshot = "/scn",
            list = "/list",
            exit = "/exit",

            unknownMsg = "Unknown command (/help)",
            helpMsg
                = "\t\'" + screenshot + '\'' + " \'id\'" + "\t\t" + "screenshot" + '\n'
                + "\t\'" + list + '\'' + "\t\t\t" + "list all connected clients" + '\n'
                + "\t\'" + exit + '\'' + "\t\t\t" + "turn off the server" + '\n'
                + "\t\'" + help + '\'' + "\t\t\t" + "display commands";
    }
}
