#include "Engine.h"

#include <android/log.h>
#include <string>

int LogPrintWrapper(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int res = __android_log_vprint(ANDROID_LOG_ERROR, "IMP", format, args);
    va_end(args);
    return res;
}

constexpr void LogMessage(const char* msg) { __android_log_write(ANDROID_LOG_INFO, "IMP", msg); }

int main(int argc, char** args)
{
    LogMessage("Hello from Imperial Engine Demo!\n");

    imp::EngineCreateParams createParams {};
    createParams.platformLogFunc = LogPrintWrapper;
    imp::Engine engine {};
    VkResult result = engine.InitializeEngine(createParams);

    LogPrintWrapper("Result from initializing engine: %d\n", result);


    return result;
}