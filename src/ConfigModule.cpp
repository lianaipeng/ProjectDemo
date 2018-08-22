#include "ConfigModule.h"

CConfigBase::CConfigBase() 
{
    m_pXmlFile          = NULL;
    m_bIsXmlOpen        = false;

    m_sBaseServiceName  = "";
    m_sBaseServiceHost  = "";
    m_nBaseServicePort  = 0;
    m_nBaseLogLevel     = 0;
    m_sBaseLogPath      = ""; 
}

CConfigBase::~CConfigBase() 
{
    
}

bool CConfigBase::ParseConfig(const string sFile)
{
    bool bRet = false;

    if (!OpenXmlConfig(sFile))
        return bRet;
    m_bIsXmlOpen = true;
    
    m_sBaseServiceName  = GetStringXmlNode(BASEPARAMNAME, "ServiceName");
    m_sBaseServiceHost  = GetStringXmlNode(BASEPARAMNAME, "ServiceHost");
    m_nBaseServicePort  = GetNumberXmlNode(BASEPARAMNAME, "ServicePort");
    m_nBaseLogLevel     = GetNumberXmlNode(BASEPARAMNAME, "LogLevel");
    m_sBaseLogPath      = GetStringXmlNode(BASEPARAMNAME, "LogPath");
    bRet = true;
    return bRet;
}

string CConfigBase::GetStringXmlNode(const string sParent, const string sChild)
{
    string sRet = "";
    if (!m_bIsXmlOpen || m_pXmlFile==NULL)
        return sRet;

    m_pXmlFile->ResetPos();
    m_pXmlFile->FindElem();
    m_pXmlFile->IntoElem();

    if (m_pXmlFile->FindElem(sParent.c_str()))
    {
        m_pXmlFile->ResetChildPos();
        m_pXmlFile->FindChildElem(sChild.c_str());
        sRet = m_pXmlFile->GetChildData();
    }
    return sRet;
}

long CConfigBase::GetNumberXmlNode(const string sParent, const string sChild)
{
    string  sRet = "";
    long    lRet = 0L;
    if (!m_bIsXmlOpen || m_pXmlFile==NULL)
        return lRet;

    m_pXmlFile->ResetPos();
    m_pXmlFile->FindElem();
    m_pXmlFile->IntoElem();

    if (m_pXmlFile->FindElem(sParent.c_str()))
    {
        m_pXmlFile->ResetChildPos();
        m_pXmlFile->FindChildElem(sChild.c_str());
        sRet = m_pXmlFile->GetChildData();
        lRet = atol(sRet.c_str());
    }
    return lRet;
}

bool CConfigBase::OpenXmlConfig(const string sFile)
{
    if (sFile == "")
        return false;

    if (m_pXmlFile == NULL)
        m_pXmlFile = new CMarkup();

    m_pXmlFile->Load(sFile.c_str());

    if (!m_pXmlFile->IsWellFormed())
    {
        return false;;
    }
    if (!m_pXmlFile->FindElem("Profile"))
    {
        return false;
    }
    if (!m_pXmlFile->IntoElem())
    {
         return false;;
    }
    return true;
}

int CConfigBase::SetBaseConfig(const string sType, int nValue, const string sValue)
{
    if (sType == "ServiceHost") 
    {
        //m_sBaseServiceHost = sValue;
        return -1;
    } 
    else if (sType == "ServicePort")
    {
        //m_nBaseServicePort = nValue;
        return -1;
    }
    else if (sType == "LogLevel")
    {
        m_nBaseLogLevel = nValue;
    }
    else if (sType == "LogPath")
    {
        //m_sBaseLogPath = sValue; 
        return -1;
    } 
    else 
    {
        return -1;
    }
    return 0;
}

int CConfigBase::GetBaseConfig(const string sType, string& sValue)
{
    int nRet = -1;
    cJSON* root = cJSON_CreateArray();
    if (root == NULL)
    {
        sValue = "";
        return nRet;
    }

    cJSON* tv0 = cJSON_CreateObject();
    if (tv0)
    {
        cJSON_AddItemToArray(root, tv0);
        cJSON_AddStringToObject(tv0, "config_type", "ServiceName");
        cJSON_AddStringToObject(tv0, "config_value", m_sBaseServiceName.c_str());
        cJSON_AddStringToObject(tv0, "config_desc", "服务名称");
    }
    cJSON* tv1 = cJSON_CreateObject();
    if (tv1)
    {
        cJSON_AddItemToArray(root, tv1);
        cJSON_AddStringToObject(tv1, "config_type", "ServiceHost");
        cJSON_AddStringToObject(tv1, "config_value", m_sBaseServiceHost.c_str());
        cJSON_AddStringToObject(tv1, "config_desc", "服务地址");
    }
    cJSON* tv2 = cJSON_CreateObject();
    if (tv2)
    {
        cJSON_AddItemToArray(root, tv2);
        cJSON_AddStringToObject(tv2, "config_type", "ServicePort");
        cJSON_AddNumberToObject(tv2, "config_value", m_nBaseServicePort);
        cJSON_AddStringToObject(tv2, "config_desc", "服务端口");
    }
    cJSON* tv3 = cJSON_CreateObject();
    if (tv3)
    {
        cJSON_AddItemToArray(root, tv3);
        cJSON_AddStringToObject(tv3, "config_type", "LogLevel");
        cJSON_AddNumberToObject(tv3, "config_value", m_nBaseLogLevel);
        cJSON_AddStringToObject(tv3, "config_desc", "日志级别");
    }
    cJSON* tv4 = cJSON_CreateObject();
    if (tv4)
    {
        cJSON_AddItemToArray(root, tv4);
        cJSON_AddStringToObject(tv4, "config_type", "LogPath");
        cJSON_AddStringToObject(tv4, "config_value", m_sBaseLogPath.c_str());
        cJSON_AddStringToObject(tv4, "config_desc", "日志路径");
    }

    char* p = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (p)
    {
        sValue = string(p);
        free(p);
        nRet = 0;
    }
    return nRet;
}


CConfigModule* CConfigModule::s_pConfig = NULL;
CConfigModule* CConfigModule::GetInstance()
{
    if (s_pConfig == NULL)
        s_pConfig = new CConfigModule();
    return s_pConfig;
}

CConfigModule::CConfigModule()
{

}

CConfigModule::~CConfigModule()
{

}
