#include "hal/mb_tuner.h"

#include <string.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iterator>
#include <thread>

#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "hal/mb_diseqc.h"
#include "hal/mb_lnb_config.h"
#include "mb_ali_globals.h"

// In OPEN and LOCK we have to use the same value
#define MBGUI_DVBS_ALI_MODE AUI_NIM_STD_DVBS2_AUTO

using namespace std::chrono_literals;

namespace {

mb::Modulation_Type from_ali_modulation_type(aui_nim_s_modulation_type _type) {
    using namespace mb;

    switch (_type) {
        case AUI_NIM_S_MODUL_QPSK:
            return Modulation_Type::QPSK;

        case AUI_NIM_S_MODUL_8PSK:
            return Modulation_Type::PSK_8;

        case AUI_NIM_S_MODUL_16APSK:
            return Modulation_Type::APSK_16;

        case AUI_NIM_S_MODUL_32APSK:
            return Modulation_Type::APSK_32;
    }

    return Modulation_Type::UNDEFINED;
}

mb::FEC_Rate from_ali_fec_rate(aui_nim_fec _rate) {
    using namespace mb;

    switch (_rate) {
        case AUI_NIM_FEC_AUTO:
            return FEC_Rate::AUTO;

        case AUI_NIM_FEC_1_2:
            return FEC_Rate::FEC_1_2;

        case AUI_NIM_FEC_2_3:
            return FEC_Rate::FEC_2_3;

        case AUI_NIM_FEC_3_4:
            return FEC_Rate::FEC_3_4;

        case AUI_NIM_FEC_5_6:
            return FEC_Rate::FEC_5_6;

        case AUI_NIM_FEC_7_8:
            return FEC_Rate::FEC_7_8;

        case AUI_NIM_FEC_1_4:
            return FEC_Rate::FEC_1_4;

        case AUI_NIM_FEC_1_3:
            return FEC_Rate::FEC_1_3;

        case AUI_NIM_FEC_2_5:
            return FEC_Rate::FEC_2_5;

        case AUI_NIM_FEC_3_5:
            return FEC_Rate::FEC_3_5;

        case AUI_NIM_FEC_4_5:
            return FEC_Rate::FEC_4_5;

        case AUI_NIM_FEC_8_9:
            return FEC_Rate::FEC_8_9;

        case AUI_NIM_FEC_9_10:
            return FEC_Rate::FEC_9_10;
    }

    return FEC_Rate::UNDEFINED;
}

}  // namespace

namespace mb {

struct Tuner::Data {
    aui_hdl hnd{nullptr};
};

Tuner::Tuner() : m_p(std::make_unique<Data>()) { ALI_EXEC(aui_nim_init(NULL)); }

Tuner::~Tuner() { aui_nim_de_init(NULL); }

bool Tuner::is_open() const {
    aui_hdl hdl{nullptr};
    return aui_find_dev_by_idx(AUI_MODULE_NIM, m_id, &hdl) == AUI_RTN_SUCCESS;
}

void Tuner::open(DVB_Mode _mode) {
    aui_nim_attr nim_attr;
    MB_ZERO(nim_attr);

    switch (_mode) {
        case DVB_Mode::DVBS:
            nim_attr.en_std = MBGUI_DVBS_ALI_MODE;
            break;

        case DVB_Mode::DVBS2:
        case DVB_Mode::DVBS2X:
            nim_attr.en_std = AUI_NIM_STD_DVBS2;
            break;

        case DVB_Mode::DVBT:
            nim_attr.en_std = AUI_NIM_STD_DVBT;
            break;

        case DVB_Mode::DVBT2:
            nim_attr.en_std = AUI_NIM_STD_DVBT2;
            break;

        case DVB_Mode::UNDEFINED:
            mb_assert(false);
            break;
    }

    mb_assert(!is_open());
    nim_attr.ul_nim_id = m_id;
    nim_attr.en_dmod_type = AUI_NIM_QPSK;
    ALI_EXEC(aui_nim_open(&nim_attr, &m_p->hnd));
}

void Tuner::close() { ALI_EXEC(aui_nim_close(m_p->hnd)); }

void Tuner::set_lnb() {}

void Tuner::blindscan(bool /*_auto*/, Frequency_t /*_start_tpuency*/,
                                            Frequency_t /*_end_tpuency*/, Polarity /*_polarity*/,
                                            bool /*_22k*/, scan_progress /*_on_progress*/,
                                            scan_done /*_on_done*/) {}

void Tuner::invoke_scan_progress(uint32_t frequency, uint32_t symbol_rate,
                                                                 uint8_t polarity, aui_nim_freq_band band,
                                                                 aui_nim_polar pol, uint8_t progress) {
    if (m_scan_process.on_progress) {
        m_scan_process.on_progress(this, frequency, symbol_rate, polarity, band,
                                                             pol, progress);
    }
}

void Tuner::invoke_scan_done() {
    if (m_scan_process.on_done) {
        m_scan_process.on_done(this);
    }
}

static int fn_aui_blindscan_callback_wrapper(
        unsigned char uc_status, unsigned char uc_polar, unsigned int u_freq,
        unsigned int u_sym, unsigned char uc_fec, void* pv_user_data,
        unsigned int u_plsn, unsigned int u_tsn, unsigned int u_stream_id) {
    // Cast user data back to Tuner instance
    if (pv_user_data != nullptr) {
        Tuner* tuner = static_cast<Tuner*>(pv_user_data);
        // Call the on_progress callback if it's set
        uint8_t start = (tuner->m_band == AUI_NIM_LOW_BAND and tuner->m_polarity == AUI_NIM_POLAR_HORIZONTAL) ? 0
                                        : (tuner->m_band == AUI_NIM_HIGH_BAND and tuner->m_polarity == AUI_NIM_POLAR_HORIZONTAL) ? 12
                                        : (tuner->m_band == AUI_NIM_LOW_BAND and tuner->m_polarity == AUI_NIM_POLAR_VERTICAL) ? 24
                                        : 36;
        auto progress = start + (12 * (u_freq - tuner->blind_scan_start_frequency) / (tuner->blind_scan_stop_frequency - tuner->blind_scan_start_frequency));
        tuner->invoke_scan_progress(u_freq, u_sym, uc_polar, tuner->m_band, tuner->m_polarity, static_cast<uint8_t>(progress));
    }
    return 0;
}

void Tuner::aui_blindscan_stop() 
{
    ALI_EXEC(aui_nim_auto_scan_stop(m_p->hnd));
}

void Tuner::aui_blindscan_start(scan_progress _on_progress, scan_done _on_done) 
{
    // Store the callbacks
    m_scan_process.on_progress = _on_progress;
    m_scan_process.on_done = _on_done;

    // Create pthread to run the scan process
    std::thread scan_thread([this]() {
        // Parâmetros de busca cega
        aui_autoscan_sat as_info;
        MB_ZERO(as_info);
        as_info.ul_start_freq = blind_scan_start_frequency;
        as_info.ul_stop_freq = blind_scan_stop_frequency;
        as_info.aui_as_cb = fn_aui_blindscan_callback_wrapper;
        as_info.pv_user_data = this;  // Pass Tuner instance as user data

        auto start_scan = [this, &as_info](aui_nim_polar polarity,
                                                                             aui_nim_freq_band band) {
            m_polarity = polarity;
            m_band = band;
            as_info.ul_polar = polarity;
            as_info.ul_freq_band = band;

            const char* pol_str =
                    (polarity == AUI_NIM_POLAR_HORIZONTAL) ? "Horizontal" : "Vertical";
            const char* band_str = (band == AUI_NIM_LOW_BAND) ? "Low" : "High";
            DEBUG(TERM_RED_BOLD << "Starting blind scan (" << pol_str << ", "
                                                    << band_str << ")\n"
                                                    << TERM_RESET);

            const auto scan_start = std::chrono::steady_clock::now();
            if (aui_nim_auto_scan_start(m_p->hnd, &as_info) != AUI_RTN_SUCCESS) {
                DEBUG("Failed to start blind scan (" << pol_str << ", " << band_str << ")\n");
            }
            const auto scan_end = std::chrono::steady_clock::now();
            const auto scan_duration = std::chrono::duration_cast<std::chrono::seconds>(scan_end - scan_start);
            DEBUG(TERM_GREEN_BOLD << "Blind scan completed in "
                                                        << scan_duration.count() << " seconds (" << pol_str << ", "
                                                        << band_str << ")\n"
                                                        << TERM_RESET);
        };

        // Execute scans for all combinations
        start_scan(AUI_NIM_POLAR_HORIZONTAL, AUI_NIM_LOW_BAND);   // 83s
        start_scan(AUI_NIM_POLAR_HORIZONTAL, AUI_NIM_HIGH_BAND);    // 10s
        start_scan(AUI_NIM_POLAR_VERTICAL, AUI_NIM_LOW_BAND);     // 90s
        start_scan(AUI_NIM_POLAR_VERTICAL, AUI_NIM_HIGH_BAND);    // 12s
        // Notify that the scan is done
        invoke_scan_done();
    });
    scan_thread.detach();
}

void Tuner::set_blindscan_params(aui_nim_polar polar,
                                 aui_nim_freq_band band)
{
    m_polarity = polar;
    m_band     = band;
}

void Tuner::get_signal_info(SignalInfo& _info) {
    aui_signal_status status;
    MB_ZERO(status);
    auto ret = aui_nim_signal_info_get(m_p->hnd, &status);

    if (AUI_RTN_SUCCESS == ret) {
        _info.signal_noise_ratio = status.ul_signal_cn;
        _info.strength = status.ul_signal_strength;
        _info.quality = status.ul_signal_quality * 10;
        _info.frequency = status.ul_freq;
        _info.symbol_rate = status.u.dvbs2.sym;
        _info.bit_error_rate = status.ul_bit_error_rate;
        _info.mode_type = from_ali_modulation_type(status.u.dvbs2.modulation);
        _info.fec_rate = from_ali_fec_rate(status.u.dvbs2.fec);
        _info.signal_type = DVB_Mode::DVBS2;

        float snr = static_cast<float>(_info.signal_noise_ratio) / 100.0f;
         //DEBUG_MSG(HAL, DEBUG, "SNR: " << std::fixed << std::setprecision(2) << snr
         //                << ", Quality: " << _info.quality/10
         //                << "%, Strength: " << (int)(_info.strength)
         //                << "%, BER: " << _info.bit_error_rate << ", "
         //                << to_str(_info.mode_type) << ", "
         //                << to_str(_info.fec_rate) << ", "
         //                << to_str(_info.signal_type) << "\n");
        if(is_locked())
        {
            calibrate_signal_info(_info);
        }
        snr = static_cast<float>(_info.signal_noise_ratio) / 100.0f;
        DEBUG_MSG(HAL, DEBUG, "SNR: " << std::fixed << std::setprecision(2) << snr
                        << ", Quality: " << _info.quality/10 
                        << "%, Strength: " << (int)(_info.strength) 
                        << "%, BER: " << _info.bit_error_rate << ", " 
                        << to_str(_info.mode_type) << ", " 
                        << to_str(_info.fec_rate) << ", " 
                        << to_str(_info.signal_type) << "\n");
    } else {
        DEBUG_MSG(HAL, ERROR, "Get signal info failed: " << dec << ret << "\n");
    }
}

void Tuner::calibrate_signal_info(SignalInfo &_info)
{
    double snr = static_cast<double>(_info.signal_noise_ratio) / 100.0;
    double quality =
        1.927 * snr * snr
        - 21.42 * snr
        + 55.55;

    quality = std::clamp(quality, 0.0, 100.0) * 10;

    _info.quality = static_cast<int>(std::round(quality));

    double strength = _info.strength * 1.30;

    if(strength > 100)
        strength = 100;

    _info.strength = (int)strength;
}

void Tuner::scan_event(ScanEvent /*_event*/, void* /*data*/) {}

void Tuner::lock(const Transponder* _tp) 
{
    unlock();
    auto config = Config::get_config();
    Satellite satellite = config->get_satellite_by_id((uint16_t)_tp->transponder_id.satellite_id());
    m_current_transponder = std::move(*_tp);
    auto freq = _tp->transponder_id.frequency() / 1000;

    auto lnbf_config = get_lnbf_config(satellite.band, satellite.type, false, freq, _tp->transponder_id.polarity());
    lock(_tp, std::move(lnbf_config));
}

void Tuner::lock(const Transponder* _tp, const LNBF_Config _lnbf_config) 
{
    DEBUG_MSG(TUNER, DEBUG, "satellite_id: " << _tp->transponder_id.satellite_id() << ", frequency: " << _tp->transponder_id.frequency() << "MHz/" << _tp->symbol_rate << "Kbps\n");
    auto freq = _tp->transponder_id.frequency() / 1000;
    aui_nim_connect_param params;
    MB_ZERO(params);
    // Satelite Source, WTF?
    params.connect_param.sat.ul_src = 0;
    params.ul_freq = _lnbf_config.l_freq;

    params.connect_param.sat.ul_polar = _tp->transponder_id.polarity() == Polarity::Horizontal ? AUI_NIM_POLAR_HORIZONTAL : 
                                        _tp->transponder_id.polarity() == Polarity::Vertical ? AUI_NIM_POLAR_VERTICAL :
                                        _tp->transponder_id.polarity() == Polarity::Left ? AUI_NIM_CPOLAR_LEFT : AUI_NIM_CPOLAR_RIGHT;
    params.connect_param.sat.ul_symrate = _tp->symbol_rate;
    params.connect_param.sat.ul_freq_band = freq < LO_5150 ? AUI_NIM_LOW_BAND : 
                             freq > KU_11700 ? AUI_NIM_HIGH_BAND : AUI_NIM_LOW_BAND;
    params.connect_param.sat.std = AUI_NIM_STD_DVBS2_AUTO;    
    params.en_demod_type = AUI_NIM_QPSK;
    ALI_EXEC(aui_nim_connect(m_p->hnd, &params));
    DEBUG_MSG(HAL, INFO, "Try lock:" << "\n\tFrequency: " << _tp->transponder_id.frequency() << "MHz"
                                     << "\n\tSymbol Rate: " << _tp->symbol_rate << "Kbps"
                                     << "\n\tPolarity: " << (_tp->transponder_id.polarity() == Polarity::Horizontal ? "Horizontal" : 
                                                        _tp->transponder_id.polarity() == Polarity::Vertical ? "Vertical" :
                                                        _tp->transponder_id.polarity() == Polarity::Left ? "Left" : "Right")
                                     << "\n\tDVB Mode: " << to_str(_tp->dvb_mode)
                                     << "\n\tLB: " << _lnbf_config.l_freq 
                                     << "\n\t22KHz tone: " << (params.connect_param.sat.ul_freq_band == AUI_NIM_HIGH_BAND ? "on" : "off") << "\n");

    // Configura porta DiseqC
    set_diseqc(_tp->satellite_id);
}

void Tuner::set_diseqc(uint16_t satellite_id)
{
    DEBUG_MSG(HAL, INFO, "Satellite Id: " << dec << (int)satellite_id << "\n");
    auto config = Config::get_config();
    auto diseqc_port = config->get_diseqc_port(satellite_id);
    DEBUG_MSG(HAL, INFO, "Diseqc Port: " << dec << (int)diseqc_port << "\n");
    if(diseqc_port == 255)
    {
        DEBUG_MSG(HAL, INFO, "No Diseqc switch found. \n");
        return;
    }

    DiseqC_Controller diseqc_controller;

    if (diseqc_controller.output(diseqc_port+1))
    {
        DEBUG_MSG(HAL, INFO, "Diseqc Port: " << dec << (int)diseqc_port << " set\n");
    }
    else
    {
        DEBUG_MSG(HAL, INFO, "Diseqc Port: " << dec << (int)diseqc_port << " failed\n");
    }
}

bool Tuner::is_locked() const
{
    if (is_open())
    {
        auto lock_status = AUI_NIM_STATUS_UNKNOWN;
        ALI_EXEC(aui_nim_is_lock(m_p->hnd, reinterpret_cast<int*>(&lock_status)));
        return lock_status == AUI_NIM_STATUS_LOCKED;
    }

    return false;
}

void Tuner::unlock()
{
    if (is_locked())
    {
        DEBUG_MSG(HAL, INFO, "Tuner " << dec << static_cast<int>(m_id) << " unlocked\n");
        ALI_EXEC(aui_nim_disconnect(m_p->hnd));
    }

    m_current_transponder = {};
}

}  // namespace mb

/*
Frequency(MHz)  SNR(db) Quality(%)  Strength(%) FEC         Converted Quality(%)    Converted strength(%)  
11740           12.28   39          65          FEC_2_3     86                      85
11780           12.28   39          64          FEC_2_3     86                      85
11820           11.92   37          64          FEC_2_3     75                      85
11860           12.17   38          63          FEC_2_3     81                      85
11900           11.09   34          62          FEC_2_3     54                      75
11940           11.60   36          62          FEC_2_3     70                      81
12000           11.67   36          53          FEC_2_3     54                      75
12120           11.58   36          57          FEC_2_3     70                      73
12460           11.46   35          58          FEC_2_3     70                      77
12480           11.73   36          50          FEC_3_5     70                      63
12580           10.84   33          57          FEC_5_6     54                      75
*/
