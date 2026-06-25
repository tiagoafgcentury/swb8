#include "mb_task_tuner.h"
#include "mb_task_demux.h"
#include "mb_events.h"

#include "common/mb_assert.h"
#include "common/mb_config.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "../../../ui/lvgl/mb_osd_translate.h"

#include "hal/mb_lnb_config.h"
#include "hal/mb_tuner.h"

extern "C" {
    #include <aui_nim.h>
}

#include <thread>

using namespace std::chrono_literals;

namespace mb {

namespace {

int m_cfg_idx_snr { -1 };

// Lineup scan hits many TPs in sequence; 5s is often too short for marginal carriers (see tuner LOCK FAILED logs).
constexpr auto LINEUP_SCAN_LOCK_TIMEOUT = 15s;

static const auto CLARO_HOME_CHANNEL = Transponder
{
    Transponder_Id{12120000, Polarity::Vertical, 1},
                   29892,
                   DVB_Mode::DVBS2,
                   101,
                   Network_Id_Claro,
                   Network_Id_Claro,
                   true
};

static const auto SKY_HOME_CHANNEL = Transponder
{
    Transponder_Id{10722000, Polarity::Vertical, 2},
                   30000,
                   DVB_Mode::DVBS2,
                   24614,
                   Network_Id_Sky,
                   Network_Id_Sky,
                   true
};

}

struct Task_Tuner::LNBf_Autodetect_Ctx
{
    struct LNBF_Search_Params
    {
        Band band = Band::UNDEFINED;
        LNBF_Type lnbf_type = LNBF_Type::UNDEFINED;
        bool inverted = false;
        const Transponder *tp = nullptr;
    };

    static constexpr auto MAX_LNBF_PARAMS = 3;
    const std::array<LNBF_Search_Params, MAX_LNBF_PARAMS> LNBF_PARAMS =
    {
        LNBF_Search_Params{Band::Ku, LNBF_Type::Multi, false, &CLARO_HOME_CHANNEL},
        LNBF_Search_Params{Band::Ku, LNBF_Type::Universal, false, &CLARO_HOME_CHANNEL},
        LNBF_Search_Params{Band::Ku, LNBF_Type::Universal, false, &SKY_HOME_CHANNEL},
    };
    int m_cfg_idx { -1 };

    std::chrono::steady_clock::time_point m_deadline;
    std::weak_ptr<Event_Autodetect_LNBf> m_event;

    void set_lnbf_cfg(Task_Tuner *_this, LNBF_Type _lnbf_type);
    void set_default_tp_cfg(Task_Tuner *_this);
    void start_next_cfg(Task_Tuner *_this);
    void process(Task_Tuner *_this);
    void finish(Task_Tuner *_this);
};

struct Task_Tuner::Blind_Scan_Ctx
    : public std::enable_shared_from_this<Task_Tuner::Blind_Scan_Ctx>
{
    BlindScanStep step{BlindScanStep::Idle};
    bool scan_running{false};

    std::chrono::steady_clock::time_point m_deadline;
    std::weak_ptr<Event_Blind_Scan_Progress> m_event;

    void start(Task_Tuner*);
    void process(Task_Tuner*);
    void start_step(Task_Tuner*);
    void stop(Task_Tuner*);
    void finish(Task_Tuner*);
};


Task_Tuner::Task_Tuner():
    m_tuner(std::make_unique<Tuner>())
{
    mb_assert(s_task_tuner == nullptr);
    s_task_tuner = this;
}

Task_Tuner::~Task_Tuner()
{
    mb_assert(s_task_tuner != nullptr);
    s_task_tuner = nullptr;
}

void Task_Tuner::handle_event_transponder_lock(const Transponder *_tp, bool _force)
{
    if(not _tp)
    {
        m_tuner->close();
        m_state = State::NORMAL;
        m_tp_locked = {};
        m_tp_trying = {};
        return;
    }

    auto signalinfo = m_tuner->get_signal_info();
    auto symbol_rate = signalinfo.symbol_rate;
    auto tp_id = m_tuner->get_locked_transponder();
    if(tp_id == _tp->transponder_id and symbol_rate == _tp->symbol_rate and not _force)
    {
        post_event_transponder_locked({ .tp = m_tp_locked, .success = true, .already_locked = true });
        return;
    }

    s_task_demux->clear_table_queue();
    m_tuner->close();
    m_tuner->open(_tp->dvb_mode);

    m_state = State::LOCKING;
    m_tp_locked.set_id(0);
    m_tp_trying = _tp->transponder_id;
    m_tuner->lock(_tp);
    m_lock_started = decltype(m_lock_started)::clock::now();
}

void Task_Tuner::on_locked()
{
    m_state = State::NORMAL;
    auto _tp = m_tuner->get_locked_transponder();
    m_tp_locked = _tp.transponder_id;
    m_tp_trying.set_id(0);
    auto tp = Lineup_Mutex_Ref::get_current_lineup()->get_transponder(m_tp_locked);

    if(tp && tp->pat_version_number == INVALID_PAT_VERSION)
    {
        s_task_demux->pat_table_require();
    }
    post_event_transponder_locked({ .tp = m_tp_locked, .success = true, .already_locked = false });
}

void Task_Tuner::process()
{
    switch(m_state)
    {
        case State::NORMAL:
            return;

        case State::LOCKING:
        case State::UNLOCKED:
        {
            if(m_tuner->is_locked())
            {
                on_locked();
            }
            else
            {
                if(m_state == State::LOCKING)
                {
                    const auto lock_running_for = decltype(m_lock_started)::clock::now() - m_lock_started;
                    using namespace std::chrono;

                    const auto lock_timeout =
                        (s_task_demux && s_task_demux->state() == Task_Demux::ST_LOCKING)
                            ? LINEUP_SCAN_LOCK_TIMEOUT
                            : LOCK_TIMEOUT;

                    if(lock_timeout < lock_running_for)
                    {
                        DEBUG_MSG(TUNER, ERROR, "LOCK FAILED for: " << m_tp_trying << " after " << duration_cast<milliseconds>(lock_running_for).count() / 1000.0 << "\n");
                        post_event_transponder_locked({ .tp = m_tp_trying, .success = false, .already_locked = false });
                        m_state = State::UNLOCKED;
                    }
                }
            }

            return;
        }

        case State::LNBF_AUTODETECT:
        {
            if(m_lnbf_autodetect_ctx)
            {
                m_lnbf_autodetect_ctx->process(this);
            }
            else
            {
                m_state = State::NORMAL;
            }
        }
    }
}

std::tuple<Transponder_Id, Task_Tuner::Is_Locked> Task_Tuner::get_current_transponder()
{
    if(s_task_tuner->m_tp_locked.id())
    {
        return {s_task_tuner->m_tp_locked, true};
    }
    else
    {
        return {s_task_tuner->m_tp_trying, false};
    }
}

SignalInfo Task_Tuner::get_signal_info()
{
    return s_task_tuner->m_tuner->get_signal_info();
}

bool Task_Tuner::is_open()
{
    return s_task_tuner->m_tuner->is_open();
}

bool Task_Tuner::is_locked()
{
    return s_task_tuner->m_tuner->is_open() and s_task_tuner->m_tuner->is_locked();
}

void Task_Tuner::handle_event_autodetect_lnbf_start(std::weak_ptr<Event_Autodetect_LNBf> _event)
{
    m_state = State::LNBF_AUTODETECT;

    if(!m_tuner->is_open())
    {
        m_tuner->open(DVB_Mode::DVBS2);
    }

    m_lnbf_autodetect_ctx = std::make_shared<LNBf_Autodetect_Ctx>();
    m_lnbf_autodetect_ctx->m_event = std::move(_event);
    m_lnbf_autodetect_ctx->start_next_cfg(this);
}

void Task_Tuner::handle_event_blind_scan_progress(std::weak_ptr<Event_Blind_Scan_Progress> _event)
{
    DEBUG_MSG(TUNER, INFO, TERM_GREEN_BOLD "Blind scan progress event received\n" TERM_RESET);
    if(!m_tuner->is_open()) 
    {
        m_tuner->open(DVB_Mode::DVBS2);
    }

    if (auto event = _event.lock())
    {
        m_tuner->set_diseqc(event->sat_id);
    }
    m_blind_scan_ctx = std::make_shared<Blind_Scan_Ctx>();
    m_blind_scan_ctx->m_event = std::move(_event);
    if(auto event = m_blind_scan_ctx->m_event.lock())
    {
        event->stop_callback = [this]()
        {
            if(m_blind_scan_ctx)
            {
                m_blind_scan_ctx->stop(this);
            }
        };
    }
    m_blind_scan_ctx->start(this);
}

void Task_Tuner::LNBf_Autodetect_Ctx::start_next_cfg(Task_Tuner *_this)
{
    m_cfg_idx++;
    if(m_cfg_idx < MAX_LNBF_PARAMS)
    {
        const auto config = Config::get_config();
        const auto &cfg = LNBF_PARAMS[m_cfg_idx];
        const auto tp = cfg.tp;
        auto freq = tp->transponder_id.frequency() / 1000;
        auto lnbf_config = get_lnbf_config(cfg.band, cfg.lnbf_type, cfg.inverted, freq, tp->transponder_id.polarity());
        _this->m_tuner->lock(tp, std::move(lnbf_config));
        m_deadline = std::chrono::steady_clock::now() + 3s;

        if(auto ptr = m_event.lock())
        {
            if(m_cfg_idx == 0)
            {
                ptr->callback(Event_Autodetect_LNBf::Status::Started, 0, tp->transponder_id);
            }

            auto lnbf_ad_calc_progress = (m_cfg_idx * 100) / (MAX_LNBF_PARAMS * 100);
            ptr->callback(Event_Autodetect_LNBf::Status::Progress, lnbf_ad_calc_progress, tp->transponder_id);
            Event_Transponder_data progress;
            progress.frequency = freq;
            progress.symbol_rate = tp->symbol_rate;
            progress.polarity = tp->transponder_id.polarity() == Polarity::Horizontal ? 'H' : 'V';
            progress.satellite = config->selected_satellite_config().name;
            progress.band = cfg.band;
            progress.lnbf_type = cfg.lnbf_type;
            Task::post_event_autodetect_progress(progress);
            DEBUG_MSG(TUNER, DEBUG, tp->transponder_id.frequency() << " kHz, Polarity: " << (tp->transponder_id.polarity() == Polarity::Horizontal ? "H" : "V") << "\n");
        }
        else
        {
            // event died, so we should just stop
            finish(_this);
        }
    }
    else
    {
        finish(_this);
    }
}

void Task_Tuner::handle_event_set_lnbf_type(LNBF_Type _lnbf_type)
{
    m_state = State::NORMAL;
    if(!m_tuner->is_open())
    {
        m_tuner->open(DVB_Mode::DVBS2);
    }

    m_lnbf_autodetect_ctx = std::make_shared<LNBf_Autodetect_Ctx>();
    m_lnbf_autodetect_ctx->set_lnbf_cfg(this, _lnbf_type);
}

void Task_Tuner::handle_event_change_lnbf_type(std::function<void(const std::string &result)> _callback)
{
    m_state = State::NORMAL;
    if(!m_tuner->is_open())
    {
        m_tuner->open(DVB_Mode::DVBS2);
    }

    auto max_lnbf_params = LNBf_Autodetect_Ctx::MAX_LNBF_PARAMS;
    auto lnbf_autodetect_ctx = std::make_shared<LNBf_Autodetect_Ctx>();
    auto lnbf_params = lnbf_autodetect_ctx->LNBF_PARAMS;
    m_cfg_idx_snr = (m_cfg_idx_snr + 1) % max_lnbf_params;
    const auto config = Config::get_config();
    const auto &cfg = lnbf_params[m_cfg_idx_snr];
    const auto &tp = cfg.tp;
    config->set_lnbf_type(cfg.lnbf_type);
    auto freq = tp->transponder_id.frequency() / 1000;
    auto lnbf_config = get_lnbf_config(cfg.band, cfg.lnbf_type, cfg.inverted, freq, tp->transponder_id.polarity());
    m_tuner->lock(tp, std::move(lnbf_config));

    if ( _callback )
    {
        std::string result = cfg.tp->network_id == Network_Id_Claro ? "Star One D2" : "Sky B1";
        result += " - " + std::string(tr(__Frequencia)) + ": ";
        result += std::to_string(cfg.tp->transponder_id.frequency() / 1000) + "MHz | ";
        result += cfg.tp->transponder_id.polarity() == Polarity::Horizontal ? "H" : "V";
        result += " | " + std::to_string(cfg.tp->symbol_rate) + " kbps | ";
        result += cfg.lnbf_type == LNBF_Type::Universal ? "LNBF Universal" : "LNBF Multiponto";
        _callback( result );
    }
}

void Task_Tuner::handle_event_set_default_lnbf()
{
    m_state = State::NORMAL;

    if(!m_tuner->is_open())
    {
        m_tuner->open(DVB_Mode::DVBS2);
    }
    m_lnbf_autodetect_ctx = std::make_shared<LNBf_Autodetect_Ctx>();
    m_lnbf_autodetect_ctx->set_default_tp_cfg(this);
}

void Task_Tuner::Blind_Scan_Ctx::stop(Task_Tuner* _this)
{
    DEBUG_MSG(TUNER, INFO, "Blind scan stopped by user\n");
    _this->m_tuner->aui_blindscan_stop();
}

void Task_Tuner::Blind_Scan_Ctx::start(Task_Tuner* _this)
{
    DEBUG_MSG(TUNER, INFO, "Blind scan started\n");

    m_deadline = std::chrono::steady_clock::now() + std::chrono::minutes(10);

    std::weak_ptr<Blind_Scan_Ctx> weak_ctx = shared_from_this();

    _this->m_tuner->aui_blindscan_start(
        /* on_progress */
        [weak_event = m_event](Tuner*, uint32_t f, uint32_t sr,
                               uint8_t p, aui_nim_freq_band b,
                               aui_nim_polar pol, uint8_t prog)
        {
            if (auto e = weak_event.lock())
            {
                e->callback(Event_Blind_Scan_Progress::Status::Progress,
                            0, f, sr, p, prog, b, pol);
            }
        },

        /* on_done */
        [weak_ctx](Tuner*)
        {
            if (auto ctx = weak_ctx.lock())
            {
                ctx->finish(nullptr); // Task_Tuner será resolvido abaixo
            }
        }
    );
}


void Task_Tuner::Blind_Scan_Ctx::process(Task_Tuner* _this)
{
    if (step == BlindScanStep::Done)
    {
        finish(_this);
        return;
    }

    if (!scan_running)
    {
        start_step(_this);
        scan_running = true;
        return;
    }

    if (m_deadline < std::chrono::steady_clock::now())
    {
        DEBUG_MSG(TUNER, WARN, "Blind scan timeout\n");
        finish(_this);
    }
}

void Task_Tuner::Blind_Scan_Ctx::start_step(Task_Tuner* _this)
{
    aui_nim_polar polar;
    aui_nim_freq_band band;

    switch (step)
    {
        case BlindScanStep::H_Low:
            polar = AUI_NIM_POLAR_HORIZONTAL;
            band  = AUI_NIM_LOW_BAND;
            break;
        case BlindScanStep::H_High:
            polar = AUI_NIM_POLAR_HORIZONTAL;
            band  = AUI_NIM_HIGH_BAND;
            break;
        case BlindScanStep::V_Low:
            polar = AUI_NIM_POLAR_VERTICAL;
            band  = AUI_NIM_LOW_BAND;
            break;
        case BlindScanStep::V_High:
            polar = AUI_NIM_POLAR_VERTICAL;
            band  = AUI_NIM_HIGH_BAND;
            break;
        default:
            step = BlindScanStep::Done;
            return;
    }

    scan_running = true;

    // 👇 configura estado do tuner
    _this->m_tuner->set_blindscan_params(polar, band);

    std::weak_ptr<Blind_Scan_Ctx> weak_ctx = shared_from_this();

    _this->m_tuner->aui_blindscan_start(
        /* on_progress */
        [weak_event = m_event](Tuner*, uint32_t f, uint32_t sr,
                               uint8_t p, aui_nim_freq_band b,
                               aui_nim_polar pol, uint8_t prog)
        {
            if (auto e = weak_event.lock())
            {
                e->callback(Event_Blind_Scan_Progress::Status::Progress,
                            0, f, sr, p, prog, b, pol);
            }
        },

        /* on_done */
        [weak_ctx](Tuner*)
        {
            if (auto ctx = weak_ctx.lock())
            {
                ctx->scan_running = false;
                ctx->step = static_cast<BlindScanStep>(
                    static_cast<int>(ctx->step) + 1
                );
            }
        }
    );
}

void Task_Tuner::Blind_Scan_Ctx::finish(Task_Tuner* _this)
{
    if (auto e = m_event.lock())
    {
        e->callback(Event_Blind_Scan_Progress::Status::Success,
                    0, 0, 0, 0, 100,
                    AUI_NIM_LOW_BAND,
                    AUI_NIM_POLAR_HORIZONTAL);
    }

    if (_this)
        _this->m_blind_scan_ctx.reset();

    DEBUG_MSG(TUNER, INFO, "Blind scan finished\n");
}



void Task_Tuner::LNBf_Autodetect_Ctx::set_lnbf_cfg(Task_Tuner *_this, LNBF_Type _lnbf_type)
{
    m_cfg_idx = _lnbf_type == LNBF_Type::Universal ? 0 : 1;
    const auto config = Config::get_config();
    config->set_lnbf_type(_lnbf_type);
    const auto &cfg = LNBF_PARAMS[m_cfg_idx];
    const auto &tp = cfg.tp;
    auto freq = tp->transponder_id.frequency() / 1000;
    auto lnbf_config = get_lnbf_config(cfg.band, cfg.lnbf_type, cfg.inverted, freq, tp->transponder_id.polarity());
    _this->m_tuner->lock(tp, std::move(lnbf_config));
    m_deadline = std::chrono::steady_clock::now() + 3s;
    Event_Transponder_data progress;
    progress.frequency = freq;
    progress.symbol_rate = tp->symbol_rate;
    progress.polarity = tp->transponder_id.polarity() == Polarity::Horizontal ? 'H' : 'V';
    progress.satellite = config->selected_satellite_config().name;
    progress.band = cfg.band;
    progress.lnbf_type = cfg.lnbf_type;
    Task::post_event_autodetect_progress(progress);
    DEBUG_MSG(TUNER, DEBUG, tp->transponder_id.frequency() << " kHz, Polarity: " << (tp->transponder_id.polarity() == Polarity::Horizontal ? "H" : "V") << "\n");
}

void Task_Tuner::LNBf_Autodetect_Ctx::set_default_tp_cfg(Task_Tuner *_this)
{
    m_cfg_idx = 0;
    const auto config = Config::get_config();
    const auto &cfg = LNBF_PARAMS[m_cfg_idx];
    const auto &tp = cfg.tp;
    config->set_lnbf_type(cfg.lnbf_type);
    auto freq = tp->transponder_id.frequency() / 1000;
    auto lnbf_config = get_lnbf_config(cfg.band, cfg.lnbf_type, cfg.inverted, freq, tp->transponder_id.polarity());
    _this->m_tuner->lock(tp, std::move(lnbf_config));
    m_deadline = std::chrono::steady_clock::now() + 3s;
    Event_Transponder_data progress;
    progress.frequency = freq;
    progress.symbol_rate = tp->symbol_rate;
    progress.polarity = tp->transponder_id.polarity() == Polarity::Horizontal ? 'H' : 'V';
    progress.satellite = config->selected_satellite_config().name;
    progress.band = cfg.band;
    progress.lnbf_type = cfg.lnbf_type;
    Task::post_event_autodetect_progress(progress);
    DEBUG_MSG(TUNER, DEBUG, tp->transponder_id.frequency() << " kHz, Polarity: " << (tp->transponder_id.polarity() == Polarity::Horizontal ? "H" : "V") << "\n");
}

void Task_Tuner::LNBf_Autodetect_Ctx::process(Task_Tuner *_this)
{
    if(_this->m_tuner->is_locked())
    {
        DEBUG_MSG(TUNER, DEBUG, _this->m_tp_locked.frequency() << " kHz, Polarity: " << (_this->m_tp_locked.polarity() == Polarity::Horizontal ? "H" : "V") << "\n");
        finish(_this);
    }
    else if(m_deadline < decltype(m_deadline)::clock::now())
    {
        start_next_cfg(_this);
    }
}

void Task_Tuner::LNBf_Autodetect_Ctx::finish(Task_Tuner *_this)
{
    if(auto ptr = m_event.lock())
    {
        const auto is_locked = _this->is_locked();

        if(is_locked)
        {
            const auto &cfg = LNBF_PARAMS[m_cfg_idx];
            Event_LNBF_Params lnbf_params;
            auto network_id = cfg.tp->network_id;
            lnbf_params.band = cfg.band;
            lnbf_params.lnbf_type = cfg.lnbf_type;
            lnbf_params.inverted = false;
            auto config = Config::get_config();
            config->set_lnbf_type(cfg.lnbf_type);
            config->set_satellite_config(network_id);
            lnbf_params.sat_id = 0;
            if (network_id == Network_Id_Claro)
            {
                lnbf_params.sat_id = 1;
                Task::post_event_cas_switch_folder(false);
            }
            else if (network_id == Network_Id_Sky)
            {
                lnbf_params.sat_id = 2;
                Task::post_event_cas_switch_folder(true);
            }
            Task::post_event_lnbf_config_save(std::move(lnbf_params));
            ptr->callback(Event_Autodetect_LNBf::Status::Success, 100, cfg.tp->transponder_id);
            post_event_transponder_locked({ .tp = cfg.tp->transponder_id, .success = true, .already_locked = false });
        }
        else
        {
            ptr->callback(Event_Autodetect_LNBf::Status::Failed, 100, Transponder_Id{});
        }

        post_event_autodetect_lnbf_finished(is_locked);
        DEBUG_MSG(TUNER, WARN, "LNBf Autodetect finished with status: " << (is_locked ? TERM_GREEN_BOLD "Success" : TERM_BLUE_BOLD "Failed") << TERM_RESET << "\n"
                                "Frequency: " << _this->m_tp_locked.frequency() << "\n");
    }

    _this->m_lnbf_autodetect_ctx.reset();
    _this->m_state = State::NORMAL;
}

#ifdef MBGUI_PERIODIC_DUMP
void Task_Tuner::handle_event_debbug_dump_status()
{
    auto info = get_signal_info();
    DEBUG_MSG(TUNER, DEBUG, "Tuner: " << (is_locked() ? "Locked" : "Unlocked") << " Freq: " << dec << info.frequency << " Q: " << info.quality << " S: " << info.strength << "\n");
}

#endif // MBGUI_PERIODIC_DUMP

} //namespace mb
