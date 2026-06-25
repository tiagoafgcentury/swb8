#pragma once

#include "common/mb_types.h"
#include "mb_events.h"

namespace mb {

// IPC Message Helpers
#define MSGCPY(FIELD) strncpy(msg.payload.FIELD, _event.FIELD.data(), sizeof(msg.payload.FIELD))

enum IPC_Priority
{
    PRIORITY_NORMAL,
};

enum IPC_Address
{
    ipc_addr_mbgui = 0,
    ipc_addr_mbcas = 1,
};

enum IPC_Command
{
    ipc_cmd_cas_fingerprint_get,
    ipc_cmd_cas_fingerprint_ready,
    ipc_cmd_cas_popup_display_message,
    ipc_cmd_cas_reset_needed,
    ipc_cmd_cas_request_descramble,
    ipc_cmd_cas_request_descramble_done,
    ipc_cmd_cas_request_descramble_pmt_update,
    ipc_cmd_cas_request_descramble_stop,
    ipc_cmd_cas_request_descramble_stop_done,
    ipc_cmd_cas_system_factory_reset,
    ipc_cmd_cas_system_factory_reset_done,
    ipc_cmd_cas_system_need_reset,
    ipc_cmd_cas_send_cat_table_section,
    ipc_cmd_cas_zone_id_changed,
    ipc_cmd_player_change_audio,
    ipc_cmd_reset_pin_code,
    ipc_cmd_save_zone_id,
    ipc_cmd_cas_request_pvr_timeshift_start,
    ipc_cmd_cas_request_pvr_timeshift_play,
    ipc_cmd_cas_request_pvr_timeshift_stop,
    ipc_cmd_cas_request_pvr_record_start,
    ipc_cmd_cas_request_pvr_record_stop,
    ipc_cmd_cas_request_pvr_record_pause,
    ipc_cmd_cas_request_pvr_record_resume,
    ipc_cmd_cas_request_pvr_play_start,
    ipc_cmd_cas_request_pvr_play_stop,
    ipc_cmd_cas_request_pvr_play_pause,
    ipc_cmd_cas_request_pvr_play_resume,
    ipc_cmd_cas_request_pvr_play_forward,
    ipc_cmd_cas_request_pvr_play_rewind,
    ipc_cmd_cas_request_pvr_play_next,
    ipc_cmd_cas_send_pvr_status,
    ipc_cmd_cas_system_need_exit,
    ipc_cmd_cas_switch_folder,

};

struct IPC_Message
{
    IPC_Address source;
    IPC_Command cmd;
};

struct MSG_CAS_Fingerprint_Payload
{
    char nuid[16];
    char caid[16];
    char scua[16];
    char cak_version[20];
    char project_info[40];
    char chipset_type[20];
    char chipset_revision[20];
};

struct MSG_CAS_Fingerprint
{
    IPC_Message header;
    MSG_CAS_Fingerprint_Payload payload;
};

constexpr auto MSG_CAS_POPUP_MESSAGE_PAYLOAD_SIZE = MBGUI_SOCKET_MSGSIZE - 128;

struct MSG_CAS_Popup_Message_Payload
{
    Message_Categories category;
    uint16_t size;
    char message[MSG_CAS_POPUP_MESSAGE_PAYLOAD_SIZE];
};

static_assert(sizeof(MSG_CAS_Popup_Message_Payload) <= MBGUI_SOCKET_MSGSIZE);

struct MSG_CAS_Popup_Message
{
    IPC_Message header;
    MSG_CAS_Popup_Message_Payload payload;
};

struct MSG_CAS_Request_Descramble_Payload
{
    NID_t original_network_id;
    TS_ID_t transport_stream_id;
    PID_t video_pid;
    PID_t audio_pid;
    PID_t subtitle_pid;
    uint16_t pmt_section_data_size;
    uint8_t pmt_section_data[MAX_DVB_SECTION_DATA_LENGTH];

    MSG_CAS_Request_Descramble_Payload(const Event_CAS_Request_Descramble _event)
    {
        original_network_id = _event.original_network_id;
        transport_stream_id = _event.transport_stream_id;
        video_pid = _event.video_pid;
        audio_pid = _event.audio_pid;
        subtitle_pid = _event.subtitle_pid;
        mb_assert(_event.pmt_section_data.size() <= MAX_DVB_SECTION_DATA_LENGTH);
        pmt_section_data_size = std::min(MAX_DVB_SECTION_DATA_LENGTH, _event.pmt_section_data.size());
        memcpy(pmt_section_data, _event.pmt_section_data.data(), pmt_section_data_size);
    }

    void copy_to_event(Event_CAS_Request_Descramble &_event) const
    {
        _event.original_network_id = original_network_id;
        _event.transport_stream_id = transport_stream_id;
        _event.video_pid = video_pid;
        _event.audio_pid = audio_pid;
        _event.subtitle_pid = subtitle_pid;
        _event.pmt_section_data = {pmt_section_data, pmt_section_data_size};
    }
};

static_assert(sizeof(MSG_CAS_Request_Descramble_Payload) <= MBGUI_SOCKET_MSGSIZE);

struct MSG_CAS_Request_Descramble
{
    IPC_Message header;
    MSG_CAS_Request_Descramble_Payload payload;
};

struct MSG_CAS_Request_Descramble_Done
{
    IPC_Message header;
    bool payload;
};

struct MSG_Utils_Set_Time
{
    IPC_Message header;
    Event_Time payload;
};

struct MSG_CAS_CAT_Table_Section_Payload
{
    bool is_last;
    uint16_t cat_section_data_size;
    uint8_t cat_section_data[MAX_DVB_SECTION_DATA_LENGTH];

    MSG_CAS_CAT_Table_Section_Payload(const Event_CAS_CAT_Table_Section _event):
        is_last(_event.is_last)
    {
        mb_assert(_event.cat_section_data.size() <= MAX_DVB_SECTION_DATA_LENGTH);
        cat_section_data_size = std::min(MAX_DVB_SECTION_DATA_LENGTH, _event.cat_section_data.size());
        memcpy(cat_section_data, _event.cat_section_data.data(), cat_section_data_size);
    }

    void copy_to_event(Event_CAS_CAT_Table_Section &_event) const
    {
        _event.is_last = is_last;
        _event.cat_section_data = {cat_section_data, cat_section_data_size};
    }
};

struct MSG_CAS_CAT_Table_Section
{
    IPC_Message header;
    MSG_CAS_CAT_Table_Section_Payload payload;
};

struct MSG_Zone_Id_Payload
{
    Zone_ID_t zone_id;
    Segment_ID_t segment_id;
};

struct MSG_CAS_Zone_Id_Save
{
    IPC_Message header;
    MSG_Zone_Id_Payload payload;
};

struct MSG_CAS_Audio_Changed
{
    IPC_Message header;
    PID_t payload;
};

struct MSG_CAS_Switch_Folder
{
    IPC_Message header;
    bool is_sky;
};

struct MSG_CAS_PVR_Speed
{
    IPC_Message header;
    uint16_t payload;
};

struct MSG_CAS_PVR_Record_Param_Payload
{
    uint16_t pcr_pid = 0;
    uint16_t video_pid = 0;
    uint16_t video_type = 0;
    uint16_t audio_pid_count = 0;
    uint16_t audio_pid[AUI_MAX_PVR_AUDIO_PID] = {};
    uint16_t audio_type[AUI_MAX_PVR_AUDIO_PID] = {};
    char filename[255] = {};
    char url[255] = {};

    MSG_CAS_PVR_Record_Param_Payload(const Event_PVR_Record_Param &_event)
    {
        pcr_pid    = _event.pcr_pid;
        video_pid  = _event.video_pid;
        video_type = _event.video_type;

        audio_pid_count = std::min<uint16_t>(
            _event.audio_pid_count,
            AUI_MAX_PVR_AUDIO_PID
        );

        for(uint16_t i = 0; i < audio_pid_count; ++i)
        {
            audio_pid[i]  = _event.audio_pid[i];
            audio_type[i] = _event.audio_type[i];
        }

        std::snprintf(filename, sizeof(filename), "%s", _event.filename.c_str());
        std::snprintf(url, sizeof(url), "%s", _event.url.c_str());

        filename[sizeof(filename) - 1] = '\0';
        url[sizeof(url) - 1] = '\0';
    }

    void copy_to_event(Event_PVR_Record_Param &_event) const
    {
        _event.pcr_pid    = pcr_pid;
        _event.video_pid  = video_pid;
        _event.video_type = video_type;

        _event.audio_pid_count = std::min<uint16_t>(
            audio_pid_count,
            AUI_MAX_PVR_AUDIO_PID
        );

        for(uint16_t i = 0; i < _event.audio_pid_count; ++i)
        {
            _event.audio_pid[i]  = audio_pid[i];
            _event.audio_type[i] = audio_type[i];
        }

        _event.filename = filename;
        _event.url      = url;
    }
};


struct MSG_CAS_PVR_Record_Param
{
    IPC_Message header;
    MSG_CAS_PVR_Record_Param_Payload payload;
};

struct MSG_CAS_PVR_Url
{
    IPC_Message header;
    char payload[255];
};

constexpr size_t MAX_MOUNT_POINT_LEN = 128;
constexpr size_t MAX_FS_TYPE_LEN     = 32;
constexpr uint16_t PVR_STATUS_PAYLOAD_VERSION = 1;

struct MSG_CAS_PVR_Send_Status_Payload
{
    uint16_t version;
    uint32_t seq;
    uint8_t  state;
    uint32_t record_current_time;
    uint32_t player_total_time;
    uint32_t player_current_time;
    uint32_t timeshift_record_curr_time;
    uint32_t timeshift_play_current_time;

    char mount_point[MAX_MOUNT_POINT_LEN];
    char filesystem_type[MAX_FS_TYPE_LEN];

    MSG_CAS_PVR_Send_Status_Payload(const Event_PVR_Status& event)
    {
        std::memset(this, 0, sizeof(*this));

        version             = PVR_STATUS_PAYLOAD_VERSION;
        seq                 = event.seq;
        state               = event.state;
        record_current_time = event.record_current_time;
        player_total_time   = event.player_total_time;
        player_current_time = event.player_current_time;
        timeshift_record_curr_time   = event.timeshift_record_curr_time;
        timeshift_play_current_time  = event.timeshift_play_current_time;

        std::snprintf(mount_point, sizeof(mount_point), "%s",
                      event.mount_point.c_str());

        std::snprintf(filesystem_type, sizeof(filesystem_type), "%s",
                      event.filesystem_type.c_str());
    }

    bool copy_to_event(Event_PVR_Status& event) const
    {
        if (version != PVR_STATUS_PAYLOAD_VERSION)
        {
            return false; // payload inválido / fora de sincronia
        }
        event.seq                 = seq;
        event.state               = state;
        event.record_current_time = record_current_time;
        event.player_total_time   = player_total_time;
        event.player_current_time = player_current_time;
        event.timeshift_record_curr_time = timeshift_record_curr_time;
        event.timeshift_play_current_time = timeshift_play_current_time;
        event.mount_point      = mount_point;
        event.filesystem_type  = filesystem_type;
        return true;
    }
};


struct MSG_CAS_PVR_Send_Status
{
    IPC_Message header;
    MSG_CAS_PVR_Send_Status_Payload payload;
};


} // namespace mb
