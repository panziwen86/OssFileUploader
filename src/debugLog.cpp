#include"debugLog.h"
#include<sys/syscall.h>
#include<stdlib.h>
#include<time.h>
#include<sys/sysinfo.h>

#define DEBUG_LEVEL	DEBUG_LEVEL_MAX

int gDebugLevel = DEBUG_LEVEL_MAX;

typedef struct tagModuleNameInfo
{
    int         module;
    const char *moduleName;
}ModuleNameInfo;
ModuleNameInfo moduleNameTbl[] =
{
    {API,				"API"},
    {COMMON_LIB,		"COMMON_LIB"},
    {RTMP_SRS_LIB,		"RTMP_SRS_LIB"},
    {RTMP_CLIENT,		"RTMP_CLIENT"},
    {UNKNOWN,			"UNKNOWN"},
};

#define T_COLOR_NRM  "\x1B[0m"
#define T_COLOR_RED  "\x1B[31m"
#define T_COLOR_GRN  "\x1B[32m"
#define T_COLOR_YEL  "\x1B[33m"
#define T_COLOR_BLU  "\x1B[34m"
#define T_COLOR_MAG  "\x1B[35m"
#define T_COLOR_CYN  "\x1B[36m"
#define T_COLOR_WHT  "\x1B[37m"
const char *gTexColor[DEBUG_LEVEL_MAX] = {T_COLOR_NRM, T_COLOR_RED, T_COLOR_YEL, T_COLOR_GRN};

void __wtoDebug(int module, int debugLevel, const char *file, const int line, const char *format, ...)
{
#define LINE_BUFFER_SIZE        (1024)
    va_list args;
    char pLineBuf[LINE_BUFFER_SIZE];
    uint32_t bufIdx = 0;
    struct tm date;
    struct timeval tv;
    char logTimsStr[32] = {0};

    unsigned int modID = (module >= UNKNOWN) ? UNKNOWN : module;
    const char *tColor = T_COLOR_RED;
    const char *basename = strrchr(file, '/');
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &date);
    sprintf(logTimsStr, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
            date.tm_year+1900, date.tm_mon+1, date.tm_mday, 
            date.tm_hour, date.tm_min, date.tm_sec, tv.tv_usec/1000);
    bufIdx += snprintf(pLineBuf + bufIdx, LINE_BUFFER_SIZE, "%s <%06ld>[%s %s:%d]",
            logTimsStr,
            syscall(SYS_gettid), moduleNameTbl[modID].moduleName, 
            (NULL == basename)?file:basename + 1, line);

    va_start(args, format);
    bufIdx += vsnprintf(pLineBuf + bufIdx, LINE_BUFFER_SIZE - bufIdx, format, args);
    va_end(args);

    /*Make sure we are NULL terminated*/
    pLineBuf[LINE_BUFFER_SIZE - 1] = 0;
    if (debugLevel > gDebugLevel)
		return;
	
	if (debugLevel < DEBUG_LEVEL_MAX)
    {
        tColor = gTexColor[debugLevel];
    }
    printf("%s%s%s\n", tColor, pLineBuf, T_COLOR_NRM);
}

