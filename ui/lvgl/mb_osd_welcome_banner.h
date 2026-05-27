#pragma once

#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd.h"
#include "tasks/mb_task_cas.h"

#include "lvgl.h"
#include <functional>

namespace mb {

class OSD_Welcome_Banner: public OSD
{
public:
    typedef std::function<void(void)> Banner_CB_t;

private:
    static constexpr auto width = DISPLAY_WIDTH / 1.6;
    static constexpr auto heigth = DISPLAY_HEIGHT / 1.6;

    struct Banner_Claro
    {
        static constexpr auto qr_box_size = 96;

        static constexpr auto text_sathd = "Assistir os canais do SATHD Regional é fácil. É só ativar seu receptor gratuitamente no site www.sathdregional.com";
        static constexpr auto text_link = "Aponte a câmera do celular no QR Code ao lado para ativar seu receptor";
    };

    struct Banner_SKY
    {
        static constexpr auto qr_box_size = 245;
        static constexpr auto qr_box_x = 879;
        static constexpr auto qr_box_y = 330;
    };

    //Menu_Options
    lv_obj_t *m_banner_screen { nullptr };
    lv_obj_t *m_banner { nullptr };
    lv_obj_t *m_qrcode { nullptr };
    NID_t m_network_id {};

    std::shared_ptr<Task_CAS::Fingerprint_Callback> m_process_fingerprint_cb;
    void set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua);

public:
    OSD_Welcome_Banner(OSD *_parent);
    virtual ~OSD_Welcome_Banner();

    void hide_welcome_banner();
    void show_welcome_banner();
};

} // namespace mb
