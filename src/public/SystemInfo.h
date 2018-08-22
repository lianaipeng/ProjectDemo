#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

//#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <string>
using std::string;

class CSystemInfo
{
public:
    CSystemInfo();
    ~CSystemInfo();

    static CSystemInfo* GetInstance();  

    string GetHostName();
    int    GetCpuNum();
    float  GetCurLoad();
    float  GetMaxLoad();

    string GetDate();
    string GetTime();
    long   GetTimestamp(int nFlag=0); //0 秒 1 毫秒 2 微秒

private:
    static CSystemInfo*  s_pSystem;

    int                  m_nCpuNum;
    string               m_sHostName;
    float                m_fCurLoadAverage;
    long                 m_nLastTimestamp;
};

#endif
