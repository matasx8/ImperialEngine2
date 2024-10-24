#include "Layers.h"

VkResult imp::GetInstanceLayers(std::vector<std::string>& layers)
{
    uint32_t layerPropertyCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
        return result;
    if (layerPropertyCount > 0)
    {
        std::vector<VkLayerProperties> props{ layerPropertyCount };
        result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, props.data());
        if (result != VK_SUCCESS)
            return result;

        layers.resize(props.size());

        // Accept every layer for now
        for (size_t i = 0; i < props.size(); i++)
            layers[i] = props[i].layerName;
    }

	return result;
}

VkResult imp::GetInstanceExtensions(const std::vector<std::string>& enabledLayers, const std::vector<const char*>& preferred, std::vector<std::string>& actual)
{
    std::vector<VkExtensionProperties> props;
    uint32_t layerPropertyCount = 0;

    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &layerPropertyCount, nullptr);
    if (result != VK_SUCCESS)
        return result;

    props.resize(layerPropertyCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &layerPropertyCount, props.data());

    for (const auto& prop : props)
    {
        for (size_t i = 0; i < preferred.size(); i++)
        {
            if (strcmp(prop.extensionName, preferred[i]) == 0)
            {
                actual.push_back(prop.extensionName);
                break;
            }
        }
    }

    for (const auto& layerName : enabledLayers)
    {
        result = vkEnumerateInstanceExtensionProperties(layerName.c_str(), &layerPropertyCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        props.resize(layerPropertyCount);
        result = vkEnumerateInstanceExtensionProperties(layerName.c_str(), &layerPropertyCount, props.data());
        if (result != VK_SUCCESS)
            return result;

        for (const auto& prop : props)
        {
            for (size_t i = 0; i < preferred.size(); i++)
            {
                if (strcmp(prop.extensionName, preferred[i]) == 0)
                {
                    actual.push_back(prop.extensionName);
                    break;
                }
            }
        }
    }

    return VK_SUCCESS;
}

VkResult imp::CheckAllRequiredExtensionsSupported(VkPhysicalDevice device)
{
    std::vector<VkExtensionProperties> props;
    uint32_t extensionPropertyCount = 0;

    VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionPropertyCount, nullptr);
    if (result != VK_SUCCESS)
        return result;

    props.resize(extensionPropertyCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionPropertyCount, props.data());

    int count = 0;
    for (const auto& prop : props)
    {
        for (size_t i = 0; i < g_RequiredDeviceExtensions.size(); i++)
        {
            if (strcmp(prop.extensionName, g_RequiredDeviceExtensions[i]) == 0)
            {
                count++;
                break;
            }
        }
    }

    if (count == g_RequiredDeviceExtensions.size())
        return VK_SUCCESS;

    return VK_ERROR_INITIALIZATION_FAILED;
}
