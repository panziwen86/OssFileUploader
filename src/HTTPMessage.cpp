#include"HTTPMessage.h"
#include"platform.h"
#include"debugLog.h"
#include<math.h>

bool CHTTPMessage::setFirstLine(std::string firstLine)
{
    std::vector<std::string> resVec = splitWithStl(firstLine, " ");
    if(resVec.size() < 2)
    {
        DEBUG_ERROR(COMMON_LIB, "invalid http first line:%s", firstLine.c_str());
        return false;
    }
    if(checkVersion(resVec[0]) && atoi(resVec[1].c_str()) > 0)
    {
        //is http response
        m_version = resVec[0];
        m_code = resVec[1];
        m_codeDesc = firstLine.substr(m_version.length() + 1 + m_code.length() + 1);
        m_firstLine = firstLine;
    }
    else if(resVec.size() == 3 && checkMethod(resVec[0]) && checkVersion(resVec[2]))
    {
        //is http request 
        m_method = resVec[0];
        m_uri = resVec[1];
        m_version = resVec[2];
        m_firstLine = firstLine;
    }
    else
    {
        DEBUG_ERROR(COMMON_LIB, "invalid http first line:%s", firstLine.c_str());
        return false;
    }

    return true;
}

void CHTTPMessage::setHeader(std::string key, std::string value)
{
    if(key.empty() || value.empty())
    {
        return;
    }
    m_headers[key] = value;
}

void CHTTPMessage::setBody(const char* data, int len, const char* type, bool useChunked)
{
    if(data == NULL || len <= 0)
    {
        return;
    }

    if(useChunked)
    {
        setHeader("Transfer-Encoding", "chunked");
    }
    else
    {
        char lenStr[32] = {0};
        sprintf(lenStr, "%d", len);
        setHeader("Content-Length", lenStr);
    }
    if(type != NULL && strlen(type) > 0)
    {
        setHeader("Content-Type", type);
    }
    m_body.assign(data, len);
}

std::string CHTTPMessage::header(void)const
{
    if(m_firstLine.empty() || m_headers.empty())
    {
        return "";
    }
    std::string header = m_firstLine + "\r\n";
    std::map<std::string, std::string>::const_iterator iter = m_headers.begin();
    for(;iter != m_headers.end();iter++)
    {
        header += iter->first + ": ";
        header += iter->second + "\r\n";
    }
    if(!m_method.empty())
    {
        //是请求才自动添加必要头
        if(m_headers.find("Connection") == m_headers.end())
        {
            header += "Connection: close\r\n";
        }
        if(m_headers.find("Content-Length") == m_headers.end() && m_body.empty())
        {
            header += "Content-Length: 0\r\n";
        }
    }
    header += "\r\n";
    return header;
}

std::string CHTTPMessage::message(void)const
{
    std::string msg = header();
    if(msg.empty())
    {
        return msg;
    }
    if(!m_body.empty())
    {
        msg += m_body;
    }

    return msg;
}

int CHTTPMessage::parse(const char* data, int len)
{
    const char* headerEndPos = strstr(data, "\r\n\r\n");
    if(headerEndPos == NULL)
    {
        return 1;
    }

    if(strstr(data, "Content-Length:") != NULL) 
    {
        int httpContentLen = atoi(strstr(data, "Content-Length:") + strlen("Content-Length:"));
        int httpHeadLen = (strstr(data, "\r\n\r\n") - data + 4);
        if(len - httpHeadLen < httpContentLen)
        {
            return 1;
        }
        if(!parseHeader(data, httpHeadLen-4))
        {
            m_body.clear();
            return 2;
        }
        m_body.assign(data + httpHeadLen, httpContentLen);
    }
    else
    {
        //parse chunked body
        bool getLastChunkData = false;
        const char* parsePos = NULL;
        const char* pChunkedStart = strstr(data, "\r\n\r\n") + 4;
        while(pChunkedStart != data + len && strstr(pChunkedStart, "\r\n") != NULL)
        {
            int chunkDataSize = 0;
            char chunkSizeStr[16] = "";
            int chunkSizeStrLen = 0;
            parsePos = strstr(pChunkedStart, "\r\n");
            chunkSizeStrLen = parsePos - pChunkedStart;
            memcpy(chunkSizeStr, pChunkedStart, chunkSizeStrLen);
            //DEBUG_INFO(DEV_MNG,"chunkSizeStr:[%s]",chunkSizeStr);
            for(int i = chunkSizeStrLen-1; i >= 0; i--)
            {
                if(chunkSizeStr[i]>='0' && chunkSizeStr[i]<='9') {
                    chunkDataSize += (chunkSizeStr[i]-48) * pow(16,chunkSizeStrLen-i-1);
                }
                else if(chunkSizeStr[i]>='A' && chunkSizeStr[i]<='F') {
                    chunkDataSize += (chunkSizeStr[i]-55) * pow(16,chunkSizeStrLen-i-1);
                }
                else if(chunkSizeStr[i]>='a' && chunkSizeStr[i]<='f') {
                    chunkDataSize += (chunkSizeStr[i]-87) * pow(16,chunkSizeStrLen-i-1);
                }
            }
            //DEBUG_INFO(RTMP_PUB, "getWingtoUploadInfo http response body chunksize:%d\n",chunkDataSize);
            if(chunkDataSize == 0)
            {
                //最后一个chunksize必须为0
                getLastChunkData = true;
                break;
            }
            const char* pExpectChunkEndPos = pChunkedStart + chunkSizeStrLen + strlen("\r\n") + chunkDataSize + strlen("\r\n");
            if(pExpectChunkEndPos > data + len)
            {
                //当前chunk数据没接收完
                break;
            }
            //printf("-------------:%d\n",chunkDataSize);
            parsePos += strlen("\r\n");
            std::string chunkData(parsePos, chunkDataSize);
            m_body += chunkData;
            parsePos += chunkDataSize + strlen("\r\n");
            pChunkedStart = parsePos;
        }
        if(!getLastChunkData)
        {
            m_body.clear();
            return 1;
        }
        int httpHeadLen = strstr(data, "\r\n\r\n") - data;
        if(!parseHeader(data, httpHeadLen))
        {
            m_body.clear();
            return 2;
        }
    }

    return 0;
}

bool CHTTPMessage::checkVersion(const std::string version)
{
    if(version == "HTTP/0.9" || version == "HTTP/1.0"
        || version == "HTTP/1.1" || version == "HTTP/2.0")
    {
        return true;
    }

    return false;
}

bool CHTTPMessage::checkMethod(const std::string method)
{
    if(method == "GET" || method == "POST"
        || method == "PUT" || method == "DELETE"
        || method == "HEAD" || method == "PATCH"
        || method == "OPTIONS")
    {
        return true;
    }

    return false;    
}

bool CHTTPMessage::parseHeader(const char* data, int len)
{
    const char* pos = strstr(data, "\r\n");
    if(pos == NULL || pos - data <= 0 || pos - data > len - 2)
    {
        return false;
    }
    if(!setFirstLine(std::string(data, pos - data)))
    {
        return false;
    }
    pos += 2;
    std::vector<std::string> headers = splitWithStl(std::string(pos, len-(pos-data)), "\r\n");
    for(int i=0;i<headers.size();i++)
    {
        pos = strstr(headers[i].c_str(), ":");
        if(pos != NULL)
        {
            const char* endPos = headers[i].c_str() + headers[i].length();
            std::string key(headers[i].c_str(),pos);
            pos++;
            while(pos < endPos)
            {
                if(*pos != ' ')
                {
                    break;
                }
                pos++;
            }
            std::string value(pos, endPos);
            if(!key.empty() && !value.empty())
            {
                m_headers[key] = value;
            }
        }
    }

    return true;
}