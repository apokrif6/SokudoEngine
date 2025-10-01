#include <string>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "UserInterface.h"
#include "core/events/input-events/KeyEvent.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"
#include "core/ui/ProfilingUIWindow.h"
#include "core/ui/CameraUIWindow.h"
#include "core/ui/SceneUIWindow.h"
#include "core/ui/RenderingUIWindow.h"

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

    if (ImGui::BeginTabBar("Tabs"))
    {
        Core::UI::ProfilingUIWindow::getBody();
        Core::UI::CameraUIWindow::getBody();
        Core::UI::RenderingUIWindow::getBody();
        Core::UI::SceneUIWindow::getBody();

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

void Core::Renderer::UserInterface::onEvent(const Event& event)
{
    if (const auto* keyEvent = dynamic_cast<const KeyEvent*>(&event))
    {
        if (keyEvent->action == GLFW_PRESS)
        {
            switch (keyEvent->key)
            {
            case GLFW_KEY_W:
                UI::SceneUIWindow::setSceneObjectManipulationOperation(ImGuizmo::TRANSLATE);
                break;
            case GLFW_KEY_E:
                UI::SceneUIWindow::setSceneObjectManipulationOperation(ImGuizmo::SCALE);
                break;
            case GLFW_KEY_R:
                UI::SceneUIWindow::setSceneObjectManipulationOperation(ImGuizmo::ROTATE);
                break;
            default:
                break;
            }
        }
    }
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
