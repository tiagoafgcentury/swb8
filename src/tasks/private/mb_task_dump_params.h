#pragma once

#include "mb_events.h"
#include "common/mb_types.h"
#include "common/mb_lineup.h"
#include "common/static_string.h"

#include <functional>

namespace mb {

#ifndef NDEBUG

const char *dump_params();
const char *dump_params(const uint8_t *msg_ptr, size_t msg_len);
const char *dump_params(Event_Save_Application_State _event);
const char *dump_params(const Service *_srv);
const char *dump_params(const Event_Lineup_Ready &_event);
const char *dump_params(Service_ID_t _service_id);
const char *dump_params(const Event_Sound &_event);
const char *dump_params(const Event_Display_Message &_event);
const char *dump_params(const Transponder *_tp);
const char *dump_params(const Transponder *_tp, bool _force);
const char *dump_params(const Event_Tuner_Lock &_event);
const char *dump_params(const Satellite &);
const char *dump_params(PID_t _pid, std::weak_ptr<Event_OTA_DSI>);
const char *dump_params(PID_t _pid);
const char *dump_params(std::function<void(std::vector<Satellite>)> &);
const char *dump_params(std::function<void(std::string)> &);
const char *dump_params(const Event_CAS_Fingerprint &_event);
const char *dump_params(const Event_Time &_e);
const char *dump_params(std::weak_ptr<Event_Autodetect_LNBf>);
const char *dump_params(std::weak_ptr<Event_Blind_Scan_Progress>);
const char *dump_params(std::weak_ptr<Event_List_Update>);
const char *dump_params(std::weak_ptr<Event_List_Update>, bool);
const char *dump_params(std::weak_ptr<Event_List_Update_Channel_List>);

const char *dump_params(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_info);
const char *dump_params(const DVB_Table_Section &_table);
const char *dump_params(const Event_CAS_Request_Descramble &_event);
const char *dump_params(const Event_LNBF_Params &_event);
const char *dump_params(const Event_System_Settings &_event);
const char *dump_params(const Event_CAS_CAT_Table_Section &_event);
const char *dump_params(PID_t _pid, Service_ID_t _service_id, const DVB_Table_Section &_pmt);
const char *dump_params(const Event_Transponder_data &_progress);
const char *dump_params(Service_ID_t _service_id, const DVB_Table_Section &_pmt);
const char *dump_params(PID_t _pid, Service_ID_t _service_id, uint8_t _current_version_number);
const char* dump_params(Zone_ID_t _from_zone_id, Zone_ID_t _to_zone_id);
const char *dump_params(Channel_List_Type _channel_list_type, std::vector<Lineup::Channel_Info> _channel_info);
const char *dump_params(std::string url);
const char *dump_params(std::string url, uint8_t mode);
const char *dump_params(LNBF_Type _lnbf_type);
const char *dump_params(const Event_CC &_event);
const char *dump_params(const Event_Subtitle_Image &_event);
const char *dump_params(CC_Type _type);
const char *dump_params(Service_ID_t _service_id, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
const char *dump_params(bool _call_pvr, bool _call_sleep_timer, std::chrono::time_point<std::chrono::system_clock> _time_to_end);
const char* dump_params(Satellite_Operator _operator, Zone_ID_t _zone_id);
const char *dump_params(const Event_PVR_Record_Param &_event);
const char *dump_params(uint16_t _mp_speed);
const char *dump_params(const Event_PVR_Status &_event);



#ifdef MB_STATIC_STRING
const char *dump_params(Static_String _string);
#endif
const char *dump_params(std::function<void(std::vector<ScheduleEntry>)> &);
const char *dump_params(ScheduleEntry &_entry);
const char *dump_params(Event_USB_Plug _event);

#endif

} // namespace mb
