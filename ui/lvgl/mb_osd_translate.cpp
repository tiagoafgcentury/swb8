#include <iostream>
#include <map>

#include "mb_osd_translate.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "common/mb_state_file.h"

namespace {

typedef std::array<const char *, static_cast<int>(mb::OSD_Translate::Text_Index::COUNT)> Translations;
typedef std::array<Translations, static_cast<int>(mb::OSD_Translate::Lang_Index::COUNT)> Dictionaries;

Translations s_portuguese =
{
#include "translate_field_2.inc"
};

Translations s_english =
{
#include "translate_field_3.inc"
};

Dictionaries s_dictionaries = { s_portuguese, s_english };

}

namespace mb {

OSD_Translate OSD_Translate::s_instance;

OSD_Translate::OSD_Translate()
{
    State_File::App_State_File file;
    auto new_lang = static_cast<Lang_Index>(file.language_mode);
    mb_assert(new_lang >= Lang_Index::por and new_lang < Lang_Index::COUNT);

    if (new_lang >= Lang_Index::por and new_lang < Lang_Index::COUNT)
    {
        m_language = new_lang;
    }
}

OSD_Translate::~OSD_Translate()
{
}

std::string_view OSD_Translate::translate(Text_Index _index)
{
    mb_assert(_index < Text_Index::COUNT);
    mb_assert(s_dictionaries.size() > static_cast<size_t>(s_instance.m_language));
    mb_assert(s_dictionaries[static_cast<int>(s_instance.m_language)].size() > static_cast<size_t>(_index));
    return s_dictionaries[static_cast<int>(s_instance.m_language)][static_cast<int>(_index)];
}

// Inicializa idioma com parâmetro recebido
void OSD_Translate::set_language(Lang_Index _index)
{
    s_instance.m_language = _index;
}

// Inicializa idioma com parâmetro recebido
bool OSD_Translate::is_portuguese()
{
    return s_instance.m_language == Lang_Index::por ? true : false;
}

// Inicializa idioma com parâmetro recebido
bool OSD_Translate::is_english()
{
    return s_instance.m_language == Lang_Index::eng ? true : false;
}

OSD_Translate::Lang_Index OSD_Translate::next_language()
{
    s_instance.m_language = s_instance.m_language == Lang_Index::por ? Lang_Index::eng : Lang_Index::por;
    return s_instance.m_language;
}

OSD_Translate::Lang_Index OSD_Translate::get_language()
{
    return s_instance.m_language;
}

std::string_view OSD_Translate::get_current_language()
{
    if (s_instance.m_language == Lang_Index::por)
    {
        return s_dictionaries[static_cast<int>(s_instance.m_language)][static_cast<int>(Text_Index::__Portugues)];
    }

    return s_dictionaries[static_cast<int>(s_instance.m_language)][static_cast<int>(Text_Index::__Ingles)];
}

std::string_view OSD_Translate::translate(DiseqC_Type _type)
{
    static char buffer[15];

    switch (_type)
    {
        case DiseqC_Type::None:
            return tr(__Nenhum);

        case DiseqC_Type::DiseqC_1_0:
            return "DiseqC 1.0";

        case DiseqC_Type::DiseqC_1_1:
            return "DiseqC 1.1";

        case DiseqC_Type::LNBF_Switch:
        {
            [[maybe_unused]] auto sz = snprintf(buffer, sizeof(buffer), "%s 13/18V", tr(__Chave).data());
            mb_assert(sz < (int)sizeof(buffer));
            return buffer;
        }
    }

    return tr(__Desconhecido);
}

std::string_view OSD_Translate::translate(Band _band)
{
    switch (_band)
    {
        case Band::C:
            return tr(__Banda_C);

        case Band::Ku:
            return tr(__Banda_KU);

        case Band::UNDEFINED:
            break;
    }

    return tr(__Desconhecido);
}

std::string_view OSD_Translate::translate(LNBF_Type _type)
{
    switch (_type)
    {
        case LNBF_Type::Mono:
            return tr(__Monoponto);

        case LNBF_Type::Multi:
            return tr(__Multiponto);

        case LNBF_Type::Universal:
            return tr(__Universal);

        case LNBF_Type::UNDEFINED:
            break;
    }

    return tr(__Desconhecido);
}

std::string_view OSD_Translate::translate(LNBF_Position _position)
{
    switch (_position)
    {
        case LNBF_Position::Normal:
            return tr(__Normal);

        case LNBF_Position::Inverted:
            return tr(__Invertido);
    }

    return tr(__Desconhecido);
}

std::string_view OSD_Translate::translate(Polarity _polarity)
{
    switch (_polarity)
    {
        case Polarity::Horizontal:
            return tr(__Horizontal);

        case Polarity::Vertical:
            return tr(__Vertical);

        case Polarity::Left:
            return tr(__Esquerda);

        case Polarity::Right:
            return tr(__Direita);

        case Polarity::UNDEFINED:
            break;
    }

    return tr(__Desconhecido);
}

} // namespace mb
