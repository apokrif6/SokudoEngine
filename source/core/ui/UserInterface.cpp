#include <string>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "UserInterface.h"
#include "core/events/input-events/KeyEvent.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"
#include "core/ui/ProfilingUIWindow.h"
#include "core/ui/SceneUIWindow.h"
#include "imgui_internal.h"
#include "inspector/InspectorUIWindow.h"
#include "MiscUIWindow.h"
#include "ViewportUIWindow.h"

bool Core::Renderer::UserInterface::init(VkRenderData& renderData)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

    if (!CommandBuffer::init(renderData, imguiCommandBuffer))
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
    CommandBuffer::cleanup(renderData, imguiCommandBuffer);

    ImGui::StyleColorsDark();

    return true;
}

void Core::Renderer::UserInterface::update(VkRenderData& renderData, float deltaTime)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiID dockspaceID = ImGui::GetID("SokudoEngineDockspace");

    if (!ImGui::DockBuilderGetNode(dockspaceID))
    {
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

        ImGuiID dockLeft, dockRight;
        ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.25f, &dockLeft, &dockRight);

        ImGuiID dockLeftBottom, dockLeftTop;
        ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.35f, &dockLeftBottom, &dockLeftTop);

        ImGuiID dockBottom, dockTop;
        ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.25f, &dockBottom, &dockTop);

        ImGuiID dockRightMisc, dockCenterViewport;
        ImGui::DockBuilderSplitNode(dockTop, ImGuiDir_Right, 0.25f, &dockRightMisc, &dockCenterViewport);

        ImGui::DockBuilderDockWindow("Scene", dockLeftTop);
        ImGui::DockBuilderDockWindow("Inspector", dockLeftBottom);
        ImGui::DockBuilderDockWindow("Profiling", dockBottom);
        ImGui::DockBuilderDockWindow("Viewport", dockCenterViewport);
        ImGui::DockBuilderDockWindow("Misc", dockRightMisc);

        ImGui::DockBuilderFinish(dockspaceID);
    }

    ImGui::DockSpaceOverViewport(dockspaceID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    setupImGuiStyle();

    UI::SceneUIWindow::getBody();
    UI::InspectorUIWindow::getBody();
    UI::ProfilingUIWindow::getBody();
    UI::MiscUIWindow::getBody();
    UI::ViewportUIWindow::getBody();

    ImGui::EndFrame();
}

void Core::Renderer::UserInterface::draw(VkRenderData& renderData)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderData.rdCommandBuffer);
}

void Core::Renderer::UserInterface::cleanup(VkRenderData& renderData)
{
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
                UI::ViewportUIWindow::setSceneObjectManipulationOperation(ImGuizmo::TRANSLATE);
                break;
            case GLFW_KEY_E:
                UI::ViewportUIWindow::setSceneObjectManipulationOperation(ImGuizmo::SCALE);
                break;
            case GLFW_KEY_R:
                UI::ViewportUIWindow::setSceneObjectManipulationOperation(ImGuizmo::ROTATE);
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
    style.Alpha = 1.f;
    style.WindowRounding = 6.f;
    style.ChildRounding = 6.f;
    style.FrameRounding = 4.f;
    style.PopupRounding = 6.f;
    style.ScrollbarRounding = 9.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 6.f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

    ImVec4* colors = style.Colors;

    const ImVec4 voidBackground = ImVec4(0.04f, 0.f, 0.12f, 1.f);
    const ImVec4 midnightPurple = ImVec4(0.06f, 0.01f, 0.15f, 0.98f);
    const ImVec4 electricViolet = ImVec4(0.45f, 0.05f, 0.75f, 1.f);
    const ImVec4 electricVioletLight = ImVec4(0.55f, 0.15f, 0.85f, 1.f);
    const ImVec4 electricVioletBright = ImVec4(0.65f, 0.2f, 0.95f, 1.f);
    const ImVec4 textWhite = ImVec4(0.95f, 0.95f, 1.f, 1.f);
    const ImVec4 textDisabled = ImVec4(0.5f, 0.4f, 0.6f, 1.f);

    colors[ImGuiCol_WindowBg] = voidBackground;
    colors[ImGuiCol_ChildBg] = voidBackground;
    colors[ImGuiCol_PopupBg] = midnightPurple;

    colors[ImGuiCol_Text] = textWhite;
    colors[ImGuiCol_TextDisabled] = textDisabled;

    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.05f, 0.45f, 1.f);
    colors[ImGuiCol_ButtonHovered] = electricViolet;
    colors[ImGuiCol_ButtonActive] = electricVioletBright;

    colors[ImGuiCol_Header] = electricViolet;
    colors[ImGuiCol_HeaderHovered] = electricVioletLight;
    colors[ImGuiCol_HeaderActive] = electricVioletBright;

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.05f, 0.45f, 1.f);
    colors[ImGuiCol_TabHovered] = electricVioletLight;
    colors[ImGuiCol_TabActive] = electricViolet;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.04f, 0.f, 0.1f, 1.f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.25f, 0.05f, 0.45f, 1.f);

    colors[ImGuiCol_TitleBg] = voidBackground;
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.02f, 0.18f, 1.f);
    colors[ImGuiCol_TitleBgCollapsed] = voidBackground;

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.f, 0.08f, 0.f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.05f, 0.6f, 1.f);
    colors[ImGuiCol_ScrollbarGrabHovered] = electricVioletLight;
    colors[ImGuiCol_ScrollbarGrabActive] = electricVioletBright;

    colors[ImGuiCol_CheckMark] = ImVec4(0.9f, 0.6f, 1.f, 1.f);

    colors[ImGuiCol_SliderGrab] = electricViolet;
    colors[ImGuiCol_SliderGrabActive] = electricVioletBright;

    colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.1f, 0.5f, 1.f);
    colors[ImGuiCol_SeparatorHovered] = electricVioletLight;
    colors[ImGuiCol_SeparatorActive] = electricVioletBright;

    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.02f, 0.22f, 1.f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.05f, 0.35f, 1.f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.1f, 0.5f, 1.f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.05f, 0.75f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = electricVioletLight;
    colors[ImGuiCol_ResizeGripActive] = electricVioletBright;

    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.1f, 0.5f, 0.5f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
    
    colors[ImGuiCol_DockingPreview] = ImVec4(0.45f, 0.05f, 0.75f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg] = voidBackground;

    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.f, 0.43f, 0.35f, 1.f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 0.7f, 0.f, 1.f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.f, 0.6f, 0.f, 1.f);
    
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.2f, 1.f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.f, 1.f, 1.f, 0.06f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45f, 0.05f, 0.75f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.f, 1.f, 0.f, 0.9f);
    colors[ImGuiCol_NavHighlight] = electricViolet;
    colors[ImGuiCol_NavWindowingHighlight] = textWhite;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}