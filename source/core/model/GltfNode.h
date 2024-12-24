#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class GltfNode
{
  public:
    static std::shared_ptr<GltfNode> createRoot(int rootNodeNum);

    void addChildren(std::vector<int> childNodes);

    std::vector<std::shared_ptr<GltfNode>> getChildren() { return mChildNodes; }

    [[nodiscard]] int getNodeNum() const { return mNodeNum; }

    void setNodeName(const std::string& name) { mNodeName = name; }

    void setScale(glm::vec3 scale) { mScale = scale; }

    void setTranslation(glm::vec3 translation) { mTranslation = translation; }

    void setRotation(glm::quat rotation) { mRotation = rotation; }

    void calculateLocalTRSMatrix();

    void calculateNodeMatrix(glm::mat4 parentNodeMatrix);

    glm::mat4 getNodeMatrix() { return mNodeMatrix; }

    void printTree();

  private:
    void printNodes(std::shared_ptr<GltfNode> startNode, int indent);

    int mNodeNum = 0;
    std::string mNodeName;

    std::vector<std::shared_ptr<GltfNode>> mChildNodes{};

    glm::vec3 mScale = glm::vec3(1.0f);
    glm::vec3 mTranslation = glm::vec3(0.0f);
    glm::quat mRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    glm::mat4 mLocalTRSMatrix = glm::mat4(1.0f);
    glm::mat4 mNodeMatrix = glm::mat4(1.0f);
};
