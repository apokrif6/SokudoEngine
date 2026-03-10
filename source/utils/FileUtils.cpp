#include "FileUtils.h"
#include <filesystem>

std::string Core::Utils::FileUtils::getRelativePath(const std::string_view& fullPath)
{
    try
    {
        return std::filesystem::relative(fullPath, std::filesystem::current_path()).generic_string();
    }
    catch (...)
    {
        return std::string(fullPath);
    }
}

std::string Core::Utils::FileUtils::getParentDirectory(const std::string_view& filePath)
{
    if (const std::filesystem::path path(filePath); path.has_parent_path())
    {
        return (path.parent_path() / "").generic_string();
    }
    return "";
}
