#include "mb_osd_diagnosis.h"

#include "mb_menu_resources.h"
#include "mb_osd_translate.h"
#include "mb_osd_fonts.h"

#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "common/mb_version.h"
#include "common/mb_satellites.h"
#include "common/mb_state_file.h"

#include "tasks/mb_task_tuner.h"

#include "mb_terms_of_use.h"
#include "mb_events.h"
#include "mb_zone_id.h"
#include "../../project_version.h"
#include "hal/mb_system.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <sstream>


namespace mb {

OSD_Diagnosis *OSD_Diagnosis::s_instance { nullptr };

OSD_Diagnosis::OSD_Diagnosis(OSD *_parent):
    OSD(_parent)
{
    s_instance = this;

    auto config = Config::get_config();
    const auto &sat_config = config->selected_satellite_config();
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    Service *_srv = current_lineup->get_current_service();
    m_service_id = _srv ? _srv->service_id() : 0;
    m_transponder_id = _srv ? _srv->transponder_id() : Transponder_Id{};
    Polarity polarity = Polarity::UNDEFINED;
    switch (sat_config.network_id)
    {
        case Network_Id_Claro:
            m_satellite_info.name = "Star One D2";
            m_oper = Satellite_Operator::Claro;
            m_tps = MB_Satellites::get_transponder_list_for_snr(m_oper);
            if (!m_tps.empty())
            {
                m_satellite_info.frequency = m_tps[0].transponder_id.frequency();
                m_satellite_info.symbol_rate = m_tps[0].symbol_rate;
                polarity = m_tps[0].transponder_id.polarity();
                m_satellite_info.polarity[0] = polarity == Polarity::Horizontal ? 'H' : 'V';
            }
            break;

        case Network_Id_Sky:
            m_satellite_info.name = "Sky B1";
            m_oper = Satellite_Operator::Sky;
            m_tps = MB_Satellites::get_transponder_list_for_snr(m_oper);
            if (!m_tps.empty())
            {
                m_satellite_info.frequency = m_tps[0].transponder_id.frequency();
                m_satellite_info.symbol_rate = m_tps[0].symbol_rate;
                polarity = m_tps[0].transponder_id.polarity();
                m_satellite_info.polarity[0] = polarity == Polarity::Horizontal ? 'H' : 'V';
            }
            break;

        default:
            m_satellite_info.name = sat_config.name.data();
            if (_srv)
            {
                m_satellite_info.frequency = _srv->transponder_id().frequency() / 1000;
                polarity = _srv->transponder_id().polarity();
                m_satellite_info.polarity[0] = polarity == Polarity::Horizontal ? 'H' : 'V';
                auto tp = current_lineup->get_transponder(_srv->transponder_id());
                m_satellite_info.symbol_rate = tp ? tp->symbol_rate : 0;
            }
            else
            {
                m_satellite_info.frequency = 0;
                m_satellite_info.symbol_rate = 0;
                m_satellite_info.polarity[0] = 'U';
            }
            break;
    }
    m_satellite_info.polarity[1] = '\0';

    DEBUG_MSG(OSD, DEBUG," Current Transponder ID: " << m_transponder_id << "\n");
    DEBUG_MSG(OSD, DEBUG," Current Service ID: " << m_service_id << "\n");
    if (polarity != Polarity::UNDEFINED)
    {
        for (uint i = 0; i < current_lineup->services.size(); i++)
        {
            auto service = &current_lineup->services[i];
            auto frequency = service->transponder_id().frequency();
            auto _pol = service->transponder_id().polarity();
            if ( frequency == m_satellite_info.frequency and _pol == polarity)
            {
                DEBUG_MSG(OSD, DEBUG, "Service: " << service->name() << "\n");
                DEBUG_MSG(OSD, DEBUG, "Freq: " << frequency << "\n");
                Task::post_event_channel_change(POST_CALLER service);
                break;
            }
        }
    }

    auto satellite_id = config->get_current_satellite();
    auto satellite = config->get_satellite_by_id(satellite_id);
    m_satellite_info.switch_type = to_str(satellite.switch_type);
    m_satellite_info.switch_position = std::to_string(satellite.switch_pos);
}

OSD_Diagnosis::~OSD_Diagnosis()
{
    DELETE_OBJ(m_svr_data);
    DELETE_OBJ(m_main);
    DELETE_TIMER(m_tmr_signal);

    // Retorna ao canal sintonizado anterior
    DEBUG_MSG(OSD, DEBUG," Current Transponder ID: " << m_transponder_id << "\n");
    DEBUG_MSG(OSD, DEBUG," Current Service ID: " << m_service_id << "\n");
    if (m_service_id != 0)
    {
        auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
        Service *service = current_lineup->get_service(m_service_id, m_transponder_id);
        if (service)
        {
            Task::post_event_channel_change(POST_CALLER service);
        }
    }

    if (m_stop_player)
    {
        Task::post_event_player_stop();
    }
    remove_focus(); 
}

bool OSD_Diagnosis::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
            Task::post_event(m_callback);
            return true;

        default:
            return true;
    }

    return false;
}

void OSD_Diagnosis::show_diagnostics(Diagnostic_CB _callback, bool _stop_player)
{
    set_focus();
    m_callback = _callback;
    m_stop_player = _stop_player;
    constexpr auto start = 10;
    constexpr auto space = 28;
    constexpr auto left = 10;
    std::string separator = ": ";

    // Tela principal
    m_main = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), main_x, main_y, main_w, main_h, OSD_COLOR_BLACK);

    // Lambda function to create a label with the same style for all diagnostic items
    auto create_diagnostic_label = [this](lv_obj_t *parent, std::string_view text, int *line_num) {
        auto obj = set_label_text(parent, text, left, start + (*line_num) * space, font_semi_20, OSD_COLOR_WHITE);
        (*line_num)++;
        return obj;
    };

    // Box 1
    int line_num = 0 ;
    m_box_1 = create_rect(m_main, box_1_x, box_1_y, box_1_w, box_1_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_radius(m_box_1, 10, LV_PART_MAIN);

    std::string text = std::string(tr(__Modelo)) + separator + MBGUI_PRODUCT_NAME + " " + MBGUI_MODEL_NAME;
    create_diagnostic_label(m_box_1, text, &line_num);

    text = tr(__Versao_do_Software).data() + separator + MB_OSD_Version::get_major_minor_patch_version();
    create_diagnostic_label(m_box_1, text, &line_num);

    // Box 2
    line_num = 0;
    m_box_2 = create_rect(m_main, box_2_x, box_2_y, box_2_w, box_2_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_radius(m_box_2, 10, LV_PART_MAIN);

    text = std::string(tr(__Lista_de_Canais)) + separator;
    create_diagnostic_label(m_box_2, text, &line_num);

    auto [tvs, radios] = count_tvs_and_radios();
    text = std::to_string(tvs) + std::string(" ") + std::string(tr(__Canais_de_TV));
    create_diagnostic_label(m_box_2, text, &line_num);

    text = std::to_string(radios) + std::string(" ") + std::string(tr(__Canais_de_Radio));
    create_diagnostic_label(m_box_2, text, &line_num);

    // Box 3
    line_num = 0;
    m_box_3 = create_rect(m_main, box_3_x, box_3_y, box_3_w, box_3_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_radius(m_box_3, 10, LV_PART_MAIN);

    text = std::string(tr(__Informacoes_do_Receptor)) + separator;
    create_diagnostic_label(m_box_3, text, &line_num);

    m_caid = create_diagnostic_label(m_box_3, tr(__ID_do_Receptor), &line_num);
    m_scua = create_diagnostic_label(m_box_3, tr(__Codigo_de_Acesso), &line_num);

    text = tr(__Data_hora).data() + separator;

    text += (static_cast<Clock_Type>(g_clock_set) != Clock_Type::Manual)
            ? tr(__Automatico).data()
            : tr(__Manual).data();

    auto system_time = System::get_system_time().to_local_time();

    char time_text[6];
    snprintf(time_text, sizeof(time_text), "%.2d:%.2d",
         system_time.hour(),
         system_time.minute());

    text += " ";
    text += time_text;

    create_diagnostic_label(m_box_3, text, &line_num);

    uint32_t zone_id = Zone_ID::get_zone_id(Satellite_Operator::Claro);
    std::string zone_id_str = "Star One D2 - Zone ID: ";
    if (zone_id == 0)
    {
        zone_id_str += tr(__Nao_ativado);
    }
    else 
    {
        zone_id_str += std::to_string(zone_id);
    }
    create_diagnostic_label(m_box_3, zone_id_str, &line_num);

    uint32_t bouquet_id = Zone_ID::get_zone_id(Satellite_Operator::Sky) + 25000;
    std::string bouquet_id_str = "SKY B1 - Seg. Bouquet: ";
    if (bouquet_id == 25000)
    {
        bouquet_id_str += tr(__Nao_ativado);
    }
    else
    {
        bouquet_id_str += std::to_string(bouquet_id);
    }
    create_diagnostic_label(m_box_3, bouquet_id_str, &line_num);

    auto data = cat(MBGUI_LAST_ACTIVATION_DATE_FILE);
    text = tr(__Data_da_ultima_ativacao).data() + separator + data;
    create_diagnostic_label(m_box_3, text, &line_num);

    Terms_File::App_Terms_of_Use terms;
    text = tr(__Termos_e_condicoes_de_uso).data() + separator;
    text += terms.version;
    create_diagnostic_label(m_box_3, text, &line_num);

    auto terms_conditions_date = cat(MBGUI_TERMS_CONDITIONS_DATE_FILE);
    text = tr(__Aceite_dos_termos_de_uso).data() + separator + terms_conditions_date;
    create_diagnostic_label(m_box_3, text, &line_num);

    // Box 4
    line_num = 0;
    m_box_4 = create_rect(m_main, box_4_x, box_4_y, box_4_w, box_4_h, OSD_COLOR_GREY_DARK);
    lv_obj_set_style_radius(m_box_4, 10, LV_PART_MAIN);
    
    text = std::string(tr(__Satelite).data()) + separator;
    create_diagnostic_label(m_box_4, text, &line_num);

    {
        std::stringstream text;
        text << m_satellite_info.name << " "
             << m_satellite_info.frequency / 1000 << "MHz "
             << m_satellite_info.symbol_rate << "Kbps "
             << m_satellite_info.polarity;
        create_diagnostic_label(m_box_4, text.str(), &line_num);
    }

    {
        std::stringstream text;
        text << tr(__Codificado) 
        << " | " << m_satellite_info.switch_type 
        << " " << m_satellite_info.switch_position;

        create_diagnostic_label(m_box_4, text.str(), &line_num);
    }

    // Cria timer para atualizar dados e barra de progresso
    create_quality_bar();
    m_snr_data = set_label_text(m_main, "SNR: 0dB", snr_x, snr_y, font_25, OSD_COLOR_WHITE);
    m_tmr_signal = lv_timer_create(update_antenna_signal_cb, 500, this);
    lv_timer_set_repeat_count(m_tmr_signal, -1);
    MB_OSD_Footer::draw(m_main, tr(__Pressione_voltar_para_voltar), -40);
    using namespace std::placeholders;
    m_process_scua_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Diagnosis::set_cas_fingerprint, this, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_scua_cb);
    update_antenna_signal();
    Task::post_event_player_stop();
}

void OSD_Diagnosis::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    _caid = "CAID: " + _caid;
    _scua = "SCUA: " + _scua;
    lv_label_set_text(m_caid, _caid.c_str());
    lv_label_set_text(m_scua, _scua.c_str());
}

std::tuple<int, int> OSD_Diagnosis::count_tvs_and_radios()
{
    auto radios = 0;
    auto tvs = 0;

    for (auto current_lineup = Lineup_Mutex_Ref::get_current_lineup(); auto &srv : current_lineup->services)
    {
        Basic_Service_Type srvType = to_basic_type(srv.service_type());

        if (srvType == Basic_Service_Type::TV)
        {
            ++tvs;
        }
        else if (srvType == Basic_Service_Type::Radio)
        {
            ++radios;
        }
    }

    return { tvs, radios };
}

void OSD_Diagnosis::print_satellite()
{
    // Verifica se o label já foi criado
    if (m_svr_data)
    {
        return;
    }

    auto config = Config::get_config();
    auto band = OSD_Translate::translate(config->band());
    auto type = OSD_Translate::translate(config->lnbf_type());
    // Busca dados do serviço atual
    std::stringstream stream;
    stream << tr(__Satelite) << " " << m_satellite_info.name
           << " - " << band << " - " << type
           << " - " << tr(__Normal) << " - "
           << tr(__Canal_de_referencia) << " | "
           << (m_satellite_info.frequency / 1000) << "MHz/"
           << m_satellite_info.symbol_rate << "Kbps | "
           << m_satellite_info.polarity;
    DEBUG_MSG(OSD, DEBUG, stream.str() << "\n");
    m_svr_data = set_label_text(m_main, stream.str(), 0, 0, font_20, OSD_COLOR_WHITE);
    lv_obj_align(m_svr_data, LV_ALIGN_TOP_MID, 0, slider_y - 30);

    #warning "Remove debug prints later - Heron 07/01/2026"
    DEBUG( static_cast<int>(config->selected_satellite()) << ":" << std::left << std::setw(15) << m_satellite_info.name <<
            static_cast<int>(config->band()) << ":" << std::setw(10) << to_str(config->band()) <<
            static_cast<int>(config->lnbf_type()) << ":" << std::setw(11) << to_str(config->lnbf_type()) << "\n");
}

void OSD_Diagnosis::update_antenna_signal_cb(lv_timer_t *_timer)
{
    OSD_Diagnosis *thiz = static_cast<OSD_Diagnosis *>(lv_timer_get_user_data(_timer));
    thiz->update_antenna_signal();
}

void OSD_Diagnosis::update_antenna_signal()
{
    // Atualiza sinal da antena
    auto signal_info = Task_Tuner::get_signal_info();
    int quality = signal_info.quality / 10;
    lv_label_set_text_fmt(m_slider_label, "%s: %d%%", tr(__Qualidade).data(), quality);
    lv_slider_set_value(m_slider, quality, LV_ANIM_ON);
    double snr = signal_info.signal_noise_ratio;
    lv_label_set_text_fmt(m_snr_data, "SNR: %0.2f dB", snr / 100);
    print_satellite();
}

void OSD_Diagnosis::create_quality_bar()
{
    /*Create a slider in the center of the display*/
    m_slider = lv_slider_create(m_main);
    lv_obj_set_pos(m_slider, slider_x, slider_y);
    lv_obj_set_size(m_slider, slider_w, slider_h);
    lv_obj_set_style_anim_duration(m_slider, 500, 0);
    // Set the indicator color to green
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREEN, LV_PART_INDICATOR);
    // Set the background color to light gray
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_GREY, LV_PART_MAIN);
    // Make the knob completely transparent
    lv_obj_set_style_bg_opa(m_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_main, "", slider_label_x, slider_label_y, font_25, OSD_COLOR_WHITE);
    lv_label_set_text(m_slider_label, "");
    lv_obj_align(m_slider_label, LV_ALIGN_TOP_MID, 0, slider_label_y);
}

void OSD_Diagnosis::got_focus()
{
}

} // namespace mb
