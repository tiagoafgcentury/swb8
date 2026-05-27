#include "mb_tuner.h"
#include "mb_globals.h"
#include "mb_system.h"

#include <mt_common.h>
#include <mt_unf_frontend.h>

#include <atomic>
#include <map>
#include <chrono>

#include <unistd.h>
#include <string.h>
#include <mt_error_mpi.h>

SINGULARITY_TOKEN

namespace {

mt_unf_fe_polar_t to_montage_polarity(mb::Polarity _polarity)
{
    using namespace mb;

    switch(_polarity)
    {
        case Polarity::Horizontal:
            return MT_UNF_FE_POLARIZATION_H;

        case Polarity::Vertical:
            return MT_UNF_FE_POLARIZATION_V;

        case Polarity::Left:
            return MT_UNF_FE_POLARIZATION_L;

        case Polarity::Right:
            return MT_UNF_FE_POLARIZATION_R;

        case Polarity::UNDEFINED:
            return MT_UNF_FE_POLARIZATION_BUTT;
    }

    mb_assert(false);
};

mb::Polarity from_montage_polatiry(mt_unf_fe_polar_t _polarity)
{
    using namespace mb;

    switch(_polarity)
    {
        case MT_UNF_FE_POLARIZATION_H:
            return Polarity::Horizontal;

        case MT_UNF_FE_POLARIZATION_V:
            return Polarity::Vertical;

        case MT_UNF_FE_POLARIZATION_L:
            return Polarity::Left;

        case MT_UNF_FE_POLARIZATION_R:
            return Polarity::Right;

        case MT_UNF_FE_POLARIZATION_BUTT:
        default:
            return Polarity::UNDEFINED;
    }
}

mb::Tuner *s_tuners[4] { nullptr, nullptr, nullptr, nullptr };

mb::Tuner::ScanEvent from_montage_scan_event(mt_unf_fe_blindscan_evt_t _type)
{
    using namespace mb;

    switch(_type)
    {
        case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
            return Tuner::ScanEvent::Status;

        case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:
            return Tuner::ScanEvent::Progress;

        case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
            return Tuner::ScanEvent::NewReseult;

        case MT_UNF_FE_BLINDSCAN_EVT_LOCKED:
            return Tuner::ScanEvent::Locked;

        case MT_UNF_FE_BLINDSCAN_EVT_UNLOCK:
            return Tuner::ScanEvent::UnLock;

        case MT_UNF_FE_BLINDSCAN_EVT_FINDTP:
            return Tuner::ScanEvent::FoundTP;

        case MT_UNF_FE_BLINDSCAN_EVT_BUTT:
        default:
            return Tuner::ScanEvent::UNDEFINED;
    }
}

mb::ModulationType from_montage_modulation_type(mt_unf_modulation_type_t _type)
{
    using namespace mb;

    switch(_type)
    {
        case MT_UNF_MOD_TYPE_DEFAULT:
            return ModulationType::Default;

        case MT_UNF_MOD_TYPE_QAM_4:
            return ModulationType::QAM_4;

        case MT_UNF_MOD_TYPE_QAM_4_NR:
            return ModulationType::QAM_4_NR;

        case MT_UNF_MOD_TYPE_QAM_16:
            return ModulationType::QAM_16;

        case MT_UNF_MOD_TYPE_QAM_32:
            return ModulationType::QAM_32;

        case MT_UNF_MOD_TYPE_QAM_64:
            return ModulationType::QAM_64;

        case MT_UNF_MOD_TYPE_QAM_128:
            return ModulationType::QAM_128;

        case MT_UNF_MOD_TYPE_QAM_256:
            return ModulationType::QAM_256;

        case MT_UNF_MOD_TYPE_QAM_512:
            return ModulationType::QAM_512;

        case MT_UNF_MOD_TYPE_BPSK:
            return ModulationType::BPSK;

        case MT_UNF_MOD_TYPE_QPSK:
            return ModulationType::QPSK;

        case MT_UNF_MOD_TYPE_DQPSK:
            return ModulationType::DQPSK;

        case MT_UNF_MOD_TYPE_8PSK:
            return ModulationType::PSK_8;

        case MT_UNF_MOD_TYPE_16APSK:
            return ModulationType::APSK_16;

        case MT_UNF_MOD_TYPE_32APSK:
            return ModulationType::APSK_32;

        case MT_UNF_MOD_TYPE_64APSK:
            return ModulationType::APSK_64;

        case MT_UNF_MOD_TYPE_128APSK:
            return ModulationType::APSK_128;

        case MT_UNF_MOD_TYPE_256APSK:
            return ModulationType::APSK_256;

        case MT_UNF_MOD_TYPE_8APSK_L:
            return ModulationType::APSK_L_8;

        case MT_UNF_MOD_TYPE_16APSK_L:
            return ModulationType::APSK_L_16;

        case MT_UNF_MOD_TYPE_32APSK_L:
            return ModulationType::APSK_L_32;

        case MT_UNF_MOD_TYPE_64APSK_L:
            return ModulationType::APSK_L_64;

        case MT_UNF_MOD_TYPE_128APSK_L:
            return ModulationType::APSK_L_128;

        case MT_UNF_MOD_TYPE_256APSK_L:
            return ModulationType::APSK_L_256;

        case MT_UNF_MOD_TYPE_8VSB:
            return ModulationType::VSB_8;

        case MT_UNF_MOD_TYPE_16VSB:
            return ModulationType::VSB_16;

        case MT_UNF_MOD_TYPE_AUTO:
            return ModulationType::AUTO;

        case MT_UNF_MOD_TYPE_BUTT:
            return ModulationType::UNDEFINED;

        default:
            return ModulationType::UNDEFINED;
    }
}

mb::FECRate from_montage_fec_rate(unf_fe_fecrate_t _rate)
{
    using namespace mb;

    switch(_rate)
    {
        case MT_UNF_FE_FEC_AUTO:
            return FECRate::AUTO;

        case MT_UNF_FE_FEC_1_2:
            return FECRate::FEC_1_2;

        case MT_UNF_FE_FEC_2_3:
            return FECRate::FEC_2_3;

        case MT_UNF_FE_FEC_3_4:
            return FECRate::FEC_3_4;

        case MT_UNF_FE_FEC_4_5:
            return FECRate::FEC_4_5;

        case MT_UNF_FE_FEC_5_6:
            return FECRate::FEC_5_6;

        case MT_UNF_FE_FEC_6_7:
            return FECRate::FEC_6_7;

        case MT_UNF_FE_FEC_7_8:
            return FECRate::FEC_7_8;

        case MT_UNF_FE_FEC_8_9:
            return FECRate::FEC_8_9;

        case MT_UNF_FE_FEC_9_10:
            return FECRate::FEC_9_10;

        case MT_UNF_FE_FEC_1_4:
            return FECRate::FEC_1_4;

        case MT_UNF_FE_FEC_1_3:
            return FECRate::FEC_1_3;

        case MT_UNF_FE_FEC_2_5:
            return FECRate::FEC_2_5;

        case MT_UNF_FE_FEC_3_5:
            return FECRate::FEC_3_5;

        case MT_UNF_FE_FEC_5_9:
            return FECRate::FEC_5_9;

        case MT_UNF_FE_FEC_7_9:
            return FECRate::FEC_7_9;

        case MT_UNF_FE_FEC_4_15:
            return FECRate::FEC_4_15;

        case MT_UNF_FE_FEC_7_15:
            return FECRate::FEC_7_15;

        case MT_UNF_FE_FEC_8_15:
            return FECRate::FEC_8_15;

        case MT_UNF_FE_FEC_11_15:
            return FECRate::FEC_11_15;

        case MT_UNF_FE_FEC_13_18:
            return FECRate::FEC_13_18;

        case MT_UNF_FE_FEC_9_20:
            return FECRate::FEC_9_20;

        case MT_UNF_FE_FEC_11_20:
            return FECRate::FEC_11_20;

        case MT_UNF_FE_FEC_23_36:
            return FECRate::FEC_23_36;

        case MT_UNF_FE_FEC_25_36:
            return FECRate::FEC_25_36;

        case MT_UNF_FE_FEC_11_45:
            return FECRate::FEC_11_45;

        case MT_UNF_FE_FEC_13_45:
            return FECRate::FEC_13_45;

        case MT_UNF_FE_FEC_14_45:
            return FECRate::FEC_14_45;

        case MT_UNF_FE_FEC_26_45:
            return FECRate::FEC_26_45;

        case MT_UNF_FE_FEC_28_45:
            return FECRate::FEC_28_45;

        case MT_UNF_FE_FEC_29_45:
            return FECRate::FEC_29_45;

        case MT_UNF_FE_FEC_31_45:
            return FECRate::FEC_31_45;

        case MT_UNF_FE_FEC_32_45:
            return FECRate::FEC_32_45;

        case MT_UNF_FE_FEC_77_90:
            return FECRate::FEC_77_90;

        case MT_UNF_FE_FEC_RESERVED:
        case MT_UNF_FE_FEC_UNDEF:
        case MT_UNF_FE_FECRATE_BUTT:
        default:
            return FECRate::UNDEFINED;
    }
}

mb::DVBMode from_montage_signal_type(mt_unf_fe_sig_type_t _type)
{
    using namespace mb;

    switch(_type)
    {
        case MT_UNF_FE_SIG_TYPE_SAT:
            return DVBMode::DVBS;

        case MT_UNF_FE_SIG_TYPE_SAT_2:
            return DVBMode::DVBS2;

        default:
            return DVBMode::UNDEFINED;
    }
}

void scan_notify_callback(uint32_t _tuner_id, mt_unf_fe_blindscan_evt_t msg, void *p_param)
{
    auto tuner = s_tuners[_tuner_id];
    tuner->scan_event(from_montage_scan_event(msg), p_param);
};

}

namespace mb {

#ifndef NDEBUG
std::ostream &operator<<(std::ostream &_stream, const mt_unf_fe_connect_para_t &_params)
{
    _stream << _params.sig_type << ", " << _params.connect_param.sat.freq;
    return _stream;
}


std::ostream &operator<<(std::ostream &_stream, const Transponder &_tp)
{
    _stream << _tp.transponder_id;
    return _stream;
}

#endif // NDEBUG

std::vector<Tuner::TunerDef> Tuner::list_tunners()
{
    std::vector<Tuner::TunerDef> result;
    result.emplace_back(0);
    result.emplace_back(1);
    result.emplace_back(2);
    result.emplace_back(3);
    return result;
}

struct Tuner::Data
{
    DVBMode modulation { DVBMode::UNDEFINED };
};

Tuner::Tuner(uint8_t _id):
    m_id(_id)
{
    SINGULARITY_CHECK(false);
    s_tuners[m_id] = this;
    MT_EXEC(mt_unf_fe_init());
}

Tuner::~Tuner()
{
    s_tuners[m_id] = nullptr;
    MT_EXEC(mt_unf_fe_deinit());
    SINGULARITY_CHECK(true);
}

void Tuner::open(DVBMode _mode)
{
    MT_EXEC(mt_unf_fe_open(m_id));
    mt_unf_fe_attr_t fe_attr;
    MB_ZERO(fe_attr);
    MT_EXEC(mt_unf_fe_get_default_attr(m_id, &fe_attr));

    switch(_mode)
    {
        case DVBMode::UNDEFINED:
            mb_assert(false); // Should never happen
            return;

        case DVBMode::DVBS:
        case DVBMode::DVBS2:
        case DVBMode::DVBS2X: // TODO Check this for DVBS2X
            fe_attr.sig_type        = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
            fe_attr.tuner_type      = MT_UNF_TUNER_TYPE_M88TS6011;
            fe_attr.tuner_addr      = 0x58;
            fe_attr.demod_dev_type  = MT_UNF_DEMOD_DEV_TYPE_M88CT8K;
            fe_attr.demod_addr      = 0xd0;
            fe_attr.output_mode     = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
            fe_attr.demod_i2c_id    = 0;
            fe_attr.tuner_i2c_id[0] = 0;
            break;

        case DVBMode::DVBT:
            fe_attr.sig_type        = MT_UNF_FE_SIG_TYPE_DVB_T;
            fe_attr.tuner_type      = MT_UNF_TUNER_TYPE_M88TC6800;
            fe_attr.tuner_addr      = 0xc6;
            fe_attr.demod_dev_type  = MT_UNF_DEMOD_DEV_TYPE_M88CT8K;
            fe_attr.demod_addr      = 0x18;
            fe_attr.output_mode     = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
            fe_attr.demod_i2c_id    = 0;
            fe_attr.tuner_i2c_id[0] = 0;
            break;

        case DVBMode::DVBT2:
            fe_attr.sig_type        = MT_UNF_FE_SIG_TYPE_DVB_T2;
            fe_attr.tuner_type      = MT_UNF_TUNER_TYPE_M88TC6800;
            fe_attr.tuner_addr      = 0xc6;
            fe_attr.demod_dev_type  = MT_UNF_DEMOD_DEV_TYPE_M88CT8K;
            fe_attr.demod_addr      = 0x18;
            fe_attr.output_mode     = MT_UNF_FE_OUTPUT_MODE_PARALLEL;
            fe_attr.demod_i2c_id    = 0;
            fe_attr.tuner_i2c_id[0] = 0;
            break;
    }

    MT_EXEC(mt_unf_fe_set_attr(m_id, &fe_attr));
    m_p->modulation = _mode;
    set_lnb();
    // Force HARD reset
    MT_EXEC(mt_unf_fe_switch_22k(m_id, MT_UNF_FE_SWITCH_22K_22));
    MT_EXEC(mt_unf_fe_switch_22k(m_id, MT_UNF_FE_SWITCH_22K_0));
    MT_EXEC(mt_unf_fe_switch_22k(m_id, MT_UNF_FE_SWITCH_22K_NONE));
}

void Tuner::close()
{
    if(m_modulation != DVBMode::UNDEFINED)  // Check if we are open
    {
        m_modulation = DVBMode::UNDEFINED;
        MT_EXEC(mt_unf_fe_switch_22k(m_id, MT_UNF_FE_SWITCH_22K_NONE));
        MT_EXEC(mt_unf_fe_set_lnb_power(m_id, MT_UNF_FE_LNB_POWER_OFF));
        MT_EXEC(mt_unf_fe_close(m_id));
    }
}

void Tuner::set_lnb()
{
    MT_EXEC(mt_unf_fe_set_lnb_power(m_id, MT_UNF_FE_LNB_POWER_OFF));
    mt_unf_fe_lnb_config_t lnb_config;
    MB_ZERO(lnb_config);
    lnb_config.lnb_type = MT_UNF_FE_LNB_DUAL_FREQUENCY; //enType;
    lnb_config.low_lo = 9750;                           //u32LowLOFreq;
    lnb_config.high_lo = 10600;                         //u32HighLOFreq;
    lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_KU;        //enBand;
    MT_EXEC(mt_unf_fe_set_lnb_config(m_id, &lnb_config));
    MT_EXEC(mt_unf_fe_set_lnb_power(m_id, MT_UNF_FE_LNB_POWER_ON));
}

void Tuner::blindscan(bool _auto, Frequency_t _start_tpuency, Frequency_t _end_tpuency, Polarity _polarity, bool _22k,
                      scan_progress _on_progress, scan_done _on_done)
{
    m_scan_process = {};
    m_scan_process.on_progress = _on_progress;
    m_scan_process.on_done = _on_done;
    mb_assert(m_modulation == DVBMode::DVBS || m_modulation == DVBMode::DVBS2);
    mt_unf_fe_blindscan_para_t scan_params;
    MB_ZERO(scan_params);

    if(_auto)
    {
        /* Auto */
        scan_params.mode = MT_UNF_FE_BLINDSCAN_MODE_AUTO;
        /* If your diseqc device need config polarization and 22K, you need register the callback */
        // scan_params.scan_para.sat.diseqc_set = [](mt_u32 tuner_id, mt_unf_fe_polar_t polar,
        //                                                                   mt_unf_fe_lnb_22k_t enLNB22K)
        // {
        // };
    }
    else
    {
        scan_params.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
        scan_params.scan_para.sat.polar = to_montage_polarity(_polarity);
        scan_params.scan_para.sat.lnb_22k = _22k ? MT_UNF_FE_LNB_22K_ON : MT_UNF_FE_LNB_22K_OFF; //enLNB22K;
        scan_params.scan_para.sat.start_freq = _start_tpuency * 1000;
        scan_params.scan_para.sat.stop_freq = _end_tpuency * 1000;
        scan_params.scan_para.sat.diseqc_set = MT_NULL;
    }

    scan_params.scan_para.sat.scan_notify = scan_notify_callback;
    MT_EXEC(mt_unf_fe_blindscan_start(m_id, &scan_params));
}

void Tuner::get_signal_info(Tuner::SignalInfo &_info)
{
    // Signal-noise ratio
    mt_unf_fe_get_snr(m_id, &(_info.signal_noise_ratio));
    mt_unf_fe_get_ber(m_id, &(_info.bit_error_rate));
    mt_unf_fe_get_signal_strength(m_id, &(_info.strength));
    mt_unf_fe_get_signal_quality(m_id, &(_info.quality));
    mt_unf_fe_signal_info_t info;
    MB_ZERO(info);
    mt_unf_fe_get_signal_info(m_id, &info);
    _info.frequency = info.sig_info.sat.freq;
    _info.symbol_rate = info.sig_info.sat.symbol_rate;
    _info.mode_type = from_montage_modulation_type(info.sig_info.sat.mode_type);
    _info.fec_rate = from_montage_fec_rate(info.sig_info.sat.fec_rate);
    _info.signal_type = from_montage_signal_type(info.sig_type);
}

void Tuner::scan_event(ScanEvent _event, void *data)
{
    auto punNotify { static_cast<mt_unf_fe_blindscan_notify_t *>(data) };

    switch(_event)
    {
        case ScanEvent::Status:
            if(MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
            {
                DEBUG_MSG("Scan fail" << endl);
            }
            else if((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
            {
                const auto &transponders = m_scan_process.transponders;
                uint32_t i { 0u };

                for(const auto &tp : transponders)
                {
                    DEBUG_MSG("Found: " << i++ << " TP:" << tp.transponder_id << "\tSR:" << tp.symbol_rate << "\n");
                }

                if(m_scan_process.on_done)
                {
                    m_scan_process.on_done(this);
                }
            }

            break;

        case ScanEvent::Progress:
            if(m_scan_process.on_progress)
            {
                m_scan_process.on_progress(this, *(punNotify->progress_percent));
            }

            break;

        case ScanEvent::NewReseult:
        {
            auto result = punNotify->result;
            Transponder info;
            info.transponder_id.set_frequency(result->freq, from_montage_polatiry(result->polar));
            info.symbol_rate = result->symbol_rate;
            m_scan_process.transponders.emplace_back(std::move(info));
            break;
        }

        case ScanEvent::Locked:
            break;

        case ScanEvent::UnLock:
            break;

        case ScanEvent::FoundTP:
            break;

        case ScanEvent::UNDEFINED:
            DEBUG_MSG("ScanEvent::UNDEFINED\n");
            break;
    }
}


bool Tuner::lock(const Transponder *_tp, std::chrono::milliseconds _timeout)
{
    DEBUG_MSG("Try lock:" << dec <<
              "\n\tTP: " << _tp->transponder_id <<
              "\n\tSR: " << _tp->symbol_rate <<
              "\n\tDVB Mode: " << to_str(_tp->dvb_mode) <<
              "\n" << endl
             );
    auto start { std::chrono::steady_clock::now() };
    auto elapsed { std::chrono::milliseconds(0) };
    m_current_transponder = Transponder_Id{};
    m_current_polarity = Polarity::UNDEFINED;

    while(elapsed < _timeout)
    {
        if(m_modulation != _tp->dvb_mode)
        {
            close();
            open(_tp->dvb_mode);
        }

        auto is_high_tpuency = _tp->transponder_id.frequency() >= 11700000;
        auto switch_22k { is_high_tpuency ? MT_UNF_FE_SWITCH_22K_22 : MT_UNF_FE_SWITCH_22K_0 };
        MT_EXEC(mt_unf_fe_switch_22k(m_id, switch_22k));
        mt_unf_fe_connect_para_t connect_params;
        MB_ZERO(connect_params);
        connect_params.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        connect_params.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
        connect_params.connect_param.sat.freq = _tp->transponder_id.frequency();
        connect_params.connect_param.sat.sym_rate = _tp->symbol_rate;
        connect_params.connect_param.sat.polarization = to_montage_polarity(_tp->transponder_id.polarity());
        connect_params.connect_param.sat.onoff_22k = is_high_tpuency ? 1 : 0;
        connect_params.connect_param.sat.lnb_status = 0;
        auto ret { mt_unf_fe_connect(m_id, &connect_params, _timeout.count()) };
        auto end { std::chrono::steady_clock::now() };
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        switch(ret)
        {
            case -1:
            case MT_SUCCESS:
            {
                uint loopTimes { connect_params.channel_set_info.lock_time / 10u };

                if(loopTimes < 1)
                {
                    loopTimes = 1;
                }

#ifndef NDEBUG
                mt_unf_fe_status_t last_tuner_status;
                MB_ZERO(last_tuner_status);
                last_tuner_status.lock_status = MT_UNF_FE_SIGNAL_BUTT;
                last_tuner_status.unlock_reason = MT_UNF_FE_STATE_BUTT;
#endif

                for(uint i = 0; i < loopTimes; i ++)
                {
                    mt_unf_fe_status_t tuner_status;
                    MB_ZERO(tuner_status);
                    ret = mt_unf_fe_get_status(m_id, &tuner_status);

                    if(ret == MT_SUCCESS)
                    {
                        m_current_transponder = _tp->transponder_id;

                        if(MT_UNF_FE_SIGNAL_LOCKED == tuner_status.lock_status)
                        {
#ifndef NDEBUG
                            Tuner::SignalInfo info;
                            get_signal_info(info);
                            DEBUG_MSG("\n****************** LOCKED ******************\n"
                                      << "\nSNR: " << dec << info.signal_noise_ratio
                                      << "\nBER: " << info.bit_error_rate
                                      << "\nStrength: " << info.strength
                                      << "\nQuality: " << info.quality
                                      << "\nFrequency: " << info.frequency
                                      << "\nS/R: " << info.symbol_rate
                                      << "\nModulation: " << mod_type_to_string(info.mode_type)
                                      << "\nFEC: " << fec_rate_to_string(info.fec_rate)
                                      << "\nType: " << signal_type_to_string(info.signal_type)
                                      << "\n\n"
                                     );
#endif
                            return true;
                        }

#ifndef NDEBUG

                        if(tuner_status.lock_status != last_tuner_status.lock_status ||
                                tuner_status.unlock_reason != last_tuner_status.unlock_reason)
                        {
                            DEBUG_MSG("\nTuner stattus: " << dec << tuner_status.lock_status <<
                                      "\tReason: " << tuner_status.unlock_reason);
                            last_tuner_status = std::move(tuner_status);
                        }
                        else
                        {
                            DEBUG_MSG(".");
                            std::this_thread::sleep_for(10ns);
                        }

#endif
                    }
                }

                DEBUG_MSG("\n");
            }
            break;

            default:
                DEBUG_MSG("FAILED: mt_unf_fe_connect(" << static_cast<int>(m_id) << "," << connect_params << "): " << mt_get_error_msg(ret) << endl);
        }

        DEBUG_MSG("Retry " << _tp << "\n");
        std::this_thread::sleep_for(10ns);
    }

    DEBUG_MSG("Tuner " << static_cast<int>(m_id) << " failed " << _tp << "\n");
    return false;
}

const char *mod_type_to_string(ModulationType _type)
{
    switch(_type)
    {
        case ModulationType::Default:
            return "Default";

        case ModulationType::QAM_4:
            return "QAM_4";

        case ModulationType::QAM_4_NR:
            return "QAM_4_NR";

        case ModulationType::QAM_16:
            return "QAM_16";

        case ModulationType::QAM_32:
            return "QAM_32";

        case ModulationType::QAM_64:
            return "QAM_64";

        case ModulationType::QAM_128:
            return "QAM_128";

        case ModulationType::QAM_256:
            return "QAM_256";

        case ModulationType::QAM_512:
            return "QAM_512";

        case ModulationType::BPSK:
            return "BPSK";

        case ModulationType::QPSK:
            return "QPSK";

        case ModulationType::DQPSK:
            return "DQPSK";

        case ModulationType::PSK_8:
            return "PSK_8";

        case ModulationType::APSK_16:
            return "APSK_16";

        case ModulationType::APSK_32:
            return "APSK_32";

        case ModulationType::APSK_64:
            return "APSK_64";

        case ModulationType::APSK_128:
            return "APSK_128";

        case ModulationType::APSK_256:
            return "APSK_256";

        case ModulationType::APSK_L_8:
            return "APSK_L_8";

        case ModulationType::APSK_L_16:
            return "APSK_L_16";

        case ModulationType::APSK_L_32:
            return "APSK_L_32";

        case ModulationType::APSK_L_64:
            return "APSK_L_64";

        case ModulationType::APSK_L_128:
            return "APSK_L_128";

        case ModulationType::APSK_L_256:
            return "APSK_L_256";

        case ModulationType::VSB_8:
            return "VSB_8";

        case ModulationType::VSB_16:
            return "VSB_16";

        case ModulationType::AUTO:
            return "AUTO";

        case ModulationType::UNDEFINED:
        default:
            return "Undefined";
    }
}

const char *signal_type_to_string(DVBMode _type)
{
    switch(_type)
    {
        case DVBMode::DVBS:
            return "DVBS";

        case DVBMode::DVBS2:
            return "DVBS2";

        case DVBMode::DVBS2X:
            return "DVBS2X";

        default:
            return "UNDEFINED";
    }
}

const char *fec_rate_to_string(FECRate _fec_rate)
{
    switch(_fec_rate)
    {
        case FECRate::AUTO:
            return "AUTO";

        case FECRate::FEC_1_2:
            return "FEC_1_2";

        case FECRate::FEC_2_3:
            return "FEC_2_3";

        case FECRate::FEC_3_4:
            return "FEC_3_4";

        case FECRate::FEC_4_5:
            return "FEC_4_5";

        case FECRate::FEC_5_6:
            return "FEC_5_6";

        case FECRate::FEC_6_7:
            return "FEC_6_7";

        case FECRate::FEC_7_8:
            return "FEC_7_8";

        case FECRate::FEC_8_9:
            return "FEC_8_9";

        case FECRate::FEC_9_10:
            return "FEC_9_10";

        case FECRate::FEC_1_4:
            return "FEC_1_4";

        case FECRate::FEC_1_3:
            return "FEC_1_3";

        case FECRate::FEC_2_5:
            return "FEC_2_5";

        case FECRate::FEC_3_5:
            return "FEC_3_5";

        case FECRate::FEC_5_9:
            return "FEC_5_9";

        case FECRate::FEC_7_9:
            return "FEC_7_9";

        case FECRate::FEC_4_15:
            return "FEC_4_15";

        case FECRate::FEC_7_15:
            return "FEC_7_15";

        case FECRate::FEC_8_15:
            return "FEC_8_15";

        case FECRate::FEC_11_15:
            return "FEC_11_15";

        case FECRate::FEC_13_18:
            return "FEC_13_18";

        case FECRate::FEC_9_20:
            return "FEC_9_20";

        case FECRate::FEC_11_20:
            return "FEC_11_20";

        case FECRate::FEC_23_36:
            return "FEC_23_36";

        case FECRate::FEC_25_36:
            return "FEC_25_36";

        case FECRate::FEC_11_45:
            return "FEC_11_45";

        case FECRate::FEC_13_45:
            return "FEC_13_45";

        case FECRate::FEC_14_45:
            return "FEC_14_45";

        case FECRate::FEC_26_45:
            return "FEC_26_45";

        case FECRate::FEC_28_45:
            return "FEC_28_45";

        case FECRate::FEC_29_45:
            return "FEC_29_45";

        case FECRate::FEC_31_45:
            return "FEC_31_45";

        case FECRate::FEC_32_45:
            return "FEC_32_45";

        case FECRate::FEC_77_90:
            return "FEC_77_90";

        default:
            return "Unknown";
    }
}

const char *polarity_to_string(Polarity _type)
{
    switch(_type)
    {
        case Polarity::Horizontal:
            return "Horizontal";

        case Polarity::Vertical:
            return "Vertical";

        case Polarity::Left:
            return "Left";

        case Polarity::Right:
            return "Right";

        case Polarity::UNDEFINED:
        default:
            return "Undefined";
    }
}

} // namespace mb
