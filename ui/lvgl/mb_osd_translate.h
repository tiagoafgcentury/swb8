#pragma once

#include "common/mb_globals.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "common/mb_lineup.h"

#include <memory>
#include <string_view>

namespace mb {

class OSD_Translate final
{
public:
    enum class Lang_Index
    {
        por,
        eng,
        COUNT
    };

private:
    OSD_Translate();

    static OSD_Translate s_instance;

    Lang_Index m_language = Lang_Index::por;

public:
    virtual ~OSD_Translate();

    enum class Text_Index
    {
#include "translate_field_1.inc"
        COUNT
    };

    static std::string_view get_current_language();
    static void set_language(Lang_Index _index);
    static Lang_Index next_language(void);
    static Lang_Index get_language();
    static bool is_portuguese();
    static bool is_english();

    static std::string_view translate(Text_Index _index);
    static std::string_view translate(DiseqC_Type _type);
    static std::string_view translate(Band _band);
    static std::string_view translate(LNBF_Type _type);
    static std::string_view translate(LNBF_Position _position);
    static std::string_view translate(Polarity _polarity);
};

#define tr(TEXT) OSD_Translate::translate(OSD_Translate::Text_Index::TEXT)

} // namespace mb
