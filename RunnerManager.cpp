#include <time.h>
#include <stdio.h>
#include <set>

#include "RunnerManager.h"

void RunnerManager::DeleteRunner(int sockFd)
{
    map<int, Runner*>::iterator iter = mSockRunnerMap.find(sockFd);

    if (iter != mSockRunnerMap.end())
    {
        mSockRunnerMap.erase(sockFd);
    }
}

void RunnerManager::AddRunner(int sockFd, Runner* pRunner)
{
    //printf("RunnerManager::AddRunner sockFd is %d, pRunner is %p\n", sockFd, pRunner);
    mSockRunnerMap[sockFd] = pRunner;
}

Runner* RunnerManager::FindRunnerByFd(int sockFd)
{
    Runner* ret = NULL;
    
    map<int, Runner*>::iterator iter = mSockRunnerMap.find(sockFd);

    if ((iter != mSockRunnerMap.end()) && (iter->second != NULL))
    {
        ret = iter->second;
    }

    return ret;
}

void RunnerManager::RunnerTimeout()
{
    //printf("RunnerTimeout: current time is %d\n", time(NULL));

    std::set<int> keySet;
    std::set<int>::iterator keySetIter;

    std::map<int, Runner*>::iterator mapIter = mSockRunnerMap.begin();
    while (mapIter != mSockRunnerMap.end())
    {
        keySet.insert(mapIter->first);
        //printf("RunnerTimeout: key is %d\n", mapIter->first);
    
        mapIter++;
    }

    keySetIter = keySet.begin();
    while (keySetIter != keySet.end())
    {
        mapIter = mSockRunnerMap.find(*keySetIter);
        if ((mapIter != mSockRunnerMap.end()) && (mapIter->second != NULL))
        {
            Runner* pRunner = mapIter->second;
            //printf("RunnerTimeout: key is %d, value is %p\n", mapIter->first, mapIter->second);
            
            pRunner->SocketTimeout();
        }
        
        keySetIter++;
    }
}

