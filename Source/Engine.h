#pragma once
#include "volk.h"
#include "Log.h"
#include "Queue.h"
#include "CommandBufferPool.h"
#include "SubmitSyncManager.h"

#include <vector>
#include <string>
#include <limits>

namespace imp
{
    struct EngineCreateParams
    {
        const void* pNext;
        LogFunc platformLogFunc;
    };

    struct SubmitParams
    {
        VkQueue queue; // Probably need to wrap VkQueue so I can differentiate between types of queues
        const VkCommandBuffer* pCommandBuffers;
        uint32_t commandBufferCount;
    };

    class Engine
    {
    public:
        Engine() = default;

        VkResult Initialize(const EngineCreateParams& params);
        VkResult Shutdown();

        SubmitSync Submit(const SubmitParams* pParams, uint32_t paramsCount);
        VkResult WaitForSubmitSync(const SubmitSync& sync, uint64_t timeout = ULLONG_MAX);
        
    private:

        VkResult CreateInstance(const EngineCreateParams& params);
        void DestroyInstance();

        VkResult SelectPhysicalDevice(const EngineCreateParams& params);

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        std::vector<std::string> m_EnabledInstanceLayers = {};
        std::vector<std::string> m_EnabledInstanceExtensions = {};

        // Can be the same pool if Queue families are the same
        CommandBufferPool* m_GraphicsCommandPool = nullptr;
        CommandBufferPool* m_ComputeCommandPool = nullptr;

        SubmitSyncManager m_SubmitSyncManager;

        Queue m_Queue = {};
    };   
}