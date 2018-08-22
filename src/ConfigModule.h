#ifndef __CONFIG_MODULE_H__
#define __CONFIG_MODULE_H__

#include <string>
using std::string;

#include "Markup.h"
#include "cJSON.h"

#define BASEPARAMNAME "BaseParam"


class CConfigBase
{
public:
    CConfigBase();
    virtual ~CConfigBase();

    bool ParseConfig(const string sFile);
    string GetStringXmlNode(const string sParent, const string sChild);
    long GetNumberXmlNode(const string sParent, const string sChild);

    int SetBaseConfig(const string sType, int nValue, const string sValue);
    int GetBaseConfig(const string sType, string& sValue);

    inline string GetBaseServiceName()     {return m_sBaseServiceName;}
    inline string GetBaseServiceHost()     {return m_sBaseServiceHost;}
    inline int    GetBaseServicePort()     {return m_nBaseServicePort;}
    inline int    GetBaseLogLevel()        {return m_nBaseLogLevel;}
    inline string GetBaseLogPath()         {return m_sBaseLogPath;}

private:
    bool OpenXmlConfig(string sFile);

private:
    CMarkup*        m_pXmlFile;
    bool            m_bIsXmlOpen;

    string          m_sBaseServiceName;
    string          m_sBaseServiceHost;
    int             m_nBaseServicePort;
    int             m_nBaseLogLevel;
    string          m_sBaseLogPath;
};

class CConfigModule : public CConfigBase
{
public:
    CConfigModule();
    ~CConfigModule();

    static CConfigModule* GetInstance(); 
private:
    static CConfigModule* s_pConfig;
};

#endif
