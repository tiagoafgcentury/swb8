#pragma once

#include <string>

#if __has_include(<cJSON.h>)
    #include <cJSON.h>
#else
    #include <cjson/cJSON.h>
#endif

namespace mb {

class Terms_File
{
public:
    struct App_Terms_of_Use
    {
        App_Terms_of_Use();

        std::string version;
        std::string text;
        std::string satellite;
    };

    static App_Terms_of_Use get_terms_of_use();
    static void accept_terms_of_use();
    static bool terms_has_been_accepted();
};
}
