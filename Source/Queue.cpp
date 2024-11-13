#include "Queue.h"
#include "Layers.h"
#include "Log.h"

#include <vector>
#include <array>

namespace imp
{
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


    static std::vector<VkDeviceQueueCreateInfo> CreateQueueCreateInfos(const QueueFamilyIndices& indices, const float* priorities)
    {
        std::vector<VkDeviceQueueCreateInfo> qcis {};

        if (indices.computeFamily == indices.graphicsFamily)
        {
            VkDeviceQueueCreateInfo qci {};
            qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qci.queueCount = 2;
            qci.pQueuePriorities = priorities;
            qci.queueFamilyIndex = indices.computeFamily; // any family since they're the same

            qcis.push_back(qci);
            return qcis;
        }

        qcis.resize(2);
        for (size_t i = 0; i < 2; i++)
        {
            qcis[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qcis[i].queueCount = 1;
            qcis[i].pQueuePriorities = priorities;
        }

        qcis[0].queueFamilyIndex = indices.graphicsFamily;
        qcis[1].queueFamilyIndex = indices.computeFamily;

        return qcis;
    }

    VkResult Queue::Initialize(VkPhysicalDevice physicalDevice)
    {
        VkResult result = FindQueueFamilies(physicalDevice);
        if (result != VK_SUCCESS)
            return result;

        std::array<float, 2> priorities { 1.0f, 1.0f };
        const auto qcis = CreateQueueCreateInfos(m_QueueFamilyIndices, priorities.data());

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

        if (result != VK_SUCCESS)
            return result;

        result = AquireDeviceQueues();
        return result;
    }

    VkResult Queue::ShutDown()
    {
        vkDestroyDevice(m_Device, nullptr);
        return VK_SUCCESS;
    }

    VkResult Queue::FindQueueFamilies(VkPhysicalDevice physicalDevice)
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

    VkResult Queue::AquireDeviceQueues()
    {
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily, 0, &m_GraphicsQ);

        uint32_t computeQueueIndex = m_QueueFamilyIndices.graphicsFamily == m_QueueFamilyIndices.computeFamily ? 1 : 0;
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.computeFamily, 0, &m_ComputeQ);

        if (m_GraphicsQ == VK_NULL_HANDLE || m_ComputeQ == VK_NULL_HANDLE)
            return VK_ERROR_INITIALIZATION_FAILED;

        return VK_SUCCESS;
    }
}