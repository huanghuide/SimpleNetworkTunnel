#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Common.h"
#include "Log.h"
#include "RunnerManager.h"
#include "Runner.h"
#include "InnerInitiator.h"

bool gStopFlag = false;

RoleType gRoleType = SERVER;
int gInnerListenPort = BB_LISTEN_PORT;

std::string gOuterDstIpAddrArray[MAX_DST_NUM];
int gOuterDstPortArray[MAX_DST_NUM];
int gOuterDstCounter = 0;
int gOuterRunnerCounter = 0;

Log gLog;
RunnerManager gRunnerManager;
InnerInitiator* gInnerInitiator = NULL;

fd_set gReadSockFdSet;
fd_set gWriteSockFdSet;

int gMaxSockFd = 0;

void Trim(string& str)
{
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

string GetBBTime()
{
    string ret = "";
    char buf[256];
    struct tm* local;
    time_t t;

    t = time(NULL);
    local = localtime(&t);

    sprintf(buf, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
    ret += buf;

    return ret;
}

void AddSockFdIntoSet(int sockFd)
{
    FD_SET(sockFd, &gReadSockFdSet);
    FD_SET(sockFd, &gWriteSockFdSet);
    
    gMaxSockFd = (gMaxSockFd > sockFd) ? gMaxSockFd : sockFd;
}

void DelSockFdFromSet(int sockFd)
{
    if ((sockFd > 0) && (FD_ISSET(sockFd, &gReadSockFdSet)))
    {
        FD_CLR(sockFd, &gReadSockFdSet);
    }

    if ((sockFd > 0) && (FD_ISSET(sockFd, &gWriteSockFdSet)))
    {
        FD_CLR(sockFd, &gWriteSockFdSet);
    }

    while ((gMaxSockFd > 0) && (!FD_ISSET(gMaxSockFd, &gReadSockFdSet)))
    {
        gMaxSockFd--;
    }
}

void CloseSockFd(int sockFd)
{
    if (sockFd > 0)
    {
        gRunnerManager.DeleteRunner(sockFd);
        DelSockFdFromSet(sockFd);
        
        close(sockFd);        
    }
}

int SockSendPacket(int sockFd, char* buf, int bufLen)
{
    int ret = -1;

    if ((sockFd > 0) && (buf != NULL) && (bufLen > 0))
    {
        int nLeft = bufLen;        
        int nSend = 0;
        int tmpRet = 0;
        int errCount = 0;

        while (nLeft > 0)
        {
            tmpRet = send(sockFd, (buf + nSend), nLeft, 0);

            if (tmpRet < 0)
            {
                if (/*(errno == EAGAIN) || */(errno == EWOULDBLOCK) || (errno == EINTR) || (errno == EINPROGRESS))
                {
                    errCount++;
                    
                    if (errCount > 5)
                    {
                        gLog.Write("SockSendPacket: error count exceeds 2 times! errno %d\n", errno);
                        break;
                    }
                    else
                    {
                        usleep(1000);
                        continue;
                    }
                }
                else
                {
                    gLog.Write("SockSendPacket: send socket error! errno %d\n", errno);
                    break;
                }
            }
            else if (tmpRet == 0)
            {
                ret = 0;
                break;
            }
            else
            {
                errCount = 0;
                
                nLeft -= tmpRet;
                nSend += tmpRet;
                ret = nSend;
            }
        }
    }

    return ret;
}

void PrintBuf(char* buf, int bufLen)
{
    /*
    printf("\n------------------ PRINT BUF (length: %d) ----------------\n", bufLen);
    printf("The string is: %s\n", buf);
    printf("----------\n");
   
    printf("Octets are:\n");

    for (int index = 0; index < bufLen; index++)
    {        
        if ((index > 0) && (index % 32 == 0))
        {
            printf("\n");
        }

        printf("%02x ", (unsigned char)(buf[index]));
    }
    printf("\n------------------------------------------------------------\n");
    */
}

void EncodeSendBuf(char* buf, int bufLen, char* xorValue, int totalSendBytes)
{
    for (int index = 0; index < bufLen; index++)
    {
        buf[index] = buf[index] ^ (xorValue[(index + totalSendBytes) % 4]);
    }
}

void DecodeRecvBuf(char* buf, int bufLen, char* xorValue, int totalRecvBytes)
{
    EncodeSendBuf(buf, bufLen, xorValue, totalRecvBytes);
}



