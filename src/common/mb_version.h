#pragma once

#include "mb_globals.h"
#include "../../project_version.h"

#include <string>

class MB_OSD_Version
{

public:

    static std::string get_major_minor_version_str()
    {
        std::string version_minor = PROJECT_VERSION_MINOR;
        version_minor = version_minor.length() == 1 ? "0" + version_minor : version_minor;
        std::string software_version = PROJECT_VERSION_MAJOR + version_minor;
        return software_version;
    }

    static std::string get_major_minor_version()
    {
        std::string version_minor = PROJECT_VERSION_MINOR;
        version_minor = version_minor.length() == 1 ? "0" + version_minor : version_minor;
        std::string software_version = std::string("v") + PROJECT_VERSION_MAJOR + "." + version_minor;
        return software_version;
    }

    static std::string get_major_minor_patch_version()
    {
        std::string version_major_minor = get_major_minor_version();
        std::string version_patch = PROJECT_VERSION_PATCH;
        version_patch = version_patch.length() == 1 ? "0" + version_patch : version_patch;
        return version_major_minor + "." + version_patch;
    }

    static std::string get_full_version()
    {
        std::string software_version = get_major_minor_patch_version();
        std::string version_tweak = PROJECT_VERSION_TWEAK;
        version_tweak = version_tweak.length() == 1 ? "0" + version_tweak : version_tweak;
        return software_version + "." + version_tweak;
    }


private:


};
