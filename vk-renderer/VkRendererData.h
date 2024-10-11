#pragma once

#include "vk_mem_alloc.h"
#include "VkBootstrap.h"

struct VkRendererData
{
    VmaAllocator rdAllocator;
    vkb::Instance rdVkbInstance{};
    vkb::Device rdVkbDevice{};
    vkb::Swapchain rdVkbSwapchain{};

    std::vector<VkImage> rdSwapchainImages;
    std::vector<VkImageView> rdSwapchainImageViews;
    std::vector<VkFramebuffer> rdFramebuffers;
};
