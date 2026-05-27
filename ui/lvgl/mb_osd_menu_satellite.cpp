#include "mb_osd_menu_satellite.h"
#include "mb_osd_manual_search.h"
#include "common/mb_globals.h"
#include "common/mb_config.h"
#include "mb_menu_resources.h"
#include "mb_events.h"
#include "mb_osd.h"
#include "mb_osd_password.h"
#include "mb_osd_confirm_save.h"
#include <lvgl.h>
#include <filesystem>

namespace {
} 

namespace mb {

OSD_Menu_Satellite::OSD_Menu_Satellite(OSD *_parent):
    OSD(_parent),
    m_keys(offset_x, offset_y, button_w, button_h, spacing, button_x, button_y)
{
    m_keys =
    {
        tr(__Busca_automatica),
        tr(__Instala_Facil),
        tr(__Editar_satelite),
        tr(__Busca_manual),
        tr(__Editar_chaves)
    };
}

OSD_Menu_Satellite::~OSD_Menu_Satellite()
{
    DELETE_OBJ(m_footer);
    DELETE_OBJ(m_main_box);
    // Retira item do caminho de migalhas
    Osd_Breadcrumb::s_instance.remove_name();
    // Remove foco do controle remoto
    remove_focus();
}

bool OSD_Menu_Satellite::handle_event_remote_control(const Event_Remote_Control &_event)
{
    // Tecla OK: exibe lista de satélites
    switch (_event.key)
    {
        case Remote_Control_Key::KEY_OK:
        {
            auto active = static_cast<Func_Active>(m_keys.get_selected());

            switch (active)
            {
                case Func_Active::Auto_Search:
                {
                    auto_search();
                    break;
                }

                case Func_Active::Easy:
                {
                    easy_install();
                    break;
                }

                case Func_Active::Satellite:
                {
                    edit_satellite();
                    break;
                }

                case Func_Active::Manual_Search:
                {
                    manual_search();
                    break;
                }

                case Func_Active::Edit_Keys:
                {
                    edit_diseq_switch();
                    break;
                }
            }

            return true;
        }

        // Pressionado "Voltar", retorna com false
        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event([this]()
            {
                m_callback(false);
            });
            return true;
        }

        // Seleciona próximo botão
        case Remote_Control_Key::KEY_CHDOWN:
        {
            m_keys.next();
            change_breadcrumb();
            return true;
        }

        // Seleciona botão anterior
        case Remote_Control_Key::KEY_CHUP:
        {
            m_keys.previous();
            change_breadcrumb();
            return true;
        }

        default:
            return true;
    }
}


void OSD_Menu_Satellite::manual_search()
{

    auto background = create_password_box();
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Satellite::manual_search_confirm_callback, this, _1);
    m_password->show_password(callback, background, "0000");
}

void OSD_Menu_Satellite::manual_search_confirm_callback(bool _result)
{
    if (_result)
    {
        if (!m_auto_search)
        {
            m_auto_search = std::make_unique<OSD_Manual_Search>(this);
        }
        m_auto_search->show_manual_search_list(std::bind(&OSD_Menu_Satellite::manual_search_callback, this));
    }
    remove_password_box();
}

void OSD_Menu_Satellite::manual_search_callback()
{
    m_auto_search.reset();
}

void OSD_Menu_Satellite::show_menu_satellite(Menu_Satellite_t _callback, lv_obj_t *_bgd)
{
    // Direciona recepção de tecla
    DEBUG_MSG(OSD, DEBUG, "show_menu_satellite()\n");
    set_focus();
    m_bgd = _bgd;
    m_callback = _callback;
    // Cria área do menu
    m_main_box = create_rect(m_bgd, 0, 100, width, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_main_box);
    // Desenha teclado
    m_keys.set_background(m_main_box);
    m_keys.set_vertical();
    m_keys.set_back_color(OSD_COLOR_BLACK);
    m_keys.draw();
    m_keys.select();
    // Desenha linha vertical
    m_line_left = create_rect(m_main_box, 0, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_left);
    m_line_right = create_rect(m_main_box, width - 3, 0, 3, heigth, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line_right);
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    // Cria cortina sobe o menu que fica inicialmente apagada
    m_bgd_fade = create_rect(m_main_box, 0, 0, width - 3, heigth, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_bgd_fade);
    lv_obj_set_style_bg_opa(m_bgd_fade, LV_OPA_50, 0);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    // Preenche caminho de migalhas
    Osd_Breadcrumb::s_instance.add_name(tr(__Busca_automatica));
    // Cria rodapé
    m_footer = MB_OSD_Footer::draw(m_bgd, tr(__Selecione_a_opcao_desejada_e_pressione_ok_ou_pressione_voltar), -40);
    lv_obj_null_on_delete(&m_footer);
    lv_obj_align(m_footer, LV_ALIGN_BOTTOM_LEFT, 10, -40);
}

void OSD_Menu_Satellite::edit_satellite()
{
    auto background = create_password_box();
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Satellite::edit_satellite_callback, this, _1);
    m_password->show_password(callback, background, "0000");
}

void OSD_Menu_Satellite::edit_satellite_callback(bool _result)
{
    if (_result)
    {
        if (!m_select_satellite)
        {
            m_select_satellite = std::make_unique<Select_Satellite>(this);
        }
        using namespace std::placeholders;
        m_select_satellite->show_select_satellite(std::bind(&OSD_Menu_Satellite::select_satellite_for_edit_satellite_callback, this, _1, _2));
    }
    remove_password_box();
}

void OSD_Menu_Satellite::select_satellite_for_edit_satellite_callback(Satellite _satellite, bool _result)
{
    // Salva satélite recebido e apaga tela com lista de satélites
    Satellite satellite = _satellite;

    if (_result)
    {
        auto type = OSD_Translate::translate(_satellite.type);
        DEBUG("Type: " << type << "\n");
        if (!m_edit_satellite)
        {
            m_edit_satellite = std::make_unique<OSD_Edit_Satellite>(this);
        }
        using namespace std::placeholders;
        m_edit_satellite->edit_satellite(std::bind(&OSD_Menu_Satellite::show_edit_satellite_callback, this, _1), satellite);
    }
    m_select_satellite.reset();
}

void OSD_Menu_Satellite::show_edit_satellite_callback(bool )
{

    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    m_edit_satellite.reset();
    edit_satellite_callback(true);
}

void OSD_Menu_Satellite::change_breadcrumb()
{
    auto active = static_cast<Func_Active>(m_keys.get_selected());
    switch (active)
    {
        case Func_Active::Auto_Search:
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Busca_automatica));
            break;

        case Func_Active::Easy:
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Instala_Facil));
            break;

        case Func_Active::Satellite:
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Editar_satelite));
            break;

        case Func_Active::Manual_Search:
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Busca_manual));
            break;

        case Func_Active::Edit_Keys:
            Osd_Breadcrumb::s_instance.replace_last_name(tr(__Editar_chaves));
            break;
    }
}

void OSD_Menu_Satellite::easy_install()
{
    auto background = create_password_box();
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Satellite::easy_install_callback, this, _1);
    m_password->show_password(callback, background, "0000");
}

void OSD_Menu_Satellite::easy_install_callback(bool _result)
{
    if (_result)
    {
        m_save_bckg = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), 715, 100, 460, 534, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_save_bckg);
        // Texto com a pergunta
        std::string text;
        text += tr(__Todos_os_canais_serao_perdidos);
        text += "\n";
        text += tr(__Deseja_confirmar);
        m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
        m_confirm_save->show_confirm_save(std::bind(&OSD_Menu_Satellite::remove_all_channels_confirm_callback, this, std::placeholders::_1), m_save_bckg, text.c_str(), false);
    }

    remove_password_box();
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);

    if (not _result)
    {
        lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Menu_Satellite::remove_all_channels_confirm_callback(bool _ok)
{
    if (_ok)
    {
        Task::post_event_delete_all_services();
        Task::post_event_cas_exit();
        lv_obj_add_flag(m_main_box, LV_OBJ_FLAG_HIDDEN);
        if (!m_instala_facil)
        {
            m_instala_facil = std::make_unique<OSD_Instala_Facil>(this);
        }
        m_instala_facil->show_menu_easy_install(std::bind(&OSD_Menu_Satellite::show_menu_easy_install_callback, this), false);
    }

    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    DELETE_OBJ(m_save_bckg);
    m_confirm_save.reset();
}

void OSD_Menu_Satellite::show_menu_easy_install_callback()
{
    lv_obj_remove_flag(m_main_box, LV_OBJ_FLAG_HIDDEN);
    m_instala_facil.reset();
}

void OSD_Menu_Satellite::auto_search()
{
    auto background = create_password_box();
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Satellite::auto_search_callback, this, _1);
    m_password->show_password(callback, background, "0000");
}

void OSD_Menu_Satellite::auto_search_confirm_callback(bool _ok)
{
    if (_ok)
    {
        Task::post_event_delete_all_services();
        lv_obj_add_flag(m_main_box, LV_OBJ_FLAG_HIDDEN);

        if (!mb_choose_satellite)
        {
            mb_choose_satellite =
                std::make_unique<OSD_Auto_Search_Choose_Satellite>(this);
        }


        mb_choose_satellite->show_select_satellite(
            std::bind(&OSD_Menu_Satellite::select_satellite_for_auto_search_callback,
                      this));
    }

    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);

    DELETE_OBJ(m_save_bckg);
    m_confirm_save.reset();
}

void OSD_Menu_Satellite::auto_search_callback(bool _result)
{
    if (_result)
    {
        m_save_bckg = create_rect(get_main_screen(OSD_Layer::MAIN_MENU),
                                  715, 100, 460, 534, OSD_COLOR_BLACK);
        lv_obj_null_on_delete(&m_save_bckg);

        std::string text;
        text += tr(__Todos_os_canais_serao_perdidos);
        text += "\n";
        text += tr(__Deseja_confirmar);

        m_confirm_save = std::make_unique<OSD_Confirm_Save>(this);
        m_confirm_save->show_confirm_save(
            std::bind(&OSD_Menu_Satellite::auto_search_confirm_callback,
                      this,
                      std::placeholders::_1),
            m_save_bckg,
            text.c_str(),
            false);
    }

    remove_password_box();

    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);

    if (!_result)
    {
        lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
        lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    }
}

void OSD_Menu_Satellite::select_satellite_for_auto_search_callback()
{
    mb_choose_satellite.reset();
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_main_box, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t *OSD_Menu_Satellite::create_password_box()
{
    lv_obj_remove_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_GREY, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);

    if (!m_password)
    {
        m_password = std::make_unique<OSD_Password>(this);
    }

    return create_rect(m_bgd, 320, 100, 380, 130, OSD_COLOR_BLACK);
}

void OSD_Menu_Satellite::remove_password_box()
{
    DELETE_OBJ(m_passwd);
    lv_obj_add_flag(m_bgd_fade, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_remove_flag(m_main_box, LV_OBJ_FLAG_HIDDEN);
    m_password.reset();
}

void OSD_Menu_Satellite::edit_diseq_switch()
{
    auto background = create_password_box();
    using namespace std::placeholders;
    auto callback = std::bind(&OSD_Menu_Satellite::edit_diseq_switch_callback, this, _1);
    m_password->show_password(callback, background, "0000");
}

void OSD_Menu_Satellite::edit_diseq_switch_callback(bool _result)
{
    if (_result)
    {
        if (!m_edit_diseq_switch)
        {
            m_edit_diseq_switch = std::make_unique<Select_Switch>(this);
        }
        m_edit_diseq_switch->show_switch_list(std::bind(&OSD_Menu_Satellite::show_edit_diseq_switch_callback, this,
                      std::placeholders::_1));
    }
    remove_password_box();
}

void OSD_Menu_Satellite::show_edit_diseq_switch_callback(bool _result)
{
    m_edit_diseq_switch.reset();
    lv_obj_add_flag(m_line_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(m_line_left, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    if(_result)
    {
        Task::post_event([this]()
        {
            m_callback(true);
        });
    }
}

} // namespace mb
