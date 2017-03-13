#ifndef _RUNNER_H_
#define _RUNNER_H_

#include "Common.h"

class Runner
{
public:
    Runner(RunnerType type);
    virtual ~Runner();
    
    RunnerType GetRunnerType() { return mRunnerType; }  
    
    virtual void SocketRead(int sockFd) = 0;
	virtual void SocketWrite(int sockFd) {};
    virtual void SocketTimeout() {};

protected:
    RunnerType mRunnerType;

    int mSockFd;

	vector<string> mSendMsg;
};

#endif