#include "Engine.h"

namespace imp
{
    Engine::Engine()
    {
    }

    VkResult Engine::InitializeEngine(const void* pNext)
    {
        return volkInitialize();
    }

    VkResult Engine::ShutdownEngine(const void* pNext)
    {
        return VK_SUCCESS;
    }
}