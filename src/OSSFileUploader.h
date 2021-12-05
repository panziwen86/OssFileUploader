#ifndef _OSSFILEUPLOADER_H_
#define _OSSFILEUPLOADER_H_

#include"HTTPClient.h"

struct OSSConfig
{
    std::string endpoint;
    std::string bucket;
    std::string keyId;
    std::string keySecret;
    std::string securityToken;
};

class COSSFileUploader
{
public:
    enum
    {
        UPLOADER_SUCCESS,
        UPLOADER_USERQUIT,
        UPLOADER_NETERROR,
        UPLOADER_OSSERROR,
        UPLOADER_UNKNOWN
    };
    COSSFileUploader(bool bHttps = false);
    ~COSSFileUploader();
    void setConfig(const OSSConfig& config);
    int uploadFile(const char* uploadObject, const char* fileData, int dataSize, const char* fileType = "image/jpeg");
    void userQuit(void);
private:
    std::string createOssAuthorization(std::string keyId, std::string keySecret, std::string method,
        std::string content_type, std::string date, std::string ossHeaders, std::string resource);
    std::string getGMTTimeStr(void);

private:
    CHTTPClient* m_httpClient;
    OSSConfig m_config;
    CWTOLock m_configLock;
    bool m_bHttps;
};

#endif