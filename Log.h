#ifndef _LOG_H_
#define _LOG_H_

#include "Common.h"

class Log
{
public:
    Log() {}
    virtual ~Log() {}

    void Write(string log);
    void Write(char* format, ...);
};


#endif

