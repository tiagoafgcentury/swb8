#include "mb_osd_select_switch.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_fade_canvas.h"

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task_eit_events.h"
#include "mb_osd_service_table_progress.h"
#include "common/mb_satellites.h"

#include <lvgl.h>
#include <sys/time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <time.h>
#include <iterator>

namespace mb {

Select_Switch* Select_Switch::s_instance{nullptr};

// Inicializa objetos no construtor
Select_Switch::Select_Switch(OSD *_parent):
    OSD(_parent)
{
    s_instance = this;
}
 
// Destrutor
Select_Switch::~Select_Switch()
{
    s_instance = nullptr;
    Osd_Breadcrumb::s_instance.remove_name();
    DELETE_OBJ(m_main);
    DELETE_TIMER(m_refresh_timer);
    remove_focus();
}

bool Select_Switch::handle_event_remote_control(const Event_Remote_Control &_event)
{
    auto func = m_switch_keys.get_selected_mapped_callback(_event.key);
    if (func)
    {
        DEBUG_MSG(OSD, DEBUG, "Executing mapped callback for key: " << to_str(_event.key) << "\n");
        func();
        return true;
    }
    DEBUG_MSG(OSD, DEBUG, "No callback mapped for key: " << to_str(_event.key) << "\n");
    return true;
}

void Select_Switch::show_switch_list(select_switch_callback_t _callback)
{
    m_callback = _callback;
    set_focus();
    // Cria a tela principal
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, offset_y, width, height, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main);
    // Desenha caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name("DiseqC 1.0");
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_main, tr(__Selecione_o_satelite_desejado_e_pressione_ok_para_continuar), -40);
    lv_obj_null_on_delete(&m_footer);
    // Carrega lista de satélites para preencher os nomes das chaves
    auto load_satellite_list_callback = std::bind(&Select_Switch::load_satellite_list_callback, this, std::placeholders::_1);
    // Só desenha o painel de opções após receber a lista de satélites, para já preencher os nomes das chaves
    Task::post_event_satellite_list_load(load_satellite_list_callback);
    // Troca o nome do satélite dummy traduzindo para o idioma do usuário
    m_dummy_satellite.name = tr(__Nenhum);
}

void Select_Switch::draw_switch_panel()
{
   // Desenha painel de opções
    m_switch_keys.set_parent(m_main);
    m_switch_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_switch_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_switch_keys.set_group_pos(m_switch_keys_x, m_switch_keys_y);
    m_switch_keys.set_group_key_size(m_switch_keys_w, m_switch_keys_h);
    m_switch_keys.set_group_align(LV_ALIGN_CENTER);
    m_switch_keys.add_key_with_header(m_switch_list[0].name, "LNBF 1", m_switch_callbacks);
    m_switch_keys.add_key_with_header(m_switch_list[1].name, "LNBF 2", m_switch_callbacks);
    m_switch_keys.add_key_with_header(m_switch_list[2].name, "LNBF 3", m_switch_callbacks);
    m_switch_keys.add_key_with_header(m_switch_list[3].name, "LNBF 4", m_switch_callbacks);

    // Desenha teclas de comando
    m_command_group = m_switch_keys.add_group();
    m_switch_keys.set_group_orientation(MB_OSD_Enhanced_Keys::Orientation::Horizontal);
    m_switch_keys.set_group_colors(OSD_COLOR_GREY_DARK, OSD_COLOR_ORANGE);
    m_switch_keys.set_group_disabled_text_color(OSD_COLOR_GREY);
    m_switch_keys.set_group_pos(m_command_keys_x, m_command_keys_y);
    m_switch_keys.set_group_key_size(m_command_keys_w, m_command_keys_h);
    m_switch_keys.set_group_align(LV_ALIGN_CENTER);
    m_switch_keys.add_key(tr(__Voltar), m_voltar_callbacks);
    m_switch_keys.add_key(tr(__Buscar), m_buscar_callbacks);
    // Desabilita a tecla de buscar por padrão, só habilita quando tiver um satélite com posição de chave válida
    m_switch_keys.set_key_enabled(m_command_group, 1, false);
    // Desenha o painel de opções
    m_switch_keys.draw();
}

void Select_Switch::next_callback()
{
    m_switch_keys.next();
}

void Select_Switch::previous_callback()
{
    m_switch_keys.prev();
}

void Select_Switch::next_group_callback()
{
    m_switch_keys.next_group();
}

void Select_Switch::previous_group_callback()
{
    m_switch_keys.prev_group();
}

void Select_Switch::load_satellite_list_callback(const std::vector<Satellite> &satellites)
{
    // Carrega a lista recebida e adiciona um satélite vazio no fim
    m_satellite_list = satellites;
    // add a dummy satellite with id (uint16_t)-1 to the end of the list to represent empty positions
    m_satellite_list.push_back(m_dummy_satellite);
    // Preenche a lista de satélites das chaves apenas com os satélites que tem tipo de chave DiseqC 1.0, limitando a 4 satélites (pois só temos 4 chaves) e ordenando pela posição da chave 
    m_switch_list = {};
    while (m_switch_list.size() < diseq_1_0_switches_size)
    {
        m_switch_list.push_back(m_dummy_satellite);
    } 

    for ( size_t i=0; i < m_satellite_list.size(); ++i)
    {
        if (m_satellite_list[i].switch_type == DiseqC_Type::DiseqC_1_0)
        {
            auto index = m_satellite_list[i].switch_pos;
            m_switch_list[index] = m_satellite_list[i];
        }
    }
    draw_switch_panel();

    // Habilita a tecla de buscar caso tenha pelo menos um satélite com posição de chave válida
    bool has_valid_satellite = std::any_of(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id != (uint16_t)-1;
    });
    m_switch_keys.set_key_enabled(m_command_group, 1, has_valid_satellite);
}

void Select_Switch::pick_satellite()
{
    // Habilita a tecla de buscar caso tenha pelo menos um satélite com posição de chave válida
    bool has_valid_satellite = std::any_of(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id != (uint16_t)-1;
    });

    // Verifica qual opção de chave está selecionada e chama função para selecionar satélite
    auto index = m_switch_keys.get_selected_key();
    auto label = m_switch_keys.get_selected_label();
    DEBUG_MSG(OSD, INFO, "Opção de chave selecionada: " << (int)index << " - " << label << "\n");
    
    // Find the iterator of the currently selected satellite in the master list
    auto current_it = std::find_if(m_satellite_list.begin(), m_satellite_list.end(), [&](const Satellite &s) {
        return s.id == m_switch_list[index].id;
    });

    // Start searching from the next element (or from the beginning if not found or at the end)
    auto search_start = (current_it != m_satellite_list.end() && std::next(current_it) != m_satellite_list.end()) ? std::next(current_it) : m_satellite_list.begin();

    auto next_satellite_it = m_satellite_list.end();

    // 1st pass: search from the current pos to the end
    for (auto it = search_start; it != m_satellite_list.end(); ++it) {
        auto used_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [&](const Satellite &used) {
            return used.id == it->id && used.id != m_switch_list[index].id;
        });
        if (used_it == m_switch_list.end() || it->switch_type == DiseqC_Type::None || std::string(it->name) == "") {
            next_satellite_it = it;
            break;
        }
    }

    // 2nd pass: wrap around and search from the beginning to the start point
    if (next_satellite_it == m_satellite_list.end()) {
        for (auto it = m_satellite_list.begin(); it != search_start; ++it) {
            auto used_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [&](const Satellite &used) {
                return used.id == it->id && used.id != m_switch_list[index].id;
            });
            if (used_it == m_switch_list.end() || it->switch_type == DiseqC_Type::None || std::string(it->name) == "") {
                next_satellite_it = it;
                break;
            }
        }
    }

    // Transfere o próximo satélite disponível para m_switch_list na posição index
    if (next_satellite_it != m_satellite_list.end())
    {
        m_switch_list[index] = *next_satellite_it;
        DEBUG_MSG(OSD, INFO, "Satélite selecionado: " << m_switch_list[index].name << " (ID: " << m_switch_list[index].id << ")\n");
    }
    else {
        m_switch_list[index] = m_dummy_satellite;
        DEBUG_MSG(OSD, INFO, "Nenhum satélite disponível, usando satélite vazio\n");
    }
    // Atualiza o nome da chave selecionada para o nome do satélite selecionado
    m_switch_keys.set_label(0, index, m_switch_list[index].name);

    // Habilita a tecla de buscar caso tenha pelo menos um satélite com posição de chave válida
    bool still_has_valid_satellite = std::any_of(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id != (uint16_t)-1;
    });
    if (still_has_valid_satellite != has_valid_satellite) {
        m_switch_keys.set_key_enabled(m_command_group, 1, still_has_valid_satellite);    
    }

    // Print for debug purposes the list of currently selected satellites
    DEBUG_MSG(OSD, INFO, "Satélites atualmente selecionados:\n");
    for (size_t i = 0; i < m_switch_list.size(); ++i) {
        const auto &s = m_switch_list[i];
        DEBUG_MSG(OSD, INFO, "Chave " << i << ": " << s.name << " (ID: " << s.id << ")\n");
    }
}

void Select_Switch::update_satellite_list()
{
    // Seta todos os satélites como DiseqC_Type::None inicialmente
    for (auto &s : m_satellite_list)
    {
        s.switch_type = DiseqC_Type::None;
        s.switch_pos = 0;
    }

    // Busca a configuração de de cada uma das chaves e transfere para o satélite
    for (size_t i = 0; i < m_switch_list.size(); ++i)
    {
        DEBUG_MSG(OSD, INFO, "Atualizando satélite da chave " << i << ": " << m_switch_list[i].name << " (ID: " << m_switch_list[i].id << ")\n");
        if (m_switch_list[i].id != (uint16_t)-1)
        {
            for(auto & temp_s : m_satellite_list)
            {
                if (temp_s.id == m_switch_list[i].id)
                {
                    temp_s.switch_type = DiseqC_Type::DiseqC_1_0;
                    temp_s.switch_pos = i;
                    DEBUG_MSG(OSD, INFO, "Satélite atualizado: " << temp_s.name << " - Switch type: " << (int)temp_s.switch_type << " - Switch pos: " << (int)temp_s.switch_pos << "\n");
                    break;
                }
            }
        }
    }

    for (auto &s : m_satellite_list)
    {
        DEBUG_MSG(OSD, INFO, "Satélite: " << s.name << " - Switch type: " << (int)s.switch_type << " - Switch pos: " << (int)s.switch_pos << "\n");
        Task::post_event_update_satellite(s);
    }
    //auto config = Config::get_config();
    //config->load_satellite_list();
}

void Select_Switch::buscar_callback()
{
    // Atualiza banco de dados com as posições dos satélites selecionados para busca
    update_satellite_list();
    m_warning_content = set_label_text_static(m_main, tr(__Verificando_sua_instalacao), 0, 0, font_semi_40, OSD_COLOR_GREEN);
    lv_obj_null_on_delete(&m_warning_content);
    lv_obj_align(m_warning_content, LV_ALIGN_CENTER, 0, 0);
    lv_timer_create([](lv_timer_t *timer)
    {
        if (s_instance)
        {
            DELETE_OBJ(s_instance->m_warning_content);
        }
        DELETE_TIMER(timer);
    }, 4000, nullptr);
    lv_obj_fade_out(m_warning_content, 4000, 0);
    start_switch_detection();
}

void Select_Switch::buscar_confirm_callback(bool _result)
{
    m_message_box.reset();
    if (_result)
    {
        Task::post_event_delete_all_services();
        start_search();
    }
}

void Select_Switch::start_search()
{
    // Look for satellite_id = 2 in m_switch_list and if exists, transfer to first position of m_satellite_scan_list
    m_satellite_scan_list.clear();
    auto it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 2;
    });
    if (it != m_switch_list.end()) {
        m_satellite_scan_list.insert(m_satellite_scan_list.begin(), *it);
    }

    // Look for satellite_id = 1 in m_switch_list and if exists, transfer to next position of m_satellite_scan_list
    it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 1;
    });
    if (it != m_switch_list.end()) {
        m_satellite_scan_list.push_back(*it);
    }

    // Look for remaining satellites in m_switch_list and if exists, transfer to m_satellite_scan_list
    for (const auto &s : m_switch_list) {
        if (s.id != 1 && s.id != 2 and s.id != (uint16_t)-1) {
            auto it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [&](const Satellite &sat) {
                return sat.id == s.id;
            });
            if (it != m_switch_list.end()) {
                m_satellite_scan_list.push_back(*it);
            }
        }
    }

    // If no satellite was selected, return
    if (m_satellite_scan_list.empty()) {
        DEBUG_MSG(OSD, INFO, "Nenhum satélite selecionado para busca\n");
        return;
    }

    // Print for debug purposes
    m_satellite_scan_index = 0;
    for (size_t i = 0; i < m_satellite_scan_list.size(); ++i) {
        const auto &s = m_satellite_scan_list[i];
        DEBUG_MSG(OSD, INFO, "Satélite para busca " << i << ": " << s.name << " (ID: " << s.id << ")\n");
    }
    DEBUG_MSG(OSD, INFO, "Iniciando busca com " << m_satellite_scan_list.size() << " satélites\n");
    
    // Check if terms of use OSD needs to be shown (if there's any with satellite_id = 2) and show it, otherwise post event to start search
    bool has_sky_satellite = std::any_of(m_satellite_scan_list.begin(), m_satellite_scan_list.end(), [](const Satellite &s) {
        return s.id == 2;
    });
    if (has_sky_satellite) {
        if (!m_osd_terms_of_use) {
            m_osd_terms_of_use = std::make_unique<OSD_Terms_of_Use>(this);
        }
        m_osd_terms_of_use->show_menu_terms_of_use(std::bind(&Select_Switch::terms_of_use_callback, this, std::placeholders::_1));
    }
    else {
        blind_scan();
    }
}

void Select_Switch::terms_of_use_callback(bool _result)
{
    m_osd_terms_of_use.reset();
    if (_result) {
        blind_scan();
    }
}

void Select_Switch::blind_scan()
{
    // Cria tabela de serviços
    create_services_table();
    lv_obj_add_flag(m_footer, LV_OBJ_FLAG_HIDDEN);

    // Check if it is the last satellite to be scanned, if so, post event to update channel list and return
    if (m_satellite_scan_index >= m_satellite_scan_list.size())
    {        
        DEBUG_MSG(OSD, INFO, TERM_RED_BOLD "Busca finalizada para todos os satélites selecionados\n" TERM_RESET);
        m_service_table->add_status(tr(__Busca_de_canais_finalizada));

        lv_timer_create([](lv_timer_t *timer)
        {
            if (s_instance)
            {
                s_instance->m_service_table->reset();
                s_instance->save_config_params();
                auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
                if (not current_lineup->services.empty())
                {
                    s_instance->m_exit_all = true;
                }
                s_instance->voltar_callback();
                lv_obj_remove_flag(s_instance->m_footer, LV_OBJ_FLAG_HIDDEN);
            }
            lv_timer_del(timer);
        }, 3000, nullptr);
        return;
    }

    DEBUG_MSG(OSD, INFO, TERM_GREEN_BOLD "Iniciando busca cega para o satélite " << m_satellite_scan_list[m_satellite_scan_index].name << "\n" TERM_RESET);
    char status_msg[128];
    snprintf(status_msg, sizeof(status_msg), tr(__Buscando_canais_no_satelite).data(), m_satellite_scan_list[m_satellite_scan_index].name.data());
    m_service_table->add_status(status_msg);
    DEBUG_MSG(OSD, INFO, TERM_GREEN_BOLD "m_satellite_scan_index: " << m_satellite_scan_index << "\n" TERM_RESET);
    std::string title = std::string(tr(__Buscando)) + " " + m_satellite_scan_list[m_satellite_scan_index].name + " (" + std::to_string(m_satellite_scan_index + 1) + "/" + std::to_string(m_satellite_scan_list.size()) + ")";
    m_service_table->set_title(title);

    // Lambda function to execute post event passing NID_t with satellite_id of the current satellite being scanned
    auto post_scan_event = [this](NID_t _nid) {
        auto config = Config::get_config();
        config->set_satellite_config(_nid);
        using namespace std::placeholders;
        bool is_last = (m_satellite_scan_index + 1 >= m_satellite_scan_list.size());
        bool has_sky = std::any_of(m_satellite_scan_list.begin(), m_satellite_scan_list.end(), [](const Satellite &s) {
            return s.id == 2;
        });
        bool has_claro = std::any_of(m_satellite_scan_list.begin(), m_satellite_scan_list.end(), [](const Satellite &s) {
            return s.id == 1;
        });
        m_channel_list_update_callback = std::make_shared<Event_List_Update>(Event_List_Update{
            .callback = std::bind(&Select_Switch::channel_list_update_callback, this, _1),
            .partial_callback = std::bind(&Select_Switch::channel_list_partial_callback, this, _1, _2),
            .clear_table = (m_satellite_scan_index == 0),
            .emit_lineup_ready = is_last,
            .scan_sat_count = static_cast<uint16_t>(m_satellite_scan_list.size()),
            .scan_sat_index = static_cast<uint16_t>(m_satellite_scan_index),
            .has_sky = has_sky,
            .has_claro = has_claro
        });
        Task::post_event_lineup_build(m_channel_list_update_callback);
    };

    m_progress_current_value = 0;
    m_progress_last_value = 0;
    m_service_table->set_progress(0);
    auto satellite = m_satellite_scan_list[m_satellite_scan_index];
    if (satellite.id == 2) {
        post_scan_event(Network_Id_Sky);
        m_progress_step_value = 11;

    }
    else if (satellite.id == 1) {
        post_scan_event(Network_Id_Claro);
        m_progress_step_value = 200;
    }
    else {
        blind_scan_generic();
        m_progress_step_value = 1;
    }
    ++m_satellite_scan_index;
}

void Select_Switch::blind_scan_generic()
{
    // Busca qual é satélite selecionado
    m_satellite = m_satellite_scan_list[m_satellite_scan_index];
    DEBUG("Starting blind scan on satellite: " << m_satellite.name << "\n");
    DEBUG("Satellite ID: " << dec << m_satellite.id << "\n");
    m_detected_transponders.clear();
    m_total_transponders_found = 0;
    m_clear_table = (m_satellite_scan_index == 0);
    m_emit_lineup_ready = (m_satellite_scan_index + 1 >= m_satellite_scan_list.size());
    uint32_t sat_id_local = m_satellite.id;
    m_post_event_blind_scan = std::make_shared<Event_Blind_Scan_Progress>(Event_Blind_Scan_Progress
    {

        .callback = [this, sat_id_local](Event_Blind_Scan_Progress::Status _status, uint32_t, uint32_t frequency, uint32_t sr, uint32_t polarity, uint8_t progress, aui_nim_freq_band _band, aui_nim_polar _polar)
        {
            Task::post_event([this, _status, sat_id_local, frequency, sr, polarity, progress, _band, _polar]()
            {
                if (s_instance != this) { return; }
                m_current_progress = (progress * 50) / 100;
                if(sr != 0)
                {
                    std::cout << (int)progress << "%  "
                        << std::setw(8) << std::left << (int)sat_id_local
                        << std::setw(8) << std::left << (int)frequency
                        << std::setw(8) << std::left << (int) sr
                        << (_polar == AUI_NIM_POLAR_HORIZONTAL ? "H - " : "V - ")
                        << (_band == AUI_NIM_LOW_BAND ? "Low" : "High")
                        << "\n";

                    Transponder tp;
                    auto f = calculate_frequency(frequency, _band, _polar);
                    tp.transponder_id.set_frequency(f, (static_cast<Polarity>(_polar)), sat_id_local );
                    tp.symbol_rate = sr;
                    tp.dvb_mode = DVB_Mode::DVBS2X;
                    m_detected_transponders.push_back(tp);
                    DEBUG_MSG(OSD, INFO, "Transponder detectado: " << f << "Khz/" << sr << "Kbps/" << (_polar == AUI_NIM_POLAR_HORIZONTAL ? "H\n" : "V\n"));
                }
                // Verifica se deve finalizar o processo de busca cega
                if (_status == Event_Blind_Scan_Progress::Status::Success)
                {
                    DEBUG_MSG(OSD, INFO, "Busca cega finalizada para o satélite " << m_satellite.name << "\n");
                    this->finish_blind_scan(m_detected_transponders);
                }
            });
        },
        .stop_callback = nullptr
    });
    m_event_weak = std::weak_ptr<Event_Blind_Scan_Progress>(m_post_event_blind_scan);
    if (auto event = m_event_weak.lock())
    {
        event->sat_id = m_satellite.id;
    }
    Task::post_event_blind_scan_progress(m_event_weak);
}

uint32_t Select_Switch::calculate_frequency(uint32_t _frequency, aui_nim_freq_band _band, aui_nim_polar _polarity)
{
    uint32_t result ;
    if (m_satellite.band == Band::C)
    {
        uint32_t lo = 5150;
        if (m_satellite.type == LNBF_Type::Multi && _polarity == AUI_NIM_POLAR_VERTICAL)
        {
            lo = 5750;
        }
        result = lo - _frequency;
    }
    else 
    {
        if (m_satellite.type == LNBF_Type::Universal)
        {
            uint32_t lo = _band == AUI_NIM_LOW_BAND ? 9750 : 10600;
            result = lo + _frequency;
        }
        else 
        {
            uint32_t lo = _polarity == AUI_NIM_POLAR_HORIZONTAL ? 11400 : 10250;
            result = lo + _frequency;
        }
    }
    DEBUG_MSG(OSD, DEBUG, "Calculated frequency: " << result * 1000 << " kHz (Input frequency: " << _frequency << " kHz\n");
    return result *1000;
}

void Select_Switch::finish_blind_scan(std::vector<Transponder> tp_list)
{
    // Check if instance is still valid
    if (s_instance != this) { return; }
    // If transponder list is empty, goto next satellite
    if (tp_list.empty())    {
        DEBUG_MSG(OSD, INFO, "Nenhum transponder detectado para o satélite " << m_satellite.name << ", pulando para o próximo satélite\n");
        blind_scan();
        return;
    }

    // Set satellite_id on all detected transponders
    for (auto& tp : tp_list) {
        tp.satellite_id = m_satellite.id;
    }
    // Seta config com satélite atual
    auto config = Config::get_config();
    config->set_satellite_config_by_id(m_satellite.id, tp_list);
    //config->set_satellite_config(Network_Id::Network_Id_Generic);
    DEBUG_MSG(OSD, INFO, "Total de transponders detectados: " << m_detected_transponders.size() << "\n");
    using namespace std::placeholders;
    bool has_sky = std::any_of(m_satellite_scan_list.begin(), m_satellite_scan_list.end(), [](const Satellite &s) {
        return s.id == 2;
    });
    bool has_claro = std::any_of(m_satellite_scan_list.begin(), m_satellite_scan_list.end(), [](const Satellite &s) {
        return s.id == 1;
    });
    m_channel_list_update_callback = std::make_shared<Event_List_Update>(Event_List_Update{
        .callback = std::bind(&Select_Switch::channel_list_update_callback, this, _1),
        .partial_callback = std::bind(&Select_Switch::channel_list_partial_callback, this, _1, _2),
        .clear_table = m_clear_table,
        .emit_lineup_ready = m_emit_lineup_ready,
        .scan_sat_count = static_cast<uint16_t>(m_satellite_scan_list.size()),
        .scan_sat_index = static_cast<uint16_t>(m_satellite_scan_index - 1),
        .has_sky = has_sky,
        .has_claro = has_claro
    });
    Task::post_event_lineup_build(m_channel_list_update_callback);
}

void Select_Switch::channel_list_update_callback(bool /*_is_done*/)
{
    if (s_instance != this) { return; }

    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto number_of_services_found = current_lineup->services.size();
    DEBUG_MSG(OSD, INFO, "NUMBER OF SERVICES FOUND: " << number_of_services_found << "\n");

    char status_msg[128];
    snprintf(status_msg, sizeof(status_msg), tr(__Total_de_canais_encontrados).data(), (int)number_of_services_found);
    m_service_table->add_status(status_msg);

    // Delay to show the message before continuing
    lv_timer_create([](lv_timer_t *timer)
    {
        if (s_instance)
        {
            s_instance->m_service_table->add_status(tr(__Filtrando_servicos_de_TV_e_Radio));

            // Another delay for the filtering message
            lv_timer_create([](lv_timer_t *timer2)
            {
                if (s_instance)
                {
                    Task::post_event(std::bind(&Select_Switch::blind_scan, s_instance));
                }
                lv_timer_del(timer2);
            }, 1000, nullptr);
        }
        lv_timer_del(timer);
    }, 1000, nullptr);
}

void Select_Switch::channel_list_partial_callback(size_t _transponder_seq, std::vector<Service> _services)
{
    if (s_instance != this)
    {
        return;
    }

    if (m_detected_transponders.empty())
    {
        return;
    }

    // Exibindo progresso na barra de 50% a 100%
    int progress = 50 + static_cast<int>(((_transponder_seq + 1) * 49) / m_detected_transponders.size());
    if (progress > 99)
    {
        progress = 99;
    }
    DEBUG_MSG(OSD, DEBUG, "Progress: " << progress << "% (" << (_transponder_seq + 1) << "/" << m_detected_transponders.size() << ")\n");
    if (_services.size() > 0)
    {
        char status_msg[128];
        snprintf(status_msg, sizeof(status_msg), tr(__Total_de_canais_encontrados).data(), (int)_services.size());
        m_service_table->add_status(status_msg);
    }
}

void Select_Switch::refresh_progress()
{
    if (!m_service_table)
    {
        return;
    }

    if (m_progress_current_value > 1000)
    {
        return;
    }

    m_progress_current_value += m_progress_step_value;
    uint8_t current_value = uint8_t(m_progress_current_value/10);
    if (current_value == m_progress_last_value)
    {
        return;
    }

    m_progress_last_value = current_value;
    m_service_table->set_progress(std::min<uint8_t>(current_value, 100));
}

void Select_Switch::save_config_params()
{
    DEBUG_MSG(OSD, INFO, "Saving configuration parameters...\n");
    DEBUG_MSG(OSD, INFO, "Satellite name: " << m_satellite.name << "\n");
    DEBUG_MSG(OSD, INFO, "Satellite id: " << dec << m_satellite.id << "\n");
    Config::get_config()->set_current_satellite(m_satellite.id);
    Task::post_event_easy_install_save(true);
    Task::post_event_display_clear();
    Task::post_event_lineup_save();
    Task::post_event_clock_need_update();
}

void Select_Switch::create_services_table()
{
    if (m_service_table)
        return;
    m_service_table = std::make_unique<MB_OSD_Service_Table_Progress<>>(this);
    m_service_table->create_services_table(m_main);

    // Cria timer repetitivo com intervalos de 1 segundo para atualizar a tela
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create([](lv_timer_t *timer)
        {
            auto thiz = static_cast<Select_Switch *>(lv_timer_get_user_data(timer));
            thiz->refresh_progress();
        }, 1000, this);
        lv_timer_ready(m_refresh_timer);
    }
}

void Select_Switch::start_switch_detection()
{
    if (m_detection_timer)
    {
        return;
    }

    m_is_searching = true;
    m_detection_status = DETECTION_IDLE;
    m_detection_timer = lv_timer_create([](lv_timer_t *timer)
    {
        auto thiz = static_cast<Select_Switch *>(lv_timer_get_user_data(timer));
        thiz->detection_process();
        if (!thiz->m_is_searching)
        {
            DELETE_TIMER(thiz->m_detection_timer);
        }
    }, 2000, this);
    lv_timer_ready(m_detection_timer);
}

void Select_Switch::detection_process()
{
    switch (m_detection_status)
    {
        case DETECTION_IDLE:
        {
            DEBUG_MSG(OSD, INFO, "Iniciando processo de detecção de switch...\n");
            m_sky_detected = false;
            m_claro_detected = false;
            if (!start_sky_detection())
            {
                if(!start_claro_detection())
                {
                    finish_detection();
                }
            }
            break;
        }

        case DETECTING_SKY:
        {
            DEBUG_MSG(OSD, INFO, "Processo de detecção de switch - etapa Sky\n");
            m_sky_detected = signal_detected();
            if(!start_claro_detection())
            {
                finish_detection();
            }
            break;
        }

        case DETECTING_CLARO:
        {
            DEBUG_MSG(OSD, INFO, "Processo de detecção de switch - etapa Claro\n");
            m_claro_detected = signal_detected();
            finish_detection();
            break;
        }

        default:
             break;
    }
}

bool Select_Switch::start_sky_detection()
{
    auto sky_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 2;
    });
    
    if (sky_it != m_switch_list.end())
    {
        m_detection_status = DETECTING_SKY;
        auto m_tp_sky = MB_Satellites::get_transponder_list_for_snr(Satellite_Operator::Sky);
        m_transponder = m_tp_sky[0];
        Task::post_event_transponder_lock(POST_CALLER &m_transponder);
    }
    return sky_it != m_switch_list.end();
}

bool Select_Switch::start_claro_detection()
{
    auto claro_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 1;
    });
    if (claro_it != m_switch_list.end())
    {
        m_detection_status = DETECTING_CLARO;
        auto m_tp_claro = MB_Satellites::get_transponder_list_for_snr(Satellite_Operator::Claro);
        m_transponder = m_tp_claro[0];
        Task::post_event_transponder_lock(POST_CALLER &m_transponder);
    }
    return claro_it != m_switch_list.end();
}

void Select_Switch::finish_detection()
{
    m_detection_status = DETECTION_FINISHED;
    m_is_searching = false;
    // Só considera erro se tiver pelo menos um satélite de cada operador na lista, caso contrário, pode ser que o usuário não tenha aquele satélite e não é um erro de detecção
    auto sky_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 2;
    });
    auto claro_it = std::find_if(m_switch_list.begin(), m_switch_list.end(), [](const Satellite &s) {
        return s.id == 1;
    });

    bool sky = false;
    bool claro = false;
    if (sky_it != m_switch_list.end() && m_sky_detected)
    {
        DEBUG_MSG(OSD, INFO, "Sinal detectado para o satélite Sky\n");
    }
    else if (sky_it != m_switch_list.end())
    {
        sky = true;
        DEBUG_MSG(OSD, ERROR, "Satélite Sky presente mas sinal não detectado\n");
    }
    if (claro_it != m_switch_list.end() && m_claro_detected)
    {
        DEBUG_MSG(OSD, INFO, "Sinal detectado para o satélite Claro\n");
    }
    else if (claro_it != m_switch_list.end())
    {
        claro = true;
        DEBUG_MSG(OSD, ERROR, "Satélite Claro presente mas sinal não detectado\n");
    }

    m_detection_error = false;
    if ((sky_it != m_switch_list.end() && !m_sky_detected) || (claro_it != m_switch_list.end() && !m_claro_detected))
    {
        std::string error_message = "";
        if(sky and claro)
        {
            error_message = std::string(tr(__Satelite_Sky_Star_One_D2_nao_detectados)) +
                            std::string(tr(__Verifique_sua_instalacao));
        }
        else if(sky)
        {
            error_message = std::string(tr(__Satelite_Sky_nao_detectado)) +                                                                                                 std::string(tr(__Verifique_sua_instalacao));
        }
        else if(claro)
        {
            error_message = std::string(tr(__Satelite_Star_One_D2_nao_detectado)) +         std::string(tr(__Verifique_sua_instalacao));
        }

        if (not m_message_box)
        {
            m_message_box = std::make_unique<OSD_Message_Box>(this);
        }
        m_message_box->show_message_box_ok(std::bind(&Select_Switch::satellite_error_callback, this), error_message);
        m_detection_error = true;
    }
    else
    {
        if (not m_message_box)
        {
            m_message_box = std::make_unique<OSD_Message_Box>(this);
        }
        m_message_box->show_message_box_yes_no(std::bind(&Select_Switch::buscar_confirm_callback, this, std::placeholders::_1), tr(__Esta_acao_vai_apagar_todos_canais_deseja_continuar), false);
    }
}

void Select_Switch::satellite_error_callback()
{
    m_message_box.reset();
}

bool Select_Switch::signal_detected()
{
    auto signal_info = Task_Tuner::get_signal_info();
    return signal_info.quality >= 20;
}


} // namespace mb
