#pragma once

#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_osd_translate.h"
#include "mb_events.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_tuner.h"
#include "tasks/mb_task_player.h"
#include "mb_osd_keys.h"
#include "mb_remote_control_handler.h"

#include <map>

namespace mb {

class MB_Detect_Lnbf : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool, Transponder_Id)> mb_detect_lnbf_cb_t;

private:
    Satellite satellite;

    lv_timer_t  *m_refresh_timer { nullptr };

    enum status_e
    {
        __detect,
        __fail,
        __success,
        __STATUS_COUNT
    };
    status_e status = status_e::__detect;

    // Timeout para detectar LNBF
    static constexpr auto TIMEOUT = 15;
    int current_progress = 0;

    mb_detect_lnbf_cb_t m_callback;
    static void refresh_cb(lv_timer_t *tm);
    void looking_for_sat_config();

    std::shared_ptr<Event_Autodetect_LNBf> m_post_event_autodetect_lnbf_start_event;
    std::weak_ptr<Event_Autodetect_LNBf> m_event_weak;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    MB_Detect_Lnbf(OSD *_parent);
    virtual ~MB_Detect_Lnbf();

    void lnbf_detection_start(mb_detect_lnbf_cb_t _callback, Satellite sat);
    void refresh_progress();
};

} // namespace mb
