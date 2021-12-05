#ifndef _ISOCKET_H_
#define _ISOCKET_H_

#include"platform.h"

class ISocket
{
public:
    enum
    {
        ISOCKET_SUCCESS = 0,
        ISOCKET_USERQUIT = -0x100,
        ISOCKET_ERROR,
        ISOCKET_SERVERCLOSED,
        ISOCKET_TIMEOUT,
        ISOCKET_NODATA
    };
public:
    ISocket(){};
	virtual ~ISocket(){};
	virtual int CreateSocket() = 0;
	virtual int CloseSocket() = 0;
	virtual int Connect(const char* ip, unsigned short port, long long timeout = 3000) = 0;
	virtual int Recv(void *buf, size_t n, int flags = 0, struct sockaddr_in* addr = NULL) = 0;
	virtual int Send(void *buf, size_t n, int flags = 0, struct sockaddr_in* addr = NULL) = 0;
	virtual void UserQuit() = 0;
	virtual SOCKET GetFd() = 0;
};

#endif
