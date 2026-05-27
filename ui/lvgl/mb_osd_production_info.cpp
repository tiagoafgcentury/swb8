#include "mb_osd_production_info.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_osd_footer.h"

#include "common/mb_globals.h"
#include "common/mb_version.h"

#include "../../project_version.h"


namespace mb {

OSD_Production_Info::OSD_Production_Info(OSD *_parent):
    OSD(_parent)
{
}

OSD_Production_Info::~OSD_Production_Info()
{
    DELETE_OBJ(m_bgd_main);
    //remove_focus();
}

/*
bool OSD_Production_Info::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        default:
            return true;
    }

    return false;
}
*/
void OSD_Production_Info::show_production_info(Production_Info_CB_t _callback)
{
    //set_focus();
    m_callback = _callback;
    constexpr auto top = 20;
    constexpr auto space = 50;
    constexpr auto left = 20;
    m_bgd_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_move_foreground(m_bgd_main);
    lv_obj_null_on_delete(&m_bgd_main);
    lv_obj_set_style_bg_opa(m_bgd_main, LV_OPA_TRANSP, 0);
    auto m_main = create_rect(m_bgd_main, start_x, start_y, start_w, start_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_main, LV_OPA_100, 0);
    lv_obj_set_style_radius(m_main, 10, LV_PART_MAIN);
    lv_obj_align(m_main, LV_ALIGN_CENTER, 0, 0);
    auto text_model = MBGUI_PRODUCT_NAME " " MBGUI_MODEL_NAME;
    auto model = set_label_text(m_main, text_model, 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_align(model, LV_ALIGN_TOP_LEFT, left, top);
    auto text_version = std::string("Software: ") + MB_OSD_Version::get_full_version();
    auto version = set_label_text(m_main, text_version.data(), 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_align(version, LV_ALIGN_TOP_LEFT, left, top + (1 * space));
    m_scua = set_label_text(m_main, "SCUA: ", 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_scua);
    lv_obj_align(m_scua, LV_ALIGN_TOP_LEFT, left, top + (2 * space));
    m_caid = set_label_text(m_main, "CAID: ", 0, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_caid);
    lv_obj_align(m_caid, LV_ALIGN_TOP_LEFT, left, top + (3 * space));
    auto usb = set_label_text(m_main, "Pendrive detectado com sucesso!", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(usb, LV_ALIGN_TOP_LEFT, left, top + (4 * space));
    using namespace std::placeholders;
    m_process_fingerprint_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Production_Info::set_cas_fingerprint, this, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_fingerprint_cb);
    //MB_OSD_Footer::draw(m_main, tr(__Pressione_voltar_para_voltar), -20);
}

void OSD_Production_Info::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    auto text_caid = "CAID: " + _caid;
    auto text_scua = "SCUA: " + _scua;
    lv_label_set_text_fmt(m_caid, text_caid.data());
    lv_label_set_text_fmt(m_scua, text_scua.data());
}

} //mb
