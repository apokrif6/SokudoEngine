#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "GltfNode.h"
#include "core/tools/Logger.h"

std::shared_ptr<Core::Model::GltfNode>Core::Model::GltfNode::createRoot(int rootNodeNum)
{
    auto mParentNode = std::make_shared<GltfNode>();
    mParentNode->mNodeNum = rootNodeNum;
    return mParentNode;
}

void Core::Model::GltfNode::addChildren(const std::vector<int>& childNodes)
{
    for (const int childNode : childNodes)
    {
        std::shared_ptr<Core::Model::GltfNode>child = std::make_shared<GltfNode>();
        child->mNodeNum = childNode;

        mChildNodes.push_back(child);
    }
}

void Core::Model::GltfNode::calculateLocalTRSMatrix()
{
    glm::mat4 sMatrix = glm::scale(glm::mat4(1.0f), mScale);
    glm::mat4 rMatrix = glm::mat4_cast(mRotation);
    glm::mat4 tMatrix = glm::translate(glm::mat4(1.0f), mTranslation);
    mLocalTRSMatrix = tMatrix * rMatrix * sMatrix;
}

void Core::Model::GltfNode::calculateNodeMatrix(const glm::mat4& parentNodeMatrix) { mNodeMatrix = parentNodeMatrix * mLocalTRSMatrix; }

void Core::Model::GltfNode::printTree()
{
    Logger::log(1, "%s: ---- tree ----\n", __FUNCTION__);
    Logger::log(1, "%s: parent : %i (%s)\n", __FUNCTION__, mNodeNum, mNodeName.c_str());
    for (const auto& childNode : mChildNodes)
    {
        Core::Model::GltfNode::printNodes(childNode, 1);
    }
    Logger::log(1, "%s: -- end tree --\n", __FUNCTION__);
}

void Core::Model::GltfNode::printNodes(const std::shared_ptr<Core::Model::GltfNode>& startNode, const int indent)
{
    std::string indentString;
    for (size_t i = 0; i < indent; ++i)
    {
        indentString += " ";
    }
    indentString += "-";
    Logger::log(1, "%s: %s child : %i (%s)\n", __FUNCTION__, indentString.c_str(), startNode->mNodeNum,
                startNode->mNodeName.c_str());

    for (const auto& childNode : startNode->mChildNodes)
    {
        Core::Model::GltfNode::printNodes(childNode, indent + 1);
    }
}
