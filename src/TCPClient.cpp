#include"TCPClient.h"
#include"debugLog.h"

CTCPClient::CTCPClient(void)
{
	memset(&m_fdr , 0 ,sizeof(fd_set));
	memset(&m_fdw , 0 ,sizeof(fd_set));
	m_sock = INVALID_SOCKET;
	m_userQuit = false;
}

CTCPClient::~CTCPClient(void)
{
	CloseSocket();
}

int CTCPClient::CreateSocket()
{
	m_sock = SOCK_CREATE(AF_INET, SOCK_STREAM, 0);
	if(m_sock <= 2)
    {
		DEBUG_ERROR(COMMON_LIB, "CTCPClient::CreateSocket error invalid socket:%d", m_sock);
        m_sock = INVALID_SOCKET;
		return ISOCKET_ERROR;
    }
	if(true)
    {
#ifndef _WIN32
        int opts = fcntl(m_sock, F_GETFL);
        SOCK_IOCTL( m_sock, F_SETFL, opts | O_NONBLOCK);
#else
		unsigned long ul = 1;
		SOCK_IOCTL(m_sock, FIONBIO, &ul);
#endif
	}
    
	linger lger = {1,0};
    SOCK_SETOPT(m_sock, 0, SO_LINGER, (const char*)&lger, sizeof(linger));
    DEBUG_INFO(COMMON_LIB, "create socket=%d", m_sock);
	return ISOCKET_SUCCESS;
}

void CTCPClient::UserQuit()
{
	m_userQuit = true;
}

int CTCPClient::Connect(const char* ip, unsigned short port, long long timeout)
{
	long long endTime = GetTick() + timeout;
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INET_ADDR(ip);
	memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

	DEBUG_INFO(COMMON_LIB, "CTCPClient::Connect m_sock=%d", m_sock);

	if (SOCK_CONNECT(m_sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
    {    
		int error = -1;
		SOCKLEN len = sizeof(SOCKLEN);
		while (true)
		{
			struct timeval _timeout;
			_timeout.tv_sec = 0;
			_timeout.tv_usec = 10000;

			FD_ZERO(&m_fdw);
			FD_SET(m_sock, &m_fdw);
            int selectRet = SOCK_SELECT(m_sock + 1, NULL, &m_fdw, NULL, &_timeout);
			if(selectRet > 0)
            {
				SOCK_GETOPT(m_sock, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
				if (error != 0)
                {
					if(!(error == EINTR || error == EAGAIN))
					{
						DEBUG_ERROR(COMMON_LIB, "CTCPClient::Connect=%s:%d error: %d", ip, port, error);
                        CloseSocket();
						return ISOCKET_ERROR;
					}
				}
                else
                {
					break;
                }
			}
            if (GetTick() > endTime)
			{
                CloseSocket();
				DEBUG_ERROR(COMMON_LIB, "CTCPClient::Connect=%s:%d failed: SOCKET_TIMEOUT!!", ip, port);
				return ISOCKET_TIMEOUT;
			}
			if (m_userQuit)
			{
                CloseSocket();
				DEBUG_ERROR(COMMON_LIB, "CTCPClient::Connect=%s:%d  failed: SOCKET_USERQUIT!!", ip, port);
				return ISOCKET_USERQUIT;
			}
            SLEEP(10);
		}		
	}
	DEBUG_INFO(COMMON_LIB, "CTCPClient::Connect=%s:%d success.", ip, port);
	return ISOCKET_SUCCESS;
}

int CTCPClient::Send(void* buf, size_t n, int flags, struct sockaddr_in* addr)
{
    if (m_sock == INVALID_SOCKET)
    {
		DEBUG_ERROR(COMMON_LIB, "send err invalid socket m_sock=%d", m_sock);
        return ISOCKET_ERROR;
    }
	FD_ZERO(&m_fdw);
	FD_SET(m_sock, &m_fdw);
	struct timeval	_timeout;
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 100000;
	SOCK_SELECT(m_sock + 1, NULL, &m_fdw, NULL, &_timeout);
	if (!FD_ISSET(m_sock, &m_fdw))
    {
		//DEBUG_ERROR(COMMON_LIB, "CTCPClient::Send failed: SOCKET_NODATA!!");
		return ISOCKET_NODATA;
	}
	return (int)SOCK_SEND(m_sock, (const char*)buf, n, flags);
}

int CTCPClient::Recv(void* buf, size_t n, int flags, struct sockaddr_in* addr)
{
    if (m_sock == INVALID_SOCKET)
	{
		DEBUG_ERROR(COMMON_LIB, "CTCPClient::recv err invalid socket m_sock=%d", m_sock);
        return ISOCKET_ERROR;
    }
	
	FD_ZERO(&m_fdr);
	FD_SET(m_sock, &m_fdr);
	struct timeval	_timeout;
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 10000;
	int selectRet = SOCK_SELECT(m_sock + 1, &m_fdr, NULL, NULL, &_timeout);
	if(selectRet > 0)
    {
        int recvLen = 0;
		if (0 == (recvLen = SOCK_RECV(m_sock, (char*)buf, n, flags)))
		{
			//DEBUG_ERROR(COMMON_LIB, "CTCPClient::Recv failed: SOCKET_SERVERCLOSED!!");
            CloseSocket();
			return ISOCKET_SERVERCLOSED;
		}
        //printf("CTCPClient:RECV %d", recvLen);
		return recvLen;
	}
    else if (selectRet < 0)
	{
		DEBUG_ERROR(COMMON_LIB, "CTCPClient::select failed: SOCKET_ERROR!! error=%d", errno);
        CloseSocket();
		return ISOCKET_ERROR;
	}
    else
    {
		//DEBUG_INFO(COMMON_LIB, "CTCPClient::Recv failed: SOCKET_NODATA!!");
		return ISOCKET_NODATA;
	}
}

SOCKET CTCPClient::GetFd()
{
	return m_sock;
}

int CTCPClient::CloseSocket()
{
	if (m_sock > 0 && m_sock != INVALID_SOCKET)
	{
		DEBUG_INFO(COMMON_LIB, "Close Socket=%d", m_sock);
		SOCK_CLOSE(m_sock);
	}
	m_sock = INVALID_SOCKET;
	m_userQuit = false;
	return ISOCKET_SUCCESS;
}
