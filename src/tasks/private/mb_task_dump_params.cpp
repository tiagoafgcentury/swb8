#include "mb_task_dump_params.h"

#include "common/mb_lineup.h"
#include "common/mb_hash.h"

namespace mb {

#ifndef NDEBUG
static char buffer[1000];

const char *dump_params()
{
    return "";
}

const char *dump_params(const uint8_t *msg_ptr, size_t msg_len)
{
    auto dest = buffer;
    auto src = msg_ptr;
    msg_len = std::min(msg_len, sizeof(buffer));

    for(size_t i = 0; i < msg_len; i++, src++, dest += 2)
    {
        snprintf(dest, 3, "%.2x", *src);
    }

    return buffer;
}

const char *dump_params(Event_Save_Application_State _event)
{
    snprintf(buffer, sizeof(buffer), "%c, %d, %d, %c, %c", _event.got_state ? 'T' : 'F', _event.current_channel,
             _event.volume, _event.mute ? 'T' : 'F', _event.stand_by ? 'T' : 'F');
    return buffer;
}

const char *dump_params(const Service *_srv)
{
    if(_srv)
    {
        snprintf(buffer, sizeof(buffer), "%d", _srv->viewer_channel());
        return buffer;
    }
    else
    {
        return "<null>";
    }
}

const char *dump_params(const Event_Lineup_Ready &_event)
{
    snprintf(buffer, sizeof(buffer), "%s, %f", to_str(_event.origin).data(), static_cast<double>(_event.duration.count()) / 1000.0);
    return buffer;
}

const char *dump_params(Service_ID_t _service_id)
{
    snprintf(buffer, sizeof(buffer), "%d", _service_id);
    return buffer;
}

const char *dump_params(const Event_Sound &_event)
{
    snprintf(buffer, sizeof(buffer), "%d, %c", _event.vol, _event.muted ? 'T' : 'F');
    return buffer;
}

const char *dump_params(const Event_Display_Message &_event)
{
    return _event.message.data();
}

const char *dump_params(const Transponder *_tp)
{
    if(_tp)
    {
        snprintf(buffer, sizeof(buffer), "%u%c", _tp->transponder_id.frequency(), _tp->transponder_id.polarity() == Polarity::Horizontal ? 'H' : 'V');
        return buffer;
    }
    return "<null>";
}

const char *dump_params(const Transponder *_tp, bool _force)
{
    if(_tp)
    {
        snprintf(buffer, sizeof(buffer), "%u%c, force: %c", _tp->transponder_id.frequency(), _tp->transponder_id.polarity() == Polarity::Horizontal ? 'H' : 'V', _force ? 'T' : 'F');
        return buffer;
    }
    return "<null>";
}

const char *dump_params(const Event_Tuner_Lock &_event)
{
    snprintf(buffer, sizeof(buffer), "%u%c,%c,%c", _event.tp.frequency(),
             _event.tp.polarity() == Polarity::Horizontal ? 'H' : 'V',
             _event.success ? 'T' : 'F', _event.already_locked ? 'T' : 'F');
    return buffer;
}

const char *dump_params(const Satellite &)
{
    snprintf(buffer, sizeof(buffer), "Salvar satélite");
    return buffer;
}

const char *dump_params(PID_t _pid, std::weak_ptr<Event_OTA_DSI>)
{
    snprintf(buffer, sizeof(buffer), "Função ota, PID(%d)", _pid);
    return buffer;
}

const char *dump_params(std::function<void(std::vector<Satellite>)> &)
{
    snprintf(buffer, sizeof(buffer), "Função");
    return buffer;
}

const char* dump_params(std::function<void(std::string)> &)
{
    snprintf(buffer, sizeof(buffer), "Dados do transponder");
    return buffer;
}

const char *dump_params(const Event_Transponder_data &)
{
    snprintf(buffer, sizeof(buffer), "Progresso da detecção do LNBF");
    return buffer;
}

const char *dump_params(const Event_CAS_Fingerprint &_event)
{
    snprintf(buffer, sizeof(buffer), "NUID %s CAID %s SCUA %s CAK Version '%s' Proj. Info '%s' Chip '%s' Rev. '%s'",
             _event.nuid.c_str(), _event.caid.c_str(), _event.scua.c_str(), _event.cak_version.c_str(),
             _event.project_info.c_str(), _event.chipset_type.c_str(), _event.chipset_revision.c_str());
    return buffer;
}

const char *dump_params(const Event_Time &_e)
{
    snprintf(buffer, sizeof(buffer), "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
             _e.year, _e.month, _e.day, _e.hour, _e.minute, _e.second);
    return buffer;
}

const char *dump_params(std::weak_ptr<Event_Autodetect_LNBf>)
{
    return "<Event_Autodetect_LNBf>";
}

const char *dump_params(std::weak_ptr<Event_Blind_Scan_Progress>)
{
    return "<Event_Blind_Scan_Progress>";
}

const char *dump_params(std::weak_ptr<Event_List_Update>, bool)
{
    return "<_lineup_build>";
}

const char *dump_params(std::weak_ptr<Event_List_Update>)
{
    return "<event>";
}

const char *dump_params(std::weak_ptr<Event_List_Update_Channel_List>)
{
    return "<event update channel list>";
}

const char *dump_params(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_info)
{
    snprintf(buffer, sizeof(buffer), "Type: %s <Channel_Info:size %zu>", to_str(_channel_list_type).data(), _channel_info.size());
    return buffer;
}

const char *dump_params(const DVB_Table_Section &_table)
{
    static char buffer[21];
    auto c = _table.data();
    auto size = std::min<int>(_table.size(), 10);

    if(size)
    {
        for(int i = 0; i < size; i++)
        {
            snprintf(&buffer[i * 2], 3, "%.2x", c[i]);
        }
    }
    else
    {
        buffer[0] = 0;
    }

    return buffer;
}

const char *dump_params(const Event_CAS_Request_Descramble &_event)
{
    snprintf(buffer, sizeof(buffer), "ONID %.4d, TSID %.4d VPID %.4d APID %.4d CCPID %.4d PMT (%u) %s",
             _event.original_network_id, _event.transport_stream_id, _event.video_pid, _event.audio_pid, _event.subtitle_pid,
             _event.pmt_section_data.size(), dump_params(_event.pmt_section_data));
    return buffer;
}

const char *dump_params(const Event_LNBF_Params &_event)
{
    snprintf(buffer, sizeof(buffer), "Band %d, LNBF_Type %d, inverted %s",
             (int)_event.band, (int)_event.lnbf_type, _event.inverted ? "I" : "N");
    return buffer;
}

const char *dump_params(const Event_System_Settings &_event)
{
    snprintf(buffer, sizeof(buffer), "TZ%u,%s,%s,%s,%s", static_cast<uint32_t>(_event.time_zone),
             to_str(_event.clock_status).data(), to_str(_event.resolution).data(),
             to_str(_event.color_standard).data(), to_str(_event.aspect_mode).data());
    return buffer;
}

const char *dump_params(const Event_CAS_CAT_Table_Section &_event)
{
    snprintf(buffer, sizeof(buffer), "CAT (%u) %s",
             _event.cat_section_data.size(), dump_params(_event.cat_section_data));
    return buffer;
}

const char *dump_params(PID_t _pid, Service_ID_t _service_id, const DVB_Table_Section &)
{
    snprintf(buffer, sizeof(buffer), "PID %u, Service Id %u, PMT", _pid, _service_id);
    return buffer;
}

const char *dump_params(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number)
{
    snprintf(buffer, sizeof(buffer), "PID %d, Service_id %d, Ver %d", _pid, _service_id, _current_version_number);
    return buffer;
}

const char *dump_params(Service_ID_t _service_id, const DVB_Table_Section &_pmt)
{
    snprintf(buffer, sizeof(buffer), "Service id: %d DVB Table Section size: %u", _service_id, _pmt.size());
    return buffer;
}

const char* dump_params(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id)
{
    snprintf(buffer, sizeof(buffer), "From_Zone_id: %d To_Zone_id: %d", _from_zone_id, _to_zone_id);
    return buffer;
}

const char *dump_params(std::string url)
{
    snprintf(buffer, sizeof(buffer), "%s", url.c_str());
    return buffer;
}

const char *dump_params(std::string url, uint8_t _mode)
{
    snprintf(buffer, sizeof(buffer), "%s - Mode: %d", url.c_str(), _mode);
    return buffer;
}

const char *dump_params(LNBF_Type _lnbf_type)
{
    snprintf(buffer, sizeof(buffer), "LNBF_Type: %s", to_str(_lnbf_type).data());
    return buffer;
}

const char *dump_params(const Event_CC &_event)
{
    static_assert(MBGUI_CC_MAX_LINES == 4);
    snprintf(buffer, sizeof(buffer), "L1: %d,%d %s | L2 %d,%d %s | L3 %d,%d %s | L4 %d,%d %s",
             _event.lines[0].x, _event.lines[0].y, _event.lines[0].text.data(),
             _event.lines[1].x, _event.lines[1].y, _event.lines[1].text.data(),
             _event.lines[2].x, _event.lines[2].y, _event.lines[2].text.data(),
             _event.lines[3].x, _event.lines[3].y, _event.lines[3].text.data()
    );
    return buffer;
}

const char *dump_params(const Event_Subtitle_Image &_event)
{
    snprintf(buffer, sizeof(buffer), "SIZE: %d", _event.data.size());
    return buffer;
}

const char *dump_params(CC_Type _type)
{
    snprintf(buffer, sizeof(buffer), "CC_Type: %s", to_str(_type).data());
    return buffer;
}


const char *dump_params(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{

    // Converter para time_t
    std::time_t tt = std::chrono::system_clock::to_time_t(_time_to_end);

    // Quebrar em struct tm (localtime)
    struct tm tm_val;
    localtime_r(&tt, &tm_val);  // ou localtime_s no Windows

    // Formatar: YYYY-MM-DD HH:MM:SS
    snprintf(buffer, sizeof(buffer), "Service id: %d Time to end: %04d-%02d-%02d %02d:%02d:%02d",
             _service_id,
             tm_val.tm_year + 1900,
             tm_val.tm_mon + 1,
             tm_val.tm_mday,
             tm_val.tm_hour,
             tm_val.tm_min,
             tm_val.tm_sec);

    return buffer;
}

const char *dump_params(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end)
{
    // Converter para time_t
    std::time_t tt = std::chrono::system_clock::to_time_t(_time_to_end);

    // Quebrar em struct tm (localtime)
    struct tm tm_val;
    localtime_r(&tt, &tm_val);  // ou localtime_s no Windows

    // Formatar: YYYY-MM-DD HH:MM:SS
    snprintf(buffer, sizeof(buffer), "Call_pvr id: %d call_sleep_timer: %d Time to end: %04d-%02d-%02d %02d:%02d:%02d",
             _call_pvr,
             _call_sleep_timer,
             tm_val.tm_year + 1900,
             tm_val.tm_mon + 1,
             tm_val.tm_mday,
             tm_val.tm_hour,
             tm_val.tm_min,
             tm_val.tm_sec);

    return buffer;
}

#ifdef MB_STATIC_STRING
const char *dump_params(Static_String _string)
{
    return _string.data();
}
#endif

const char *dump_params(std::function<void(std::vector<ScheduleEntry>)> &)
{
    snprintf(buffer, sizeof(buffer), "Função");
    return buffer;
}

const char *dump_params(ScheduleEntry &_entry)
{
    /* Faltam:
    std::chrono::time_point<std::chrono::system_clock> time_to_start;
    std::chrono::time_point<std::chrono::system_clock> time_to_end;
    Schedule_Operation operation;
    Schedule_Repeat repeat;
    Schedule_Status status;
    */
    snprintf(buffer, sizeof(buffer), "id %d, service %d", _entry.id, _entry.service_id);
    return buffer;
}

const char *dump_params(Event_USB_Plug _event)
{
    const char *type;

    if(_event.type == Event_USB_Plug::Plugged_In)
    {
        type = "plugado";
    }
    else
    {
        type = "removido";
    }

    snprintf(buffer, sizeof(buffer), "USB %s", type);
    return buffer;
}

const char* dump_params(Satellite_Operator _operator, Zone_ID_t _zone_id)
{
    snprintf(buffer, sizeof(buffer), "%s, %d", to_str(_operator).data(), _zone_id);
    return buffer;
}

const char *dump_params(const Event_PVR_Record_Param &)
{
    snprintf(buffer, sizeof(buffer), "Função");
    return buffer;
}

const char *dump_params(const Event_PVR_Status &_event)
{
    snprintf(buffer, sizeof(buffer), "State %d, Rec_Curr_Time %u, Ply_Curr_Time %u Ply_Total_Time %u %s %s",
             _event.state, _event.record_current_time, _event.player_total_time, _event.player_current_time,             _event.mount_point.c_str(), _event.filesystem_type.c_str());

    return buffer;
}

#endif

} // namespace mb
