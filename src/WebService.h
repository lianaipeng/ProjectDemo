#ifndef __WEB_SERVICE_H__
#define __WEB_SERVICE_H__

#include <map>
using std::map;
using std::string;

#include "public/public_include.h"

enum REQUEST_STATUS_CODE {
    REQUEST_SUCCESS         = 200,
    REQUEST_UNKNOWN_ERROR   = 600,
    REQUEST_PARAM_ERROR     = 601,
    REQUEST_NOT_FOUNT       = 602,
    REQUEST_NOT_SUPPORT     = 603
};

// ############ Web接口服务 ############
class CWebInterface 
{
public:
    CWebInterface() {};
    ~CWebInterface() {};
    
    //注册服务
    virtual bool OnRegisterInterface() = 0;
    //处理函数
    virtual bool OnServiceExecute(const HttpRequest &request, string &sRes) = 0;

    static void AddInterface(const string& sInterface, void* dwPtr);
    static void* DelInterface(const string& sInterface);
    static void* FindInterface(const string& sInterface);
    
    static bool OnProcessService(const HttpRequest &request, string &sRes);
    static void ResponseMsg(int nStatusCode, const string sStatus, const string sResult, string &sResponse);

private:
    static map<string, void*>   s_mapInterface;
    static BroadvTool::Mutex    s_mapLock;
};


// ############ Web监听服务 ############
class CWebService : public WebServer, public Servlet
{
public:
    CWebService() {};
    ~CWebService() {};

    /*
    //启动监听网络服务
    int Start(string sIP, unsigned short nPort);

    //停止监听服务网络
    void Stop();
    */
protected:
    virtual void OnAccept();

    virtual int OnService(const HttpRequest &request, HttpResponse &response, ServletHandler &handler);
};

#endif
