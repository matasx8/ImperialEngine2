#include "Engine.h"

#include <android/log.h>
#include <string>

constexpr void LogMessage(const char* msg) { __android_log_write(ANDROID_LOG_INFO, "IMP", msg); }

int main(int argc, char** args)
{
    LogMessage("Hello from Imperial Engine Demo!");
    return imp::Add(argc, 4);
}