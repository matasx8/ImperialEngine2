#pragma once
#include "volk.h"

#include <deque>

namespace
{
    struct SubmitSync
    {
        uint64_t submit;
        VkSemaphore semaphore;
        VkFence fence;
    };

    class SubmitSyncManager
    {
    public:

        SubmitSyncManager() = default;

        VkResult Initialize(VkDevice device);
        VkResult Shutdown(VkDevice device);

        SubmitSync GetSubmitSync();


    private:

        std::deque<SubmitSync> m_Syncs;
    };
}