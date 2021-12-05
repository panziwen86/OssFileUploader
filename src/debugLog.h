#ifndef DEBUGLOG_H_
#define DEBUGLOG_H_
#include <errno.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

enum DebugLevel
{
	DEBUG_LEVEL_NONE,
    DEBUG_LEVEL_ERROR,
    DEBUG_LEVEL_WARNING,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_MAX,
};

enum MODULE
{
    API,
    COMMON_LIB,
    RTMP_SRS_LIB,
    RTMP_CLIENT,
    UNKNOWN,
};

#include <libgen.h>
void __wtoDebug(int module, int debugLevel, const char *file, const int line, const char *format, ...) \
           __attribute__ ((__format__(printf, 5, 6)));

#define DEBUG_ERROR(module, format...) __wtoDebug(module, DEBUG_LEVEL_ERROR, __FILE__, __LINE__, format)
#define DEBUG_WARNING(module, format...) __wtoDebug(module, DEBUG_LEVEL_WARNING, __FILE__, __LINE__, format)
#define DEBUG_INFO(module, format...) __wtoDebug(module, DEBUG_LEVEL_INFO, __FILE__, __LINE__, format)

const char *ms2str(char *pBuffer, int64_t ms);
const char *sec2str(char *pBuffer, int32_t sec);

#endif /* DEBUGLOG_H_ */

