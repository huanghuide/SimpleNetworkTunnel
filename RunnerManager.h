#ifndef _RUNNERMANAGER_H_
#define _RUNNERMANAGER_H_

#include <map>

#include "Runner.h"

using namespace std;

class RunnerManager
{
public:
    RunnerManager() {}
    virtual ~RunnerManager() {}
    
    void AddRunner(int sockFd, Runner* pRunner);
    void DeleteRunner(int sockFd);
    Runner* FindRunnerByFd(int sockFd);
    
    void RunnerTimeout();

    int GetConnNum() { return mSockRunnerMap.size(); }

protected:
    std::map<int, Runner*> mSockRunnerMap;
};

#endif