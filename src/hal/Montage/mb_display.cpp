#include "mb_display.h"
#include "mb_globals.h"
#include "mt_unf_disp.h"

#include <atomic>

#define MT_DAC_CVBS 0 //Rock_hu 自己增加定义以后删除
#define MT_DAC_YPBPR_Y 1
#define MT_DAC_YPBPR_PB 2
#define MT_DAC_YPBPR_PR 3

/* DAC */
#define DAC_CVBS MT_DAC_CVBS
#define DAC_YPBPR_Y MT_DAC_YPBPR_Y
#define DAC_YPBPR_PB MT_DAC_YPBPR_PB
#define DAC_YPBPR_PR MT_DAC_YPBPR_PR

SINGULARITY_TOKEN

namespace mb {

Display::Display()
{
    SINGULARITY_CHECK(false);
    auto const format = MT_UNF_ENC_FMT_1080i_60;
    MT_EXEC(MT_UNF_DISP_Init());
    MT_UNF_DISP_INTF_S interfaces[2];
    MB_ZERO(interfaces);
    /* set display1 interface */
    interfaces[0].enIntfType                = MT_UNF_DISP_INTF_TYPE_YPBPR;
    interfaces[0].unIntf.stYPbPr.u8DacY     = DAC_YPBPR_Y;
    interfaces[0].unIntf.stYPbPr.u8DacPb    = DAC_YPBPR_PB;
    interfaces[0].unIntf.stYPbPr.u8DacPr    = DAC_YPBPR_PR;
    interfaces[1].enIntfType                = MT_UNF_DISP_INTF_TYPE_HDMI;
    interfaces[1].unIntf.enHdmi             = MT_UNF_HDMI_ID_0;
    MT_EXEC(MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY1, &interfaces[0], 2));
    /* set display0 interface */
    interfaces[0].enIntfType            = MT_UNF_DISP_INTF_TYPE_CVBS;
    interfaces[0].unIntf.stCVBS.u8Dac   = DAC_CVBS;
    MT_EXEC(MT_UNF_DISP_AttachIntf(MT_UNF_DISPLAY0, &interfaces[0], 1));
    MT_EXEC(MT_UNF_DISP_Attach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1));
    /* set display1 format*/
    MT_EXEC(MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, format));

    switch(format)
    {
        case MT_UNF_ENC_FMT_1080P_60:
        case MT_UNF_ENC_FMT_1080i_60:
        case MT_UNF_ENC_FMT_1080P_30:
        case MT_UNF_ENC_FMT_1080P_24:
        case MT_UNF_ENC_FMT_720P_60:
        case MT_UNF_ENC_FMT_480P_60:
        case MT_UNF_ENC_FMT_NTSC:
        case MT_UNF_ENC_FMT_4096X2160_24:
        case MT_UNF_ENC_FMT_3840X2160_30:
        case MT_UNF_ENC_FMT_3840X2160_24:
        {
            MT_EXEC(MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_NTSC));
            break;
        }

        case MT_UNF_ENC_FMT_1080P_50:
        case MT_UNF_ENC_FMT_1080i_50:
        case MT_UNF_ENC_FMT_1080P_25:
        case MT_UNF_ENC_FMT_720P_50:
        case MT_UNF_ENC_FMT_576P_50:
        case MT_UNF_ENC_FMT_PAL:
        case MT_UNF_ENC_FMT_3840X2160_25:
        {
            MT_EXEC(MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL));
            break;
        }
    }
}

Display::~Display()
{
    MT_EXEC(MT_UNF_DISP_Close(MT_UNF_DISPLAY1));
    MT_EXEC(MT_UNF_DISP_Close(MT_UNF_DISPLAY0));
    MT_EXEC(MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1));
    MT_EXEC(MT_UNF_DISP_DeInit());
    SINGULARITY_CHECK(true);
}

void Display::clear()
{
    MT_UNF_DISP_BG_COLOR_S bg_color;
    MB_ZERO(bg_color);
    MT_EXEC(MT_UNF_DISP_SetBgColor(MT_UNF_DISPLAY1, &bg_color));
    MT_EXEC(MT_UNF_DISP_Open(MT_UNF_DISPLAY1));
    MT_EXEC(MT_UNF_DISP_Open(MT_UNF_DISPLAY0));
}

void Display::take_snapshot()
{
    // MT_UNF_VIDEO_FRAME_INFO_S frame;
    // MB_ZERO(frame);
    // MT_UNF_DISP_AcquireSnapshot(MT_UNF_DISPLAY1, &frame);
    // MT_UNF_DISP_ReleaseSnapshot(MT_UNF_DISPLAY1, &frame);
}


} // namespace mb
