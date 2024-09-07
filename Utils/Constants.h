#pragma once
#include <filesystem>
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
            quit = "/q",

            unknownMsg = "Unknown command (/help)",
            helpMsg
                = '\'' + screenshot + '\'' + "\t\t\t" + "screenshot" + '\n'
                + '\'' + list + '\'' + "\t\t\t" + "list all connected clients" + '\n'
                + '\'' + exit + '\'' + " or " + '\'' + quit + '\'' + '\t' + "turn off the server" + '\n'
                + '\'' + help + '\'' + "\t\t\t" + "display commands";
    }
}
