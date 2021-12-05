#include<stdio.h>
#include"HTTPClient.h"
#include"OSSFileUploader.h"
#include <iostream>

#define TEST_HTTP_CLIENT
//#define TEST_OSS_UPLOAD

int main(int argc, char** argv)
{
    int ret = 0;
#ifdef TEST_HTTP_CLIENT
    CHTTPMessage request, response;

    // CHTTPClient client("172.16.1.36", 20203);
    // request.setFirstLine("POST /user/user/customer/user/4/provision/1 HTTP/1.1");
    // request.setHeader("Host", "172.16.1.36:20203");

    CHTTPClient client("www.baidu.com", 443, true);
    request.setFirstLine("GET / HTTP/1.1");
    request.setHeader("Host", "www.baidu.com:443");

    // CHTTPClient client("rts-atlas-dev.wingto.com", 443, true);
    // request.setFirstLine("GET /hls/755d38e6d163e820edda070141c443aa49480051/2021/11/19/091340.m3u8?bucket=wingto-test1&time=1637284420-1637284471&expires=1637292217&sign=0432cac58be50f905ddbbaf1e1fc6042 HTTP/1.1");
    // request.setHeader("Host", "rts-atlas-dev.wingto.com:443");

    printf("http request:\n%s", request.message().c_str());
    ret = client.request(&request, &response);
    if(ret != 0)
    {
        printf("http client request error:%d\n", ret);
        return 1;
    }
    printf("http response:\n%s\n", response.message().c_str());
#endif

#ifdef TEST_OSS_UPLOAD
    OSSConfig config = {
        "endpoint",
        "bucket",
        "keyid",
        "keysecret",
        ""
    };
    COSSFileUploader uploader(true);
    uploader.setConfig(config);
    //测试发送假图片数据
    int picSize = 1023;
    char* picData = new char[picSize];
    ret = uploader.uploadFile("panziwen/test.jpg", picData, picSize);
    delete []picData;
    if(ret != 0)
    {
        printf("COSSFileUploader uploadFile error:%d\n", ret);
        return 1;
    }
#endif

    return 0;
}
