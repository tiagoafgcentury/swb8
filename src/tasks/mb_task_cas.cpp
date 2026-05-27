#include "mb_task_cas.h"

#include "cas/nagra/mb_cas_main.h"
#include "common/mb_assert.h"
#include "common/mb_globals.h"
#include "mb_events.h"

#include <filesystem>
#include <thread>

namespace mb {

Task_CAS::Task_CAS()
#if defined(MBGUI_APP_CAS)
    : m_nagra(std::make_unique<Nagra>())
#endif
{
    mb_assert(s_task_cas == nullptr);
    s_task_cas = this;
#if defined(MBGUI_APP_CAS)
    m_nagra->set_callback_need_reset([](){
        post_event_system_need_reset();
        g_mbcas_keep_running.store(false, std::memory_order_relaxed);
    });
    m_nagra->set_popup_callback(
        [](const Event_Display_Message& event)
    {
        post_event_cas_popup_message(event);
    });
    post_event(std::bind(&Task_CAS::handle_event_cas_fingerprint_get, this));
#endif
}

Task_CAS::~Task_CAS()
{
    mb_assert(s_task_cas == this);
    s_task_cas = nullptr;
}

Task_CAS *Task_CAS::get_instance()
{
    return s_task_cas;
}

void Task_CAS::process()
{
#if defined(MBGUI_APP_CAS)

    if(m_nagra)
    {
        m_nagra->process();
    }

#endif
}

// IPC Events
#if defined(MBGUI_APP_CAS)
void Task_CAS::handle_event_cas_fingerprint_get()
{
    Event_CAS_Fingerprint e;
    std::tie(e.nuid, e.caid, e.scua, e.cak_version, e.project_info, e.chipset_type, e.chipset_revision) = m_nagra->get_fingerprint();
    post_event_cas_fingerprint_ready(e);
}

void Task_CAS::handle_event_cas_request_descramble(const Event_CAS_Request_Descramble &_event)
{
    auto ret = m_nagra->request_program_descrambling(_event.original_network_id,
               _event.transport_stream_id,
               _event.pmt_section_data,
               _event.video_pid,
               _event.audio_pid,
               _event.subtitle_pid);
    post_event_cas_request_descramble_done(ret == Nagra::Status::OK);
}

void Task_CAS::handle_event_cas_request_descramble_stop()
{
    m_nagra->request_program_descrambling_stop();
    post_event_cas_popup_message({});
    post_event_cas_request_descramble_stop_done();
}

void Task_CAS::handle_event_cas_request_descramble_pmt_update(const Event_CAS_Request_Descramble &_event)
{
    m_nagra->request_update_pmt_section(_event.pmt_section_data);
}

void Task_CAS::handle_event_cas_send_cat_table_section(const Event_CAS_CAT_Table_Section &_event)
{
    m_nagra->set_cat_table_section(_event.cat_section_data.data(), _event.cat_section_data.size(), _event.is_last);
}

void Task_CAS::handle_event_player_change_audio(PID_t _new_pid)
{
    m_nagra->change_audio_pid(_new_pid);
}

void Task_CAS::handle_event_cas_pvr_timeshift_start(Event_PVR_Record_Param _param)
{
    m_nagra->request_cas_pvr_timeshift_start(_param);
}

void Task_CAS::handle_event_cas_pvr_timeshift_play()
{
    m_nagra->request_cas_pvr_timeshift_play();
}

void Task_CAS::handle_event_cas_pvr_timeshift_stop()
{
    m_nagra->request_cas_pvr_timeshift_stop();
}

void Task_CAS::handle_event_cas_pvr_record_start(Event_PVR_Record_Param _param)
{
    m_nagra->request_cas_pvr_record_start(_param);
}

void Task_CAS::handle_event_cas_pvr_record_stop()
{
    m_nagra->request_cas_pvr_record_stop();
}

void Task_CAS::handle_event_cas_pvr_record_pause()
{
    m_nagra->request_cas_pvr_record_pause();
}

void Task_CAS::handle_event_cas_pvr_record_resume()
{
    m_nagra->request_cas_pvr_record_resume();
}

void Task_CAS::handle_event_cas_pvr_play_start(std::string url)
{
    m_nagra->request_cas_pvr_play_start(url);
}

void Task_CAS::handle_event_cas_pvr_play_stop()
{
    m_nagra->request_cas_pvr_play_stop();
}

void Task_CAS::handle_event_cas_pvr_play_pause()
{
       m_nagra->request_cas_pvr_play_pause();
}

void Task_CAS::handle_event_cas_pvr_play_resume()
{
       m_nagra->request_cas_pvr_play_resume();
}

void Task_CAS::handle_event_cas_pvr_play_forward(uint16_t _mp_speed)
{
    m_nagra->request_cas_pvr_play_forward(_mp_speed);
}

void Task_CAS::handle_event_cas_pvr_play_rewind(uint16_t _mp_speed)
{
    m_nagra->request_cas_pvr_play_rewind(_mp_speed);
}

void Task_CAS::handle_event_cas_pvr_play_next(std::string url)
{
    m_nagra->request_cas_pvr_play_next(url);
}

void Task_CAS::handle_event_cas_exit()
{
    g_mbcas_keep_running = false;
    g_mbcas_container_restart = true;

    for(int i = 0; i < MBGUI_SOCKET_MAX_MESSAGES + 5; i++)
    {
        Task::run_processes();
        std::this_thread::yield();
    }
}

void Task_CAS::handle_event_system_factory_reset()
{
#if 0
    m_nagra.reset();
    namespace fs = std::filesystem;
    std::error_code ec_dir;

    for(auto const &dir_entry : fs::directory_iterator(MBGUI_NAGRA_STORAGE_PATH, ec_dir))
    {
        std::error_code ec;
        fs::remove_all(dir_entry.path(), ec);
        DEBUG_MSG("Remove file: (" << ec << ") " << dir_entry << "\n");
    }

    g_mbcas_keep_running = false;
    g_mbcas_container_restart = true;
#endif
    g_mbcas_keep_running = false;
    g_mbcas_container_restart = true;
    // Ensure that the post_event_system_factory_reset_done is sent
    post_event_system_factory_reset_done();

    for(int i = 0; i < MBGUI_SOCKET_MAX_MESSAGES + 5; i++)
    {
        Task::run_processes();
        std::this_thread::yield();
    }
}

#else

void Task_CAS::handle_event_cas_fingerprint_ready(const Event_CAS_Fingerprint &_event)
{
    DEBUG_MSG(TASK, INFO, "Got fingerprint: " << hex << _event.nuid << "\n");
    auto cb = m_fingerprint_callback.lock();

    if(cb)
    {
        cb->operator()(_event.nuid, _event.caid, _event.scua, _event.cak_version, _event.project_info,
                       _event.chipset_type, _event.chipset_revision);
        m_fingerprint_callback = {};
    }
}

void Task_CAS::get_cas_fingerprint(std::shared_ptr<Fingerprint_Callback> _callback)
{
    m_fingerprint_callback = _callback;
    post_event_cas_fingerprint_get();
}

#endif

}
