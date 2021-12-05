#ifndef _CBASE64CODEC_H_
#define _CBASE64CODEC_H_

#include <string>

class CBase64Codec
{
public:
    CBase64Codec();
    ~CBase64Codec();

public:
    std::string Encode(const unsigned char *pSrc, int len);
    std::string Decode(const unsigned char *pSrc, int len, int &outLen);
    std::string EncodeJpg(const unsigned char *pSrc, int len);
};

#endif
