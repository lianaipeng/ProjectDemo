#include "LogModule.h"

void StartLogModule()
{
    string sFile = CConfigModule::GetInstance()->GetBaseServiceName();
    string sHost = CConfigModule::GetInstance()->GetBaseServiceHost();
    int    nPort = CConfigModule::GetInstance()->GetBaseServicePort();
    string sPath = CConfigModule::GetInstance()->GetBaseLogPath();
    int    nLevel = CConfigModule::GetInstance()->GetBaseLogLevel();

    //printf("###################### StartLogModule\n");
    BroadvTool::g_DefaultLogSrv.Enable(true, true);
    BroadvTool::g_DefaultLogSrv.SetDir(sPath);
    BroadvTool::g_DefaultLogSrv.SetFile(sFile);
    BroadvTool::g_DefaultLogSrv.SetLevel(nLevel);
    TestLogLevel();

    Log_Info("Start process port:%d path:%s", nPort, sPath.c_str());
    //Log_Debug("Start process port:%d path:%s count:", nPort, sPath.c_str(), BroadvTool::LEVEL_COUNT);
}

void SetLogLevel()
{
    int nLevel = CConfigModule::GetInstance()->GetBaseLogLevel();
    Log_Warn("change log level:%d:", nLevel);

    BroadvTool::g_DefaultLogSrv.SetLevel(nLevel);
    TestLogLevel();
}

void TestLogLevel()
{
    Log_Debug("############ TestLogLevel");
    Log_Info("############ TestLogLevel");
    Log_Warn("############ TestLogLevel");
    Log_Error("############ TestLogLevel");
}
