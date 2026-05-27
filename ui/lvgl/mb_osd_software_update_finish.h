#pragma once

#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_footer.h"

#include "common/mb_globals.h"
#include "mb_remote_control_handler.h"
#include "mb_events.h"

#include <iostream>
#include <string>
#include <unistd.h>

namespace mb {

class OSD_Translate;

class OSD_Software_Update_finish : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Software_Update_Finish_CB_t;

private:
    static constexpr auto offset_x = 0;
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT;

    lv_obj_t *m_main               { nullptr };
    static constexpr auto title_x = 0;
    static constexpr auto title_y = 280;
    static constexpr auto title_h = 53;

    static constexpr auto footer_y =      -40;

    lv_obj_t *m_btn_ok                      { nullptr };
    lv_obj_t *m_lbl_ok                      { nullptr };
    Software_Update_Finish_CB_t m_callback;

    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 530;
    static constexpr auto button_y = 570;
    static constexpr auto spacing = 0;

    void reset_env_flag();
protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Software_Update_finish(OSD *_parent);
    virtual ~OSD_Software_Update_finish();
    virtual void show_menu_software_update_finish(Software_Update_Finish_CB_t _callback);
};

} // namespace mb
