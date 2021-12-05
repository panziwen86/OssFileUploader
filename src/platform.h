#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef _WIN32
	#include <windows.h>
	#include <WinSock2.h>
	#include <stdio.h>
	#pragma comment(lib, "ws2_32.lib")
	#include <thread>
	#include <mutex>
#else
	#include <pthread.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <errno.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/syscall.h>

	#ifdef ANDROID
		#include <fcntl.h>
	#else
		#include <sys/fcntl.h>
	#endif

	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <ctype.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <sys/syscall.h>
    #include<netdb.h>
#endif

#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

#ifndef _WIN32
    #define INFINITE 0xFFFFFFFF  // Infinite timeout
#endif

#ifdef _WIN32
    #define SLEEP(t) Sleep(t)

    // socket
    typedef	int SOCKLEN;

    #define SOCK_INIT WSADATA data;\
        WSAStartup(MAKEWORD(2, 2), &data);
    #define SOCK_DEINIT WSACleanup();
    #define SOCK_IOCTL(x, y, z) ::ioctlsocket(x, y, z)
    #define SOCK_CLOSE(x) ::closesocket(x)
    #define SOCK_CREATE(af, type, protocol) ::socket(af, type, protocol)
    #define SOCK_LISTEN(s, backlog) ::listen(s, backlog)
    #define SOCK_BIND(s, addr, namelen) ::bind(s, addr, namelen)
    #define SOCK_ACCEPT(s, addr, addrlen) ::accept(s, addr, addrlen)
    #define SOCK_CONNECT(s, name, namelen) ::connect(s, name, namelen)
    #define SOCK_SEND(s, buf, len, flags) ::send(s, buf, len, flags)
    #define SOCK_RECV(s, buf, len, flags) ::recv(s, buf, len, flags)
    #define SOCK_SENDTO(s, buf, len, flags, to, tolen) ::sendto(s, buf, len, flags, to, tolen)
    #define SOCK_RECVFROM(s, buf, len, flags, from, fromlen) ::recvfrom(s, buf, len, flags, from, fromlen)
    #define SOCK_SELECT(nfds, readfds, writefds, exceptfds, timeout) ::select(nfds, readfds, writefds, exceptfds, timeout)
    #define SOCK_SETOPT(s, level, optname, optval, optlen) ::setsockopt(s, level, optname, optval, optlen)
    #define SOCK_GETOPT(s, level, optname, optval, optlen) ::getsockopt(s, level, optname, optval, optlen)
    #define HTONL(hostlong) ::htonl(hostlong)
    #define NTOHL(hostlong) ::ntohl(netlong)
    #define HTONS(hostshort) ::htons(hostshort)
    #define NTOHS(hostshort) ::ntohs(netshort)
    #define INET_ADDR(cp) inet_addr(cp)
#else	/* linux or mac */
    #define SLEEP(t) usleep(t*1000)

    typedef	int SOCKET;
    typedef	socklen_t SOCKLEN;

    #define SOCK_INIT
    #define SOCK_DEINIT
    #define SOCK_IOCTL(x, y, z) ::fcntl((x), (y), (z))
    #define SOCK_CLOSE(x) ::close((x))
    #define SOCK_CREATE(af, type, protocol) ::socket(af, type, protocol)
    #define SOCK_LISTEN(s, backlog) ::listen(s, backlog)
    #define SOCK_BIND(s, addr, namelen) ::bind(s, addr, namelen)
    #define SOCK_ACCEPT(s, addr, addrlen) ::accept(s, addr, addrlen)
    #define SOCK_CONNECT(s, name, namelen) ::connect(s, name, namelen)
    #define SOCK_SEND(s, buf, len, flags) ::send(s, buf, len, flags)
    #define SOCK_RECV(s, buf, len, flags) ::recv(s, buf, len, flags)
    #define SOCK_SENDTO(s, buf, len, flags, to, tolen) ::sendto(s, buf, len, flags, to, tolen)
    #define SOCK_RECVFROM(s, buf, len, flags, from, fromlen) ::recvfrom(s, buf, len, flags, from, fromlen)
    #define SOCK_SELECT(nfds, readfds, writefds, exceptfds, timeout) ::select(nfds, readfds, writefds, exceptfds, timeout)
    #define SOCK_SETOPT(s, level, optname, optval, optlen) ::setsockopt(s, level, optname, optval, optlen)
    #define SOCK_GETOPT(s, level, optname, optval, optlen) ::getsockopt(s, level, optname, optval, optlen)
    #define HTONL(hostlong) ::htonl(hostlong)
    #define NTOHL(hostlong) ::ntohl(netlong)
    #define HTONS(hostshort) ::htons(hostshort)
    #define NTOHS(hostshort) ::ntohs(netshort)
    #define INET_ADDR(cp) inet_addr(cp)
#endif

inline std::vector<std::string> splitWithStl(const std::string &str, const std::string &pattern)
{
    std::vector<std::string> resVec;

	if ("" == str)
    {
        return resVec;
    }
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);
    size_t size = strs.size();
    while (pos != std::string::npos)
    {
        std::string x = strs.substr(0,pos);
        resVec.push_back(x);
        strs = strs.substr(pos + pattern.length(),size);
        pos = strs.find(pattern);
    }
    
    return resVec;
}

inline long long GetTick(void)
{
#ifdef _WIN32
	//return GetTickCount64();
	return GetTickCount();
#elif defined MAC_OS
    struct timeval us;
    gettimeofday(&us, NULL);
    return (us.tv_sec*1000 + us.tv_usec/1000);
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((long long)ts.tv_sec * 1000LL + ts.tv_nsec/(1000 * 1000));
#endif
}

inline std::string dnsResolve(std::string host)
{
    int family = AF_UNSPEC;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    
    addrinfo* r = NULL;
    if(getaddrinfo(host.c_str(), NULL, &hints, &r))
    {
        if(r != NULL)
        {
            free(r);
        }
        return "";
    }
    
    char shost[64];
    memset(shost, 0, sizeof(shost));
    if (getnameinfo(r->ai_addr, r->ai_addrlen, shost, sizeof(shost), NULL, 0, NI_NUMERICHOST))
    {
        free(r);
        return "";
    }

    family = r->ai_family;
    free(r);
    return std::string(shost);
}

//线程锁
class CWTOLock
{
public:
	pthread_mutex_t m_mtx;
public:
	CWTOLock()
	{
		//初始化锁的属性  
		pthread_mutexattr_t attr;  
		pthread_mutexattr_init(&attr);  
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);//设置锁的属性为可递归  
		//设置锁的属性  
		pthread_mutex_init(&m_mtx, &attr);
		//销毁  
		pthread_mutexattr_destroy(&attr);  
	}
	~CWTOLock()	{pthread_mutex_destroy(&m_mtx);}
	void Lock()	{pthread_mutex_lock(&m_mtx);}
	bool TryLock(){if(pthread_mutex_trylock(&m_mtx) == 0)return true; return false;}
	void UnLock(){pthread_mutex_unlock( &m_mtx);}
};

//自动锁
class CWTOAutoLocker
{
private:
	CWTOLock & m_lck;
public:
	CWTOAutoLocker( CWTOLock & lck ):m_lck(lck)
	{
		m_lck.Lock();
	}
	~CWTOAutoLocker()
	{
		m_lck.UnLock();
	}
};

class CWTOThreadCondition
{
private:
	pthread_cond_t m_cond;
public:
	CWTOThreadCondition(){pthread_cond_init(&m_cond, NULL);}
	~CWTOThreadCondition()	{ pthread_cond_destroy(&m_cond);}
	void Wait(CWTOLock& lock)
	{
		pthread_cond_wait(&m_cond, &(lock.m_mtx));
	};
	bool TimedWait(CWTOLock& lock, unsigned int timeout_ms)
	{
		struct timeval now;
		struct timespec abstime;
		gettimeofday(&now, NULL);
		now.tv_sec += timeout_ms / 1000;
		unsigned long ms = timeout_ms % 1000;
		if(now.tv_usec + ms * 1000 >= 1000000)
		{
			now.tv_sec += 1;
			now.tv_usec -= (1000000 - (ms * 1000));
		}
		else
		{
			now.tv_usec += ms * 1000;
		}
		abstime.tv_sec = now.tv_sec;
		abstime.tv_nsec = now.tv_usec * 1000; /* nanoseconds */

		if(ETIMEDOUT == pthread_cond_timedwait(&m_cond, &(lock.m_mtx), &abstime))
		{
			return false;
		}
		return true;
	};
    void NotifyOne(){pthread_cond_signal(&m_cond);};
	void NotifyAll(){pthread_cond_broadcast(&m_cond);};
	void NotifyOne(CWTOLock& lock){ lock.Lock();pthread_cond_signal(&m_cond);lock.UnLock();};
	void NotifyAll(CWTOLock& lock){ lock.Lock();pthread_cond_broadcast(&m_cond);lock.UnLock();};
};

#endif
