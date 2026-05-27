#include "mb_lnbf_detection.h"
#include "mb_osd_footer.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"

#include "common/mb_lineup.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "tasks/mb_task.h"


#include <lvgl.h>
#include <stdio.h>

namespace mb {

MB_Detect_Lnbf::MB_Detect_Lnbf(OSD *_parent):
    OSD(_parent)
{
}

MB_Detect_Lnbf::~MB_Detect_Lnbf()
{
    remove_focus();
    DELETE_TIMER(m_refresh_timer);
}

bool MB_Detect_Lnbf::handle_event_remote_control(const Event_Remote_Control &)
{
    return true;
}

void MB_Detect_Lnbf::lnbf_detection_start(mb_detect_lnbf_cb_t _callback, Satellite sat)
{
    set_focus();
    m_callback = _callback;
    satellite = sat;

    // Cria timer periódico de 1 segundo
    if (!m_refresh_timer)
    {
        m_refresh_timer = lv_timer_create(refresh_cb, 1000, this);
        lv_timer_set_repeat_count(m_refresh_timer, -1);
    }

    // Inicia detecção automática do LNBF
    looking_for_sat_config();
}

void MB_Detect_Lnbf::looking_for_sat_config()
{
    m_post_event_autodetect_lnbf_start_event = std::make_shared<Event_Autodetect_LNBf>(Event_Autodetect_LNBf
    {
        .callback = [this](Event_Autodetect_LNBf::Status _status, [[maybe_unused]] int _progress, Transponder_Id tp)
        {
            switch (_status)
            {
                case Event_Autodetect_LNBf::Status::Started:
                    break;

                case Event_Autodetect_LNBf::Status::Progress:
                    break;

                case Event_Autodetect_LNBf::Status::Failed:
                    break;

                case Event_Autodetect_LNBf::Status::Success:
                    Task::post_event(std::bind(m_callback, true, tp));
                    DEBUG_MSG(OSD, DEBUG, "LNBf Autodetect status: " << static_cast<int>(_status) << ", progress: " << _progress << ", frequency: " << tp.frequency() << "\n");
                    break;

                default:
                    break;
            }
        }
    });
    m_event_weak = std::weak_ptr<Event_Autodetect_LNBf>(m_post_event_autodetect_lnbf_start_event);
    Task::post_event_autodetect_lnbf_start(m_event_weak);
}

void MB_Detect_Lnbf::refresh_cb(lv_timer_t *tm)
{
    if (tm == nullptr)
    {
        return;
    }

    MB_Detect_Lnbf *thiz = static_cast<MB_Detect_Lnbf *>(lv_timer_get_user_data(tm));

    if (thiz)
    {
        thiz->refresh_progress();
    }
}

void MB_Detect_Lnbf::refresh_progress()
{
    if (++current_progress >= TIMEOUT)
    {
        Task::post_event(std::bind(m_callback, false, Transponder_Id{}));
    }
}

void MB_Detect_Lnbf::got_focus()
{
}

} // namespace mb
