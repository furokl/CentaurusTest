#pragma once

#include <Lmcons.h>
#include <iostream>

struct FClientInfo
{
    inline static int idCounter{};
    
    int id;
    SOCKET sock;
    char time[20];
    char user[UNLEN + 1];
    char name[MAX_COMPUTERNAME_LENGTH + 1];
    char ipv4[16];
    
    FClientInfo()
        : id(idCounter++)
        , sock{}
        , time{}
        , user{}
        , name{}
        , ipv4{}
    {
    }

    friend std::ostream& operator<< (std::ostream &os, const FClientInfo &clientInfo)
    {
        return os   << "ID: " << clientInfo.id << " | "
                    << "Time: " << clientInfo.time << " | "
                    << "Username: " << clientInfo.user << " | "
                    << "PC Name: " << clientInfo.name << " | "
                    << "IP: " << clientInfo.ipv4 << std::endl;
    }
};