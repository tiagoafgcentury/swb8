#include "mb_osd_software_update_finish.h"

#include "mb_menu_resources.h"
#include "mb_osd_translate.h"

#include "tasks/mb_task_application.h"
#include "tasks/mb_task_database.h"

#include "common/mb_globals.h"
#include "common/mb_version.h"

#include "fw_env.h"

#include "../../project_version.h"

#include <lvgl.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace mb {

OSD_Software_Update_finish::OSD_Software_Update_finish(OSD *_parent):
    OSD(_parent)
{
}

OSD_Software_Update_finish::~OSD_Software_Update_finish()
{
    DELETE_OBJ(m_main);
}

void OSD_Software_Update_finish::reset_env_flag()
{
    fw_env_open();
    fw_env_write("sw_updt_status_flag", "0");
    fw_env_close();
}

bool OSD_Software_Update_finish::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        {
            DEBUG_MSG(OSD, WARN, "OK\n");
            auto sw_updt_status_flag = fw_getenv_safe("sw_updt_status_flag");

            if (sw_updt_status_flag and strcmp(sw_updt_status_flag, "3") == 0)
            {
                Task::post_event(std::bind(m_callback, true));
            }
            else // sw_updt_status_flag != "3"
            {
                if (Lineup_Mutex_Ref::is_empty())
                {
                    Task::post_event_lineup_load();
                }

                Task::post_event(std::bind(m_callback, false));
            }

            reset_env_flag();
            return true;
        }

        default:
            break;
    }

    return true;
}

void OSD_Software_Update_finish::show_menu_software_update_finish(Software_Update_Finish_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    // MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    auto upg_status = fw_getenv_safe("upg_status");

    if (upg_status and strcmp(upg_status, "2") == 0)
    {
        auto title = set_label_text(m_main, tr(__Software_atualizado_com_sucesso), 0, 0, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, title_y);
        std::string txt = MB_OSD_Version::get_major_minor_version();//MB_OSD_Version::get_full_version();
        auto sw_version = set_label_text(m_main, txt.c_str(), 0, 0, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_align(sw_version, LV_ALIGN_TOP_MID, 0, title_y + 65);
    }
    else if ((upg_status and strcmp(upg_status, "1") == 0) || (upg_status and strcmp(upg_status, "3") == 0))
    {
        auto title = set_label_text(m_main, tr(__Falha_na_atualizacao_de_software), 0, 0, font_semi_40, OSD_COLOR_WHITE);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, title_y);
    }

    fw_env_open();
    fw_env_write("upg_status", "0");
    fw_env_close();
    // Rodapé
    MB_OSD_Footer::draw(m_main, tr(__Pressione_ok_para_continuar), footer_y);
    m_btn_ok = create_rect(m_main, button_x, button_y, button_w, button_h, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_btn_ok);
    lv_obj_set_style_radius(m_btn_ok, 25, DEFAULT_SELECTOR);
    m_lbl_ok = set_label_text_static(m_btn_ok, tr(__Finalizar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_lbl_ok);
    lv_obj_align(m_lbl_ok, LV_ALIGN_CENTER, 0, 0);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.init(m_main);
    Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_software));
}

} // namespace mb
