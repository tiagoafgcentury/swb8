#include "mb_osd_terms_of_use.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "mb_terms_of_use.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "tasks/mb_task.h"

namespace mb {

OSD_Terms_of_Use *OSD_Terms_of_Use::s_instance { nullptr };

OSD_Terms_of_Use::OSD_Terms_of_Use(OSD *_parent):
    OSD(_parent),
    m_keys(0, 0, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        tr(__Voltar),
        tr(__Proximo)
    };
    s_instance = this;
}

OSD_Terms_of_Use::~OSD_Terms_of_Use()
{
    DELETE_OBJ(m_main);
    s_instance = nullptr;
}

bool OSD_Terms_of_Use::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Osd_Breadcrumb::s_instance.remove_name();
            call_callback(false);
            return true;

        case Remote_Control_Key::KEY_OK:
        {
            if (m_state == State::Accept)
            {
                m_accepted = true;
                lv_obj_remove_flag(m_circle_sel, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_color(m_accept, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
                m_keys.remove_disabled(1);
                m_keys.select();
                m_keys.next();
                m_state = State::Buttons;
            }
            else if (m_state == State::Buttons)
            {
                Osd_Breadcrumb::s_instance.remove_name();
                auto selected = m_keys.get_selected();

                if (selected == 0)
                {
                    call_callback(false);
                }
                else
                {
                    Terms_File::accept_terms_of_use();
                    call_callback(true);
                }
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLUP:
            if (m_state == State::Page)
            {
                m_cursor_pos = m_cursor_pos + SCROLL_PAGE_Y;

                if (m_cursor_pos >= MAX_SCROLL_Y)
                {
                    m_cursor_pos = MAX_SCROLL_Y;
                    m_page_num = MAX_NUM_PAGE;
                    lv_label_set_text_fmt(m_arrow_label, "%s %d/%d", tr(__Pagina).data(), m_page_num, MAX_NUM_PAGE);
                    m_read_all_pages = true;
                    lv_obj_remove_flag(m_arrow_left_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_arrow_rigth, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_left, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_rigth_sel, LV_OBJ_FLAG_HIDDEN);
                    //Habilita os termos
                    lv_obj_set_style_bg_color(m_circle, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    lv_obj_set_style_text_color(m_accept, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                }
                else
                {
                    lv_obj_remove_flag(m_arrow_left_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_arrow_rigth_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_left, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_rigth, LV_OBJ_FLAG_HIDDEN);
                    m_page_num += 1;
                    lv_label_set_text_fmt(m_arrow_label, "%s %d/%d", tr(__Pagina).data(), m_page_num, MAX_NUM_PAGE);
                }

                lv_obj_scroll_to_y(m_label, m_cursor_pos, LV_ANIM_ON);
                lv_obj_send_event(m_label, LV_EVENT_SCROLL_BEGIN, nullptr);
            }
            else if (m_state == State::Buttons)
            {
                m_keys.next();
            }

            return true;

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            if (m_state == State::Page)
            {
                m_cursor_pos = m_cursor_pos - SCROLL_PAGE_Y;

                if (m_cursor_pos <= 0)
                {
                    m_cursor_pos = 0;
                    m_page_num = 1;
                    lv_label_set_text_fmt(m_arrow_label, "%s %d/%d", tr(__Pagina).data(), m_page_num, MAX_NUM_PAGE);
                    lv_obj_add_flag(m_arrow_left_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_rigth, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_arrow_left, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_arrow_rigth_sel, LV_OBJ_FLAG_HIDDEN);
                }
                else
                {
                    lv_obj_remove_flag(m_arrow_left_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_remove_flag(m_arrow_rigth_sel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_left, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(m_arrow_rigth, LV_OBJ_FLAG_HIDDEN);
                    m_page_num -= 1;
                    lv_label_set_text_fmt(m_arrow_label, "%s %d/%d", tr(__Pagina).data(), m_page_num, MAX_NUM_PAGE);
                }

                lv_obj_scroll_to_y(m_label, m_cursor_pos, LV_ANIM_ON);
                lv_obj_send_event(m_label, LV_EVENT_SCROLL_BEGIN, nullptr);
            }
            else if (m_state == State::Buttons)
            {
                m_keys.next();
            }

            return true;
        }

        case Remote_Control_Key::KEY_CHDOWN:
            if (m_read_all_pages == true)
            {
                m_keys.unselect();
                int tmp = static_cast<int>(m_state) + 1;
                m_state = static_cast<State>(tmp % static_cast<int>(State::COUNT));
                lv_obj_send_event(m_label, LV_EVENT_SCROLL_END, nullptr);

                if (m_state == State::Page)
                {
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_BROWN_LIGHT, DEFAULT_SELECTOR);

                    if (m_accepted == false)
                    {
                        lv_obj_set_style_text_color(m_accept, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    }

                    lv_obj_send_event(m_label, LV_EVENT_SCROLL_BEGIN, nullptr);
                }
                else if (m_state == State::Accept)
                {
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    lv_obj_set_style_text_color(m_accept, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
                }
                else if (m_state == State::Buttons)
                {
                    m_keys.select();
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);

                    if (m_accepted == false)
                    {
                        lv_obj_set_style_text_color(m_accept, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    }
                }
            }

            break;

        case Remote_Control_Key::KEY_CHUP:
        {
            if (m_read_all_pages == true)
            {
                m_keys.unselect();
                int count = static_cast<int>(m_state) - 1;
                auto max = static_cast<int>(State::COUNT);
                m_state = static_cast<State>((max + count) % max);
                lv_obj_send_event(m_label, LV_EVENT_SCROLL_END, nullptr);

                if (m_state == State::Page)
                {
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_BROWN_LIGHT, DEFAULT_SELECTOR);

                    if (m_accepted == false)
                    {
                        lv_obj_set_style_text_color(m_accept, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    }

                    lv_obj_send_event(m_label, LV_EVENT_SCROLL_BEGIN, nullptr);
                }
                else if (m_state == State::Accept)
                {
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    lv_obj_set_style_text_color(m_accept, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
                }
                else if (m_state == State::Buttons)
                {
                    m_keys.select();
                    lv_obj_set_style_bg_color(m_label, OSD_COLOR_WHITE, DEFAULT_SELECTOR);

                    if (m_accepted == false)
                    {
                        lv_obj_set_style_text_color(m_accept, OSD_COLOR_WHITE, DEFAULT_SELECTOR);
                    }
                }
            }
        }

        return true;

        default:
            return true;
    }

    return false;
}

void OSD_Terms_of_Use::show_menu_terms_of_use(Terms_of_Use_CB_t _callback)
{
    set_focus();
    m_callback = _callback;
    //MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    // Título
    auto title = create_rect(m_main, 0, 0, width, title_h, OSD_COLOR_BLACK);
    auto title_label = set_label_text_static(title, tr(__Termos_e_condicoes_de_uso_do_skyb1), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    draw_page();
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.set_horizontal();
    m_keys.draw();
    m_keys.set_disabled(1);
    // Termos de aceite
    m_circle = create_rect(m_main, circle_x, circle_y, circle_w, circle_h, OSD_COLOR_GREY);
    lv_obj_null_on_delete(&m_circle);
    lv_obj_set_style_radius(m_circle, circle_h / 2, DEFAULT_SELECTOR);
    m_circle_sel = create_rect(m_circle, 0, 0, circle_w / 2, circle_h / 2, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_circle_sel);
    lv_obj_set_style_radius(m_circle_sel, circle_h / 4, DEFAULT_SELECTOR);
    lv_obj_align(m_circle_sel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(m_circle_sel, LV_OBJ_FLAG_HIDDEN);
    m_accept = set_label_text(m_main, tr(__Aceito_e_concordo_com_os_termos_e_condicoes_de_uso), accept_x, accept_y, font_20, OSD_COLOR_GREY);
    lv_obj_null_on_delete(&m_accept);
    m_arrow_left = load_image(m_main, LOGO_ARROW_LEFT_20x20, arrow_left_x, arrow_left_y, arrow_left_w, arrow_left_h);
    lv_obj_null_on_delete(&m_arrow_left);
    m_arrow_left_sel = load_image(m_main, LOGO_ARROW_LEFT_SEL_20x20, arrow_left_x, arrow_left_y, arrow_left_w, arrow_left_h);
    lv_obj_null_on_delete(&m_arrow_left_sel);
    lv_obj_add_flag(m_arrow_left_sel, LV_OBJ_FLAG_HIDDEN);
    m_arrow_rigth = load_image(m_main, LOGO_ARROW_RIGTH_20x20, arrow_rigth_x, arrow_rigth_y, arrow_rigth_w, arrow_rigth_h);
    lv_obj_null_on_delete(&m_arrow_rigth);
    lv_obj_add_flag(m_arrow_rigth, LV_OBJ_FLAG_HIDDEN);
    m_arrow_rigth_sel = load_image(m_main, LOGO_ARROW_RIGTH_SEL_20x20, arrow_rigth_x, arrow_rigth_y, arrow_rigth_w, arrow_rigth_h);
    lv_obj_null_on_delete(&m_arrow_rigth_sel);
    char text[128];
    snprintf(text, sizeof(text), "%s 1/%d", tr(__Pagina).data(), MAX_NUM_PAGE);
    m_arrow_label = set_label_text(m_main, text, arrow_label_x, arrow_label_y, font_20, OSD_COLOR_WHITE);
    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Termos_de_uso));
}

void OSD_Terms_of_Use::draw_page()
{
    m_page = create_rect(m_main, 0, 0, page_w, page_h, OSD_COLOR_BROWN_LIGHT);
    lv_obj_align(m_page, LV_ALIGN_TOP_MID, 0, page_y);
    m_label = lv_textarea_create(m_page);
    lv_obj_set_style_bg_color(m_label, OSD_COLOR_BROWN_LIGHT, DEFAULT_SELECTOR);
    lv_obj_set_style_radius(m_label, 0, DEFAULT_SELECTOR);
    lv_obj_set_size(m_label, page_w, page_h);
    lv_obj_align(m_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(m_label, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_text_font(m_label, font_20, 0);
    lv_obj_set_style_text_color(m_label, OSD_COLOR_BLACK, 0);
    lv_obj_set_style_text_letter_space(m_label, 1, 0);
    lv_obj_set_style_text_line_space(m_label, 2, 0);
    set_cas_fingerprint("<CAID>", "<SCUA>");
    using namespace std::placeholders;
    // Este callback TEM que estar posicionado após a criação de m_qrcode
    m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Terms_of_Use::set_cas_fingerprint, this, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
}

void OSD_Terms_of_Use::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    Terms_File::App_Terms_of_Use terms;
    size_t pos = terms.text.find("%s");

    if (pos != std::string::npos)
    {
        terms.text.replace(pos, 2, _caid);
    }

    pos = terms.text.find("%s");

    if (pos != std::string::npos)
    {
        terms.text.replace(pos, 2, _scua);
    }

    lv_textarea_set_text(m_label, terms.text.c_str());
    lv_obj_send_event(m_label, LV_EVENT_SCROLL_BEGIN, nullptr);
    lv_obj_scroll_to_y(m_label, 0, LV_ANIM_ON);
}

void OSD_Terms_of_Use::hide_menu()
{
    remove_focus();
    Osd_Breadcrumb::s_instance.remove_name();
    call_callback(false);
}

void OSD_Terms_of_Use::got_focus()
{
}

} // namespace mb
