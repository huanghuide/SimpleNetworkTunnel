#ifndef _OUTERRUNNER_H_
#define _OUTERRUNNER_H_

#include <string>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Runner.h"

class InnerRunner;

using namespace std;

enum RunnerStatus
{
    INIT,
    CONNECTING,
    ESTABLISHED
};

class OuterRunner : public Runner
{
public:
    OuterRunner(InnerRunner* innerRunner);
    virtual ~OuterRunner();

    void SocketRead(int sockFd);
    void SocketTimeout();

	void AddPendingMsg(char* buf, int bufLen);
	void SendPendingMsg();

protected:
    void HandleError();

    void ProcessInitStatus();
    void ProcessConnectingStatus();
    void ProcessEstablishedStatus();
    
    InnerRunner* mInnerRunner;
    
    RunnerStatus mSendStatus;
};

#endif
