#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "Runner.h"
#include "RunnerManager.h"

Runner::Runner(RunnerType type) :
    mRunnerType(type),
    mSockFd(0)
{
}

Runner::~Runner()
{
    CloseSockFd(mSockFd);
}

