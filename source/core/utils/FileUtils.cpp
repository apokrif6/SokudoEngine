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
