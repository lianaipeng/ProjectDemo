#include "stdafx.h"
#include "Log.h"

#include "Utility.h"
#include "FileEx.h"

namespace BroadvTool
{

// ��־ģ���ṩһ��ȫ�ֵ�Ĭ��д�����
CLogSrv g_DefaultLogSrv;


CLogSrv::CLogSrv()
{
	m_bPrint = true;
	m_bFile = false;
	m_bStop = true;
	m_nLevel = 3;
}

CLogSrv::~CLogSrv()
{
	m_bStop = true;
	Thread::Join(this);
}

void CLogSrv::SetDir(const std::string & dir)
{
	m_strDir = dir;
}

void CLogSrv::SetFile(const std::string & file)
{
	m_strFile = file;
}

void CLogSrv::SetLevel(unsigned int level)
{
    if (level < 1) 
        m_nLevel = 1;
    else if (level > 4)
        m_nLevel = 4;
    else 
        m_nLevel = level;
}

unsigned int CLogSrv::GetLevel() const
{
	return m_nLevel;
}

void CLogSrv::Enable(bool p, bool f)
{
	m_bPrint = p;
	m_bFile = f;

	// ��ֹͣһ�¡����ܴ��ڡ���д���߳�
	m_bStop = true;
	Thread::Join(this);
	if (m_bFile)
	{
		m_bStop = false;
		Run();
	}
	else
	{
		AutoLock mx(m_cs);
		MsgVec vec;
		m_vecMsg.swap(vec);
	}
}

void CLogSrv::WriteLog(unsigned int level, const char * file, unsigned int line, const char * func, const char * buf)
{
	if (level < m_nLevel)
		return;

	char tm[64]={0};
	struct tm t;
	CUtility::GetLocalTime(&t);
	sprintf(tm, "%04d-%02d-%02d %02d:%02d:%02d",
		t.tm_year+1900, t.tm_mon+1, t.tm_mday,
		t.tm_hour, t.tm_min, t.tm_sec);
	char lv[512]={0};
	//sprintf(lv, "%d", level);
	sprintf(lv, "[%s] [%s:%d:%s]", LogLevelStr[level-1], file, line, func);

	std::string str = tm;
	str += " ";
	str += lv;
	str += " ";
	str += buf;
	str += "\n";

	if (m_bPrint)
	{
#ifdef	TRACE
		TRACE("%s", str.c_str());
#endif
		printf("%s", str.c_str());
	}

	if (m_bFile)
	{
		AutoLock mx(m_cs);
		if (m_vecMsg.size() > 500000)
		{
			m_vecMsg.clear();
			WriteLog(4, "WriteLog", __LINE__, __func__, "CLogSrv cache full��clear it�� size=10000");
		}
		m_vecMsg.push_back(str);
	}	
}

/*
void CLogSrv::FormatLog(unsigned int level,const char * fmt, ...)
{
	if (level < m_nLevel)
		return;

	char buf[4096]={0};
	va_list ptr; 
	va_start(ptr, fmt);
	//vsprintf(buf, fmt, ptr); 
    vsnprintf(buf, 4096, fmt, ptr);
	va_end(ptr);

	WriteLog(level, buf);
}
*/

void CLogSrv::Routine()
{
	FILE * fp = NULL;
	int nHour = -1;
	while (!m_bStop)
	{
		Thread::Sleep(1000);
		if (m_vecMsg.empty())
			continue;
		
		if (m_bStop)
			break;

		struct tm t;
		CUtility::GetLocalTime(&t);
		if (nHour != t.tm_hour)
		{
			// һ��Сʱ��һ��FILE�ļ�
			char buf[256]={0};
            /*
			sprintf(buf, "%s%c%04d%c%02d%c%02d%c%04d%02d%02d_%02d.log", 
				m_strDir.empty() ? CFileEx::GetExeDirectory().c_str() : m_strDir.c_str(), 
				CFileEx::Separator(), 
				t.tm_year+1900, 
				CFileEx::Separator(), 
				t.tm_mon+1, 
				CFileEx::Separator(),
				t.tm_mday,
				CFileEx::Separator(),
				t.tm_year+1900,
				t.tm_mon+1,
				t.tm_mday,
				t.tm_hour);
                */
			sprintf(buf, "%s%c%04d-%02d-%02d%c%s_%02d.log", 
				m_strDir.empty() ? CFileEx::GetExeDirectory().c_str() : m_strDir.c_str(), 
                CFileEx::Separator(),
				t.tm_year+1900,
				t.tm_mon+1, 
				t.tm_mday,
                CFileEx::Separator(),
                m_strFile.empty() ? "log" : m_strFile.c_str(),
				t.tm_hour);
			CFileEx::CreateFolderForFile(buf);

			if (fp != NULL)
			{
				fclose(fp);
				fp = NULL;
			}
			fp = fopen(buf, "a+");
			nHour = t.tm_hour;
		}

		if (fp != NULL)
		{
			MsgVec vec;
			m_cs.Lock();
			vec.swap(m_vecMsg);
			m_cs.Unlock();

			for (size_t i = 0; i < vec.size(); ++i)
			{
				fprintf(fp, "%s", vec[i].c_str());
			}
			fflush(fp);
		}
	}

	// �߳��˳����ر��ļ�
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
}


};	// namespace BroadvTool




