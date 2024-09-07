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
};
