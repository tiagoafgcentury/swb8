
#include "mb_osd_box_info.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "mb_events.h"
#include "../../project_version.h"
#include "common/mb_version.h"


#include <lvgl.h>
#include <stdio.h>
#include <string.h>


namespace mb {

OSD_Box_Info::OSD_Box_Info(OSD *_parent):
    OSD(_parent)
{
}

OSD_Box_Info::~OSD_Box_Info()
{
    remove_focus();
}

bool OSD_Box_Info::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        case Remote_Control_Key::KEY_MENU:
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Box_Info::show_box_info(lv_obj_t *_bgd, Box_Info_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    lv_area_t coords;
    lv_obj_get_coords(_bgd, &coords);
    lv_point_t pos;
    std::string text;
    text += tr(__Modelo);
    text += ": ";
    auto product_box = set_label_text(_bgd, text.c_str(), 20, 60, font_20, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(product_box, text.size(), &pos);
    set_label_text(_bgd, MBGUI_PRODUCT_NAME " " MBGUI_MODEL_NAME, pos.x + 20, 60, font_semi_20, OSD_COLOR_WHITE);
    text = tr(__Versao_do_Software);
    text += ": ";
    auto model_box = set_label_text(_bgd, text.c_str(), 20, 90, font_20, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(model_box, text.size(), &pos);
    auto txt = MB_OSD_Version::get_major_minor_version();
    set_label_text(_bgd, txt.c_str(), pos.x + 20, 90, font_semi_20, OSD_COLOR_WHITE);
    text = tr(__Ativacao);
    text += ": ";
    auto activation_box = set_label_text(_bgd, text.c_str(), 20, 120, font_20, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(activation_box, text.size(), &pos);
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    set_label_text(_bgd, sat_config.name.c_str(), pos.x + 20, 120, font_semi_20, OSD_COLOR_WHITE);
    m_instruction_box = create_rect(_bgd, instruction_x, instruction_y, instruction_w, instruction_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_instruction_box);
    m_instruction = set_label_text(m_instruction_box, tr(__Aponte_a_camera_qrcode_ativar), 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_instruction);
    lv_label_set_long_mode(m_instruction, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(m_instruction, instruction_w);
    lv_obj_set_style_text_align(m_instruction, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(m_instruction, LV_ALIGN_CENTER, 0, 0);
    // Cria qrcode do sathdregional
    m_qrcode = create_qrcode(_bgd, 96);
    lv_obj_null_on_delete(&m_qrcode);
    lv_obj_align(m_qrcode, LV_ALIGN_DEFAULT, 20, 200);
    text = tr(__ID_do_Receptor);
    auto caid_box = set_label_text(_bgd, text.c_str(), 20, 0, font_20, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(caid_box, text.size(), &pos);
    DEBUG_MSG(OSD, DEBUG, "caid_pos: " << pos.x << " " << pos.y << "\n");
    m_caid_number = set_label_text(_bgd, "", pos.x + 30, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_caid_number);
    text = tr(__Codigo_de_Acesso);
    auto scua_box = set_label_text(_bgd, text.c_str(), 20, 30, font_20, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(scua_box, text.size(), &pos);
    m_scua_number = set_label_text(_bgd, "", pos.x + 30, 30, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_scua_number);
    using namespace std::placeholders;
    // Este callback TEM que estar posicionado após a criação de m_qrcode
    m_process_fingerprint_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Box_Info::set_cas_fingerprint, this, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_fingerprint_cb);
}

void OSD_Box_Info::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    lv_label_set_text(m_caid_number, _caid.c_str());
    lv_label_set_text(m_scua_number, _scua.c_str());
    qrcode_update(Config::get_config()->selected_satellite_config().network_id, m_qrcode, std::move(_caid), std::move(_scua));
}

void OSD_Box_Info::got_focus()
{
}

} // namespace mb
