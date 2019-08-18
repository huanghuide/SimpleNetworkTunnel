#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <list>
#include <string>
#include <vector>
#include <string.h>

#include "stdio.h"
#include "stdlib.h"

using namespace std;

#define BUF_LEN (16 * 1024)
#define MSG_LEN (2 * 1024)

#define TCP_SOCK_BUF_SIZE 256 * 1024

#define XOR_VALUE_LEN 4

#define UINT8   unsigned char
#define UINT16  unsigned short
#define UINT32  unsigned int
#define UINT64  unsigned long long

#define BB_LISTEN_PORT  7777
#define BB_CTRL_PORT    8888
#define BB_ADMIN_PORT   9999

#define BB_SQUID_ADDR   "127.0.0.1"
#define BB_SQUID_PORT   3128

#define HEADER_BB_FLAG  0x5AC3967B
#define SIMPLE_XOR_VALUE	0xA5

#define MSG_ID_INIT     1

#define MSG_ID_ADMIN_STOP           100
#define MSG_ID_ADMIN_REFRESH_CARD   101
#define MSG_ID_ADMIN_PRINT_TRAFFIC  102

enum RunnerType
{
    INNER_INITIATOR = 1,
    INNER_RUNNER,
    INNER_CONTROLLER,
    INNER_ADMIN,
    OUTER_RUNNER
};

enum RoleType
{
	CLIENT,
    MIDDLE,
	SERVER
};

extern RoleType gRoleType;
extern int gInnerListenPort;
extern char gOuterDstIpAddr[20];
extern int gOuterDstPort;

class Log;
class Runner;
class RunnerManager;
class InnerInitiator;

extern Log gLog;
extern RunnerManager gRunnerManager;
extern InnerInitiator* gInnerInitiator;
extern bool gStopFlag;

extern fd_set gReadSockFdSet;
extern fd_set gWriteSockFdSet;
extern int gMaxSockFd;

void Trim(string& str);
string GetBBTime(); // Return Hour:Minute:Second

void AddSockFdIntoSet(int sockFd);
void DelSockFdFromSet(int sockFd);
void CloseSockFd(int sockFd);

int SockSendPacket(int sockFd, char* buf, int bufLen);

void PrintBuf(char* buf, int bufLen);

void EncodeSendBuf(char* buf, int bufLen, char* xorValue, int totalSendBytes);
void DecodeRecvBuf(char* buf, int bufLen, char* xorValue, int totalRecvBytes);

#endif
