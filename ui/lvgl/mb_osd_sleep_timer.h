#pragma once

#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_menu_data.h"
#include "mb_remote_control_handler.h"
#include "mb_osd_translate.h"

#include <memory>
#include <map>

namespace mb {

class OSD_Sleep_Timer: public OSD, public Remote_Control_Handler
{
private:
    typedef std::function<void(void)> Sleep_Timer_CB_t;

    // Variáveis desta classe
    lv_obj_t    *m_main_menu { nullptr };
    lv_obj_t    *m_bgd_timer { nullptr };
    lv_obj_t    *m_lbl_timer { nullptr };
    static int s_sleep_timer_value;
    static lv_timer_t *s_sleep_timer;
    lv_timer_t *m_exit_timer{ nullptr };

    static constexpr auto st_start_x = 915;
    static constexpr auto st_start_y = 220;
    static constexpr auto st_start_w = 380;
    static constexpr auto st_start_h = 50;

    // Lista de possiveis tempos em minutos para desligar o receptor
    static constexpr int sleep_timer_value_array[6] = { 0, 15, 30, 45, 60, 120 };

    static void process_sleep_timer_cb(lv_timer_t *_tm);
    static void process_exit_timer_cb(lv_timer_t *_tm);
    void process_sleep_timer();
    void start_sleep_timer();
    void delete_sleep_timer();
    void change_sleep_timer_value();
    void update_sleep_timer_value();

    Sleep_Timer_CB_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    OSD_Sleep_Timer(OSD *_parent);
    virtual ~OSD_Sleep_Timer();

    // Funções de acesso externo
    void        start_standby();
    int         get_sleep_timer_value();
    std::pair<int, std::string> set_sleep_timer_value();
    std::pair<int, std::string> get_sleep_timer_pair();

    void show_sleep_timer(Sleep_Timer_CB_t _callback);
    static OSD_Sleep_Timer s_instance;
};

} // namespace mb

