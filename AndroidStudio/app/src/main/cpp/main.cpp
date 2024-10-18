#include <jni.h>
#include <android_native_app_glue.h>
#include <dlfcn.h>
#include <android/log.h>
#include <vector>
#include <string>
#include <string_view>

template<typename... Args>
constexpr void LogError(Args... args) { __android_log_print(ANDROID_LOG_ERROR, "IMP", args...); }
constexpr void LogMessage(const char* msg) { __android_log_write(ANDROID_LOG_INFO, "IMP", msg); }

typedef int (*MainFunc)(int, char*[]);

void SplitString(jsize strLen, const char* str, std::vector<std::string>& argv, char delimiter)
{
    std::string_view input(str, strLen);
    std::size_t start = 0;
    std::size_t end = 0;

    while ((end = input.find(delimiter, start)) != std::string_view::npos)
    {
        if (end != start) {
            std::size_t length = end - start;
            argv.emplace_back(input.data() + start, length);
        }
        start = end + 1;
    }

    if (start < strLen) {
        std::size_t length = strLen - start;
        char* newArg = new char[length + 1];
        argv.emplace_back(input.data() + start, length);
    }
}

void GetIntentArgs(struct android_app *pApp, std::vector<std::string>& argv)
{
    JNIEnv* env;
    pApp->activity->vm->AttachCurrentThread(&env, nullptr);

    jobject activity = pApp->activity->clazz;
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getIntentMethod = env->GetMethodID(activityClass, "getIntent", "()Landroid/content/Intent;");
    jobject intent = env->CallObjectMethod(activity, getIntentMethod);

    jclass intentClass = env->GetObjectClass(intent);
    jmethodID getStringExtraMethod = env->GetMethodID(intentClass, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

    jstring args = (jstring)env->CallObjectMethod(intent, getStringExtraMethod, env->NewStringUTF("args"));

    // argv first argument is usually the executable path
    argv.push_back(pApp->activity->internalDataPath);

    if (args)
    {
        const char* longArgs = env->GetStringUTFChars(args, nullptr);
        jsize strLen = env->GetStringUTFLength(args);
        SplitString(strLen, longArgs, argv, ' ');
        env->ReleaseStringUTFChars(args, longArgs);
    }

    env->DeleteLocalRef(args);
}

static bool g_ExitNow = false;

void HandleCmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_DESTROY:
            LogMessage("[IMP Native Activity] APP_CMD_DESTROY event received, will kill app soon\n");
            g_ExitNow = true;
            break;
        case APP_CMD_PAUSE:
            LogMessage("[IMP Native Activity] APP_CMD_PAUSE event received, will kill app soon\n");
            g_ExitNow = true;
            break;
        default:
            break;
    }
}

std::vector<char*> GetNonConstPtrs(std::vector<std::string>& strings)
{
    std::vector<char*> copies;
    copies.reserve(strings.size());

    for (auto& str : strings)
        copies.push_back(&str[0]);

    return copies;
}

void android_main(struct android_app *pApp)
{
    pApp->onAppCmd = HandleCmd;

    void* handle = dlopen("libapplication.so", RTLD_LAZY);
    if (!handle)
    {
        LogError("Failed to load library: %s", dlerror());
        return;
    }

    MainFunc mainFunc = (MainFunc) dlsym(handle, "main");
    const char* dlsymError = dlerror();
    if (dlsymError)
    {
        LogError("Failed to load symbol: %s", dlsymError);
        dlclose(handle);
        return;
    }

    std::vector<std::string> argv;
    GetIntentArgs(pApp, argv);
    std::vector<char*> throwawayCopyArgv = GetNonConstPtrs(argv);

    //int result = mainFunc(argv.size(), throwawayCopyArgv.data());
    int result = mainFunc(1, nullptr);
    LogError("Result from main: %d", result);

    bool finished = false;
    while (true)
    {
        int events;
        struct android_poll_source* source;

        if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0)
        {
            if (source != nullptr)
                source->process(pApp, source);

            if (g_ExitNow)
            {
                // Android ART processes don't die after exit, must kill manually
                exit(0);
                return;
            }

            if(!finished)
            {
                dlclose(handle);
                ANativeActivity_finish(pApp->activity);
                finished = true;
            }
        }
    }
}
