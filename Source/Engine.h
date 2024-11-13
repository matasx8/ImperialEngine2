#pragma once
#include "volk.h"
#include "Log.h"
#include "Queue.h"

#include <vector>
#include <string>

namespace imp
{
    struct EngineCreateParams
    {
        const void* pNext;
        LogFunc platformLogFunc;
    };

    class Engine
    {
    public:
        Engine() = default;

        VkResult Initialize(const EngineCreateParams& params);
        VkResult Shutdown();
        
    private:

        VkResult CreateInstance(const EngineCreateParams& params);
        void DestroyInstance();

        VkResult SelectPhysicalDevice(const EngineCreateParams& params);

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        std::vector<std::string> m_EnabledInstanceLayers = {};
        std::vector<std::string> m_EnabledInstanceExtensions = {};

        Queue m_Queue = {};
    };   
}