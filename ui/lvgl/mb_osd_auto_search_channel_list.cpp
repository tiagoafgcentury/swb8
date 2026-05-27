#include "mb_osd_auto_search_channel_list.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"
#include "mb_osd_fonts.h"

#include "mb_menu_resources.h"
#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"

#include "tasks/mb_task_player.h"
#include "tasks/mb_task_application.h"
#include "tasks/mb_task_osd.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_tuner.h"

#include <lvgl.h>
#include <functional>

#include "aui_common.h"

namespace mb {

OSD_Auto_Search_Channel_List *OSD_Auto_Search_Channel_List::s_instance { nullptr };

OSD_Auto_Search_Channel_List::OSD_Auto_Search_Channel_List(OSD *_parent):
    OSD(_parent)
{
    lv_style_init(&m_style_main);
    lv_style_init(&m_style_indicator);
    s_instance = this;
}

OSD_Auto_Search_Channel_List::~OSD_Auto_Search_Channel_List()
{
    s_instance = nullptr;
    lv_style_reset(&m_style_main);
    lv_style_reset(&m_style_indicator);
    DELETE_TIMER(m_refresh_timer);
    DELETE_OBJ(m_mainscreen);
    DELETE_OBJ(m_bgd);
    remove_focus();
}

// Processa tecla recebida
bool OSD_Auto_Search_Channel_List::handle_event_remote_control(const Event_Remote_Control &_event)
{
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_VOLTAR:
        {
            // Cancela a busca cega em andamento
            if (m_status == Status::Transponder_Scanning)
            {
                m_post_event_blind_scan->stop();
            }
            save_config_params();
            Task::post_event(std::bind(m_callback, false));
            break;
        }

        case Remote_Control_Key::KEY_OK:
        {
            if (m_status ==  Status::Finished_With_Success)
            {
                save_config_params();
                Task::post_event(std::bind(m_callback, true));
            }
            else
            {
                Task::post_event(std::bind(m_callback, false));
            }
            break;
        }

        default:
            break;
    }

    return true;
}

// Mostra a tela de seleção de satélite
void OSD_Auto_Search_Channel_List::auto_search_channel_list(OSD_Auto_Search_Channel_List_CB_t _callback, Satellite _sat, bool _need_bgd)
{
    m_callback = _callback;
    set_focus();
    // Carrega satélite recbido como argumento em variável da classe
    m_satellite = _sat;

    if(_need_bgd)
    {
        m_bgd = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_bgd);
        // Cria a tela principal
        m_mainscreen = create_rect(m_bgd, offset_x, offset_y, width, heigth, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_mainscreen);
    }
    else
    {
        // Cria a tela principal
        m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), offset_x, offset_y, width, heigth, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_mainscreen);
    }
    // Título da página
    m_title_label = set_label_text(m_mainscreen, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_title_label);
    lv_obj_align(m_title_label, LV_ALIGN_TOP_MID, 0, 0);
    create_services_table();
    // Criar barra de progresso
    create_progress_bar();
 
    // Cria timer repetitivo com intervalos de 2 segundos para atualizar a tela
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 2000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    // Caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Lista_de_Canais));
    m_btn_ok = create_rect(m_mainscreen, 0, 0, 200, 40, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_btn_ok);
    lv_obj_set_style_radius(m_btn_ok, 25, DEFAULT_SELECTOR);
    lv_obj_align(m_btn_ok, LV_ALIGN_BOTTOM_MID, 0, -80);
    auto label_ok = set_label_text(m_btn_ok, tr(__Finalizar), 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_align(label_ok, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(m_btn_ok, LV_OBJ_FLAG_HIDDEN);
    // Cria rodapé
    m_lbl_footer_back = MB_OSD_Footer::draw(m_mainscreen, tr(__Pressione_voltar_para_voltar), -40);
    lv_obj_null_on_delete(&m_lbl_footer_back);
    m_lbl_footer_ok = MB_OSD_Footer::draw(m_mainscreen, tr(__Pressione_ok_para_continuar), -40);
    lv_obj_null_on_delete(&m_lbl_footer_ok);
    lv_obj_add_flag(m_lbl_footer_ok, LV_OBJ_FLAG_HIDDEN);
    // Inicia o processo de busca cega
    start_blind_scan();
    // Cria baner com informações do progresso
    create_info_banner();
}

void OSD_Auto_Search_Channel_List::create_info_banner()
{
     m_info_banner = create_rect(m_mainscreen, info_banner_x, info_banner_y, info_banner_w, info_banner_h, OSD_COLOR_GREEN);
     lv_obj_null_on_delete(&m_info_banner);
     m_info_label = set_label_text(m_info_banner, tr(__Buscando_frequencias), 0, 0, font_20, OSD_COLOR_WHITE);
     lv_obj_null_on_delete(&m_info_label);
     lv_obj_align(m_info_label, LV_ALIGN_CENTER, 0, 0);
}

// Inicia o processo de busca cega
void OSD_Auto_Search_Channel_List::start_blind_scan()
{
    // Busca qual é satélite selecionado
    DEBUG("Starting blind scan on satellite: " << m_satellite.name << "\n");
    DEBUG("Satellite ID: " << dec << m_satellite.id << "\n");
    DEBUG("Band: " << (m_satellite.band == Band::C ? "C-BAND" : "KU-BAND") << "\n");
    DEBUG("LNBf Type: " << (m_satellite.type == LNBF_Type::Mono ? "MONO" :
                          m_satellite.type == LNBF_Type::Multi ? "MULTI" :
                          m_satellite.type == LNBF_Type::Universal ? "UNIVERSAL" : "UNDEFINED") << "\n");
    DEBUG("LNBf Position: " << static_cast<int>(m_satellite.position) << "\n");
    DEBUG("DiseqC Type: " << static_cast<int>(m_satellite.switch_type) << "\n");
    DEBUG("DiseqC Position: " << static_cast<int>(m_satellite.switch_pos) << "\n");
    
    // Inicia o processo de busca cega
    m_status = Status::Transponder_Scanning;
    m_detected_transponders.clear();
    uint32_t sat_id_local = m_satellite.id;
    m_post_event_blind_scan = std::make_shared<Event_Blind_Scan_Progress>(Event_Blind_Scan_Progress
    {
        .callback = [this, sat_id_local](Event_Blind_Scan_Progress::Status _status, uint32_t, uint32_t frequency, uint32_t sr, uint32_t polarity, uint8_t progress, aui_nim_freq_band _band, aui_nim_polar _polar)
        {
            Task::post_event([this, _status, sat_id_local, frequency, sr, polarity, progress, _band, _polar]()
            {
                if (s_instance != this)
                {
                    return;
                }
                if (m_status != Status::Transponder_Scanning)
                {
                    return;
                }
                m_current_progress = (progress * 50) / 100;
                if(sr != 0)
                {
                    std::cout << std::dec << (int)progress << "%  "
                        << std::setw(8) << std::left << std::dec << (int)sat_id_local
                        << std::setw(8) << std::left << std::dec << (int)frequency
                        << std::setw(8) << std::left << std::dec << (int) sr
                        << (_polar == AUI_NIM_POLAR_HORIZONTAL ? "Horizontal " : "Vertical ")
                        << std::setw(8) << std::left << (_band == AUI_NIM_LOW_BAND ? "Low" : "High")
                        << "\n";

                    Transponder tp;
                    auto f = calculate_frequency(frequency, _band, _polar);
                    tp.transponder_id.set_frequency(f, (static_cast<Polarity>(_polar)), sat_id_local );
                    tp.symbol_rate = sr;
                    tp.dvb_mode = DVB_Mode::DVBS2X;
                    m_detected_transponders.push_back(tp);
                }
                // Verifica se deve finalizar o processo de busca cega
                if (_status == Event_Blind_Scan_Progress::Status::Success)
                {
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

uint32_t OSD_Auto_Search_Channel_List::calculate_frequency(uint32_t _frequency, aui_nim_freq_band _band, aui_nim_polar _polarity)
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
    return result *1000;
}

// Finaliza o processo de busca cega
void OSD_Auto_Search_Channel_List::finish_blind_scan(std::vector<Transponder> tp_list)
{

    if (s_instance != this)
    {
        return;
    }

    //std::vector<Transponder> tp_list_2;
    //if (!tp_list.empty())
    //{
        //tp_list_2.push_back(tp_list.front());
    //}

    m_status = Status::Lineup_Scanning;
    // Set satellite_id on all detected transponders
    for (auto& tp : tp_list) {
        tp.satellite_id = m_satellite.id;
    }
    // Seta config com satélite atual
    auto config = Config::get_config();
    config->set_satellite_config_by_id(m_satellite.id, tp_list);
    DEBUG_MSG(OSD, DEBUG, "Total de transponders detectados: " << m_detected_transponders.size() << "\n");
    using namespace std::placeholders;
    m_channel_list_update_callback = std::make_shared<Event_List_Update>(Event_List_Update{
        .callback = std::bind(&OSD_Auto_Search_Channel_List::channel_list_update_callback, this, _1),
        .partial_callback = std::bind(&OSD_Auto_Search_Channel_List::channel_list_partial_callback, this, _1, _2),
        .scan_sat_count = 1,
        .scan_sat_index = 0,
        .has_sky = (m_satellite.id == 2),
        .has_claro = (m_satellite.id == 1)
    });
    Task::post_event_lineup_build(m_channel_list_update_callback);

}

void OSD_Auto_Search_Channel_List::channel_list_update_callback(bool /*_is_done*/)
{

    if (s_instance != this)
    {
        return;
    }

    if (m_refresh_timer)
    {
        lv_timer_delete(m_refresh_timer);
        m_refresh_timer = nullptr;
    }
    m_status = Status::Finished_With_Success;    
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    auto number_of_services_found = current_lineup->services.size();
    lv_slider_set_value(m_slider, 100, LV_ANIM_ON);
    lv_obj_clear_flag(m_btn_ok, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(m_lbl_footer_ok, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_lbl_footer_back, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_slider_label, LV_OBJ_FLAG_HIDDEN);
    DEBUG_MSG(OSD, DEBUG, "NUMBER OF SERVICES FOUND: " << number_of_services_found << "\n");
}

void OSD_Auto_Search_Channel_List::channel_list_partial_callback(size_t _transponder_seq, const std::vector<Service>& _services)
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

    lv_slider_set_value(m_slider, progress, LV_ANIM_ON);
    lv_label_set_text_fmt(m_slider_label, "%d%%", progress);

    
    /*
    **  Preenche a tabela de canais com os canais encontrados
    */
    uint16_t size = MAX_CHANNEL_LIST_VIEW - 1;
    uint16_t total_tv = 0;
    uint16_t total_radio = 0;

    for (const auto &srv : _services)
    {
        Basic_Service_Type srvType = to_basic_type(srv.service_type());
        if (srvType == Basic_Service_Type::TV)
        {
            total_tv += 1;
        }
        else if (srvType == Basic_Service_Type::Radio)
        {
            total_radio += 1;
        }
    }

    const uint16_t tv_start = total_tv > (size - 1) ? static_cast<uint16_t>(total_tv - (size - 1)) : 0;
    const uint16_t radio_start = total_radio > (size - 1) ? static_cast<uint16_t>(total_radio - (size - 1)) : 0;
    uint16_t tv_seen = 0;
    uint16_t radio_seen = 0;
    uint16_t tv_row = 1;
    uint16_t radio_row = 1;

    auto format_channel = [](lv_obj_t* label, const auto& srv)
    {
        int ch = srv.viewer_channel();
        int width = (ch >= 10000) ? 5 : 4;

        lv_label_set_text_fmt(label, "%0*d - %s",
                            width,
                            ch,
                            srv.name().data());
    };

    for (const auto &srv : _services)
    {
        Basic_Service_Type srvType = to_basic_type(srv.service_type());

        if (srvType == Basic_Service_Type::TV)
        {
            tv_seen += 1;
            if (tv_seen > tv_start && tv_row < size)
            {
                format_channel(m_tv_lines_table[tv_row], srv);
                tv_row += 1;
            }
        }
        else if (srvType == Basic_Service_Type::Radio)
        {
            radio_seen += 1;
            if (radio_seen > radio_start && radio_row < size)
            {
                format_channel(m_radio_lines_table[radio_row], srv);
                radio_row += 1;
            }
        }
    }

    if (total_tv > 0)
    {
        lv_label_set_text_fmt(m_tv_lines_table[size], tr(__Total_TV).data(), total_tv, total_tv == 2 ? tr(__canal).data() : tr(__canais).data());
    }

    if (total_radio > 0)
    {
        lv_label_set_text_fmt(m_radio_lines_table[size], tr(__Total_Radios).data(), total_radio, total_radio == 2 ? tr(__canal).data() : tr(__canais).data());
    }
}

void OSD_Auto_Search_Channel_List::refresh_cb(lv_timer_t *_tm)
{
    if (_tm == nullptr)
    {
        return;
    }

    OSD_Auto_Search_Channel_List *thiz = static_cast<OSD_Auto_Search_Channel_List *>(lv_timer_get_user_data(_tm));

    if (thiz)
    {
        thiz->refresh_progress();
    }
}

void OSD_Auto_Search_Channel_List::refresh_progress()
{
    auto progress = lv_slider_get_value(m_slider);
    if (m_status == Status::Transponder_Scanning)
    {
        if(progress != m_current_progress)
        {
            lv_slider_set_value(m_slider, m_current_progress, LV_ANIM_ON);
            lv_label_set_text_fmt(m_slider_label, "%d%%", m_current_progress);
        }

        if(m_total_transponders_found != m_detected_transponders.size())
        {
            m_total_transponders_found = m_detected_transponders.size();
            lv_label_set_text_fmt(m_info_label, tr(__Encontradas_x_frequencias).data(), m_total_transponders_found);
        }
    }
}

void OSD_Auto_Search_Channel_List::create_services_table()
{
    uint16_t size = MAX_CHANNEL_LIST_VIEW;
    uint16_t idx;
    m_bgd_table = create_rect(m_mainscreen, bgd_table_x, bgd_table_y, bgd_table_w, bgd_table_h, OSD_COLOR_BLUE);
    lv_obj_null_on_delete(&m_bgd_table);
    lv_obj_align(m_bgd_table, LV_ALIGN_TOP_MID, 0, 55);

    for (idx = 0; idx < size; idx++)
    {
        auto y = table_top + (TABLE_SPACING * idx);

        if ((idx == 0) || (idx == size - 1))
        {
            m_table_line[idx] = create_rect(m_bgd_table, 0, y, table_line_w, table_line_h, OSD_COLOR_GREY_MEDIUM);
            lv_obj_null_on_delete(&m_table_line[idx]);
            m_tv_lines_table[idx] =  set_label_text(m_table_line[idx], "", 0, 0, font_semi_20, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_tv_lines_table[idx]);
            lv_obj_align(m_tv_lines_table[idx], LV_ALIGN_LEFT_MID, 30, 0);
            m_radio_lines_table[idx] =  set_label_text(m_table_line[idx], "", 0, 0, font_semi_20, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_radio_lines_table[idx]);
            lv_obj_align(m_radio_lines_table[idx], LV_ALIGN_LEFT_MID, 485, 0);
        }
        else
        {
            m_table_line[idx] = create_rect(m_bgd_table, 0, y, table_line_w, table_line_h, OSD_COLOR_BLACK);
            lv_obj_null_on_delete(&m_table_line[idx]);
            create_rect(m_table_line[idx], 0, 0, table_line_w, 2, OSD_COLOR_GREY_DARK);
            m_tv_lines_table[idx] = set_label_text(m_table_line[idx], "", 0, 0, font_20, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_tv_lines_table[idx]);
            lv_obj_align(m_tv_lines_table[idx], LV_ALIGN_LEFT_MID, 30, 0);
            m_radio_lines_table[idx] = set_label_text(m_table_line[idx], "", 0, 0, font_20, OSD_COLOR_WHITE);
            lv_obj_null_on_delete(&m_radio_lines_table[idx]);
            lv_obj_align(m_radio_lines_table[idx], LV_ALIGN_LEFT_MID, 485, 0);
        }
    }

    lv_label_set_text(m_tv_lines_table[0], "TV");
    lv_label_set_text(m_radio_lines_table[0], tr(__Radio).data());
    create_rect(m_bgd_table, 455, 0, 2, bgd_table_h, OSD_COLOR_GREY_DARK);
}

void OSD_Auto_Search_Channel_List::create_progress_bar()
{
    lv_style_set_bg_opa(&m_style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&m_style_main, OSD_COLOR_GREY_MEDIUM);
    lv_style_set_bg_color(&m_style_indicator, OSD_COLOR_BLUE);
    m_bgd_slider = create_rect(m_mainscreen, m_bgd_slider_x, m_bgd_slider_y, m_bgd_slider_w, m_bgd_slider_h, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_slider);
    m_slider = lv_bar_create(m_bgd_slider);
    lv_obj_null_on_delete(&m_slider);
    lv_obj_add_style(m_slider, &m_style_main, LV_PART_MAIN);
    lv_obj_add_style(m_slider, &m_style_indicator, LV_PART_INDICATOR);
    lv_obj_set_size(m_slider, slider_w - 4, slider_h);
    lv_obj_align(m_slider, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_anim_duration(m_slider, 2000, 0);
    lv_obj_set_style_bg_color(m_slider, OSD_COLOR_YELLOW, LV_PART_INDICATOR);
    /*Create a label below the slider*/
    m_slider_label = set_label_text(m_bgd_slider, "", 0, 0, font_semi_25, OSD_COLOR_WHITE);
    lv_obj_null_on_delete(&m_slider_label);
    lv_obj_align(m_slider_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_label_set_text(m_slider_label, "0%");
}

void OSD_Auto_Search_Channel_List::save_config_params()
{
    DEBUG_MSG(OSD, DEBUG, "Saving configuration parameters...\n");
    DEBUG_MSG(OSD, DEBUG, "Satellite name: " << m_satellite.name << "\n");
    DEBUG_MSG(OSD, DEBUG, "Satellite id: " << dec << m_satellite.id << "\n");
    Config::get_config()->set_current_satellite(m_satellite.id);
    Task::post_event_easy_install_save(true);
    Task::post_event_display_clear();
    Task::post_event_lineup_save();
    Task::post_event_clock_need_update();
}

} // namespace mb
