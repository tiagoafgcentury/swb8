#include "mb_osd_subtitle_configuration.h"
#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_osd_fonts.h"
#include "mb_events.h"
#include "common/mb_state_file.h"

#include <lvgl.h>

namespace mb {

OSD_Subtitle_Configuration::OSD_Subtitle_Configuration(OSD *_parent):
    OSD(_parent)
{
}

OSD_Subtitle_Configuration::~OSD_Subtitle_Configuration()
{
	DELETE_OBJ(m_main);
    DELETE_OBJ(m_title);
    remove_focus();
}

bool OSD_Subtitle_Configuration::handle_event_remote_control(const Event_Remote_Control &_event)
{
    auto func = m_keys.get_selected_mapped_callback(_event.key);
    if (func)
    {
        DEBUG_MSG(OSD, DEBUG, "Executing mapped callback for key: " << to_str(_event.key) << "\n");
        func();
        return true;
    }
    DEBUG_MSG(OSD, DEBUG, "No callback mapped for key: " << to_str(_event.key) << "\n");
    return true;
}

void OSD_Subtitle_Configuration::show_menu_subtitle_configuration(subtitle_configuration_callback_t _callback)
{
	set_focus();
	m_callback = _callback;
    // Cria a tela principal
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset, width, height, OSD_COLOR_BLACK);
    m_title = set_label_text(m_main, tr(__Configuracoes_de_legenda), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 0);
    // Desenha linha separadora
    create_rect(m_main, line_x+line_w, line_y, 3, 3*line_h + 2*line_s, OSD_COLOR_ORANGE);
    // Características da legenda
    m_keys.set_parent(m_main);
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Vertical);
    m_keys.set_group_colors(OSD_COLOR_BLACK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_BLACK);
    m_keys.set_group_selected_color(OSD_COLOR_BLACK);
    m_keys.set_group_pos(line_x, line_y);
    m_keys.set_group_key_size(m_keys_w+20, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_LEFT_MID);
    m_keys.add_key(tr(__Tamanho_da_fonte), m_title_callbacks);
    m_keys.add_key(tr(__Cor_da_fonte), m_title_callbacks);
    m_keys.add_key(tr(__Cor_de_fundo), m_title_callbacks);
    // Teclas de tamanho da fonte
    m_size_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y);
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Pequeno), nullptr);
    m_keys.add_key(tr(__Medio), nullptr);
    m_keys.add_key(tr(__Grande), nullptr);
    m_keys.set_group_enabled(m_size_group, false);
    // Teclas de cor da fonte
    m_color_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y + line_h + line_s);
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Branco), nullptr);
    m_keys.add_key(tr(__Amarelo), nullptr);
    m_keys.add_key(tr(__Preto), nullptr);
    m_keys.set_group_enabled(m_color_group, false);
    // Teclas de cor de fundo
    m_background_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y + 2*(line_h + line_s));
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Transparente), nullptr);
    m_keys.add_key(tr(__Cinza), nullptr);
    m_keys.add_key(tr(__Amarelo), nullptr);
    m_keys.set_group_enabled(m_background_group, false);
    // Tecla Salvar
    m_save_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_save_x, m_save_y);
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Salvar), m_save_callbacks);
    // Desenha o painel de opções
    m_keys.draw();
    // Busca as configurações atuais para mostrar como selecionados
    State_File::App_State_File file;
    auto subtitle_font_size = file.subtitle_font_size;
    auto subtitle_font_color = file.subtitle_font_color;
    auto subtitle_background_color = file.subtitle_background_color;
    // Aplica seleção inicial
    m_keys.set_key_marked(m_size_group, static_cast<int>(subtitle_font_size),true);
    m_keys.set_key_marked(m_color_group, static_cast<int>(subtitle_font_color), true);
    m_keys.set_key_marked(m_background_group, static_cast<int>(subtitle_background_color), true);
    draw_subtitle_preview();
}

void OSD_Subtitle_Configuration::draw_subtitle_preview()
{
    // Cria ponteiro para classe de converter parâmetros da legenda
    Subtitle_Converter converter;
    // Carrega variáveis atuais para o exemplo
    auto subtitle_font_size = m_keys.get_key_marked(m_size_group);
    auto font_size = converter.to_font(subtitle_font_size);
    auto subtitle_font_color = m_keys.get_key_marked(m_color_group);
    auto font_color = converter.to_font_color(subtitle_font_color);
    auto subtitle_background_color = m_keys.get_key_marked(m_background_group);
    auto background_color = converter.to_background_color(subtitle_background_color);

    if (!m_subtitle_box)
    {
        m_subtitle_box = create_rect(m_main, subt_x, subt_y, subt_w, subt_h, background_color);
        lv_obj_set_style_radius(m_subtitle_box, 10, 0);
        m_subtitle_text = set_label_text(m_subtitle_box, tr(__Este_e_um_exemplo_de_legenda), 0, 0, font_size, font_color);
        lv_obj_align(m_subtitle_text, LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        lv_obj_set_style_bg_color(m_subtitle_box, background_color, 0);
        lv_obj_set_style_text_color(m_subtitle_text, font_color, 0);
        lv_obj_set_style_text_font(m_subtitle_text, font_size, 0);
        lv_obj_align(m_subtitle_text, LV_ALIGN_CENTER, 0, 0);
    }

    if ( subtitle_background_color == static_cast<int>(Subtitle_Background_Color::None) )
    {
        lv_obj_set_style_bg_opa(m_subtitle_box, LV_OPA_TRANSP, 0);
    }
    else
    {
        lv_obj_set_style_bg_opa(m_subtitle_box, LV_OPA_COVER, 0);
    }
}

void OSD_Subtitle_Configuration::process_save_callback()
{
    auto subtitle_font_size = m_keys.get_key_marked(m_size_group);
    auto subtitle_font_color = m_keys.get_key_marked(m_color_group);
    auto subtitle_background_color = m_keys.get_key_marked(m_background_group);
    State_File::App_State_File file;
    file.subtitle_font_size = static_cast<Subtitle_Font_Size>(subtitle_font_size);
    file.subtitle_font_color = static_cast<uint8_t>(subtitle_font_color);
    file.subtitle_background_color = static_cast<uint8_t>(subtitle_background_color);
    file.write();
    Task::post_event(std::bind(m_callback, true));
}

void OSD_Subtitle_Configuration::next_callback()
{
    m_keys.next(false);
}

void OSD_Subtitle_Configuration::previous_callback()
{
    m_keys.prev(false);
}

void OSD_Subtitle_Configuration::process_volup_callback()
{
    auto selected_func = m_keys.get_selected_key();
    auto group = selected_func == 0 ? m_size_group : (selected_func == 1 ? m_color_group : m_background_group);
    m_keys.next_marked(group);
    draw_subtitle_preview();
}

void OSD_Subtitle_Configuration::process_voldown_callback()
{
    auto selected_func = m_keys.get_selected_key();
    auto group = selected_func == 0 ? m_size_group : (selected_func == 1 ? m_color_group : m_background_group);
    m_keys.prev_marked(group);
    draw_subtitle_preview();
}



} // namespace mb
