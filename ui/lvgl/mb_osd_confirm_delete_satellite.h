#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "dvb/mb_dvb_utc_mjd.h"

#include "mb_menu_data.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "tasks/mb_task_database.h"
#include "mb_osd_edit_delete_satellite.h"
#include "mb_osd_breadcrumb.h"

#include <memory>
#include <array>

namespace mb {

class Confirm_Delete_Satellite :  public Remote_Control_Handler, public OSD
{
public:
    typedef std::function<void(bool)> confirm_delete_satellite_cb_t;

private:
    static constexpr auto offset_y = 180;
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT - offset_y;
    static constexpr uint16_t instructions_offset = 250;
    static constexpr auto START_POS_X = 90;
    static constexpr auto start_y = 147;

    lv_obj_t *m_mainscreen { nullptr };

    lv_obj_t *m_title_box { nullptr };
    static constexpr auto title_width = 400;
    static constexpr auto title_heigth = 53;
    static constexpr auto title_x = 0;
    static constexpr auto title_y = 0;
    lv_obj_t *m_title_label { nullptr };

    lv_obj_t *m_subtitle_box { nullptr };
    static constexpr auto subtitle_width = 700;
    static constexpr auto subtitle_heigth = 53;
    static constexpr auto subtitle_x = (width - subtitle_width) / 2;
    static constexpr auto subtitle_y = 247 - offset_y;
    lv_obj_t *m_subtitle_label { nullptr };

    lv_obj_t *m_btn_voltar { nullptr };
    lv_obj_t *m_lbl_voltar { nullptr };

    bool m_sat_mandatory = false;
    /*
     * Estrutura padrão de botões alinhados horizontalmente
     */
    enum class Btn_Active
    {
        Yes,
        No,
        COUNT
    };
    Btn_Active m_btn_seletected = Btn_Active::No;

    std::array<lv_obj_t *, static_cast<size_t>(Btn_Active::COUNT)> m_buttons;
    std::array<lv_obj_t *, static_cast<size_t>(Btn_Active::COUNT)> m_labels;
    std::array<std::string_view, static_cast<size_t>(Btn_Active::COUNT)> m_func_names =
    {
        tr(__Sim),
        tr(__Nao)
    };

    static constexpr auto button_w = 220;
    static constexpr auto button_h = 50;
    static constexpr auto button_x = 409;
    static constexpr auto button_y = 382 - offset_y;
    static constexpr auto button_s = 652 - button_x;

    void draw_buttons();
    void select();
    void unselect();
    /*
     * Fim da Estrutura padrão de botões
     */

    confirm_delete_satellite_cb_t m_callback;

protected:
    virtual bool handle_event_remote_control(const Event_Remote_Control &_event) override;

public:
    Confirm_Delete_Satellite(OSD *_parent);
    virtual ~Confirm_Delete_Satellite();

    void confirm_delete_satellite(confirm_delete_satellite_cb_t _callback, Satellite satellite);
};

} // namespace mb
