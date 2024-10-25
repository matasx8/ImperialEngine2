#include "Queue.h"
#include "Layers.h"
#include "Log.h"

#include <vector>
#include <array>

static int GetDesiredQueue(std::vector<VkQueueFamilyProperties>& fams, VkQueueFlags desiredFlags, VkQueueFlags undesiredFlags)
{
    int i = 0;
    for (auto& fam : fams)
    {
        if (fam.queueFlags & desiredFlags && (fam.queueFlags & undesiredFlags) == 0 && fam.queueCount)
        {
            fam.queueCount--;
            return i;
        }
        i++;
    }
    return -1;
}

VkResult imp::Queue::Initialize(VkPhysicalDevice physicalDevice)
{
    VkResult result = FindQueueFamilies(physicalDevice);
    if (result != VK_SUCCESS)
        return result;

    float priority = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 2> qcis {};
    for (size_t i = 0; i < qcis.size(); i++)
    {
        qcis[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qcis[i].queueCount = 1;
        qcis[i].pQueuePriorities = &priority;
    }

    qcis[0].queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily;
    qcis[1].queueFamilyIndex = m_QueueFamilyIndices.computeFamily;

    result = CheckAllRequiredExtensionsSupported(physicalDevice);
    if (result != VK_SUCCESS)
    {
        g_Log("Error: Device does not support all required device extensions.\n");
        return result;
    }

    VkPhysicalDeviceFeatures2 features {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    auto featuresSupported = features;
    features.features.samplerAnisotropy = VK_TRUE;
    features.features.multiDrawIndirect = VK_TRUE;

    VkPhysicalDeviceVulkan11Features features11 {};
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    auto features11Supported = features11;
    features11.storageBuffer16BitAccess = VK_TRUE;
    features11.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures {};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    auto indexingFeaturesSupported = indexingFeatures;
    indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

    features11.pNext = &indexingFeatures;
    features.pNext = &features11;

    features11Supported.pNext = &indexingFeaturesSupported;
    featuresSupported.pNext = &features11Supported;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &featuresSupported);

    bool allFeaturesSupported =
        features.features.samplerAnisotropy &&
        features.features.multiDrawIndirect &&
        features11.storageBuffer16BitAccess &&
        features11.shaderDrawParameters &&
        indexingFeatures.shaderStorageBufferArrayNonUniformIndexing &&
        indexingFeatures.descriptorBindingPartiallyBound &&
        indexingFeatures.runtimeDescriptorArray &&
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing;

    // TODO: when changing implementation to allow user to supply features via pNext must provide a callback or something to
    // check if their feature is supported.

    if (!allFeaturesSupported)
        return VK_ERROR_FEATURE_NOT_PRESENT;

    VkDeviceCreateInfo dci {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = static_cast<uint32_t>(qcis.size());
    dci.pQueueCreateInfos = qcis.data();
    dci.enabledExtensionCount = static_cast<uint32_t>(g_RequiredDeviceExtensions.size());
    dci.ppEnabledExtensionNames = g_RequiredDeviceExtensions.data();

    result = vkCreateDevice(physicalDevice, &dci, nullptr, &m_Device);
}

VkResult imp::Queue::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());

    m_QueueFamilyIndices.graphicsFamily = GetDesiredQueue(queueFamilyList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0);
    if (m_QueueFamilyIndices.graphicsFamily == -1)
        return VK_ERROR_INITIALIZATION_FAILED;

    m_QueueFamilyIndices.computeFamily = GetDesiredQueue(queueFamilyList, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (m_QueueFamilyIndices.computeFamily != -1)
        return VK_SUCCESS;

    m_QueueFamilyIndices.computeFamily = GetDesiredQueue(queueFamilyList, VK_QUEUE_COMPUTE_BIT, 0);
    if (m_QueueFamilyIndices.computeFamily == 1)
        return VK_ERROR_INITIALIZATION_FAILED;

    return VK_SUCCESS;
}
