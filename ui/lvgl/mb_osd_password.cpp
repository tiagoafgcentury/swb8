#include "mb_osd_password.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd_fonts.h"
#include "tasks/mb_task.h"

namespace {
}

static lv_style_t passwdStyle;
namespace mb {

OSD_Password::OSD_Password(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&passwdStyle);
    lv_style_set_bg_color(&passwdStyle, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_border_color(&passwdStyle, OSD_COLOR_GREY);
    lv_style_set_border_width(&passwdStyle, 3);
    Osd_Breadcrumb::s_instance.add_name(tr(__Senha));
}

OSD_Password::~OSD_Password()
{
    lv_style_reset(&passwdStyle);
    DELETE_OBJ(m_main_screen);
    remove_focus();
}

void OSD_Password::show_password(Password_CB_t _callback, lv_obj_t *_bgd, const char *_password)
{
    DEBUG_MSG(OSD, DEBUG, "OSD_PASSWORD\n");
    m_callback = _callback;
    m_main_screen = _bgd;
    memcpy(m_password, _password, sizeof(m_password));
    // Inicializa variáveis de senha
    m_pass_buffer = "";
    // Direciona recepção de tecla
    set_focus();
    lv_obj_null_on_delete(&m_main_screen);
    auto text = set_label_text_static(m_main_screen, tr(__Entre_com_a_senha), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 0, 0);

    for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
    {
        m_passwd_info[i].rect = create_rect(m_main_screen, (i * 94), 37, 83, 83, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&(m_passwd_info[i].rect));
        lv_obj_set_style_border_color(m_passwd_info[i].rect, i == 0 ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
        lv_obj_set_style_border_width(m_passwd_info[i].rect, 3, LV_PART_MAIN);
        m_passwd_info[i].circle = create_rect(m_passwd_info[i].rect, 0, 0, 24, 24, OSD_COLOR_GREY_DARK);
        lv_obj_null_on_delete(&m_passwd_info[i].circle);
        lv_obj_set_style_radius(m_passwd_info[i].circle, 12, DEFAULT_SELECTOR);
        lv_obj_align(m_passwd_info[i].circle, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
    }

    // Mensagem de erro
    m_warning_content = set_label_text_static(m_main_screen, tr(__Senha_incorreta), 0, 0, font_25, OSD_COLOR_RED);
    lv_obj_align(m_warning_content, LV_ALIGN_CENTER, 0, 150);
    lv_obj_add_flag(m_warning_content, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Password::clear_password_fields()
{
    for (int i = 0; i < MAX_CHAR_PASSWD ; i++)
    {
        lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(m_passwd_info[i].rect, OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
    }

    lv_obj_set_style_border_color(m_passwd_info[0].rect, OSD_COLOR_ORANGE, LV_PART_MAIN);
    m_pass_buffer = "";
}

// Processa tecla recebida
bool OSD_Password::handle_event_remote_control(const Event_Remote_Control &_event)
{
    char key = 255;
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
            key = to_char(_event.key);
            break;
        }

        // Pressionado "Voltar"
        case Remote_Control_Key::KEY_VOLTAR:
            Osd_Breadcrumb::s_instance.remove_name();
            Task::post_event(std::bind(m_callback, false));
            return true;

        // Tecla não processada neste contexto
        default:
            DEBUG_MSG(OSD, DEBUG, "Key 0x" << hex << setfill('0') << setw(8) << static_cast<unsigned int>(_event.key) << " not handled in password context\n");
            return true;
    }

    // Adiciona caracter na string
    m_pass_buffer += key;
    // Apaga mensagem de erro
    lv_obj_add_flag(m_warning_content, LV_OBJ_FLAG_HIDDEN);
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

    DEBUG_MSG(OSD, DEBUG, "Password: " << m_pass_buffer << "\n");

    // Ao pressionar a 4a. tecla, inicia timer para verificar senha
    if (m_pass_buffer.size() == MAX_CHAR_PASSWD)
    {
        if (m_warning_content)
        {
            lv_obj_remove_flag(m_warning_content, LV_OBJ_FLAG_HIDDEN);
        }

        // Verifica se senha foi digitada corretamente na quarta tecla
        if (m_pass_buffer == m_password)
        {
            lv_timer_t *_timer { nullptr };
            _timer = lv_timer_create(timer_callback, 100, this);
            lv_timer_set_repeat_count(_timer, 1);
            lv_timer_set_auto_delete(_timer, true);
        }
        else
        {
            lv_timer_t *_timer { nullptr };
            _timer = lv_timer_create(clear_password_fields_callback, 100, this);
            lv_timer_set_repeat_count(_timer, 1);
            lv_timer_set_auto_delete(_timer, true);
        }
    }

    return true;
}

void OSD_Password::timer_callback(lv_timer_t *_timer)
{
    OSD_Password *thiz = static_cast<OSD_Password *>(lv_timer_get_user_data(_timer));
    Osd_Breadcrumb::s_instance.remove_name();
    Task::post_event(std::bind(thiz->m_callback, true));
}

void OSD_Password::clear_password_fields_callback(lv_timer_t *_timer)
{
    OSD_Password *thiz = static_cast<OSD_Password *>(lv_timer_get_user_data(_timer));
    thiz->clear_password_fields();
}

} // namespace mb
