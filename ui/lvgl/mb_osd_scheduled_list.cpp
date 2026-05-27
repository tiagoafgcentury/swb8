
#include "mb_osd_scheduled_list.h"
#include "mb_osd_scheduled_edit.h"
#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_message_box.h"

#include "tasks/mb_task_database.h"

#include "common/mb_globals.h"
#include "hal/mb_display.h"
#include "dvb/mb_dvb_utc_mjd.h"
#include "mb_events.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include "mb_osd_keys.h"


namespace mb {

OSD_Scheduled_List::OSD_Scheduled_List(OSD *_parent):
    OSD(_parent)
{
}

OSD_Scheduled_List::~OSD_Scheduled_List()
{
    m_entries.clear();
    m_table_data.clear();
    m_schedule.clear();
    DELETE_OBJ(m_bgd);
    remove_focus();
}

void OSD_Scheduled_List::show_menu_schedule_edit_callback()
{
    m_scheduled_edit.reset();
    load_schedule_list();
}

bool OSD_Scheduled_List::handle_event_remote_control(const Event_Remote_Control &_event)
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
            if (m_schedule.size() > 0)
            {
                ScheduleEntry s_entry = m_schedule[m_selected_item];
                m_scheduled_edit = std::make_unique<OSD_Scheduled_Edit>(this);
                m_scheduled_edit->show_scheduled_edit(std::bind(&OSD_Scheduled_List::show_menu_schedule_edit_callback, this), s_entry);
            }
            else
            {
                if (not m_message_box)
                {
                    m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
                }

                m_message_box->show_message_box_ok(std::bind(&OSD_Scheduled_List::empty_schedule_list_callback, this), tr(__Nenhum_evento_para_editar));
            }

            break;
        }

        case Remote_Control_Key::KEY_5:
        {
            if (m_schedule.size() > 0)
            {
                if (not m_message_box)
                {
                    m_message_box = std::make_unique<OSD_Message_Box>(nullptr);
                }

                m_message_box->show_message_box_yes_no(std::bind(&OSD_Scheduled_List::remove_item_schedule_list_callback, this, std::placeholders::_1), tr(__Deseja_remover_o_evento_da_lista), false);
            }

            break;
        }

        case Remote_Control_Key::KEY_PLUS:
        {
            m_scheduled_edit = std::make_unique<OSD_Scheduled_Edit>(this);
            ScheduleEntry s_entry;
            s_entry.id = 0;
            m_scheduled_edit->show_scheduled_edit(std::bind(&OSD_Scheduled_List::show_menu_schedule_edit_callback, this), s_entry);
            return true;
        }

        case Remote_Control_Key::KEY_CHUP:
        {
            if (m_entries.size() > 0)
            {
                move_menu_up();
            }

            return true;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        {
            if (m_entries.size() > 0)
            {
                move_menu_down();
            }

            return true;
        }

        default:
            return true;
    }

    return true;
}

void OSD_Scheduled_List::empty_schedule_list_callback()
{
    m_message_box.reset();
}

void OSD_Scheduled_List::remove_item_schedule_list_callback(bool _result)
{
    if (_result)
    {
        if (!m_schedule.empty() && m_selected_item < (int)m_schedule.size())
        {
            ScheduleEntry s_entry = m_schedule[m_selected_item];
            Task::post_event_delete_schedule(s_entry.id);
            m_schedule.erase(m_schedule.begin() + m_selected_item);

            // Ajustar para sempre ir para o anterior
            if (m_selected_item > 0)
            {
                m_selected_item--;
            }
        }

        // Garantir que índice não saia do limite após remoção
        if (m_selected_item >= (int)m_schedule.size())
        {
            m_selected_item = 0;//(int)m_schedule.size() - 1;
        }

        load_schedule_list();
    }

    m_message_box.reset();
}

void OSD_Scheduled_List::show_scheduled_list(Scheduled_List_Callback_CB_t _callback)
{
    set_focus();
    m_callback = _callback;

    if (!m_main_menu)
    {
        //MainMenu
        m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);
        lv_obj_set_style_bg_opa(m_bgd, LV_OPA_TRANSP, 0);
        m_main_menu = create_rect(m_bgd, 0, 120, width, heigth, OSD_COLOR_BLACK);
        add_clock(m_main_menu, 1090, 5);
        auto bgd_bottom = create_rect(m_bgd, 0, 630, DISPLAY_WIDTH, 90, OSD_COLOR_BLACK);
        MB_OSD_Footer::draw(bgd_bottom, tr(__Pressione_ok_para_editar_para_incluir_ou_para_excluir), -40);
        create_table();
        load_schedule_list();
    }
}

void OSD_Scheduled_List::load_schedule_list()
{
    DEBUG_MSG(OSD, DEBUG, "Loading final schedule list...\n");
    Task::post_event_schedule_load(std::bind(&OSD_Scheduled_List::load_schedule_list_callback, this, std::placeholders::_1));
}

void OSD_Scheduled_List::load_schedule_list_callback(std::vector<ScheduleEntry> _schedule)
{
    DEBUG_MSG(OSD, DEBUG, "Loading schedule list...\n");

    if (_schedule.size() > 0)
    {
        m_entries.clear();
        m_schedule = std::move(_schedule); //copia todos os dados da estrutura

        for (auto &s : m_schedule)
        {
            Schedule entry;
            entry.id = s.id;

            for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
            {
                if (srv.service_id() == s.service_id)
                {
                    entry.channel = srv.name();
                    goto SERVICE_FOUND;
                }
            }

            // service_id não encontrado
            //mb_assert(false);
            entry.channel = tr(__Servico);
            entry.channel += " ";
            entry.channel += std::to_string(s.service_id);
SERVICE_FOUND:
            auto time_to_start = UTC_MJD(s.time_to_start).to_local_time();
            char buf1[80];
            sprintf(buf1, "%.2d/%.2d", time_to_start.day(), time_to_start.month());
            entry.date = buf1;
            DEBUG_MSG(OSD, DEBUG, "entry.date: " << entry.date << "\n");
            sprintf(buf1, "%.2d:%.2d", time_to_start.hour(), time_to_start.minute());
            entry.time = buf1;
            DEBUG_MSG(OSD, DEBUG, "entry.time: " << entry.time << "\n");

            switch (s.operation)
            {
                case Schedule_Operation::RECORD:
                    entry.operation = tr(__Gravar);
                    break;

                case Schedule_Operation::REMIND:
                    entry.operation = tr(__Lembrar);
                    break;
            }

            switch (s.repeat)
            {
                case Schedule_Repeat::ONCE:
                    entry.repeat = tr(__Uma_vez);
                    break;

                case Schedule_Repeat::DAILY:
                    entry.repeat = tr(__Diariamente);
                    break;

                case Schedule_Repeat::WEEKLY:
                    entry.repeat = tr(__Semanalmente);
                    break;
            }

            switch (s.status)
            {
                case Schedule_Status::ACTIVE:
                    entry.status = tr(__Ativo);
                    break;

                case Schedule_Status::PAUSED:
                    entry.status = tr(__Pausado);
                    break;
            }

            m_entries.push_back(entry);
        }
    }
    else
    {
        m_entries.clear();
    }

    m_start_pos = 0;//m_selected_item;
    move_selection(0, m_selected_item);
    DEBUG_MSG(OSD, DEBUG, "Loaded " << m_entries.size() << " schedules\n");
}

void OSD_Scheduled_List::create_line(lv_obj_t *_bgd, Schedule &_data, bool _visible, bool _save)
{
    std::array<std::string_view, max_num_cols> text
    {
        _data.channel,
        _data.date,
        _data.time,
        _data.operation,
        _data.repeat,
        _data.status
    };
    auto select = create_rect(_bgd, 0, 2, table_w - 2, row_height - 4, OSD_COLOR_ORANGE);
    lv_obj_set_style_radius(select, 20, DEFAULT_SELECTOR);
    lv_obj_set_style_bg_opa(select, LV_OPA_TRANSP, 0);

    for (int i = 0, x = 0; i < max_num_cols; ++i)
    {
        auto bgd_col = create_rect(select, x + 5, 3, col_widths[i] - 10, row_height - 10, OSD_COLOR_BLACK);
        lv_obj_set_style_bg_opa(bgd_col, LV_OPA_TRANSP, 0);
        auto lbl = set_label_text(bgd_col, text[i], 0, 0, font_semi_25, OSD_COLOR_WHITE);

        if (i == 0)
        {
            lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 10, 0);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
            lv_obj_set_width(lbl, col_widths[i] - 10);
            lv_obj_set_height(lbl, 30);
        }
        else
        {
            lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
        }

        x += col_widths[i];

        if (i < (max_num_cols - 1))
        {
            create_rect(_bgd, x, 0, 2, row_height, OSD_COLOR_GREY_DARK);
        }

        create_rect(_bgd, 0, ((i + 1) * (row_height) - 2), table_w, 2, OSD_COLOR_GREY_DARK);

        if (_visible)
        {
            lv_obj_remove_flag(_bgd, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(_bgd, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (_save)
    {
        m_table_data.emplace_back(_bgd, nullptr, nullptr, nullptr);
    }
}

void OSD_Scheduled_List::create_table()
{
    // Cabeçalho
    auto header = create_rect(m_main_menu, offset_x, offset_y, table_w, row_height, OSD_COLOR_GREY_MEDIUM);
    create_line(header, m_headers, true, false);
    m_bgd_table = create_rect(m_main_menu, offset_x, offset_y + row_height, table_w, table_h, OSD_COLOR_BLACK);
    lv_obj_set_scroll_dir(m_bgd_table, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(m_bgd_table, LV_SCROLLBAR_MODE_AUTO);

    for (size_t r = 0; r < MAX_SCHEDULE_LIST_VIEW; ++r)
    {
        Schedule empty_sched = {};
        auto line = create_rect(m_bgd_table, 0, r * row_height, table_w, row_height, OSD_COLOR_BLACK);
        create_line(line, empty_sched, false, true);
    }
}

void OSD_Scheduled_List::reset_selection()
{
    for (const auto &table_data : m_table_data)
    {
        if (table_data.sel)
        {
            lv_obj_set_style_bg_opa(table_data.sel, LV_OPA_TRANSP, 0);
        }
    }
}

void OSD_Scheduled_List::display_channel_list(uint16_t _start_idx, uint16_t _selected_item)
{
    uint16_t size = 0;

    if (m_entries.size() >= _start_idx)
    {
        size = std::min<uint16_t>(MAX_SCHEDULE_LIST_VIEW, m_entries.size() - _start_idx);
    }
    else
    {
        reset_selection();
    }

    for (uint16_t idx = _start_idx; idx < _start_idx + size; idx++)
    {
        auto &table = m_table_data[(idx - _start_idx)];
        std::array<std::string_view, max_num_cols> text
        {
            m_entries[idx].channel,
            m_entries[idx].date,
            m_entries[idx].time,
            m_entries[idx].operation,
            m_entries[idx].repeat,
            m_entries[idx].status
        };
        lv_obj_remove_flag(table.sel, LV_OBJ_FLAG_HIDDEN);
        auto _select = lv_obj_get_child(table.sel, 0);

        for (int i = 0; i < max_num_cols; ++i)
        {
            auto _box = lv_obj_get_child(_select, i);
            auto _lbl = lv_obj_get_child(_box, 0);
            lv_label_set_text(_lbl, text[i].data());
        }

        if (idx == _selected_item)
        {
            lv_obj_set_style_bg_color(_select, OSD_COLOR_ORANGE, 0);
            lv_obj_set_style_bg_opa(_select, LV_OPA_COVER, 0);
        }
        else
        {
            lv_obj_set_style_bg_color(_select, OSD_COLOR_BLACK, 0);
            lv_obj_set_style_bg_opa(_select, LV_OPA_TRANSP, 0);
        }
    }

    if (size < MAX_SCHEDULE_LIST_VIEW)
    {
        for (uint16_t idx = size; idx < MAX_SCHEDULE_LIST_VIEW; idx++)
        {
            const auto &table = m_table_data[idx];
            lv_obj_add_flag(table.sel, LV_OBJ_FLAG_HIDDEN);
            /*
                        auto _select = lv_obj_get_child(table.sel, 0);
                        for (int i = 0;  i < max_num_cols; ++i)
                        {
                            auto _box = lv_obj_get_child(table.sel, i);
                            auto _lbl = lv_obj_get_child(_box, 0);
                            lv_label_set_text(_lbl, "");
                        }
                        lv_obj_set_style_bg_color(_select, OSD_COLOR_BLACK, 0);
                        lv_obj_set_style_bg_opa(_select, LV_OPA_TRANSP, 0);
            */
        }
    }
}

void OSD_Scheduled_List::set_selection(uint16_t _list_to)
{
    if (_list_to < m_start_pos)
    {
        m_start_pos = _list_to;
    }
    else if (_list_to >= (m_start_pos + MAX_SCHEDULE_LIST_VIEW))
    {
        m_start_pos = _list_to - MAX_SCHEDULE_LIST_VIEW + 1;
    }

    display_channel_list(m_start_pos, _list_to);
}

void OSD_Scheduled_List::move_selection(uint16_t /*_from*/, uint16_t _to)
{
    set_selection(_to);
}

void OSD_Scheduled_List::move_menu_up()
{
    auto current_pos = m_selected_item;
    auto count = m_entries.size();
    m_selected_item = (m_selected_item + count - 1) % count;
    move_selection(current_pos, m_selected_item);
}

void OSD_Scheduled_List::move_menu_down()
{
    auto current_pos = m_selected_item;
    auto count = m_entries.size();
    m_selected_item = (m_selected_item + count + 1) % count;
    move_selection(current_pos, m_selected_item);
}

} // namespace mb
