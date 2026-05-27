#pragma once

#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_enhanced_keys.h"
#include "tasks/mb_task.h"
#include "mb_osd_subtitle_converter.h"

#include <memory>
#include <functional>

namespace mb {

class OSD_Subtitle_Configuration : public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> subtitle_configuration_callback_t;

private:
    subtitle_configuration_callback_t m_callback;
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

    static constexpr auto offset = 124;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto height = DISPLAY_HEIGHT - offset;
	lv_obj_t *m_main { nullptr };
    lv_obj_t *m_title { nullptr };
    lv_obj_t *m_font_size { nullptr };
    lv_obj_t *m_font_color { nullptr };
    lv_obj_t *m_background_color { nullptr };
    lv_obj_t *m_subtitle_box { nullptr };
    lv_obj_t *m_subtitle_text { nullptr };

    static constexpr auto line_x = 80;
    static constexpr auto line_y = 100;
    static constexpr auto line_w = 250; 
    static constexpr auto line_h = 50;
    static constexpr auto line_s = 20;

    // Grupos de teclas
    size_t m_title_group = 0;
    size_t m_size_group = 0;
    size_t m_color_group = 0;
    size_t m_background_group = 0;
    size_t m_save_group = 0;

    // Teclado
    MB_OSD_Enhanced_Keys m_keys;
    static constexpr auto m_keys_w = 220;
    static constexpr auto m_keys_h = 50;
    static constexpr auto m_keys_x = line_x + line_w + line_s;
    static constexpr auto m_keys_y = line_y;
    static constexpr auto m_keys_spacing = line_h + line_s;
    // Tecla salvar
    static constexpr auto m_save_x = (width - m_keys_w) / 2;
    static constexpr auto m_save_y = height - 3 * line_h + line_s;
    // Texto de exemplo
    static constexpr auto subt_y = height - 5 * line_h;
    static constexpr auto subt_w = 800;
    static constexpr auto subt_h = 80;
    static constexpr auto subt_x = (width - subt_w) / 2;

    std::map<Remote_Control_Key, MB_OSD_Enhanced_Keys::KeyCallback> m_title_callbacks = {
        { Remote_Control_Key::KEY_VOLDOWN, std::bind(&OSD_Subtitle_Configuration::process_voldown_callback, this) },
        { Remote_Control_Key::KEY_VOLUP, std::bind(&OSD_Subtitle_Configuration::process_volup_callback, this) },
        { Remote_Control_Key::KEY_CHDOWN, std::bind(&OSD_Subtitle_Configuration::next_callback, this) },
        { Remote_Control_Key::KEY_CHUP, std::bind(&OSD_Subtitle_Configuration::previous_callback, this) },
        { Remote_Control_Key::KEY_VOLTAR, std::bind(&OSD_Subtitle_Configuration::voltar_callback, this) },
    };

    std::map<Remote_Control_Key, MB_OSD_Enhanced_Keys::KeyCallback> m_save_callbacks = {
        { Remote_Control_Key::KEY_CHUP, std::bind(&OSD_Subtitle_Configuration::previous_callback, this) },
        { Remote_Control_Key::KEY_CHDOWN, std::bind(&OSD_Subtitle_Configuration::next_callback, this) },
        { Remote_Control_Key::KEY_VOLTAR, std::bind(&OSD_Subtitle_Configuration::voltar_callback, this) },
        { Remote_Control_Key::KEY_OK, std::bind(&OSD_Subtitle_Configuration::process_save_callback, this) },
    };

    void voltar_callback() { Task::post_event(std::bind(m_callback, false)); }
    void next_callback();
    void previous_callback();
    void process_font_callback();
    void process_color_callback();
    void process_background_callback();
    void process_save_callback();
    void draw_subtitle_preview();
    void process_volup_callback();
    void process_voldown_callback();

public:
    OSD_Subtitle_Configuration(OSD *_parent);
    virtual ~OSD_Subtitle_Configuration();
	 void show_menu_subtitle_configuration(subtitle_configuration_callback_t _callback);
};

} // namespace mb
