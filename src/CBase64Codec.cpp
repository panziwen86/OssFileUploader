
#include "CBase64Codec.h"

CBase64Codec::CBase64Codec()
{

}

CBase64Codec::~CBase64Codec()
{

}

std::string CBase64Codec::EncodeJpg(const unsigned char *pSrc, int len)
{
    //编码表
    const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    //返回值
    std::string strEncode;
    unsigned char Tmp[4] = {0};
    int LineLength = 0;
    for(int i = 0; i < (int)(len / 3); i++)
    {
        Tmp[1] = *pSrc++;
        Tmp[2] = *pSrc++;
        Tmp[3] = *pSrc++;
        strEncode += EncodeTable[Tmp[1] >> 2];
        strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        strEncode += EncodeTable[Tmp[3] & 0x3F];
        if(LineLength += 4, LineLength == 76)
        {
            strEncode += "\r\n";	/* 由于发邮件时，subject也采用了base64编码，而换行符将导致混乱，故不加换行符，实验证明不影响图片显示 */
            LineLength = 0;
        }
    }
    //对剩余数据进行编码
    int Mod = len % 3;
    if(Mod == 1)
    {
        Tmp[1] = *pSrc++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
        strEncode += "==";
    }
    else if(Mod == 2)
    {
        Tmp[1] = *pSrc++;
        Tmp[2] = *pSrc++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
        strEncode += "=";
    }

    return strEncode;
}


std::string CBase64Codec::Encode(const unsigned char *pSrc, int len)
{
    //编码表
    const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    //返回值
    std::string strEncode;
    unsigned char Tmp[4] = {0};
    int LineLength = 0;
    for(int i = 0; i < (int)(len / 3); i++)
    {
        Tmp[1] = *pSrc++;
        Tmp[2] = *pSrc++;
        Tmp[3] = *pSrc++;
        strEncode += EncodeTable[Tmp[1] >> 2];
        strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        strEncode += EncodeTable[Tmp[3] & 0x3F];
        if(LineLength += 4, LineLength == 76)
        {
            //strEncode += "\r\n";	/* 由于发邮件时，subject也采用了base64编码，而换行符将导致混乱，故不加换行符，实验证明不影响图片显示 */
            LineLength = 0;
        }
    }
    //对剩余数据进行编码
    int Mod = len % 3;
    if(Mod == 1)
    {
        Tmp[1] = *pSrc++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
        strEncode += "==";
    }
    else if(Mod == 2)
    {
        Tmp[1] = *pSrc++;
        Tmp[2] = *pSrc++;
        strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
        strEncode += "=";
    }

    return strEncode;
}

std::string CBase64Codec::Decode(const unsigned char *pSrc, int len, int &outLen)
{
    //解码表
    const char DecodeTable[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        62, // '+'
        0, 0, 0,
        63, // '/'
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
        0, 0, 0, 0, 0, 0, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
        0, 0, 0, 0, 0, 0,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
    };
    //返回值
    std::string strDecode;
    int nValue;
    int i = 0;
    while (i < len)
    {
        if (*pSrc != '\r' && *pSrc != '\n')
        {
            nValue = DecodeTable[*pSrc++] << 18;
            nValue += DecodeTable[*pSrc++] << 12;
            strDecode += (nValue & 0x00FF0000) >> 16;
            outLen++;
            if (*pSrc != '=')
            {
                nValue += DecodeTable[*pSrc++] << 6;
                strDecode += (nValue & 0x0000FF00) >> 8;
                outLen++;
                if (*pSrc != '=')
                {
                    nValue += DecodeTable[*pSrc++];
                    strDecode += nValue & 0x000000FF;
                    outLen++;
                }
            }
            i += 4;
        }
        else// 回车换行,跳过
        {
            pSrc++;
            i++;
        }
    }

    return strDecode;
}

// EOF
