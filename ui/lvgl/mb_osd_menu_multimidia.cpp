#include "mb_osd_menu_multimidia.h"
#include "mb_osd_translate.h"
#include "mb_osd_footer.h"
#include "mb_menu_resources.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"
#include <aui_nim.h>
#include "hal/mb_tuner.h"
#include "common/mb_globals.h"

#include <lvgl.h>
#include <stdio.h>
#include <filesystem>
//#include "esp_mac.h"


namespace mb {

OSD_Menu_Multimidia *OSD_Menu_Multimidia::s_instance { nullptr };

OSD_Menu_Multimidia::OSD_Menu_Multimidia(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, btn_w, btn_h, btn_s, btn_x, btn_y)
{
    s_instance = this;
}

OSD_Menu_Multimidia::~OSD_Menu_Multimidia()
{
    s_instance = nullptr;
    DELETE_OBJ(m_message_box);
    DELETE_OBJ(m_main);
    DELETE_TIMER(m_timer);
    DELETE_TIMER(m_usb_check_timer);
    // Reseta o caminho de migalhas
    Osd_Breadcrumb::s_instance.remove_name();
    Osd_Breadcrumb::s_instance.clear();
}


void OSD_Menu_Multimidia::verify_usb_mounted_timer_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Multimidia *thiz = static_cast<OSD_Menu_Multimidia *>(lv_timer_get_user_data(_tm));
    thiz->verify_usb_mounted_timer();
}

void OSD_Menu_Multimidia::verify_usb_mounted_timer()
{
    namespace fs = std::filesystem;

    if (fs::exists(USB_PATH))
    {
        DELETE_TIMER(m_usb_check_timer);
        if (!m_timer)
        {
            m_status = Status::Detection;
            m_timer = lv_timer_create(process_timer_cb, 1000, this);
            lv_timer_set_repeat_count(m_timer, -1);
            lv_timer_set_auto_delete(m_timer, true);
        }
    }
}

bool OSD_Menu_Multimidia::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(m_callback);
            return true;
        }

        case Remote_Control_Key::KEY_OK:
        {
            if (m_keys.get_enabled_count() == 0)
            {
                return true;
            }

            DELETE_TIMER(m_timer);
            auto selected = static_cast<MultimediaOption>(m_keys.get_selected());

            switch (selected)
            {
                case MultimediaOption::Videos:
                {
                    enter_menu(m_video_files);
                    break;
                }

                case MultimediaOption::Music:
                {
                    enter_menu(m_music_files);
                    break;
                }

                case MultimediaOption::Photos:
                {
                    enter_menu(m_photo_files);
                    break;
                }

                case MultimediaOption::Recordings:
                {
                    enter_menu(m_ts_files);
                    break;
                }

                case MultimediaOption::COUNT:
                default:
                    mb_assert(false);
                    break;
            }

            break;
        }

        case Remote_Control_Key::KEY_CHUP:
        {
            m_keys.previous();
            break;
        }

        case Remote_Control_Key::KEY_CHDOWN:
        {
            m_keys.next();
            break;
        }

        default:
            return true;
    }

    return true;
}

void OSD_Menu_Multimidia::show_menu_multimidia(Osd_Menu_Multimidia_CB_t _callback, lv_obj_t *_bgd)
{
    set_focus();
    m_callback = _callback;
    DEBUG_MSG(OSD, DEBUG, "iniciando menu multimidia\n");
    //MainMenu
    m_bgd = _bgd;
    lv_obj_null_on_delete(&m_bgd);
    m_main = create_rect(m_bgd, main_x, main_y, main_w, main_h, OSD_COLOR_BLACK);
    // Add transparency to the main menu
    lv_obj_null_on_delete(&m_main);
    // Inicializa o caminho de migalhas
    Osd_Breadcrumb::s_instance.init(m_bgd);
    Osd_Breadcrumb::s_instance.add_name(tr(__Multimidia));
    // Desenha botões
    m_keys.clear();
    m_keys.set_background(m_main);

    for (const auto &option : m_options)
    {
        m_keys.add_label(option.text);
    }

    m_keys.set_vertical();
    m_keys.draw();
    // Desabilita todas as teclas
    disable_keys();
    // Desenha caixa de mensagem
    m_message_box = create_rect(m_main, 0, 0, message_box_w, message_box_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_border_width(m_message_box, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(m_message_box, OSD_COLOR_ORANGE, LV_PART_MAIN);
    lv_obj_set_style_radius(m_message_box, 20, DEFAULT_SELECTOR);
    lv_obj_set_width(m_message_box, message_box_w - 20);
    lv_obj_align(m_message_box, LV_ALIGN_CENTER, 0, 0);
    fill_message_box(tr(__Iniciando_multimedia));

    // Cria timer para simular a listagem de arquivos
    if (!m_timer)
    {
        m_timer = lv_timer_create(process_timer_cb, 1000, this);
        lv_timer_set_repeat_count(m_timer, -1);
        lv_timer_set_auto_delete(m_timer, true);
    }
}

void OSD_Menu_Multimidia::process_timer_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Menu_Multimidia *thiz = static_cast<OSD_Menu_Multimidia *>(lv_timer_get_user_data(_tm));
    thiz->process_timer();
}

void OSD_Menu_Multimidia::process_timer()
{
    switch (m_status)
    {
        case Status::Detection:
            DEBUG_MSG(OSD, DEBUG, "detection\n");
            fill_message_box(tr(__Procurando_pendrive_por_favor_aguarde));
            detect_flash_disk();
            break;

        case Status::Detected:
            DEBUG_MSG(OSD, DEBUG, "detected\n");
            fill_message_box(tr(__Listando_arquivos_de_multimidia));
            list_usb_files();

            if (m_video_files.empty() && m_music_files.empty() && m_photo_files.empty() && m_ts_files.empty())
            {
                m_status = Status::Empty;
            }
            else
            {
                enable_keys();
                m_status = Status::Listing;
            }

            break;

        case Status::Listing:
            DEBUG_MSG(OSD, DEBUG, "listing\n");
            //fill_message_box(tr(__Selecione_a_opcao_desejada));
            DELETE_OBJ(m_message_box);
            m_status = Status::Waiting;
            break;

        case Status::Waiting:
            DEBUG_MSG(OSD, DEBUG, "waiting\n");
            lv_timer_pause(m_timer);
            // Aguardando a seleção do usuário
            break;

        case Status::Empty:
            DEBUG_MSG(OSD, DEBUG, "empty\n");
            fill_message_box(std::string(tr(__Nenhum_arquivo_foi_encontrado)));
            m_status = Status::Finished;
            break;

        case Status::Device_not_found:
            DEBUG_MSG(OSD, DEBUG, "device not found\n");
            fill_message_box(std::string(tr(__Nenhum_dispositivo_USB_encontrado)));
            m_status = Status::Finished;
            break;

        case Status::Failed:
            DEBUG_MSG(OSD, DEBUG, "failed\n");
            m_status = Status::Finished;
            break;

        case Status::Finished:
            DEBUG_MSG(OSD, DEBUG, "finished\n");
            //fill_message_box(tr(__Finalizando));
            m_status = Status::Leaving;
            break;

        case Status::Leaving:
            DEBUG_MSG(OSD, DEBUG, "leaving\n");
            //Task::post_event(m_callback);
            DELETE_TIMER(m_timer);
            if (!m_usb_check_timer)
            {
                m_usb_check_timer = lv_timer_create(verify_usb_mounted_timer_cb, 3000, this);
                lv_timer_set_repeat_count(m_usb_check_timer, -1);
                lv_timer_set_auto_delete(m_usb_check_timer, true);
            }
            break;

        default:
            break;
    }
}

void OSD_Menu_Multimidia::fill_message_box(std::string_view _text)
{
    DELETE_OBJ(m_message_text);
    m_message_text = set_label_text(m_message_box, _text.data(), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_set_width(m_message_text, message_box_w - 20);
    lv_label_set_long_mode(m_message_text, LV_LABEL_LONG_WRAP);
    lv_obj_align(m_message_text, LV_ALIGN_CENTER, 20, 0);
}

void OSD_Menu_Multimidia::detect_flash_disk()
{
    namespace fs = std::filesystem;

    if (fs::exists(USB_PATH))
    {
        m_status = Status::Detected;
        DEBUG_MSG(OSD, DEBUG, "Caminho USB existe: " << USB_PATH << "\n");
    }
    else
    {
        m_status = Status::Device_not_found;
        DEBUG_MSG(OSD, DEBUG, "Caminho USB não existe: " << USB_PATH << "\n");
    }
}

void OSD_Menu_Multimidia::disable_keys()
{
    for (size_t index = 0 ; index < m_keys.get_size() ; index++)
    {
        m_keys.set_disabled(index);
    }
}

void OSD_Menu_Multimidia::enable_keys()
{
    m_keys.select();

    for (auto it = m_keys.get_size() ; it ; it--)
    {
        m_keys.next();
        DEBUG_MSG(OSD, DEBUG, "Habilitando tecla: " << m_keys.get_selected() << "\n");

        if (m_keys.is_first_enabled())
        {
            break;
        }
    }
}

template <typename T>
void filter_files(const T &_formats, std::vector<std::filesystem::path> *_files, const std::string &_name)
{
    for (const auto &f : _formats)
    {
        if (_name.ends_with(f))
        {
            _files->push_back(_name);
            break;
        }
    }
}

void OSD_Menu_Multimidia::list_usb_files()
{
#ifndef NDEBUG
    // Medir tempo de execução
    auto start = std::chrono::steady_clock::now();
#endif

    // Carrega arquivos da usb
    // @todo: Caminho usb alterado temporariamente para testes, Heron 2025-07-07
    for (const auto &dir_entry : std::filesystem::recursive_directory_iterator(USB_PATH))
    {
        if (std::filesystem::is_regular_file(dir_entry) && dir_entry.path().string().find(".Trash") == std::string::npos)
        {
            // Verifica se é um arquivo de vídeo
            auto name = std::string("file://") + std::string(dir_entry.path());
            filter_files(COMPATIBLE_VIDEO_FORMATS, &m_video_files, name);
            filter_files(COMPATIBLE_AUDIO_FORMATS, &m_music_files, name);
            filter_files(COMPATIBLE_PHOTO_FORMATS, &m_photo_files, name);
#ifndef NDEBUG
            {
                auto end = std::chrono::steady_clock::now();
                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                DEBUG_MSG(OSD, DEBUG, " " << (diff.count()) << " ms - " << name << "\n");
            }

#endif
        }
    }

    // Verifica recursivamente se existe uma pasta "PVR" dentro de USB_PATH
    for (const auto &dir_entry : std::filesystem::recursive_directory_iterator(USB_PATH))
    {
        if (std::filesystem::is_directory(dir_entry) && dir_entry.path().filename() == "PVR")
        {
            auto path = dir_entry.path();
            DEBUG_MSG(OSD, DEBUG, "Pasta PVR encontrada: " << path << "\n");

            // Percorre as subpastas dentro de PVR
            for (const auto &sub_dir_entry : std::filesystem::directory_iterator(path))
            {
                if (std::filesystem::is_directory(sub_dir_entry) && sub_dir_entry.path().string().find(".Trash") == std::string::npos)
                {
                    // Verifica se é uma pasta de gravação
                    auto name = std::string(sub_dir_entry.path());
                    m_ts_files.push_back(name);
                    DEBUG_MSG(OSD, DEBUG, "Gravação encontrada: " << name << "\n");
                }
            }
        }
    }

    DEBUG_MSG(OSD, DEBUG, "Arquivos de vídeos = "   << m_video_files.size() << "\n"
              "Arquivos de músicas = "  << m_music_files.size() << "\n"
              "Arquivos de fotos = "    << m_photo_files.size() << "\n"
              "Arquivos de gravação = " << m_ts_files.size() << "\n");

    if (m_video_files.size() > 0)
    {
        m_keys.set_enabled(static_cast<size_t>(MultimediaOption::Videos));
    }

    if (m_music_files.size() > 0)
    {
        m_keys.set_enabled(static_cast<size_t>(MultimediaOption::Music));
    }

    if (m_photo_files.size() > 0)
    {
        m_keys.set_enabled(static_cast<size_t>(MultimediaOption::Photos));
    }

    if (m_ts_files.size() > 0)
    {
        m_keys.set_enabled(static_cast<size_t>(MultimediaOption::Recordings));
    }

#ifndef NDEBUG
    {
        auto end = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        DEBUG_MSG(OSD, DEBUG, "Tempo de listagem: " << (diff.count()) << " ms\n");
    }

#endif
}

void OSD_Menu_Multimidia::enter_menu(std::vector<std::filesystem::path> _usb_files)
{
    if (!m_osd_menu_video)
    {
        m_osd_menu_video = std::make_unique<OSD_Menu_Multimidia_Video>(this);
    }

    m_osd_menu_video->show_menu_videos(std::bind(&OSD_Menu_Multimidia::menu_callback, this), m_bgd, std::move(_usb_files));
}

void OSD_Menu_Multimidia::menu_callback()
{
    m_osd_menu_video.reset();
}

} // namespace mb
