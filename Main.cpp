#include <signal.h>

#include "Common.h"
#include "InnerInitiator.h"
#include "RunnerManager.h"
#include "Log.h"

void ParseIpPort(char* ipPortStr)
{
    gLog.Write("ipPortStr is %s", ipPortStr);

    if (ipPortStr != NULL)
    {
        char* pos = strchr(ipPortStr, ':');
        if (pos != NULL)
        {
            char tmp[200];
            int ipStrLen = pos - ipPortStr;

            memcpy(tmp, ipPortStr, ipStrLen);
            tmp[ipStrLen] = '\0';

            string ipAddr(tmp);            
            int port = atoi(pos + 1);

            if ((ipAddr.length() > 0) && (port > 0))
            {
                gLog.Write("gOuterDstCounter is %d, ipAddr %s, port %d", gOuterDstCounter, ipAddr.c_str(), port);
                gOuterDstIpAddrArray[gOuterDstCounter] = ipAddr;
                gOuterDstPortArray[gOuterDstCounter] = port;
                gOuterDstCounter++;
            }
        }
    }
}

void ParseParamsAndInit(int argc, char *argv[])
{
	int ch;
	opterr = 0;

	// Default setting
	gRoleType = SERVER;
	gInnerListenPort = BB_LISTEN_PORT;

	// Read parameters
	while ((ch = getopt(argc, argv, "r:l:o:")) != EOF)
	{
		switch (ch)
		{
		case 'r': // Role
            gLog.Write("Param (r): %s", optarg);
			if (strcmp(optarg, "client") == 0)
			{
				gRoleType = CLIENT;
			}
            else if (strcmp(optarg, "middle") == 0)
			{
				gRoleType = MIDDLE;
			}
			else
			{
				gRoleType = SERVER;
			}

			break;

		case 'l': // inner listen port
            gLog.Write("Param (l): %s", optarg);
			gInnerListenPort = atoi(optarg);
			break;

		case 'o': // outer dest IP address
            gLog.Write("Param (o): %s", optarg);
            ParseIpPort(optarg);
			break;
		}
	}

	// Init variables
	gInnerInitiator = new InnerInitiator(gInnerListenPort);
}

int main(int argc, char *argv[])
{
    fd_set readFdSet;
    fd_set writeFdSet;
    struct timeval timeout;
    int ret;

    gLog.Write("---------------------");
    gLog.Write("BBServer start !!!!!!");
    gLog.Write("---------------------");

    signal(SIGPIPE, SIG_IGN);
    ParseParamsAndInit(argc, argv);

    while (!gStopFlag)
    {
        FD_ZERO(&readFdSet);         
        readFdSet = gReadSockFdSet;
        writeFdSet = gWriteSockFdSet;

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        ret = select(gMaxSockFd + 1, &readFdSet, NULL, NULL, &timeout);

        if (ret > 0)
        {
            for (int sockFd = gMaxSockFd; sockFd > 0; sockFd--)
            {
                if (FD_ISSET(sockFd, &readFdSet))
                {
                    Runner* pRunner = gRunnerManager.FindRunnerByFd(sockFd);
                    
                    if (pRunner != NULL)
                    {
                        pRunner->SocketRead(sockFd);
                    }

                    FD_CLR(sockFd, &readFdSet);
                }

                if (FD_ISSET(sockFd, &writeFdSet))
                {
                    Runner* pRunner = gRunnerManager.FindRunnerByFd(sockFd);
                    
                    if (pRunner != NULL)
                    {
                        pRunner->SocketWrite(sockFd);
                    }
                
                    FD_CLR(sockFd, &writeFdSet);
                }
            }
        }
        else if (ret == 0)
        {
            gRunnerManager.RunnerTimeout();
        }
    }

    gLog.Write("~~~~~~~~~~~~~~~~~~~");
    gLog.Write("BBServer end !!!!!!");
    gLog.Write("~~~~~~~~~~~~~~~~~~~");
    gLog.Write("                   ");
}
