#include "mb_osd_set_clock.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"

#include "hal/mb_system.h"

#include <lvgl.h>

#include <sstream>

namespace {
}

namespace mb {

OSD_Set_Clock::OSD_Set_Clock(OSD *_parent):
    OSD(_parent),
    m_options(offset_x, offset_y, option_w, option_h, option_s, option_x, option_y)
{
    auto system_time = System::get_system_time().to_local_time();
    char buf[100];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", system_time.day(), system_time.month(), system_time.year());
    m_options.add_label(buf);
    snprintf(buf, sizeof(buf), "%02d:%02d", system_time.hour(), system_time.minute());
    m_options.add_label(buf);
    lv_style_init(&m_style);
}

OSD_Set_Clock::~OSD_Set_Clock()
{
    DELETE_TIMER(m_refresh_timer);
    lv_style_reset(&m_style);
    Osd_Breadcrumb::s_instance.remove_name();
    remove_focus();
}

// Processa tecla recebida
bool OSD_Set_Clock::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if ((m_mode_active == Mode_Active::Editing_Date) ||
            (m_mode_active == Mode_Active::Editing_Time))
    {
        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLTAR:
            {
                DELETE_OBJ(m_textarea);
                m_mode_active = Mode_Active::Browsing;
                break;
            }

            case Remote_Control_Key::KEY_VOLUP:
            case Remote_Control_Key::KEY_VOLDOWN:
            {
                if (m_textarea)
                {
                    auto pos = lv_textarea_get_cursor_pos(m_textarea);
                    DEBUG_MSG(OSD, DEBUG, "Text area pos: " <<  pos << "\n");

                    if (_event.key == Remote_Control_Key::KEY_VOLUP)
                    {
                        pos++;

                        if (pos == 2 or pos == 5)
                        {
                            if (m_mode_active == Mode_Active::Editing_Time and pos == 5)
                            {
                                pos = 4;
                            }
                            else
                            {
                                pos++;
                            }
                        }
                    }
                    else
                    {
                        pos--;

                        if (pos == 2 or pos == 5)
                        {
                            pos--;
                        }
                    }

                    //if (pos >= 0)
                    {
                        lv_textarea_set_cursor_pos(m_textarea, pos);
                    }
                }

                break;
            }

            default:
            {
                process_date_time(_event.key);
                break;
            }
        }
    }
    else
    {
        auto selected = m_options.get_selected();

        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLTAR:
                Task::post_event(m_callback);
                break;

            case Remote_Control_Key::KEY_OK:
                m_timeout = 3;

                if (selected == 0) //process_date
                {
                    edit_date(m_options.get_label(selected), selected);
                }
                else //process_clock
                {
                    edit_clock(m_options.get_label(selected), selected);
                }

                break;

            case Remote_Control_Key::KEY_CHUP:
                m_options.previous();
                break;

            case Remote_Control_Key::KEY_CHDOWN:
                m_options.next();
                break;

            default:
                break;
        }
    }

    return true;
}

void OSD_Set_Clock::show_set_clock(Set_Clock_CB_t _callback, lv_obj_t *_parent_screen)
{
    // Direciona recepção de tecla
    set_focus();
    m_parent_screen = _parent_screen;
    m_callback = _callback;
    // Cria área de ajusta do relógio
    lv_obj_null_on_delete(&_parent_screen);
    // Cria botões de confirmação
    m_options.set_background(_parent_screen);
    m_options.set_vertical();
    m_options.draw();
    m_options.select();
    Osd_Breadcrumb::s_instance.add_name(tr(__Manual));
}

void OSD_Set_Clock::refresh_cb(lv_timer_t *tm)
{
    OSD_Set_Clock *thiz = static_cast<OSD_Set_Clock *>(lv_timer_get_user_data(tm));
    thiz->refresh_progress();
}

void OSD_Set_Clock::refresh_progress()
{
    m_timeout--;

    if (m_timeout == 0)
    {
        DELETE_OBJ(m_wrong_date);
    }
}


void OSD_Set_Clock::process_date_time(Remote_Control_Key key)
{
    // Se foi pressionado ok atualizar data e hora
    if (Remote_Control_Key::KEY_OK == key)
    {
        // Busca data e hora no campo de digitação
        std::string str = lv_textarea_get_text(m_textarea);
        std::vector<std::string> timedate;
        split_string(timedate, str, ' ');
        // Cria variável e armazena dia, mês, ano, hora e minuto
        std::vector<std::string> tm;
        auto system_time = System::get_system_time();
        uint16_t year = system_time.year();
        uint8_t month = system_time.month(), day = system_time.day(), hour = system_time.hour(), min = system_time.minute();
        DEBUG_MSG(OSD, DEBUG, timedate[0]);
        char buf[100];

        if (m_mode_active == Mode_Active::Editing_Date)
        {
            split_string(tm, timedate[0], '/');
            day = static_cast<uint8_t>(std::stoi(tm[0]));
            month = static_cast<uint8_t>(std::stoi(tm[1]));
            year = static_cast<uint16_t>(std::stoi(tm[2]));
        }
        else if (m_mode_active == Mode_Active::Editing_Time)
        {
            split_string(tm, timedate[0], ':');
            hour = static_cast<uint8_t>(std::stoi(tm[0]));
            min = static_cast<uint8_t>(std::stoi(tm[1]));

            // Verifica os limites
            if (hour > 23)
            {
                hour = 23;
            }

            if (min > 59)
            {
                min = 59;
            }

            snprintf(buf, sizeof(buf), "%02d:%02d", hour, min);
            m_options.set_label(1, buf);
        }

        if (is_valid_date(day, month, year))
        {
            Task::post_event_clock_set_time(Event_Time
            {
                .year = year,
                .month = month,
                .day = day,
                .hour = hour,
                .minute = min,
                .second = 0
            });
            snprintf(buf, sizeof(buf), "%02d/%02d/%04d", day, month, year);
            m_options.set_label(0, buf);
        }
        else
        {
            DEBUG_MSG(OSD, WARN, "DATA INVALIDA");
            // Cria timer para atualização de tela
            m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
            lv_timer_set_repeat_count(m_refresh_timer, 3);
            m_wrong_date = set_label_text_static(m_parent_screen, tr(__Data_invalida), 0, 0, font_25, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_wrong_date);
            lv_obj_align(m_wrong_date, LV_ALIGN_CENTER, 0, 0);
        }

        DELETE_OBJ(m_textarea);
        m_mode_active = Mode_Active::Browsing;
    }

    // Verifica se foi digitado algum número
    char res;
    char c;

    if ((res = get_digit(key)) != '\n')
    {
        res += '0';
        // Troca o caracter marcado pelo cursor
        lv_textarea_add_char(m_textarea, res);
        lv_textarea_delete_char_forward(m_textarea);
        lv_textarea_cursor_right(m_textarea);
        c = lv_textarea_get_current_char(m_textarea);
        DEBUG_MSG(OSD, DEBUG, "Caracter = " << c << "\n");

        if (isdigit(c))
        {
            lv_textarea_cursor_left(m_textarea);
        }
    }
}

void OSD_Set_Clock::create_textarea(uint16_t idx)
{
    auto btn = m_options.get_button(idx);
    //m_textarea = lv_textarea_create(btn);
    m_textarea = lv_textarea_create(btn);
    lv_obj_null_on_delete(&m_textarea);
    lv_textarea_set_one_line(m_textarea, true);
    lv_obj_set_size(m_textarea, 160, 48);
    lv_obj_align(m_textarea, LV_ALIGN_CENTER, 0, 0);
    lv_style_set_border_color(&m_style, OSD_COLOR_WHITE);
    lv_obj_add_style(m_textarea, &m_style, (int)LV_PART_CURSOR | (int)LV_STATE_FOCUSED);
    lv_obj_add_state(m_textarea, LV_STATE_FOCUSED);
    lv_obj_set_scrollbar_mode(m_textarea, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(m_textarea, 0, DEFAULT_SELECTOR);
    lv_obj_set_style_text_align(m_textarea, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(m_textarea,  font_semi_25, 0);
    lv_obj_set_style_text_color(m_textarea,  OSD_COLOR_WHITE, 0);
    lv_obj_set_style_bg_color(m_textarea, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
}

void OSD_Set_Clock::edit_date(lv_obj_t *lbl, uint16_t idx)
{
    m_mode_active = Mode_Active::Editing_Date;
    std::string str = lv_label_get_text(lbl);
    create_textarea(idx);
    lv_textarea_add_text(m_textarea, str.c_str());
    lv_textarea_set_cursor_pos(m_textarea, 0);
    DEBUG_MSG(OSD, DEBUG, "DATA: " << str << "\n");
}

void OSD_Set_Clock::edit_clock(lv_obj_t *lbl, uint16_t idx)
{
    m_mode_active = Mode_Active::Editing_Time;
    std::string str = lv_label_get_text(lbl);
    create_textarea(idx);
    lv_textarea_add_text(m_textarea, str.c_str());
    lv_textarea_set_cursor_pos(m_textarea, 0);
    DEBUG_MSG(OSD, DEBUG, "HORA: " << str << "\n");
}


} // namespace mb
