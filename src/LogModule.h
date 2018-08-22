/*日志相关的操作*/
#ifndef __LOG_MODULE_H__
#define __LOG_MODULE_H__

#include <string>
using std::string;

#include "Log.h"
#include "ConfigModule.h"

void StartLogModule();

void SetLogLevel();

void TestLogLevel();

#endif
