#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
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
#include <string.h>
#include <string>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <fcntl.h>
#include <algorithm>
#include <netinet/tcp.h>


#include "Common.h"
#include "RunnerManager.h"
#include "OuterRunner.h"
#include "InnerRunner.h"
#include "Log.h"

OuterRunner::OuterRunner(InnerRunner* innerRunner) :
    Runner(OUTER_RUNNER),
    mInnerRunner(innerRunner),
    mSendStatus(INIT)
{
    mDstIpAddr = gOuterDstIpAddrArray[gOuterRunnerCounter % gOuterDstCounter];
    mDstPort = gOuterDstPortArray[gOuterRunnerCounter % gOuterDstCounter];

    gOuterRunnerCounter++;
    gOuterRunnerCounter = gOuterRunnerCounter % gOuterDstCounter;
}

OuterRunner::~OuterRunner()
{
}

void OuterRunner::SocketRead(int sockFd)
{
    bool successFlag = false;
    char buf[MSG_LEN];
    int recvLen = 0;
    
    if (mSendStatus == ESTABLISHED)
    {
        while (true)
        {
            recvLen = recv(mSockFd, buf, MSG_LEN, 0);

            if (recvLen > 0)
            {
            	if ((gRoleType == MIDDLE) || (gRoleType == CLIENT))
            	{
            		for (int index = 0; index < recvLen; index++)
            		{
            			buf[index] = buf[index] ^ SIMPLE_XOR_VALUE;
            		}
            	}

                //printf("Receive from the remote server. SockFd %d, Length %d\n", mSockFd, recvLen);
                //PrintBuf(buf, recvLen);
                
                if (mInnerRunner != NULL)
                {
                    mInnerRunner->AddPendingMsg(buf, recvLen);
                }
                
                successFlag = true;

                break;
            }
            else if (recvLen == 0)
            {
                // socket is normally closed
                successFlag = false;
                
                break;
            }
            else
            {
                if ((errno == EWOULDBLOCK) || (errno == EINTR) || (errno == EINPROGRESS))
                {
                    usleep(1000);
                    continue;
                }
                else 
                {
                    gLog.Write("OuterRunner::SocketRead receive socket error! errno %d\n", errno);
                    successFlag = false;
                    
                    break;
                }
            }
        }

        if (!successFlag)
        {
            HandleError();
        }
    }  
}

void OuterRunner::SocketTimeout()
{
    //printf("SocketTimeout: mSockFd %d, mSendStatus %d\n", mSockFd, mSendStatus);
    
    if (mSendStatus == INIT)
    {
    }
    if (mSendStatus == CONNECTING)
    {
        ProcessConnectingStatus();
    }
    else
    {
        SendPendingMsg();
    }
}

void OuterRunner::AddPendingMsg(char* buf, int bufLen)
{
    string pendingMsg(buf, bufLen);

    mSendMsg.push_back(pendingMsg);
    SendPendingMsg();
}

void OuterRunner::SendPendingMsg()
{
    if (mSendStatus == INIT)
    {
        ProcessInitStatus();
    }
    else if (mSendStatus == ESTABLISHED)
    {
        for (int index = 0; index < mSendMsg.size(); index++)
        {
        	char* msgPos = ((char*)(mSendMsg[index].c_str()));
        	int msgLen = mSendMsg[index].length();

        	char* msgBuf = new char[msgLen + 1];

        	memset(msgBuf, 0, msgLen + 1);
        	memcpy(msgBuf, msgPos, msgLen);

            //printf("Sending message to the remote server. SockFd %d, msgLen %d\n", mSockFd, msgLen);
            //PrintBuf(msgBuf, msgLen);

        	if ((gRoleType == MIDDLE) || (gRoleType == CLIENT))
        	{
        		for (int index = 0; index < msgLen; index++)
        		{
        			msgBuf[index] = msgBuf[index] ^ SIMPLE_XOR_VALUE;
        		}
        	}

            int sendBytes = SockSendPacket(mSockFd, msgBuf, msgLen);
                        
            if (sendBytes != mSendMsg[index].length())
            {
            	delete [] msgBuf;
                HandleError();

                return;
            }

            delete [] msgBuf;
        }

        mSendMsg.clear();
    }
}

void OuterRunner::HandleError()
{
    if (mSockFd > 0)
    {
        //printf("OuterRunner Error! sockFd %d, errno %d\n", mSockFd, errno);
        CloseSockFd(mSockFd);
        mSockFd = 0;
    }
    
    if (mInnerRunner != NULL)
    {
        mInnerRunner->HandleError();
    }
}

void OuterRunner::ProcessInitStatus()
{
    if (mSockFd <= 0)
    {
        mSockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (mSockFd < 0)
        {
            gLog.Write("socket");
            HandleError();
        }

        gRunnerManager.AddRunner(mSockFd, this);

        // Set as non-block socket
        int flags = fcntl(mSockFd, F_GETFL, 0);
        fcntl(mSockFd, F_SETFL, flags | O_NONBLOCK);

        // Set the timeout for sending and receiving
        struct timeval timeout = {3, 0};  
        
        flags = setsockopt(mSockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
        if (flags != 0)
        {
            HandleError();  
        }

        flags = setsockopt(mSockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
        if (flags != 0)
        {
            HandleError();  
        }

        // Set the send and recv buffer
        int bufSize = TCP_SOCK_BUF_SIZE;
        int bufLen = sizeof(int);

        flags = setsockopt(mSockFd, SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, bufLen);
        if (flags != 0)
        {
            gLog.Write("Set socket sending buffer error!");        
            HandleError();  
        }

        flags = setsockopt(mSockFd, SOL_SOCKET, SO_RCVBUF, (char *)&bufSize, bufLen);
        if (flags != 0)
        {
            gLog.Write("Set socket receiving buffer error!");        
            HandleError();  
        }

        // Disable Nagle algorithm
        int enable = 1;
        flags = setsockopt(mSockFd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
        if (flags != 0)
        {
            gLog.Write("Disable NAGLE algorithm error!");        
            HandleError();  
        }
    }

    struct sockaddr_in addr;
    
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mDstPort);
    addr.sin_addr.s_addr = inet_addr(mDstIpAddr.c_str());

    int retConn = connect(mSockFd, (sockaddr*)&addr, sizeof(addr));
    
    if (retConn != 0)
    {
        if ((errno == EINPROGRESS) || (errno == EALREADY))
        {
            mSendStatus = CONNECTING;
        }
        else if (errno == EISCONN)
        {
            ProcessEstablishedStatus();
            mSendStatus = ESTABLISHED;

            ProcessEstablishedStatus();
        }
        else
        {
            gLog.Write("OuterRunner::ProcessConnectingStatus: errno %d\n", errno);
            HandleError();                
        }
    }
    else
    {
        gRunnerManager.AddRunner(mSockFd, this);

        mSendStatus = ESTABLISHED;

        ProcessEstablishedStatus();
    }
}

void OuterRunner::ProcessConnectingStatus()
{
    if (mSockFd > 0)
    {
        struct sockaddr_in addr;
        
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(mDstPort);
        addr.sin_addr.s_addr = inet_addr(mDstIpAddr.c_str());

        int retConn = connect(mSockFd, (sockaddr*)&addr, sizeof(addr));
        
        if (retConn != 0)
        {
            if ((errno == EINPROGRESS) || (errno == EALREADY))
            {
                mSendStatus = CONNECTING;
            }
            else if (errno == EISCONN)
            {
                ProcessEstablishedStatus();
                mSendStatus = ESTABLISHED;
            }
            else
            {
                gLog.Write("OuterRunner::ProcessConnectingStatus: errno %d\n", errno);
                HandleError();                
            }
        }
        else
        {
            ProcessEstablishedStatus();
            mSendStatus = ESTABLISHED;
        }
    }
}

void OuterRunner::ProcessEstablishedStatus()
{
    if (mSockFd > 0)
    {
        // Set as block socket
        int flags = fcntl(mSockFd, F_GETFL, 0);
        int ret = fcntl(mSockFd, F_SETFL, flags & ~O_NONBLOCK);

        if (ret != 0)
        {
            gLog.Write("OuterRunner::ProcessEstablishedStatus: Set socket block failed!\n");
            HandleError();
        }
        else
        {        
            AddSockFdIntoSet(mSockFd);
        }

        SendPendingMsg();
    }
}


