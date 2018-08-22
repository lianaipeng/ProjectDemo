#ifndef _WIN32
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#endif

#include "LogModule.h"
#include "WebService.h"
#include "WebModule.h"
#include "RedisClient.h"
#include "HttpClient.h"
#include "KafkaProducer.h"
#include "KafkaConsumer.h"

bool g_exit = false;

static void sig_handle(int sig) 
{
    if (sig == SIGINT) {
        Log_Debug("sig_handle");
        g_exit = true;
    }
}

int main(int argc, char **argv) 
{
#ifndef _WIN32
    signal(SIGINT, sig_handle);            // Ctrl+C
    signal(SIGPIPE, SIG_IGN);
#endif

    string sConfig = "";
    if (argc == 1) {
        sConfig = "./config/profile.xml";
    } else if (argc == 2) {
        sConfig = string(argv[1]);
    } else {
        printf("Error.argv:%s sConfig:%s\n", argv[0], sConfig.c_str());
        return -1;
    }
    
    if (!CConfigModule::GetInstance()->ParseConfig(sConfig)) {
        printf("Error.ParseConfig sConfig:%s\n", sConfig.c_str());
        return -1;
    }
    
    StartLogModule();

    string sHost = CConfigModule::GetInstance()->GetBaseServiceHost();
    int nPort = CConfigModule::GetInstance()->GetBaseServicePort();
    CWebService* pWebService = new CWebService();
    pWebService->Start(sHost, nPort);

    CWebModule* pWebModule = new CWebModule();
    pWebModule->OnRegisterInterface();
    
    /*
    CRedisClient* pRedis = new CRedisClient();
    */
    /*
    CHttpClient* pHttp = new CHttpClient();
    pHttp->SetDebug(true);
    string sResponse = "";
    //pHttp->Get("http://www.baidu.com", sResponse);
    pHttp->Gets("https://www.baidu.com", sResponse);
    //pHttp->Get("http://curl.haxx.se/ca/cacert.pem", sResponse);
    //pHttp->Gets("https://www.alipay.com", sResponse);
    */
    /*
    CKafkaProducer* pProducer= new CKafkaProducer();
    pProducer->Init("192.168.1.41:9091", "test", 1);
    string sTmp = "dddddddd";
    pProducer->Send(sTmp.c_str(), sTmp.length());
    */

    CKafkaConsumer *pConsumer = new CKafkaConsumer();
    pConsumer->Init("192.168.1.41:9091", "test", "0", "11", NULL, NULL);
    pConsumer->Start(1000);
    while(!g_exit)
    {
        sleep(5);
    }

    return 0;
}
