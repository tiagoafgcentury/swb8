#include "mb_osd_software_update.h"
#include "../../project_version.h"
#include "common/mb_version.h"

#include "mb_menu_resources.h"
#include "mb_osd_footer.h"
#include "mb_osd_translate.h"
#include "mb_zone_id.h"

#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"

#include "fw_env.h"

#include "tasks/mb_task.h"
#include "tasks/mb_task_tuner.h"

#include "hal/mb_lnb_config.h"
#include "hal/mb_tuner.h"
#include "hal/mb_watchdog.h"

#include <lvgl.h>
#include <filesystem>
#include <regex>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/mount.h>

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace mb {

OSD_Software_Update *OSD_Software_Update::s_instance{nullptr};

OSD_Software_Update::OSD_Software_Update(OSD *_parent):
    OSD(_parent),
    m_options(0, offset_y, option_w, option_h, option_s, option_x, option_y)
{
    m_current_progress = 0;
    s_instance = this;
}

OSD_Software_Update::~OSD_Software_Update()
{
    m_tp_params.clear();
    m_files.clear();
    DELETE_TIMER(m_refresh_timer);
    DELETE_TIMER(m_sw_update_timer);
    DELETE_OBJ(m_bgd_main);
    DELETE_OBJ(m_main);
    s_instance = nullptr;
#ifdef MBGUI_FORCED_UPDATE
    Osd_Breadcrumb::s_instance.remove_name();
#endif
}

bool OSD_Software_Update::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Durante a detecção do OTA, não aceita eventos do controle remoto
    if (m_status != Status::Updated
            and m_status != Status::Success
            and m_status != Status::Fail)
    {
        DEBUG_MSG(OSD, WARN, "OTA em andamento, não aceita eventos do controle remoto\n");
        return true;
    }

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            if (m_status != Status::Fail)
            {
                DEBUG_MSG(OSD, DEBUG, "Voltar\n");
                stop_timer(false);
            }

            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            DEBUG_MSG(OSD, DEBUG, "OK\n");

            if (m_status == Status::Success or m_status == Status::Updated)
            {
                stop_timer(true);
            }
            else if (m_status == Status::Fail)
            {
                if (m_is_more_informations)
                {
                    if (m_updt_sattelite)
                    {
                        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Atualizacao_de_software_via_satelite));
                    }
                    else
                    {
                        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Atualizacao_de_software_via_usb));
                    }

                    m_is_more_informations = false;
                    lv_obj_add_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
                }
                else
                {
                    if (m_options.get_selected() == 0)
                    {
                        Osd_Breadcrumb::s_instance.replace_last_name(tr(__Mais_Informacoes));
                        m_is_more_informations = true;
                        lv_obj_remove_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
                    }
                    else
                    {
                        stop_timer(true);
                        Osd_Breadcrumb::s_instance.remove_name();
                    }
                }
            }

            return true;
        }

        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
        {
            if (m_status == Status::Fail)
            {
                m_options.next();
            }

            return true;
        }

        default:
            break;
    }

    return true;
}

void OSD_Software_Update::show_menu_software_update(Software_Update_CB_t _callback, bool satellite, bool _is_easy_install)
{
    set_focus();
    m_callback = _callback;
    m_is_easy_install = _is_easy_install;
    m_updt_sattelite = satellite;
    // MainMenu
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    create_more_informations_screen();
    // Criar barra de progresso
    create_progress_bar();

    // Cria timer para 10 segundos
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    // Popula tela
    if (satellite)
    {
        auto network_id = Config::get_config()->selected_satellite_config().network_id;
        if(network_id == Network_Id_Sky)
        {      
            populate(Status::Sky_Init);
            m_sky_timer = 0 ;
        }
        else
        {
            m_tp_params = MB_Satellites::get_transponder_list_for_ota(Satellite_Operator::Claro);
            populate(Status::TP_Init);
        }  
        Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_software_via_satelite));
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Selecionado atualização via USB\n");
        // Caminho de migalhas
        Osd_Breadcrumb::s_instance.add_name(tr(__Atualizacao_de_software_via_usb));
        populate(Status::USB);
    }
}

void OSD_Software_Update::force_sw_update_cb(lv_timer_t *tm)
{
    if (tm == nullptr or lv_timer_get_user_data(tm) == nullptr)
    {
        return;
    }

    OSD_Software_Update *thiz = static_cast<OSD_Software_Update *>(lv_timer_get_user_data(tm));
#ifndef NDEBUG
#ifndef MBMODO_NFS
    fw_env_open();
    fw_env_write("upg_flag", "1");

    if (thiz->m_is_easy_install)
    {
        fw_env_write("sw_updt_status_flag", "3");
    }
    else
    {
        fw_env_write("sw_updt_status_flag", "1");
    }

    if (thiz->m_updt_sattelite)
    {
        const auto config = Config::get_config();
        const auto &tp = thiz->m_tp_params[thiz->m_tp_pointer];
        auto frequency = tp.transponder.transponder_id.frequency() / 1000;
        auto pol = (tp.transponder.transponder_id.polarity() == Polarity::Horizontal ? "h" : "v");
        auto lnbf_config = get_lnbf_config(config->band(), config->lnbf_type(), false, frequency, tp.transponder.transponder_id.polarity());
        auto tone = lnbf_config.tone_22khz ? "on" : "off";
        DEBUG_MSG(OSD, DEBUG, "upg_flag = 1 \n");
        DEBUG_MSG(OSD, DEBUG, "frequency = " << lnbf_config.l_freq << "\n");
        DEBUG_MSG(OSD, DEBUG, "symbol_rate = " << tp.transponder.symbol_rate << "\n");
        DEBUG_MSG(OSD, DEBUG, "pol = " << pol << "\n");
        DEBUG_MSG(OSD, DEBUG, "22khz = " << tone << "\n");
        DEBUG_MSG(OSD, DEBUG, "PID = " << tp.pid << "\n");
        DEBUG_MSG(OSD, DEBUG, "Tempo de detecção: " << thiz->m_detection_time << " <----\n");
        fw_env_write_int("frequency", lnbf_config.l_freq);
        fw_env_write_int("symbol_rate", tp.transponder.symbol_rate);
        fw_env_write("pol", pol);
        fw_env_write("22khz", tone);
        fw_env_write_int("PID", tp.pid);
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "FILE TO UPDATE: " << thiz->m_strFile.c_str() << "\n");
        fw_env_write("otatype", "usb");
        fw_env_write("usbotapath", thiz->m_strFile.c_str());
    }
    fw_env_close();

#if MBGUI_FORCED_UPDATE
    Task::post_event_system_factory_reset();
#endif
    Watchdog::system_reset();
    return;
#endif
#endif
}

bool OSD_Software_Update::is_pendrive_mounted()
{
    std::ifstream file("/proc/self/mountinfo");

    if (!file.is_open())
    {
        DEBUG_MSG(OSD, ERROR, "Erro ao abrir o arquivo /proc/self/mountinfo\n");
        return false;
    }

    std::string line;
    std::string target_mount = USB_PATH;
    std::string mount_point;
    std::string device;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string field;
        std::vector<std::string> fields;

        // Dividir a linha em campos
        while (iss >> field)
        {
            fields.push_back(field);
        }

        // Verificar se a linha contém a pasta de montagem desejada
        DEBUG_MSG(OSD, DEBUG, "fields:  " << fields[4] << "\n");

        if (fields.size() > 4 && fields[4].find(target_mount) == 0)
        {
            mount_point = fields[4];
            device = fields[8];
            break;
        }
    }

    file.close();

    if (!mount_point.empty() && !device.empty())
    {
        DEBUG_MSG(OSD, DEBUG, "Pasta de montagem: " << mount_point << "\n");
        DEBUG_MSG(OSD, DEBUG, "Dispositivo: " << device << "\n");
        return true;
    }
    else
    {
        DEBUG_MSG(OSD, WARN, "Pasta de montagem não encontrada.\n");
    }

    return false;
}

void OSD_Software_Update::to_usb()
{
    // Atraso para efeito visual
    if (m_current_progress == 1)
    {
        DEBUG_MSG(OSD, DEBUG, "USB_PATH: " << USB_PATH << "\n");
        std::error_code _;

        if (is_pendrive_mounted() && std::filesystem::is_directory(USB_PATH, _))
        {
            m_files = list_usb_files(USB_PATH);

            if (m_files.size() == 0)
            {
                m_sc_fail = tr(__Arquivo_nao_encontrado);
                lv_label_set_text(m_subtitle, tr(__Arquivo_nao_encontrado).data());
                DEBUG_MSG(OSD, WARN, "Software Update file NOT found!" << "\n");
                return;
            }

            DEBUG_MSG(OSD, DEBUG, "UPDATE_FILE: " << m_files[0] << "\n");
            m_strFile = "/" + m_files[0];
            lv_label_set_text(m_subtitle, tr(__Atualizacao_encontrada_com_sucesso_Midiabox_sera_reiniciado).data());
            m_current_progress = 100;
            DELETE_OBJ(m_footer);
            lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
            lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);

            // Cria timer para exibir mensagem na tela
            if (!m_sw_update_timer)
            {
                m_sw_update_timer = lv_timer_create(force_sw_update_cb, 2000, this);
                lv_timer_set_repeat_count(m_sw_update_timer, 1);
            }
        }
        else
        {
            m_sc_fail = tr(__Pendrive_nao_encontrado);
            lv_label_set_text(m_subtitle, tr(__Pendrive_nao_encontrado).data());
            DEBUG_MSG(OSD, WARN, "Pendrive não encontrado\n");
            return;
        }
    }
    else if (m_current_progress == 3)
    {
        m_current_progress = 99;
        to_fail();
    }

    return;
}

std::vector<std::string> OSD_Software_Update::list_usb_files(std::string_view _path)
{
    // Raiz onde todos os arquivos de vídeo serão buscados
    std::vector<std::string> result = {};
    std::vector<std::string> files = {};

    // Iterate over the std::filesystem::directory_entry elements using `auto`
    for (const auto &dir_entry : fs::recursive_directory_iterator(_path))
    {
        if (dir_entry.is_regular_file() &&
                dir_entry.path().string().find("/.") == std::string::npos)
        {
            DEBUG_MSG(OSD, DEBUG, "File: " << dir_entry.path().filename().string() << "\n");
            files.push_back(dir_entry.path().filename());
        }
    }

    // Filtra arquivos de vídeo compatíveis
    static constexpr auto filter_updates = { ".b8" };

    for (const auto &f : filter_updates)
    {
        std::copy_if(files.begin(), files.end(),
                     std::back_inserter(result),
                     [f](const std::string & str)
        {
            return str.find(f) != std::string::npos;
        });
    }

    for (auto r : result)
    {
        DEBUG_MSG(OSD, DEBUG, "[" << __FUNCTION__ << "]" << " - " << r << "\n");
    }

    return result;
}

void OSD_Software_Update::to_success()
{
    // Desabilita o callback
    m_ota_callback.reset();
    lv_timer_pause(m_refresh_timer);
    populate(Status::Success);
    m_current_progress = 10;
    DELETE_OBJ(m_slider_label);
    DELETE_OBJ(m_slider);
    draw_button();
}

void OSD_Software_Update::to_fail()
{
    // Desabilita o callback
    m_ota_callback.reset();

    if (m_refresh_timer)
    {
        lv_timer_pause(m_refresh_timer);
    }

    populate(Status::Fail);
    m_current_progress = 10;
    DELETE_OBJ(m_slider_label);
    DELETE_OBJ(m_slider);
    draw_buttons();
}

void OSD_Software_Update::show_menu_channel_list_update_callback()
{
    if (!mb_osd_activate)
    {
        mb_osd_activate = std::make_unique<OSD_Activate>(this);
    }

    auto stb_activated = false;
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    if(Zone_ID::get_zone_id(_oper) > 0)
    {
        stb_activated = true;
    }
    mb_osd_activate->show_menu_activate(std::bind(&OSD_Software_Update::show_menu_activate_callback, this, std::placeholders::_1), stb_activated);
}

void OSD_Software_Update::show_menu_activate_callback(bool _result)
{
    stop_timer(_result);
    mb_osd_activate.reset();
    m_osd_channel_list_update.reset();
    Osd_Breadcrumb::s_instance.remove_name();
}

void OSD_Software_Update::looking_for_channel_list_update()
{
    if (!m_osd_channel_list_update)
    {
        m_osd_channel_list_update = std::make_unique<OSD_Channel_List_Update>(this);
    }

    m_osd_channel_list_update->show_menu_channel_list_update(std::bind(&OSD_Software_Update::show_menu_channel_list_update_callback, this));
}

void OSD_Software_Update::create_progress_bar()
{
    /*Create a slider in the center of the display*/
    m_slider = lv_slider_create(m_main);
    lv_obj_null_on_delete(&m_slider);
    lv_obj_set_pos(m_slider, slider_x, slider_y);
    lv_obj_set_size(m_slider, slider_w, slider_h);
    lv_obj_set_style_anim_duration(m_slider, 2000, 0);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_main, "", slider_label_x, slider_label_y, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_slider_label);
    lv_label_set_text(m_slider_label, "");
    // Set the indicator color to green
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_BLUE, LV_PART_INDICATOR);
    // Set the background color to light gray
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREY, LV_PART_MAIN);
    // Make the knob completely transparent
    lv_obj_set_style_bg_opa(m_slider, LV_OPA_TRANSP, LV_PART_KNOB);
}

void OSD_Software_Update::refresh_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr or lv_timer_get_user_data(_tm) == nullptr)
    {
        return;
    }

    OSD_Software_Update *thiz = static_cast<OSD_Software_Update *>(lv_timer_get_user_data(_tm));
    thiz->refresh_progress();
}

void OSD_Software_Update::refresh_progress()
{
    m_detection_time++;
    m_ota_current_time++;

    if (m_current_progress <= 100)
    {
        lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
        ++m_current_progress;
    }

    switch (m_status)
    {
        case Status::TP_Init:
            tp_init();
            break;

        case Status::TP_Lock:
            tp_lock();
            break;

        case Status::TP_Locked:
            tp_locked();
            break;

        case Status::TP_Lock_Fail:
            tp_lock_fail();
            break;

        case Status::OTA_Detect:
            ota_detect();
            break;

        case Status::OTA_Detected:
            break;

        case Status::OTA_Detect_Fail:
            ota_detect_fail();
            break;

        case Status::Fail:
            to_fail();
            break;

        case Status::Success:
            to_success();
            break;

        case Status::USB:
            to_usb();
            break;

        case Status::Updated:
            break;

        case Status::Sky_Init:
            process_ota_sky();
            break;
    }
}

void OSD_Software_Update::tp_lock_fail()
{
    DEBUG_MSG(OSD, DEBUG, "[" << __FUNCTION__ << "]" << " - " << __LINE__ << "\n");
    ++m_tp_pointer;
    m_sc_fail = tr(__Atualizacao_via_satelite_nao_encontrada);
    populate(Status::TP_Init);
}

void OSD_Software_Update::ota_detect_fail()
{
    DEBUG_MSG(OSD, DEBUG, "[" << __FUNCTION__ << "]" << " - " << __LINE__ << "\n");
    m_sc_fail = tr(__Atualizacao_via_satelite_nao_encontrada);
    ++m_tp_pointer;
    populate(Status::TP_Init);
}

void OSD_Software_Update::ota_detect()
{
    if (++m_ota_current_time == s_ota_timeout)
    {
        populate(Status::OTA_Detect_Fail);
    }
}

void OSD_Software_Update::tp_locked()
{
    const auto &tp = m_tp_params[m_tp_pointer];
    m_ota_callback = std::make_shared<Event_OTA_DSI>();
    m_ota_callback->callback = process_ota_cb;
    Task::post_event_ota_update_get(tp.pid, m_ota_callback);
    populate(Status::OTA_Detect);
    m_ota_current_time = 0;
}

void OSD_Software_Update::process_ota_sky()
{
    if ( ++m_sky_timer < 3 )
    {
        return ;
    }

    m_tp_params = MB_Satellites::get_transponder_list_for_ota(Satellite_Operator::Sky);
    DEBUG_MSG(OSD, DEBUG, "Total de transponder encontrados: " << m_tp_params.size() << "\n");
    if ( m_tp_params.size() == 0 )
    {
        m_sc_fail = tr(__Atualizacao_via_satelite_nao_encontrada);
        populate(Status::Fail);
        m_current_progress = 100;
        lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
        return ;
    }

    for (const auto &tp : m_tp_params)
    {
        DEBUG_MSG(OSD, DEBUG, "Frequency: " << std::dec << tp.transponder.transponder_id.frequency() / 1000 << " MHz"
            << "\n\tSymbol Rate: " << std::dec << tp.transponder.symbol_rate << " kbps"
            << "\n\tPolarity: " << (tp.transponder.transponder_id.polarity() == Polarity::Horizontal ? "H" : "V")
            << "\n\tPID: " << std::dec << tp.pid
            << "\n\tSoftware version: " << std::dec << tp.software_version << "\n");
    }
    auto _software_ota = m_tp_params[m_tp_pointer].software_version;

    populate(Status::OTA_Detected);
    m_current_progress = 100;
    lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
    lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
    // Valores recebidos via OTA
    auto software_installed_str = MB_OSD_Version::get_major_minor_version_str();
    uint16_t software_installed = std::stoi(software_installed_str);
    DEBUG_MSG(OSD, INFO, "Software instalado: " << dec << software_installed << "\n");
    DEBUG_MSG(OSD, INFO, "Software ota: " << dec << _software_ota << "\n");

    if (software_installed >= _software_ota)
    {
        DEBUG_MSG(OSD, WARN, "A versão de software encontrada não é mais recente que a instalada\n");
        populate(Status::Updated);
        draw_button();
        return;
    }

    lv_label_set_text(m_subtitle, tr(__Atualizacao_encontrada_com_sucesso_Midiabox_sera_reiniciado).data());
    DELETE_OBJ(m_footer);

    if (!m_sw_update_timer)
    {
        m_sw_update_timer = lv_timer_create(force_sw_update_cb, 1000, this);
        lv_timer_set_repeat_count(m_sw_update_timer, 1);
    }

}

void OSD_Software_Update::process_ota([[maybe_unused]] uint32_t _product, uint16_t _software_ota, [[maybe_unused]] uint16_t _sw_min)
{
    populate(Status::OTA_Detected);
    m_current_progress = 100;
    lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
    lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
    // Valores recebidos via OTA
    DEBUG_MSG(OSD, INFO, "Produto: " << _product << "\n");
    DEBUG_MSG(OSD, INFO, "Software mínimo: " << _sw_min << "\n");
    // Valores do software instalado
    auto software_installed_str = MB_OSD_Version::get_major_minor_version_str();
    uint16_t software_installed = std::stoi(software_installed_str);
    DEBUG_MSG(OSD, INFO, "Software instalado: " << dec << software_installed << "\n");
    DEBUG_MSG(OSD, INFO, "Software ota: " << _software_ota << "\n");

    if (software_installed >= _software_ota)
    {
        DEBUG_MSG(OSD, WARN, "A versão de software encontrada não é mais recente que a instalada\n");
        populate(Status::Updated);
        draw_button();
        return;
    }

    lv_label_set_text(m_subtitle, tr(__Atualizacao_encontrada_com_sucesso_Midiabox_sera_reiniciado).data());
    DELETE_OBJ(m_footer);

    if (!m_sw_update_timer)
    {
        m_sw_update_timer = lv_timer_create(force_sw_update_cb, 1000, this);
        lv_timer_set_repeat_count(m_sw_update_timer, 1);
    }
}

void OSD_Software_Update::process_ota_cb(uint32_t _product, uint16_t _sw, uint16_t _sw_min)
{
    s_instance->process_ota(_product, _sw, _sw_min);
}

void OSD_Software_Update::tp_lock()
{
    auto res = Task_Tuner::is_locked();
    DEBUG_MSG(OSD, DEBUG, "res = " << res << ", tp_lock_current_time = " << m_tp_lock_current_time << "\n");

    if (res)
    {
        populate(Status::TP_Locked);
        m_ota_current_time = 0;
    }
    else if (++m_tp_lock_current_time == s_tp_lock_timeout)
    {
        populate(Status::TP_Lock_Fail);
    }
}

void OSD_Software_Update::tp_init()
{
    // Inicia busca de novo transponder
    if (m_tp_pointer < m_tp_params.size())
    {
        auto &tp = m_tp_params[m_tp_pointer];
        Task::post_event_transponder_lock(POST_CALLER &tp.transponder);
        m_tp_lock_current_time = 0;
        populate(Status::TP_Lock);
    }
    else
    {
        populate(Status::Fail);
        m_current_progress = 100;
        lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
    }
}

void OSD_Software_Update::populate(Status _status)
{
    m_status = _status;
    switch (m_status)
    {
        case Status::USB:
        {
            DEBUG_MSG(OSD, DEBUG, "Atualização via USB\n");
            break;
        }

        case Status::TP_Init:
        {
            DEBUG_MSG(OSD, DEBUG, "Buscando transponder\n");
            break;
        }

        case Status::TP_Lock:
        {
            DEBUG_MSG(OSD, DEBUG, "Tentando locar transponder\n");
            break;
        }

        case Status::TP_Locked:
        {
            DEBUG_MSG(OSD, DEBUG, "Transponder locado\n");
            break;
        }

        case Status::TP_Lock_Fail:
        {
            DEBUG_MSG(OSD, WARN, "Falha ao locar transponder\n");
            break;
        }

        case Status::OTA_Detect:
        {
            DEBUG_MSG(OSD, DEBUG, "Detectando OTA\n");
            break;
        }

        case Status::OTA_Detected:
        {
            DEBUG_MSG(OSD, DEBUG, "OTA detectado\n");
            break;
        }

        case Status::OTA_Detect_Fail:
        {
            DEBUG_MSG(OSD, WARN, "Falha ao detectar OTA\n");
            break;
        }

        case Status::Fail:
        {
            DEBUG_MSG(OSD, WARN, "Falha na atualização\n");
            break;
        }

        case Status::Success:
        {
            DEBUG_MSG(OSD, DEBUG, "Atualização realizada com sucesso\n");
            break;
        }

        case Status::Updated:
        {
            DEBUG_MSG(OSD, DEBUG, "Software atualizado\n");
            break;
        }
        case Status::Sky_Init:
        {
            DEBUG_MSG(OSD, DEBUG, "Atualização via satélite Sky\n");
            break;
        }
    }

    const auto &sc = m_screen_content[_status];
    // Título
    DELETE_OBJ(m_title);
    m_title = set_label_text(m_main, sc.title, 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title);
    lv_obj_align(m_title, LV_ALIGN_TOP_MID, 0, title_y);
    // Subtítulo
    DELETE_OBJ(m_subtitle);
    if (_status == Status::Fail)
    {
        m_subtitle = set_label_text(m_main, m_sc_fail, 0, 0, font_25, OSD_COLOR_WHITE);
    }
    else
    {
        m_subtitle = set_label_text(m_main, sc.subtitle, 0, 0, font_25, OSD_COLOR_WHITE);
    }

    lv_obj_null_on_delete(&m_subtitle);
    lv_obj_align(m_subtitle, LV_ALIGN_TOP_MID, 0, subtitle_y);
    // Rodapé
    DELETE_OBJ(m_footer);
    m_footer = MB_OSD_Footer::draw(m_main, sc.footer, footer_y);
    lv_obj_null_on_delete(&m_footer);
}

void OSD_Software_Update::create_more_informations_screen()
{
    m_bgd_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_main);
    //m_bgd_main = create_rect(m_main, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
    //lv_obj_null_on_delete(&m_bgd_main);
    auto title = set_label_text(m_bgd_main, tr(__Atualizacao_de_software), 0, 0, font_semi_40, OSD_COLOR_WHITE);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    auto subtitle = set_label_text(m_bgd_main, tr(__Para_mais_informacoes_de_como_atualizar), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 67);
    auto qrcode_site = create_qrcode(m_bgd_main, 96);
    lv_obj_align(qrcode_site, LV_ALIGN_TOP_MID, 0, 130);
    lv_qrcode_update(qrcode_site, MBGUI_CENTURY_HOMEPAGE, strlen(MBGUI_CENTURY_HOMEPAGE));
    auto lbl_site = set_label_text(m_bgd_main, "Site:", 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_align(lbl_site, LV_ALIGN_TOP_MID, 0, 235);
    auto img_site = load_image(m_bgd_main, LOGO_LINK_25x25, 0, 0, 25, 25);
    lv_obj_align(img_site, LV_ALIGN_TOP_MID, -160,  260);
    auto lbl_link_site = set_label_text(m_bgd_main, MBGUI_CENTURY_HOMEPAGE, 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_align(lbl_link_site, LV_ALIGN_TOP_MID, 0, 260);
    auto lbl_sac = set_label_text(m_bgd_main, "SAC:", 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_align(lbl_sac, LV_ALIGN_TOP_MID, 0, 300);
    auto img_whatsapp = load_image(m_bgd_main, LOGO_WHATSAPP_25x25, 0, 0, 27, 27);
    lv_obj_align(img_whatsapp, LV_ALIGN_TOP_MID, -90, 325);
    auto lbl_number = set_label_text(m_bgd_main, MBGUI_CENTURY_WHASTAPP_NUMBER, 0, 0, font_semi_20, OSD_COLOR_WHITE);
    lv_obj_align(lbl_number, LV_ALIGN_TOP_MID, 0, 325);
    auto button = create_rect(m_bgd_main, 0, 0, 220, 50, OSD_COLOR_ORANGE);
    lv_obj_set_style_radius(button, 25, DEFAULT_SELECTOR);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -100);
    auto lbl_ok = set_label_text(button, tr(__Voltar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(lbl_ok, LV_ALIGN_CENTER, 0, 0);
    MB_OSD_Footer::draw(m_bgd_main, tr(__Pressione_ok_para_continuar), footer_y);
    lv_obj_add_flag(m_bgd_main, LV_OBJ_FLAG_HIDDEN);
}

void OSD_Software_Update::draw_button()
{
    DELETE_OBJ(m_button_next);
    m_button_next = create_rect(m_main, next_x, next_y, next_w, next_h, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_button_next);
    lv_obj_set_style_radius(m_button_next, next_h / 2, DEFAULT_SELECTOR);
    m_label_next = set_label_text(m_button_next, tr(__Proximo), 0, 0, font_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_label_next);
    lv_obj_align(m_label_next, LV_ALIGN_CENTER, 0, 0);
}

void OSD_Software_Update::draw_buttons()
{
    m_options.add_label(tr(__Mais_Informacoes));
    m_options.add_label(tr(__Finalizar));
    m_options.set_background(m_main);
    m_options.set_horizontal();
    m_options.draw();
    m_options.select();
    m_options.next();
}

void OSD_Software_Update::stop_timer(bool _result)
{
    if (m_refresh_timer)
    {
        lv_timer_pause(m_refresh_timer);
        DELETE_TIMER(m_refresh_timer);
    }
    Task::post_event(std::bind(m_callback, _result));
}

void OSD_Software_Update::hide_menu()
{
    remove_focus();
    stop_timer(true);
}

} // namespace mb
