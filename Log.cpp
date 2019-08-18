#include <stdarg.h>

#include "Log.h"

#define LOG_FILE "/tmp/SimpleProxyLog.txt"

void Log::Write(string log)
{
    FILE* fp;

    fp = fopen(LOG_FILE, "a");

    if (fp != NULL)
    {
        time_t t;
        struct tm* local;
        char buf[2048];
        
        t = time(NULL);
        local = localtime(&t);

        sprintf(buf, "(%04d-%02d-%02d %02d:%02d:%02d) %s\n", (local->tm_year + 1900), (local->tm_mon + 1), local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, log.c_str());
        fputs(buf, fp);  

        //printf(buf); // Also print to the screen

        fclose(fp);
    }
}

void Log::Write(char* format, ...)
{
    char buf[2048];
    va_list args;
    
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    string bufStr(buf);
    Write(bufStr);
}



