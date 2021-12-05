#ifndef _SSLCLIENT_H_
#define _SSLCLIENT_H_

#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"

#include"TCPClient.h"

class CSSLClient : public CTCPClient
{
public:
	CSSLClient(void);
	~CSSLClient(void);
	
public:
	virtual int CreateSocket();
	virtual int Connect(const char* ip, unsigned short port, long long timeout = 3000);
	virtual int Send(void* buf, size_t n, int flags, struct sockaddr_in* addr = NULL);
	virtual int Recv(void* buf, size_t n, int flags, struct sockaddr_in* addr = NULL);
	virtual int CloseSocket();
    
    void SetCAPath(const char* path);
    void SetPemPath(const char* path);
    
protected:
	SSL_CTX* mCtx;
	SSL* mSsl;
    std::string m_caPath;
    std::string m_pemPath;
};

#endif  
