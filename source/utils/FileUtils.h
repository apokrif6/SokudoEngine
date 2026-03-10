#pragma once

#include <string>

namespace Core::Utils
{
class FileUtils
{
public:
    static std::string getRelativePath(const std::string_view& fullPath);

    static std::string getParentDirectory(const std::string_view& filePath);
};
} // namespace Core::Utils