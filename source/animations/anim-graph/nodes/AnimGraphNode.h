#pragma once

#include "uuid.h"
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

protected:
    virtual void onPropertyChanged(const std::string&) {}

private:
    uuids::uuid mUUID = generateUUID();

    std::unordered_map<std::string, PropertyValue> mProperties;
};
} // namespace Core::Animations