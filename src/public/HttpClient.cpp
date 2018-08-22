#include "HttpClient.h"

#include <string.h>
#include "Log.h"
#include "SystemInfo.h"


CHttpClient::CHttpClient() :
    m_bDebug(false), 
    m_nTimeout(10)
{
}

CHttpClient::~CHttpClient()
{
}

void CHttpClient::SetDebug(bool bDebug)
{
    m_bDebug = bDebug;
}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
    if(itype == CURLINFO_TEXT)
    {
        Log_Debug("[TEXT]%s\n", pData);
    }
    else if(itype == CURLINFO_HEADER_IN)
    {
        Log_Debug("[HEADER_IN]%s\n", pData);
    }
    else if(itype == CURLINFO_HEADER_OUT)
    {
        Log_Debug("[HEADER_OUT]%s\n", pData);
    }
    else if(itype == CURLINFO_DATA_IN)
    {
        Log_Debug("[DATA_IN]%s\n", pData);
    }
    else if(itype == CURLINFO_DATA_OUT)
    {
        Log_Debug("[DATA_OUT]%s\n", pData);
    }
    return 0;
}                

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    Log_Info("datasize %d", nmemb);
    return nmemb;
}

int CHttpClient::Get(const std::string & strUrl, std::string & strResponse)
{
    long lStart = CSystemInfo::GetInstance()->GetTimestamp(1);

    if(strncmp(strUrl.c_str(),"https://",strlen("https://")) == 0) 
    {
        Log_Error("not support https");
        return -1;
    } 

    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        Log_Error("curl_easy_init error");
        return CURLE_FAILED_INIT;
    }
    if(m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    /**
     * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
     * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
     */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_nTimeout);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    long lEnd = CSystemInfo::GetInstance()->GetTimestamp(1);
    Log_Info("timeout:%lds request time:%ldms res:%d", m_nTimeout, lEnd-lStart, res);
    return res;
}

int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
    long lStart = CSystemInfo::GetInstance()->GetTimestamp(1);

    if(strncmp(strUrl.c_str(),"https://",strlen("https://")) == 0) 
    {
        Log_Error("not support https");
        return -1;
    } 

    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        Log_Error("curl_easy_init error");
        return CURLE_FAILED_INIT;
    }
    if(m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    
    // POST argumnet
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strPost.size());
    // end POST argument

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_nTimeout);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    long lEnd = CSystemInfo::GetInstance()->GetTimestamp(1);
    Log_Info("timeout:%lds request time:%ldms res:%d", m_nTimeout, lEnd-lStart, res);
    return res;
}

int CHttpClient::Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath)
{
    long lStart = CSystemInfo::GetInstance()->GetTimestamp(1);

    if(strncmp(strUrl.c_str(),"https://",strlen("https://")) != 0) 
    {
        Log_Error("only support https");
        return -1;
    }

    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        Log_Error("curl_easy_init error");
        return CURLE_FAILED_INIT;
    }
    if(m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    /* CURLOPT_SSL_VERIFYPEER
     * 设为0表示不检查证书
     * 设为1表示检查证书中是否有CN(common name)字段
     * 设为2表示在1的基础上校验当前的域名是否与CN匹配
     * (libcurl_7.28.1之后的版本)，这个调试选项由于经常被开发者用错，被去掉了，因此目前也不支持1了，只有0/2两种取值
     */
    if(NULL == pCaPath)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        // 启用时会通过HTTP代理来传输
        //curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1);
        // 启用时会将服务器服务器返回的"Location: "放在header中递归的返回给服务器，使用CURLOPT_MAXREDIRS可以限定递归返回的数量。
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        // CURLOPT_SSL_VERIFYHOST 默认为2
        //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_nTimeout);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    long lEnd = CSystemInfo::GetInstance()->GetTimestamp(1);
    Log_Info("timeout:%lds request time:%ldms res:%d", m_nTimeout, lEnd-lStart, res);
    return res;
}

int CHttpClient::Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath)
{
    long lStart = CSystemInfo::GetInstance()->GetTimestamp(1);

    if(strncmp(strUrl.c_str(),"https://",strlen("https://")) != 0) 
    {
        Log_Error("only support https");
        return -1;
    }

    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        Log_Error("curl_easy_init error");
        return CURLE_FAILED_INIT;
    }
    if(m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);

    // POST argumnet
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strPost.size());
    // end POST argument

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    /* CURLOPT_SSL_VERIFYPEER
     * 设为0表示不检查证书
     * 设为1表示检查证书中是否有CN(common name)字段
     * 设为2表示在1的基础上校验当前的域名是否与CN匹配
     * (libcurl_7.28.1之后的版本)，这个调试选项由于经常被开发者用错，被去掉了，因此目前也不支持1了，只有0/2两种取值
     */
    if(NULL == pCaPath)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        // 启用时会通过HTTP代理来传输
        //curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1);
        // 启用时会将服务器服务器返回的"Location: "放在header中递归的返回给服务器，使用CURLOPT_MAXREDIRS可以限定递归返回的数量。
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    }
    else
    {
        //缺省情况就是PEM，所以无需设置，另外支持DER
        //curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        // CURLOPT_SSL_VERIFYHOST 默认为2
        //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_nTimeout);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    long lEnd = CSystemInfo::GetInstance()->GetTimestamp(1);
    Log_Info("timeout:%lds request time:%ldms res:%d", m_nTimeout, lEnd-lStart, res);
    return res;
}
