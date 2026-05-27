#include "hal/mb_display.h"
#include "hal/mb_system.h"
#include "common/mb_globals.h"
#include "common/mb_state_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <atomic>

#include <aui_dis.h>
#include <aui_osd.h>
#include <alisldis.h>
#include "mb_ali_globals.h"

namespace {
mb::Display *s_instance { nullptr };
}

namespace mb {

struct Display::Data
{
    void *hnd { nullptr };
    void *hnd_analog { nullptr };

    aui_dis_info get_info()
    {
        aui_dis_info info;
        MB_ZERO(info);
        ALI_EXEC(aui_dis_get(hnd, AUI_DIS_GET_INFO, &info));
        DEBUG_MSG(HAL, DEBUG, "Source: " << info.source_width << " x " << info.source_height << "\n");
        DEBUG_MSG(HAL, DEBUG, "Display: " << info.des_width << " x " << info.des_height << "\n");
        //DEBUG_MSG(HAL, DEBUG, "Display: " << info.a << " x " << info.des_height << "\n");
        return info;
    }
};

Display *Display::get_instance()
{
    return s_instance;
}

Display::Display():
    m_p(std::make_unique<Data>())
{
    s_instance = this;
    ALI_EXEC(aui_dis_init(nullptr));
    aui_attr_dis attr_dis;
    MB_ZERO(attr_dis);
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    ALI_EXEC(aui_dis_open(&attr_dis, &m_p->hnd));
    MB_ZERO(attr_dis);
    attr_dis.uc_dev_idx = AUI_DIS_SD;
    ALI_EXEC(aui_dis_open(&attr_dis, &m_p->hnd_analog));
    MB_ZERO(attr_dis);
    // Busca parâmetros salvos na memória
    State_File::App_State_File file;
    set_all_display_settings(static_cast<Resolution_Standard>(file.resolution), static_cast<Aspect_Mode>(file.aspect_mode), static_cast<Color_Standard>(file.color_standard));
    aui_dis_info info;
    MB_ZERO(info);
    ALI_EXEC(aui_dis_get(m_p->hnd, AUI_DIS_GET_INFO, &info));
    DEBUG_MSG(HAL, DEBUG, "Source: " << info.source_width << " x " << info.source_height << "\n");
    DEBUG_MSG(HAL, DEBUG, "Display: " << info.des_width << " x " << info.des_height << "\n");

    if(System::fake_stand_by_mode())
    {
        set_cvbs_off();
    }
    else
    {
        set_cvbs_on();
    }
}

Display::~Display()
{
    aui_attr_dis attr_dis;
    MB_ZERO(attr_dis);
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    ALI_EXEC(aui_dis_close(&attr_dis, &m_p->hnd));
    clear();
    s_instance = nullptr;
}

void Display::set_all_display_settings(Resolution_Standard _resolution_standard, Aspect_Mode _aspect_mode, Color_Standard _color_standard)
{
    set_resolution_standard(static_cast<Resolution_Standard>(_resolution_standard));
    set_aspect_mode(static_cast<Aspect_Mode>(_aspect_mode));
    set_color_standard(static_cast<Color_Standard>(_color_standard));
}

void Display::clear()
{
    //Clear framebuffer
    if(s_instance and s_instance->m_p)
    {
        ALI_EXEC(aui_dis_fill_black_screen(s_instance->m_p->hnd));
    }
}

void Display::take_snapshot()
{
    aui_capture_pic cap_param;
    MB_ZERO(cap_param);
    constexpr int data_buffer_size = 1024 * 1024 * 2;
    unsigned char data_buffer[data_buffer_size];
    cap_param.puc_out_data_buf = data_buffer;
    cap_param.ui_out_data_buf_size = data_buffer_size;
    aui_dis_capture_pic(m_p->hnd, &cap_param);
}

void Display::resize_video_to_quadrant(Quadrant _q)
{
    if(_q == Quadrant::Fullscreen)
    {
        ALI_EXEC(aui_dis_mode_set(m_p->hnd, AUI_VIEW_MODE_FULL, nullptr, nullptr));
    }
    else if(_q == Quadrant::Q2)
    {
        aui_dis_zoom_rect src_rect, dst_rect;
        MB_ZERO(src_rect);
        MB_ZERO(dst_rect);
        src_rect.ui_height = 2880;
        src_rect.ui_width = 720;
        src_rect.ui_startX = 0;
        src_rect.ui_startY = 0;
        dst_rect.ui_height = 1440; //(src_rect.ui_height / 2) -16;
        dst_rect.ui_width = 360; //(src_rect.ui_width / 2) - 30;
        dst_rect.ui_startX = 360; //(src_rect.ui_width / 2) + 30;
        dst_rect.ui_startY = 0;
        ALI_EXEC(aui_dis_mode_set(m_p->hnd, AUI_VIEW_MODE_PREVIEW, &src_rect, &dst_rect));
    }
    else if(_q == Quadrant::Q1)
    {
        aui_dis_zoom_rect src_rect, dst_rect;
        MB_ZERO(src_rect);
        MB_ZERO(dst_rect);
        src_rect.ui_height = 2880;
        src_rect.ui_width = 720;
        src_rect.ui_startX = 0;
        src_rect.ui_startY = 0;
        dst_rect.ui_height = (src_rect.ui_height / 3);
        dst_rect.ui_width = (src_rect.ui_width / 3);
        dst_rect.ui_startX = (src_rect.ui_width / 2) + 10;
        dst_rect.ui_startY = (src_rect.ui_height / 9);
        ALI_EXEC(aui_dis_mode_set(m_p->hnd, AUI_VIEW_MODE_PREVIEW, &src_rect, &dst_rect));
    }
    else if(_q == Quadrant::Q4)
    {
        aui_dis_zoom_rect src_rect, dst_rect;
        MB_ZERO(src_rect);
        MB_ZERO(dst_rect);
        src_rect.ui_height = 2880;
        src_rect.ui_width = 720;
        src_rect.ui_startX = 0;
        src_rect.ui_startY = 0;
        dst_rect.ui_height = (src_rect.ui_height / 4);
        dst_rect.ui_width = (src_rect.ui_width / 4);
        dst_rect.ui_startX = (src_rect.ui_width / 2) + 140;
        dst_rect.ui_startY = (src_rect.ui_height / 6);
        ALI_EXEC(aui_dis_mode_set(m_p->hnd, AUI_VIEW_MODE_PREVIEW, &src_rect, &dst_rect));
    }
}

Color_Standard Display::get_color_standard()
{
    aui_dis_info info;
    MB_ZERO(info);
    auto hnd = m_p->hnd_analog;
    ALI_EXEC(aui_dis_get(hnd, AUI_DIS_GET_INFO, &info));

    switch(info.tvsys)
    {
        /**
         V alue to specify the* <b> PAL4.43(==PAL_BDGHI), fh=15.625 and fv=50 </b>
         video display format, in particular:
         - @b 576I video display format for interlaced display
         - @b 576P video display format for progressive display
         */
        case AUI_DIS_TVSYS_PAL:
            return Color_Standard::PAL_M_50;

        /**
         V alue to specify the* <b> NTSC3.58, fh=15.734 and fv=59.94 </b> video display
         format, in particular
             - @b 480I video display format for interlaced display
             - @b 480P video display format for progressive display
             */
        case AUI_DIS_TVSYS_NTSC:
            return Color_Standard::NTSC_60;

        /**
         V alue to specify the* <b> PAL3.58, fh=set_color_standard15.734 and fv=59.94 </b> video display
         format
             */
        case AUI_DIS_TVSYS_PAL_M:
            return Color_Standard::PAL_M_60;

        /**
         V alue to specify the* <b> PAL4.43(changed PAL mode), fh=15.625 and fv=50 </b>
         video display format
         */
        case AUI_DIS_TVSYS_PAL_N:
            return Color_Standard::PAL_N_50;

        /**
         V alue to specify the* <b> PAL, fh=15.734 and fv=59.94 </b> video display format
         */
        case AUI_DIS_TVSYS_PAL_60:

        /**
         V alue to specify the* <b> NTSC4.43, fh=15.734 and fv=59.94 </b> video display
         format
             */
        case AUI_DIS_TVSYS_NTSC_443:

        /**
         @ attention  This vid*eo display format is @a reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_MAC:

        /**
         V alue to specify the* <b> 720p50 </b> video display format, which has
         720 lines per frame and 50 frames per second

         @note   ALi chipsets don't support the video display format <b> 720i50 </b>
         */
        case AUI_DIS_TVSYS_LINE_720_50:

        /**
         V alue to specify the* <b> 720p60 </b> video display format, which has
         720 lines per frame and 60 frames per second

         @note   ALi chipsets don't support the video display format <b> 720i60 </b>
         */
        case AUI_DIS_TVSYS_LINE_720_60:

        /**set_color_standard
         V alue to specify the*
         - @b 1080p25 video display format for progressive display
         - @b 1080i50 video display format for interlaced  display

         which has 1080 lines per frame and 25 frames per second.

         @note   The @b 1080p25 video display format is reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_LINE_1080_25:

        /**
         V alue to specify the*
         - @b 1080p30 video display format for progressive display
         - @b 1080i60 video display format for interlaced  display

         which has 1080 lines per frame and 30 frames per second.

         @note   The @b 1080p30 video display format is reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_LINE_1080_30:

        /**
         V alue to specify the* <b> 1080p50 </b> video display format, which has
         1080 lines per frame and 50 frames per second.

         @note   This value is only for the @b 1080p50 video display format, for the
         @b 1080i50 video display format please refer to the enum value
         #AUI_DIS_TVSYS_LINE_1080_25
         */
        case AUI_DIS_TVSYS_LINE_1080_50:

        /**
         V alue to specify the* <b> 1080p60 </b> video display format, which has
         1080 lines per frame and 60 frames per second.

         @note   This value is only for the @b 1080p60 video display format, for the
         @b 1080i60 video display format please refer to the enum value
         #AUI_DIS_TVSYS_LINE_1080_30
         */
        case AUI_DIS_TVSYS_LINE_1080_60:

        /**
         @ attention  This vid*eo display format is @a reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_LINE_1080_24:

        /**
         @ attention  This vid*eo display format is @a reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_LINE_1152_ASS:

        /**
         @ attention  This vid*eo display format is @a reserved to ALi R&D Dept. then
         user can ignore it
         */
        case AUI_DIS_TVSYS_LINE_1080_ASS:

        /**
         V alue to specify the* <b> PAL3.58, fh=15.625 and fv=50 </b> video display
         format
             */
        case AUI_DIS_TVSYS_PAL_NC:

        /**
         V alue to specify the* <b> 3840x2160 @24Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_3840X2160_24:

        /**
         V alue to specify the* <b> 3840x2160 @25Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_3840X2160_25:

        /**
         V alue to specify the* <b> 3840x2160 @30Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_3840X2160_30:

        /**
         V alue to specify the* <b> 3840x2160 @50Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_3840X2160_50:

        /**
         V alue to specify the* <b> 3840x2160 @60Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_3840X2160_60:

        /**
         V alue to specify the* <b> 4096x2160 @24Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_4096X2160_24:

        /**
         V alue to specify the* <b> 4096x2160 @25Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_4096X2160_25:

        /**
         V alue to specify the* <b> 4096x2160 @30Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_4096X2160_30:

        /**
         V alue to specify the* <b> 4096x2160 @50Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_4096X2160_50:

        /**
         V alue to specify the* <b> 4096x2160 @60Hz </b> video display format
         */
        case AUI_DIS_TVSYS_LINE_4096X2160_60:

        /**
         V alue to specify the* <b> NTSC3.58, fh=15.734 Hz, fv=59.94 Hz </b>
         video display format
         */
        case AUI_DIS_TVSYS_NTSC_J:
            return Color_Standard::NTSC_60;
            break;
    }

    return Color_Standard::UNDEFINED;
}

void Display::set_cvbs_on()
{
    auto hnd_analog = m_p->hnd_analog;
    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
    ALI_EXEC(aui_dis_dac_unreg(hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_dac_reg(hnd_analog, ui_dac_attr, ui_ary_num));
    // Busca parâmetros salvos na memória
    State_File::App_State_File file;
    set_color_standard(static_cast<Color_Standard>(file.color_standard));
}

void Display::set_cvbs_off()
{
    auto hnd_analog = m_p->hnd_analog;
    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_NONE};
    ALI_EXEC(aui_dis_dac_unreg(hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_dac_reg(hnd_analog, ui_dac_attr, ui_ary_num));
}

void Display::set_color_standard(Color_Standard _color_standard)
{
    aui_dis_tvsys tvsys;
    tvsys = _color_standard == Color_Standard::NTSC_60 ? AUI_DIS_TVSYS_NTSC_J :
            _color_standard == Color_Standard::PAL_M_50 ? AUI_DIS_TVSYS_PAL :
            _color_standard == Color_Standard::PAL_N_50 ? AUI_DIS_TVSYS_PAL_N : AUI_DIS_TVSYS_PAL_M;

    if(_color_standard == Color_Standard::NTSC_60)
    {
        tvsys = AUI_DIS_TVSYS_NTSC_J;
        DEBUG_MSG(HAL, DEBUG, "NTSC_60\n");
    }
    else if(_color_standard == Color_Standard::PAL_M_50)
    {
        tvsys = AUI_DIS_TVSYS_PAL;
        DEBUG_MSG(HAL, DEBUG, "PAL_M_50\n");
    }
    else if(_color_standard == Color_Standard::PAL_N_50)
    {
        tvsys = AUI_DIS_TVSYS_PAL_N;
        DEBUG_MSG(HAL, DEBUG, "PAL_N_50\n");
    }
    else if(_color_standard == Color_Standard::PAL_M_60)
    {
        tvsys = AUI_DIS_TVSYS_PAL_M;
        DEBUG_MSG(HAL, DEBUG, "PAL_M_60\n");
    }
    else
    {
        tvsys = AUI_DIS_TVSYS_PAL;
        DEBUG_MSG(HAL, DEBUG, "PAL\n");
    }

    DEBUG_MSG(HAL, DEBUG, "tvsys: " << (int)tvsys << "\n");
    auto hnd_analog = m_p->hnd_analog;
    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
    ALI_EXEC(aui_dis_dac_unreg(hnd_analog, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_tv_system_set(hnd_analog, tvsys, 1));
    unsigned int tve_src = AUI_DIS_SD;
    ALI_EXEC(aui_dis_set(hnd_analog, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
    ALI_EXEC(aui_dis_dac_reg(hnd_analog, ui_dac_attr, ui_ary_num));
}

//aui_dis_aspect_ratio en_asp_ratio
void Display::set_aspect_mode(Aspect_Mode _mode)
{
    aui_dis_aspect_ratio en_asp_ratio;
    aui_dis_match_mode en_asp_mode;

    switch(_mode)
    {
        case Aspect_Mode::PILLBOX_16x9:
            en_asp_ratio = AUI_DIS_AP_16_9;
            en_asp_mode = AUI_DIS_MM_PILLBOX;
            break;

        case Aspect_Mode::PANSCAN_16x9:
            en_asp_ratio = AUI_DIS_AP_16_9;
            en_asp_mode = AUI_DIS_MM_PANSCAN;
            break;

        case Aspect_Mode::LETTERBOX_16x9:
            en_asp_ratio = AUI_DIS_AP_16_9;
            en_asp_mode = AUI_DIS_MM_LETTERBOX;
            break;

        case Aspect_Mode::FULLSCREEN_16x9: //NAO SEI QUAL É
            en_asp_ratio = AUI_DIS_AP_16_9;
            en_asp_mode = AUI_DIS_MM_NORMAL_SCALE;
            break;

        case Aspect_Mode::PANSCAN_4x3:
            en_asp_ratio = AUI_DIS_AP_4_3;
            en_asp_mode = AUI_DIS_MM_PANSCAN;
            break;

        case Aspect_Mode::LETTERBOX_4x3:
            en_asp_ratio = AUI_DIS_AP_4_3;
            en_asp_mode = AUI_DIS_MM_LETTERBOX;
            break;

        case Aspect_Mode::FULLSCREEN_4x3: //NAO SEI QUAL É
            en_asp_ratio = AUI_DIS_AP_4_3;
            en_asp_mode = AUI_DIS_MM_NORMAL_SCALE;
            break;

        case Aspect_Mode::AUTO:
        default:
            en_asp_ratio = AUI_DIS_AP_AUTO;
            en_asp_mode = {};
            break;
    }

    auto hnd = m_p->hnd;
    //ALI_EXEC(aui_dis_match_mode_set(hnd, en_asp_mode));
    ALI_EXEC(aui_dis_aspect_ratio_set_ext(hnd, en_asp_ratio, en_asp_mode));
}

Aspect_Mode Display::get_aspect_mode()
{
    Aspect_Mode result = Aspect_Mode::AUTO;
    enum dis_display_mode display_match_mode = DIS_DM_PANSCAN;
    enum dis_aspect_ratio aspect_ratio = DIS_AR_AUTO;
    /*
        ALI_EXEC(alisldis_get_display_mode(m_p->hnd, &display_match_mode));
        ALI_EXEC(alisldis_get_aspect(m_p->hnd, &aspect_ratio));
        aui_dis_aspect_ratio en_asp_ratio;
        aui_dis_match_mode en_asp_mode;

        if (aspect_ratio == DIS_AR_16_9)
            en_asp_ratio = AUI_DIS_AP_16_9;
        else if (aspect_ratio == DIS_AR_4_3)
            en_asp_ratio = AUI_DIS_AP_4_3;
        else
            en_asp_ratio = AUI_DIS_AP_AUTO;

        switch (display_match_mode)
        {

            case DIS_DM_PILLBOX:
                result = Aspect_Mode::PILLBOX_16x9;
                break;

            case DIS_DM_PANSCAN:
                if (en_asp_ratio == AUI_DIS_AP_16_9)
                {
                    result = Aspect_Mode::PANSCAN_16x9;
                }
                else
                {
                    result = Aspect_Mode::PANSCAN_4x3;
                }
                break;

            case DIS_DM_LETTERBOX:
                if (en_asp_ratio == AUI_DIS_AP_16_9)
                {
                    result = Aspect_Mode::LETTERBOX_16x9;
                }
                else
                {
                    result = Aspect_Mode::LETTERBOX_4x3;
                }
                break;

            case DIS_DM_NORMAL_SCALE:
            default:
                if (en_asp_ratio == AUI_DIS_AP_16_9)
                {
                    result = Aspect_Mode::FULLSCREEN_16x9;
                }
                else
                {
                    result = Aspect_Mode::FULLSCREEN_4x3;
                }
                break;
        }

        if (en_asp_ratio == AUI_DIS_AP_AUTO)
        {
            result = Aspect_Mode::AUTO;
        }
    */
    return result;
}

void Display::set_resolution_standard(Resolution_Standard _resolution_standard)
{
    uint8_t progressive;
    aui_dis_tvsys tvsys;
    tvsys = _resolution_standard == Resolution_Standard::_480i_60Hz ? AUI_DIS_TVSYS_NTSC :
            _resolution_standard == Resolution_Standard::_480p_60Hz ? AUI_DIS_TVSYS_NTSC :
            _resolution_standard == Resolution_Standard::_720p_60Hz ? AUI_DIS_TVSYS_LINE_720_60 :
            _resolution_standard == Resolution_Standard::_1080i_30Hz ? AUI_DIS_TVSYS_LINE_1080_30 : AUI_DIS_TVSYS_LINE_1080_60;

    if((_resolution_standard == Resolution_Standard::_480i_60Hz) ||
            (_resolution_standard == Resolution_Standard::_1080i_30Hz))
    {
        progressive = 0;
    }
    else
    {
        progressive = 1;
    }

    auto hnd = m_p->hnd;
    unsigned int ui_ary_num = 4;
    unsigned int ui_dac_attr[4] = {0xFF, 0xFF, 0xFF, AUI_DIS_TYPE_CVBS};
    ALI_EXEC(aui_dis_dac_unreg(hnd, AUI_DIS_TYPE_UNREG_CVBS));
    ALI_EXEC(aui_dis_tv_system_set(hnd, tvsys, progressive));

    if(_resolution_standard == Resolution_Standard::_480i_60Hz)
    {
        unsigned int tve_src = AUI_DIS_SD;
        ALI_EXEC(aui_dis_set(hnd, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
    }
    else if(_resolution_standard == Resolution_Standard::_480p_60Hz)
    {
        unsigned int tve_src = AUI_DIS_HD;
        ALI_EXEC(aui_dis_set(hnd, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
    }

    ALI_EXEC(aui_dis_dac_reg(hnd, ui_dac_attr, ui_ary_num));
}

Resolution_Standard Display::get_resolution_standard()
{
    auto result = Resolution_Standard::_1080i_30Hz;
    auto hnd = m_p->hnd;
    aui_dis_tvsys tvsys;
    unsigned char puc_b_progressive;
    aui_dis_tv_system_get(hnd, &tvsys, &puc_b_progressive);

    switch(tvsys)
    {
        case AUI_DIS_TVSYS_PAL:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_PAL = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_NTSC:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_NTSC = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_PAL_M:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_PAL_M = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_PAL_N:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_PAL_N = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_PAL_60:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_PAL_60 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_NTSC_443:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_NTSC_443 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_MAC:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_MAC = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_720_50:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_720_50 = " << tvsys << "\n");
            //result = Resolution_Standard::_720p_50Hz;
            break;

        case AUI_DIS_TVSYS_LINE_720_60:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_720_60 = " << tvsys << "\n");
            result = Resolution_Standard::_720p_60Hz;
            break;

        case AUI_DIS_TVSYS_LINE_1080_25:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1080_25 = " << tvsys << "\n");
            //result = Resolution_Standard::_1080p_25Hz;
            break;

        case AUI_DIS_TVSYS_LINE_1080_30:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1080_30 = " << tvsys << "\n");
            //result = Resolution_Standard::_1080p_30Hz;
            break;

        case AUI_DIS_TVSYS_LINE_1080_50:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1080_50 = " << tvsys << "\n");
            //result = Resolution_Standard::_1080p_50Hz;
            break;

        case AUI_DIS_TVSYS_LINE_1080_60:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1080_60 = " << tvsys << "\n");
            result = Resolution_Standard::_1080p_60Hz;
            break;

        case AUI_DIS_TVSYS_LINE_1080_24:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1080_24 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_1152_ASS:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1152_ASS = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_1080_ASS:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_1152_ASS = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_PAL_NC:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_PAL_NC = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_3840X2160_24:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_3840X2160_24 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_3840X2160_25:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_3840X2160_25 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_3840X2160_30:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_3840X2160_30 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_3840X2160_50:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_3840X2160_50 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_3840X2160_60:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_3840X2160_60 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_4096X2160_24:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_4096X2160_24 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_4096X2160_25:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_4096X2160_25 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_4096X2160_30:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_4096X2160_30 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_4096X2160_50:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_4096X2160_50 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_LINE_4096X2160_60:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_LINE_4096X2160_60 = " << tvsys << "\n");
            break;

        case AUI_DIS_TVSYS_NTSC_J:
            DEBUG_MSG(HAL, DEBUG, "AUI_DIS_TVSYS_NTSC_J = " << tvsys << "\n");
            break;

        default:
            DEBUG_MSG(HAL, ERROR, "UNDEFINED = " << tvsys << "\n");
            break;
    }

    return result;
}

} // namespace mb
