#include "Engine.h"
#include "Layers.h"

#include <vector>
#include <algorithm>
#include <iterator>

namespace imp
{
    Engine::Engine()
    {
    }

    VkResult Engine::InitializeEngine(const EngineCreateParams& params)
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

        result = SelectPhysicalDevice(params);
        if (result != VK_SUCCESS)
            return result;
        g_Log("Vulkan Physical Device was successfully created.");

        return result;
    }

    VkResult Engine::ShutdownEngine()
    {
        DestroyInstance();
        return VK_SUCCESS;
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

        std::vector<std::string> layers;
        VkResult result = GetInstanceLayers(layers);
        if (result != VK_SUCCESS)
            return result;

        std::vector<std::string> actualExtensions;
        std::vector<const char*> preferredExtensions = g_PreferredInstanceExtensions; // TODO: can use pNext to request instance extensions, though not implemented yet
        result = GetInstanceExtensions(layers, preferredExtensions, actualExtensions);
        if (result != VK_SUCCESS)
            return result;

        for (const auto& layer : layers)
            g_Log("Will enable layer: %s\n", layer.c_str());

        for (const auto& extension : actualExtensions)
            g_Log("Will enable instance extension: %s\n", extension.c_str());

        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        std::vector<const char*> layerNames(layers.size());
        std::transform(layers.begin(), layers.end(), layerNames.begin(), [](const std::string& str) { return str.c_str(); });
        createInfo.ppEnabledLayerNames = layerNames.data();

        createInfo.enabledExtensionCount = static_cast<uint32_t>(actualExtensions.size());
        std::vector<const char*> extensionNames(actualExtensions.size());
        std::transform(actualExtensions.begin(), actualExtensions.end(), extensionNames.begin(), [](const std::string& str) { return str.c_str(); });
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