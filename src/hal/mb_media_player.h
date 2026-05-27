#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include "common/mb_globals.h"

namespace mb {

class Media_Player
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
        Paused,
        Pausing,
        Finish,
        Error,
    };

    enum class MP_Speed_Forward
    {
        MP_SPEED_NORMAL = 0,
        MP_SPEED_FASTFORWARD_2,
        MP_SPEED_FASTFORWARD_4,
        MP_SPEED_FASTFORWARD_8,
        MP_SPEED_FASTFORWARD_16,
        MP_SPEED_FASTFORWARD_24,
    };
    enum class MP_Speed_Rewind
    {
        MP_SPEED_NORMAL = 0,
        MP_SPEED_FASTREWIND_2,
        MP_SPEED_FASTREWIND_4,
        MP_SPEED_FASTREWIND_8,
        MP_SPEED_FASTREWIND_16,
        MP_SPEED_FASTREWIND_24,
    };

    enum class Seek_Mode
    {
        NONE,
        FORWARD,
        REWIND
    };

    enum class Player_Mode
    {
        Audio,
        Video
    };

private:
    void seek_relative(int delta_seconds);
    Seek_Mode m_seek_mode = Seek_Mode::NONE;
    Player_Mode m_player_mode = Player_Mode::Video;
    int       m_seek_step = 0;
    unsigned int cur_time = 0;
    unsigned int total_time = 0;
    std::chrono::steady_clock::time_point m_last_seek_tick{};
    static constexpr auto SEEK_TICK_INTERVAL = std::chrono::milliseconds(500);
    std::chrono::steady_clock::time_point m_seekable{};
    static constexpr auto SEEKABLE_INTERVAL = std::chrono::milliseconds(50);

protected:
    struct Data;
    std::unique_ptr<Data> m_p;

    std::atomic<State> m_state;
#if __cplusplus >= 201703L
    static_assert(std::atomic<State>::is_always_lock_free);
#endif

public:
    Media_Player();
    virtual ~Media_Player();

    void open(std::string url, Player_Mode _mode);
    void close();
    void start();
    void stop();
    void pause();
    void resume();
    void video_forward(MP_Speed_Forward mp_speed);
    void video_rewind(MP_Speed_Rewind mp_speed);
    void audio_rewind(MP_Speed_Rewind mp_speed);
    void audio_forward(MP_Speed_Forward mp_speed);
    unsigned int get_total_time();
    unsigned int get_current_time();
    void process_seek_tick();
    void stop_seek();

    State state() const
    {
        return m_state.load(std::memory_order_acquire);
    }
};

} // namespace mb
