#pragma once
#include "volk.h"

#include <vector>
#include <string>

namespace imp
{
	inline static constexpr std::initializer_list<const char*> g_PreferredInstanceExtensions {"VK_EXT_debug_utils"};

	VkResult GetInstanceLayers(std::vector<std::string>& layers);
	VkResult GetInstanceExtensions(const std::vector<std::string>& enabledLayers, const std::vector<const char*>& preferred, std::vector<std::string>& actual);
}