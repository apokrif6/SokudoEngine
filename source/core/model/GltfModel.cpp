#define GLM_ENABLE_EXPERIMENTAL
#include "GltfModel.h"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/vk-renderer/buffers/IndexBuffer.h"
#include "core/tools/Logger.h"
#include "stb_image.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

bool Core::Model::GltfModel::loadModel(Core::Renderer::VkRenderData& renderData,
                                       Core::Renderer::VkGltfRenderData& gltfRenderData,
                                       const std::string& modelFilename, const std::string& textureFilename)
{
    std::future<bool> textureLoadFuture =
        Core::Renderer::Texture::loadTexture(renderData, gltfRenderData.rdGltfModelTexture, textureFilename);
    if (!textureLoadFuture.get())
    {
        Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
        return false;
    }
    Logger::log(1, "%s: glTF model texture '%s' successfully loaded\n", __FUNCTION__, modelFilename.c_str());

    mModel = std::make_shared<tinygltf::Model>();

    tinygltf::TinyGLTF gltfLoader;
    std::string loaderErrors;
    std::string loaderWarnings;
    bool result = gltfLoader.LoadASCIIFromFile(mModel.get(), &loaderErrors, &loaderWarnings, modelFilename);

    if (!loaderWarnings.empty())
    {
        Logger::log(1, "%s: warnings while loading glTF model:\n%s\n", __FUNCTION__, loaderWarnings.c_str());
    }

    if (!loaderErrors.empty())
    {
        Logger::log(1, "%s: errors while loading glTF model:\n%s\n", __FUNCTION__, loaderErrors.c_str());
    }

    if (!result)
    {
        perror("Error");
        Logger::log(1, "%s error: could not load file '%s', because of '%s'\n", __FUNCTION__, textureFilename.c_str(),
                    stbi_failure_reason());
        return false;
    }

    createVertexBuffers(renderData, gltfRenderData);
    createIndexBuffer(renderData, gltfRenderData);

    getJointData();
    getWeightData();
    getInvBindMatrices();

    int nodeCount = static_cast<int>(mModel->nodes.size());
    int rootNode = mModel->scenes.at(0).nodes.at(0);
    Logger::log(1, "%s: model has %i nodes, root node is %i\n", __FUNCTION__, nodeCount, rootNode);

    mRootNode = GltfNode::createRoot(rootNode);
    getNodeData(mRootNode, glm::mat4(1.0f));
    getNodes(mRootNode);

    mSkeletonMesh = std::make_shared<Core::Renderer::VkMesh>();

    mRootNode->printTree();

    renderData.rdGltfTriangleCount = getTriangleCount();

    return true;
}

void Core::Model::GltfModel::getJointData()
{
    std::string jointsAccessorAttrib = "JOINTS_0";
    int jointsAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(jointsAccessorAttrib);
    Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, jointsAccessor, jointsAccessorAttrib.c_str());

    const tinygltf::Accessor& accessor = mModel->accessors.at(jointsAccessor);
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    int jointVecSize = static_cast<int>(accessor.count);
    Logger::log(1, "%s: %i short vec4 in JOINTS_0\n", __FUNCTION__, jointVecSize);
    mJointVec.resize(jointVecSize);

    std::memcpy(mJointVec.data(), &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);

    mNodeToJoint.resize(mModel->nodes.size());

    const tinygltf::Skin& skin = mModel->skins.at(0);
    for (size_t i = 0; i < skin.joints.size(); ++i)
    {
        int destinationNode = skin.joints.at(i);
        mNodeToJoint.at(destinationNode) = static_cast<int>(i);
        Logger::log(2, "%s: joint %i affects node %i\n", __FUNCTION__, i, destinationNode);
    }
}

void Core::Model::GltfModel::getWeightData()
{
    std::string weightsAccessorAttrib = "WEIGHTS_0";
    int weightAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(weightsAccessorAttrib);
    Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, weightAccessor, weightsAccessorAttrib.c_str());

    const tinygltf::Accessor& accessor = mModel->accessors.at(weightAccessor);
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    int weightVecSize = static_cast<int>(accessor.count);
    Logger::log(1, "%s: %i vec4 in WEIGHTS_0\n", __FUNCTION__, weightVecSize);
    mWeightVec.resize(weightVecSize);

    std::memcpy(mWeightVec.data(), &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
}

void Core::Model::GltfModel::getInvBindMatrices()
{
    const tinygltf::Skin& skin = mModel->skins.at(0);
    int invBindMatAccessor = skin.inverseBindMatrices;

    const tinygltf::Accessor& accessor = mModel->accessors.at(invBindMatAccessor);
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    mInverseBindMatrices.resize(skin.joints.size());
    mJointMatrices.resize(skin.joints.size());

    std::memcpy(mInverseBindMatrices.data(), &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
}

std::shared_ptr<Core::Renderer::VkMesh> Core::Model::GltfModel::getSkeleton(bool enableSkinning)
{
    mSkeletonMesh->vertices.resize(mModel->nodes.size() * 2);
    mSkeletonMesh->vertices.clear();

    /* start from Armature child */
    getSkeletonPerNode(mRootNode->getChildren().at(0), enableSkinning);
    return mSkeletonMesh;
}

void Core::Model::GltfModel::getSkeletonPerNode(const std::shared_ptr<GltfNode>& treeNode, bool enableSkinning)
{
    auto parentPos = glm::vec3(0.0f);
    if (enableSkinning)
    {
        parentPos = glm::vec3(treeNode->getNodeMatrix() * glm::vec4(1.0f));
    }
    else
    {
        glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(treeNode->getNodeNum())));
        parentPos = bindMatrix * treeNode->getNodeMatrix() * glm::vec4(1.0f);
    }
    Core::Renderer::VkVertex parentVertex{};
    parentVertex.position = parentPos;
    parentVertex.color = glm::vec3(0.0f, 1.0f, 1.0f);

    for (const auto& childNode : treeNode->getChildren())
    {
        auto childPos = glm::vec3(0.0f);
        if (enableSkinning)
        {
            childPos = glm::vec3(childNode->getNodeMatrix() * glm::vec4(1.0f));
        }
        else
        {
            glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(childNode->getNodeNum())));
            childPos = bindMatrix * childNode->getNodeMatrix() * glm::vec4(1.0f);
        }
        Core::Renderer::VkVertex childVertex{};
        childVertex.position = childPos;
        childVertex.color = glm::vec3(0.0f, 0.0f, 1.0f);
        mSkeletonMesh->vertices.emplace_back(parentVertex);
        mSkeletonMesh->vertices.emplace_back(childVertex);

        getSkeletonPerNode(childNode, enableSkinning);
    }
}

void Core::Model::GltfModel::getNodes(const std::shared_ptr<GltfNode>& treeNode)
{
    int nodeNum = treeNode->getNodeNum();
    std::vector<int> childNodes = mModel->nodes.at(nodeNum).children;

    /* remove the child node with skin/mesh metadata */
    auto removeIt =
        std::remove_if(childNodes.begin(), childNodes.end(), [&](int num) { return mModel->nodes.at(num).skin != -1; });
    childNodes.erase(removeIt, childNodes.end());

    treeNode->addChildren(childNodes);
    glm::mat4 treeNodeMatrix = treeNode->getNodeMatrix();

    for (auto& childNode : treeNode->getChildren())
    {
        getNodeData(childNode, treeNodeMatrix);
        getNodes(childNode);
    }
}

void Core::Model::GltfModel::getNodeData(const std::shared_ptr<GltfNode>& treeNode, const glm::mat4& parentNodeMatrix)
{
    int nodeNum = treeNode->getNodeNum();
    const tinygltf::Node& node = mModel->nodes.at(nodeNum);
    treeNode->setNodeName(node.name);

    if (!node.translation.empty())
    {
        treeNode->setTranslation(glm::make_vec3(node.translation.data()));
    }
    if (!node.rotation.empty())
    {
        treeNode->setRotation(glm::make_quat(node.rotation.data()));
    }
    if (!node.scale.empty())
    {
        treeNode->setScale(glm::make_vec3(node.scale.data()));
    }

    treeNode->calculateLocalTRSMatrix();
    treeNode->calculateNodeMatrix(parentNodeMatrix);

    mJointMatrices.at(mNodeToJoint.at(nodeNum)) =
        treeNode->getNodeMatrix() * mInverseBindMatrices.at(mNodeToJoint.at(nodeNum));
}

void Core::Model::GltfModel::draw(const Core::Renderer::VkRenderData& renderData,
                                  const Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    /* texture */
    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdPipelineLayout, 0, 1,
                            &gltfRenderData.rdGltfModelTexture.texTextureDescriptorSet, 0, nullptr);

    /* vertex buffer */
    VkDeviceSize offset = 0;
    for (size_t i = 0; i < 5; ++i)
    {
        vkCmdBindVertexBuffers(renderData.rdCommandBuffer, i, 1,
                               &gltfRenderData.rdGltfVertexBufferData.at(i).rdVertexBuffer, &offset);
    }

    /* index buffer */
    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, gltfRenderData.rdGltfIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT16);

    if (renderData.rdGPUVertexSkinning)
    {
        vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderData.rdGltfGPUPipeline);
    }
    else
    {
        vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderData.rdGltfPipeline);
    }

    vkCmdDrawIndexed(renderData.rdCommandBuffer,
                     static_cast<uint32_t>(renderData.rdGltfTriangleCount * 3), 1, 0, 0, 0);
}

void Core::Model::GltfModel::cleanup(Core::Renderer::VkRenderData& renderData,
                                     Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    for (size_t i = 0; i < 5; ++i)
    {
        Core::Renderer::VertexBuffer::cleanup(renderData, gltfRenderData.rdGltfVertexBufferData.at(i));
    }
    Core::Renderer::IndexBuffer::cleanup(renderData, gltfRenderData.rdGltfIndexBufferData);

    Core::Renderer::Texture::cleanup(renderData, gltfRenderData.rdGltfModelTexture);

    mModel.reset();
}

void Core::Model::GltfModel::uploadVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                 Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    for (size_t i = 0; i < 5; ++i)
    {
        const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(i));
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

        Core::Renderer::VertexBuffer::uploadData(renderData, gltfRenderData.rdGltfVertexBufferData.at(i), buffer,
                                                 bufferView);
    }
}

void Core::Model::GltfModel::applyVertexSkinning(Core::Renderer::VkRenderData& renderData,
                                                 Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(0));
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    std::memcpy(mAlteredPositions.data(), &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);

    for (size_t i = 0; i < mJointVec.size(); ++i)
    {
        glm::ivec4 jointIndex = glm::make_vec4(mJointVec.at(i));
        glm::vec4 weightIndex = glm::make_vec4(mWeightVec.at(i));
        glm::mat4 skinMat = weightIndex.x * mJointMatrices.at(jointIndex.x) +
                            weightIndex.y * mJointMatrices.at(jointIndex.y) +
                            weightIndex.z * mJointMatrices.at(jointIndex.z) +
                            weightIndex.w * mJointMatrices.at(jointIndex.w);
        mAlteredPositions.at(i) = skinMat * glm::vec4(mAlteredPositions.at(i), 1.0f);
    }

    Core::Renderer::VertexBuffer::uploadData(renderData, gltfRenderData.rdGltfVertexBufferData.at(0),
                                             mAlteredPositions);
}

void Core::Model::GltfModel::uploadIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                               Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
    const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
    const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

    Core::Renderer::IndexBuffer::uploadData(renderData, gltfRenderData.rdGltfIndexBufferData, indexBuffer,
                                            indexBufferView);
}

void Core::Model::GltfModel::createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                 Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    gltfRenderData.rdGltfVertexBufferData.resize(primitives.attributes.size());
    mAttribAccessors.resize(primitives.attributes.size());

    for (const auto& attrib : primitives.attributes)
    {
        const std::string attribType = attrib.first;
        const int accessorNum = attrib.second;

        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorNum);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

        if ((attribType != "POSITION") && (attribType != "NORMAL") && (attribType != "TEXCOORD_0")
            && (attribType != "JOINTS_0") && (attribType != "WEIGHTS_0"))
        {
            Logger::log(1, "%s: skipping attribute type %s\n", __FUNCTION__, attribType.c_str());
            continue;
        }

        Logger::log(1, "%s: data for %s uses accessor %i\n", __FUNCTION__, attribType.c_str(), accessorNum);
        if (attribType == "POSITION")
        {
            int numPositionEntries = accessor.count;
            mAlteredPositions.resize(numPositionEntries);
            Logger::log(1, "%s: loaded %i vertices from glTF file\n", __FUNCTION__, numPositionEntries);
        }

        mAttribAccessors.at(attributes.at(attribType)) = accessorNum;

        /* buffers for position, normal and tex coordinates */
        Core::Renderer::VertexBuffer::init(
            renderData, gltfRenderData.rdGltfVertexBufferData.at(attributes.at(attribType)), bufferView.byteLength);
    }
}

void Core::Model::GltfModel::createIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                               Core::Renderer::VkGltfRenderData& gltfRenderData)
{
    /* buffer for vertex indices */
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
    const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
    const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

    Core::Renderer::IndexBuffer::init(renderData, gltfRenderData.rdGltfIndexBufferData, indexBufferView.byteLength);
}

int Core::Model::GltfModel::getTriangleCount()
{
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);

    unsigned int triangles = 0;
    switch (primitives.mode)
    {
    case TINYGLTF_MODE_TRIANGLES:
        triangles = indexAccessor.count / 3;
        break;
    default:
        Logger::log(1, "%s error: unknown draw mode %i\n", __FUNCTION__, primitives.mode);
        break;
    }
    return triangles;
}