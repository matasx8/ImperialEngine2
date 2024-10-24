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
    if (m_QueueFamilyIndices.computeFamily != 1)
        return VK_ERROR_INITIALIZATION_FAILED;

    return VK_SUCCESS;
}
