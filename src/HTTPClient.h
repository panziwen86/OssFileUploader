#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include"ISocket.h"
#include"HTTPMessage.h"

class CHTTPClient
{
public:
    enum
    {
        HTTP_SUCCESS = 0,
        HTTP_INVALID_PARAM,
        HTTP_CONNECT_ERROR,
        HTTP_SOCKET_ERROR,
        HTTP_TIMEOUT,
        HTTP_INVALID_RESPONSE,
        HTTP_USER_QUIT
    };
    CHTTPClient(const char* domain = NULL, int port = 80, bool bHttps = false);
    ~CHTTPClient();
    bool setServerAddr(const char* domain, int port = 80);
    int request(const char* req, int reqLen, CHTTPMessage* pRes, int timeouts = 20);
    int request(const CHTTPMessage* req, CHTTPMessage* pRes, int timeouts = 20);
    void userQuit(void);
private:
    ISocket* m_client;
    std::string m_ip;
    int m_port;
    bool m_bUserQuit;
    char* m_recvBuf;
    const int m_recvBufSize;
};

#endif