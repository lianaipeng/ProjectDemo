#include "WebService.h"

map<string, void*>  CWebInterface::s_mapInterface;
BroadvTool::Mutex   CWebInterface::s_mapLock;


void CWebInterface::AddInterface(const string& sInterface, void* dwPtr)
{
    if (dwPtr == NULL || sInterface == "")
    {
        Log_Error("AddInterface:%s void:%p", sInterface.c_str(), dwPtr);
        return ;
    }

    if (FindInterface(sInterface) == NULL)
    {
        s_mapLock.Lock();
        s_mapInterface[sInterface] = dwPtr;
        s_mapLock.Unlock();
        //Log_Info("AddInterface:%s OK", sInterface.c_str());
    }
}

void* CWebInterface::DelInterface(const string& sInterface)
{
    void *ptr = NULL;
    s_mapLock.Lock();
    map<string, void*>::iterator iter = s_mapInterface.find(sInterface);
    if (iter != s_mapInterface.end())
    {
        ptr = iter->second;
        s_mapInterface.erase(iter);
    }
    s_mapLock.Unlock();
    return ptr;
}

void* CWebInterface::FindInterface(const string& sInterface)
{
    void *ptr = NULL;
    s_mapLock.Lock();
    map<string, void*>::iterator iter = s_mapInterface.find(sInterface);
    if (iter != s_mapInterface.end())
    {
        ptr = iter->second;
    }
    s_mapLock.Unlock();
    return ptr;    
}

bool CWebInterface::OnProcessService(const HttpRequest &request, string &sRes)
{
    void *ptr = FindInterface(request.m_strURL);
    if (ptr)
    {
        HttpRequest req = request;
        CWebInterface *pService = (CWebInterface*)ptr;
        return pService->OnServiceExecute(req, sRes);
    } 
    else
    {
        Log_Error("not found requst url:%s", request.m_strURL.c_str());
    }
    return false;
}

void CWebInterface::ResponseMsg(int nStatusCode, const string sStatus, const string sResult, string &sResponse)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
        return ;

    cJSON_AddNumberToObject(root, "timesec", (int)time(0));
    cJSON_AddNumberToObject(root, "ec", nStatusCode);
    cJSON_AddStringToObject(root, "em", sStatus.c_str());
    cJSON_AddStringToObject(root, "result", sResult.c_str());

    char *p = cJSON_PrintUnformatted(root);
    if (p) 
    {
        sResponse = string(p);
        free(p);
    }
    cJSON_Delete(root);
    return;
}


// ############ Web监听服务 ############
void CWebService::OnAccept()
{
    SockAddr cAddr;
    memset(&cAddr, 0, sizeof(cAddr));
    SOCKET s = Accept(&cAddr);
    if (s == INVALID_SOCKET) 
    {
        Log_Error("OnAccept failed, err=%d client:%s", WSAGetLastError(), cAddr.GetIP().c_str());
        return;
    }

    if (!JoinThread(this, s)) 
    {
        Socket tmp(s);
        Log_Error("OnAccept procecss thread full, size=%d", m_nMaxThread);
        return;
    }
}

int CWebService::OnService(const HttpRequest &request, HttpResponse &response, ServletHandler &handler)
{
    if (CWebInterface::OnProcessService(request, response.m_strBody))
    {
        response.Line("Content-Length", (int)response.m_strBody.size());
    }
    else
    {
        response.m_nStatusCode = 404;
        response.m_strStatusMsg = "Not Found";
    }
    
    return ServletReturnType_SendResponse;
}
