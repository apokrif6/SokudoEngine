#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "UserInterface.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"
#include "core/animations/Animator.h"
#include "core/engine/Engine.h"


bool Core::Renderer::UserInterface::init(VkRenderData& renderData)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    VkDescriptorPoolSize imguiPoolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                             {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                             {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo imguiPoolInfo{};
    imguiPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    imguiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    imguiPoolInfo.maxSets = 1000;
    imguiPoolInfo.poolSizeCount = std::size(imguiPoolSizes);
    imguiPoolInfo.pPoolSizes = imguiPoolSizes;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &imguiPoolInfo, nullptr,
                               &renderData.rdImguiDescriptorPool))
    {
        Logger::log(1, "%s error: could not init ImGui descriptor pool \n", __FUNCTION__);
        return false;
    }

    ImGui_ImplGlfw_InitForVulkan(renderData.rdWindow, true);

    ImGui_ImplVulkan_InitInfo imguiInitInfo{};
    imguiInitInfo.Instance = renderData.rdVkbInstance.instance;
    imguiInitInfo.PhysicalDevice = renderData.rdVkbDevice.physical_device;
    imguiInitInfo.Device = renderData.rdVkbDevice.device;
    imguiInitInfo.Queue = renderData.rdGraphicsQueue;
    imguiInitInfo.DescriptorPool = renderData.rdImguiDescriptorPool;
    imguiInitInfo.RenderPass = renderData.rdRenderpass;
    imguiInitInfo.MinImageCount = 2;
    imguiInitInfo.ImageCount = renderData.rdSwapchainImages.size();
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&imguiInitInfo);

    VkCommandBuffer imguiCommandBuffer;

    if (!Core::Renderer::CommandBuffer::init(renderData, imguiCommandBuffer))
    {
        Logger::log(1, "%s error: could not create texture upload command buffers\n", __FUNCTION__);
        return false;
    }

    if (vkResetCommandBuffer(imguiCommandBuffer, 0) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to reset imgui command buffer\n", __FUNCTION__);
        return false;
    }

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(imguiCommandBuffer, &cmdBeginInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to begin imgui command buffer\n", __FUNCTION__);
        return false;
    }

    ImGui_ImplVulkan_CreateFontsTexture();

    if (vkEndCommandBuffer(imguiCommandBuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to end staging command buffer\n", __FUNCTION__);
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &imguiCommandBuffer;

    VkFence imguiBufferFence;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(renderData.rdVkbDevice.device, &fenceInfo, nullptr, &imguiBufferFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to imgui buffer fence\n", __FUNCTION__);
        return false;
    }

    if (vkResetFences(renderData.rdVkbDevice.device, 1, &imguiBufferFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: imgui buffer fence reset failed", __FUNCTION__);
        return false;
    }

    if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, imguiBufferFence) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to imgui init command buffer\n", __FUNCTION__);
        return false;
    }

    if (vkWaitForFences(renderData.rdVkbDevice.device, 1, &imguiBufferFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: waiting for imgui init fence failed", __FUNCTION__);
        return false;
    }

    vkDestroyFence(renderData.rdVkbDevice.device, imguiBufferFence, nullptr);
    Core::Renderer::CommandBuffer::cleanup(renderData, imguiCommandBuffer);

    ImGui::StyleColorsDark();

    return true;
}

void Core::Renderer::UserInterface::update(VkRenderData& renderData)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags imguiWindowFlags = 0;
    imguiWindowFlags |= ImGuiWindowFlags_NoCollapse;
    imguiWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

    setupImGuiStyle();

    ImGui::Begin("Sokudo Engine", nullptr, imguiWindowFlags);

    static float newFps = 0.0f;
    if (renderData.rdFrameTime > 0.0)
    {
        newFps = 1.0f / renderData.rdFrameTime * 1000.f;
    }

    mFramesPerSecond = (mAveragingAlpha * mFramesPerSecond) + (1.0f - mAveragingAlpha) * newFps;

    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("Profiling"))
        {
            ImGui::Text("Frames per second:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());

            ImGui::Text("Frame Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");

            ImGui::Text("Matrix Generation Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdMatrixGenerateTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");

            ImGui::Text("Matrix Upload Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdUploadToUBOTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Camera"))
        {
            ImGui::Text("Camera Position:");
            ImGui::SameLine();
            ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

            ImGui::Text("View Yaw:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdViewYaw).c_str());

            ImGui::Text("View Pitch:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdViewPitch).c_str());

            std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
            ImGui::Text("Window Dimensions:");
            ImGui::SameLine();
            ImGui::Text("%s", windowDims.c_str());

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene"))
        {
            ImGui::Text("Field Of View");
            ImGui::SameLine();
            ImGui::SliderInt("FOV", &renderData.rdFieldOfView, 40, 150);

            ImGui::Checkbox("Draw World Coordinate Arrows", &renderData.rdDrawWorldCoordinateArrows);
            ImGui::Checkbox("Draw Model Coordinate Arrows", &renderData.rdDrawModelCoordinateArrows);

            if (ImGui::Button("Reset Rotation"))
            {
                renderData.rdResetAngles = true;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::Text("X Rotation");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SliderInt("##ROTX", &renderData.rdRotXAngle, 0, 360);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Y Rotation");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SliderInt("##ROTY", &renderData.rdRotYAngle, 0, 360);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
            ImGui::Text("Z Rotation");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::SliderInt("##ROTZ", &renderData.rdRotZAngle, 0, 360);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Animation"))
        {
            ImGui::Checkbox("Should play animation", &renderData.shouldPlayAnimation);
            ImGui::Checkbox("Should draw debug skeleton", &renderData.shouldDrawDebugSkeleton);

            std::vector<std::string>& loadedAnimations =
                Core::Engine::getInstance().getSystem<Animations::Animator>()->loadedAnimations;
            if (ImGui::BeginCombo("##Loaded animations",
                                  loadedAnimations[renderData.selectedAnimationIndexToPlay].c_str(),
                                  ImGuiComboFlags_WidthFitPreview))
            {
                for (int i = 0; i < loadedAnimations.size(); ++i)
                {
                    const bool isAnimationToPlaySelected = (renderData.selectedAnimationIndexToPlay == i);
                    if (ImGui::Selectable(loadedAnimations[i].c_str(), isAnimationToPlaySelected))
                    {
                        renderData.selectedAnimationIndexToPlay = i;
                    }

                    if (isAnimationToPlaySelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Core::Renderer::UserInterface::draw(VkRenderData& renderData)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderData.rdCommandBuffer);
}

void Core::Renderer::UserInterface::cleanup(VkRenderData& renderData)
{
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdImguiDescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Core::Renderer::UserInterface::setupImGuiStyle() const
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 0.8f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.05f, 0.15f, 1.f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.04f, 0.12f, 1.f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.06f, 0.18f, 0.95f);

    style.Colors[ImGuiCol_Border] = ImVec4(0.35f, 0.1f, 0.6f, 0.6f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.85f, 1.f, 1.f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.4f, 0.6f, 1.f);

    style.Colors[ImGuiCol_Header] = ImVec4(0.45f, 0.2f, 0.65f, 0.8f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.6f, 0.3f, 0.8f, 0.9f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.7f, 0.35f, 0.9f, 1.f);

    style.Colors[ImGuiCol_Button] = ImVec4(0.45f, 0.2f, 0.65f, 0.6f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.3f, 0.8f, 0.85f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.35f, 0.9f, 1.f);

    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.1f, 0.3f, 1.f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.15f, 0.55f, 1.f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.2f, 0.6f, 1.f);

    style.Colors[ImGuiCol_Tab] = ImVec4(0.35f, 0.15f, 0.5f, 0.8f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.55f, 0.25f, 0.75f, 0.9f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.65f, 0.3f, 0.85f, 1.f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.2f, 0.1f, 0.3f, 0.8f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.3f, 0.15f, 0.45f, 0.9f);

    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.25f, 0.1f, 0.4f, 1.f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.4f, 0.2f, 0.6f, 1.f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.05f, 0.25f, 0.75f);

    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.04f, 0.12f, 1.f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.4f, 0.2f, 0.6f, 0.6f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.25f, 0.75f, 0.8f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.7f, 0.35f, 0.9f, 1.f);

    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.5f, 0.25f, 0.75f, 0.5f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.65f, 0.3f, 0.85f, 0.7f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.75f, 0.35f, 0.95f, 0.9f);

    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6f, 0.3f, 0.8f, 0.9f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.35f, 0.95f, 1.f);

    style.Colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.15f, 0.6f, 0.6f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.5f, 0.25f, 0.75f, 0.8f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.3f, 0.85f, 1.f);
}
