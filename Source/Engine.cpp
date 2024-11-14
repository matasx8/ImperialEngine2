#include "Engine.h"
#include "Layers.h"
#include "Debug.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <set>

namespace imp
{
    VkResult Engine::Initialize(const EngineCreateParams& params)
    {
        if (params.platformLogFunc)
            g_Log = params.platformLogFunc;

        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
            return result;
        g_Log("VOLK initialized successfully.\n");

        result = CreateInstance(params);
        if (result != VK_SUCCESS)
            return result;
        g_Log("Vulkan Instance was successfully created.");

        volkLoadInstance(m_Instance);

        if (ShouldInitDebugger(m_EnabledInstanceLayers, m_EnabledInstanceExtensions))
        {
            VkResult res = InitializeDebugger(m_Instance);
            if (res != VK_SUCCESS)
                g_Log("Vulkan debugger was not initialized. Return code: %d\n", res);
        }

        result = SelectPhysicalDevice(params);
        if (result != VK_SUCCESS)
            return result;
        g_Log("Vulkan Physical Device was successfully created.");

        m_Queue.Initialize(m_PhysicalDevice);

        volkLoadDevice(m_Queue.GetDevice());

        m_GraphicsCommandPool = new CommandBufferPool();
        result = m_GraphicsCommandPool->Initialize(m_Queue.GetDevice());

        const auto& queueFamilyIndices = m_Queue.GetQueueFamilyIndices();
        if (queueFamilyIndices.graphicsFamily == queueFamilyIndices.computeFamily)
        {
            m_ComputeCommandPool = m_GraphicsCommandPool;
        }
        else
        {
            m_ComputeCommandPool = new CommandBufferPool();
            result = m_ComputeCommandPool->Initialize(m_Queue.GetDevice());
        }

        result = m_SubmitSyncManager.Initialize(m_Queue.GetDevice());

        return result;
    }

    VkResult Engine::Shutdown()
    {
        VkResult result;

        result = m_SubmitSyncManager.Shutdown(m_Queue.GetDevice());
        result = m_GraphicsCommandPool->Shutdown(m_Queue.GetDevice());
        if (m_GraphicsCommandPool == m_ComputeCommandPool)
        {
            m_ComputeCommandPool->Shutdown(m_Queue.GetDevice());
            delete m_ComputeCommandPool;
        }
        delete m_GraphicsCommandPool;

        result = m_Queue.ShutDown();
        DestroyDebugger(m_Instance);
        DestroyInstance();
        return result;
    }

    SubmitSync Engine::Submit(const SubmitParams* pParams, uint32_t paramsCount)
    {
        uint32_t numUniqueQueues = 1;
        if (paramsCount > 1)
        {
            std::set<VkQueue> uniqueQueues;
            for (uint32_t i = 0; i < paramsCount; i++)
            {
                uniqueQueues.insert(pParams[i].queue);
            }
            numUniqueQueues = uniqueQueues.size();
        }

        if (numUniqueQueues > 1)
        {
            // !HERE: continue here
            // submit empty queue that waits for 1 dependency and signals numUniqueueQueues dependencies

            // then do the regular submits

            // then submit empty queue that waits for numUniqueueQueues dependencies and signals one

            // we get this forking dependency thingy that allows submission to parallel queues


        }
        std::vector<VkSubmitInfo> submits;
        submits.resize(paramsCount);

        for (uint32_t i = 0; i < paramsCount; i++)
        {
            auto& si = submits[i];
            si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            si.waitSemaphoreCount = 0;
            si.pWaitSemaphores = nullptr;
            si.pWaitDstStageMask = nullptr;
            si.pSignalSemaphores = nullptr;
            si.commandBufferCount = 0;
            si.pCommandBuffers = nullptr;
        }

        // TODO: need to group by queue
        VkFence fence;
        VkResult result = vkQueueSubmit(pParams->queue, 1, submits.data(), fence);
        if (result != VK_SUCCESS)
            g_Log("Failed to submit to Queue with result: %d\n", result);

        return SubmitSync();
    }

    VkResult Engine::WaitForSubmitSync(const SubmitSync& sync, uint64_t timeout)
    {
        return VkResult();
    }

    VkResult Engine::CreateInstance(const EngineCreateParams& params)
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Imperial Engine 2";
        appInfo.applicationVersion = 1;
        appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkResult result = GetInstanceLayers(m_EnabledInstanceLayers);
        if (result != VK_SUCCESS)
            return result;

        std::vector<const char*> preferredExtensions = g_PreferredInstanceExtensions; // TODO: can use pNext to request instance extensions, though not implemented yet
        result = GetInstanceExtensions(m_EnabledInstanceLayers, preferredExtensions, m_EnabledInstanceExtensions);
        if (result != VK_SUCCESS)
            return result;

        for (const auto& layer : m_EnabledInstanceLayers)
            g_Log("Will enable layer: %s\n", layer.c_str());

        for (const auto& extension : m_EnabledInstanceExtensions)
            g_Log("Will enable instance extension: %s\n", extension.c_str());

        createInfo.enabledLayerCount = static_cast<uint32_t>(m_EnabledInstanceLayers.size());
        std::vector<const char*> layerNames(m_EnabledInstanceLayers.size());
        std::transform(m_EnabledInstanceLayers.begin(), m_EnabledInstanceLayers.end(), layerNames.begin(), [](const std::string& str) { return str.c_str(); });
        createInfo.ppEnabledLayerNames = layerNames.data();

        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_EnabledInstanceExtensions.size());
        std::vector<const char*> extensionNames(m_EnabledInstanceExtensions.size());
        std::transform(m_EnabledInstanceExtensions.begin(), m_EnabledInstanceExtensions.end(), extensionNames.begin(), [](const std::string& str) { return str.c_str(); });
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        return result;
    }

    void Engine::DestroyInstance()
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    VkResult Engine::SelectPhysicalDevice(const EngineCreateParams& params)
    {
        uint32_t deviceCount = 0;
        VkResult result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        if (deviceCount == 0)
        {
            g_Log("Error: Could not find any Vulkan Physical Devices.\n");
            return VK_ERROR_FEATURE_NOT_PRESENT;
        }

        std::vector<VkPhysicalDevice> devices { deviceCount };
        result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
        if (result != VK_SUCCESS)
            return result;

        VkPhysicalDeviceProperties props;
        for (const auto& device : devices)
        {
            vkGetPhysicalDeviceProperties(device, &props);

            // TODO: use pNext for custom selection when needed, so far selecting first available since on mobile there will likely be only one
            if (true)
            {
                m_PhysicalDevice = device;

                g_Log("Selected Physical Device: \"%s\"\n", props.deviceName);
                return VK_SUCCESS;
            }
        }

        return VK_ERROR_DEVICE_LOST;
    }
}