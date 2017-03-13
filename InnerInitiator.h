#ifndef _INNERINITIATOR_H_
#define _INNERINITIATOR_H_

#include <netinet/in.h>

#include "Runner.h"

class InnerInitiator : public Runner
{
public:
    InnerInitiator(int listenPort);
    virtual ~InnerInitiator();
    
    void Init();
    void SocketRead(int sockFd);

protected:
    struct sockaddr_in mAddr;
    int mAddrLen;
    int mListenPort;
};

#endif