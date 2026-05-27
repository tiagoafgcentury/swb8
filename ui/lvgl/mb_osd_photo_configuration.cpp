#include "mb_osd_photo_configuration.h"
#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_osd_fonts.h"
#include "mb_events.h"
#include "common/mb_state_file.h"

#include <lvgl.h>

namespace mb {

OSD_Photo_Configuration::OSD_Photo_Configuration(OSD *_parent):
    OSD(_parent)
{
}

OSD_Photo_Configuration::~OSD_Photo_Configuration()
{
    DELETE_OBJ(m_main);
    DELETE_OBJ(m_title);
    remove_focus();
}

bool OSD_Photo_Configuration::handle_event_remote_control(const Event_Remote_Control &_event)
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

void OSD_Photo_Configuration::show_menu_photo_configuration(photo_configuration_callback_t _callback)
{
    set_focus();
    m_callback = _callback;
    // Cria a tela principal
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset, width, height, OSD_COLOR_BLACK);
    m_title = set_label_text(m_main, tr(__Configuracoes_de_exibicao_de_fotos), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 0);
    // Desenha linha separadora
    create_rect(m_main, line_x+line_w, line_y, 3, 4*line_h + 3*line_s, OSD_COLOR_ORANGE);
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
    m_keys.add_key(tr(__Slide_show), m_title_callbacks);
    m_keys.add_key(tr(__Tempo_de_exibicao), m_title_callbacks);
    m_keys.add_key(tr(__Tipo_de_transicao), m_title_callbacks);
	 m_keys.add_key(tr(__Relacao_de_aspecto), m_title_callbacks);
    // Slide show
    m_slide_show_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y);
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Habilitado), nullptr);
    m_keys.add_key(tr(__Desabilitado), nullptr);
    m_keys.set_group_enabled(m_slide_show_group, false);
    // Tempo de exibição
    m_exibition_time_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y + line_h + line_s);
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__3_segundos), nullptr);
    m_keys.add_key(tr(__5_segundos), nullptr);
    m_keys.add_key(tr(__10_segundos), nullptr);
    m_keys.set_group_enabled(m_exibition_time_group, false);
    // Tipo de transição
    m_transition_type_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y + 2*(line_h + line_s));
    m_keys.set_group_key_size(m_keys_w-20, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Para_esquerda), nullptr);
    m_keys.add_key(tr(__Para_direita), nullptr);
    m_keys.add_key(tr(__Para_cima), nullptr);
    m_keys.add_key(tr(__Para_baixo), nullptr);
    m_keys.set_group_enabled(m_transition_type_group, false);
    // Relação de aspecto
    m_aspect_ratio_group = m_keys.add_group();
    m_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_keys.set_group_pos(m_keys_x, m_keys_y + 3*(line_h + line_s));
    m_keys.set_group_key_size(m_keys_w, m_keys_h);
    m_keys.set_group_spacing(line_s, line_s);
    m_keys.set_group_align(LV_ALIGN_CENTER);
    m_keys.add_key(tr(__Manter), nullptr);
    m_keys.add_key(tr(__Ignorar), nullptr);
    m_keys.set_group_enabled(m_aspect_ratio_group, false);
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
    auto slide_show = file.slide_show == 0 ? 1 : 0; // Inverte valor para manter compatibilidade com versões anteriores
    auto slide_show_transition_time = file.slide_show_transition_time;
    auto slide_show_transition_type = file.slide_show_transition_type;
    auto slide_show_aspect_ratio = file.slide_show_aspect_ratio;
    m_keys.set_key_marked(m_slide_show_group, static_cast<int>(slide_show), true);
    m_keys.set_key_marked(m_exibition_time_group, static_cast<int>(slide_show_transition_time), true);
    m_keys.set_key_marked(m_transition_type_group, static_cast<int>(slide_show_transition_type), true);
    m_keys.set_key_marked(m_aspect_ratio_group, static_cast<int>(slide_show_aspect_ratio), true);
}

void OSD_Photo_Configuration::process_save_callback()
{
    auto selected_slide_show = m_keys.get_key_marked(m_slide_show_group) == 0 ? 1 : 0; // Inverte valor para manter compatibilidade com versões anteriores
    auto selected_exibition_time = m_keys.get_key_marked(m_exibition_time_group);
    auto selected_transition_type = m_keys.get_key_marked(m_transition_type_group);
    auto selected_aspect_ratio = m_keys.get_key_marked(m_aspect_ratio_group);
    State_File::App_State_File file;
    file.slide_show = static_cast<uint8_t>(selected_slide_show);
    file.slide_show_transition_time = static_cast<uint8_t>(selected_exibition_time);
    file.slide_show_transition_type = static_cast<uint8_t>(selected_transition_type);
    file.slide_show_aspect_ratio = static_cast<uint8_t>(selected_aspect_ratio);
    file.write();
    Task::post_event(std::bind(m_callback, true));
}

void OSD_Photo_Configuration::next_callback()
{
    m_keys.next(false);
}

void OSD_Photo_Configuration::previous_callback()
{
    m_keys.prev(false);
}

void OSD_Photo_Configuration::process_volup_callback()
{
    auto selected_func = m_keys.get_selected_key();
    auto group = selected_func == 0 ? m_slide_show_group : 
                (selected_func == 1) ? m_exibition_time_group : 
                (selected_func == 2) ? m_transition_type_group : 
                m_aspect_ratio_group;
    m_keys.next_marked(group);
}

void OSD_Photo_Configuration::process_voldown_callback()
{
    auto selected_func = m_keys.get_selected_key();
    auto group = selected_func == 0 ? m_slide_show_group : 
                (selected_func == 1) ? m_exibition_time_group : 
                (selected_func == 2) ? m_transition_type_group : 
                m_aspect_ratio_group;
    m_keys.prev_marked(group);
}

} // namespace mb
