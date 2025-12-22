#include "Skeleton.h"
#include "core/vk-renderer/buffers/VertexBuffer.h"

void Core::Renderer::Debug::Skeleton::init(Core::Renderer::VkRenderData& renderData) { createVertexBuffer(renderData); }

void Core::Renderer::Debug::Skeleton::createVertexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::init(renderData, mDebugLinesBuffer, sizeof(LineVertex) * MAX_DEBUG_BONES_LINES,
                                       "DebugSkeleton");
}

void Core::Renderer::Debug::Skeleton::update(Core::Renderer::VkRenderData& renderData,
                                             const std::vector<DebugBone>& bones)
{
    std::vector<LineVertex> lineVertices;
    for (const auto& bone : bones)
    {
        lineVertices.push_back({bone.start, glm::vec3(1, 1, 0)});
        lineVertices.push_back({bone.end, glm::vec3(1, 1, 0)});
    }
    mDebugLinesCount = static_cast<uint32_t>(bones.size() * 2);
    Core::Renderer::VertexBuffer::uploadData(renderData, mDebugLinesBuffer, lineVertices);
}

void Core::Renderer::Debug::Skeleton::draw(Core::Renderer::VkRenderData& renderData)
{
    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdDebugSkeletonPipelineLayout, 0, 1,
                            &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1, &mDebugLinesBuffer.rdVertexBuffer, &offset);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdDebugSkeletonPipeline);
    vkCmdSetLineWidth(renderData.rdCommandBuffer, 3.0f);

    vkCmdDraw(renderData.rdCommandBuffer, mDebugLinesCount, 1, 0, 0);
}

void Core::Renderer::Debug::Skeleton::cleanup(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::cleanup(renderData, mDebugLinesBuffer);
}
