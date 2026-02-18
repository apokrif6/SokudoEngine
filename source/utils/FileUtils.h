#pragma once

#include <string>

namespace Core::Utils
{
class FileUtils
{
public:
    static std::string getRelativePath(const std::string_view& fullPath);
};
} // namespace Core::Utils