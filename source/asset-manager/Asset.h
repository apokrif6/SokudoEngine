#pragma once

#include <string>

namespace Core::Assets
{
class Asset
{
public:
    explicit Asset(std::string path) : mPath(std::move(path)) {}

    virtual ~Asset() = default;

    const std::string& getPath() const { return mPath; }

protected:
    std::string mPath;
};
} // namespace Core::Assets