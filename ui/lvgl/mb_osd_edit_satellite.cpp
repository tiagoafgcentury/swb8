// Description: Tela de edição de satélite
#include "mb_osd_edit_satellite.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "common/mb_types.h"
#include "common/mb_state_file.h"

#include "tasks/mb_task_eit_events.h"
#include "mb_events.h"


#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <src/misc/lv_color.h>
#include <src/misc/lv_types.h>
#include <src/misc/lv_color.h>
#include <time.h>

namespace mb {

OSD_Edit_Satellite *OSD_Edit_Satellite::s_instance { nullptr };

// Inicializa objetos no construtor
OSD_Edit_Satellite::OSD_Edit_Satellite(OSD *_parent):
    OSD(_parent),
    m_keys_main(0, 0, button_width, button_heigth, button_spacing, button_x, button_y),
    m_keys_options(cover_w, offset, btn_w, btn_h, btn_spacing, btn_x, btn_y),
    m_key_save(0, 0, btn_save_width, btn_save_heigth, 0, btn_save_x, btn_save_y),
    m_key_voltar(0, 0, btn_w, btn_h, 0, btn_x, btn_y),
    m_key_diseq_c(0, 0, btn_diseq_c_w, btn_diseq_c_h, btn_diseq_c_s, btn_diseq_c_x, btn_diseq_c_y)
{    
    s_instance = this;
}

OSD_Edit_Satellite::~OSD_Edit_Satellite()
{
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_bgd_sat);
    DELETE_OBJ(m_mainscreen);

    remove_focus();
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name(false);
    Osd_Breadcrumb::s_instance.remove_name(true);
    discard_changes();
}

bool OSD_Edit_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            if ( m_bgd_sat != nullptr )
            {
                if ( static_cast<Func_Active>(m_keys_main.get_selected()) == Func_Active::Switch )
                {
                    process_diseq_c(_event);
                }
                else
                {
                    reset_sat_options();
                }
            }
            else
            {
                Task::post_event(std::bind(m_callback, false));
            }
            break;
        }

        case Remote_Control_Key::KEY_CHUP:
        case Remote_Control_Key::KEY_CHDOWN:
        {
            if ( m_bgd_sat != nullptr and static_cast<Func_Active>(m_keys_main.get_selected()) == Func_Active::Switch )
            {
                process_diseq_c(_event);
            }
            else if ( m_bgd_sat == nullptr )
            {

                // Se nenhuma opção estiver habilitada, ignora
                if ( m_keys_main.get_enabled_count() == 0 )
                {
                    break;
                }

                reset_sat_options();
                if ( m_save_active )
                {
                    m_save_active = false;
                    m_key_save.unselect();
                    m_keys_main.go_to_first_enabled();
                }
                else if ( m_keys_main.is_last_enabled() and _event.key == Remote_Control_Key::KEY_CHDOWN )
                {
                    m_save_active = true;
                    m_keys_main.unselect();
                    m_key_save.select();
                }
                else if ( m_keys_main.is_first_enabled() and _event.key == Remote_Control_Key::KEY_CHUP )
                {
                    m_save_active = true;
                    m_keys_main.unselect();
                    m_key_save.select();
                }
                else
                {
                    if (_event.key == Remote_Control_Key::KEY_CHUP)
                        m_keys_main.previous();
                    else
                        m_keys_main.next();
                }
            }
            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            process_ok_button();
            break;
        }
       
        case Remote_Control_Key::KEY_VOLUP:
        case Remote_Control_Key::KEY_VOLDOWN:
        {
            if ( m_bgd_sat != nullptr and static_cast<Func_Active>(m_keys_main.get_selected()) == Func_Active::Switch )
            {
                process_diseq_c(_event);
            }
            else
            {
                process_changes(_event);
            }
            break;
        }
            
        default:
        {
            DEBUG("Unhandled key: " << std::hex << static_cast<uint32_t>(_event.key) << "\n");
            break;
        }
    }
    return true;
}

void OSD_Edit_Satellite::edit_satellite(osd_edit_satellite_callback_t _callback, Satellite _sat)
{
    m_callback = _callback;
    set_focus();
    // Carrega satélite recebido como argumento em variável da classe
    m_current_satellite = _sat;
    m_previous_satellite = _sat;
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset, width, heigth - offset, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_mainscreen);
    draw_titles();
    draw_buttons();
    // Desenha botão salvar
    m_key_save.set_background(m_mainscreen);
    m_key_save.add_label(tr(__Salvar));
    m_key_save.draw();
    // Desenha uma área translúcida em cima dos menus atuais
    m_cover_area = create_rect(m_mainscreen, cover_x, cover_y, cover_w, cover_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_cover_area);
    lv_obj_set_style_bg_opa(m_cover_area, LV_OPA_50, 0);
    lv_obj_add_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
    Osd_Breadcrumb::s_instance.add_name({m_current_satellite.name, tr(__Editar)});

    // Carrega lista de canais do satélite para verificar opções de DiseqC
    auto callback = [this](const std::vector<Satellite> &sats)
    {
        if(sats.size() == 0) { return; }
        m_satellites = sats;
    };    
    Task::post_event_satellite_list_load(std::bind(callback, std::placeholders::_1));
}

void OSD_Edit_Satellite::lnbf_detection()
{
    // Inicia detecção de LNBF
    auto title = set_label_text(m_bgd_sat, tr(__Detectando_tipo_de_LNBF_da_sua_antena), 20, 20, font_semi_25, OSD_COLOR_WHITE);
    auto subtitle = set_label_text(m_bgd_sat, tr(__Por_favor_aguarde), 20, 40, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 60);

    // Detecta LNBF
    m_detect_lnbf = std::make_unique<MB_Detect_Lnbf>(this);
    m_detect_lnbf->lnbf_detection_start([this](bool _result, Transponder_Id _tp)
    {
        DELETE_TIMER(m_refresh_timer);
        m_progress = 100 ;
        if (_result)
        {
            auto config = Config::get_config();
            auto band = OSD_Translate::translate(config->band());
            auto type = OSD_Translate::translate(config->lnbf_type());
            // Busca dados do serviço atual
            DEBUG_MSG(OSD, INFO, band << " - " << type << " - " << tr(__Normal) << "\n");
            lnbf_detection_success(_tp);
        }
        else
        {
            DEBUG_MSG(OSD, WARN, "LNBF detection failed\n");
            lnbf_detection_failed();
        }
        m_detect_lnbf.reset();
    }, m_current_satellite);

    // Cria barra de progresso e inicia timer de atualização
    create_progress_bar();
    m_refresh_timer = lv_timer_create(update_progress_bar_cb, 200, this);
    lv_timer_set_repeat_count(m_refresh_timer, -1);
    m_progress = 0;
}

void OSD_Edit_Satellite::lnbf_detection_failed()
{
    // Atualiza barra de progresso
    lv_label_set_text(m_slider_label, tr(__Nao_foi_possivel_detectar_o_lnbf).data());
    lv_slider_set_value(m_slider, m_progress, LV_ANIM_ON);
    // Set the indicator color to red
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_RED, LV_PART_INDICATOR);
    // Desenha tecla voltar
    draw_voltar_button();
}

void OSD_Edit_Satellite::lnbf_detection_success(Transponder_Id _tp)
{
    // Tipo de lnbf detectado
    auto config = Config::get_config();
    auto lnbf_type = config->lnbf_type();
    std::string lnbf_type_str = std::string(tr(__LNBF_detectado_com_sucesso)) + std::string(": ") + std::string(OSD_Translate::translate(lnbf_type));
    DEBUG_MSG(OSD, INFO, lnbf_type_str << "\n");

    // Atualiza barra de progresso
    lv_label_set_text(m_slider_label, lnbf_type_str.data());
    lv_slider_set_value(m_slider, m_progress, LV_ANIM_ON);
    // Desenha tecla voltar
    draw_voltar_button();
    // Verifica se a frequencia do transponder é da Claro ou Sky
    if(_tp.frequency() != 12120000 ) 
    {
        if (m_bgd_sat)
        {
            auto message = set_label_text(m_bgd_sat, tr(__Erro_Satelite_detectado_nao_e_o_selecionado ), 0, 0, font_semi_25, OSD_COLOR_RED);
            lv_obj_align(message, LV_ALIGN_CENTER, 0, 50);
        }
    }
    else 
    {
        // Atualiza dados do satélite
        size_t func_idx = static_cast<size_t>(Func_Active::LNBF_Type);
        m_current_satellite.type = lnbf_type;
        m_keys_main.set_label(func_idx, OSD_Translate::translate(m_current_satellite.type).data());
    }
}

void OSD_Edit_Satellite::draw_voltar_button()
{
    auto x = (width - cover_w - 130 - btn_w ) / 2 ;
    auto y = cover_h - btn_h ;
    m_key_voltar.clear();
    m_key_voltar.set_background(m_bgd_sat);
    m_key_voltar.set_y(y);
    m_key_voltar.set_x(x);
    m_key_voltar.add_label(tr(__Voltar));
    m_key_voltar.draw();
    m_key_voltar.select();
}

void OSD_Edit_Satellite::process_ok_button()
{
    DEBUG("OK pressed\n");
    // Verifica se existe algum botão ativo
    if ( m_keys_main.get_enabled_count() == 0 )
    {
        DEBUG("No active button\n");
        return;
    }

    if ( m_save_active )
    {
        DEBUG("Save active\n");
        save_changes();
        return;
    }

    // Processa botão ativo
    Func_Active func = static_cast<Func_Active>(m_keys_main.get_selected());
    if ( !m_bgd_sat )
    {
        switch (func)
        {
            case Func_Active::Name:
            {
                DEBUG("Name active\n");
                edit_name();
                break;
            }

            case Func_Active::LNBF_Detection:
            case Func_Active::LNBF_Type:
            case Func_Active::Band:
            case Func_Active::Polarity:
            case Func_Active::Switch:
            {
                DEBUG("Options active\n");
                draw_sat_options();
                break;
            }

            default:
            {
                DEBUG("Function not implemented yet\n");
                break;
            }    
        }
    }
    // Processa seleção dentro das opções
    else if ( m_bgd_sat )
    {
        DEBUG("Options active\n");
        if ( func == Func_Active::Switch )
        {
            process_diseq_c(Event_Remote_Control{Remote_Control_Key::KEY_OK});
            return;
        }

        switch (func)
        {
            case Func_Active::Band:
            {
                auto band = m_keys_options.get_selected() == 0 ? Band::C : Band::Ku;
                m_current_satellite.band = band;
                int idx = m_keys_main.get_selected();
                m_keys_main.set_label(idx, OSD_Translate::translate(m_current_satellite.band));
                reset_sat_options();

                if ( band == Band::C and m_current_satellite.type == LNBF_Type::Universal )
                {
                    // Se mudar para banda C, força tipo de LNBF Monoponto
                    m_current_satellite.type = LNBF_Type::Mono;
                    int idx = m_keys_main.get_selected();
                    m_keys_main.set_label(idx + 1, OSD_Translate::translate(m_current_satellite.type).data());
                }
                else if ( band == Band::Ku )
                {
                    if ( m_current_satellite.type == LNBF_Type::Mono )
                    {
                        // Se mudar para banda Ku, e tipo Monoponto, altera para Universal
                        m_current_satellite.type = LNBF_Type::Universal;
                        int idx = m_keys_main.get_selected();
                        m_keys_main.set_label(idx + 1, OSD_Translate::translate(m_current_satellite.type).data());
                    }

                    if ( m_current_satellite.position == LNBF_Position::Inverted )
                    {
                        // Se mudar para banda Ku, e polaridade Invertida, altera para Normal
                        m_current_satellite.position = LNBF_Position::Normal;
                        int idx = m_keys_main.get_selected();
                        m_keys_main.set_label(idx + 2, OSD_Translate::translate(m_current_satellite.position).data());
                    }
                }
                break;
            }

            case Func_Active::LNBF_Type:
            {
                auto lnbf_type = LNBF_Type::Universal;
                if (m_current_satellite.band == Band::Ku)
                {
                    lnbf_type = m_keys_options.get_selected() == 0 ? LNBF_Type::Universal : LNBF_Type::Multi;
                }
                else
                {
                    lnbf_type = m_keys_options.get_selected() == 0 ? LNBF_Type::Mono : LNBF_Type::Multi;
                }
                m_current_satellite.type = lnbf_type;

                int idx = m_keys_main.get_selected();
                m_keys_main.set_label(idx, OSD_Translate::translate(m_current_satellite.type).data());
                reset_sat_options();
                break;
            }

            case Func_Active::Polarity:
            {
                auto polarity = m_keys_options.get_selected() == 0 ? LNBF_Position::Normal : LNBF_Position::Inverted;
                m_current_satellite.position = polarity;
                int idx = m_keys_main.get_selected();
                m_keys_main.set_label(idx, OSD_Translate::translate(m_current_satellite.position));
                reset_sat_options();
                break;
            }

            case Func_Active::Switch:
            {
                DEBUG("Switch active\n");
                std::string diseqc = std::string(OSD_Translate::translate(m_current_satellite.switch_type));
                if ( diseqc != "Nenhum" )
                {
                    diseqc += " : " + std::to_string(m_current_satellite.switch_pos + 1);
                }
                DEBUG_MSG(OSD, INFO, "Current switch type: " << diseqc << "\n");
                m_keys_main.set_label(static_cast<size_t>(Func_Active::Switch), diseqc);
                reset_sat_options();
                break;
            }

            default:
            {
                if ( m_bgd_sat != nullptr )
                {
                    reset_sat_options();
                }
                else
                {
                    DEBUG("Function not implemented yet\n");
                }
                break;
            }    
        }
    }
}

void OSD_Edit_Satellite::edit_name()
{
    lv_obj_remove_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
    // Área do teclado
    lv_area_t area;
    area.x1 = cover_w;
    area.y1 = offset;
    area.x2 = width - cover_w;
    area.y2 = cover_h;

    // Esconde tecla salvar
    lv_obj_set_style_bg_color(m_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    auto callback = [ =, this](const Satellite & s)
    {
        // Atualiza nome da satélite
        m_current_satellite.name = s.name;
        int idx = m_keys_main.get_selected();
        m_keys_main.set_label(idx, m_current_satellite.name.c_str());
        reset_sat_options();
        lv_obj_add_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
        m_osd_keyboard.reset();
    };

    m_osd_keyboard = std::make_unique<OSD_Keyboard>(this);
    m_osd_keyboard->osd_keyboard(callback, area, m_current_satellite);
}

void OSD_Edit_Satellite::reset_sat_options()
{
    if (m_bgd_sat)
    {
        m_key_save.show();
        lv_obj_set_style_bg_color(m_line, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        lv_obj_add_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
        m_keys_options.clear();
        m_key_diseq_c.clear();   // must clear before parent is deleted by LVGL
        m_diseq_c_active = false; // reset so the next opening starts in the correct state
        DELETE_OBJ(m_bgd_sat);
    }
}

void OSD_Edit_Satellite::draw_sat_options()
{
    // Se opções já foram desenhadas, não faz nada
    if ( m_bgd_sat )
    {
        return;
    }

    auto on_option_selected = [this]() {
        m_key_save.hide();
        lv_obj_set_style_bg_color(m_line, OSD_COLOR_GREY, DEFAULT_SELECTOR);
        lv_obj_remove_flag(m_cover_area, LV_OBJ_FLAG_HIDDEN);
        auto w = width - cover_w - 130;
        m_bgd_sat = create_rect(m_mainscreen, cover_w, 0, w, cover_h, OSD_COLOR_BLACK);
        create_rect(m_bgd_sat, 0, 0, 3, cover_h, OSD_COLOR_ORANGE);
    };

    auto default_keys_options = [this]() {
        m_keys_options.clear();
        m_keys_options.set_background(m_bgd_sat);
        m_keys_options.set_horizontal();
        m_keys_options.set_y(btn_y);
        m_keys_options.set_width(btn_w);
        m_keys_options.set_height(btn_h);
        m_keys_options.set_spacing(btn_spacing);
        m_keys_options.set_fonts(font_25, font_semi_25);
    };

    auto m_selected_button = static_cast<Func_Active>(m_keys_main.get_selected());
    switch (m_selected_button)
    {
        case Func_Active::LNBF_Detection:
        {
            on_option_selected();
            default_keys_options();
            lnbf_detection();
            break;
        }

        case Func_Active::Band:
        {
            on_option_selected();
            default_keys_options();
            m_keys_options.set_y(270);
            m_keys_options.add_label(tr(__Banda_C));
            m_keys_options.add_label(tr(__Banda_KU));
            m_keys_options.draw();
            m_keys_options.select();
            break;
        }

        case Func_Active::LNBF_Type:
        {
            on_option_selected();
            default_keys_options();
            m_keys_options.set_y(344);

            size_t selected_index = 0;

            if (m_current_satellite.band == Band::Ku)
            {
                m_keys_options.add_label(tr(__Universal));
                m_keys_options.add_label(tr(__Multiponto));

                if (m_current_satellite.type == LNBF_Type::Universal)
                    selected_index = 0;
                else if (m_current_satellite.type == LNBF_Type::Multi)
                    selected_index = 1;
            }
            else
            {
                m_keys_options.add_label(tr(__Monoponto));
                m_keys_options.add_label(tr(__Multiponto));

                if (m_current_satellite.type == LNBF_Type::Mono)
                    selected_index = 0;
                else if (m_current_satellite.type == LNBF_Type::Multi)
                    selected_index = 1;
            }
            m_keys_options.draw();
            m_keys_options.select(selected_index);
            break;
        }

        case Func_Active::Switch:
        {
            on_option_selected();
            default_keys_options();
            m_keys_options.set_y(482);
            m_keys_options.set_width(btn_diseq_c_w);
            m_keys_options.set_spacing(btn_diseq_c_s);
            m_keys_options.add_label("OFF");
            m_keys_options.add_label("1");
            m_keys_options.add_label("2");
            m_keys_options.add_label("3");
            m_keys_options.add_label("4");
            m_keys_options.draw();
            disable_diseq_c();
            size_t select = m_current_satellite.switch_type == DiseqC_Type::None ? 0 : 
                            static_cast<size_t>(m_current_satellite.switch_pos) + 1;
            m_keys_options.select(select);
            DEBUG_MSG(OSD, INFO, "Switch type: " << OSD_Translate::translate(m_current_satellite.switch_type) << " - " << (int)select << "\n");
            break;
        }

        case Func_Active::Name:
        case Func_Active::Save:
        default:
            break;
    }
}

void OSD_Edit_Satellite::disable_diseq_c()
{
    for (const auto &sat : m_satellites)
    {
        if ( sat.switch_type == DiseqC_Type::DiseqC_1_0 and sat.id != m_current_satellite.id )
        {
            auto select = sat.switch_pos + 1;
            m_keys_options.set_disabled(select);
            DEBUG_MSG(OSD, INFO, "Disabling DiseqC 1.0 position: " << select << " for satellite: " << sat.name << "\n");
        }
    }
}

void OSD_Edit_Satellite::draw_diseq_c_buttons(DiseqC_Type _current_type)
{
    m_key_diseq_c.clear();
    if ( _current_type == DiseqC_Type::None )
    {
        DEBUG("DiseqC None selected, nothing to draw\n");
        return;
    }

    if ( _current_type == DiseqC_Type::DiseqC_1_0 or _current_type == DiseqC_Type::DiseqC_1_1 )
    {
        DEBUG("Drawing DiseqC buttons for type: " << OSD_Translate::translate(_current_type) << "\n");
        m_key_diseq_c.add_label("1");
        m_key_diseq_c.add_label("2");
        m_key_diseq_c.add_label("3");
        m_key_diseq_c.add_label("4");
        m_diseq_c_selected = std::min(m_diseq_c_selected, 3);
        m_key_diseq_c.set_lines(2);
        m_key_diseq_c.set_columns(3);
    }

    if ( _current_type == DiseqC_Type::DiseqC_1_1 )
    {
        DEBUG("Drawing additional DiseqC buttons for type: " << OSD_Translate::translate(_current_type) << "\n");
        m_key_diseq_c.add_label("5");
        m_key_diseq_c.add_label("6");
        m_key_diseq_c.add_label("7");
        m_key_diseq_c.add_label("8");
        m_key_diseq_c.add_label("9");
        m_key_diseq_c.add_label("10");
        m_key_diseq_c.add_label("11");
        m_key_diseq_c.add_label("12");
        m_key_diseq_c.add_label("13");
        m_key_diseq_c.add_label("14");
        m_key_diseq_c.add_label("15");
        m_key_diseq_c.add_label("16");
        m_key_diseq_c.set_lines(6);
        m_key_diseq_c.set_columns(3);
    }

    m_key_diseq_c.set_background(m_bgd_sat);
    m_key_diseq_c.set_fonts(font_20, font_semi_20);
    m_key_diseq_c.set_spacing(160, 60);

    m_key_diseq_c.draw();
    DEBUG("m_diseq_c_selected: " << static_cast<int>(m_diseq_c_selected) << "\n");
    m_key_diseq_c.select(m_diseq_c_selected);
    m_key_diseq_c.set_disable_all();
}

void OSD_Edit_Satellite::process_diseq_c(const Event_Remote_Control &_event)
{
    DEBUG("Processing DiseqC...\n");
    if (!m_diseq_c_active)
    {
        if(_event.key == Remote_Control_Key::KEY_VOLUP)
        {
            m_keys_options.next();
            DiseqC_Type diseq_type = static_cast<DiseqC_Type>(m_keys_options.get_selected());
            draw_diseq_c_buttons(diseq_type);
            return;
        }

        if(_event.key == Remote_Control_Key::KEY_VOLDOWN)
        {
            m_keys_options.previous();
            DiseqC_Type diseq_type = static_cast<DiseqC_Type>(m_keys_options.get_selected());
            draw_diseq_c_buttons(diseq_type);
            return;
        }

        if(_event.key == Remote_Control_Key::KEY_CHUP or _event.key == Remote_Control_Key::KEY_CHDOWN)
        {
            m_diseq_c_active = true;
            m_keys_options.set_disable_all();
            m_key_diseq_c.set_enable_all();
        }

        if(_event.key == Remote_Control_Key::KEY_VOLTAR)
        {
            reset_sat_options();
        }
        
        if(_event.key == Remote_Control_Key::KEY_OK)
        {
            size_t func_idx = static_cast<size_t>(Func_Active::Switch);
            auto selected = m_keys_options.get_selected();
            auto diseq_type = selected == 0 ? DiseqC_Type::None : DiseqC_Type::DiseqC_1_0;
            m_current_satellite.switch_type = diseq_type;
            auto diseq_type_str = OSD_Translate::translate(diseq_type);
            DEBUG("Selected DiseqC type: " << diseq_type_str << "\n");
            if ( diseq_type != DiseqC_Type::None )
            {
                std::string diseq_type_str_full = std::string(diseq_type_str) + " : " + std::to_string(selected);
                DEBUG("Selected DiseqC position: " << diseq_type_str_full << "\n");
                m_keys_main.set_label(func_idx, diseq_type_str_full.data());
                m_current_satellite.switch_pos = selected -1;
            }
            else
            {
                m_keys_main.set_label(func_idx, diseq_type_str.data());
                m_current_satellite.switch_pos = 0;
            }
            reset_sat_options();
        }
    }
    else
    {
        if(_event.key == Remote_Control_Key::KEY_VOLUP)
        {
            DEBUG("KEY_VOLUP pressed\n");
            m_key_diseq_c.next();
        }

        if(_event.key == Remote_Control_Key::KEY_VOLDOWN)
        {
            DEBUG("KEY_VOLDOWN pressed\n");
            m_key_diseq_c.previous();
        }

        if(_event.key == Remote_Control_Key::KEY_CHUP)
        {
            DEBUG("KEY_CHUP pressed\n");
            if ( m_key_diseq_c.is_first_line() )
            {
                DEBUG("First line reached, exiting DiseqC selection\n");
                m_diseq_c_active = false;
                m_diseq_c_selected = m_key_diseq_c.get_selected();
                m_key_diseq_c.set_disable_all();
                m_keys_options.set_enable_all();
            }
            else 
            {
                DEBUG("Moving to previous line\n");
                m_key_diseq_c.previous_line();
            }
        }

        if(_event.key == Remote_Control_Key::KEY_CHDOWN)
        {
            DEBUG("KEY_CHDOWN pressed\n");
            if ( m_key_diseq_c.is_last_line() )
            {
                DEBUG("Last line reached, exiting DiseqC selection\n");
                m_diseq_c_active = false;
                m_diseq_c_selected = m_key_diseq_c.get_selected();
                m_key_diseq_c.set_disable_all();
                m_keys_options.set_enable_all();
            }
            else 
            {
                DEBUG("Moving to next line\n");
                m_key_diseq_c.next_line();
            }
        }

        if(_event.key == Remote_Control_Key::KEY_OK or _event.key == Remote_Control_Key::KEY_VOLTAR)
        {
            DEBUG("KEY_OK or KEY_VOLTAR pressed, exiting DiseqC selection\n");
            m_diseq_c_active = false;
            m_diseq_c_selected = m_key_diseq_c.get_selected();
            m_key_diseq_c.set_disable_all();
            m_keys_options.set_enable_all();
        }
    }
}

void OSD_Edit_Satellite::draw_buttons()
{ 
    // Inicializa textos dos botões
    m_keys_main.add_label(tr(__Detectar_lnbf));
    m_keys_main.add_label(m_current_satellite.name);
    m_keys_main.add_label(OSD_Translate::translate(m_current_satellite.band));
    m_keys_main.add_label(OSD_Translate::translate(m_current_satellite.type).data());
    m_keys_main.add_label(OSD_Translate::translate(m_current_satellite.position));

    std::string diseqc = std::string(OSD_Translate::translate(m_current_satellite.switch_type));
    if ( diseqc != "Nenhum" )
    {
        diseqc += " : " + std::to_string(m_current_satellite.switch_pos + 1);
    }
    m_keys_main.add_label(diseqc.data());
    // Desenha botões principais
    m_keys_main.set_background(m_mainscreen);
    m_keys_main.set_vertical();
    m_keys_main.set_back_color(OSD_COLOR_BLACK);
    m_keys_main.draw();

    // Habilita ou desabilita botões conforme política de rede
    if (m_current_satellite.network_policies == Network_Policies::Sky)
    {
        DEBUG("Sky network policy: disabling some buttons\n");
        m_keys_main.set_disable_all();
    }
    else if (m_current_satellite.network_policies == Network_Policies::TVRO)
    {
        DEBUG("TVRO network policy: disabling some buttons\n");
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::Name));
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::Band));
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::Polarity));
    }
    else
    {
        DEBUG("No network policy: almost all buttons enabled\n");
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::Band));
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::LNBF_Detection));
        m_keys_main.set_disabled(static_cast<size_t>(Func_Active::Polarity));
    }
    m_keys_main.select();
    m_keys_main.go_to_first_enabled();

    // Desenha linha
    m_line = create_rect(m_mainscreen, line_x, line_y, line_width, line_heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line);
}

void OSD_Edit_Satellite::draw_titles()
{
    // Cria botões
    for (size_t i = 0; i < m_title_names.size() - 1; i++)
    {
        m_titles[i] = create_rect(m_mainscreen, title_x, 3 + i * title_spacing, title_width, title_heigth, OSD_COLOR_BLACK);
        lv_obj_set_style_radius(m_titles[i], title_heigth / 2, DEFAULT_SELECTOR);
        m_labels[i] = set_label_text(m_titles[i], m_title_names[i], 0, 0, font_25, OSD_COLOR_WHITE);
        lv_obj_align(m_labels[i], LV_ALIGN_LEFT_MID, 0, 0);
    }
}

void OSD_Edit_Satellite::create_progress_bar()
{
    /*Create a slider in the center of the display*/
    m_slider = lv_slider_create(m_bgd_sat);
    lv_obj_set_size(m_slider, slider_w, slider_h);
    lv_obj_align(m_slider, LV_ALIGN_CENTER, 0, slider_y);
    lv_obj_set_style_anim_duration(m_slider, 1000, 0);
    // Set the indicator color to green
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREEN, LV_PART_INDICATOR);
    // Set the background color to light gray
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREY, LV_PART_MAIN);
    // Make the knob completely transparent
    lv_obj_set_style_bg_opa(m_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_bgd_sat, "Detecção de LNBF", slider_label_x, slider_label_y, font_25, OSD_COLOR_WHITE);
    lv_obj_align(m_slider_label, LV_ALIGN_CENTER, 0, slider_label_y);
}

void OSD_Edit_Satellite::update_progress_bar_cb(lv_timer_t *_timer)
{
    OSD_Edit_Satellite *thiz = static_cast<OSD_Edit_Satellite *>(lv_timer_get_user_data(_timer));
    thiz->update_progress_bar();
}

void OSD_Edit_Satellite::update_progress_bar()
{
    // Atualiza barra de progresso
    if ( m_progress <= 100 )
    {
        m_progress += 4 ;
        m_progress = std::min(m_progress, 100) ;
        // Atualiza barra de progresso
        std::stringstream ss ;
        ss << tr(__Progresso) << ": " << m_progress << "%" ;
        lv_label_set_text(m_slider_label, ss.str().c_str());
        lv_slider_set_value(m_slider, m_progress, LV_ANIM_ON);
    }
}

void OSD_Edit_Satellite::process_changes(const Event_Remote_Control &_event)
{
    if ( m_bgd_sat == nullptr )
    {
        return;
    }

    DEBUG("Processing changes...\n");
    auto m_selected_button = static_cast<Func_Active>(m_keys_main.get_selected());
    if (_event.key == Remote_Control_Key::KEY_VOLUP)
    {
        m_keys_options.next();
    }
    else if (_event.key == Remote_Control_Key::KEY_VOLDOWN)
    {
        m_keys_options.previous();
    }
    switch (m_selected_button)
    {
        case Func_Active::Band:
        {
            DEBUG("Changing Band...\n");
            auto band = Band::C;
            band = m_keys_options.get_selected() == 0 ? Band::C : Band::Ku;
            DEBUG("Selected Band: " << OSD_Translate::translate(band) << "\n");
            m_current_satellite.band = band;
            break;
        }

        case Func_Active::LNBF_Type:
        {
            DEBUG("Changing LNBF Type...\n");
            auto lnbf_type = LNBF_Type::Universal;
            if (m_current_satellite.band == Band::Ku)
            {
                lnbf_type = m_keys_options.get_selected() == 0 ? LNBF_Type::Universal : LNBF_Type::Multi;
            }
            else
            {
                lnbf_type = m_keys_options.get_selected() == 0 ? LNBF_Type::Mono : LNBF_Type::Multi;
            }
            DEBUG("Selected LNBF Type: " << OSD_Translate::translate(lnbf_type) << "\n");
            m_current_satellite.type = lnbf_type;
            break;
        }

        case Func_Active::Polarity:
        {
            DEBUG("Changing Polarity...\n");
            auto polarity = m_keys_options.get_selected() == 0 ? LNBF_Position::Normal : LNBF_Position::Inverted;
            DEBUG("Selected Polarity: " << OSD_Translate::translate(polarity) << "\n");
            m_current_satellite.position = polarity;
            break;
        }
#if 0
        case Func_Active::Switch:
        {
            DEBUG("Changing Switch...\n");
            m_keys_options.next();
            auto selected = m_keys_options.get_selected();
            if ( selected == 0)
            {
                m_current_satellite.switch_type = DiseqC_Type::None;
                m_current_satellite.switch_pos = 0;
            }
            else
            {
                m_current_satellite.switch_type = DiseqC_Type::DiseqC_1_0;
                m_current_satellite.switch_pos = selected - 1;
            }
            DEBUG("Selected Switch type: " << OSD_Translate::translate(m_current_satellite.switch_type) << "\n");
            break;
        }
#endif
        default:
        {
            DEBUG_MSG(OSD, ERROR, "Function not implemented yet\n");
            break;
        }
    }
}

void OSD_Edit_Satellite::save_changes()
{
    DEBUG("Saving changes...\n");
    DEBUG("Id: " << m_current_satellite.id << "\n");
    DEBUG("Name: " << m_current_satellite.name << "\n");
    bool result = false;
    // Verifica se houve alguma alteração
    if ( m_current_satellite.id == 0 )
    {
        DEBUG("New satellite, saving...\n");
        Task::post_event_add_satellite(m_current_satellite);
        //auto config = Config::get_config();
        //config->load_satellite_list();
    }
    else if (m_current_satellite != m_previous_satellite)
    {
        DEBUG("Changes detected\n");
        update_satellite(m_current_satellite);
        //auto config = Config::get_config();
        //config->load_satellite_list();
        result = true;
    }
    else 
    {
        DEBUG("No changes detected\n");
    }
    Task::post_event(std::bind(m_callback, result));
}

void OSD_Edit_Satellite::update_satellite(const Satellite &satellite)
{
    State_File::App_State_File file;
    DEBUG("Id: " << file.current_satellite_id << "\n");
    DEBUG("Edited Id:" << satellite.id << "\n");
    Task::post_event_update_satellite(satellite);
    if (file.current_satellite_id == satellite.id)
    {
        auto type = satellite.type;
        DEBUG("LNBF type:" << (type == LNBF_Type::Universal ? "Universal" : type == LNBF_Type::Multi ? "Multi" : "Indefinido") << "\n");
        auto config = Config::get_config();
        config->set_lnbf_type(type);
        file.lnbf_type = type;
        file.write();
        Task::post_event_set_lnbf_type(type);

        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        Task::post_event_transponder_lock(POST_CALLER &current_lineup->transponders[0],true);
    } 
}

void OSD_Edit_Satellite::discard_changes()
{
}



} // namespace mb
