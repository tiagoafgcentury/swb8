#include "mb_osd_menu_suporte.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_password.h"
#include "mb_osd_footer.h"
#include "tasks/mb_task.h"
#include <lvgl.h>

namespace mb {

OSD_Menu_Support::OSD_Menu_Support(OSD *_parent):
    OSD(_parent)
{
}

OSD_Menu_Support::~OSD_Menu_Support()
{
    DELETE_OBJ(m_footer);
    DELETE_OBJ(m_main_box);
    remove_focus();
}

// Processa tecla recebida
bool OSD_Menu_Support::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Pressionado "Voltar", retorna com false
    if (_event.key == Remote_Control_Key::KEY_VOLTAR)
    {
        Task::post_event(m_callback);
    }

    return true;
}

void OSD_Menu_Support::show_menu_support(Menu_Support_CB_t _callback, lv_obj_t *_bgd)
{
    // Direciona recepção de tecla
    DEBUG_MSG(OSD, DEBUG, "show_menu_support()\n");
    set_focus();
    m_callback = _callback;
    // Cria área do menu
    m_main_box = create_rect(_bgd, 0, 100, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    // Desenha linha vertical
    create_rect(m_main_box, 0, 0, 3, heigth, OSD_COLOR_ORANGE);
    set_label_text_static(m_main_box, tr(__Fale_conosco), 20, 20, font_semi_25, OSD_COLOR_WHITE);
    load_image(m_main_box, LOGO_WHATSAPP_25x25, 20, 90, 27, 27);
    set_label_text_static(m_main_box, whastapp_number, 56, 90, font_semi_20, OSD_COLOR_WHITE);
    load_image(m_main_box, LOGO_EMAIL_25x19, 20, 141, 25, 19);
    set_label_text_static(m_main_box, email_address, 56, 136, font_semi_20, OSD_COLOR_WHITE);
    load_image(m_main_box, LOGO_LINK_25x25, 20, 182, 25, 25);
    set_label_text_static(m_main_box, link_century, 56, 182, font_semi_20, OSD_COLOR_WHITE);
    m_qrcode = create_qrcode(m_main_box, 96);
    lv_obj_null_on_delete(&m_qrcode);
    lv_obj_align(m_qrcode, LV_ALIGN_DEFAULT, 20, 227);
    auto lbl_descricao = set_label_text_static(m_main_box, tr(__Aponte_a_camera_qrcode_falar_conosco), 133, 232, font_20, OSD_COLOR_WHITE);
    lv_obj_set_width(lbl_descricao, 176);
    lv_obj_set_height(lbl_descricao, 81);
    auto img_logo_b8 = load_image(m_main_box, LOGO_MIDIABOX_BRANCO_208x30, 20, 170, 208, 30);
    lv_obj_align(img_logo_b8, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    // Cria cortina sobe o menu que fica inicialmente apagada
    auto bgd_fade = create_rect(m_main_box, 0, 0, width - 3, heigth, OSD_COLOR_BLACK);
    lv_obj_set_style_bg_opa(bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(bgd_fade, LV_OBJ_FLAG_HIDDEN);
    lv_qrcode_update(m_qrcode, link_qrcode.data(), link_qrcode.size());
    m_footer = MB_OSD_Footer::draw(_bgd, tr(__Pressione_voltar_para_voltar), -40);
    lv_obj_null_on_delete(&m_footer);
    lv_obj_align(m_footer, LV_ALIGN_BOTTOM_LEFT, 130, -40);
    load_image(m_main_box, LOGO_FRAN, 500, 90, 179, 200);
#ifdef MBGUI_USE_RLOTTIE
    auto support_logo = lv_rlottie_create_from_file(m_main_box, 40, 40, ANIM_SUPPORT);
    lv_rlottie_set_play_mode(support_logo, LV_RLOTTIE_CTRL_LOOP);
    lv_obj_align(support_logo, LV_ALIGN_DEFAULT, 170, 10);
#endif
}

} // namespace mb
