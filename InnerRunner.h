#ifndef _INNERRUNNER_H_
#define _INNERRUNNER_H_

#include "Common.h"
#include "Runner.h"

class OuterRunner;

class InnerRunner : public Runner
{
public:
    InnerRunner();
    virtual ~InnerRunner();

    void SetSockFd(int sockFd) { mSockFd = sockFd; }
    
    void SocketRead(int sockFd);
	void AddPendingMsg(char* buf, int bufLen);
	void SendPendingMsg();
    void HandleError();
    void SocketTimeout();

protected:
    void ClearAll();

    void ProcessCtrlMessage(char* buf, int bufLen);
    void ProcessDataMessage(char* buf, int bufLen);

    OuterRunner* mOuterRunner;
};

#endif