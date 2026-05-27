#include "mb_osd_menu_config.h"
#include "mb_osd.h"
#include "mb_osd_confirm_save.h"
#include "mb_osd_confirm_default.h"
#include "mb_menu_resources.h"
#include "mb_menu_data.h"
#include "common/mb_globals.h"
#include "common/mb_state_file.h"
#include "mb_osd_translate.h"
#include "mb_events.h"
#include "mb_zone_id.h"
#include "hal/mb_display.h"
#include "../../project_version.h"

#include <tasks/mb_task_application.h>
#include <tasks/mb_task_osd.h>
#include "tasks/mb_task.h"
#include "tasks/mb_task_database.h"
#include "mb_remote_control_handler.h"

#include "fw_env.h"

#include <lvgl.h>

#include <math.h>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "stdio.h"
#include <iomanip>
#include <unistd.h>
#include <cstdio>
#include <filesystem>

namespace mb {

OSD_Menu_Config::OSD_Menu_Config(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y),
    m_options(offset_x + width, offset_y, option_w, option_h, option_s, option_x, option_y)
{
    m_keys =
    {
        tr(__Idioma),
        tr(__Resolucao),
        tr(__Padrao_de_cor),
        tr(__Relacao_de_aspecto),
        tr(__Data_e_hora),
        tr(__Controle_parental),
        tr(__Padrao_de_fabrica),
    };

    State_File::App_State_File file;
    m_param_settings.aspect_mode = static_cast<Aspect_Mode>(file.aspect_mode);
    m_param_settings.color_standard = static_cast<Color_Standard>(file.color_standard);
    m_param_settings.resolution = static_cast<Resolution_Standard>(file.resolution);
    m_param_settings.clock_status = static_cast<Clock_Type>(file.clock_status);
    m_param_settings.time_zone = static_cast<Timezone_Mode>(file.time_zone);
    m_param_settings.language_mode = static_cast<Language_Mode>(file.language_mode);
}

OSD_Menu_Config::~OSD_Menu_Config()
{
    DELETE_OBJ(m_passwd);
    DELETE_OBJ(m_main_screen);
    DELETE_OBJ(m_footer);
    DELETE_OBJ(m_save_bckg);
    DELETE_OBJ(m_bgd_fade);
    DELETE_OBJ(m_center_line);
    DELETE_OBJ(m_bgd_options);
    DELETE_OBJ(m_main);
    remove_focus();
}

// Processa tecla recebida
bool OSD_Menu_Config::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
            if (m_column_active == Column_Active::First)
            {
                process_ok();
                m_column_active = Column_Active::Second;
            }
            else if (m_column_active == Column_Active::Second)
            {
                process_options();
            }

            break;

        case Remote_Control_Key::KEY_VOLTAR:
            if (m_column_active == Column_Active::First)
            {
                Task::s_task_osd->set_force_refresh(false);
                Task::post_event_system_display_settings_save(std::move(m_param_settings));
                Task::post_event(std::bind(m_callback, false));
            }
            else if (m_column_active == Column_Active::Second)
            {
                // Desabilita refresh automático de tela para redimensionar OSD após troca de resolução
                if (m_options.get_selected() == 1)
                {
                    Task::s_task_osd->set_force_refresh(false);
                }

                m_column_active = Column_Active::First;
                m_options.clear();
                lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
                DELETE_OBJ(m_bgd_fade);
                DELETE_OBJ(m_center_line);
                DELETE_OBJ(m_bgd_options);
            }

            break;

        case Remote_Control_Key::KEY_CHUP:
            if (m_column_active == Column_Active::First)
            {
                m_keys.previous();
                update_breadcrumb();
            }
            else if (m_column_active == Column_Active::Second)
            {
                m_options.previous();
            }

            break;

        case Remote_Control_Key::KEY_CHDOWN:
            if (m_column_active == Column_Active::First)
            {
                m_keys.next();
                update_breadcrumb();
            }
            else if (m_column_active == Column_Active::Second)
            {
                m_options.next();
            }

            break;

        default:
            break;
    }

    return true;
}

void OSD_Menu_Config::update_breadcrumb()
{
    auto function_active = static_cast<Func_Index>(m_keys.get_selected());
    switch (function_active)
    {
        case Func_Index::Language:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Idioma));
            break;
        }

        case Func_Index::Resolution:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Resolucao));
            break;
        }

        case Func_Index::Color:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Padrao_de_cor));
            break;
        }

        case Func_Index::Aspect:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Relacao_de_aspecto));
            break;
        }

        case Func_Index::Clock:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Data_e_hora));
            break;
        }

        case Func_Index::Parental_Control:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Controle_parental));
            break;
        }

        case Func_Index::Default:
        {
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Padrao_de_fabrica));
            break;
        }
    }
}

// Cria o menu de configuração
void OSD_Menu_Config::show_menu_config(Config_CB_t _callback, lv_obj_t *bgd)
{
    DEBUG_MSG(OSD, DEBUG, "show_menu_config()\n");
    set_focus();
    m_bgd = bgd;
    m_callback = _callback;
    // Cria área cobrindo toda a tela com opacidade parcial
    m_main = create_rect(m_bgd, 0, 0, width, height, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    Osd_Breadcrumb::s_instance.add_name(tr(__Idioma));
    // Desenha linhas verticais
    m_left_line = create_rect(m_main, 0, 0, 3, height, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_left_line);
    // Desenha teclado
    m_keys.set_background(m_main);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_bgd, tr(__Selecione_a_opcao_desejada_e_pressione_ok_ou_pressione_voltar), -40);
    lv_obj_null_on_delete(&m_footer);
    lv_obj_align(m_footer, LV_ALIGN_BOTTOM_LEFT, 10, -40);
}

void OSD_Menu_Config::process_ok()
{
    auto selected = m_keys.get_selected();

    switch (selected)
    {
        case 0 :
        {
            draw_language();
            break;
        }

        case 1 :
        {
            draw_resolution();
            break;
        }

        case 2 :
        {
            draw_color_pattern();
            break;
        }

        case 3 :
        {
            draw_aspect_mode();
            break;
        }

        case 4 :
        {
            draw_time_and_date();
            break;
        }

        case 5 :
        {
            draw_parental_control();
            break;
        }

        case 6 :
        {
            draw_factory_default();
            break;
        }
    }
}

void OSD_Menu_Config::process_options()
{
    auto selected = m_keys.get_selected();

    switch (selected)
    {
        case 0 :
        {
            process_language();
            break;
        }

        case 1 :
        {
            process_resolution();
            break;
        }

        case 2 :
        {
            process_color_pattern();
            break;
        }

        case 3 :
        {
            process_aspect_mode();
            break;
        }

        case 4 :
        {
            process_time_and_date();
            break;
        }

        case 5 :
        {
            process_factory_default();
            break;
        }
    }
}

void OSD_Menu_Config::process_factory_default()
{
}

void OSD_Menu_Config::factory_default_password_callback(bool _ok)
{
    // Draw confirmation UI
    if (_ok)
    {
        draw_save_background();
        // Texto com a pergunta
        std::string text;
        text += tr(__Todos_os_canais_e_configuracoes_serao_perdidos);
        text += "\n";
        text += tr(__Deseja_confirmar);
        m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
        m_confirm_save->show_confirm_save(std::bind(&OSD_Menu_Config::factory_default_confirm_callback, this, std::placeholders::_1), m_save_bckg, text.c_str(), false);
    }
    else
    {
        lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        DELETE_OBJ(m_bgd_fade);
        DELETE_OBJ(m_center_line);
        DELETE_OBJ(m_bgd_options);
    }

    m_column_active = Column_Active::First;
    m_osd_password.reset();
    DELETE_OBJ(m_passwd);
}

void OSD_Menu_Config::factory_default_confirm_callback(bool _ok)
{
    if (_ok)
    {
        // Apaga arquivo com termos de uso
        {
            namespace fs = std::filesystem;
            fs::remove(MBGUI_TERMS_CONDITIONS_DATE_FILE);
        }
        Task::post_event_system_factory_reset();
    }

    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    DELETE_OBJ(m_save_bckg);
    DELETE_OBJ(m_bgd_fade);
    DELETE_OBJ(m_center_line);
    DELETE_OBJ(m_bgd_options);
    m_column_active = Column_Active::First;
    m_confirm_save.reset();

    if (_ok)
    {
        Task::post_event(std::bind(m_callback, true));
    }
}

void OSD_Menu_Config::draw_factory_default()
{
    create_fade();
    create_bgd_options(true);
    set_label_text_static(m_bgd_options, tr(__Padrao_de_fabrica), 20, 0, font_semi_30, OSD_COLOR_WHITE);
    m_passwd = create_rect(m_bgd_options, 20, 173 - offset_y, 380, 130, OSD_COLOR_BLACK);

    if (!m_osd_password)
    {
        m_osd_password = std::make_unique<OSD_Password>(this);
    }

    lv_obj_null_on_delete(&m_passwd);
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Config::factory_default_password_callback, this, _1);
    m_osd_password->show_password(callback, m_passwd, "0000");
}

void OSD_Menu_Config::process_time_and_date()
{
    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    auto selected = m_options.get_selected();

    Clock_Type new_clock;

    if (network_id == Network_Id_Sky)
    {
        // SKY só tem Auto e Timezone
        new_clock = (selected == 0) ? Clock_Type::Auto
                                   : Clock_Type::Timezone;
    }
    else
    {
        switch (selected)
        {
            case 0: new_clock = Clock_Type::Auto; break;
            case 1: new_clock = Clock_Type::Manual; break;
            default: new_clock = Clock_Type::Timezone; break;
        }
    }

    m_param_settings.clock_status = new_clock;
    m_options.set_previously_selected(selected);

    // Ações específicas
    if (new_clock == Clock_Type::Manual)
    {
        create_clock_area();
    }
    else if (new_clock == Clock_Type::Timezone)
    {
        create_timezone_area();
    }
}

void OSD_Menu_Config::create_config_area()
{
    auto x1 = 1000;
    auto x2 = x1 + 190 + 23 + 3;

    if (!m_config_area)
    {
        m_config_area = create_rect(m_bgd, 586, 0, x2 - x1, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_config_area);
        lv_obj_set_style_bg_opa(m_config_area, LV_OPA_50, 0);
        m_right_line = create_rect(m_config_area, 0, 0, 3, height, OSD_COLOR_ORANGE);
        lv_obj_null_on_delete(&m_right_line);
        lv_obj_set_style_bg_color(m_center_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
        m_config_fade = create_rect(m_bgd, 306, 0, 280, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_config_fade);
        lv_obj_set_style_bg_opa(m_config_fade, LV_OPA_20, 0);
    }
}

// Exibe área para ajuste manual do relógio
void OSD_Menu_Config::create_clock_area()
{
    // Cria transparencia sobre o menu e background para próxima tela
    create_config_area();
    // Texto com a pergunta
    m_set_clock = std::make_unique<OSD_Set_Clock>(this);
    m_set_clock->show_set_clock([ =, this]()
    {
        // Apaga tela de ajuste de relógio
        lv_obj_set_style_bg_color(m_center_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        DELETE_OBJ(m_config_area);
        DELETE_OBJ(m_config_fade);
        DELETE_OBJ(m_right_line);
        m_set_clock.reset();
    }, m_config_area);
}

// Exibe área para ajuste manual do relógio
void OSD_Menu_Config::create_timezone_area()
{
    // Cria transparencia sobre o menu e background para próxima tela
    create_config_area();
    // Texto com a pergunta
    m_set_timezone = std::make_unique<OSD_Set_Timezone>(this);
    m_set_timezone->show_set_timezone([ =, this](Timezone_Mode _timezone_mode)
    {
        // Apaga tela de ajuste de relógio
        lv_obj_set_style_bg_color(m_center_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        DELETE_OBJ(m_config_area);
        DELETE_OBJ(m_config_fade);
        DELETE_OBJ(m_right_line);
        m_param_settings.time_zone = _timezone_mode;
        m_set_timezone.reset();
    }, m_config_area, m_param_settings.time_zone);
}

void OSD_Menu_Config::draw_time_and_date()
{
    auto network_id = Config::get_config()->selected_satellite_config().network_id;
    // Desenha tela de seleção de fuso horário
    m_option_names.clear();
    m_option_names.emplace_back(tr(__Automatico));
    if(network_id != Network_Id_Sky)
    {
        m_option_names.emplace_back(tr(__Manual));
    }

    m_option_names.emplace_back(tr(__Fuso_horario));
    draw_options();
    // Seta modo atual do relógio
    auto selected = m_param_settings.clock_status;
    m_options.set_previously_selected(static_cast<int>(selected));
}

void OSD_Menu_Config::draw_aspect_mode()
{

    m_option_names.clear();
    m_option_names.emplace_back(tr(__Automatico));
    m_option_names.emplace_back("16x9 Pillar Box");
    m_option_names.emplace_back("16x9 Pan&Scan");
    m_option_names.emplace_back("16x9 Letter Box");
    m_option_names.emplace_back(tr(__16x9_Tela_cheia));
    m_option_names.emplace_back("4x3 Pan&Scan");
    m_option_names.emplace_back("4x3 Letter Box");
    m_option_names.emplace_back(tr(__4x3_Tela_cheia));
    draw_options();
    // Seta relação de aspecto atual
    auto current_aspect_mode = static_cast<int>(m_param_settings.aspect_mode);
    m_options.set_previously_selected(current_aspect_mode);
}

void OSD_Menu_Config::process_aspect_mode()
{
    Aspect_Mode current_aspect_mode = m_param_settings.aspect_mode;
    auto selected = m_options.get_selected();
    auto aspect_mode = static_cast<Aspect_Mode>(selected);
    // Imprime relação de aspecto atual
    draw_save_background();
    //Display::get_instance()->set_aspect_mode(aspect_mode);
    Display::get_instance()->set_all_display_settings(m_param_settings.resolution, aspect_mode, m_param_settings.color_standard);
    // Texto com a pergunta
    std::string text;
    text = tr(__Relacao_de_aspecto_selecionada);
    //text += to_str(current_aspect_mode);

    switch (aspect_mode)
    {
        case Aspect_Mode::AUTO:
            text += tr(__Automatico);
            break;

        case Aspect_Mode::PILLBOX_16x9:
            text += "16x9 Pillar Box";
            break;

        case Aspect_Mode::PANSCAN_16x9:
            text += "16x9 Pan&Scan";
            break;

        case Aspect_Mode::LETTERBOX_16x9:
            text += "16x9 Letter Box";
            break;

        case Aspect_Mode::FULLSCREEN_16x9:
            text += std::string(tr(__16x9_Tela_cheia));
            break;

        case Aspect_Mode::PANSCAN_4x3:
            text += "4x3 Pan&Scan";
            break;

        case Aspect_Mode::LETTERBOX_4x3:
            text += "4x3 Letter Box";
            break;

        case Aspect_Mode::FULLSCREEN_4x3:
            text += std::string(tr(__4x3_Tela_cheia));
            break;
    }

    text += "\n";
    text += tr(__Deseja_confirmar);
    m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
    m_confirm_save->show_confirm_save([ =, this](bool _save_ok)
    {
        DELETE_OBJ(m_save_bckg);

        if (_save_ok)
        {
            m_options.set_previously_selected(selected);
            m_param_settings.aspect_mode = aspect_mode;
        }
        else
        {
            //Display::get_instance()->set_aspect_mode(current_aspect_mode);
            Display::get_instance()->set_all_display_settings(m_param_settings.resolution, current_aspect_mode, m_param_settings.color_standard);
        }

        m_confirm_save.reset();
    }, m_save_bckg, text, true);
}

/**
 * @brief Desenha padrão de cor
 *
 * @return void
 *
 * @note Aplica padrão de cor selecionado
 */
void OSD_Menu_Config::process_color_pattern()
{
    // Imprime padrão de cor atual
    //Color_Standard color = Display::get_instance()->get_color_standard();
    Color_Standard color = m_param_settings.color_standard;
    auto selected = m_options.get_selected();
    auto new_color = selected == 0 ? Color_Standard::PAL_M_60 : Color_Standard::NTSC_60;
    draw_save_background();
    //Seta o novo padrao de cor
    //Display::get_instance()->set_color_standard(new_color);
    Display::get_instance()->set_all_display_settings(m_param_settings.resolution, m_param_settings.aspect_mode, new_color);
    std::string text;
    text += tr(__Padrao_de_cor_alterado_para);
    text += new_color == Color_Standard::PAL_M_60 ? "PAL-M" : "NTSC";
    text += "\n";
    text += tr(__Deseja_confirmar);
    m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
    m_confirm_save->show_confirm_save([ =, this](bool _save_ok)
    {
        DELETE_OBJ(m_save_bckg);

        if (_save_ok)
        {
            auto select = m_options.get_selected();
            m_options.set_previously_selected(select);
            auto c = select == 0 ? Color_Standard::PAL_M_60 : Color_Standard::NTSC_60;
            m_param_settings.color_standard = c;
            DEBUG_MSG(OSD, DEBUG, "Padrão aplicado: " << (int)new_color << "\n");
        }
        else
        {
            //Display::get_instance()->set_color_standard(color);
            Display::get_instance()->set_all_display_settings(m_param_settings.resolution, m_param_settings.aspect_mode, color);
        }

        m_confirm_save.reset();
    }, m_save_bckg, text, true);
}

/**
 * @brief Desenha tela de seleção de padrão de cor
 *
 * @return void
 *
 * @note Desenha tela de seleção de padrão de cor e indica o padrão atual
 */
void OSD_Menu_Config::draw_color_pattern()
{
    m_option_names.clear();
    m_option_names.push_back("PAL-M");
    m_option_names.push_back("NTSC");
    draw_options();
    Color_Standard color = m_param_settings.color_standard;
    DEBUG_MSG(OSD, DEBUG, "Padrão de cor atual = " << (color == Color_Standard::PAL_M_60 ? "PAL-M" : "NTSC") << "\n");
    auto selected = color == Color_Standard::PAL_M_60 ? 0 : 1;
    m_options.set_previously_selected(selected);
}

/**
 * @brief Processa a seleção de idioma
 *
 * @return void
 *
 * @note Seta o idioma selecionado e simula evento de controle remoto para atualizar idioma
 */
void OSD_Menu_Config::process_language()
{
    auto lang = m_options.get_selected();
    OSD_Translate::set_language(static_cast<OSD_Translate::Lang_Index>(lang));
    m_param_settings.language_mode = static_cast<Language_Mode>(lang);
    DEBUG_MSG(OSD, DEBUG, "Idioma atual = " << (int)lang << "\n");
    // Simula evento de controle remoto para atualizar idioma
    std::vector<Event_Remote_Control> events =
    {
        Event_Remote_Control{Remote_Control_Key::KEY_VOLTAR},
        Event_Remote_Control{Remote_Control_Key::KEY_VOLTAR},
    };

    for (auto event : events)
    {
        Remote_Control_Handler::post_event_remote_control(event);
    }
}

/**
 * @brief Desenha tela de seleção de idioma
 *
 * @return void
 *
 * @note Desenha tela de seleção de idioma e seta o idioma atual
 */
void OSD_Menu_Config::draw_language()
{
    m_option_names.clear();
    m_option_names.push_back("Português");
    m_option_names.push_back("English");
    draw_options();

    // Caso o idioma não seja o português, escolher o próximo
    if (m_param_settings.language_mode == Language_Mode::Ingles)
    {
        m_options.set_previously_selected(1);
        m_options.next();
    }
    else
    {
        m_options.set_previously_selected(0);
    }
}

/**
 * @brief Aplica a resolução selecionada pelo usuário
 *
 * @return void
 *
 * @note Seta a resolução selecionada e cria tela de confirmação de alteração de resolução
 */
void OSD_Menu_Config::process_resolution()
{
    Resolution_Standard current_resolution = m_param_settings.resolution;
    Resolution_Standard selected_resolution = Resolution_Standard::_1080i_30Hz;
    std::string selected_resolution_str = "1080i";
    auto option = m_options.get_selected();

    switch (option)
    {
        case 0:
            selected_resolution = Resolution_Standard::_480i_60Hz;
            selected_resolution_str = "480i";
            break;

        case 1:
            selected_resolution = Resolution_Standard::_480p_60Hz;
            selected_resolution_str = "480p";
            break;

        case 2:
            selected_resolution = Resolution_Standard::_720p_60Hz;
            selected_resolution_str = "720p";
            break;

        case 3:
            selected_resolution = Resolution_Standard::_1080i_30Hz;
            selected_resolution_str = "1080i";
            break;

        case 4:
            selected_resolution = Resolution_Standard::_1080p_60Hz;
            selected_resolution_str = "1080p (60Hz)";
            break;

        default:
            break;
    }

    // Desenha tela de confirmação de alteração de resolução
    draw_save_background();
    //Seta a nova resolução
    //Display::get_instance()->set_resolution_standard(selected_resolution);
    Display::get_instance()->set_all_display_settings(selected_resolution, m_param_settings.aspect_mode, m_param_settings.color_standard);
    // Texto com a pergunta
    std::string text;
    text += tr(__Resolucao_alterada_para);
    text += selected_resolution_str;
    text += "\n";
    text += tr(__Deseja_confirmar);
    m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
    m_confirm_save->show_confirm_save([ =, this](bool _save_ok)
    {
        DELETE_OBJ(m_save_bckg);

        if (_save_ok)
        {
            m_options.set_previously_selected(option);
            m_param_settings.resolution = selected_resolution;
        }
        else
        {
            //Display::get_instance()->set_resolution_standard(current_resolution);
            Display::get_instance()->set_all_display_settings(current_resolution, m_param_settings.aspect_mode, m_param_settings.color_standard);
        }

        m_confirm_save.reset();
    }, m_save_bckg, text.c_str(), true);
}

/**
 * @brief Desenha tela de seleção de resolução
 *
 * @return void
 *
 * @note Desenha tela de seleção de resolução e seta a resolução atual
 */
void OSD_Menu_Config::draw_resolution()
{
    m_option_names.clear();
    m_option_names.emplace_back("480i");
    m_option_names.emplace_back("480p");
    m_option_names.emplace_back("720p");
    m_option_names.emplace_back("1080i");
    m_option_names.emplace_back("1080p (60Hz)");
    draw_options();
    // Habilita refresh automático de tela para redimensionar OSD após troca de resolução
    Task::s_task_osd->set_force_refresh(true);
    auto resolution = m_param_settings.resolution;

    switch (resolution)
    {
        case Resolution_Standard::_480i_60Hz:
            m_options.set_previously_selected(0);
            break;

        case Resolution_Standard::_480p_60Hz:
            m_options.set_previously_selected(1);
            break;

        case Resolution_Standard::_720p_60Hz:
            m_options.set_previously_selected(2);
            break;

        case Resolution_Standard::_1080i_30Hz:
            m_options.set_previously_selected(3);
            break;

        case Resolution_Standard::_1080p_60Hz:
            m_options.set_previously_selected(4);
            break;

        default:
            break;
    }
}

void OSD_Menu_Config::create_fade()
{
    // Coloca fade sobre o menu
    if (!m_bgd_fade)
    {
        m_bgd_fade = create_rect(m_main, 0, 0, width, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd_fade);
        lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
        lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    }
}

void OSD_Menu_Config::create_bgd_options(bool _large)
{
    if (_large)
    {
        m_bgd_options = create_rect(m_bgd, 306, 0, 520, height, OSD_COLOR_BLACK);
    }
    else
    {
        m_bgd_options = create_rect(m_bgd, 306, 0, 280, height, OSD_COLOR_BLACK);
    }

    lv_obj_null_on_delete(&m_bgd_options);
    m_center_line = create_rect(m_bgd_options, 0, 0, 3, height, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_center_line);
    lv_obj_set_style_bg_opa(m_bgd_options, LV_OPA_20, 0);
}

void OSD_Menu_Config::draw_options()
{
    create_fade();
    create_bgd_options(false);
    // Desenha teclado
    m_options.clear();

    for (const auto& name : m_option_names)
    {
        m_options.add_label(name);
    }

    m_options.set_background(m_bgd_options);
    m_options.set_vertical();
    m_options.draw();
    m_options.select();
}

// Busca o texto referente a data e hora setadas no relógio do sistema
void OSD_Menu_Config::get_time_date(char *buffer, int len)
{
    auto t = time(NULL);
    auto tm = localtime(&t);
    snprintf(buffer, len, "%02d/%02d/%04d %02d:%02d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 19 * 100, tm->tm_hour, tm->tm_min);
    DEBUG_MSG(OSD, DEBUG, buffer << "\n");
}

// Exibe área para ajuste manual de zip code
void OSD_Menu_Config::create_zip_area()
{
    // Funciona para ajuste de clock em modo manual
    if (m_zip_ta)
    {
        DEBUG_MSG(OSD, DEBUG, "Área para digitação de zip code criada anteriormente\n");
        return;
    }

    // Cria área de digitação de data/hora
    m_zip_ta = lv_textarea_create(m_main_screen);
    lv_obj_null_on_delete(&m_zip_ta);
    lv_textarea_set_one_line(m_zip_ta, true);
    lv_obj_set_pos(m_zip_ta, x_pos + text_width + 10, y_zip);
    lv_obj_set_style_text_font(m_zip_ta, font_25, 0);
    lv_textarea_add_text(m_zip_ta, "");
    lv_textarea_set_accepted_chars(m_zip_ta, "0123456789");
    lv_textarea_set_max_length(m_zip_ta, 3);
    DEBUG_MSG(OSD, DEBUG, "Criada área para digitação de zip code\n");
    // Busca conteúdo da caixa de texto, se estiver vazia, pega hora do sistema
    auto config = Config::get_config();
    Satellite_Operator _oper = config->selected_satellite_config().network_id == Network_Id_Sky ? Satellite_Operator::Sky : Satellite_Operator::Claro;
    m_local_zone_id = Zone_ID::get_zone_id(_oper);
    std::string txt = std::to_string(m_local_zone_id);
    lv_textarea_set_text(m_zip_ta, txt.c_str());
    DEBUG_MSG(OSD, DEBUG, "Zone id inicial: " << txt << "\n");
}

// Executa processamento das teclas enquanto estiver selecionado __zip
void OSD_Menu_Config::process_zip(Remote_Control_Key _key)
{
    // Verifica se foi digitado algum número
    char res;

    if ((res = get_digit(_key)) != '\n')
    {
        res += '0';
        lv_textarea_add_char(m_zip_ta, res);
        const char *txt;
        txt = lv_textarea_get_text(m_zip_ta);
        m_local_zone_id = std::stoi(txt);
        //ZoneID::s_instance.set_zoneid(zoneid);
        //zoneid = ZoneID::s_instance.get_zoneid();
        std::string text = std::to_string(m_local_zone_id);
        lv_textarea_set_text(m_zip_ta, text.c_str());
        return;
    }

    // Tecla apagar caracter
    if (_key == Remote_Control_Key::KEY_VOLDOWN)
    {
        lv_textarea_delete_char(m_zip_ta);
        return;
    }
}

void OSD_Menu_Config::factory_default_callback()
{
    m_confirm_default.reset();
}

void OSD_Menu_Config::process_factory_default(Remote_Control_Key _key)
{
    // Processa apenas a tecla OK
    DEBUG_MSG(OSD, DEBUG, "process_factory_default()\n");

    if (Remote_Control_Key::KEY_OK != _key)
    {
        return;
    }

    // Confirma se nova resolução deve ser aplicada ou não
    m_confirm_default = std::make_unique<OSD_Confirm_Default>(this);
    m_confirm_default->show_confim_default(std::bind(&OSD_Menu_Config::factory_default_callback, this), m_main_screen);
}

void  OSD_Menu_Config::draw_save_background()
{
    m_save_bckg = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), save_x1, save_y1, save_x2 - save_x1, save_y2, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_save_bckg);
}

void OSD_Menu_Config::draw_parental_control()
{
    DEBUG_MSG(OSD, DEBUG, "draw_parental_control()\n");
    create_fade();
    create_bgd_options(true);
    m_parental_title = set_label_text_static(m_bgd_options, tr(__Controle_parental), 20, 0, font_semi_30, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_parental_title);
    m_passwd = create_rect(m_bgd_options, 20, 173 - offset_y, 380, 130, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_passwd);

    if (!m_osd_password)
    {
        m_osd_password = std::make_unique<OSD_Password>(this);
    }

    State_File::App_State_File file;
    char parental_password[5] = "";
    memcpy(parental_password, file.parental_password, sizeof(parental_password));
    DEBUG_MSG(OSD, DEBUG, "Parental password: " << parental_password << " - " << file.parental_password << "\n");
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Config::parental_control_password_callback, this, _1);
    m_osd_password->show_password(callback, m_passwd, parental_password);
}

void OSD_Menu_Config::parental_control_password_callback(bool _ok)
{
    DEBUG_MSG(OSD, DEBUG, "parental_control_password_callback()\n");
    m_osd_password.reset();
    DELETE_OBJ(m_passwd);

    if (not _ok)
    {
        DEBUG_MSG(OSD, DEBUG, "Parental control password callback CANCEL\n");
        lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        DELETE_OBJ(m_bgd_fade);
        DELETE_OBJ(m_center_line);
        DELETE_OBJ(m_bgd_options);
        m_column_active = Column_Active::First;
        return;
    }

    // Draw background for save confirmation
    draw_save_background();
    m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
    m_confirm_save->show_confirm_save(std::bind(&OSD_Menu_Config::process_parental_control, this, std::placeholders::_1), m_save_bckg, tr(__Deseja_alterar_a_senha_parental), false);
}

void OSD_Menu_Config::process_parental_control(bool _ok)
{
    DEBUG_MSG(OSD, DEBUG, "process_parental_control()\n");
    m_confirm_save.reset();
    DELETE_OBJ(m_save_bckg);

    if (_ok)
    {
        lv_label_set_text(m_parental_title, tr(__Crie_sua_senha).data());
        DELETE_OBJ(m_passwd);
        m_passwd = create_rect(m_bgd_options, 20, 173 - offset_y, 520, height, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_passwd);

        if (!m_osd_change_password)
        {
            m_osd_change_password = std::make_unique<OSD_Change_Password>(this);
        }
        m_osd_change_password->change_parental_password(std::bind(&OSD_Menu_Config::process_parental_control_callback, this, std::placeholders::_1), m_passwd);
    }
    else
    {
        config_parental_control();
    }
}

void OSD_Menu_Config::process_parental_control_callback(bool _ok)
{
    DEBUG_MSG(OSD, DEBUG, "process_parental_control_callback()\n");
    // Draw confirmation UI
    DELETE_OBJ(m_passwd);
    DELETE_OBJ(m_save_bckg);
    m_osd_change_password.reset();
    m_osd_password.reset();

    if (_ok)
    {
        DEBUG_MSG(OSD, DEBUG, "Parental control password callback OK\n");
        config_parental_control();
    }
    else
    {
        lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        DELETE_OBJ(m_save_bckg);
        DELETE_OBJ(m_bgd_fade);
        DELETE_OBJ(m_center_line);
        DELETE_OBJ(m_bgd_options);
        m_column_active = Column_Active::First;
    }
}

void OSD_Menu_Config::config_parental_control()
{
    DEBUG_MSG(OSD, DEBUG, "config_parental_control()\n");
    DELETE_OBJ(m_parental_title);

    if (!m_osd_parental_control)
    {
        m_osd_parental_control = std::make_unique<OSD_Parental_Control>();
    }

    m_osd_parental_control->config_parental_control(std::bind(&OSD_Menu_Config::config_parental_control_callback, this, std::placeholders::_1), m_bgd_options);
}

void OSD_Menu_Config::config_parental_control_callback(bool _ok)
{
    DEBUG_MSG(OSD, DEBUG, "config_parental_control_callback()\n");

    // Draw confirmation UI
    if (_ok)
    {
        DEBUG_MSG(OSD, DEBUG, "Parental control config callback OK\n");
    }
    else
    {
        DEBUG_MSG(OSD, DEBUG, "Parental control config callback CANCEL\n");
    }

    lv_obj_set_style_bg_color(m_left_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    DELETE_OBJ(m_save_bckg);
    DELETE_OBJ(m_bgd_fade);
    DELETE_OBJ(m_center_line);
    DELETE_OBJ(m_bgd_options);
    m_column_active = Column_Active::First;
    m_osd_parental_control.reset();
    
}

} // namespace mb
