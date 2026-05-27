#include "common/mb_globals.h"
#include "mb_osd_menu_plus_sleep.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_clock.h"
#include "tasks/mb_task.h"
#include "common/mb_lineup.h"
#include "hal/mb_system.h"

#include <lvgl.h>
#include <filesystem>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <fstream>

namespace mb {

int OSD_Menu_Plus_Sleep::s_sleep_timer_value = 0;
lv_timer_t *OSD_Menu_Plus_Sleep::s_sleep_timer = nullptr;
lv_obj_t *OSD_Menu_Plus_Sleep::s_sleep_logo = nullptr;

OSD_Menu_Plus_Sleep::OSD_Menu_Plus_Sleep(OSD *_parent):
    OSD(_parent)
{
    DELETE_TIMER(s_sleep_timer);
}

OSD_Menu_Plus_Sleep::~OSD_Menu_Plus_Sleep()
{
    // Atualiza timer com o valor processado
    set_sleep_timer_value();

    remove_focus();
    DELETE_OBJ(m_box_sleep);
    DELETE_TIMER(m_exit_timer);
}

bool OSD_Menu_Plus_Sleep::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Reinicia timer
    if(m_exit_timer)
    {
        lv_timer_reset(m_exit_timer);
    }

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(std::bind(m_callback));
            return true;

        case Remote_Control_Key::KEY_OK:
        case Remote_Control_Key::KEY_VOLUP:
            sleep_timer_next();
            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
            sleep_timer_previous();
            return true;

        default:
            return true;
    }
    return true;
}

void OSD_Menu_Plus_Sleep::show_menu_plus_sleep(lv_obj_t *_bgd, Plus_Sleep_CB_t _callback)
{
    // Direciona recepção de tecla e função de retorno
    set_focus();
    m_callback = _callback;
    m_main_screen = _bgd;
    m_box_sleep = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1 - 20, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_box_sleep);
    lv_obj_set_style_bg_opa(m_box_sleep, 0, LV_PART_MAIN);
    auto box_message = create_rect(m_box_sleep, 300, 110, 400, 50, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(box_message, 0, LV_PART_MAIN);
    auto message = set_label_text_static(box_message, tr(__Selecione_o_tempo_para_desligar), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(message, LV_ALIGN_TOP_RIGHT, 0, 0);
    m_button = create_rect(m_box_sleep, 750, 100, 260, 50, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_button);
    lv_obj_set_style_radius(m_button, 25, DEFAULT_SELECTOR);
    // Cria botão com valor do timer 
    m_button_label = set_label_text(m_button, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_button_label);
    lv_obj_align(m_button_label, LV_ALIGN_CENTER, 0, 0);
    update_sleep_timer_label();
    // Inicia timer para sair após tempo sem pressionar nenhuma tecla
    start_exit_timer();
}

void OSD_Menu_Plus_Sleep::exit_sleep_screen_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Plus_Sleep *thiz = static_cast<OSD_Menu_Plus_Sleep *>(lv_timer_get_user_data(_tm));
    Task::post_event(std::bind(thiz->m_callback));
    thiz->m_exit_timer = nullptr;
}

void OSD_Menu_Plus_Sleep::start_exit_timer()
{
    // Inicia timer para apagar informações do canal
    if (m_exit_timer == nullptr)
    {
        m_exit_timer = lv_timer_create(exit_sleep_screen_cb, 6000, this);
        lv_timer_set_repeat_count(m_exit_timer, 1);
    }
    else
    {
        lv_timer_reset(m_exit_timer);
    }
}

void OSD_Menu_Plus_Sleep::sleep_timer_next()
{
    // Estado atual do timer
    auto current = get_sleep_timer_pair();
    DEBUG_MSG(OSD, DEBUG, "sleep timer = " << current.first << " - " << current.second << "\n");

    // Varre a lista em ordem crescente
    for (const auto &value : sleep_timer_value_array)
    {
        if (value > s_sleep_timer_value)
        {
            s_sleep_timer_value = value;
            break;
        }
    }
    // Se não ocorreu mudança, volta para o menor
    if ( current.first == s_sleep_timer_value )
    {
        s_sleep_timer_value = sleep_timer_value_array[0];
    }
    // Atualiza label do botão
    update_sleep_timer_label();
}

void OSD_Menu_Plus_Sleep::sleep_timer_previous()
{
    // Estado atual do timer
    auto current = get_sleep_timer_pair();
    DEBUG_MSG(OSD, DEBUG, "sleep timer = " << current.first << " - " << current.second << "\n");

    // Varre a lista em ordem decrescente
    int prev = sleep_timer_value_array[0];
    for (const auto &value : sleep_timer_value_array)
    {
        if (value >= s_sleep_timer_value)
        {
            s_sleep_timer_value = prev;
            break;
        }
        prev = value;
    }
    // Se não ocorreu mudança, volta para o maior
    if ( current.first == s_sleep_timer_value )
    {
        s_sleep_timer_value = sleep_timer_value_array[5];
    }
    // Atualiza label do botão
    update_sleep_timer_label();
}

// Inicializa idioma com parâmetro recebido
void OSD_Menu_Plus_Sleep::set_sleep_timer_value()
{
    // Inicia ou finaliza timer de acordo com valor calculado
    if (s_sleep_timer_value)
    {
        DEBUG_MSG(OSD, DEBUG, "Iniciando timer de " << s_sleep_timer_value << "\n");
        start_sleep_timer();
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Apagando sleep timer\n");
        delete_sleep_timer();
    }
}

std::pair<int, std::string> OSD_Menu_Plus_Sleep::get_sleep_timer_pair()
{
    std::ostringstream oss;
    int hour = s_sleep_timer_value / 60;
    int min = s_sleep_timer_value % 60;
    if (hour > 0) {
        oss << hour << " hora" << (hour > 1 ? "s" : "");
        if (min > 0) 
        {
            oss << " e ";
        }
    }

    if (min > 0) 
    {
        oss << min << " minuto" << (min > 1 ? "s" : "");
    }

    if (hour == 0 && min == 0) {
        oss << "Deslig.";
    }
    return {s_sleep_timer_value, oss.str()};
}

void OSD_Menu_Plus_Sleep::delete_sleep_timer()
{
    DEBUG_MSG(OSD, DEBUG, "Zerando sleep timer\n");
    DELETE_TIMER(s_sleep_timer);
    s_sleep_timer_value = 0;

#ifdef MBGUI_USE_RLOTTIE
    DELETE_OBJ(s_sleep_logo);
#endif
}

void OSD_Menu_Plus_Sleep::start_sleep_timer()
{
    // Inicia timer para entrar em standby
    DELETE_TIMER(s_sleep_timer);
    s_sleep_timer = lv_timer_create(process_sleep_timer_cb, 60 * 1000, this);
    // Tempo para fim do timer
    lv_timer_set_repeat_count(s_sleep_timer, s_sleep_timer_value);
#ifdef MBGUI_USE_RLOTTIE
    DELETE_OBJ(s_sleep_logo);
    if(s_sleep_timer_value > 0)
    {
        s_sleep_logo = lv_rlottie_create_from_file(get_main_screen(OSD_Layer::MAIN_MENU), 50, 50, ANIM_SLEEP);
        lv_obj_null_on_delete(&s_sleep_logo);
        lv_obj_align(s_sleep_logo, LV_ALIGN_TOP_RIGHT, -80, 208);
        //lv_obj_set_style_image_opa(s_sleep_logo, 80, LV_PART_MAIN);
        //lv_obj_set_style_bg_opa(s_sleep_logo, LV_OPA_80, LV_PART_MAIN);
        lv_rlottie_set_play_mode(s_sleep_logo, LV_RLOTTIE_CTRL_LOOP);
    }
#endif
}

void OSD_Menu_Plus_Sleep::process_sleep_timer()
{
    if (--s_sleep_timer_value == 0)
    {
        delete_sleep_timer();
        start_standby();
    }
    auto current = get_sleep_timer_pair();
    DEBUG_MSG(OSD, DEBUG, "sleep timer = " << current.second << "\n");
}

void OSD_Menu_Plus_Sleep::process_sleep_timer_cb(lv_timer_t *tm)
{
    OSD_Menu_Plus_Sleep *thiz = static_cast<OSD_Menu_Plus_Sleep *>(lv_timer_get_user_data(tm));
    thiz->process_sleep_timer();
}

void OSD_Menu_Plus_Sleep::update_sleep_timer_label()
{
    auto current = get_sleep_timer_pair();
    DEBUG_MSG(OSD, DEBUG, "sleep timer = " << current.second << "\n");
    lv_label_set_text(m_button_label, current.second.c_str());
}

void OSD_Menu_Plus_Sleep::start_standby()
{
    Task::post_event_toggle_power();
}

}
