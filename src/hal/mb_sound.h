#pragma once

#include "common/mb_types.h"

#include <atomic>
#include <memory>
#include <chrono>

namespace mb {

class Sound
{
public:
    enum class State
    {
        Idle,
        Opened,
        Closing,
        Starting,
        Started,
        Stopping,
        Stopped,
        Error,
    };

protected:
    struct Data;
    std::unique_ptr<Data> m_p;

    std::atomic<State> m_state;
#if __cplusplus >= 201703L
    static_assert(std::atomic<State>::is_always_lock_free);
#endif

public:
    Sound();
    virtual ~Sound();

    static Sound *get_instance();

    void increment_volume(int _volume);
    void set_volume(Volume_t _volume);
    Volume_t get_volume();
    bool mute_toggle();
    bool mute_state();
    void set_tone(double snr);
    void stop_tone();
};

} // namespace mb
