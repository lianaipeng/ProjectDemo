#include "WebModule.h"
#include "ConfigModule.h"

#define INTERFACE_PING_STATUS  "/interface/ping"
#define INTERFACE_SET_CONFIG  "/interface/set/config"
#define INTERFACE_GET_CONFIG  "/interface/get/config"
#define INTERFACE_EXIT_PROCESS  "/interface/exit/process"


// ############ Web接口模块 ############
CWebModule::CWebModule()
{
}

CWebModule::~CWebModule()
{
}

//注册服务
bool CWebModule::OnRegisterInterface()
{
    CWebInterface::AddInterface(INTERFACE_PING_STATUS, this); 
    CWebInterface::AddInterface(INTERFACE_SET_CONFIG, this); 
    CWebInterface::AddInterface(INTERFACE_GET_CONFIG, this); 
    CWebInterface::AddInterface(INTERFACE_EXIT_PROCESS, this); 
}

//处理函数
bool CWebModule::OnServiceExecute(const HttpRequest &request, string &sRes)
{
    // 参数判断
    map<string,string> tmp = request.m_mapArgs;
    string sSerName = tmp["servicename"];
    string sService = CConfigModule::GetInstance()->GetBaseServiceName();
    if (sSerName != sService)
    {
        Log_Error("servicename:%s request servicename:%s", sService.c_str(), sSerName.c_str());
        CWebInterface::ResponseMsg(REQUEST_PARAM_ERROR, "param error", "", sRes);
        return false;
    }

    // 接口处理
    if (request.m_strURL == INTERFACE_PING_STATUS)
    {
        return GetPingStatus(request, sRes);
    }
    else if (request.m_strURL == INTERFACE_SET_CONFIG) 
    {
        string sType    = tmp["config_type"];
        string sValue   = tmp["config_value"];
        int    nValue   = atoi(tmp["config_value"].c_str());

        if (CConfigModule::GetInstance()->SetBaseConfig(sType, nValue, sValue) == 0) 
            CWebInterface::ResponseMsg(REQUEST_SUCCESS, "success", "", sRes);
        else 
            CWebInterface::ResponseMsg(REQUEST_NOT_SUPPORT, "not support", "", sRes);
        return true;
    }
    else if (request.m_strURL == INTERFACE_GET_CONFIG) 
    {
        CConfigModule::GetInstance()->GetBaseConfig("", sRes);
        return true;
    }
    else if (request.m_strURL == INTERFACE_EXIT_PROCESS) 
    {
        
        return true;
    }
    return false;
}

bool CWebModule::GetPingStatus(const HttpRequest &request, string & strJson)
{
    map<string,string> tmp = request.m_mapArgs;
    string strUUID              = tmp["uuid"];
    string strID                = tmp["id"];
    long lTimestamp             = atol(tmp["timestamp"].c_str());

    CWebInterface::ResponseMsg(REQUEST_SUCCESS, "success", "", strJson);
    //CWebInterface::ResponseMsg(REQUEST_PARAM_ERROR, "param error", "", strJson);
    return true;
}
