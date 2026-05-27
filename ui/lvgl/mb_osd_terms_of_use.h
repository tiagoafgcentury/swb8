#pragma once

#include "mb_osd.h"
#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"
#include "mb_osd_breadcrumb.h"
#include "mb_osd_keys.h"
#include "tasks/mb_task_database.h"
#include "tasks/mb_task_cas.h"
#include "dvb/mb_dvb_utc_mjd.h"

namespace mb {

class OSD_Translate;

class OSD_Terms_of_Use : public OSD, public Remote_Control_Handler
{
public:
    typedef std::function<void(bool)> Terms_of_Use_CB_t;

private:
    static constexpr auto offset_y = 120;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;

    lv_obj_t *m_main               { nullptr };
    static constexpr auto title_y = 0;
    static constexpr auto title_h = 30;

    lv_obj_t *m_page               { nullptr };
    lv_obj_t *m_page_label         { nullptr };
    lv_obj_t *m_label         { nullptr };
    static constexpr auto page_x = 185;
    static constexpr auto page_y = 179 - offset_y;
    static constexpr auto page_w = 910;
    static constexpr auto page_h = 327;

    lv_obj_t *m_circle            { nullptr };
    lv_obj_t *m_circle_sel        { nullptr };
    static constexpr auto circle_x = 185;
    static constexpr auto circle_y = 533 - offset_y;
    static constexpr auto circle_w = 20;
    static constexpr auto circle_h = 20;

    lv_obj_t *m_accept            { nullptr };
    static constexpr auto accept_x = 218;
    static constexpr auto accept_y = circle_y;
    static constexpr auto accept_w = 472;
    static constexpr auto accept_h = 27;


    lv_obj_t *m_arrow_left            { nullptr };
    lv_obj_t *m_arrow_left_sel        { nullptr };
    static constexpr auto arrow_left_x = 935;
    static constexpr auto arrow_left_y = circle_y;
    static constexpr auto arrow_left_w = 20;
    static constexpr auto arrow_left_h = 20;

    lv_obj_t *m_arrow_label            { nullptr };
    static constexpr auto arrow_label_x = 965;
    static constexpr auto arrow_label_y = circle_y;
    static constexpr auto arrow_label_w = 94;
    static constexpr auto arrow_label_h = 27;

    lv_obj_t *m_arrow_rigth { nullptr };
    lv_obj_t *m_arrow_rigth_sel { nullptr };
    static constexpr auto arrow_rigth_x = 1073;
    static constexpr auto arrow_rigth_y = circle_y;
    static constexpr auto arrow_rigth_w = 20;
    static constexpr auto arrow_rigth_h = 20;

    static constexpr auto MAX_SCROLL_Y = 1800;
    static constexpr auto SCROLL_PAGE_Y = 300;

    enum class State
    {
        Page,
        Accept,
        Buttons,
        COUNT
    };
    State m_state = State::Page;

    // Teclado
    MB_OSD_Keys m_keys;
    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 409;
    static constexpr auto button_y = 599 - offset_y;
    static constexpr auto spacing = 652 - button_x;

    Terms_of_Use_CB_t m_callback;
    void call_callback(bool _value)
    {
        if (m_callback)
        {
            auto cb = std::move(m_callback); // Zera a var do this
            Task::post_event(std::bind(cb, _value));
        }
    }

    // Indica que o usuário leu os direitos e pode prosseguir
    bool m_accepted = false;
    bool m_read_all_pages = false;
    uint8_t m_page_num = 1;
    static constexpr auto MAX_NUM_PAGE = 7;
    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_scua_cb;

    int16_t m_cursor_pos = 0;
    void draw_page();
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;
    virtual void got_focus() override;
    static OSD_Terms_of_Use *s_instance;

#ifndef NDEBUG
    virtual const char *HANDLER_NAME() const
    {
        return typeid(*this).name();
    }

#endif

public:
    OSD_Terms_of_Use(OSD *_parent);
    virtual ~OSD_Terms_of_Use();

    virtual void show_menu_terms_of_use(Terms_of_Use_CB_t _callback);
    virtual void hide_menu();

    void process();
};

} // namespace mb
