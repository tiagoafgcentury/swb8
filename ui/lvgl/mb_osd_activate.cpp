#include "mb_osd_activate.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_osd_footer.h"
#include "mb_osd_fonts.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"

#include "tasks/mb_task.h"
#include "tasks/mb_task_application.h"

#include "hal/mb_sound.h"

#include <lvgl.h>
#include <stdio.h>

namespace mb {

OSD_Activate *OSD_Activate::s_instance { nullptr };

OSD_Activate::OSD_Activate(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    s_instance = this;
}

OSD_Activate::~OSD_Activate()
{
    DELETE_OBJ(m_main);
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name();
}

bool OSD_Activate::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        {
            if (m_status == Status::Start)
            {
                to_success();
            }
            else if (m_status == Status::Success)
            {
                Sound::get_instance()->set_volume(50);
                Task::post_event_display_clear();
                Task::post_event_easy_install_save(true);
                Task::post_event(std::bind(m_callback, true));
                auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();

                if (not current_lineup->services.empty())
                {
                    auto service = &current_lineup->services[0];
                    Task::post_event_channel_change(POST_CALLER service);
                }
                Task::post_event_clock_need_update();
            }

            break;
        }

        default:
            return true;
    }

    return true;
}

void OSD_Activate::show_menu_activate(Activate_CB_t _callback, bool _stb_activated)
{
    set_focus();
    m_callback = _callback;
    //MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    m_keys.set_background(m_main);
    // desenha instruções
    auto txt = std::string(tr(__Acesse)) + ":\n" + MBGUI_CENTURY_HOMEPAGE;
    m_instructions = set_label_text(m_main, txt.c_str(), instructions_x, instructions_y, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_instructions);
    m_point = set_label_text(m_main, tr(__Aponte_a_camera_qrcode_ativar), point_x, point_y, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_point);
    lv_obj_set_width(m_point, 176);
    lv_obj_set_height(m_point, 90);
    m_line = create_rect(m_main, line_x, line_y, line_w, line_h, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_line);
    m_or = create_rect(m_main, or_x, or_y, or_w, or_h, OSD_COLOR_BLACK);
    auto or_label = set_label_text_static(m_or, tr(__Ou), or_w, or_y, font_20, OSD_COLOR_WHITE);
    lv_obj_align(or_label, LV_ALIGN_CENTER, 0, 0);
    if(not _stb_activated)
    {
        m_qrcode = create_qrcode(m_main, 96);
        lv_obj_align(m_qrcode, LV_ALIGN_DEFAULT, box_x, 190);
        lv_obj_null_on_delete(&m_qrcode);
        m_caid = set_label_text_static(m_main, "CAID: ", instructions_x, caid_y, font_20, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_caid);
        m_scua = set_label_text_static(m_main, "SCUA: ", instructions_x, scua_y, font_20, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_scua);
        using namespace std::placeholders;
        // Este callback TEM que estar posicionado após a criação de m_qrcode
        m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Activate::set_cas_fingerprint, this, _2, _3));
        Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
        // Desenha botão
        draw_button();
        // Caminho de migalhas
        Osd_Breadcrumb::s_instance.add_name(tr(__Ativ_do_Midiabox));
        // Finaliza tela
        populate(Status::Start);
    }
    else
    {
        to_success();
    }

}

void OSD_Activate::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    if (m_caid and m_scua and m_qrcode)
    {
        lv_label_set_text_fmt(m_caid, "CAID: %s", _caid.c_str());
        lv_label_set_text_fmt(m_scua, "SCUA: %s", _scua.c_str());
        qrcode_update(Config::get_config()->selected_satellite_config().network_id, m_qrcode, std::move(_caid), std::move(_scua));
    }
}

void OSD_Activate::populate(Status st)
{
    m_status = st;
    const auto &sc = m_screen_content[st];
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main, sc.title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    m_subtitle = set_label_text(m_main, sc.subtitle, 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, subtitle_y);
}

void OSD_Activate::to_success()
{
    // Apaga itens não usados
    DELETE_OBJ(m_instructions);
    DELETE_OBJ(m_caid);
    DELETE_OBJ(m_scua);
    DELETE_OBJ(m_qrcode);
    DELETE_OBJ(m_or);
    DELETE_OBJ(m_line);
    DELETE_OBJ(m_point);
    Osd_Breadcrumb::s_instance.replace_last_name(tr(__Instalacao_concluida));
    // Conteúdo do título
    char buf1[1024] = { 0 };
    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    snprintf(buf1, sizeof(buf1), "%s %s", tr(__Instalacao_concluida_com_sucesso_para_o_satelite).data(), sat_config.name.data());
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main, buf1, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_long_mode(m_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(m_title, title_w);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    m_subtitle = set_label_text(m_main, tr(__Aproveite_o_seu), 555, 371 - offset_y, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    // Imagem
    m_logo_b8 = load_image(m_main, LOGO_MIDIABOX_BRANCO_518x75, 381, 400 - offset_y, 518, 75);
    lv_obj_null_on_delete(&m_logo_b8);
    // Redesenha teclas
    m_keys.clear();
    m_keys.add_label(tr(__Finalizar));
    m_keys.draw();
    m_keys.select();
    m_status = Status::Success;
    load_image(m_main, LOGO_HELIO, 250, 300 - offset_y, 133, 200);
}

void OSD_Activate::to_fail()
{
    populate(Status::Fail);
    // Apaga itens não usados
    DELETE_OBJ(m_instructions);
    DELETE_OBJ(m_caid);
    DELETE_OBJ(m_scua);
    DELETE_OBJ(m_qrcode);
    DELETE_OBJ(m_or);
    DELETE_OBJ(m_line);
    DELETE_OBJ(m_point);
    // Reposiciona título e subtítulo
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 277 - offset_y);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, 344 - offset_y);
}

void OSD_Activate::draw_button()
{
    m_keys.clear();
    m_keys.add_label(tr(__Proximo));
    m_keys.draw();
    m_keys.select();
}

void OSD_Activate::hide_menu()
{
    remove_focus();
    Task::post_event(std::bind(m_callback, true));
}

void OSD_Activate::got_focus()
{
}

} // namespace mb
