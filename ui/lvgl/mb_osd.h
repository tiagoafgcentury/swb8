#pragma once

#include "mb_menu_data.h"
#include "hal/mb_remote_control_keys.h"

#include <math.h>
#include <memory>
#include <string_view>

namespace mb {

class OSD
{
private:
    void close_all_parents(std::vector<OSD *> _children);

protected:
    OSD *m_parent = nullptr;

    void close_all_parents();

public:
    // Numeric values acording to "Anexo E_ Diretrizes Regionalização_v1.1.pdf"
    enum class Parental_Control
    {
        CLASSIFICACAO_LIVRE = 0,
        CLASSIFICACAO_10 = 7,
        CLASSIFICACAO_12 = 9,
        CLASSIFICACAO_14 = 11,
        CLASSIFICACAO_16 = 13,
        CLASSIFICACAO_18 = 15,
        CLASSIFICACAO_INDEFINIDA
    };

    enum class OSD_Layer
    {
        RADIO_INFO,
        WELCOME_BANNER,
        CLOSED_CAPTION_LAYER,
        PARENTAL_BLOCK_SCREEN,
        AUDIO_LR_LAYER,
        NO_SIGNAL,
        FACTORY_INFO_SCREEN,
        MEDIA_PLAYER_LAYER,
        VOLUME_LAYER,
        SLEEP_TIMER_LAYER,
        MAIN_INFO,
        MAIN_MENU,
        USB_DEVICE_LAYER,
        LAYER_COUNT // Must always be last, not a real layer
    };

    enum class OSD_Channels_List_Type
    {
        TV_Channels,
        Radio_Channels,
    };

    struct Parental_Control_Data_t
    {
        std::string_view text ;
        int age ;
        lv_color_t color;
        int border;
    };

    OSD(OSD *_parent);
    virtual ~OSD();

    static void init_layers();
    static lv_obj_t *get_main_screen(OSD_Layer _layer_priority);
    static lv_obj_t *create_line(lv_obj_t *bgd, int x, int y, lv_color_t color);
    static lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color);
    static lv_obj_t *create_circle(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color);
    static lv_obj_t *create_qrcode(lv_obj_t *bgd, int size);
    static lv_obj_t *load_image(lv_obj_t *bgd, std::string_view logo, int x, int y, int w, int h);
    static lv_obj_t *set_label_text(lv_obj_t *bgd, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor);
    static lv_obj_t *set_label_text_static(lv_obj_t *bgd, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor);
    static lv_obj_t *update_or_create_label(lv_obj_t *parent, lv_obj_t **existing_obj, std::string_view text, int x, int y, lv_font_t *font, lv_color_t fontColor);
    static lv_point_t lv_label_get_char_pos(lv_obj_t *obj, std::string_view text, std::string_view chr);
    static lv_obj_t *classificacao_indicativa(lv_obj_t *bgd, Parental_Control pc, int x, int y, int size);
    static std::pair<Parental_Control, Parental_Control_Data_t> get_parental_control_data(Parental_Control _parental_control);
    static char get_digit(Remote_Control_Key _key);

    static constexpr auto s_activation_link_claro = "https://www.centurybr.com.br/suporte/ativacao-do-sathd-regional";
    static constexpr auto s_activation_link_sky = "https://www.novaparabolica.com.br/ativar-equipamento";
    static void qrcode_update(NID_t _network_id, lv_obj_t *_qrcode, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

    

    static void split_string(std::vector<std::string> &_tokens, const std::string &_str, char _delim);
    static bool is_valid_date(uint16_t _day, uint16_t _month, uint16_t _year);
    static bool is_leap_year(uint16_t _year);
};

} // namespace mb
