#ifndef _HTTPMESSAGE_H_
#define _HTTPMESSAGE_H_

#include<string>
#include<map>

class CHTTPMessage
{
public:
    bool setFirstLine(std::string firstLine);
    void setHeader(std::string key, std::string value);
    void setBody(const char* data, int len, const char* type = "application/json", bool useChunked = false);
    std::string message(void)const;
    std::string header(void)const;
    int parse(const char* data, int len);
private:
    bool checkVersion(const std::string version);
    bool checkMethod(const std::string method);
    bool parseHeader(const char* data, int len);
public:
    std::string m_firstLine;
    std::string m_method;
    std::string m_uri;
    std::string m_version;
    std::string m_code;
    std::string m_codeDesc;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
};

#endif