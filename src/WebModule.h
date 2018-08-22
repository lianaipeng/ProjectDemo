#ifndef __WEB_MODULE_H__
#define __WEB_MODULE_H__

#include <string>

#include "Log.h"
#include "WebService.h"

using std::string;

// ############ Web接口模块 ############
class CWebModule : public CWebInterface 
{
public:
    CWebModule();
    ~CWebModule();

    //注册服务
    virtual bool OnRegisterInterface();
    //处理函数
    virtual bool OnServiceExecute(const HttpRequest &request, string &strRes);
    
private:
    bool GetPingStatus(const HttpRequest &request, string & strJson);
};

#endif
