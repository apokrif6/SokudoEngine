#include <string>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "UserInterface.h"
#include "core/events/input-events/KeyEvent.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"
#include "ui/ProfilingUIWindow.h"
#include "ui/SceneUIWindow.h"
#include "imgui_internal.h"
#include "inspector/InspectorUIWindow.h"
#include "MiscUIWindow.h"
#include "ViewportUIWindow.h"
#include "core/tools/ColorConverter.h"

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
        ImGui::DockBuilderDockWindow("Inspector", dockRightMisc);
        ImGui::DockBuilderDockWindow("Profiling", dockBottom);
        ImGui::DockBuilderDockWindow("Viewport", dockCenterViewport);
        ImGui::DockBuilderDockWindow("Misc", dockLeftBottom);

        ImGui::DockBuilderFinish(dockspaceID);
    }

    ImGui::DockSpaceOverViewport(dockspaceID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    setupImGuiStyle();

    UI::SceneUIWindow::renderBody();
    UI::InspectorUIWindow::renderBody();
    UI::ProfilingUIWindow::renderBody();
    UI::MiscUIWindow::renderBody();
    UI::ViewportUIWindow::renderBody();

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
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdImguiDescriptorPool, nullptr);
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
    style.WindowRounding = 8.f;
    style.ChildRounding = 6.f;
    style.FrameRounding = 4.f;
    style.PopupRounding = 6.f;
    style.GrabRounding = 12.f;
    style.TabRounding = 4.f;
    style.ScrollbarSize = 12.f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.FrameBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    ImVec4* colors = style.Colors;

    const auto deepBackground = Tools::ColorConverter::toLinear(ImVec4(0.12f, 0.12f, 0.12f, 1.f));
    const auto panelBackground = Tools::ColorConverter::toLinear(ImVec4(0.16f, 0.16f, 0.17f, 1.f));
    const auto inputBackground = Tools::ColorConverter::toLinear(ImVec4(0.20f, 0.20f, 0.22f, 1.f));

    const auto violetDark = Tools::ColorConverter::toLinear(ImVec4(0.4f, 0.2f, 0.65f, 1.f));
    const auto violetMain = Tools::ColorConverter::toLinear(ImVec4(0.58f, 0.3f, 0.88f, 1.f));
    const auto violetNeon = Tools::ColorConverter::toLinear(ImVec4(0.75f, 0.5f, 1.f, 1.f));

    const auto textWhite = Tools::ColorConverter::toLinear(ImVec4(0.9f, 0.9f, 0.92f, 1.f));
    const auto textGrey = Tools::ColorConverter::toLinear(ImVec4(0.55f, 0.55f, 0.58f, 1.f));
    const auto border = Tools::ColorConverter::toLinear(ImVec4(0.25f, 0.25f, 0.27f, 0.6f));

    colors[ImGuiCol_Text] = textWhite;
    colors[ImGuiCol_TextDisabled] = textGrey;

    colors[ImGuiCol_WindowBg] = deepBackground;
    colors[ImGuiCol_ChildBg] = panelBackground;
    colors[ImGuiCol_PopupBg] = panelBackground;

    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);

    colors[ImGuiCol_FrameBg] = inputBackground;
    colors[ImGuiCol_FrameBgHovered] = Tools::ColorConverter::toLinear(ImVec4(0.24f, 0.24f, 0.27f, 1.0f));
    colors[ImGuiCol_FrameBgActive] = Tools::ColorConverter::toLinear(ImVec4(0.28f, 0.28f, 0.32f, 1.0f));

    colors[ImGuiCol_TitleBg] = deepBackground;
    colors[ImGuiCol_TitleBgActive] = deepBackground;
    colors[ImGuiCol_TitleBgCollapsed] = deepBackground;

    colors[ImGuiCol_MenuBarBg] = deepBackground;

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = border;
    colors[ImGuiCol_ScrollbarGrabHovered] = textGrey;
    colors[ImGuiCol_ScrollbarGrabActive] = violetMain;

    colors[ImGuiCol_CheckMark] = violetNeon;
    colors[ImGuiCol_SliderGrab] = violetDark;
    colors[ImGuiCol_SliderGrabActive] = violetMain;

    colors[ImGuiCol_Button] = violetDark;
    colors[ImGuiCol_ButtonHovered] = violetMain;
    colors[ImGuiCol_ButtonActive] = violetNeon;

    colors[ImGuiCol_Header] = violetDark;
    colors[ImGuiCol_HeaderHovered] = violetMain;
    colors[ImGuiCol_HeaderActive] = violetNeon;

    colors[ImGuiCol_Separator] = border;
    colors[ImGuiCol_SeparatorHovered] = violetDark;
    colors[ImGuiCol_SeparatorActive] = violetMain;

    colors[ImGuiCol_Tab] = deepBackground;
    colors[ImGuiCol_TabHovered] = violetDark;
    colors[ImGuiCol_TabActive] = panelBackground;
    colors[ImGuiCol_TabUnfocused] = deepBackground;
    colors[ImGuiCol_TabUnfocusedActive] = panelBackground;

    colors[ImGuiCol_DockingPreview] = ImVec4(violetMain.x, violetMain.y, violetMain.z, 0.4f);
    colors[ImGuiCol_DockingEmptyBg] = deepBackground;

    colors[ImGuiCol_TableHeaderBg] = panelBackground;
    colors[ImGuiCol_TableBorderStrong] = border;
    colors[ImGuiCol_TableBorderLight] = Tools::ColorConverter::toLinear(ImVec4(0.2f, 0.2f, 0.22f, 1.f));
    colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt] = Tools::ColorConverter::toLinear(ImVec4(1.f, 1.f, 1.f, 0.03f));

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.f, 0.f, 0.f, 0.f);
    colors[ImGuiCol_ResizeGripHovered] = violetDark;
    colors[ImGuiCol_ResizeGripActive] = violetNeon;

    colors[ImGuiCol_PlotLines] = violetMain;
    colors[ImGuiCol_PlotLinesHovered] = violetNeon;
    colors[ImGuiCol_PlotHistogram] = violetMain;
    colors[ImGuiCol_PlotHistogramHovered] = violetNeon;

    colors[ImGuiCol_NavHighlight] = violetMain;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(violetMain.x, violetMain.y, violetMain.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = violetNeon;
}