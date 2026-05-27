#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include "../../common/mb_globals.h"

#include "aui_common.h"
#include "aui_common_list.h"
#include "aui_pvr.h"

// least common multiple of 188 and 1024
#define TS_DEFAULT_BLOCK_SIZE (47 * 1024)

namespace mb {

struct ACS_Reencrypt;

class PVR_Cas
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

    enum class Function_State
    {
        None,
        Record,
        Play,
        Timeshift,
    };

    enum class PVR_Speed_Forward
    {
        PVR_SPEED_NORMAL = 0,
        PVR_SPEED_FASTFORWARD_2,
        PVR_SPEED_FASTFORWARD_4,
        PVR_SPEED_FASTFORWARD_8,
        PVR_SPEED_FASTFORWARD_16,
        PVR_SPEED_FASTFORWARD_32,
    };
    enum class PVR_Speed_Rewind
    {
        PVR_SPEED_NORMAL = 0,
        PVR_SPEED_FASTREWIND_2,
        PVR_SPEED_FASTREWIND_4,
        PVR_SPEED_FASTREWIND_8,
        PVR_SPEED_FASTREWIND_16,
        PVR_SPEED_FASTREWIND_32,
    };

    enum class PVR_Record_Mode
    {
        PVR_REC_AND_TMS_DISK = 0,
        PVR_REC_ONLY_DISK = 1,
        PVR_TMS_ONLY_DISK = 2,
    };

    struct pvr_record_param
    {
        uint16_t dmx_id;
        uint16_t video_pid;
        uint16_t video_type;
        uint16_t audio_pid_count;
        uint16_t audio_pid[AUI_MAX_PVR_AUDIO_PID];
        uint16_t *audio_desc_pid;
        uint16_t audio_type[AUI_MAX_PVR_AUDIO_PID];
        uint16_t pcr_pid;
        uint16_t encrypt_type;
        uint16_t is_ca_mode;
        std::string filename;
        uint16_t do_decrypt;
        uint16_t key_fixed;
        uint16_t record_flow; // TS or DSS
        aui_pvr_rec_mode rec_mode;
    };

    struct pvr_playback_param
    {
        std::string filename;
        uint16_t record_index;
        uint16_t start_mode;
        uint16_t encrypt_type;
        uint16_t key_fixed;
        uint16_t ply_flow; // TS or DSS
    };

private:

    static constexpr auto pvr_buffer_len = 1572864; //((1 * 1024 * 1024) + (512 * 1024));
    std::array<uint8_t, pvr_buffer_len> pvr_buffer;
    static constexpr auto max_record_number = 2;
    static constexpr auto max_play_number = 1;
    static constexpr auto pvr_flow_ts = 0;
    static constexpr auto pvr_flow_dss = 1;
    static constexpr auto pvr_fixed_key_flag = 1; //(1 << 0)
    static constexpr auto pvr_dss_reen_flag  = 4; //(4 << 0)
    static constexpr auto pvr_acs_reencrypt_flow = 1; //(1 << 0)
    static constexpr auto pvr_ird_reencrypt_flow = 2; //(1 << 1)

    int ff_speed    = AUI_PLAYER_SPEED_FASTFORWARD_2;
    int fb_speed    = AUI_PLAYER_SPEED_FASTREWIND_1;
    int slow_speed  = AUI_PLAYER_SPEED_SLOWFORWARD_2;

    aui_pvr_init_param ini_param;

    std::string m_mount_point = "";
    std::string m_filesystem_type = "";
    std::string m_record_filename = "";

    static void pvr_evnt_callback(aui_hdl handle, aui_evnt_pvr msg_type, unsigned int msg_code, void *user_data);
    static void pvr_callback(aui_hdl handle, aui_evnt_pvr msg_type, unsigned int msg_code, void *user_data);
    unsigned long pvr_record_open(pvr_record_param *param);
    void pvr_get_disk_size(std::string disk_name);

protected:
    struct Data;
    std::unique_ptr<Data> m_p;

    std::atomic<State> m_state;
    std::atomic<Function_State> m_function_state;

#if __cplusplus >= 201703L
    static_assert(std::atomic<State>::is_always_lock_free);
    static_assert(std::atomic<Function_State>::is_always_lock_free);
#endif

private:
    std::unique_ptr<ACS_Reencrypt> m_acs_reencrypt;

public:
    PVR_Cas();
    virtual ~PVR_Cas();

    int pvr_init();
    void pvr_deinit();

    std::string search_pvr_mount_point();
    AUI_RTN_CODE pvr_attach(std::string disk_name, PVR_Record_Mode disk_mode);
    unsigned long pvr_record_close();
    void pvr_record_pause();
    void pvr_record_resume();
    unsigned int get_pvr_record_current_time();
    std::string pvr_record_filename();
    unsigned long pvr_record_desc(pvr_record_param *record_param);
    unsigned long pvr_record(pvr_record_param *record_param);
    unsigned long pvr_record_ts(pvr_record_param *record_param);
    unsigned long pvr_record_dss(pvr_record_param *record_param);
    unsigned int get_pvr_timeshift_rec_current_time();
    unsigned int get_pvr_timeshift_play_current_time();
    bool pvr_play_open(pvr_playback_param *param);
    bool pvr_play_play();
    bool pvr_play_pause();
    bool pvr_play_rewind(PVR_Speed_Rewind _pvr_speed);
    bool pvr_play_forward(PVR_Speed_Forward _pvr_speed);
    bool pvr_play_close();
    void pvr_clear_tms();
    unsigned int get_pvr_player_total_time();
    unsigned int get_pvr_player_current_time();
    void pvr_audio_get(aui_pvr_pid_info *aui_pid_info);
    void pvr_audio_set(aui_pvr_pid_info aui_pid_info, uint8_t aud_idx);
    std::string pvr_mount_point();
    std::string pvr_filesystem_type();

#if 0

    void pvr_record_ts();
    void pvr_record_desc();
    void pvr_record_dss();
    void pvr_playback_ts();
    void pvr_playback_dss();
    void pvr_timeshift();
    void pvr_set_disk_threshold();
    void pvr_get_disk_size();
    void pvr_final_resolve();
    void pvr_offline_save();
    void pvr_offline_clear_save();
    void pvr_set_label_mode_unitest();
#endif
    auto state() const
    {
        return m_state.load(std::memory_order_acquire);
    }

    auto function_state() const
    {
        return m_function_state.load(std::memory_order_acquire);
    }

    void set_function_state(Function_State _f_state)
    {
        m_function_state.store(_f_state, std::memory_order_release);
    }
};

} // namespace mb
