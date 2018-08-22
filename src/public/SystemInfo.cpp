#include "SystemInfo.h"

CSystemInfo* CSystemInfo::s_pSystem = NULL;

CSystemInfo* CSystemInfo::GetInstance()
{
    if (s_pSystem == NULL)
    {
        s_pSystem = new CSystemInfo();
    }
    return s_pSystem;
}

CSystemInfo::CSystemInfo()
{
    m_sHostName         = "";
    m_nCpuNum           = 0;
    m_nCpuNum           = 0;
    m_sHostName         = "";
    m_fCurLoadAverage   = 0;
    m_nLastTimestamp    = 0;
}
CSystemInfo::~CSystemInfo()
{

}

string CSystemInfo::GetHostName()
{
    if (m_sHostName == "")
    {
        char szHostName[128] = {'\0'};
        gethostname(szHostName, sizeof(szHostName));
        m_sHostName = string(szHostName);
    }
    return m_sHostName;
}
int CSystemInfo::GetCpuNum()
{
    if (m_nCpuNum <= 0)
    {
        char cmdline[100];
        FILE* file = NULL;
        char line[1024];
        int CpuNum = 0;
        sprintf(cmdline, "cat /proc/cpuinfo |grep \"processor\"|wc -l");
        file = popen(cmdline, "r");
        if (file)
        {
            if (fgets(line, 1024, file) != NULL)
            {
                sscanf(line, "%d", &CpuNum );
                m_nCpuNum = CpuNum;
            }
            pclose(file);
        }
    }
    return m_nCpuNum;
}
float CSystemInfo::GetCurLoad()
{
    struct timeval cur;
    gettimeofday(&cur,NULL);
    if (cur.tv_sec - m_nLastTimestamp < 2)
    {
        return m_fCurLoadAverage;
    }

    char* szFileName = "/proc/loadavg";
    char szLine[1024] = {'\0'};
    FILE* fp = NULL;
    float fltCurLoad1 = 0.0;
    float fltCurLoad5 = 0.0;
    float fltCurLoad15 = 0.0;
    fp = fopen(szFileName, "rb");
    if (fp)
    {
        if (fgets(szLine, 1024, fp) != NULL)
        {
            sscanf(szLine, "%f %f %f", &fltCurLoad1, &fltCurLoad5, &fltCurLoad15);
            m_fCurLoadAverage = fltCurLoad1;
        }
        fclose(fp);
        m_nLastTimestamp = cur.tv_sec;
    }
    return m_fCurLoadAverage;
}
float CSystemInfo::GetMaxLoad()
{
    return GetCpuNum();
}

string CSystemInfo::GetDate()
{
    time_t timep;
    struct tm *p = NULL;
    timep = time(NULL);
    p = gmtime(&timep);
    if (p == NULL)
        return "";

    char szTime[64] = {'\0'};
    sprintf(szTime, "%04d.%02d.%02d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday);
    return string(szTime);
}

string CSystemInfo::GetTime()
{
    time_t timep;
    struct tm *p = NULL;
    timep = time(NULL);
    p = gmtime(&timep);
    if (p == NULL)
        return "";

    char szTime[64] = {'\0'};
    sprintf(szTime, "%04d/%02d/%02d %02d:%02d:%02d", 
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    return string(szTime);
}

long CSystemInfo::GetTimestamp(int nFlag)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (nFlag == 0)
        return tv.tv_sec ;
    else if (nFlag == 1)
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    else
        return tv.tv_sec * 1000000 + tv.tv_usec;
}
