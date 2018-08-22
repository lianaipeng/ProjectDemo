/*******************************************************************************
* 版权所有：	北京市博汇科技有限公司(C) 2013
* 文件名称：	Log.h 
*
* 内容摘要： 	日志类，写文件和STD输出
*
--------------------------------------------------------------------------------
* 版本		修改时间		修改人		修改内容
* V1.0		2013/08/30		龚清华		创建
* V1.1		2014/09/30		龚清华		增加CLogSrv等级输出、CLog对象
* V1.2		2015/07/27		龚清华		修正Log格式化缓冲区保护，采用vsnprintf
*******************************************************************************/
#ifndef _BroadvTool_Log_H_
#define _BroadvTool_Log_H_

#include <string>
#include <vector>
#include "Thread.h"

#include <stdio.h>
#include <stdarg.h>

namespace BroadvTool
{

static char LogLevelStr[4][8] =
{
    "DEBUG", " INFO", " WARN", "ERROR"
};

class CLogSrv : public Thread
{
typedef std::vector<std::string> MsgVec;

public:
    CLogSrv();
    virtual ~CLogSrv();

    // dir 尾部不带'\\'或'/'
    void SetDir(const std::string & dir);

    void SetFile(const std::string & file);

    // 输出 <= level 的日志
    void SetLevel(unsigned int level = 4);
    unsigned int GetLevel() const;

    // 开关 （p—printf/f-file）
    void Enable(bool p, bool f);

    // 写日志
    void WriteLog(unsigned int level, const char * file, unsigned int line, const char * sFunc, const char * buf);

    // 格式化写日志，（注意：内存限制 4096）
    //void FormatLog(unsigned int level, const char * fmt, ...);

private:
    virtual void Routine();

private:
    bool m_bStop;

    Mutex m_cs;
    MsgVec m_vecMsg;

    std::string m_strDir;
    std::string m_strFile;
    bool m_bPrint, m_bFile;

    unsigned int m_nLevel;
};


// 默认日志服务器
extern CLogSrv g_DefaultLogSrv;


// 日志对象
class CLogObj
{
public:
    CLogObj(unsigned int level) : m_nLevel(level) {}
    CLogObj(unsigned int level, std::string file, unsigned int line, std::string func) 
        : m_nLevel(level), m_nLine(line), m_sFunc(func) {
            int npos = file.rfind("/");
            if (npos > 0)
            {
                m_sFile = file.substr(npos+1);
            } 
            else 
            {
                m_sFile = "";
            }
        }
    ~CLogObj() {}

    void operator()(const char * fmt, ...) const
    {
        if (m_nLevel < g_DefaultLogSrv.GetLevel())
            return;

        char buf[4096]={0};
        va_list ptr; 
        va_start(ptr, fmt);
        //vsprintf(buf, fmt, ptr); 
        vsnprintf(buf, 4096, fmt, ptr); 
        va_end(ptr);
        g_DefaultLogSrv.WriteLog(m_nLevel, m_sFile.c_str(), m_nLine, m_sFunc.c_str(), buf);
    }

private:
    unsigned int m_nLevel;
    std::string  m_sFile;
    unsigned int m_nLine;
    std::string  m_sFunc;
};

#define Log_Debug   BroadvTool::CLogObj(1, __FILE__, __LINE__, __func__)
#define Log_Info    BroadvTool::CLogObj(2, __FILE__, __LINE__, __func__)
#define Log_Warn    BroadvTool::CLogObj(3, __FILE__, __LINE__, __func__)
#define Log_Error   BroadvTool::CLogObj(4, __FILE__, __LINE__, __func__)

};  // namespace BroadvTool

#endif  // _BroadvTool_UdpSrv_H_

