#include "mb_osd_software_update_forced.h"
#include "mb_osd_choose_home_satellite.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"
#include "mb_zone_id.h"
#include "tasks/mb_task_osd.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "common/mb_version.h"

#include "hal/mb_system.h"

#include "../../project_version.h"

#include <lvgl.h>
#include <stdlib.h>
#include <string.h>

namespace mb {

OSD_Software_Update_Forced::OSD_Software_Update_Forced(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        tr(__Satelite),
        "USB",
    };
}

OSD_Software_Update_Forced::~OSD_Software_Update_Forced()
{
    DELETE_TIMER(m_timer_get_cas_fingerprint);
    DELETE_OBJ(m_main);
    DELETE_OBJ(m_bgd_main);
    DELETE_OBJ(m_bgd_info);
    Osd_Breadcrumb::s_instance.remove_name(true);
    if (Osd_Breadcrumb::s_instance.is_empty())
    {
        Osd_Breadcrumb::s_instance.clear();
    }

    remove_focus();
}

bool OSD_Software_Update_Forced::handle_event_remote_control(const Event_Remote_Control &_event)
{
    auto active = static_cast<Func_Active>(m_keys.get_selected());

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            if (m_is_informations)
            {
                Osd_Breadcrumb::s_instance.remove_name();
                m_is_informations = false;
                lv_obj_add_flag(m_bgd_info, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                if (active == Func_Active::Satelite)
                {
                    auto config = Config::get_config();
                    config->set_satellite_by_network_policies(Network_Policies::TVRO);
                    Satellite claro_sat = config->get_satellite_by_network_policies();
                    call_lbnf_detection(claro_sat);
                }
                else if (active == Func_Active::USB)
                {
                    looking_for_software_update(false);
                }
            }

            return true;

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
            if (m_is_informations == false)
            {
                m_keys.next();
            }

            return true;

        case Remote_Control_Key::KEY_INFO:
            if (m_is_informations == false)
            {
                Osd_Breadcrumb::s_instance.add_name(tr(__Informacoes_do_Receptor));
                m_is_informations = true;
                lv_obj_remove_flag(m_bgd_info, LV_OBJ_FLAG_HIDDEN);
            }

            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Software_Update_Forced::show_menu_updt_forced(OSD_Software_Update_Forced_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    m_bgd_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_main);
    lv_obj_move_background(m_bgd_main);
    // Tela principal
    m_main = create_rect(m_bgd_main, 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.draw();
    m_keys.select();
    // Título
    m_title = set_label_text_static(m_main, title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtitulo
    m_subtitle = set_label_text_static(m_main, subtitle, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, subtitle_y);
    std::string txt = "Versão atual software: " + MB_OSD_Version::get_major_minor_version();
    auto m_sw_version = set_label_text(m_main, txt.c_str(), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(m_sw_version, LV_ALIGN_TOP_MID, 0, subtitle_y + 45);
    // Cria rodapé
    MB_OSD_Footer::draw(m_main, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);

    if (Osd_Breadcrumb::s_instance.is_initialized() == false)
    {
        Osd_Breadcrumb::s_instance.init(m_bgd_main);
    }

    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_software));
    MB_OSD_Footer::draw(m_main, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);
    create_informations_screen();
}

void OSD_Software_Update_Forced::call_lbnf_detection(Satellite satellite)
{
    auto callback = [satellite, this](bool _result, Transponder_Id)
    {
        lv_obj_remove_flag(m_main, LV_OBJ_FLAG_HIDDEN);
        m_osd_lnbf_detection.reset();

        if (_result)
        {
            looking_for_software_update(true);
            //Task::post_event(std::bind(m_callback, true));
        }
    };

    if (!m_osd_lnbf_detection)
    {
        m_osd_lnbf_detection = std::make_shared<OSD_Lnbf_Detection>(this);
        Task_OSD::set_lnbf_detection(m_osd_lnbf_detection);
    }

    m_osd_lnbf_detection->show_menu_lnbf_detection(callback, satellite);
}

void OSD_Software_Update_Forced::looking_for_software_update(bool update_mode)
{
    auto callback = [this](bool _result)
    {
        (void)_result;
        m_osd_software_update.reset();
    };

    if (!m_osd_software_update)
    {
        m_osd_software_update = std::make_unique<OSD_Software_Update>(this);
    }

    m_osd_software_update->show_menu_software_update(callback, update_mode, false);
}

void OSD_Software_Update_Forced::create_informations_screen()
{
    m_bgd_info = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_info);
    auto title = set_label_text(m_bgd_info, tr(__Informacoes_do_Receptor), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    auto box_info = create_rect(m_bgd_info, 0, 0, width / 2, heigth / 2, OSD_COLOR_BLACK);
    lv_obj_align(box_info, LV_ALIGN_TOP_MID, 0, 100);
    std::string text;
    lv_point_t pos;
    text = tr(__Modelo);
    text += std::string(": ");
    auto product_box = set_label_text(box_info, text.c_str(), 20, 0, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(product_box, text.size(), &pos);
    set_label_text(box_info, MBGUI_PRODUCT_NAME " " MBGUI_MODEL_NAME, pos.x + 20, 0, font_semi_25, OSD_COLOR_WHITE);
    text = tr(__ID_do_Receptor);
    auto caid_box = set_label_text(box_info, text.c_str(), 20, 35, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(caid_box, text.size(), &pos);
    DEBUG_MSG(OSD, WARN, "caid_pos: " << pos.x << " " << pos.y << "\n");
    m_caid_number = set_label_text(box_info, "", pos.x + 30, 35, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_caid_number);
    text = tr(__Codigo_de_Acesso);
    auto scua_box = set_label_text(box_info, text, 20, 70, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(scua_box, text.size(), &pos);
    m_scua_number = set_label_text(box_info, "", pos.x + 30, 70, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_scua_number);
    text = tr(__Versao_do_Software);
    text += ": ";
    auto model_box = set_label_text(box_info, text, 20, 105, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(model_box, text.size(), &pos);
    auto ver_txt = MB_OSD_Version::get_major_minor_patch_version();
    set_label_text(box_info, ver_txt.data(), pos.x + 20, 105, font_semi_25, OSD_COLOR_WHITE);
    auto nuid_box = set_label_text(box_info, "NUID:", 20, 140, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(nuid_box, text.size(), &pos);
    m_nuid_number = set_label_text(box_info, "", pos.x + 30, 140, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_nuid_number);
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    text = "Zone ID:";
    char buffer[20];
    sprintf(buffer, "%d", (int)Zone_ID::get_zone_id(_oper));
    auto zone_id = set_label_text(box_info, text, 20, 175, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(zone_id, text.size(), &pos);
    set_label_text(box_info, buffer, pos.x + 30, 175, font_semi_25, OSD_COLOR_WHITE);
    text = tr(__Data_de_fabricacao);
    text += ": ";
    auto manufacture_date_txt = cat(MBGUI_MANUFACTURE_DATE_FILE);
    auto manufacture_date = set_label_text(box_info, text, 20, 210, font_25, OSD_COLOR_WHITE);
    lv_label_get_letter_pos(manufacture_date, text.size(), &pos);
    set_label_text(box_info, manufacture_date_txt, pos.x + 30, 210, font_semi_25, OSD_COLOR_WHITE);
    auto button = create_rect(m_bgd_info, 0, 0, 220, 50, OSD_COLOR_ORANGE);
    lv_obj_set_style_radius(button, 25, DEFAULT_SELECTOR);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -100);
    auto lbl_ok = set_label_text(button, tr(__Voltar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl_ok, LV_ALIGN_CENTER, 0, 0);
    get_cas_fingerprint();
    MB_OSD_Footer::draw(m_bgd_info, tr(__Pressione_ok_para_continuar), footer_y);
    lv_obj_add_flag(m_bgd_info, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Software_Update_Forced::get_cas_fingerprint_callback(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Software_Update_Forced *>(lv_timer_get_user_data(_timer));
    thiz->get_cas_fingerprint();
}

void OSD_Software_Update_Forced::get_cas_fingerprint()
{
    using namespace std::placeholders;
    // Este callback TEM que estar posicionado após a criação de m_qrcode
    m_process_fingerprint_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Software_Update_Forced::set_cas_fingerprint, this, _1, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_fingerprint_cb);
}

void OSD_Software_Update_Forced::set_cas_fingerprint(NAGRA_NUID_t _nuid, NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    if (_scua.empty() or _nuid.empty() or _caid.empty())
    {
        if (not m_timer_get_cas_fingerprint)
        {
            m_timer_get_cas_fingerprint = lv_timer_create(&OSD_Software_Update_Forced::get_cas_fingerprint_callback, 500, this);
            lv_timer_set_repeat_count(m_timer_get_cas_fingerprint, 3);
        }
    }
    else
    {
        DELETE_TIMER(m_timer_get_cas_fingerprint);
    }

    lv_label_set_text(m_caid_number, _caid.c_str());
    lv_label_set_text(m_scua_number, _scua.c_str());
    lv_label_set_text(m_nuid_number, _nuid.c_str());
}

void OSD_Software_Update_Forced::got_focus()
{
}

} // namespace mb


