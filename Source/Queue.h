#pragma once
#include "volk.h"

namespace imp
{
	struct QueueFamilyIndices
	{
		int graphicsFamily;
		int computeFamily;
	};

	class Queue
	{
	public:
		Queue() = default;

		VkResult Initialize(VkPhysicalDevice physicalDevice);
		VkResult ShutDown();

		inline VkDevice GetDevice() const { return m_Device; }
		inline const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

	private:

		VkResult FindQueueFamilies(VkPhysicalDevice physicalDevice);
		VkResult AquireDeviceQueues();

		VkQueue m_GraphicsQ = VK_NULL_HANDLE;
		VkQueue m_ComputeQ = VK_NULL_HANDLE;

		QueueFamilyIndices m_QueueFamilyIndices = {};

		VkDevice m_Device = VK_NULL_HANDLE;
	};
}