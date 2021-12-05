#include"OSSFileUploader.h"
#include"debugLog.h"
#include<openssl/hmac.h>
#include"CBase64Codec.h"

COSSFileUploader::COSSFileUploader(bool bHttps) : m_bHttps(bHttps)
{
    if(!bHttps)
    {
        m_httpClient = new CHTTPClient();
    }
    else
    {
        m_httpClient = new CHTTPClient(NULL, 443, true);
    }
}

COSSFileUploader::~COSSFileUploader()
{
    if(m_httpClient != NULL)
    {
        delete m_httpClient;
    }
}

void COSSFileUploader::userQuit(void)
{
    m_configLock.Lock();
    m_config.endpoint.clear();
    m_configLock.UnLock();
    usleep(2000);
    m_httpClient->userQuit();
}

void COSSFileUploader::setConfig(const OSSConfig& config)
{
    CWTOAutoLocker lock(m_configLock);
    if(m_config.endpoint != config.endpoint)
    {
        int port = m_bHttps ? 443 : 80;
        m_httpClient->setServerAddr(config.endpoint.c_str(), port);
    }
    m_config = config;
}

int COSSFileUploader::uploadFile(const char* uploadObject, const char* fileData, int dataSize, const char* fileType)
{
    m_configLock.Lock();
    if(m_config.endpoint.empty())
    {
        return UPLOADER_USERQUIT;
    }
    std::string method = "PUT";
    std::string uri = "/" + std::string(uploadObject);
    std::string resource = "/" + m_config.bucket + uri;
    std::string Host = m_config.bucket + "." + m_config.endpoint;
    CHTTPMessage request;
    request.setFirstLine(method + " " + uri + " HTTP/1.1");
    request.setHeader("Host", Host);
    std::string ossHeaders;
    if(!m_config.securityToken.empty())
    {
        ossHeaders = "x-oss-security-token:" + m_config.securityToken + "\n";
        request.setHeader("x-oss-security-token", m_config.securityToken);
    }
    std::string Date = getGMTTimeStr();
    std::string Authorization = createOssAuthorization(m_config.keyId, m_config.keySecret, method, fileType, Date, ossHeaders, resource);
    request.setHeader("Date", Date);
    request.setHeader("Authorization", Authorization);
    request.setBody(fileData, dataSize, fileType);
    m_configLock.UnLock();
    CHTTPMessage response;
    int ret = m_httpClient->request(&request, &response);
    if(ret == 0)
    {
        if(response.m_code != "200")
        {
            DEBUG_ERROR(COMMON_LIB, "uploadFile OSS HTTP request:\n%s", request.header().c_str());
            DEBUG_ERROR(COMMON_LIB, "response:\n%s", response.message().c_str());
            return atoi(response.m_code.c_str());
        }
    }
    else
    {
        DEBUG_ERROR(COMMON_LIB, "uploadFile HTTP Client Recv error:%d", ret);
        if(ret == CHTTPClient::HTTP_USER_QUIT)
        {
            return UPLOADER_USERQUIT;
        }
        else if(ret == CHTTPClient::HTTP_TIMEOUT || ret == CHTTPClient::HTTP_SOCKET_ERROR || ret == CHTTPClient::HTTP_CONNECT_ERROR)
        {
            return UPLOADER_NETERROR;
        }
        else
        {
            return UPLOADER_UNKNOWN;
        }
    }
    DEBUG_INFO(COMMON_LIB, "uploadFile success ossObject:%s", uploadObject);
    return UPLOADER_SUCCESS;
}

std::string COSSFileUploader::createOssAuthorization(std::string keyId, std::string keySecret, std::string method,
    std::string content_type, std::string date, std::string ossHeaders, std::string resource)
{
    if(keyId.empty() || keySecret.empty() || method.empty() || date.empty() || resource.empty())
    {
        return "";
    }
    std::string sha1Data = method + "\n\n";
    if(!content_type.empty())
    {
        sha1Data += content_type;
    }
    sha1Data += "\n";
    sha1Data += date + "\n";
    if(!ossHeaders.empty())
    {
        sha1Data += ossHeaders;
    }
    sha1Data += resource;
    unsigned char digest[EVP_MAX_MD_SIZE] = {0};
    unsigned int digest_len = 0;
    HMAC(EVP_sha1(), keySecret.c_str(), keySecret.length(), (const unsigned char*)sha1Data.c_str(), sha1Data.length(), digest, &digest_len);
    CBase64Codec base64;
    std::string base64SignString = base64.Encode(digest, digest_len);
    return "OSS " + keyId + ":" + base64SignString;
}

std::string COSSFileUploader::getGMTTimeStr(void)
{
    time_t now = time(NULL);
    struct tm* tm_time = gmtime(&now);
    char timeStr[64] = {0};
    strftime(timeStr, sizeof(timeStr), "%a, %d %b %Y %X GMT", tm_time);
    return timeStr;
}