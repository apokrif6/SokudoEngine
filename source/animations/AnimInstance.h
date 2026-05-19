#pragma once

#include "anim-graph/AnimGraph.h"
#include "anim-graph/nodes/AnimGraphNodeRuntime.h"

#include <string>
#include <unordered_map>

namespace Core::Animations
{
struct AnimParamID;
class AnimInstance
{
public:
    explicit AnimInstance(std::shared_ptr<AnimGraph> graph) : mGraph(std::move(graph)) {}

    Pose evaluate(AnimationContext& context);

    void setFloat(const std::string& name, float value);
    void setBool(const std::string& name, bool value);

    [[nodiscard]] float getFloat(const std::string& name) const;
    [[nodiscard]] bool getBool(const std::string& name) const;

    void setFloat(AnimParamID id, float value);
    void setBool(AnimParamID id, bool value);

    [[nodiscard]] float getFloat(AnimParamID id) const;
    [[nodiscard]] bool getBool(AnimParamID id) const;

    [[nodiscard]] AnimGraphNodeRuntime& getRuntime(const AnimGraph::NodeID id) { return mNodeRuntime[id]; }

private:
    std::shared_ptr<AnimGraph> mGraph;

    std::unordered_map<uint32_t, float> mFloatParameters;
    std::unordered_map<uint32_t, bool> mBoolParameters;

    std::unordered_map<AnimGraph::NodeID, AnimGraphNodeRuntime> mNodeRuntime;
};

} // namespace Core::Animations