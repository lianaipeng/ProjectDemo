#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <string>
#include <map>
#include <list>
using std::string;

#include <curl/curl.h>

/*
 * 参考：https://blog.csdn.net/huyiyang2010/article/details/7664201
 */

/** curl_easy_perform 返回值
 * 0:  CURLE_OK = 0,                no error  
 * 1:  CURLE_UNSUPPORTED_PROTOCOL,  unsupported protocol  
 * 2:  CURLE_FAILED_INIT,           failed init  
 * 3:  CURLE_URL_MALFORMAT,         URL using bad/illegal format or missing URL  
 * 4:  CURLE_URL_MALFORMAT_USER,    unknown error  
 * 5:  CURLE_COULDNT_RESOLVE_PROXY, couldn’t resolve proxy name  
 * 6:  CURLE_COULDNT_RESOLVE_HOST,  couldn’t resolve host name  
 * 7:  CURLE_COULDNT_CONNECT,       couldn’t connect to server  
 * 8:  CURLE_FTP_WEIRD_SERVER_REPLY, FTP: weird server reply  
 * 9:  CURLE_FTP_ACCESS_DENIED,       
 * 10: CURLE_FTP_USER_PASSWORD_INCORRECT, unknown error  
 * 11: CURLE_FTP_WEIRD_PASS_REPLY,  FTP: unknown PASS reply  
 * 12: CURLE_FTP_WEIRD_USER_REPLY,  FTP: unknown USER reply  
 * 13: CURLE_FTP_WEIRD_PASV_REPLY,  FTP: unknown PASV reply  
 * 14: CURLE_FTP_WEIRD_227_FORMAT,  FTP: unknown 227 response format  
 * 15: CURLE_FTP_CANT_GET_HOST,     FTP: can’t figure out the host in the PASV response  
 * 16: CURLE_FTP_CANT_RECONNECT,    FTP: can’t connect to server the response code is unknown  
 * 17: CURLE_FTP_COULDNT_SET_BINARY, FTP: couldn’t set binary mode  
 * 18: CURLE_PARTIAL_FILE,          Transferred a partial file  
 * 19: CURLE_FTP_COULDNT_RETR_FILE, FTP: couldn’t retrieve (RETR failed) the specified file  
 * 20: CURLE_FTP_WRITE_ERROR,       FTP: the post-transfer acknowledge response was not OK  
 * 21: CURLE_FTP_QUOTE_ERROR,       FTP: a quote command returned error  
 * 22: CURLE_HTTP_RETURNED_ERROR,   HTTP response code said error  
 * 23: CURLE_WRITE_ERROR,           failed writing received data to disk/application  
 * 24: CURLE_MALFORMAT_USER,        unknown error  
 * 25: CURLE_UPLOAD_FAILED,         upload failed (at start/before it took off)  
 * 26: CURLE_READ_ERROR,            failed to open/read local data from file/application  
 * 27: CURLE_OUT_OF_MEMORY,         out of memory  
 * 28: CURLE_OPERATION_TIMEOUTED,   a timeout was reached  
 * 29: CURLE_FTP_COULDNT_SET_ASCII, FTP could not set ASCII mode (TYPE A)  
 * 30: CURLE_FTP_PORT_FAILED,       FTP command PORT failed  
 * 31: CURLE_FTP_COULDNT_USE_REST,  FTP command REST failed  
 * 32: CURLE_FTP_COULDNT_GET_SIZE,  FTP command SIZE failed  
 * 33: CURLE_HTTP_RANGE_ERROR,      a range was requested but the server did not deliver it  
 * 34: CURLE_HTTP_POST_ERROR,       internal problem setting up the POST  
 * 35: CURLE_SSL_CONNECT_ERROR,     SSL connect error  
 * 36: CURLE_BAD_DOWNLOAD_RESUME,   couldn’t resume download  
 * 37: CURLE_FILE_COULDNT_READ_FILE,couldn’t read a file
 * 38: CURLE_LDAP_CANNOT_BIND,      LDAP: cannot bind  
 * 39: CURLE_LDAP_SEARCH_FAILED,    LDAP: search failed  
 * 40: CURLE_LIBRARY_NOT_FOUND,     a required shared library was not found
 */

class CHttpClient 
{
public:
    CHttpClient();
    ~CHttpClient();

    void SetDebug(bool bDebug);

    /**
     * @brief HTTP GET请求
     * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
     * @param strResponse 输出参数,返回的内容
     * @return 返回是否Post成功
     */
    int Get(const std::string & strUrl, std::string & strResponse);

    /**
     * @brief HTTP POST请求
     * @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
     * @param strPost 输入参数,使用如下格式para1=val1&para2=val2&…
     * @param strResponse 输出参数,返回的内容
     * @return 返回是否Post成功
     */
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);

    /**
     * @brief HTTPS GET请求,无证书版本
     * @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
     * @param strResponse 输出参数,返回的内容
     * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
     * @return 返回是否Post成功
     */
    int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);

    /**
     * @brief HTTPS POST请求,无证书版本
     * @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
     * @param strPost 输入参数,使用如下格式para1=val1&para2=val2&…
     * @param strResponse 输出参数,返回的内容
     * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
     * @return 返回是否Post成功
     */
    int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);

private:
    bool    m_bDebug;
    int     m_nTimeout;
};

#endif
