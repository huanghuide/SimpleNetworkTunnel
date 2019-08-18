#include <signal.h>

#include "Common.h"
#include "InnerInitiator.h"
#include "RunnerManager.h"
#include "Log.h"

void ParseParamsAndInit(int argc, char *argv[])
{
	int ch;
	opterr = 0;

	// Default setting
	gRoleType = SERVER;
	gInnerListenPort = BB_LISTEN_PORT;
	memset(gOuterDstIpAddr, 0, 20);
	memcpy(gOuterDstIpAddr, BB_SQUID_ADDR, strlen(BB_SQUID_ADDR));
	gOuterDstPort = BB_SQUID_PORT;

	// Read parameters
	while ((ch = getopt(argc, argv, "r:l:o:p:")) != EOF)
	{
		switch (ch)
		{
		case 'r': // Role
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
			gInnerListenPort = atoi(optarg);
			break;

		case 'o': // outer dest IP address
			strcpy(gOuterDstIpAddr, optarg);
			break;

		case 'p': // outer dest TCP port
			gOuterDstPort = atoi(optarg);
			break;
		}
	}

	// Init variables
	gInnerInitiator = new InnerInitiator(gInnerListenPort);

	// Print the parameters
	gLog.Write("role: %d; inner listen port: %d; output dst IP: %s; outer dst port %d",
			gRoleType, gInnerListenPort, gOuterDstIpAddr, gOuterDstPort);
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
