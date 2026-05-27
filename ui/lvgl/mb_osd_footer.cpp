#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include <map>

namespace mb {

static const std::unordered_map<std::string_view, std::string_view> s_logos =
{
    { "--+--", LOGO_MAIS_27x27, },
    { "--voltar--", LOGO_VOLTAR_27x27, },
    { "--ok--", LOGO_OK_27x27, },
    { "--cc--", LOGO_CC_27x27, },
    { "--5--", LOGO_5_27x27, },
};

namespace MB_OSD_Footer {

struct Footer_Data
{
    std::string_view icon_text;
    lv_point_t point;
};

lv_obj_t *draw(lv_obj_t *_background, std::string_view _text, int _offset_y, lv_font_t *_font)
{
    std::vector<Footer_Data> icons;
    std::string label_text(_text);

    // Verifica se existe algum ícone no rodapé
    do
    {
        size_t pos;
        std::string_view icon;

        // Verifica se o texto tem a string "--", se não tiver, sair do loop
        if ((pos = label_text.find("--")) == std::string::npos)
        {
            break;
        }

        // Busca qual ícone está mapeado na primeira ocorrencia de "--"
        size_t to_be_deleted = 0;
        size_t logo_pos = -1;

        for (const auto &logo : s_logos)
        {
            if (label_text.find(logo.first) == pos)
            {
                icon = logo.second;
                logo_pos = pos;
                to_be_deleted = logo.first.size();
                break;
            }
        }

        // Se não encontrou um ícone equivalente, sai do loop
        if (logo_pos != pos)
        {
            break;
        }

        // Separa marcação do ícone pelo marcador
        label_text.replace(pos, to_be_deleted, "*   ");
        // Busca a posição em pixels do caracter '*' no texto
        {
            auto temp = mb::OSD::set_label_text(_background, label_text, 0, 0, _font, OSD_COLOR_WHITE);
            lv_obj_align(temp, LV_ALIGN_BOTTOM_MID, 0, _offset_y);
            auto p = mb::OSD::lv_label_get_char_pos(temp, label_text, "*");
            icons.push_back({.icon_text = icon, .point = p});
            lv_obj_delete(temp);
            label_text.replace(pos, 4, "    ");
        }
    }
    while (true);

    // Desenha texto final
    auto res = mb::OSD::set_label_text(_background, label_text, 0, 0, _font, OSD_COLOR_WHITE);
    lv_obj_align(res, LV_ALIGN_BOTTOM_MID, 0, _offset_y);

    // Sobrescreve os ícones
    for (const auto &it : icons)
    {
        auto logo = mb::OSD::load_image(res, it.icon_text, 0, 0, 27, 27);
        lv_obj_set_pos(logo, it.point.x, it.point.y);
    }

    icons.clear();
    return res;
}

}

}
