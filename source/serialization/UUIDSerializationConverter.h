#pragma once

#include "yaml-cpp/node/node.h"
#include "uuid.h"

template <> struct YAML::convert<uuids::uuid>
{
    static Node encode(const uuids::uuid& rhs) { return Node(uuids::to_string(rhs)); }

    static bool decode(const Node& node, uuids::uuid& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }
        if (const auto optional = uuids::uuid::from_string(node.as<std::string>()))
        {
            rhs = *optional;
            return true;
        }
        return false;
    }
}; // namespace YAML