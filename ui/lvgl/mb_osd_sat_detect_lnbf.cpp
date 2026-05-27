#include "mb_osd_sat_detect_lnbf.h"
#include "mb_osd_footer.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"

#include <lvgl.h>
#include <stdio.h>

namespace mb {

OSD_Sat_Detect_Lnbf::OSD_Sat_Detect_Lnbf(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, btn_w, btn_h, btn_s, btn_x, btn_y)
{
    lv_style_init(&m_style);
    lv_style_init(&m_style_progbar);
}


OSD_Sat_Detect_Lnbf::~OSD_Sat_Detect_Lnbf()
{
    remove_focus();
    lv_style_reset(&m_style);
    lv_style_reset(&m_style_progbar);
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_main_screen);
}

bool OSD_Sat_Detect_Lnbf::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        case Remote_Control_Key::KEY_OK:
        {
            if (m_status == Status::Detect)
            {
                Task::post_event(m_callback);
            }
            else if (m_status == Status::Fail)
            {
                auto pressed = m_keys.get_selected();

                if (pressed == 0)
                {
                    Task::post_event(m_callback);
                }
                else if (pressed == 1)
                {
                    to_detect();
                }
            }
            else if (m_status == Status::Success)
            {
                Task::post_event(m_callback);
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLUP:
        {
            m_keys.next();
            break;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            m_keys.previous();
            break;
        }

        default:
            return true;
    }

    return true;
}

void OSD_Sat_Detect_Lnbf::osd_sat_detect_lnbf(Sat_Detect_Lnbf_CB_t _callback, lv_area_t _area, Satellite _sat)
{
    set_focus();
    m_callback = _callback;
    m_satellite = _sat;
    m_main_screen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), _area.x1, _area.y1, _area.x2, _area.y2, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_screen);
    lv_obj_set_style_bg_opa(m_main_screen, LV_OPA_90, 0);
    // Desenha linha
    m_line = create_rect(m_main_screen, 0, 0, 3, _area.y2, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line);
    // Criar barra de progresso
    create_progress_bar();
    print_satellite_config();

    // Cria timer para 10 segundos
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    to_detect();
    // Inicia detecção automática do LNBF
    looking_for_sat_config();
}

void OSD_Sat_Detect_Lnbf::looking_for_sat_config()
{
    m_detect_lnbf = std::make_unique<MB_Detect_Lnbf>(this);
    m_detect_lnbf->lnbf_detection_start([this](bool _result, Transponder_Id /*tp*/)
    {
        if (_result)
        {
            DEBUG_MSG(OSD, DEBUG, "Success\n");
            to_success();
        }
        else
        {
            DEBUG_MSG(OSD, DEBUG, "Failed\n");
            to_fail();
        }

        m_detect_lnbf.reset();
    }, m_satellite);
}

void OSD_Sat_Detect_Lnbf::print_satellite_config()
{
    // Busca informações do canal atual
    auto [tp, is_locked] = Task::s_task_tuner->get_current_transponder();
    auto frequency = tp.frequency() / 1000;
    auto polarity = tp.polarity();
    auto pol = polarity == Polarity::Horizontal ? "H" : "V";
    auto tp_complete = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(tp);
    auto sr = tp_complete ? tp_complete->symbol_rate : 0;
    // Barra de informações
    char buf1[256] = { 0 };
    char buf2[256] = { 0 };
    auto config = Config::get_config();
    const auto lnbf = OSD_Translate::translate(config->lnbf_type());
    m_info_box = create_rect(m_main_screen, 0, 0, info_w, info_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_info_box);
    snprintf(buf1, sizeof(buf1), "%s: %s - %s - %s ", tr(__Satelite).data(), m_satellite.name.data(), config->band() == Band::Ku ? tr(__Banda_KU).data() : tr(__Banda_C).data(), lnbf.data());
    snprintf(buf2, sizeof(buf2), "%s | %s: %dMHz / %dKbps / %s", tr(__Normal).data(), tr(__Canal_de_referencia).data(), frequency, sr, pol);
    m_info1 = set_label_text(m_info_box, buf1, 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_info1);
    lv_obj_align(m_info1, LV_ALIGN_TOP_MID, 0, 0);
    m_info2 = set_label_text(m_info_box, buf2, 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_info2);
    lv_obj_align(m_info2, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_align(m_info_box, LV_ALIGN_CENTER, 0, -50);
}

void OSD_Sat_Detect_Lnbf::create_progress_bar()
{
    lv_style_set_bg_opa(&m_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_style_progbar, OSD_COLOR_BLUE);
    m_prgbar = lv_bar_create(m_main_screen);
    lv_obj_null_on_delete(&m_prgbar);
    lv_obj_set_width(m_prgbar, prgbar_w);
    lv_obj_set_height(m_prgbar, prgbar_h);
    lv_obj_align(m_prgbar, LV_ALIGN_CENTER, 0, 0);
    lv_style_set_anim_duration(&m_style, ANIM_BAR_DURATION);
    lv_obj_add_style(m_prgbar, &m_style_progbar, LV_PART_INDICATOR);
    lv_obj_add_style(m_prgbar, &m_style, LV_PART_MAIN);
    /*Create a label below the slider*/
    m_pgr_label = set_label_text(m_main_screen, "70%", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_pgr_label);
    lv_obj_align(m_pgr_label, LV_ALIGN_CENTER, 0, 30);
}

void OSD_Sat_Detect_Lnbf::refresh_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Sat_Detect_Lnbf *thiz = static_cast<OSD_Sat_Detect_Lnbf *>(lv_timer_get_user_data(_tm));
    thiz->refresh_progress();
}

void OSD_Sat_Detect_Lnbf::refresh_progress()
{
    switch (m_status)
    {
        case Status::Detect:
            detect();
            break;

        case Status::Fail:
            fail();
            break;

        case Status::Success:
            success();
            break;
    }
}

void OSD_Sat_Detect_Lnbf::success()
{
}

void OSD_Sat_Detect_Lnbf::detect()
{
    m_current_progress += 1;

    if (m_current_progress <= 100)
    {
        lv_slider_set_value(m_prgbar, m_current_progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_pgr_label, "%d%%", m_current_progress);
    }
}

void OSD_Sat_Detect_Lnbf::fail()
{
    if (m_current_progress)
    {
        --m_current_progress;
    }
}

void OSD_Sat_Detect_Lnbf::populate(Status _status)
{
    m_status = _status;
    const auto &sc = m_screen_content[_status];
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main_screen, sc.title, 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, 15);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    m_subtitle = set_label_text(m_main_screen, sc.subtitle, 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, 45);
    // Rodapé
    DELETE_OBJ(m_footer);
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_main_screen, tr(__Pressione_ok_para_continuar), -20);
    lv_obj_null_on_delete(&m_footer);
    // Seta cor do slider
    lv_obj_set_style_bg_color(m_prgbar, sc.bar_color, LV_PART_INDICATOR);
    // Altera status e redesenha teclas
    lv_timer_set_period(m_refresh_timer, sc.period);
    draw_buttons();
}

void OSD_Sat_Detect_Lnbf::to_detect()
{
    populate(Status::Detect);
}

void OSD_Sat_Detect_Lnbf::to_success()
{
    populate(Status::Success);
    m_current_progress = 0;
    lv_label_set_text(m_pgr_label, "100%");
    lv_slider_set_value(m_prgbar, 100, LV_ANIM_ON);
}

void OSD_Sat_Detect_Lnbf::to_fail()
{
    populate(Status::Fail);
    m_current_progress = 30;
}

void OSD_Sat_Detect_Lnbf::draw_buttons()
{
    switch (m_status)
    {
        case Status::Detect :
        {
            m_keys.clear();
            m_keys.set_background(m_main_screen);
            m_keys.add_label(tr(__Voltar));
            m_keys.set_x(150);
            m_keys.set_y(500);
            m_keys.set_width(260);
            m_keys.draw();
            m_keys.select();
        }
        break;

        case Status::Fail :
        {
            m_keys.clear();
            m_keys.set_background(m_main_screen);
            m_keys.add_label(tr(__Detectar_novamente));
            m_keys.add_label(tr(__Proximo));
            m_keys.set_x(10);
            m_keys.set_y(500);
            m_keys.set_width(260);
            m_keys.set_spacing(270);
            m_keys.draw();
            m_keys.select();
        }
        break;

        case Status::Success :
        {
            m_keys.clear();
            m_keys.set_background(m_main_screen);
            m_keys.add_label(tr(__Proximo));
            m_keys.set_x(150);
            m_keys.set_y(500);
            m_keys.set_width(260);
            m_keys.draw();
            m_keys.select();
        }
        break;
    }
}

void OSD_Sat_Detect_Lnbf::got_focus()
{
}

} // namespace mb
