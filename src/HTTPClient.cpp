#include"HTTPClient.h"
#include"TCPClient.h"
#include"SSLClient.h"
#include"debugLog.h"
#include"platform.h"

CHTTPClient::CHTTPClient(const char* domain, int port, bool bHttps) : m_recvBufSize(64*1024)
{
    if(domain != NULL && strlen(domain) > 0)
    {
        m_ip = dnsResolve(domain);
    }
    m_port = port;
    if(!bHttps)
    {
        m_client = new CTCPClient;
    }
    else
    {
        m_client = new CSSLClient;
    }
    m_bUserQuit = false;
    m_recvBuf = new char[m_recvBufSize];
}

CHTTPClient::~CHTTPClient()
{
    if(m_client != NULL)
    {
        delete m_client;
    }
    if(m_recvBuf != NULL)
    {
        delete []m_recvBuf;
    }
}

bool CHTTPClient::setServerAddr(const char* domain, int port)
{
    if(domain == NULL || strlen(domain) <= 0)
    {
        return false;
    }
    std::string ip = dnsResolve(domain);
    if(ip.empty())
    {
        return false;
    }
    m_ip = ip;
    m_port = port;
    m_client->CloseSocket();
    m_bUserQuit = false;

    return true;
}

void CHTTPClient::userQuit(void)
{
    m_bUserQuit = true;
    if(m_client != NULL)
    {
        m_client->UserQuit();
    }
}

int CHTTPClient::request(const char* req, int reqSize, CHTTPMessage* pRes, int timeouts)
{
    int ret = 0;
    if(req == NULL || reqSize <= 0)
    {
        return HTTP_INVALID_PARAM;
    }
    long long timeOutMs = GetTick() + timeouts * 1000;
    int sendDataLen = 0;
    const char* startPos = req;
    if(m_ip.empty())
    {
        DEBUG_ERROR(COMMON_LIB, "http client request error:invalid server addr");
        return HTTP_CONNECT_ERROR;
    }
    if(m_client->GetFd() == INVALID_SOCKET)
    {
        m_client->CreateSocket();
        ret = m_client->Connect(m_ip.c_str(), m_port);
        if(ret != ISocket::ISOCKET_SUCCESS)
        {
            DEBUG_ERROR(COMMON_LIB, "http client connect error:%d", ret);
            return HTTP_CONNECT_ERROR;
        }
    }
    int len = m_client->Send((void*)startPos + sendDataLen, reqSize - sendDataLen);
    if(len <= 0)
    {
        m_client->CloseSocket();
        m_client->CreateSocket();
        ret = m_client->Connect(m_ip.c_str(), m_port);
        if(ret != ISocket::ISOCKET_SUCCESS)
        {
            DEBUG_ERROR(COMMON_LIB, "http client connect error:%d", ret);
            return HTTP_CONNECT_ERROR;
        }
    }
    else
    {
        sendDataLen += len;
    }

    while(sendDataLen < reqSize && !m_bUserQuit)
    {
        len = m_client->Send((void*)startPos + sendDataLen, reqSize - sendDataLen);
        if(len <= 0)
        {
            if(len != ISocket::ISOCKET_NODATA)
            {
                DEBUG_ERROR(COMMON_LIB, "http client send error:%d", len);
                return HTTP_SOCKET_ERROR;
            }
            usleep(10000);
        }
        else
        {
            sendDataLen += len;
        }
        if(sendDataLen < reqSize)
        {
            if(GetTick() > timeOutMs)
            {
                m_client->CloseSocket();
                return HTTP_TIMEOUT;
            }
            usleep(2000);
        }
    }
    if(m_bUserQuit)
    {
        m_client->CloseSocket();
        return HTTP_USER_QUIT;
    }

    if(pRes == NULL)
    {
        //不需要接收应答，直接返回
        return HTTP_SUCCESS;
    }

    int recvDataLen = 0;
    while(recvDataLen < m_recvBufSize && !m_bUserQuit)
    {
        len = m_client->Recv((void*)m_recvBuf + recvDataLen, m_recvBufSize - recvDataLen);
        if(len <= 0)
        {
            if(len != ISocket::ISOCKET_NODATA)
            {
                m_client->CloseSocket();
                DEBUG_ERROR(COMMON_LIB, "http client recv error:%d", len);
                return HTTP_SOCKET_ERROR;
            }
            usleep(10000);
        }
        else
        {
            recvDataLen += len;
            ret = pRes->parse(m_recvBuf, recvDataLen);
            if(ret == 0)
            {
                m_client->CloseSocket();
                break;
            }
            else if(ret == 2)
            {
                DEBUG_ERROR(COMMON_LIB, "http client recv invalid http response");
                m_client->CloseSocket();
                return HTTP_INVALID_RESPONSE;
            }
        }
        if(recvDataLen < m_recvBufSize && GetTick() > timeOutMs)
        {
            m_client->CloseSocket();
            return HTTP_TIMEOUT;
        }
    }

    if(m_bUserQuit)
    {
        return HTTP_USER_QUIT;
    }

    return HTTP_SUCCESS;
}

int CHTTPClient::request(const CHTTPMessage* req, CHTTPMessage* pRes, int timeouts)
{
    if(req == NULL)
    {
        return HTTP_INVALID_PARAM;
    }
    return request(req->message().c_str(), req->message().length(), pRes, timeouts);
}