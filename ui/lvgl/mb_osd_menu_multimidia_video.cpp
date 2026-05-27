#include "mb_osd_menu_multimidia_video.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"
#include "tasks/mb_task_osd.h"
#include <aui_nim.h>
#include "hal/mb_tuner.h"
#include "common/mb_globals.h"
#include "hal/mb_display.h"

#include <lvgl.h>
#include <stdio.h>
#include <filesystem>

namespace mb {

OSD_Menu_Multimidia_Video *OSD_Menu_Multimidia_Video::s_instance { nullptr };

OSD_Menu_Multimidia_Video::OSD_Menu_Multimidia_Video(OSD *_parent):
    OSD(_parent)
{
    s_instance = this;
}

OSD_Menu_Multimidia_Video::~OSD_Menu_Multimidia_Video()
{
    s_instance = nullptr;
    DELETE_OBJ(m_footer);
    DELETE_OBJ(m_message_box);
    DELETE_OBJ(m_main);
    DELETE_OBJ(m_main_window);
    Osd_Breadcrumb::s_instance.remove_name();
}

bool OSD_Menu_Multimidia_Video::handle_event_remote_control(const Event_Remote_Control &_event)
{
    auto size = m_icon_box.size();

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            if (lv_obj_has_flag(m_bgd, LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_remove_flag(m_bgd, LV_OBJ_FLAG_HIDDEN);
            }

            Task::post_event_cas_pvr_play_stop();
            Task_Player::s_task_player->restore_signal_state();
            Task::post_event(m_callback);
            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            DEBUG_MSG(OSD, DEBUG, "Selected file = " << m_usb_files[m_selected] << "\n");
            photo_player_start();   // Start photo player if the selected file is an image
            video_player_start();   // Start video player if the selected file is a video
            //pvr_player_start();     // Start recording player if the selected file is a recording
            break;
        }

        case Remote_Control_Key::KEY_VOLUP:
        {
            if ((m_selected + 1) < size)
            {
                unselect();
                m_selected = (m_selected + 1) % size;
                select();
            }

            break;
        }

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            if (m_selected != 0)
            {
                unselect();
                m_selected = (m_selected - 1 + size) % size;
                select();
            }

            break;
        }

        case Remote_Control_Key::KEY_CHUP:
        {
            if ((m_selected / max_columns) > 0)
            {
                unselect();
                m_selected = (m_selected + size - max_columns) % size;
                select();
            }

            break;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        {
            unselect();
            m_selected = m_selected + max_columns;

            if (m_selected >= size)
            {
                m_selected = size - 1;
            }

            select();
            break;
        }

        case Remote_Control_Key::KEY_PLUS:
        {
            #warning "TODO: Open settings menu for the selected media file"
            //open_settings_menu();
            break;
        }

        default:
            break;
    }

    return true;
}

void OSD_Menu_Multimidia_Video::show_menu_videos(Osd_Menu_Multimidia_Video_CB_t _callback, lv_obj_t *_bgd, std::vector<std::filesystem::path> _usb_files)
{
    set_focus();
    m_callback = _callback;
    DEBUG_MSG(OSD, DEBUG, "iniciando menu de vídeos\n");
    // Verifica se o arquivo é um vídeo, música, foto ou gravação
    m_usb_files = std::move(_usb_files);
    // Inicia ponteiros de linhas
    m_total_lines = m_usb_files.size() / max_columns;
    m_first_line = 0;
    m_active_line = 0;
    m_last_line = m_total_lines >= m_display_lines ? (m_display_lines - 1) : (m_total_lines - 1);
    // MainMenu
    m_bgd = _bgd;
    m_main_window = create_rect(m_bgd, main_window_x, main_window_y, width, height, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_window);
    //
    auto lines = m_usb_files.size() / max_columns + 1;
    m_main = create_rect(m_main_window, 0, 0, main_w, lines * y_spacing + y_offset, OSD_COLOR_BLACK);
    // Verifia se os arquivos reecebidos são videos ou gravações

    // Define tipo de mídia para o breadcrumb e ícones, e detecta o tipo de cada arquivo para definir o ícone correto
    detect_midia_type();
    std::string_view breadcrumb;
    switch (m_midia_type)
    {
        case Midia_Type::Video:
            breadcrumb = tr(__Videos);
            break;

        case Midia_Type::Recording:
            breadcrumb = tr(__Gravacoes);
            break;

        case Midia_Type::Audio:
            breadcrumb = tr(__Musicas);
            break;

        case Midia_Type::Photo:
            breadcrumb = tr(__Fotos);
            break;

    }
    Osd_Breadcrumb::s_instance.add_name(breadcrumb);
    draw_video_files();
    draw_nav_bar();

    // Rodapé
    // if (m_midia_type == Midia_Type::Video || m_midia_type == Midia_Type::Photo)
    // {
    //     m_footer = MB_OSD_Footer::draw(m_main_window, tr(__Pressione_a_tecla_mais_para_abrir_o_menu_de_configuracoes_ou_a_tecla_voltar_para_voltar_ao_menu_inicial), -60, font_20);
    //     lv_obj_null_on_delete(&m_footer);
    // }
}

void OSD_Menu_Multimidia_Video::video_player_start()
{
    if (m_midia_type == Midia_Type::Photo)
    {
        return;
    }

    DEBUG_MSG(OSD, DEBUG, m_usb_files[m_selected] << "\n");
    lv_obj_add_flag(m_bgd, LV_OBJ_FLAG_HIDDEN);
    Task::post_event_change_osd_media_player_state();
    // Esconde tela sem sinal caso seja vídeo
    auto media_type = OSD_Media_Player::Player_Type::AUDIO;

    if (m_midia_type == Midia_Type::Video)
    {
        media_type = OSD_Media_Player::Player_Type::VIDEO;
    }
    else if (m_midia_type == Midia_Type::Recording)
    {
        media_type = OSD_Media_Player::Player_Type::TS;
    }

    Task::s_task_osd->hide_waiting_signal();

    // Inicia player de vídeo
    if (!m_play_video)
    {
        m_play_video = std::make_unique<OSD_Media_Player>(this);
    }
    m_play_video->show_media_player(std::bind(&OSD_Menu_Multimidia_Video::video_player_callback, this), media_type, m_usb_files, m_selected);
}

void OSD_Menu_Multimidia_Video::video_player_callback()
{
    lv_obj_remove_flag(m_bgd, LV_OBJ_FLAG_HIDDEN);
    Task::post_event_change_osd_home_state();
    m_play_video.reset();
}

void OSD_Menu_Multimidia_Video::photo_player_start()
{
    if (!is_image(m_usb_files[m_selected]))
    {
        return;
    }

    DEBUG_MSG(OSD, DEBUG, m_usb_files[m_selected] << "\n");
    lv_obj_add_flag(m_bgd, LV_OBJ_FLAG_HIDDEN);

    if (!m_play_photo)
    {
        m_play_photo = std::make_unique<OSD_Photo_Player>(this);
    }
    m_play_photo->show_photo_player(std::bind(&OSD_Menu_Multimidia_Video::photo_player_callback, this), m_bgd, m_usb_files, m_selected);
}

void OSD_Menu_Multimidia_Video::photo_player_callback()
{
    lv_obj_remove_flag(m_bgd, LV_OBJ_FLAG_HIDDEN);
    m_play_photo.reset();
}

void OSD_Menu_Multimidia_Video::select()
{
    static uint last_line_selected = 0;
    auto _icon = lv_obj_get_child(m_icon[m_selected], 0);
    DELETE_OBJ(_icon);
    _icon = load_image(m_icon_box[m_selected], m_logo_selected.data(), 0, 0, icon_w, icon_h);
    lv_obj_null_on_delete(&m_icon_box[m_selected]);
    lv_obj_align(_icon, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(m_text_box[m_selected], OSD_COLOR_ORANGE, LV_PART_MAIN);
    lv_bar_set_value(m_bar, m_selected, LV_ANIM_OFF);
    // Verifica alteração na posição do cursor
    uint current_select_line = m_selected / max_columns;

    if (current_select_line == last_line_selected)
    {
        return;
    }

    if (current_select_line > last_line_selected)
    {
        goto_next_line();
    }
    else
    {
        goto_previous_line();
    }

    last_line_selected = current_select_line;
}

void OSD_Menu_Multimidia_Video::unselect()
{
    auto _icon = lv_obj_get_child(m_icon[m_selected], 0);
    DELETE_OBJ(_icon);
    _icon = load_image(m_icon_box[m_selected], m_logo.c_str(), 0, 0, icon_w, icon_h);
    lv_obj_null_on_delete(&m_icon_box[m_selected]);
    lv_obj_align(_icon, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(m_text_box[m_selected], OSD_COLOR_BLACK, LV_PART_MAIN);
}

void OSD_Menu_Multimidia_Video::draw_video_files()
{
    uint line = 0;
    uint column = 0;

    for (const auto &f : m_usb_files)
    {
        std::string_view icon;
        lv_color_t bg_color;

        if (m_selected == (max_columns * line + column))
        {
            icon = m_logo_selected;
            bg_color = OSD_COLOR_ORANGE;
        }
        else
        {
            icon = m_logo;
            bg_color = OSD_COLOR_BLACK;
        }

        // Calculate x and y position
        auto x = column * x_spacing + x_offset;
        auto y = line * y_spacing + y_offset;
        // Draw file name box
        auto icon_box = create_rect(m_main, x, y, box_w, box_h, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(icon_box, 4, LV_PART_MAIN);
        // Draw text box
        auto text_box = create_rect(icon_box, 0, box_h / 2, box_w, box_h / 2, bg_color);
        lv_obj_set_style_radius(text_box, 4, LV_PART_MAIN);
        auto message = set_label_text(text_box, f.filename().native(), 0, 0, font_20, OSD_COLOR_WHITE);
        lv_obj_set_width(message, box_w);
        lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
        lv_obj_center(message);
        lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        // Draw logo
        auto icon_img = load_image(icon_box, icon, 0, 0, icon_w, icon_h);
        lv_obj_align(icon_img, LV_ALIGN_TOP_MID, 0, 0);
        // Increment column and line
        column = (column + 1) % max_columns;
        line = (column == 0) ? line + 1 : line;
        // Salva objetos no vetor
        m_icon_box.push_back(icon_box);
        m_icon.push_back(icon_img);
        m_text_box.push_back(text_box);
    }
}

void OSD_Menu_Multimidia_Video::draw_nav_bar()
{
    DEBUG_MSG(OSD, DEBUG, "size = " << dec << m_usb_files.size() << "\n");
    m_bar = lv_bar_create(m_main_window);
    lv_obj_set_size(m_bar, bar_w, bar_h);
    lv_obj_set_style_bg_color(m_bar, OSD_COLOR_ORANGE, LV_PART_INDICATOR);
    lv_obj_set_pos(m_bar, bar_x, bar_y);

    if (m_usb_files.size() <= maxVisibleItems)
    {
        lv_obj_add_flag(m_bar, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(m_bar, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_range(m_bar, m_usb_files.size(), 0);
        lv_bar_set_value(m_bar, m_usb_files.size() - maxVisibleItems, LV_ANIM_OFF);
    }
}

void OSD_Menu_Multimidia_Video::goto_next_line()
{
    DEBUG_MSG(OSD, DEBUG, "active = " << dec << m_active_line << ", first = " << m_first_line << ", last = " << m_last_line << ", total = " << m_total_lines << "\n");

    if (m_active_line < m_total_lines)
    {
        ++m_active_line;

        if (m_active_line > m_last_line)
        {
            ++m_last_line;
            ++m_first_line;
            reposition_box();
        }
    }

    DEBUG_MSG(OSD, DEBUG, "active = " << dec << m_active_line << ", first = " << m_first_line << ", last = " << m_last_line << ", total = " << m_total_lines << "\n");
}

void OSD_Menu_Multimidia_Video::goto_previous_line()
{
    DEBUG_MSG(OSD, DEBUG, "active = " << dec << m_active_line << ", first = " << m_first_line << ", last = " << m_last_line << ", total = " << m_total_lines << "\n");

    if (m_active_line != 0)
    {
        --m_active_line;

        if (m_active_line < m_first_line)
        {
            --m_last_line;
            --m_first_line;
            reposition_box();
        }
    }

    DEBUG_MSG(OSD, DEBUG, "active = " << dec << m_active_line << ", first = " << m_first_line << ", last = " << m_last_line << ", total = " << m_total_lines << "\n");
}

void OSD_Menu_Multimidia_Video::reposition_box()
{
    auto y = -1 * (m_first_line * y_spacing);
    lv_coord_t current = lv_obj_get_y(m_main);
    //lv_obj_set_y(m_main, y);
    lv_anim_t anim_y;
    lv_anim_init(&anim_y);
    lv_anim_set_var(&anim_y, m_main);
    lv_anim_set_values(&anim_y, current, y);
    lv_anim_set_exec_cb(&anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_duration(&anim_y, 500);
    lv_anim_set_path_cb(&anim_y, lv_anim_path_ease_out);
    lv_anim_start(&anim_y);
}

void OSD_Menu_Multimidia_Video::detect_midia_type()
{
    is_video(m_usb_files[m_selected]);
    is_recording(m_usb_files[m_selected]);
    is_image(m_usb_files[m_selected]);
    is_audio(m_usb_files[m_selected]);
}

bool OSD_Menu_Multimidia_Video::is_video(const std::filesystem::path &_file)
{
    bool result = false;

    const auto ext = _file.extension().native();

    for (const auto &f : COMPATIBLE_VIDEO_FORMATS)
    {
        if (strcasecmp(f.data(), ext.data()) == 0)
        {
            result = true;
            DEBUG_MSG(OSD, DEBUG, ": " << _file << "\n");
            m_midia_type = Midia_Type::Video;
            m_logo = LOGO_FILE_VIDEO_68X80;
            m_logo_selected = LOGO_FILE_VIDEO_SELECTED_68X80;
            break;
        }
    }

    return result;
}

bool OSD_Menu_Multimidia_Video::is_recording(const std::filesystem::path &_file)
{
    bool result = false;
    DEBUG_MSG(OSD, DEBUG, _file << "\n");
    namespace fs = std::filesystem;

    if (fs::is_directory(_file))
    {
        result = true;
        DEBUG_MSG(OSD, DEBUG, _file << " is a directory\n");
        m_midia_type = Midia_Type::Recording;
        m_logo = LOGO_FILE_RECORDING_67X80;
        m_logo_selected = LOGO_FILE_RECORDING_SELECTED_67X80;
    }

    return result;
}

bool OSD_Menu_Multimidia_Video::is_audio(const std::filesystem::path &_file)
{
    bool result = false;

    const auto ext = _file.extension().native();

    for (const auto &f : COMPATIBLE_AUDIO_FORMATS)
    {
        if (strcasecmp(f.data(), ext.data()) == 0)
        {
            result = true;
            DEBUG_MSG(OSD, DEBUG, ": " << _file << "\n");
            m_midia_type = Midia_Type::Audio;
            m_logo = LOGO_FILE_MUSIC_69X80;
            m_logo_selected = LOGO_FILE_MUSIC_SELECTED_69X80;
            break;
        }
    }

    return result;
}

bool OSD_Menu_Multimidia_Video::is_image(const std::filesystem::path &_file)
{
    bool result = false;

    const auto ext = _file.extension().native();

    for (const auto &f : COMPATIBLE_PHOTO_FORMATS)
    {
        if (strcasecmp(f.data(), ext.data()) == 0)
        {
            result = true;
            DEBUG_MSG(OSD, DEBUG, ": " << _file << "\n");
            m_midia_type = Midia_Type::Photo;
            m_logo = LOGO_FILE_PHOTO_68X80;
            m_logo_selected = LOGO_FILE_PHOTO_SELECTED_68X80;
            break;
        }
    }

    return result;
}

void OSD_Menu_Multimidia_Video::open_settings_menu()
{
    if (m_midia_type == Midia_Type::Video)
    {
        if (!m_subtitle_configuration)
        {
            m_subtitle_configuration = std::make_unique<OSD_Subtitle_Configuration>(nullptr);
        }
        m_cover_page = OSD::create_rect(m_main_window, 0, 0, width, height, OSD_COLOR_BLACK);
        m_subtitle_configuration->show_menu_subtitle_configuration([this](bool)
        {
            DELETE_OBJ(m_cover_page);
            m_subtitle_configuration.reset();
        });
    }
    else if (m_midia_type == Midia_Type::Photo)
    {
        if (!m_photo_configuration)
        {
            m_photo_configuration = std::make_unique<OSD_Photo_Configuration>(nullptr);
        }
        m_cover_page = OSD::create_rect(m_main_window, 0, 0, width, height, OSD_COLOR_BLACK);
        m_photo_configuration->show_menu_photo_configuration([this](bool)
        {
            DELETE_OBJ(m_cover_page);
            m_photo_configuration.reset();
        });
    }
}

} // namespace mb
