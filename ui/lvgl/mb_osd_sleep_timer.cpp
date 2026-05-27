#include <iostream>
#include <map>

#include "mb_osd_sleep_timer.h"
#include "mb_osd_fonts.h"
#include "mb_menu_resources.h"
#include "common/mb_globals.h"
#include "hal/mb_system.h"

#include "tasks/mb_task.h"

namespace {
}

namespace mb {

OSD_Sleep_Timer OSD_Sleep_Timer::s_instance(nullptr);

int OSD_Sleep_Timer::s_sleep_timer_value = 0;
lv_timer_t *OSD_Sleep_Timer::s_sleep_timer = nullptr;

OSD_Sleep_Timer::OSD_Sleep_Timer(OSD *_parent):
    OSD(_parent)
{
}

OSD_Sleep_Timer::~OSD_Sleep_Timer()
{
    DELETE_OBJ(m_main_menu);
    DELETE_TIMER(m_exit_timer);
    remove_focus();
}

// Processa tecla recebida
bool OSD_Sleep_Timer::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            break;

        case Remote_Control_Key::KEY_SLEEP:
            change_sleep_timer_value();
            update_sleep_timer_value();
            lv_timer_reset(m_exit_timer);
            break;

        case Remote_Control_Key::KEY_OK:
            set_sleep_timer_value();
            Task::post_event(m_callback);
            break;

        default:
            break;
    }

    return true;
}

void OSD_Sleep_Timer::update_sleep_timer_value()
{
    auto [_, text] = get_sleep_timer_pair();
    lv_label_set_text_fmt(m_lbl_timer, "%s: %s", tr(__Tempo_para_desligar).data(), text.c_str());
}

void OSD_Sleep_Timer::show_sleep_timer(Sleep_Timer_CB_t _callback)
{
    m_callback = _callback;
    set_focus();

    if (!m_main_menu)
    {
        m_main_menu = create_rect(get_main_screen(OSD_Layer::SLEEP_TIMER_LAYER), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main_menu);
        lv_obj_set_style_bg_opa(m_main_menu, LV_OPA_TRANSP, 0);
        lv_obj_move_background(m_main_menu);
        m_bgd_timer = create_rect(m_main_menu, st_start_x, st_start_y, st_start_w, st_start_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_timer);
        lv_obj_set_style_bg_opa(m_bgd_timer, OSD_TRANPARENCY_LIVE, 0);
        lv_obj_set_style_radius(m_bgd_timer, 25, DEFAULT_SELECTOR);
        m_lbl_timer = set_label_text(m_bgd_timer, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_lbl_timer);
        lv_obj_set_width(m_lbl_timer, st_start_w - 40);
        lv_obj_align(m_lbl_timer, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(m_lbl_timer, LV_TEXT_ALIGN_LEFT, 0);
        update_sleep_timer_value();

        if (!m_exit_timer)
        {
            m_exit_timer = lv_timer_create(process_exit_timer_cb, 5000, this);
        }
        else
        {
            lv_timer_reset(m_exit_timer);
        }
    }
}

void OSD_Sleep_Timer::process_exit_timer_cb(lv_timer_t *_tm)
{
    OSD_Sleep_Timer *thiz = static_cast<OSD_Sleep_Timer *>(lv_timer_get_user_data(_tm));
    thiz->set_sleep_timer_value();
    Task::post_event(std::bind(thiz->m_callback));
}

void OSD_Sleep_Timer::change_sleep_timer_value()
{
    // Varre o mapa de tempos
    int size = sizeof(sleep_timer_value_array) / sizeof(sleep_timer_value_array[0]);

    for (int i = 0; i < size ; ++i)
    {
        DEBUG_MSG(OSD, DEBUG, "Valor do timer : " << dec << sleep_timer_value_array[i] << std::endl);

        if (s_sleep_timer_value == sleep_timer_value_array[i])
        {
            s_sleep_timer_value = (i + 1) == size ? sleep_timer_value_array[0] : sleep_timer_value_array[i + 1];
            break;
        }

        if (s_sleep_timer_value < sleep_timer_value_array[i])
        {
            s_sleep_timer_value = sleep_timer_value_array[i];
            break;
        }
    }
}

// Inicializa idioma com parâmetro recebido
std::pair<int, std::string> OSD_Sleep_Timer::set_sleep_timer_value()
{
    std::pair<int, std::string> res;
    // Texto em função do valor do timer
    res = get_sleep_timer_pair();

    // Inicia ou finaliza timer de acordo com valor calculado
    if (s_sleep_timer_value)
    {
        DEBUG_MSG(OSD, DEBUG, "Iniciando timer de " << res.second << "\n");
        start_sleep_timer();
    }
    else
    {
        delete_sleep_timer();
    }

    return res;
}

int OSD_Sleep_Timer::get_sleep_timer_value()
{
    return s_sleep_timer_value;
}

void OSD_Sleep_Timer::delete_sleep_timer()
{
    DEBUG_MSG(OSD, DEBUG, "Zerando sleep timer\n");

    if (s_sleep_timer)
    {
        lv_timer_pause(s_sleep_timer);
        DELETE_TIMER(s_sleep_timer);
    }

    s_sleep_timer_value = 0;
}

std::pair<int, std::string> OSD_Sleep_Timer::get_sleep_timer_pair()
{
    std::string text = "";
    int hour = s_sleep_timer_value / 60;
    int min = s_sleep_timer_value % 60;
    // Cria texto referente a hora
    std::string hour_text = "";
    hour_text = hour >= 1 ? " h" : "";

    if (hour_text.size())
    {
        hour_text = std::to_string(hour) + hour_text;
    }

    // Cria texto referente ao minuto
    std::string min_text = "";
    min_text = min >= 1 ? " min" : "";

    if (min_text.size())
    {
        min_text = std::to_string(min) + min_text;
    }

    // Concatena os 2 textos e monta resultado
    if (hour_text.size())
    {
        text = hour_text;
    }

    if (hour_text.size() && min_text.size())
    {
        text += " e ";
    }

    if (min_text.size())
    {
        text += min_text;
    }

    if (text.size() == 0)
    {
        text = "Deslig.";
    }

    return {s_sleep_timer_value, text};
}

void OSD_Sleep_Timer::start_sleep_timer()
{
    // Inicia timer para apagar informações do canal
    DELETE_TIMER(s_sleep_timer);
    s_sleep_timer = lv_timer_create(process_sleep_timer_cb, 60 * 1000, this);
    DEBUG_MSG(OSD, DEBUG, "Iniciando timer: " << dec << s_sleep_timer_value << " " << hex << (void *)s_sleep_timer << "\n");
    // Tempo para fim do timer
    lv_timer_set_repeat_count(s_sleep_timer, s_sleep_timer_value);
}

void OSD_Sleep_Timer::process_sleep_timer()
{
    if (--s_sleep_timer_value == 0)
    {
        delete_sleep_timer();
        start_standby();
    }

    /*
        else
        {
            if (m_sleep_timer)
            {
                lv_timer_reset(m_sleep_timer);
            }
        }
    */
    std::pair<int, std::string> current = get_sleep_timer_pair();
    DEBUG_MSG(OSD, DEBUG, "sleep timer = " << current.second << "\n");
}

void OSD_Sleep_Timer::process_sleep_timer_cb(lv_timer_t *_tm)
{
    OSD_Sleep_Timer *thiz = static_cast<OSD_Sleep_Timer *>(lv_timer_get_user_data(_tm));
    thiz->process_sleep_timer();
}

void OSD_Sleep_Timer::start_standby()
{
    Task::post_event_toggle_power();
}

} // namespace mb
