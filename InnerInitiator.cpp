#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/tcp.h>


#include "InnerInitiator.h"
#include "RunnerManager.h"
#include "Common.h"
#include "InnerRunner.h"
#include "Log.h"


InnerInitiator::InnerInitiator(int listenPort) :
    Runner(INNER_INITIATOR),
    mListenPort(listenPort)
{
    mAddrLen = sizeof(struct sockaddr_in);
    bzero(&mAddr, mAddrLen);

    Init();
}

InnerInitiator::~InnerInitiator()
{
}

void InnerInitiator::Init()
{
    mSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mSockFd < 0)
    {
        gLog.Write("Inner Initiator socket error!");
        exit(1);
    }

    // Set the timeout for sending and receiving
    struct timeval timeout = {3, 0};  
    
    int flags = setsockopt(mSockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
    if (flags != 0)
    {
        gLog.Write("Set socket send timeout error!");        
        exit(1); 
    }

    flags = setsockopt(mSockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    if (flags != 0)
    {
        gLog.Write("Set socket receiving timeout error!");        
        exit(1);
    }

    // Set the send and recv buffer
    int bufSize = TCP_SOCK_BUF_SIZE;
    int bufLen = sizeof(int);
    
    flags = setsockopt(mSockFd, SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, bufLen);
    if (flags != 0)
    {
        gLog.Write("Set socket sending buffer error!");        
        exit(1);
    }

    flags = setsockopt(mSockFd, SOL_SOCKET, SO_RCVBUF, (char *)&bufSize, bufLen);
    if (flags != 0)
    {
        gLog.Write("Set socket receiving buffer error!");        
        exit(1);
    }

    // Disable Nagle algorithm
    int enable = 1;
    flags = setsockopt(mSockFd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    if (flags != 0)
    {
        gLog.Write("Disable NAGLE algorithm error!");        
        exit(1);
    }


    bzero(&mAddr, sizeof(mAddr));
    
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(mListenPort);
    mAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Reuse the socket
    int reuseFlag = 1;
    if (setsockopt(mSockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseFlag, sizeof(int)) != 0)
    {
        gLog.Write("Cannot reuse the socket");
        
        exit(1);
    }

    // Bind the socket
    if (bind(mSockFd, (sockaddr*)&mAddr, mAddrLen) != 0)
    {
        gLog.Write("Inner Initiator bind socket error!");

        exit(1);
    }

    // Listen the socket
    if (listen(mSockFd, 256) != 0)
    {
        gLog.Write("Inner Initiator listen socket error!");

        exit(1);
    }

    // Add into runner manager
    AddSockFdIntoSet(mSockFd);   
    gRunnerManager.AddRunner(mSockFd, this);
}

void InnerInitiator::SocketRead(int sockFd)
{
    int newSockFd = accept(mSockFd, (sockaddr*)&mAddr, (socklen_t*)&mAddrLen);

    if (newSockFd > 0)
    {
        InnerRunner* pInnerRunner = new InnerRunner();
        
        if (pInnerRunner != NULL)
        {
            pInnerRunner->SetSockFd(newSockFd);
            //printf("New INNER runner socket. newSockFd %d\n", newSockFd);
        
            AddSockFdIntoSet(newSockFd);            
            gRunnerManager.AddRunner(newSockFd, pInnerRunner);
        }
    }
}

