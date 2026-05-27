#include "mb_osd.h"
#include "mb_osd_parental_block_screen.h"
#include "mb_osd_translate.h"
#include "mb_zone_id.h"
#include "common/mb_config.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/mb_task_osd.h"
#include "common/mb_state_file.h"
#include <hal/mb_system.h>
#include "hal/mb_sound.h"

#include <lvgl.h>

namespace mb {

OSD_Parental_Block_Screen::OSD_Parental_Block_Screen(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_front_style);
    lv_style_set_bg_opa(&m_front_style, LV_OPA_COVER);
    lv_style_set_bg_color(&m_front_style, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_radius(&m_front_style, LV_RADIUS_CIRCLE);
    update_parental_control();
}

OSD_Parental_Block_Screen::~OSD_Parental_Block_Screen()
{
    //  DEBUG_LINE("Deleting OSD parental block screen objects\n");
    DELETE_TIMER(m_event_status_timer);
    DELETE_OBJ(m_front_box);
    DELETE_OBJ(m_event_status_label);
    DELETE_OBJ(m_event_name_label);
    DELETE_OBJ(m_event_rating_label);
    DELETE_OBJ(m_main_screen);
}

void OSD_Parental_Block_Screen::hide_block_screen()
{
    DELETE_OBJ(m_main_screen);
}

bool OSD_Parental_Block_Screen::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Se a tela não estiver bloqueada, libera todas as teclas para as outras classes
    if (m_status != Status::Blocked)
    {
        return false;
    }

    // Processa tecla recebida
    switch (_event.key)
    {
        // Teclas liberadas para as outras classes mesmo com a tela bloqueada
        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
            // Restaura volume
            if( not m_mute_state )
            {
                Sound::get_instance()->set_volume(m_previous_volume);
            }
            return false;
        case Remote_Control_Key::KEY_VOLTAR:
            return false;

        // Bloqueia teclas no caso de tela bloqueada
        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
        case Remote_Control_Key::KEY_MUTE:
        {
            if ( m_status == Status::Blocked )
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        }

        case Remote_Control_Key::KEY_1:
        case Remote_Control_Key::KEY_2:
        case Remote_Control_Key::KEY_3:
        case Remote_Control_Key::KEY_4:
        case Remote_Control_Key::KEY_5:
        case Remote_Control_Key::KEY_6:
        case Remote_Control_Key::KEY_7:
        case Remote_Control_Key::KEY_8:
        case Remote_Control_Key::KEY_9:
        case Remote_Control_Key::KEY_0:
        {
            process_numeric_key(to_int(_event.key));
            return true;
        }

        default:
            break;
    }

    return true;
}

void OSD_Parental_Block_Screen::delete_hide_block_screen()
{
    // Restaura volume
    if( not m_mute_state )
    {
        Sound::get_instance()->set_volume(m_previous_volume);
    }

    for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
    {
        DELETE_OBJ(m_passwd_info[i].circle);
        DELETE_OBJ(m_passwd_info[i].rect);
    }

    DELETE_OBJ(m_front_box);
    DELETE_OBJ(m_event_status_label);
    DELETE_OBJ(m_event_name_label);
    DELETE_OBJ(m_event_rating_label);
    DELETE_OBJ(m_main_screen);
    remove_focus();
}

void OSD_Parental_Block_Screen::init_parental_control()
{
    m_previous_volume = Sound::get_instance()->get_volume();
    m_mute_state = Sound::get_instance()->mute_state();
    m_next_event = 2 * _1_SECOND; // TEM que vir antes de init_timer()
    init_timer();
    delete_hide_block_screen();
    m_status = Status::Unblocked;
    //Sound::get_instance()->set_volume(0);
}

std::pair<OSD_Parental_Block_Screen::Status, uint8_t> OSD_Parental_Block_Screen::get_event_information()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto service = current_lineup->get_current_service();
    std::pair<Status, uint8_t> result;

    if (service)
    {
        auto events = Task_EIT_Events::get_all_events_for_service(service);

        if (not events.empty())
        {
            auto event = events.front();
            auto pc = get_parental_control_data(static_cast<OSD::Parental_Control>(event.parental_rating));
            auto data = pc.second;
            uint8_t event_age = data.age;
            auto parental_control = pc.first;

            DEBUG_MSG(OSD, DEBUG, event.short_event_descriptor << ": " << (int)event.parental_rating << "\n");
            DEBUG_MSG(OSD, DEBUG, "event.start_time: " << event.start_time.time_to_str() << "\n");
            DEBUG_MSG(OSD, DEBUG, "event.duration: " << (int)event.duration.count() << "\n");
            DEBUG_MSG(OSD, DEBUG, "Idade do evento: " << (int)event_age << " anos, limite de idade: " << (int)m_age_limit << " anos\n");

            // Calcula fim do evento
            UTC_MJD end_time = event.start_time.to_local_time() + static_cast<std::chrono::seconds>(event.duration.count());
            DEBUG_MSG(OSD, DEBUG, "Fim do programa atual: " << end_time.time_to_str() << "\n");
            // Captura hora atual
            auto time_now = System::get_system_time().to_local_time();
            // Calcula segundos para o fim do evento
            m_seconds_to_end = (end_time - time_now).to_unix_epoch();
            DEBUG_MSG(OSD, DEBUG, "seconds_to_end: " << std::dec << (int)m_seconds_to_end << " seconds\n");
            DEBUG_MSG(OSD, DEBUG, "minutes to end = " << std::dec << (int)(m_seconds_to_end / 60) << " minutes\n");

            if (parental_control == Parental_Control::CLASSIFICACAO_LIVRE)
            {
                result = std::make_pair(Status::Unblocked, 0);
                DEBUG_MSG(OSD, DEBUG, "Evento livre para todas as idades\n");
            }
            else if (parental_control == Parental_Control::CLASSIFICACAO_INDEFINIDA)
            {
                result = std::make_pair(Status::Unblocked, 0);
                DEBUG_MSG(OSD, DEBUG, "Evento sem classificação definida\n");
            }
            else if (m_age_limit == 0)
            {
                result = std::make_pair(Status::Unblocked, event_age);
                DEBUG_MSG(OSD, DEBUG, "Controle dos pais desabilitado, evento livre para maiores de " + std::to_string((int)event_age) + " anos\n");
            }
            else if (m_age_limit <= event_age)
            {
                result = std::make_pair(Status::Blocked, event_age);

                DEBUG_MSG(OSD, DEBUG, "Evento bloqueado para menores de " + std::to_string((int)event_age) + " anos\n");
            }
            else
            {
                result = std::make_pair(Status::Unblocked, event_age);
                DEBUG_MSG(OSD, DEBUG, "Evento livre para maiores de " + std::to_string((int)event_age) + " anos\n");
            }

            // When any event is found, set repeater max to 1 minute
            m_next_event = _1_MINUTE;
        }
        else
        {
            result = std::make_pair(Status::Unblocked, 0);
            DEBUG_MSG(OSD, DEBUG, "Nenhum evento encontrado\n");
        }
    }
    else
    {
        result = std::make_pair(Status::Unblocked, 0);
        DEBUG_MSG(OSD, DEBUG, "Nenhum canal encontrado na lista atual.\n");
    }

    return result;
}

void OSD_Parental_Block_Screen::init_timer()
{
    if (!m_event_status_timer)
    {
        //  DEBUG_MSG(TERM_BLUE_BOLD << "Creating timer with period: " << (m_next_event/1000) << " seconds\n" << TERM_RESET);
        m_event_status_timer = lv_timer_create([](lv_timer_t *timer)
        {
            auto thiz = static_cast<OSD_Parental_Block_Screen *>(lv_timer_get_user_data(timer));
            thiz->refresh_timer_callback();
            thiz->restart_timer();
        }, m_next_event, this);
        lv_timer_set_repeat_count(m_event_status_timer, -1);
    }
    else
    {
        restart_timer();
    }
}

void OSD_Parental_Block_Screen::restart_timer()
{
    lv_timer_pause(m_event_status_timer);
    lv_timer_set_period(m_event_status_timer, m_next_event);
    lv_timer_reset(m_event_status_timer);
    lv_timer_resume(m_event_status_timer);
}

void OSD_Parental_Block_Screen::refresh_timer_callback()
{
    auto system_time = System::get_system_time().to_local_time();
    if ( (int)system_time.year() < 2025 )
    {
        DEBUG_MSG(OSD, DEBUG, "Hora do sistema inválida, não verificar eventos\n");
        m_next_event = std::min(_1_MINUTE, (m_next_event + _1_SECOND));
        return;
    }

    std::stringstream stream;
    stream << "Data: "
           << std::dec << std::setw(2) << std::setfill('0') << (int)system_time.day() << "/"
           << std::dec << std::setw(2) << std::setfill('0') << (int)system_time.month() << "/"
           << std::dec << (int)system_time.year() << ", "
           << std::dec << std::setw(2) << std::setfill('0') << (int)system_time.hour() << ":"
           << std::dec << std::setw(2) << std::setfill('0') << (int)system_time.minute() << ":"
           << std::dec << std::setw(2) << std::setfill('0') << (int)system_time.second() << "h\n";
    DEBUG_MSG(OSD, DEBUG, stream.str());

    // If parental control is disabled, m_age_limit == 0, do not check events
    if (m_age_limit == 0)
    {
        m_next_event = 60 * _1_MINUTE;
        DEBUG_MSG(OSD, DEBUG, "Controle dos pais desabilitado, não verificar eventos\n");
        return;
    }

    // If the screen is unblocked by password, do not check again until next minute
    if (m_status == Status::Password_Pressed)
    {
        m_next_event = 60 * _1_MINUTE;
        DEBUG_MSG(OSD, DEBUG, "Digitada senha, próxima verificação em "
        << std::dec << (int)(m_next_event / 1000) << " segundos\n");
        return;
    }

    // If screen is blocked, next interruption is at the end of the event
    if (m_status == Status::Blocked)
    {
        // Unblocking
        DEBUG_MSG(OSD, DEBUG, "Desbloqueando tela ao fim do evento\n");
        delete_hide_block_screen();
        m_status = Status::Unblocked;
        m_next_event = _1_MINUTE;
        return;
    }

    // Check current event information
    auto [status, age] = get_event_information();
    ///auto event = process_event_information();

    if (status == Status::Blocked && m_status == Status::Unblocked)
    {
        // First time blocking
        DEBUG_MSG(OSD, DEBUG, "Bloqueando tela para menores de " << (int)age << " anos\n");
        create_block_screen(std::to_string(age));
        m_status = Status::Blocked;
        m_next_event = m_seconds_to_end * 1000; // Check again at the end of the event
    }
    else if (status == Status::Unblocked && m_status == Status::Blocked)
    {
        // Unblocking
        DEBUG_MSG(OSD, DEBUG, "Desbloqueando tela para menores de " << (int)age << " anos\n");
        delete_hide_block_screen();
        m_status = Status::Unblocked;
        m_next_event = _1_MINUTE;
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Mantendo estado atual da tela: " << (m_status == Status::Blocked ? "BLOQUEADA\n" : "DESBLOQUEADA\n")); 
        m_next_event = std::min(_1_MINUTE, (m_next_event + _1_SECOND));
    }
    DEBUG_MSG(OSD, DEBUG, "Próxima verificação em " << (m_next_event / 1000) << " segundos\n");
}

void OSD_Parental_Block_Screen::create_block_screen(std::string_view _age)
{
    m_main_screen = create_rect(get_main_screen(OSD_Layer::PARENTAL_BLOCK_SCREEN), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_screen);
    load_image(m_main_screen, LOGO_MENU_CENTURY, START_POS_X, 42, 213, 51);
    load_image(m_main_screen, LOGO_MENU_CENTURY_Cinza_400x404, 137, 158, 400, 404);
    add_clock(m_main_screen, 300, 320);
    m_front_box = create_rect(m_main_screen, 0, 0, m_front_box_w, m_front_box_h, OSD_COLOR_GREY_DARK);
    lv_obj_align(m_front_box, LV_ALIGN_CENTER, 250, 0);
    lv_obj_set_style_radius(m_front_box, 25, DEFAULT_SELECTOR);
    lv_style_set_outline_width(&m_front_style, 3);
    lv_style_set_outline_color(&m_front_style, OSD_COLOR_ORANGE);
    lv_style_set_outline_pad(&m_front_style, 4);
    lv_style_set_radius(&m_front_style, 25);
    lv_obj_add_style(m_front_box, &m_front_style, 0);
    auto logo_midiabox = load_image(m_front_box, LOGO_MIDIABOX_BRANCO_150x22, 0, 0, 150, 22);
    lv_obj_align(logo_midiabox, LV_ALIGN_BOTTOM_MID, 0, -15);
    m_event_status_label = set_label_text(m_front_box, "", 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_event_status_label);
    lv_obj_align(m_event_status_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text_fmt(m_event_status_label, tr(__Programa_inadequado_para_menores_de).data(), _age.data());
    auto text = std::string(tr(__Digite_a_senha_para_liberar_o_canal)) + ":";
    m_event_name_label = set_label_text(m_front_box, text, 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_event_name_label);
    lv_obj_align(m_event_name_label, LV_ALIGN_TOP_MID, 0, 60);

    for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
    {
        auto x =  m_password_x + (i * (m_password_w + m_password_gap));
        m_passwd_info[i].rect = create_rect(m_front_box, x, m_password_y, m_password_w, m_password_h, OSD_COLOR_WHITE);
        lv_obj_null_on_delete(&(m_passwd_info[i].rect));
        lv_obj_set_style_border_color(m_passwd_info[i].rect, i == 0 ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
        lv_obj_set_style_border_width(m_passwd_info[i].rect, 3, LV_PART_MAIN);
        m_passwd_info[i].circle = create_rect(m_passwd_info[i].rect, 0, 0, 24, 24, OSD_COLOR_GREY_DARK);
        lv_obj_null_on_delete(&m_passwd_info[i].circle);
        lv_obj_set_style_radius(m_passwd_info[i].circle, m_password_radius, DEFAULT_SELECTOR);
        lv_obj_align(m_passwd_info[i].circle, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
    }

    // Direciona controle remoto para esta tela
    set_focus();
#ifdef MBGUI_USE_RLOTTIE
    m_blocked_logo = lv_rlottie_create_from_file(m_main_screen, 100, 100, ANIM_BLOCKED);
    lv_obj_null_on_delete(&m_blocked_logo);
    lv_obj_align(m_blocked_logo, LV_ALIGN_DEFAULT, 50, DISPLAY_HEIGHT - 150);
    lv_rlottie_set_play_mode(m_blocked_logo, LV_RLOTTIE_CTRL_LOOP);
#endif

    // Desliga volume
    m_previous_volume = Sound::get_instance()->get_volume();
    Sound::get_instance()->set_volume(0);
    DEBUG_MSG(OSD, DEBUG, "Volume anterior: " << (int)m_previous_volume << "\n");
}

void OSD_Parental_Block_Screen::process_numeric_key(int num)
{
    if (m_pass_buffer.length() == MAX_CHAR_PASSWD)
    {
        m_pass_buffer = "";
        for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
        {
            lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_color(m_passwd_info[i].rect, i == 0 ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
        }
    }

    if (m_pass_buffer.length() < MAX_CHAR_PASSWD)
    {
        m_pass_buffer += std::to_string(num);
        lv_obj_clear_flag(m_passwd_info[m_pass_buffer.length() - 1].circle, LV_OBJ_FLAG_HIDDEN);
    }

    DEBUG_MSG(OSD, DEBUG, "Senha digitada: " << m_pass_buffer << "\n");
    if (m_pass_buffer.length() == MAX_CHAR_PASSWD)
    {
        DEBUG_MSG(OSD, DEBUG, "Senha completa digitada, iniciando timer de verificação\n");
        if ( !m_check_password_timer )
        {
            DEBUG_MSG(OSD, DEBUG, "Criando timer de verificação de senha\n");
            m_check_password_timer = lv_timer_create([](lv_timer_t *timer)
            {
                auto thiz = static_cast<OSD_Parental_Block_Screen *>(lv_timer_get_user_data(timer));
                thiz->password_timer_callback();
                lv_timer_pause(timer);
            }, 250, this);
            lv_timer_set_repeat_count(m_check_password_timer, -1);
            lv_timer_set_auto_delete(m_check_password_timer, true);
        }
        else
        {
            DEBUG_MSG(OSD, DEBUG, "Reiniciando timer de verificação de senha\n");
            lv_timer_reset(m_check_password_timer);
            lv_timer_resume(m_check_password_timer);
        }
    }
}

void OSD_Parental_Block_Screen::password_timer_callback()
{
    DEBUG_MSG(OSD, INFO, "Senha digitada: " << m_pass_buffer << ", senha atual: " << m_password << "\n");
    if (m_pass_buffer == m_password)
    {
        // Senha correta
        delete_hide_block_screen();
        m_status = Status::Password_Pressed;
        return;
    }

    // Limpa senha
    m_pass_buffer = "";
    for (auto i = 0; i < MAX_CHAR_PASSWD; i++)
    {
        lv_obj_add_flag(m_passwd_info[i].circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_color(m_passwd_info[i].rect, i == 0 ? OSD_COLOR_ORANGE : OSD_COLOR_GREY_MEDIUM, LV_PART_MAIN);
    }
}

void OSD_Parental_Block_Screen::update_parental_control()
{
    // Update the parental control settings
    State_File::App_State_File file;
    m_parental_rating = file.parental_rating;
    memcpy(m_password, file.parental_password, 5);
    m_age_limit = m_parental_rating == 0 ? 0 :      // Controle dos pais desabilitado
                  m_parental_rating == 1 ? 10 :
                  m_parental_rating == 2 ? 12 :
                  m_parental_rating == 3 ? 14 :
                  m_parental_rating == 4 ? 16 : 18;
    DEBUG_MSG(OSD, DEBUG, "Parental rating: " << (int)m_parental_rating << ", age limit: " << (int)m_age_limit << "\n");
}

} // namespace mb
