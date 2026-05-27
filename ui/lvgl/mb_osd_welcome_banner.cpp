#include "mb_osd.h"
#include "mb_osd_welcome_banner.h"
#include "mb_osd_translate.h"
#include "mb_zone_id.h"
#include "mb_osd_fonts.h"
#include "common/mb_config.h"

namespace mb {

OSD_Welcome_Banner::OSD_Welcome_Banner(OSD *_parent):
    OSD(_parent)
{
}

OSD_Welcome_Banner::~OSD_Welcome_Banner()
{
    hide_welcome_banner();
}

void OSD_Welcome_Banner::hide_welcome_banner()
{
    DELETE_OBJ(m_banner_screen);
}

void OSD_Welcome_Banner::show_welcome_banner()
{
    if (m_banner_screen == nullptr)
    {
        m_network_id = Config::get_config()->selected_satellite_config().network_id;

        switch (m_network_id)
        {
            case Network_Id_Sky:
            {
                // Cria qrcode da Sky
                m_banner_screen = create_rect(get_main_screen(OSD_Layer::WELCOME_BANNER), Banner_SKY::qr_box_x, Banner_SKY::qr_box_y, Banner_SKY::qr_box_size, Banner_SKY::qr_box_size, OSD_COLOR_WHITE);
                lv_obj_move_background(m_banner_screen);
                break;
            }

            default:
            {
                m_banner_screen = create_rect(get_main_screen(OSD_Layer::WELCOME_BANNER), 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OSD_COLOR_BLACK);
                lv_obj_null_on_delete(&m_banner_screen);
                lv_obj_set_style_bg_opa(m_banner_screen, LV_OPA_TRANSP, 0);
                m_banner = create_rect(m_banner_screen, 0, 0, width, heigth, OSD_COLOR_RED);
                lv_obj_null_on_delete(&m_banner);
                lv_obj_set_style_radius(m_banner, 25, DEFAULT_SELECTOR);
                lv_obj_set_style_bg_opa(m_banner, LV_OPA_80, LV_PART_MAIN);
                lv_obj_align(m_banner, LV_ALIGN_CENTER, 0, 0);
                auto logo_sathd = load_image(m_banner, LOGO_SATHD_212x82, 0, 0, 212, 82);
                lv_obj_align(logo_sathd, LV_ALIGN_TOP_MID, 0, 87);
                auto sathd_lbl = set_label_text_static(m_banner, Banner_Claro::text_sathd, 0, 0, font_20, OSD_COLOR_WHITE);
                lv_label_set_long_mode(sathd_lbl, LV_LABEL_LONG_WRAP);
                lv_obj_set_width(sathd_lbl, 277);
                lv_obj_set_height(sathd_lbl, 110);
                lv_obj_set_style_text_align(sathd_lbl, LV_TEXT_ALIGN_LEFT, 0);
                lv_obj_align(sathd_lbl, LV_ALIGN_DEFAULT, 50, 218);
                auto link_lbl = set_label_text_static(m_banner, Banner_Claro::text_link, 0, 0, font_20, OSD_COLOR_WHITE);
                lv_label_set_long_mode(link_lbl, LV_LABEL_LONG_WRAP);
                lv_obj_set_width(link_lbl, 182);
                lv_obj_set_height(link_lbl, 110);
                lv_obj_set_style_text_align(link_lbl, LV_TEXT_ALIGN_LEFT, 0);
                lv_obj_align(link_lbl, LV_ALIGN_DEFAULT, 392, 218);
                set_label_text_static(m_banner, tr(__Ou), 336, 255, font_20, OSD_COLOR_WHITE);
                create_rect(m_banner, 346, 213, 2, 40, OSD_COLOR_WHITE);
                create_rect(m_banner, 346, 283, 2, 40, OSD_COLOR_WHITE);
                auto banner_logo = load_image(m_banner_screen, LOGO_MIDIABOX_BRANCO_256x37, 0, 0, 256, 37);
                lv_obj_align(banner_logo, LV_ALIGN_TOP_LEFT, 90, 42);
                break;
            }
        }
    }

    using namespace std::placeholders;
    // Este callback TEM que estar posicionado após a criação de m_qrcode
    m_process_fingerprint_cb = std::make_shared<Task_CAS::Fingerprint_Callback>(std::bind(&OSD_Welcome_Banner::set_cas_fingerprint, this, _2, _3));
    Task_CAS::get_instance()->get_cas_fingerprint(m_process_fingerprint_cb);
}

void OSD_Welcome_Banner::set_cas_fingerprint(NAGRA_CAID_t _caid, NAGRA_SCUA_t _scua)
{
    if (not m_qrcode)
    {
        switch (m_network_id)
        {
            case Network_Id_Sky:
            {
                m_qrcode = create_qrcode(m_banner_screen, Banner_SKY::qr_box_size - 5);
                lv_obj_align(m_qrcode, LV_ALIGN_CENTER, 0, 0);
                break;
            }

            default:
            {
                // Cria qrcode do sathdregional
                m_qrcode = create_qrcode(m_banner, Banner_Claro::qr_box_size);
                lv_obj_align(m_qrcode, LV_ALIGN_DEFAULT, 608, 218);
                break;
            }
        }

        lv_obj_null_on_delete(&m_qrcode);
        qrcode_update(m_network_id, m_qrcode, std::move(_caid), std::move(_scua));
    }
}

} // namespace mb
