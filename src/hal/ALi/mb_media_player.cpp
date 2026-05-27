#include "../mb_media_player.h"

#include "mb_ali_globals.h"

#include "aui_common.h"
#include "aui_common_list.h"
#include "aui_mp.h"

#include <aui_osd.h>
#include <aui_deca.h>
#include <aui_decv.h>
#include <aui_snd.h>
#include <aui_tsg.h>

#include <atomic>
#include <unistd.h>

#include <thread>
#include <chrono>
using namespace std::chrono_literals;

namespace mb {

struct Media_Player::Data
{
    aui_attr_mp attr_mp;
    aui_hdl     mp_hdl;

    Data()
    {
        clear();
    }

    void clear()
    {
        MB_ZERO(attr_mp);
        MB_ZERO(mp_hdl);
    }

    static void  media_player_cb(enum aui_mp_message msg, void *data, void *user_data)
    {
        auto thiz = static_cast<Media_Player *>(user_data);

        switch(msg)
        {
            case AUI_MP_PLAY_BEGIN:
            {
                DEBUG_MSG(HAL, DEBUG, "AUI_MP_PLAY_BEGIN\n");
                break;
            }

            case AUI_MP_PLAY_END:
            {
                thiz->m_state.store(Media_Player::State::Finish, std::memory_order_release);
                DEBUG_MSG(HAL, DEBUG, "AUI_MP_PLAY_END\n");
                break;
            }

            case AUI_MP_VIDEO_CODEC_NOT_SUPPORT:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_VIDEO_CODEC_NOT_SUPPORT\n");
                break;
            }

            case AUI_MP_AUDIO_CODEC_NOT_SUPPORT:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_AUDIO_CODEC_NOT_SUPPORT\n");
                break;
            }

            case AUI_MP_RESOLUTION_NOT_SUPPORT:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_RESOLUTION_NOT_SUPPORT\n");
                break;
            }

            case AUI_MP_FRAMERATE_NOT_SUPPORT:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_FRAMERATE_NOT_SUPPORT\n");
                break;
            }

            case AUI_MP_NO_MEMORY:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_NO_MEMORY\n");
                break;
            }

            case AUI_MP_DECODE_ERROR:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_DECODE_ERROR\n");
                break;
            }

            case AUI_MP_ERROR_UNKNOWN:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_ERROR_UNKNOWN\n" << (char *)data << "\n");
                break;
            }

            case AUI_MP_BUFFERING:
            {
                DEBUG_MSG(HAL, DEBUG, "AUI_MP_BUFFERING\n");
                break;
            }

            case AUI_MP_ERROR_SOUPHTTP:
            {
                DEBUG_MSG(HAL, ERROR, "AUI_MP_ERROR_SOUPHTTP\n");
                break;
            }

            case AUI_MP_FRAME_CAPTURE:
            {
                char path[128];
                unsigned int h, w;
                sscanf((char *)data, "%[^;];h=%u,w=%u", path, &h, &w);
                DEBUG_MSG(HAL, DEBUG, "frame captured path " << path << "\n");
                DEBUG_MSG(HAL, DEBUG, "frame width= " << w << ", height= " << h  << "\n");
                DEBUG_MSG(HAL, DEBUG, "AUI_MP_FRAME_CAPTURE\n");
                break;
            }

            default:
                DEBUG_MSG(HAL, ERROR, "unkown callback message: " << msg << "\n");
                break;
        }
    }

    static void check_aui_dev_close()
    {
        aui_hdl hdl = NULL;

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &hdl))
        {
            //aui_deca_close(hdl);
            hdl = NULL;
        }

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_SND, 0, &hdl))
        {
            //aui_snd_close(hdl);
            hdl = NULL;
        }

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &hdl))
        {
            aui_decv_close(NULL, &hdl);
            hdl = NULL;
        }

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_TSG, 0, &hdl))
        {
            //aui_tsg_close (hdl);
            hdl = NULL;
        }

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DIS, 0, &hdl))
        {
            //aui_attr_dis attr_dis;
            //MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));
            //aui_dis_close (&attr_dis, &hdl);
            hdl = NULL;
        }

        if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_GFX, AUI_OSD_LAYER_GMA1, &hdl))
        {
            //aui_gfx_layer_close(hdl);
            hdl = NULL;
        }
    }
};

Media_Player::Media_Player():
    m_state(State::Idle)
{
}

Media_Player::~Media_Player()
{
    stop();
    close();
}

void Media_Player::open(std::string url, Player_Mode _mode)
{
    m_p = std::make_unique<Data>();
    m_p->clear();
    cur_time = 0;
    total_time = 0;
    m_player_mode = _mode;
    strcpy((char *)m_p->attr_mp.uc_file_name, url.c_str());
    DEBUG_MSG(HAL, DEBUG, "will play the file: " << m_p->attr_mp.uc_file_name << "\n");
    m_p->attr_mp.start_time        = 0;
    m_p->attr_mp.aui_mp_stream_cb  = Media_Player::Data::media_player_cb;
    m_p->attr_mp.user_data         = this;
    ALI_EXEC(aui_mp_open(&m_p->attr_mp, &m_p->mp_hdl));
    ALI_EXEC(aui_mp_set_buffering_time(m_p->mp_hdl, 10, 5, 30));
    ALI_EXEC(aui_mp_set_start2play_percent(m_p->mp_hdl, 0));
    m_state.store(State::Opened, std::memory_order_release);
}

void Media_Player::close()
{
    m_state.store(State::Closing, std::memory_order_release);

    if(m_p)
    {
        ALI_EXEC(aui_mp_set(m_p->mp_hdl, AUI_MP_SET_FD_CB_UNREG, NULL));
        ALI_EXEC(aui_mp_close(NULL, &m_p->mp_hdl));
        Media_Player::Data::check_aui_dev_close();
        m_p.reset();
    }

    m_state.store(State::Idle, std::memory_order_relaxed);
}

void Media_Player::start()
{
    mb_assert(m_p);
    mb_assert(state() == State::Opened or state() == State::Stopped);

    if(m_p)
    {
        m_state.store(State::Starting, std::memory_order_release);
        stop_seek();
        ALI_EXEC(aui_mp_start(m_p->mp_hdl));
        ALI_EXEC(aui_mp_speed_set(m_p->mp_hdl, AUI_MP_SPEED_1));
        //workaround to play without problems
        ALI_EXEC(aui_mp_pause(m_p->mp_hdl));
        ALI_EXEC(aui_mp_resume(m_p->mp_hdl));
        m_state.store(State::Started, std::memory_order_relaxed);
    }
}

void Media_Player::stop()
{
    if(m_p)
    {
        m_state.store(State::Stopping, std::memory_order_release);
        stop_seek();
        ALI_EXEC(aui_mp_stop(m_p->mp_hdl));
        m_state.store(State::Stopped, std::memory_order_relaxed);
    }
}

void Media_Player::pause()
{
    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Started == state or State::Starting == state or State::Error == state))
    {
        m_state.store(State::Pausing, std::memory_order_release);
        if(m_player_mode == Player_Mode::Audio)
        {
            stop_seek();
        }
        ALI_EXEC(aui_mp_pause(m_p->mp_hdl));
        m_state.store(State::Paused, std::memory_order_relaxed);
    }
}

void Media_Player::resume()
{
    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Paused == state or State::Pausing == state or State::Error == state))
    {
        if(m_player_mode == Player_Mode::Video)
        {
            m_state.store(State::Paused, std::memory_order_release);
        }
        stop_seek();
        ALI_EXEC(aui_mp_resume(m_p->mp_hdl));
        ALI_EXEC(aui_mp_speed_set(m_p->mp_hdl, AUI_MP_SPEED_1));
        m_state.store(State::Started, std::memory_order_relaxed);
    }
}

void Media_Player::video_forward(MP_Speed_Forward mp_speed)
{
    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Started == state or State::Starting == state or State::Error == state))
    {
        aui_mp_speed _mp_speed;

        switch(mp_speed)
        {
            case  MP_Speed_Forward::MP_SPEED_FASTFORWARD_2:
                _mp_speed = AUI_MP_SPEED_FASTFORWARD_2;
                break;

            case MP_Speed_Forward::MP_SPEED_FASTFORWARD_4:
                _mp_speed = AUI_MP_SPEED_FASTFORWARD_4;
                break;

            case MP_Speed_Forward::MP_SPEED_FASTFORWARD_8:
                _mp_speed = AUI_MP_SPEED_FASTFORWARD_8;
                break;

            case MP_Speed_Forward::MP_SPEED_FASTFORWARD_16:
                _mp_speed = AUI_MP_SPEED_FASTFORWARD_16;
                break;

            case MP_Speed_Forward::MP_SPEED_FASTFORWARD_24:
                _mp_speed = AUI_MP_SPEED_FASTFORWARD_24;
                break;

            case MP_Speed_Forward::MP_SPEED_NORMAL:
            default:
                _mp_speed = AUI_MP_SPEED_1;
                break;
        }

        ALI_EXEC(aui_mp_speed_set(m_p->mp_hdl, _mp_speed));
    }
}

void Media_Player::video_rewind(MP_Speed_Rewind mp_speed)
{
    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Started == state or State::Starting == state or State::Error == state))
    {
        aui_mp_speed _mp_speed;

        switch(mp_speed)
        {
            case  MP_Speed_Rewind::MP_SPEED_FASTREWIND_2:
                _mp_speed = AUI_MP_SPEED_FASTREWIND_2;
                break;

            case MP_Speed_Rewind::MP_SPEED_FASTREWIND_4:
                _mp_speed = AUI_MP_SPEED_FASTREWIND_4;
                break;

            case MP_Speed_Rewind::MP_SPEED_FASTREWIND_8:
                _mp_speed = AUI_MP_SPEED_FASTREWIND_8;
                break;

            case MP_Speed_Rewind::MP_SPEED_FASTREWIND_16:
                _mp_speed = AUI_MP_SPEED_FASTREWIND_16;
                break;

            case MP_Speed_Rewind::MP_SPEED_FASTREWIND_24:
                _mp_speed = AUI_MP_SPEED_FASTREWIND_24;
                break;

            case MP_Speed_Rewind::MP_SPEED_NORMAL:
            default:
                _mp_speed = AUI_MP_SPEED_1;
                break;
        }

        ALI_EXEC(aui_mp_speed_set(m_p->mp_hdl, _mp_speed));
    }
}

unsigned int Media_Player::get_current_time()
{
    if(m_p)
    {
        auto state = m_state.load(std::memory_order_acquire);
        if(state == State::Started)
        {
            if (m_seek_mode == Seek_Mode::NONE)
            {
                unsigned int time_s = 0;
                if (aui_mp_cur_time_get(m_p->mp_hdl, &time_s) == AUI_RTN_SUCCESS)
                {
                    cur_time = time_s;
                }
            }
        }
    }

    return cur_time;
}

unsigned int Media_Player::get_total_time()
{
    if(m_p)
    {
        auto state = m_state.load(std::memory_order_acquire);
        if(state == State::Started)
        {
            unsigned int time_s = 0;
            if (aui_mp_total_time_get(m_p->mp_hdl, &time_s) == AUI_RTN_SUCCESS)
            {
                total_time = time_s;
            }
        }
    }

    return total_time;
}

void Media_Player::seek_relative(int delta_seconds)
{
    if (!m_p)
        return;

    auto state = m_state.load(std::memory_order_acquire);
    if (state != State::Started && state != State::Paused)
        return;

    int new_time = static_cast<int>(cur_time) + delta_seconds;

    if (new_time < 0)
        new_time = 0;

    if (total_time > 0 && new_time > static_cast<int>(total_time))
    {
        new_time = total_time;
    }

    cur_time = static_cast<unsigned int>(new_time);

#if 0
    unsigned int cur_time_seek = 0;
    unsigned int total_time = 0;

    ALI_EXEC(aui_mp_cur_time_get(m_p->mp_hdl, &cur_time_seek));
    ALI_EXEC(aui_mp_total_time_get(m_p->mp_hdl, &total_time));

    int new_time = static_cast<int>(cur_time_seek) + delta_seconds;

    if (new_time < 0)
        new_time = 0;

    if (total_time > 0 && new_time > static_cast<int>(total_time))
        new_time = total_time;
#endif
    DEBUG_MSG(HAL, DEBUG, "MP seek: " << cur_time << "s -> " << total_time << "s\n");

    int percent = 0;
    ALI_EXEC(aui_mp_is_seekable(m_p->mp_hdl, &percent));

    auto now = std::chrono::steady_clock::now();
    if (now - m_seekable < SEEKABLE_INTERVAL)
        return;

    m_seekable = now;

    if (percent != -1)
    {
        ALI_EXEC(aui_mp_seek(m_p->mp_hdl, cur_time * 1000));
    }
}

void Media_Player::audio_forward(MP_Speed_Forward mp_speed)
{

    if (m_seek_mode == Seek_Mode::NONE && m_p)
    {
        unsigned int time_s = 0;
        if (aui_mp_cur_time_get(m_p->mp_hdl, &time_s) == AUI_RTN_SUCCESS)
        {
            cur_time = time_s;
        }
    }

    m_seek_mode = Seek_Mode::FORWARD;

    switch(mp_speed)
    {
        case MP_Speed_Forward::MP_SPEED_NORMAL: m_seek_step = 0; m_seek_mode = Seek_Mode::NONE; break;
        case MP_Speed_Forward::MP_SPEED_FASTFORWARD_2:  m_seek_step = 2;  break;
        case MP_Speed_Forward::MP_SPEED_FASTFORWARD_4:  m_seek_step = 4;  break;
        case MP_Speed_Forward::MP_SPEED_FASTFORWARD_8:  m_seek_step = 8;  break;
        case MP_Speed_Forward::MP_SPEED_FASTFORWARD_16: m_seek_step = 16; break;
        case MP_Speed_Forward::MP_SPEED_FASTFORWARD_24: m_seek_step = 24; break;
    }
}


void Media_Player::audio_rewind(MP_Speed_Rewind mp_speed)
{

    if (m_seek_mode == Seek_Mode::NONE && m_p)
    {
        unsigned int time_s = 0;
        if (aui_mp_cur_time_get(m_p->mp_hdl, &time_s) == AUI_RTN_SUCCESS)
        {
            cur_time = time_s;
        }
    }

    m_seek_mode = Seek_Mode::REWIND;

    switch(mp_speed)
    {
        case MP_Speed_Rewind::MP_SPEED_NORMAL: m_seek_step = 0; m_seek_mode = Seek_Mode::NONE; break;
        case MP_Speed_Rewind::MP_SPEED_FASTREWIND_2:  m_seek_step = -2;  break;
        case MP_Speed_Rewind::MP_SPEED_FASTREWIND_4:  m_seek_step = -4;  break;
        case MP_Speed_Rewind::MP_SPEED_FASTREWIND_8:  m_seek_step = -8;  break;
        case MP_Speed_Rewind::MP_SPEED_FASTREWIND_16: m_seek_step = -16; break;
        case MP_Speed_Rewind::MP_SPEED_FASTREWIND_24: m_seek_step = -24; break;
    }
}

void Media_Player::process_seek_tick()
{

    if (m_seek_mode == Seek_Mode::NONE)
        return;

    if(m_player_mode == Player_Mode::Audio)
    {
        auto now = std::chrono::steady_clock::now();

        if (now - m_last_seek_tick < SEEK_TICK_INTERVAL)
            return;

        m_last_seek_tick = now;

        seek_relative(m_seek_step);
    }
}


void Media_Player::stop_seek()
{
    if(m_player_mode == Player_Mode::Audio)
    {
        m_seek_mode = Seek_Mode::NONE;
        m_seek_step = 0;
    }
    else
    {
        ALI_EXEC(aui_mp_resume(m_p->mp_hdl));
        ALI_EXEC(aui_mp_seek(m_p->mp_hdl, cur_time * 1000));
    }
}

} // namespace mb
