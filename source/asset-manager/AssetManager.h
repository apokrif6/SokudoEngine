#pragma once

#include "core/Singleton.h"
#include "Asset.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Core::Assets
{
class AssetManager final : public Singleton<AssetManager>
{
    friend class Singleton;

public:
    template <typename T, typename... Args> std::shared_ptr<T> getOrCreate(const std::string& path, Args&&... args)
    {
        if (const auto it = mAssets.find(path); it != mAssets.end())
        {
            return std::static_pointer_cast<T>(it->second);
        }

        auto asset = std::make_shared<T>(path, std::forward<Args>(args)...);
        mAssets[path] = asset;
        return asset;
    }

    void clear() { mAssets.clear(); }

private:
    AssetManager() = default;

    std::unordered_map<std::string, std::shared_ptr<Asset>> mAssets;
};
} // namespace Core::Assets