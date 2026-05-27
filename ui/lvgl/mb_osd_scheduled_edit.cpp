
#include "mb_osd_scheduled_edit.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"

#include "tasks/mb_task.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task_player.h"
#include "hal/mb_system.h"

#include "common/mb_globals.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_events.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

#include "mb_osd_keys.h"

namespace mb {

OSD_Scheduled_Edit::OSD_Scheduled_Edit(OSD *_parent):
    OSD(_parent),
    m_keys_main(0, 0, button_width, button_heigth, button_spacing, button_x, button_y),
    m_keys_options(cover_w, offset, btn_w, btn_h, btn_spacing, btn_x, btn_y)
{
    m_area.x1 = cover_w;
    m_area.y1 = offset;
    m_area.x2 = width - cover_w - 130;
    m_area.y2 = cover_h;
    lv_style_init(&m_style);
}

OSD_Scheduled_Edit::~OSD_Scheduled_Edit()
{
    lv_style_reset(&m_style);
    m_list_data.clear();
    m_channel_data.clear();
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_bgd);
    remove_focus();
}


bool OSD_Scheduled_Edit::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if (m_enable_sched_screen)
    {
        if ((m_mode_active == Mode_Active::Editing_Date) ||
                (m_mode_active == Mode_Active::Editing_Time))
        {
            if (m_edit_btn_time == false and m_mode_active == Mode_Active::Editing_Time)
            {
                switch (_event.key)
                {
                    case Remote_Control_Key::KEY_VOLUP:
                    case Remote_Control_Key::KEY_VOLDOWN:
                    {
                        m_keys_options.next();
                        break;
                    }

                    case Remote_Control_Key::KEY_OK:
                    {
                        m_edit_btn_time = true;
                        auto idx = m_keys_options.get_selected();
                        create_textarea(idx);
                        auto _str = lv_label_get_text(m_keys_options.get_label(idx));
                        lv_textarea_add_text(m_textarea, _str);
                        lv_textarea_set_cursor_pos(m_textarea, 0);
                        //lv_label_set_text(m_footer, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar).data());
                        break;
                    }

                    case Remote_Control_Key::KEY_VOLTAR:
                    {
                        auto idx = m_keys_main.get_selected();
                        if(m_selected_button == Func_Active::Date)
                        {
                            m_keys_main.set_label(idx, m_final_date);
                        }
                        else if (m_selected_button == Func_Active::Time)
                        {
                            m_final_start_time = lv_label_get_text(m_keys_options.get_label(0));
                            m_final_end_time = lv_label_get_text(m_keys_options.get_label(1));
                            auto _str = m_final_start_time + " - " + m_final_end_time;
                            m_keys_main.set_label(idx, _str);
                        }

                        m_edit_btn_time = false;
                        reset_sat_options();
                        DELETE_OBJ(m_bgd_option);
                        //lv_label_set_text(m_footer, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar).data());
                        break;
                    }

                    default:
                        break;
                }
            }
            else
            {
                switch (_event.key)
                {
/*
                    case Remote_Control_Key::KEY_VOLTAR:
                    {
                        DELETE_OBJ(m_textarea);
                        m_mode_active = Mode_Active::Browsing;
                        break;
                    }
*/
                    case Remote_Control_Key::KEY_VOLUP:
                    case Remote_Control_Key::KEY_VOLDOWN:
                    {
                        if (m_textarea)
                        {
                            auto pos = lv_textarea_get_cursor_pos(m_textarea);

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
        }
        else
        {
            switch (_event.key)
            {
                case Remote_Control_Key::KEY_VOLTAR:
                {
                    reset_sat_options();
                    DELETE_OBJ(m_bgd_option);
                    break;
                }

                case Remote_Control_Key::KEY_OK:
                {
                    auto idx = m_keys_main.get_selected();

                    switch (m_selected_button)
                    {
                        case Func_Active::Channel:
                        {
                            m_entry.service_id = m_channel_data[m_selected_item].service_id;
                            m_keys_main.set_label(idx, m_channel_data[m_selected_item].channel_name);
                            m_entry_text.channel = m_channel_data[m_selected_item].channel_name;
                            break;
                        }

                        case Func_Active::Date:
                        {
                            m_keys_main.set_label(idx, m_final_date);
                            break;
                        }

                        case Func_Active::Time:
                        {
                            m_final_start_time = lv_label_get_text(m_keys_options.get_label(0));
                            m_final_end_time = lv_label_get_text(m_keys_options.get_label(1));
                            auto _str = m_final_start_time + " - " + m_final_end_time;
                            m_keys_main.set_label(idx, _str);
                            break;
                        }

                        case Func_Active::Operation:
                        {
                            auto selected = m_keys_options.get_selected();
                            m_entry.operation = selected == 0 ? Schedule_Operation::RECORD : Schedule_Operation::REMIND;
                            m_keys_main.set_label(idx, m_entry.operation == Schedule_Operation::RECORD ? tr(__Gravar) : tr(__Lembrar));
                            m_entry_text.operation = m_entry.operation == Schedule_Operation::RECORD ? tr(__Gravar) : tr(__Lembrar);
                            break;
                        }

                        case Func_Active::Repeat:
                        {
                            auto selected = m_keys_options.get_selected();
                            m_entry.repeat = (selected == 0) ? Schedule_Repeat::ONCE :
                                             (selected == 1) ? Schedule_Repeat::DAILY :
                                             Schedule_Repeat::WEEKLY;
                            m_keys_main.set_label(idx, m_entry.repeat == Schedule_Repeat::ONCE ? tr(__Uma_vez) :
                                                m_entry.repeat == Schedule_Repeat::DAILY ? tr(__Diariamente) :
                                                tr(__Semanalmente));
                            m_entry_text.operation = m_entry.repeat == Schedule_Repeat::ONCE ? tr(__Uma_vez) :
                                                  m_entry.repeat == Schedule_Repeat::DAILY ? tr(__Diariamente) :
                                                  tr(__Semanalmente);
                            break;
                        }

                        case Func_Active::Status:
                        {
                            auto selected = m_keys_options.get_selected();
                            m_entry.status = selected == 1 ? Schedule_Status::ACTIVE : Schedule_Status::PAUSED;
                            m_keys_main.set_label(idx, m_entry.status == Schedule_Status::ACTIVE ? tr(__Ativo) : tr(__Pausado));
                            m_entry_text.status = m_entry.status == Schedule_Status::ACTIVE ? tr(__Ativo) : tr(__Pausado);
                            break;
                        }

                        default:
                            break;
                    }

                    reset_sat_options();
                    DELETE_OBJ(m_bgd_option);
                    break;
                }

                case Remote_Control_Key::KEY_CHUP:
                {
                    if (m_selected_button == Func_Active::Channel)
                    {
                        move_menu_up();
                    }

                    break;
                }

                case Remote_Control_Key::KEY_CHDOWN:
                {
                    if (m_selected_button == Func_Active::Channel)
                    {
                        move_menu_down();
                    }

                    break;
                }

                case Remote_Control_Key::KEY_VOLUP:
                {
                    if (m_selected_button == Func_Active::Operation or
                            m_selected_button == Func_Active::Repeat or
                            m_selected_button == Func_Active::Status)
                    {
                        m_keys_options.next();
                    }

                    break;
                }

                case Remote_Control_Key::KEY_VOLDOWN:
                {
                    if (m_selected_button == Func_Active::Operation or
                            m_selected_button == Func_Active::Repeat or
                            m_selected_button == Func_Active::Status)
                    {
                        m_keys_options.previous();
                    }

                    break;
                }

                default:
                    break;
            }
        }
    }
    else
    {
        switch (_event.key)
        {
            case Remote_Control_Key::KEY_VOLTAR:
            {
                Task::post_event(std::bind(m_callback, true));
                return true;
            }

            case Remote_Control_Key::KEY_OK:
            {
                if (m_selected_button == Func_Active::Save)
                {
                    //Converte a data para inteiro
                    std::vector<std::string> _date;
                    split_string(_date, m_final_date, '/');
                    auto day = static_cast<uint8_t>(std::stoi(_date[0]));
                    auto month = static_cast<uint8_t>(std::stoi(_date[1]));
                    auto year = static_cast<uint16_t>(std::stoi(_date[2]));
                    std::vector<std::string> _time;
                    split_string(_time, m_final_start_time, ':');
                    auto s_hour = static_cast<uint8_t>(std::stoi(_time[0]));
                    auto s_min = static_cast<uint8_t>(std::stoi(_time[1]));
                    _time.clear();
                    split_string(_time, m_final_end_time, ':');
                    auto e_hour = static_cast<uint8_t>(std::stoi(_time[0]));
                    auto e_min = static_cast<uint8_t>(std::stoi(_time[1]));
                    m_entry.time_to_start = UTC_MJD(year, month, day, s_hour, s_min, 0).to_system_time().to_time_point<std::chrono::system_clock>();
                    m_entry.time_to_end = UTC_MJD(year, month, day, e_hour, e_min, 0).to_system_time().to_time_point<std::chrono::system_clock>();

                    //salvar no banco de dados
                    if (m_entry.id == 0)
                    {
                        Task::post_event_insert_schedule(m_entry);
                    }
                    else
                    {
                        Task::post_event_update_schedule(m_entry);
                    }

                    Task::post_event(std::bind(m_callback, false));
                }
                else
                {
                    edit_schedule_functions();
                }

                return true;
            }

            case Remote_Control_Key::KEY_CHUP:
            {
                //Habilita o botão salvar
                if (m_keys_main.is_first_enabled() && m_selected_button != Func_Active::Save)
                {
                    m_selected_button = Func_Active::Save;
                    select();
                    m_keys_main.unselect();
                }
                else
                {
                    if (m_selected_button == Func_Active::Save)
                    {
                        m_keys_main.go_to_last_enabled();
                    }
                    else
                    {
                        m_keys_main.previous();
                    }

                    unselect();
                    m_selected_button = static_cast<Func_Active>(m_keys_main.get_selected());
                }

                //move_menu_up();
                return true;
            }

            case Remote_Control_Key::KEY_CHDOWN:
            {
                //Habilita o botão salvar
                if (m_keys_main.is_last_enabled() && m_selected_button != Func_Active::Save)
                {
                    m_selected_button = Func_Active::Save;
                    select();
                    m_keys_main.unselect();
                }
                else
                {
                    if (m_selected_button == Func_Active::Save)
                    {
                        m_keys_main.go_to_first_enabled();
                    }
                    else
                    {
                        m_keys_main.next();
                    }

                    unselect();
                    m_selected_button = static_cast<Func_Active>(m_keys_main.get_selected());
                }

                //move_menu_down();
                return true;
            }

            default:
                return true;
        }
    }

    return true;
}

void OSD_Scheduled_Edit::select()
{
    lv_obj_set_style_bg_color(m_btn_save, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_set_style_text_font(m_btn_save_label, font_semi_25, 0);
}

void OSD_Scheduled_Edit::unselect()
{
    lv_obj_set_style_bg_color(m_btn_save, OSD_COLOR_GREY_MEDIUM, DEFAULT_SELECTOR);
    lv_obj_set_style_text_font(m_btn_save_label, font_25, 0);
}

void OSD_Scheduled_Edit::reset_sat_options()
{
    // Esconde tecla salvar
    lv_obj_remove_flag(m_btn_save, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_add_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
    m_enable_sched_screen = false;
    m_keys_options.clear();
    DELETE_OBJ(m_bgd_option);
}


void OSD_Scheduled_Edit::draw_schedule_options(bool create_buttons)
{
    // Esconde tecla salvar
    lv_obj_add_flag(m_btn_save, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
    m_bgd_option = create_rect(m_main_menu, cover_w, 0, 715, cover_h, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(m_bgd_option, LV_OPA_TRANSP, 0);
    lv_obj_null_on_delete(&m_bgd_option);
    create_rect(m_bgd_option, 0, 0, 3, cover_h, OSD_COLOR_ORANGE);

    if (create_buttons)
    {
        m_keys_options.clear();
        m_keys_options.set_background(m_bgd_option);
        m_keys_options.set_horizontal();
    }

    m_enable_sched_screen = true;
}

void OSD_Scheduled_Edit::set_current_channel_list_view()
{
    auto size = m_channel_data.size();
    auto _srv = Task::s_task_player->current_srv();
    m_selected_item = 0;

    if (_srv)
    {
        for (size_t idx = 0; idx < size; idx++)
        {
            const auto &ch_data = m_channel_data[idx];

            if (_srv && (_srv->service_id() == ch_data.service_id))
            {
                m_selected_item = idx;
                break;
            }
        }
    }
}

void OSD_Scheduled_Edit::update_channel_table_list()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto _current_channel_list = current_lineup->get_channel_list_type();
    m_channel_data = current_lineup->get_list(static_cast<Channel_List_Type>(_current_channel_list));

    if (m_selected_item >= m_channel_data.size())
    {
        m_selected_item = m_channel_data.size() - 1;
    }
}

void OSD_Scheduled_Edit::add_channel_list()
{
    m_list_data.clear();
    m_list_data.reserve(MAX_CHANNEL_LIST_VIEW);

    for (size_t idx = 0; idx < MAX_CHANNEL_LIST_VIEW; idx++)
    {
        auto y = 5 + (LIST_SPACING * idx);
        auto sel = create_rect(m_bgd_option, 15, y, 445, 50, OSD_COLOR_ORANGE);
        lv_obj_set_style_radius(sel, 25, DEFAULT_SELECTOR);
        lv_obj_set_style_bg_opa(sel, LV_OPA_TRANSP, 0);

        auto lbl = set_label_text(sel, "", 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 20, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
        lv_obj_set_width(lbl, 405);
        lv_obj_set_height(lbl, 35);

        m_list_data.emplace_back(sel, nullptr, nullptr, lbl);
    }
}


void OSD_Scheduled_Edit::refresh_cb(lv_timer_t *_tm)
{
    OSD_Scheduled_Edit *thiz = static_cast<OSD_Scheduled_Edit *>(lv_timer_get_user_data(_tm));
    thiz->refresh_progress();
}

void OSD_Scheduled_Edit::refresh_progress()
{
    DELETE_OBJ(m_wrong_date);
    DELETE_TIMER(m_refresh_timer);
}

void OSD_Scheduled_Edit::process_date_time(Remote_Control_Key _key)
{
    // Se foi pressionado ok atualizar data e hora
    if (Remote_Control_Key::KEY_OK == _key)
    {
        // Busca data e hora no campo de digitação
        std::string str = lv_textarea_get_text(m_textarea);
        DEBUG_MSG(OSD, DEBUG, str);
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
                auto idx = m_keys_options.get_selected();
                m_keys_options.set_label(idx, buf);
                m_final_date = timedate[0];
                m_mode_active = Mode_Active::Browsing;
            }
            else
            {
                DEBUG_MSG(OSD, DEBUG, "DATA INVALIDA");

                // Cria timer para atualização de tela
                if (m_refresh_timer)
                {
                    lv_timer_reset(m_refresh_timer);
                }
                else
                {
                    m_refresh_timer = lv_timer_create(refresh_cb, 3000, this);
                    m_wrong_date = create_rect(m_bgd_option, 30, 150, 160, 40, OSD_COLOR_BLACK);
                    lv_obj_null_on_delete(&m_wrong_date);
                    auto lbl = set_label_text(m_wrong_date, tr(__Data_invalida), 0, 0, font_semi_25, OSD_COLOR_WHITE);
                    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
                }

                timedate[0] = m_final_date;
                return;
            }
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
            auto idx = m_keys_options.get_selected();
            m_keys_options.set_label(idx, buf);
            m_edit_btn_time = false;
/*
            if (idx == 0)
            {
                m_mode_active = Mode_Active::Editing_Time;
            }
            else
            {
                m_mode_active = Mode_Active::Browsing;
            }
*/
        }

        DELETE_OBJ(m_textarea);
    }

    // Verifica se foi digitado algum número
    char res;
    char c;

    if ((res = get_digit(_key)) != '\n')
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

void OSD_Scheduled_Edit::create_textarea(uint16_t _idx)
{
    auto btn = m_keys_options.get_button(_idx);
    m_textarea = lv_textarea_create(btn);
    lv_obj_null_on_delete(&m_textarea);
    lv_textarea_set_one_line(m_textarea, true);
    lv_obj_set_size(m_textarea, 160, 55);
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

void OSD_Scheduled_Edit::edit_schedule_functions()
{

    switch (m_selected_button)
    {
        case Func_Active::Channel:
        {
            m_mode_active = Mode_Active::Browsing;
            set_current_channel_list_view();
            update_channel_table_list();
            draw_schedule_options(false);
            add_channel_list();
            set_selection(m_selected_item);
            break;
        }

        case Func_Active::Date:
        {
            m_mode_active = Mode_Active::Editing_Date;
            draw_schedule_options(true);
            m_keys_options.set_y(204); //(offset + 5 + i * title_spacing)
            m_keys_options.set_height(60);
            auto idx = m_keys_main.get_selected();
            auto str = lv_label_get_text(m_keys_main.get_label(idx));
            m_keys_options.add_label(str);
            m_keys_options.draw();
            m_keys_options.select();
            create_textarea(0);
            lv_textarea_add_text(m_textarea, str);
            lv_textarea_set_cursor_pos(m_textarea, 0);
            // Atualiza o label do footer
            //lv_label_set_text(m_footer, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar_e_voltar_para_salvar).data());
            break;
        }

        case Func_Active::Time:
        {
            m_mode_active = Mode_Active::Editing_Time;
            draw_schedule_options(true);
            m_keys_options.set_y(279); //(offset + 5 + i * title_spacing)
            m_keys_options.set_height(60);
            auto idx = m_keys_main.get_selected();
            std::string str = lv_label_get_text(m_keys_main.get_label(idx));
            auto hora_inicio = str.substr(0, 5);
            auto hora_fim = str.substr(8, 5);
            auto _rec1 = create_rect(m_bgd_option, 40, 110, 140, 40, OSD_COLOR_BLACK);
            auto _lbl1 = set_label_text(_rec1, tr(__Inicio), 0, 0, font_semi_25, OSD_COLOR_WHITE);
            lv_obj_align(_lbl1, LV_ALIGN_CENTER, 0, 0);
            m_keys_options.add_label(hora_inicio);
            auto rec2 = create_rect(m_bgd_option, 245, 110, 140, 40, OSD_COLOR_BLACK);
            auto lbl2 = set_label_text(rec2, tr(__Termino), 0, 0, font_semi_25, OSD_COLOR_WHITE);
            lv_obj_align(lbl2, LV_ALIGN_CENTER, 0, 0);
            m_keys_options.add_label(hora_fim);
            m_keys_options.draw();
            m_keys_options.select();
            // Atualiza o label do footer
            //lv_label_set_text(m_footer, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar_e_voltar_para_salvar).data());
            break;
        }

        case Func_Active::Operation:
        {
            m_mode_active = Mode_Active::Browsing;
            draw_schedule_options(true);
            m_keys_options.set_y(354); //(offset + 5 + i * title_spacing)
            m_keys_options.add_label(tr(__Gravar));
            m_keys_options.add_label(tr(__Lembrar));
            m_keys_options.draw();

            if (m_entry.operation == Schedule_Operation::REMIND)
            {
                m_keys_options.next();
            }

            m_keys_options.select();
            break;
        }

        case Func_Active::Repeat:
        {
            m_mode_active = Mode_Active::Browsing;
            draw_schedule_options(true);
            m_keys_options.set_y(429); //(offset + 5 + i * title_spacing)
            m_keys_options.add_label(tr(__Uma_vez));
            m_keys_options.add_label(tr(__Diariamente));
            m_keys_options.add_label(tr(__Semanalmente));
            m_keys_options.draw();

            if (m_entry.repeat == Schedule_Repeat::WEEKLY)
            {
                m_keys_options.previous();
            }
            else if (m_entry.repeat == Schedule_Repeat::DAILY)
            {
                m_keys_options.next();
            }

            m_keys_options.select();
            break;
        }

        case Func_Active::Status:
        {
            m_mode_active = Mode_Active::Browsing;
            draw_schedule_options(true);
            m_keys_options.set_y(504);//(offset + 5 + i * title_spacing)
            m_keys_options.add_label(tr(__Ativo));
            m_keys_options.add_label(tr(__Pausado));
            m_keys_options.draw();

            if (m_entry.status == Schedule_Status::PAUSED)
            {
                m_keys_options.next();
            }

            m_keys_options.select();
            break;
        }

        default:
            break;
    }
}

void OSD_Scheduled_Edit::show_scheduled_edit(Scheduled_Edit_CB_t _callback, ScheduleEntry _s_entry)
{
    set_focus();
    m_callback = _callback;
    m_entry = _s_entry;

    if (!m_main_menu)
    {
        //MainMenu
        m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);
        lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
        m_main_menu = create_rect(m_bgd, 0, offset_y, width, heigth, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_main_menu);
        update_channel_table_list();
        set_current_channel_list_view();
        fill_table();
        draw_titles();
        draw_buttons();
        // Desenha botão salvar
        m_btn_save = create_rect(m_main_menu, btn_save_x, btn_save_y, btn_save_width, btn_save_heigth, OSD_COLOR_GREY_MEDIUM);
        lv_obj_null_on_delete(&m_btn_save);
        lv_obj_set_style_radius(m_btn_save, btn_save_heigth / 2, DEFAULT_SELECTOR);
        m_btn_save_label = set_label_text(m_btn_save, tr(__Salvar), 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&m_btn_save_label);
        lv_obj_align(m_btn_save_label, LV_ALIGN_CENTER, 0, 0);
        add_clock(m_main_menu, 1090, 5);
        m_bgd_bottom = create_rect(m_bgd, 0, 630, DISPLAY_WIDTH, 90, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_bottom);
        m_footer = MB_OSD_Footer::draw(m_bgd_bottom, tr(__Selecione_a_opcao_desejada_e_pressione_ok_para_continuar), -40);
        lv_obj_null_on_delete(&m_footer);
        // Desenha uma área translúcida em cima dos menus atuais
        m_cover_area = create_rect(m_main_menu, cover_x, cover_y, cover_w, cover_h, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_cover_area);
        lv_obj_set_style_bg_opa(m_cover_area, LV_OPA_50, 0);
        lv_obj_add_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Scheduled_Edit::fill_table()
{
    if (m_entry.service_id > 0)
    {
        for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
        {
            if (srv.service_id() == m_entry.service_id)
            {
                m_entry_text.channel = srv.name();
                auto time_to_start = UTC_MJD(m_entry.time_to_start).to_local_time();
                auto time_to_end = UTC_MJD(m_entry.time_to_end).to_local_time();
                char buf1[80], buf2[80];
                sprintf(buf1, "%.2d/%.2d/%.4d", time_to_start.day(), time_to_start.month(), time_to_start.year());
                m_entry_text.date = buf1;
                m_final_date = buf1;
                sprintf(buf1, "%.2d:%.2d", time_to_start.hour(), time_to_start.minute());
                sprintf(buf2, "%.2d:%.2d", time_to_end.hour(), time_to_end.minute());
                m_entry_text.time = std::string(buf1) + " - " + std::string(buf2);
                m_final_start_time = buf1;
                m_final_end_time = buf2;
                break;
            }
        }
    }
    else
    {
        if(m_selected_item < m_channel_data.size())
        {
            m_entry_text.channel = m_channel_data[m_selected_item].channel_name; //pega o primeiro canal da lista
            m_entry.service_id = m_channel_data[m_selected_item].service_id;
        }
        else
        {
            m_entry_text.channel = m_channel_data[0].channel_name; //pega o primeiro canal da lista
            m_entry.service_id = m_channel_data[0].service_id;
        }
        auto system_time = System::get_system_time().to_local_time();
        const uint16_t year = system_time.year();
        const uint8_t month = system_time.month(), day = system_time.day(), hour = system_time.hour(), min = 0;
        char buf1[80], buf2[80];
        sprintf(buf1, "%.2d/%.2d/%.4d", day, month, year);
        m_entry_text.date = buf1;
        m_final_date = buf1;
        sprintf(buf1, "%.2d:%.2d", (hour + 1) % 24, min);
        sprintf(buf2, "%.2d:%.2d", (hour + 2) % 24, min);
        m_entry_text.time = std::string(buf1) + " - " + std::string(buf2);
        m_final_start_time = buf1;
        m_final_end_time = buf2;
        m_entry.repeat = Schedule_Repeat::ONCE;
        m_entry.operation = Schedule_Operation::RECORD;
        m_entry.status = Schedule_Status::ACTIVE;
    }

    switch (m_entry.operation)
    {
        case Schedule_Operation::RECORD:
            m_entry_text.operation = tr(__Gravar);
            break;

        case Schedule_Operation::REMIND:
        default:
            m_entry_text.operation = tr(__Lembrar);
            break;
    }

    switch (m_entry.repeat)
    {
        case Schedule_Repeat::DAILY:
            m_entry_text.repeat = tr(__Diariamente);
            break;

        case Schedule_Repeat::WEEKLY:
            m_entry_text.repeat = tr(__Semanalmente);
            break;

        case Schedule_Repeat::ONCE:
        default:
            m_entry_text.repeat = tr(__Uma_vez);
            break;
    }

    switch (m_entry.status)
    {
        case Schedule_Status::PAUSED:
            m_entry_text.status = tr(__Pausado);
            break;

        case Schedule_Status::ACTIVE:
        default:
            m_entry_text.status = tr(__Ativo);
            break;
    }
}

void OSD_Scheduled_Edit::draw_buttons()
{
    m_keys_main.add_label(m_entry_text.channel);
    m_keys_main.add_label(m_entry_text.date);
    m_keys_main.add_label(m_entry_text.time);
    m_keys_main.add_label(m_entry_text.operation);
    m_keys_main.add_label(m_entry_text.repeat);
    m_keys_main.add_label(m_entry_text.status);
    m_keys_main.set_background(m_main_menu);
    m_keys_main.set_vertical();
    m_keys_main.set_back_color(OSD_COLOR_BLACK);
    m_keys_main.draw();
    m_keys_main.select();
    // Desenha linha
    m_line = create_rect(m_main_menu, line_x, line_y, line_width, line_heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line);
}

void OSD_Scheduled_Edit::draw_titles()
{
    // Cria botões
    for (size_t i = 0; i < m_title_names.size() - 1; i++)
    {
        m_titles[i] = create_rect(m_main_menu, title_x, title_y + (i * title_spacing), title_width, title_heigth, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(m_titles[i], title_heigth / 2, DEFAULT_SELECTOR);
        m_labels[i] = set_label_text(m_titles[i], m_title_names[i], 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(m_labels[i], LV_ALIGN_LEFT_MID, 20, 0);
    }
}

void OSD_Scheduled_Edit::display_channel_list(uint16_t _start_idx, uint16_t _selected_item)
{
    uint16_t size = 0;

    if (_start_idx < m_channel_data.size())
    {
        size = std::min<uint16_t>(MAX_CHANNEL_LIST_VIEW, m_channel_data.size() - _start_idx);
    }
    else
    {
        for (auto &_list : m_list_data)
        {
            if (_list.sel)
            {
                lv_obj_set_style_bg_opa(_list.sel, LV_OPA_TRANSP, 0);
            }
        }
    }

    for (uint16_t idx = _start_idx; idx < _start_idx + size; idx++)
    {
        auto &list = m_list_data[(idx - _start_idx)];
        if (!list.sel || !lv_obj_is_valid(list.sel) ||
            !list.lbl || !lv_obj_is_valid(list.lbl))
        {
            continue;
        }

        auto str = std::to_string(m_channel_data[idx].viewer_channel) + " - " + m_channel_data[idx].channel_name;
        lv_label_set_text(list.lbl, str.data());

        if (idx == _selected_item)
        {
            lv_obj_set_style_bg_color(list.sel, OSD_COLOR_ORANGE, 0);
            lv_obj_set_style_bg_opa(list.sel, LV_OPA_COVER, 0);
        }
        else
        {
            lv_obj_set_style_bg_color(list.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(list.sel, LV_OPA_TRANSP, 0);
        }
    }

    if (size < MAX_CHANNEL_LIST_VIEW)
    {
        for (uint16_t idx = size; idx < MAX_CHANNEL_LIST_VIEW; idx++)
        {
            auto &list = m_list_data[idx];
            lv_label_set_text(list.lbl, "");
            lv_obj_set_style_bg_color(list.sel, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(list.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Scheduled_Edit::set_selection(uint16_t _to)
{
    if (_to < m_start_pos)
    {
        m_start_pos = _to;
    }
    else if (_to >= (m_start_pos + MAX_CHANNEL_LIST_VIEW))
    {
        m_start_pos = _to - MAX_CHANNEL_LIST_VIEW + 1;
    }

    display_channel_list(m_start_pos, _to);
}

void OSD_Scheduled_Edit::move_menu_up()
{
    auto count = m_channel_data.size();
    m_selected_item = (m_selected_item + count - 1) % count;
    set_selection(m_selected_item);
}

void OSD_Scheduled_Edit::move_menu_down()
{
    auto count = m_channel_data.size();
    m_selected_item = (m_selected_item + count + 1) % count;
    set_selection(m_selected_item);
}

} // namespace mb
