#pragma once

#include "hal/mb_remote_control_keys.h"
#include "common/mb_types.h"
#include "common/mb_globals.h"
#include "common/static_string.h"
#include "common/mb_lineup.h"

#include "aui_pvr.h"
extern "C" {
#include <aui_nim.h>
}

#include <chrono>
#include <functional>
#include <memory>

namespace mb {

class Lineup;

struct Event_Remote_Control
{
    Remote_Control_Key key;
};

struct Event_Save_Application_State
{
    bool got_state { false };
    Viewer_Channel_t current_channel { 0 };
    Volume_t volume { 0 };
    bool mute { false };
    bool stand_by { false };
    Channel_List_Type channel_list_type { 0 };
    uint16_t current_satellite_id { 0 };
};

struct Event_Autodetect_LNBf
{
    ~Event_Autodetect_LNBf()
    {
        DEBUG_MSG(EVENTS, DEBUG, "Event_Autodetect_LNBf\n");
    }

    enum class Status
    {
        Started,
        Progress,
        Success,
        Failed,
    };

    std::function<void(Status, int, Transponder_Id tp)> callback;
};


struct Event_Blind_Scan_Progress
{
    uint32_t sat_id { 0 };
    uint32_t frequency_start { 0 };
    uint32_t frequency_end { 0 };
    uint32_t frequency_current { 0 };
    uint8_t progress_percent { 0 };

    enum class Status
    {
        Started,
        Progress,
        Success,
        Failed,
    };

    std::function <void(Status, uint32_t, uint32_t, uint32_t, uint32_t, uint8_t, aui_nim_freq_band, aui_nim_polar)> callback;
    std::function<void()> stop_callback;

    void stop()
    {
        if(stop_callback)
        {
            stop_callback();
        }
    }
};

struct Event_CC_Line
{
    uint8_t x { 0 };
    uint8_t y { 0 };
    static_string text;
};

struct Event_CC
{
    std::array<Event_CC_Line, MBGUI_CC_MAX_LINES> lines;
};

struct Event_Subtitle_Image
{
    uint64_t pts;
    std::vector<uint8_t> data;
};

struct Event_Time
{
    uint16_t year { 0 };
    uint8_t month { 0 };
    uint8_t day { 0 };
    uint8_t hour { 0 };
    uint8_t minute { 0 };
    uint8_t second { 0 };
};

struct Event_Transponder_data
{
    int frequency {};
    int symbol_rate {};
    char polarity {};
    Static_String satellite;
    Band band { Band::Ku };
    LNBF_Type lnbf_type { LNBF_Type::Universal };
};

struct Event_OTA_DSI
{
    std::function<void(uint32_t, uint16_t, uint16_t)> callback;
};

struct Event_Lineup_Ready
{
    Lineup_Origin origin;
    std::chrono::milliseconds duration = {};
    bool restart = true;
};

struct Event_System_Settings
{
    Timezone_Mode time_zone { Timezone_Mode::Brasilia_UTC_3 };
    Clock_Type clock_status { Clock_Type::Auto };
    Language_Mode language_mode { Language_Mode::Portugues };
    Resolution_Standard resolution { Resolution_Standard::_1080i_30Hz };
    Color_Standard color_standard { Color_Standard::PAL_M_60 };
    Aspect_Mode aspect_mode { Aspect_Mode::AUTO };
};

struct Event_Tuner_Lock
{
    Transponder_Id tp;
    bool success { false };
    bool already_locked { false };
};

struct Event_LNBF_Params
{
    Band band = Band::UNDEFINED;
    LNBF_Type lnbf_type = LNBF_Type::UNDEFINED;
    bool inverted = false;
    uint8_t sat_id = 0;
};

struct Event_Sound
{
    Volume_t vol { 0 };
    bool muted { false };
};

struct Event_List_Update
{
    typedef bool Is_Done;
    std::function<void(Is_Done)> callback;
    std::function<void(size_t, const std::vector<Service>&)> partial_callback;
    bool clear_table = true;
    bool emit_lineup_ready = true;
    uint16_t scan_sat_count = 1;
    uint16_t scan_sat_index = 0;
    bool has_sky = false;
    bool has_claro = false;
};

struct Event_List_Update_Channel_List
{
    typedef bool Is_Done;
    std::function<void(Is_Done)> callback;
    std::function<void(size_t, const std::vector<Service>&)> partial_callback;
    std::vector<Transponder> transponders;
    uint16_t scan_sat_count = 1;
    uint16_t scan_sat_index = 0;
    bool has_sky = false;
    bool has_claro = false;
};

struct Event_Display_Message
{
    Static_String message;
    Message_Categories category;
    std::chrono::milliseconds timeout { 0 };
};

struct Event_CAS_Fingerprint
{
    NAGRA_NUID_t nuid {};
    NAGRA_CAID_t caid {};
    NAGRA_SCUA_t scua {};
    CAK_Version_t cak_version;
    Project_Info_t project_info;
    Chipset_Type_t chipset_type;
    Chipset_Revision_t chipset_revision;
};

constexpr uint16_t MAX_DVB_SECTION_DATA_LENGTH = 1024u;

struct Event_CAS_Request_Descramble
{
    NID_t original_network_id;
    TS_ID_t transport_stream_id;
    DVB_Table_Section pmt_section_data;
    PID_t video_pid;
    PID_t audio_pid;
    PID_t subtitle_pid;
};

struct Event_CAS_CAT_Table_Section
{
    bool is_last = false;
    DVB_Table_Section cat_section_data;
};

struct Event_USB_Plug
{
    enum Type
    {
        Plugged_In,
        Removed,
    };
    Type type;
};

struct Event_PVR_Record_Param
{
    uint16_t video_pid;
    uint16_t video_type;
    uint16_t audio_pid_count;
    uint16_t audio_pid[AUI_MAX_PVR_AUDIO_PID];
    uint16_t *audio_desc_pid;
    uint16_t audio_type[AUI_MAX_PVR_AUDIO_PID];
    uint16_t pcr_pid;
    std::string filename;
    std::string url;
};

struct Event_PVR_Status
{
    uint32_t seq;
    uint8_t state;
    unsigned int record_current_time;
    unsigned int player_total_time;
    unsigned int player_current_time;
    unsigned int timeshift_record_curr_time;
    unsigned int timeshift_play_current_time;
    std::string mount_point;
    std::string filesystem_type;
};

} // namespace mb
