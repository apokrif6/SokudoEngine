#pragma once

#include "uuid.h"
#include "animations/anim-graph/AnimGraphPin.h"

#include <variant>

namespace Core::Animations
{
inline uuids::uuid generateUUID()
{
    thread_local std::mt19937 range{std::random_device{}()};

    return uuids::uuid_random_generator{range}();
}

using PropertyValue = std::variant<float, int, bool, uuids::uuid>;

struct Pose;
struct AnimationContext;

class AnimGraphNode
{
public:
    virtual ~AnimGraphNode() = default;

    virtual Pose evaluate(AnimationContext& context) const = 0;

    [[nodiscard]] const uuids::uuid& getUUID() const { return mUUID; }

    void setProperty(const std::string& name, const PropertyValue& value)
    {
        mProperties[name] = value;
        onPropertyChanged(name);
    }

    [[nodiscard]] const PropertyValue* getProperty(const std::string& name) const
    {
        if (const auto it = mProperties.find(name); it != mProperties.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    [[nodiscard]] const std::vector<AnimGraphPin>& getInputs() const { return mInputs; }

    [[nodiscard]] const std::vector<AnimGraphPin>& getOutputs() const { return mOutputs; }

protected:
    virtual void onPropertyChanged(const std::string&) {}

    PinID createInputPin(const AnimGraphValueType type)
    {
        const PinID id = sNextPinID++;

        mInputs.push_back({id, AnimGraphPinKind::Input, type});

        return id;
    }

    PinID createOutputPin(const AnimGraphValueType type)
    {
        const PinID id = sNextPinID++;

        mOutputs.push_back({id, AnimGraphPinKind::Output, type});

        return id;
    }

    std::vector<AnimGraphPin> mInputs;

    std::vector<AnimGraphPin> mOutputs;

private:
    inline static PinID sNextPinID = 1;

    uuids::uuid mUUID = generateUUID();

    std::unordered_map<std::string, PropertyValue> mProperties;
};
} // namespace Core::Animations