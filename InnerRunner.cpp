#include <stdio.h>
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


#include "Common.h"
#include "InnerRunner.h"
#include "OuterRunner.h"
#include "Log.h"

#define START_HEADER_LEN    16

InnerRunner::InnerRunner() :
    Runner(INNER_RUNNER),
    mOuterRunner(NULL)
{
    if (mOuterRunner == NULL)
    {
        mOuterRunner = new OuterRunner(this);
    }
}

InnerRunner::~InnerRunner()
{
    if (mOuterRunner != NULL)
    {
        delete mOuterRunner;
        mOuterRunner = NULL;
    }
}

void InnerRunner::SocketTimeout()
{
    SendPendingMsg();
}

void InnerRunner::SocketRead(int sockFd)
{
    bool successFlag = false;

    char buf[MSG_LEN];
    int recvLen = 0;

    while (true)
    {
        recvLen = recv(mSockFd, buf, MSG_LEN, 0);

        if (recvLen > 0)
        {
            if (mOuterRunner == NULL)
            {
                mOuterRunner = new OuterRunner(this);
            }

        	if ((gRoleType == MIDDLE) || (gRoleType == SERVER))
        	{
        		for (int index = 0; index < recvLen; index++)
        		{
        			buf[index] = buf[index] ^ SIMPLE_XOR_VALUE;
        		}
        	}

            //printf("INNER: Receive message from client. SockFd %d, length %d\n", mSockFd, recvLen);
            //PrintBuf(buf, recvLen);
            
            ProcessDataMessage(buf, recvLen);
            successFlag = true;

            break;
        }
        else if (recvLen == 0)
        {
            // socket is closed normally
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
                gLog.Write("InnerRunner::SocketRead: socket closed abnormally. errno %d\n", errno);
                successFlag = false;
                
                break;
            }
        }
    }

    if (!successFlag)
    {
        ClearAll();
    }
}

void InnerRunner::AddPendingMsg(char* buf, int bufLen)
{
    string pendingMsg(buf, bufLen);

    mSendMsg.push_back(pendingMsg);
    SendPendingMsg();
}

void InnerRunner::SendPendingMsg()
{
    if (mSockFd > 0)
    {
        for (int index = 0; index < mSendMsg.size(); index++)
        {
        	char* msgPos = ((char*)(mSendMsg[index].c_str()));
        	int msgLen = mSendMsg[index].length();

        	char* msgBuf = new char[msgLen + 1];

        	memset(msgBuf, 0, msgLen + 1);
        	memcpy(msgBuf, msgPos, msgLen);

            //printf("INNER: Send message to client. mSockFd %d, length %d\n", mSockFd, msgLen);
            //PrintBuf(msgBuf, msgLen);

        	if ((gRoleType == MIDDLE) || (gRoleType == SERVER))
        	{
        		for (int index = 0; index < msgLen; index++)
        		{
        			msgBuf[index] = msgBuf[index] ^ SIMPLE_XOR_VALUE;
        		}
        	}

            int sendBytes = SockSendPacket(mSockFd, msgBuf, msgLen);

            if (sendBytes <= 0)
            {
            	delete [] msgBuf;
                ClearAll();

                return;
            }

            delete [] msgBuf;
        }

        mSendMsg.clear();
    }
}

void InnerRunner::ProcessDataMessage(char* buf, int bufLen)
{
    if ((mOuterRunner != NULL) && (bufLen > 0))
    {
        mOuterRunner->AddPendingMsg(buf, bufLen);
    }
}

void InnerRunner::ClearAll()
{
    //printf("Error! Clear all! SockFd %d\n", mSockFd);
    
    if (mOuterRunner != NULL)
    {
        delete mOuterRunner;
        mOuterRunner = NULL;
    }

    delete this;
}

void InnerRunner::HandleError()
{
    ClearAll();
}


