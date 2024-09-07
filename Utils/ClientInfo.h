#pragma once

#include <atomic>
#include <Lmcons.h>

struct FClientInfo
{
    inline static std::atomic<int> idCounter{};
    
    int id;
    char time[20];
    char user[UNLEN + 1];
    char name[UNLEN + 1];
    char ipv4[16];
    
    FClientInfo()
        : id(idCounter++)
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