#include <future>
#include <glm/gtc/type_ptr.hpp>
#include "GltfSphere.h"
#include "core/tools/Logger.h"
#include "stb_image.h"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/vk-renderer/buffers/IndexBuffer.h"
#include "core/vk-renderer/Texture.h"

bool Core::Model::GltfSphere::loadModel(Core::Renderer::VkRenderData& renderData,
                                        Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData,
                                        const std::string& modelFilename, const std::string& textureFilename)
{
    std::future<bool> textureLoadFuture =
        Core::Renderer::Texture::loadTexture(renderData, gltfSphereRenderData.rdGltfModelTexture, textureFilename);
    if (!textureLoadFuture.get())
    {
        Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
        return false;
    }
    Logger::log(1, "%s: glTF sphere model texture '%s' successfully loaded\n", __FUNCTION__, modelFilename.c_str());

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
        Logger::log(1, "%s error: could not load model '%s', because of '%s'\n", __FUNCTION__, modelFilename.c_str(),
                    stbi_failure_reason());
        return false;
    }

    createVertexBuffers(renderData, gltfSphereRenderData);
    createIndexBuffer(renderData, gltfSphereRenderData);

    renderData.rdGltfSphereTriangleCount = getTriangleCount();

    return true;
}

void Core::Model::GltfSphere::createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                  Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    gltfSphereRenderData.rdGltfVertexBufferData.resize(primitives.attributes.size());
    mAttribAccessors.resize(primitives.attributes.size());

    for (const auto& attrib : primitives.attributes)
    {
        const std::string attribType = attrib.first;
        if (!attributes.contains(attribType))
        {
            Logger::log(1, "%s: skipping attribute type %s\n", __FUNCTION__, attribType.c_str());
            continue;
        }

        const int accessorNum = attrib.second;

        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorNum);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);

        Logger::log(1, "%s: data for %s uses accessor %i\n", __FUNCTION__, attribType.c_str(), accessorNum);

        Logger::log(1, "Attribute: %s, Accessor Index: %d, BufferView Index: %d, Offset: %d, ByteLength: %d\n",
                    attribType.c_str(), accessorNum, accessor.bufferView, bufferView.byteOffset, bufferView.byteLength);

        Logger::log(1, "Buffer Index: %d, Total Buffer Size: %zu\n", bufferView.buffer,
                    mModel->buffers.at(bufferView.buffer).data.size());

        mAttribAccessors.at(attributes.at(attribType)) = accessorNum;

        Logger::log(1, "NEWLOG: Attribute: %s, BufferView Offset: %d, ByteLength: %d\n",
                    attribType.c_str(), bufferView.byteOffset, bufferView.byteLength);

        /*Logger::log(1, "SUPERLOG: Attribute: %s, Accessor Index: %d, BufferView Index: %d\n",
                    attribType.c_str(), accessorNum, accessor.bufferView);

*/
        Core::Renderer::VertexBuffer::init(
            renderData, gltfSphereRenderData.rdGltfVertexBufferData.at(attributes.at(attribType)),
            bufferView.byteLength);
    }

    /*gltfSphereRenderData.rdGltfVertexBufferData.resize(3);
    mAttribAccessors.resize(3);

    const tinygltf::Primitive& primitive = mModel->meshes.at(0).primitives.at(0);

    {
        int accessorNum = primitive.attributes.at("POSITION");
        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorNum);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);

        mAttribAccessors[0] = accessorNum;

        Core::Renderer::VertexBuffer::init(
            renderData,
            gltfSphereRenderData.rdGltfVertexBufferData[0],
            bufferView.byteLength
        );
    }

    {
        int accessorNum = primitive.attributes.at("NORMAL");
        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorNum);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);

        mAttribAccessors[1] = accessorNum;

        Core::Renderer::VertexBuffer::init(
            renderData,
            gltfSphereRenderData.rdGltfVertexBufferData[1],
            bufferView.byteLength
        );
    }

    {
        int accessorNum = primitive.attributes.at("TEXCOORD_0");
        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorNum);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);

        mAttribAccessors[2] = accessorNum;

        Core::Renderer::VertexBuffer::init(renderData, gltfSphereRenderData.rdGltfVertexBufferData[2],
                                           bufferView.byteLength);
    }*/
}

void Core::Model::GltfSphere::createIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                                Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    /* buffer for vertex indices */
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
    const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);

    Core::Renderer::IndexBuffer::init(renderData, gltfSphereRenderData.rdGltfIndexBufferData, indexBufferView.byteLength);
}


void Core::Model::GltfSphere::draw(const Core::Renderer::VkRenderData& renderData,
                                   const Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdPipelineLayout, 0, 1,
                            &gltfSphereRenderData.rdGltfModelTexture.texTextureDescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    for (size_t i = 0; i < attributes.size(); ++i)
    {
        vkCmdBindVertexBuffers(renderData.rdCommandBuffer, i, 1,
                               &gltfSphereRenderData.rdGltfVertexBufferData.at(i).rdVertexBuffer, &offset);
    }

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, gltfSphereRenderData.rdGltfIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT16);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      renderData.rdGltfSpherePipeline);

    vkCmdDrawIndexed(renderData.rdCommandBuffer,
                     static_cast<uint32_t>(renderData.rdGltfSphereTriangleCount * 3), 1, 0, 0, 0);
}

void Core::Model::GltfSphere::uploadVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                  Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    for (size_t i = 0; i < attributes.size(); ++i)
    {
        const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(i));
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

        Core::Renderer::VertexBuffer::uploadData(renderData, gltfSphereRenderData.rdGltfVertexBufferData.at(i), buffer,
                                                 bufferView);
    }
}

void Core::Model::GltfSphere::uploadIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                                Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
    const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
    const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

    Core::Renderer::IndexBuffer::uploadData(renderData, gltfSphereRenderData.rdGltfIndexBufferData, indexBuffer,
                                            indexBufferView);
}
int Core::Model::GltfSphere::getTriangleCount()
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

void Core::Model::GltfSphere::cleanup(Core::Renderer::VkRenderData& renderData,
                                      Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData)
{
    for (size_t i = 0; i < attributes.size(); ++i)
    {
        Core::Renderer::VertexBuffer::cleanup(renderData, gltfSphereRenderData.rdGltfVertexBufferData.at(i));
    }
    Core::Renderer::IndexBuffer::cleanup(renderData, gltfSphereRenderData.rdGltfIndexBufferData);

    mModel.reset();
}