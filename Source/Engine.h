#pragma once
#include "volk.h"
//#include "vulkan/vulkan.h"

namespace imp
{
    class Engine
    {
    public:
        Engine();

        VkResult InitializeEngine(const void* pNext);
        VkResult ShutdownEngine(const void* pNext);
        
    private:

    };   
}