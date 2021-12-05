#include"SSLClient.h"
#include"debugLog.h"

CSSLClient::CSSLClient(void)
{
	mSsl = NULL;
	mCtx = NULL;
}

CSSLClient::~CSSLClient(void)
{
	CloseSocket();
}

int CSSLClient::CreateSocket()
{
	return CTCPClient::CreateSocket();
}

int CSSLClient::Connect(const char* ip, unsigned short port, long long timeout)
{
	int ret = 0;
	long long endTime = GetTick() + timeout;
	do
	{
		SSL_library_init();
		SSLeay_add_ssl_algorithms();
		SSL_load_error_strings();
		if((mCtx = SSL_CTX_new(SSLv23_client_method())) == NULL)
		{
			DEBUG_ERROR(COMMON_LIB, "CSSLClient::Connect SSL_CTX_new failed!!");
			ret = ISOCKET_ERROR;
			break;
		}
		if(!m_caPath.empty() && !m_pemPath.empty())
		{
			//要求校验对方证书
			SSL_CTX_set_verify(mCtx, SSL_VERIFY_PEER, NULL);
			if(!SSL_CTX_load_verify_locations(mCtx, m_caPath.c_str(), NULL))
			{
				DEBUG_ERROR(COMMON_LIB, "CSSLClient::Connect SSL_CTX_load_verify_locations failed!!");
				ret = ISOCKET_ERROR;
				break;
			}
			if(SSL_CTX_use_certificate_file(mCtx, m_pemPath.c_str(), SSL_FILETYPE_PEM) <= 0)
			{
				ERR_print_errors_fp(stderr);
				DEBUG_ERROR(COMMON_LIB, "CSSLClient::Connect SSL_CTX_load_verify_locations failed!!");
				ret = ISOCKET_ERROR;
				break;
			}
		}
		ret = CTCPClient::Connect(ip, port, timeout);
		if (ret != ISOCKET_SUCCESS)
		{
			DEBUG_ERROR(COMMON_LIB, "CSSLClient::Connect failed: tcp connect failed!!");
			ret = ISOCKET_ERROR;
			break;
		}
		mSsl = SSL_new(mCtx);
		if(mSsl == NULL)
		{
			DEBUG_ERROR(COMMON_LIB, "CSSLClient::Connect SSL_new failed!!");
			ret = ISOCKET_ERROR;
			break;
		}
		SSL_set_fd(mSsl, m_sock);
		while(1)
		{
			ret = SSL_connect(mSsl);
			if(ret == 1)//SSL_connect success
			{
				DEBUG_INFO(COMMON_LIB, "CSSLClient::SSL_connect success.");
				break;
			}
			else if(ret == -1)
			{
				ERR_print_errors_fp(stderr);
				int ssl_err = SSL_get_error(mSsl, ret);
				if (SSL_ERROR_WANT_READ == ssl_err || SSL_ERROR_WANT_WRITE == ssl_err)
				{
					if(m_userQuit)
					{
						ret = ISOCKET_USERQUIT;
						break;
					}
					if(GetTick() > endTime)
					{
						ret = ISOCKET_TIMEOUT;
						break;
					}
					usleep(20000);
					continue;
				}
				else
				{
					DEBUG_ERROR(COMMON_LIB, "CSSLClient::SSL_connect error:%d", ssl_err);
					ret = ISOCKET_ERROR;
					break;
				}
			}
			else
			{
				DEBUG_ERROR(COMMON_LIB, "CSSLClient::SSL_connect error:%d", ret);
				ret = ISOCKET_ERROR;
				break;
			}
		}

		return ISOCKET_SUCCESS;
	}while (0);
	CloseSocket();

	return ret;
}

int CSSLClient::Send(void *buf, size_t n, int flags, struct sockaddr_in *addr)
{
	int sendBytes = 0;

	struct timeval	_timeout;
	FD_ZERO(&m_fdw);
	FD_SET(m_sock, &m_fdw);
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 20000;
	SOCK_SELECT(m_sock + 1, NULL, &m_fdw, NULL, &_timeout);
	if (!FD_ISSET(m_sock, &m_fdw)) 
	{
		return ISOCKET_NODATA;
	}
	//这里不知道为什么一次发送太多会出错，只能分多次发送，可能openssl里有缓存
	int sendMaxOnce = 10240;
	n = (n > sendMaxOnce ? sendMaxOnce : n);
	sendBytes = SSL_write(mSsl, buf, n);

	return sendBytes;
}

int CSSLClient::Recv(void *buf, size_t n, int flags, struct sockaddr_in *addr)
{
	FD_ZERO(&m_fdr );
	FD_SET(m_sock, &m_fdr);
	struct timeval	_timeout;
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 20000;
	flags = select(m_sock + 1, &m_fdr, NULL, NULL, &_timeout);
	if(flags > 0)
	{
		if (0 == (flags = SSL_read(mSsl, buf, n)))
		{
			DEBUG_ERROR(COMMON_LIB, "CSSLClient::Recv failed: ISOCKET_SERVERCLOSED");
			return ISOCKET_SERVERCLOSED/*_SOCKET_CLOSE*/;
		}
		else if(flags < 0)
		{
			int ssl_err = SSL_get_error(mSsl, flags);
			if (SSL_ERROR_WANT_READ == ssl_err)
			{
				return ISOCKET_NODATA;
			}
		}

		return flags;
	}
	else if (flags < 0)
	{
		DEBUG_ERROR(COMMON_LIB, "CSSLClient::Recv failed: ISOCKET_ERROR");
		return ISOCKET_ERROR;
	}
	else
	{
		return ISOCKET_NODATA;
	}
}

int CSSLClient::CloseSocket()
{
	CTCPClient::CloseSocket();
	if(mSsl)
	{
		SSL_shutdown(mSsl);
		SSL_free(mSsl);
		mSsl = NULL;
	}
	if(mCtx)
	{
		SSL_CTX_free(mCtx);
		mCtx = NULL;
	}
	return ISOCKET_SUCCESS;
}

void CSSLClient::SetCAPath(const char *path)
{
	if(path != NULL)
	{
		m_caPath = path;
	}
}

void CSSLClient::SetPemPath(const char *path)
{
	if(path != NULL)
	{
    	m_pemPath = path;
	}
}
