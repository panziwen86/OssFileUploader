#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include "ISocket.h"

#ifndef _WIN32
	#define INVALID_SOCKET -1
#endif

class CTCPClient : public ISocket
{
public:
	CTCPClient(void);
	virtual ~CTCPClient(void);
	virtual int CreateSocket(void);
	virtual int Connect(const char* ip, unsigned short port, long long timeout = 3000);
	virtual int Send(void* buf, size_t n, int flags = 0, struct sockaddr_in* addr = NULL);
	virtual int Recv(void* buf, size_t n, int flags = 0, struct sockaddr_in* addr = NULL);
	virtual int CloseSocket();
	virtual void UserQuit(void);
	virtual SOCKET GetFd();
protected:
	SOCKET m_sock;
	fd_set m_fdr;
	fd_set m_fdw;
	bool m_userQuit;
};

#endif
