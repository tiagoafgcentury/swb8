#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "mb_osd_fade_canvas.h"
#include "common/mb_globals.h"
#include <common/mb_config.h>
#include "mb_osd_fonts.h"

#include <lvgl.h>
#include <math.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

namespace mb {

namespace {

constexpr auto LAYER_COUNT = static_cast<unsigned int>(OSD::OSD_Layer::LAYER_COUNT);
std::array<lv_obj_t *, LAYER_COUNT> s_screens;

}

void OSD::init_layers()
{
    auto parent = lv_screen_active();
    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, LV_PART_MAIN);
    auto width = lv_obj_get_width(parent);
    auto height = lv_obj_get_height(parent) + 10;

    for (auto i = 0u; i < LAYER_COUNT; i++)
    {
        auto obj = lv_obj_create(parent);
        lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
        lv_obj_set_size(obj, width, height);
        lv_obj_set_pos(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        s_screens[i] = obj;
    }
}

OSD::OSD(OSD *_parent):
    m_parent(_parent)
{
}

OSD::~OSD()
{
}

lv_obj_t *OSD::get_main_screen(OSD_Layer _layer_priority)
{
    return s_screens[static_cast<unsigned int>(_layer_priority)];
}

lv_obj_t *OSD::create_line(lv_obj_t *bgd, int x, int y, lv_color_t color)
{
    static lv_point_precise_t line_points[] = {{0, 0}, {0, DISPLAY_HEIGHT}};
    auto line = lv_line_create(bgd);
    lv_line_set_points(line, line_points, 2);
    lv_obj_set_style_bg_color(line, color, 0);
    lv_obj_align(line, LV_ALIGN_DEFAULT, x, y);
    return line;
}

lv_obj_t *OSD::create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color)
{
    auto rect = lv_obj_create(bgd);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_scrollbar_mode(rect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect, color, 0);
    lv_obj_set_style_radius(rect, 0, DEFAULT_SELECTOR);
    lv_obj_align(rect, LV_ALIGN_DEFAULT, x, y);
    lv_obj_set_style_pad_all(rect, 0, LV_PART_MAIN);
    return rect;
}

lv_obj_t *OSD::create_circle(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color)
{
    auto circ = create_rect(bgd, x, y, w, h, color);
    lv_obj_set_style_radius(circ, h, DEFAULT_SELECTOR);
    //lv_obj_align(circ, LV_ALIGN_DEFAULT, x, y);
    return circ;
}

void OSD::qrcode_update(NID_t _network_id, lv_obj_t *_qrcode, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    _caid.erase(remove(_caid.begin(), _caid.end(), ' '), _caid.end());
    _scua.erase(remove(_scua.begin(), _scua.end(), ' '), _scua.end());
    std::string link_str;

    switch (_network_id)
    {
        case Network_Id_Sky:
        {
            link_str = s_activation_link_sky + std::string("?caid=") + _caid + "&scua=" + _scua;
            break;
        }

        default:
        {
            link_str = s_activation_link_claro + std::string("/?caid=") + _caid + "&scua=" + _scua;
            break;
        }
    }

    DEBUG_MSG(OSD, DEBUG, "link_str: " << link_str << "\n");
    lv_qrcode_update(_qrcode, link_str.c_str(), link_str.size());
}

lv_obj_t *OSD::create_qrcode(lv_obj_t *bgd, int size)
{
    auto qrcode = lv_qrcode_create(bgd);
    lv_qrcode_set_size(qrcode, size);
    lv_qrcode_set_dark_color(qrcode, OSD_COLOR_BLACK);
    lv_qrcode_set_light_color(qrcode, OSD_COLOR_WHITE);
    return qrcode;
}

lv_obj_t *OSD::load_image(lv_obj_t *bgd, std::string_view logo, int x, int y, int w, int h)
{
    auto img = lv_image_create(bgd);
    lv_image_set_src(img, logo.data());
    lv_obj_set_size(img, w, h);
    lv_obj_align(img, LV_ALIGN_DEFAULT, x, y);
    return img;
}

lv_obj_t *set_label_text_base(lv_obj_t *bgd, int x, int y, lv_font_t *font, lv_color_t fontColor)
{
    auto label = lv_label_create(bgd);
    lv_obj_set_style_text_font(label,  font, 0);
    lv_obj_set_style_text_color(label, fontColor, DEFAULT_SELECTOR);
    lv_obj_align(label, LV_ALIGN_DEFAULT, x, y);
    return label;
}

lv_obj_t *OSD::set_label_text(lv_obj_t *bgd, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor)
{
    auto label = set_label_text_base(bgd, x, y, font, fontColor);
    //lv_label_set_text(label, text.data());
    lv_label_set_text(label, std::string(text).c_str());
    return label;
}

lv_obj_t *OSD::set_label_text_static(lv_obj_t *bgd, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor)
{
    auto label = set_label_text_base(bgd, x, y, font, fontColor);
    lv_label_set_text_static(label, text.data());
    return label;
}

lv_obj_t *OSD::update_or_create_label(lv_obj_t *parent, lv_obj_t **existing_obj, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor)

{
    // 1. Create the object only if it doesn't exist
    if (*existing_obj == nullptr) {
        *existing_obj = lv_label_create(parent);
        lv_obj_null_on_delete(existing_obj);
    }

    // 2. Update properties (LVGL is smart enough to skip redraw if values haven't changed)
    lv_label_set_text(*existing_obj, std::string(text).c_str());
    lv_obj_set_style_text_color(*existing_obj, fontColor, 0);
    lv_obj_set_style_text_font(*existing_obj, font, 0);

    // Use set_pos instead of align if the coordinates are absolute
    // relative to the parent, as it's lighter on the CPU.
    lv_obj_set_pos(*existing_obj, x, y);

    return *existing_obj;
}

lv_point_t OSD::lv_label_get_char_pos(lv_obj_t *obj, std::string_view text, std::string_view chr)
{
    std::array<std::u32string, 12> charArray =
    {
        U"aãáàâä",
        U"eéèêë",
        U"iíìîï",
        U"oóòôõö",
        U"uúùûü",
        U"AÃÁÀÂÄ",
        U"EÉÈÊË",
        U"IÍÌÎÏ",
        U"OÓÒÔÕÖ",
        U"UÚÙÛÜ",
        U"cç",
        U"CÇ",
    };
    // Converte o caracter para UTF-32
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32 = converter.from_bytes(text.data(), text.data() + text.size());

    // Varre cada um dos caracteres do texto convertido
    for (auto &c : u32)
    {
        // Varre cada uma das strings do array
        for (auto str : charArray)
        {
            // Procura o caracter na string
            if (str.find(c) != std::string::npos)
            {
                // Adiciona o caracter convertido
                c = str[0];
                break;
            }
        }
    }

    // Converte o texto para UTF-8
    std::string convertedString = converter.to_bytes(u32);
    //std::cout << "Original : " << text << std::endl;
    //std::cout << "Converted: " << convertedString << std::endl;
    size_t index = 0;
    lv_point_t position;
    index = convertedString.find(chr);
    //DEBUG_MSG("Posição do caracter = " << std::dec << index << "\n");
    lv_label_get_letter_pos(obj, index, &position);
    //DEBUG_MSG("x = " << std::dec << position.x << ", y = " << position.y << "\n");
    return position;
}

namespace {

struct Classification
{
    const char *text;
    lv_color_t color;
    int border;
};

const std::map<OSD::Parental_Control, OSD::Parental_Control_Data_t> parental_control_data =
{
    {OSD::Parental_Control::CLASSIFICACAO_LIVRE,    OSD::Parental_Control_Data_t{"L", 0,OSD_COLOR_GREEN_PC, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_10,       OSD::Parental_Control_Data_t{"10", 10,OSD_COLOR_BLUE_PC, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_12,       OSD::Parental_Control_Data_t{"12", 12,OSD_COLOR_YELLOW_PC, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_14,       OSD::Parental_Control_Data_t{"14", 14,OSD_COLOR_ORANGE_PC, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_16,       OSD::Parental_Control_Data_t{"16", 16,OSD_COLOR_RED_PC, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_18,       OSD::Parental_Control_Data_t{"18", 18,OSD_COLOR_BLACK_LIGHT, 0}},
    {OSD::Parental_Control::CLASSIFICACAO_INDEFINIDA,OSD::Parental_Control_Data_t{"?", 0,OSD_COLOR_BROWN, 0}}
};

}

std::pair<OSD::Parental_Control, OSD::Parental_Control_Data_t> OSD::get_parental_control_data(OSD::Parental_Control _parental_control)
{
    OSD::Parental_Control pc;
    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    if(network_id == Network_Id_Sky)
    {
        pc = static_cast<int>(_parental_control) == 1 ? OSD::Parental_Control::CLASSIFICACAO_10 :
              static_cast<int>(_parental_control) == 2 ? OSD::Parental_Control::CLASSIFICACAO_12 :
              static_cast<int>(_parental_control) == 3 ? OSD::Parental_Control::CLASSIFICACAO_14 :
              static_cast<int>(_parental_control) == 4 ? OSD::Parental_Control::CLASSIFICACAO_16 :
              (static_cast<int>(_parental_control) == 5 || static_cast<int>(_parental_control) == 7) ? OSD::Parental_Control::CLASSIFICACAO_18 :
              OSD::Parental_Control::CLASSIFICACAO_LIVRE;
    }
    // Claro/Embratel
    else 
    {
        auto pc_it = parental_control_data.find(_parental_control);
        if (pc_it == parental_control_data.end())
        {
            // PC + 3
            // Acording to DVB Standard "Specification-for-Service-Information-SI-in-DVB-Systems_Draft_EN_300-468-v1-17-1_Dec-2021"
            // Section "6.2.28 Parental rating descriptor"
            // Some services transmit using DVB standard, other the value directly, so we retry searching suposing that the value is direct
            pc_it = parental_control_data.find(static_cast<OSD::Parental_Control>(static_cast<int>(_parental_control) + 3));
        }

        if (pc_it != parental_control_data.end())
        {
            pc = pc_it->first;
        }
        else
        {
            pc = OSD::Parental_Control::CLASSIFICACAO_INDEFINIDA;
        }
    }

    auto data = parental_control_data.find(pc);
    return std::make_pair(pc, data->second);
}

lv_obj_t *OSD::classificacao_indicativa(lv_obj_t *bgd, Parental_Control _parental_control, int x, int y, int size)
{
    lv_obj_t *class_box;
    lv_obj_t *classificacao;
    auto pc = get_parental_control_data(_parental_control);
    auto data = pc.second;

    if (size == 0)
    {
        class_box = OSD::create_rect(bgd, x, y, 30, 30, data.color);
        lv_obj_set_style_radius(class_box, 4, DEFAULT_SELECTOR);
        classificacao = OSD::set_label_text(class_box, data.text, 0, 0, font_bold_20, OSD_COLOR_WHITE);
    }
    else
    {
        class_box = OSD::create_rect(bgd, x, y, 42, 42, data.color);
        lv_obj_set_style_radius(class_box, 2, DEFAULT_SELECTOR);
        classificacao = OSD::set_label_text(class_box, data.text, 0, 0, font_bold_25, OSD_COLOR_WHITE);
    }

    lv_obj_align(classificacao, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_width(class_box, data.border, LV_PART_MAIN);
    return class_box;
}

// Converte tecla em dígito
char OSD::get_digit(Remote_Control_Key _key)
{
    switch (_key)
    {
#define RK(NUM) \
case Remote_Control_Key::KEY_ ## NUM : return NUM;
            // Teclas numéricas
            RK(0);
            RK(1);
            RK(2);
            RK(3);
            RK(4);
            RK(5);
            RK(6);
            RK(7);
            RK(8);
            RK(9);

        default:
            break;
    }

    return '\n';
}

void OSD::close_all_parents()
{
    std::vector<OSD *> children;
    children.push_back(this);
    close_all_parents(std::move(children));
}

void OSD::close_all_parents(std::vector<OSD *> _children)
{
    // find base parent
    if (m_parent != nullptr)
    {
        _children.push_back(this);
        close_all_parents(std::move(_children));
    }
    else
    {
        // this is the base parent
        // needs to be implemented
    }
}

void OSD::split_string(std::vector<std::string> &_tokens, const std::string &_str, char _delim)
{
    std::istringstream iss(_str);
    std::string token;

    while (std::getline(iss, token, _delim))
    {
        if (!token.empty())
        {
            _tokens.push_back(token);
        }
    }
}

// Função para verificar se o ano é bissexto
bool OSD::is_leap_year(uint16_t _year)
{
    return (_year % 4 == 0 && _year % 100 != 0) || (_year % 400 == 0);
}

// Função para validar a data
bool OSD::is_valid_date(uint16_t _day, uint16_t _month, uint16_t _year)
{
    if (_year < 1)
    {
        return false;
    }

    if (_month < 1 || _month > 12)
    {
        return false;
    }

    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (is_leap_year(_year))
    {
        daysInMonth[1] = 29;
    }

    return _day > 0 && _day <= daysInMonth[_month - 1];
}

} // namespace mb
