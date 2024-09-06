#pragma once

#include <atomic>

struct FClientInfo
{
    inline static std::atomic<int> idCounter{};
    
    int id;
    char time[20];
    char user[256];
    char name[256];
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