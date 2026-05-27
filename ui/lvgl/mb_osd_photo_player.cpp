#include "mb_osd_photo_player.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"
#include "common/mb_state_file.h"
#include "hal/mb_system.h"
#include "common/mb_globals.h"

#include "tasks/mb_task_player.h"

#include <lvgl.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>
#include <sys/stat.h>

#include <vector>
#include <algorithm> // For std::min

namespace mb {

OSD_Photo_Player::OSD_Photo_Player(OSD *_parent):
    OSD(_parent)
{
}

// Destrutor
OSD_Photo_Player::~OSD_Photo_Player()
{
    remove_focus();
    DELETE_OBJ(m_label_warning);
    DELETE_OBJ(m_image);
    DELETE_OBJ(m_filename);
    DELETE_OBJ(m_next_logo);
    DELETE_OBJ(m_play_stop_logo);
    DELETE_OBJ(m_previous_logo);
    DELETE_OBJ(m_bottom_mask);
    DELETE_OBJ(m_top_mask);
    DELETE_OBJ(m_main_screen);
    DELETE_TIMER(m_timer);
}

bool OSD_Photo_Player::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_VOLTAR pressed\n");

            if (m_callback)
            {
                Task::post_event(m_callback);
            }

            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_OK pressed\n");
            draw_image();
            draw_footer();
            break;
            m_play_state = (m_play_state == Play_State::Play) ? Play_State::Pause : Play_State::Play;
            std::string logo;

            if (m_play_state == Play_State::Play)
            {
                lv_timer_resume(m_timer);
                lv_timer_ready(m_timer);
                logo = LOGO_MEDIA_PLAYER_PAUSE_34X34;
            }
            else
            {
                lv_timer_pause(m_timer);
                logo = LOGO_MEDIA_PLAYER_PLAY_34X34;
            }

            DELETE_OBJ(m_play_stop_logo);
            m_play_stop_logo = load_image(m_bottom_mask, logo.c_str(), play_stop_x, base_line, m_logo_width, m_logo_height);
            break;
        }

        case Remote_Control_Key::KEY_VOLUP:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_VOLUP pressed\n");
            m_selected = (m_selected + 1) % m_playlist.size();
            draw_image();
            draw_footer();

            if (m_play_state == Play_State::Play)
            {
                lv_timer_reset(m_timer);
            }

            break;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            DEBUG_MSG(OSD, DEBUG, "KEY_VOLDOWN pressed\n");
            m_selected = (m_selected + m_playlist.size() - 1) % m_playlist.size();
            draw_image();
            draw_footer();

            if (m_play_state == Play_State::Play)
            {
                lv_timer_reset(m_timer);
            }

            break;
        }

        default:
            break;
    }

    return true;
}

// Inicia o player de fotos
void OSD_Photo_Player::show_photo_player(photo_player_callback_t _callback, lv_obj_t *_bgd, std::vector<std::filesystem::path> _playlist, uint _selected)
{
    set_focus();
    m_playlist = std::move(_playlist);
    m_callback = _callback;
    m_bgd = _bgd;
    m_selected = _selected;
    // Cria tela principal
    m_main_screen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLUE);
    lv_obj_null_on_delete(&m_main_screen);
    draw_image();
    draw_footer();
    init_slide_show();
}

void OSD_Photo_Player::init_slide_show()
{
    // Busca configurações para exibir fotos
    State_File::App_State_File file;
    m_slide_show_enabled = file.slide_show;
    m_display_time = file.slide_show_transition_time;
    m_display_time = m_display_time == 0 ? 3 : m_display_time == 1 ? 5 : 10;
    m_transition_effect = file.slide_show_transition_type;
    m_aspect_ratio = file.slide_show_aspect_ratio;
    if(not m_slide_show_enabled)
    {
        return;
    }
    DEBUG_MSG(OSD, INFO, "display_time=" << static_cast<int>(m_display_time) << "s, transition_effect=" << static_cast<int>(m_transition_effect) << ", aspect_ratio=" << static_cast<int>(m_aspect_ratio) << "\n");

    // Inicia timer para exibição de imagens
    if (!m_timer)
    {
        m_timer = lv_timer_create([](lv_timer_t *timer)
        {
            auto thiz = static_cast<OSD_Photo_Player *>(lv_timer_get_user_data(timer));
            thiz->process_timer();
        }, m_display_time*1000, this);
    }
}

void OSD_Photo_Player::process_timer()
{
    // Incrementa contador de imagens
    m_selected = (m_selected + 1) % m_playlist.size();
    draw_image();
    draw_footer();
}

void OSD_Photo_Player::draw_image()
{
    const auto& file = m_playlist[m_selected];
    std::string path = file.string();

    // Strip "file://" prefix if present
    if (path.compare(0, 7, "file://") == 0)
    {
        path = path.substr(7);
    }

    // Ensure it has the LVGL drive letter
    m_current_path = path;
    if (m_current_path.compare(0, 2, "A:") != 0)
    {
        m_current_path = "A:" + m_current_path;
    }
    DEBUG_MSG(OSD, DEBUG, "File: " << file.filename() << " Path: " << m_current_path << "\n");

    lv_image_header_t img_header {};
    std::string message;

    /* Decodifica imagem */
    lv_res_t res = lv_image_decoder_get_info(m_current_path.c_str(), &img_header);
    if (res != LV_RES_OK)
    {
        /* Pode ser imagem válida, mas grande demais para o decoder */
        DEBUG_MSG(OSD, WARN, "LVGL decoder rejected image: " << m_current_path.c_str() << "\n");
        message =
            std::string(tr(__Imagem_nao_suportada_ou_muito_grande)) +
            "\n" + file.filename().native();
    }
    /* Verificar limite máximo permitido */
    else if (img_header.w > 1920 || img_header.h > 1080)
    {
        message =
            std::string(tr(__Arquivo_muito_grande_dimensao_maxima_1920x1080)) +
            "\n" + file.filename().native();
    }

    /* Em caso de erro, mostra aviso e sai da rotina sem exibir a imagem */
    if (!message.empty())
    {
        DEBUG_MSG(OSD, DEBUG, message << "\n");
        DELETE_OBJ(m_image);
        DELETE_OBJ(m_label_warning);

        m_label_warning = set_label_text(
            m_main_screen,
            message,
            0, 0,
            font_25,
            OSD_COLOR_WHITE
        );
        lv_obj_align(m_label_warning, LV_ALIGN_CENTER, 0, 100);
        lv_obj_null_on_delete(&m_label_warning);

        m_image = load_image(
            m_main_screen,
            LOGO_FILE_PHOTO_SELECTED_68X80,
            0, 0,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT
        );
        lv_obj_center(m_image);
        lv_obj_null_on_delete(&m_image);
        return;
    }

    /* imagem válida → desenha */
    DELETE_OBJ(m_image);
    DELETE_OBJ(m_label_warning);
    m_image = lv_image_create(m_main_screen);
    lv_image_set_src(m_image, m_current_path.c_str());

    lv_obj_center(m_image);
    lv_obj_null_on_delete(&m_image);

    DEBUG_MSG(OSD, DEBUG, "Imagem exibida: " << img_header.w << "x" << img_header.h << "\n");
}

// Desenha rodapé com informações do arquivo
void OSD_Photo_Player::draw_footer()
{
    DELETE_OBJ(m_bottom_mask);
    DELETE_OBJ(m_filename);
    DELETE_OBJ(m_next_logo);
    DELETE_OBJ(m_play_stop_logo);
    DELETE_OBJ(m_previous_logo);
    m_bottom_mask = create_rect(m_main_screen, AREA_X, BOTTOMAREA_Y1, AREA_WIDTH, AREA_HEIGHT, OSD_COLOR_BLACK);
    // Área de nome e número do canal
    const auto &file = m_playlist[m_selected];
    m_filename = set_label_text(m_bottom_mask, file.filename().native(), START_POS_X, 40, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_filename);
    // Exibe imagens na barra inferior
    load_image(m_bottom_mask, LOGO_MIDIABOX_BRANCO_208x30, 983, 40, 208, 40);
    m_next_logo = load_image(m_bottom_mask, LOGO_MEDIA_PLAYER_PREVIOUS_34X34, next_x, base_line, m_logo_width, m_logo_height);
    m_play_stop_logo = load_image(m_bottom_mask, LOGO_MEDIA_PLAYER_PLAY_34X34, play_stop_x, base_line, m_logo_width, m_logo_height);
    m_previous_logo = load_image(m_bottom_mask, LOGO_MEDIA_PLAYER_NEXT_34X34, previous_x, base_line, m_logo_width, m_logo_height);
    lv_obj_fade_out(m_bottom_mask, 3000, 0);
}

} // namespace mb
