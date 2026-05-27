#include "mb_osd_change_password.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd_fonts.h"
#include "common/mb_globals.h"
#include "common/mb_state_file.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_osd.h" // Add this include for Task_OSD

namespace mb {

OSD_Change_Password::OSD_Change_Password(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_passwd_style);
    lv_style_set_bg_color(&m_passwd_style, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_border_color(&m_passwd_style, OSD_COLOR_GREY);
    lv_style_set_border_width(&m_passwd_style, 3);
    Osd_Breadcrumb::s_instance.add_name(tr(__Senha));
}

OSD_Change_Password::~OSD_Change_Password()
{
    lv_style_reset(&m_passwd_style);
    DELETE_OBJ(m_main_screen);
    remove_focus();
}

void OSD_Change_Password::change_parental_password(Password_Callback_CB_t _callback, lv_obj_t *_bgd)
{
    m_callback = _callback;
    m_main_screen = _bgd;
    // Inicializa variáveis de senha
    m_pass_buffer = std::string();
    // Direciona recepção de tecla
    set_focus();
    // Desenha caixas de senha
    set_label_text_static(m_main_screen, tr(__Senha), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    set_label_text_static(m_main_screen, tr(__Confirme_sua_senha), 0, box_y_spacing, font_semi_25, OSD_COLOR_WHITE);

    for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
    {
        // Cria retângulo para cada caracter da senha
        auto x = box_x + (i % 4) * box_x_spacing;
        auto y = box_y + (i / 4) * box_y_spacing;
        DEBUG_MSG(OSD, DEBUG, "Creating password box at (" << x << ", " << y << ")\n");
        m_passwd_info[i].rect = create_rect(m_main_screen, x, y, box_width, box_height, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&(m_passwd_info[i].rect));
        lv_obj_set_style_border_color(m_passwd_info[i].rect, i == 0 ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
        lv_obj_set_style_border_width(m_passwd_info[i].rect, 3, LV_PART_MAIN);
        m_passwd_info[i].circle = create_rect(m_passwd_info[i].rect, 0, 0, 24, 24, OSD_COLOR_GREY_DARK);
        lv_obj_null_on_delete(&m_passwd_info[i].circle);
        lv_obj_set_style_radius(m_passwd_info[i].circle, 12, DEFAULT_SELECTOR);
        lv_obj_align(m_passwd_info[i].circle, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Change_Password::clear_password_fields()
{
    for (auto &info : m_passwd_info)
    {
        lv_obj_add_flag(info.circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(info.rect, OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
    }

    lv_obj_set_style_border_color(m_passwd_info[0].rect, OSD_COLOR_ORANGE, LV_PART_MAIN);
    m_pass_buffer = "";
}

// Processa tecla recebida
bool OSD_Change_Password::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_0:
        case Remote_Control_Key::KEY_1:
        case Remote_Control_Key::KEY_2:
        case Remote_Control_Key::KEY_3:
        case Remote_Control_Key::KEY_4:
        case Remote_Control_Key::KEY_5:
        case Remote_Control_Key::KEY_6:
        case Remote_Control_Key::KEY_7:
        case Remote_Control_Key::KEY_8:
        case Remote_Control_Key::KEY_9:
        {
            process_number_key(_event);
            return true;
        }

        case Remote_Control_Key::KEY_VOLTAR:
            Osd_Breadcrumb::s_instance.remove_name();
            Task::post_event(std::bind(m_callback, false));
            return true;

        default:
            DEBUG_MSG(OSD, WARN, "Key 0x" << HEXBYTE(_event.key) << " not handled in password context\n");
            return true;
    }
}

void OSD_Change_Password::process_number_key(Event_Remote_Control _event)
{
    auto key = to_char(_event.key);
    if (key < '0' || key > '9')
    {
        DEBUG_MSG(OSD, DEBUG, "Key 0x" << HEXBYTE(_event.key) << " not handled in password context\n");
        return;
    }

    // Adiciona caracter na string
    m_pass_buffer += static_cast<char>(key);
    // Preenche o campo com asterísticos e exibe na tela
    {
        size_t pointer = m_pass_buffer.size() - 1;
        lv_obj_remove_flag(m_passwd_info[pointer].circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(m_passwd_info[pointer].rect, OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);

        if ((pointer + 1) < MAX_CHAR_PASSWD)
        {
            lv_obj_set_style_border_color(m_passwd_info[pointer + 1].rect, OSD_COLOR_ORANGE, LV_PART_MAIN);
        }
    }

    // Ao pressionar a 4a. tecla, inicia timer para verificar senha
    if (m_pass_buffer.size() == MAX_CHAR_PASSWD)
    {
        // Parse password and confirm_password
        std::string password = m_pass_buffer.substr(0, 4);
        std::string confirm_password = m_pass_buffer.substr(4, 4);
        DEBUG_MSG(OSD, DEBUG, "pass_buffer: " << m_pass_buffer << "\n");
        DEBUG_MSG(OSD, DEBUG, "password: " << password << "\n");
        DEBUG_MSG(OSD, DEBUG, "confirm_password: " << confirm_password << "\n");
        auto clear_password_fields = [this]()
        {
            lv_timer_t *_timer { nullptr };
            _timer = lv_timer_create(clear_password_fields_callback, 100, this);
            lv_timer_set_repeat_count(_timer, 1);
            lv_timer_set_auto_delete(_timer, true);
        };

        // Check new password
        DELETE_OBJ(m_warning_content);

        if (password != confirm_password)
        {
            m_warning_content = set_label_text_static(m_main_screen, tr(__Nova_senha_e_a_confirmacao_nao_conferem), 0, box_y_spacing - 40, font_semi_25, OSD_COLOR_RED);
            lv_obj_fade_out(m_warning_content, 10000, 0);
            clear_password_fields();
            return;
        }

        // Passwords match, create timer to make it visible
        DEBUG_MSG(OSD, DEBUG, "Passwords match\n");
        lv_timer_t *_timer { nullptr };
        _timer = lv_timer_create(timer_callback, 100, this);
        lv_timer_set_repeat_count(_timer, 1);
        lv_timer_set_auto_delete(_timer, true);
        // Update file with configuration
        State_File::App_State_File file;

        if (file.parental_password != password)
        {
            DEBUG_MSG(OSD, DEBUG, "Parental password changed from " << file.parental_password << " to " << password << "\n");
            memcpy(file.parental_password, password.c_str(), 5);
            file.write();
            Task::post_event(&Task_OSD::update_parental_control);
        }
    }
}

void OSD_Change_Password::timer_callback(lv_timer_t *timer)
{
    OSD_Change_Password *thiz = static_cast<OSD_Change_Password *>(lv_timer_get_user_data(timer));
    Osd_Breadcrumb::s_instance.remove_name();
    Task::post_event(std::bind(thiz->m_callback, true));
    DEBUG_MSG(OSD, DEBUG, "Password change successful\n");
}

void OSD_Change_Password::clear_password_fields_callback(lv_timer_t *timer)
{
    OSD_Change_Password *thiz = static_cast<OSD_Change_Password *>(lv_timer_get_user_data(timer));
    thiz->clear_password_fields();
}

} // namespace mb
