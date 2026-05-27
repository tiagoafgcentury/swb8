#pragma once

#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "hal/mb_demux.h"
#include "hal/mb_tuner_signal_info.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <map>

extern "C" {
    #include <aui_nim.h>
}


namespace mb {

struct LNBF_Config;

class Tuner
{
public:
    struct iterator
    {
        size_t pos { 0 };
        const Tuner *_tuner { nullptr };

        const Transponder &operator*()
        {
            return _tuner->m_scan_process.transponders[pos];
        }

        bool operator!=(const iterator &other)
        {
            return pos != other.pos;
        }

        iterator &operator++()
        {
            pos++;
            return *this;
        }
    };

    Tuner();
    virtual ~Tuner();

    typedef std::function<void(Tuner *, uint32_t freq, uint32_t sym, uint8_t polar, aui_nim_freq_band band, aui_nim_polar pol, uint8_t progress)> scan_progress;
    typedef std::function<void(Tuner *)> scan_done;

    void blindscan_auto(scan_progress _on_progress = scan_progress(), scan_done _on_done = scan_done())
    {
        blindscan(true, 0, 0, Polarity::Horizontal, false, _on_progress, _on_done);
    }

    void blindscan_manual(uint32_t _start_frequency, uint32_t _end_frequency, Polarity _polarity, bool _22k,
                          scan_progress _on_progress = scan_progress(), scan_done _on_done = scan_done())
    {
        blindscan(false, _start_frequency, _end_frequency, _polarity, _22k, _on_progress, _on_done);
    }

    bool is_open() const;
    void open(DVB_Mode _mode);
    void close();

    void lock(const Transponder *_tp);
    void lock(const Transponder *_tp, const LNBF_Config _config);
    bool is_locked() const;
    void unlock();
    void set_diseqc(uint16_t diseqc_port);
    void aui_blindscan_start(scan_progress _on_progress = scan_progress(), scan_done _on_done = scan_done());
    void aui_blindscan_stop();
    void set_blindscan_params(aui_nim_polar polar, aui_nim_freq_band band);
    void calibrate_signal_info(SignalInfo &_info);

    iterator begin() const
    {
        return {0, this};
    }

    iterator end() const
    {
        return {m_scan_process.transponders.size(), this};
    }

    void get_signal_info(SignalInfo &_info);

    SignalInfo get_signal_info()
    {
        SignalInfo result;
        get_signal_info(result);
        return result;
    }

    Transponder get_locked_transponder() const
    {
        return m_current_transponder;
    }

    void invoke_scan_progress(uint32_t frequency, uint32_t symbol_rate, uint8_t polarity, aui_nim_freq_band band, aui_nim_polar pol, uint8_t progress);
    void invoke_scan_done();
    aui_nim_polar m_polarity;
    aui_nim_freq_band m_band;
    static constexpr auto blind_scan_start_frequency = 950;   // in kHz
    static constexpr auto blind_scan_stop_frequency = 2150;   // in kHz

private:
    struct Data;
    std::unique_ptr<Data> m_p;

    uint8_t m_id { 0 };

    Transponder m_current_transponder;

    void blindscan(bool _auto, Frequency_t _start_frequency, Frequency_t _end_frequency, Polarity _polarity, bool _22k,
                   scan_progress _on_progress, scan_done _on_done);

    void set_lnb();

    struct
    {
        scan_progress on_progress;
        scan_done on_done;
        std::vector<Transponder> transponders;
    } m_scan_process;

    enum class ScanEvent
    {
        Status,
        Progress,
        NewReseult,
        Locked,
        UnLock,
        FoundTP,
        UNDEFINED
    };

    void scan_event(ScanEvent _event, void *data);
};

} // namespace mb
