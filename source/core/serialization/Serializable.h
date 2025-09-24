#pragma once

#include "yaml-cpp/yaml.h"

namespace Core::Serialization
{
class ISerializable {
public:
    virtual ~ISerializable() = default;

    virtual YAML::Node serialize() const = 0;
    virtual void deserialize(const YAML::Node& node) = 0;
};
}