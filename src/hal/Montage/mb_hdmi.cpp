#include "mb_hdmi.h"
#include "mb_globals.h"

#include <atomic>

#include <mt_unf_hdmi.h>
#undef min
#undef max

SINGULARITY_TOKEN

namespace {

MT_UNF_HDMI_CALLBACK_FUNC_S g_hdmi_callback;

}

namespace mb {

HDMI::HDMI()
{
    SINGULARITY_CHECK(false);
    MT_EXEC(MT_UNF_HDMI_Init());
    MT_UNF_HDMI_DELAY_S delay;
    MB_ZERO(delay);
    MT_EXEC(MT_UNF_HDMI_GetDelay(MT_UNF_HDMI_ID_0, &delay));
    delay.bForceFmtDelay = MT_TRUE;
    delay.bForceMuteDelay = MT_TRUE;
    delay.u32FmtDelay = 500;
    delay.u32MuteDelay = 120;
    MT_EXEC(MT_UNF_HDMI_SetDelay(MT_UNF_HDMI_ID_0, &delay));
    MB_ZERO(g_hdmi_callback);
    g_hdmi_callback.pfnHdmiEventCallback = reinterpret_cast<MT_UNF_HDMI_CALLBACK>(hdmi_callback);
    g_hdmi_callback.pPrivateData = this;
    MT_EXEC(MT_UNF_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_0, &g_hdmi_callback));
    MT_UNF_HDMI_OPEN_PARA_S open_param;
    MB_ZERO(open_param);
    open_param = {MT_UNF_HDMI_DEFAULT_ACTION_HDMI, HDMI_INITIALIZED};
    MT_EXEC(MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &open_param));
    hdmi_init();
}

HDMI::~HDMI()
{
    MT_EXEC(MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_0));
    MT_EXEC(MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_0));
    MT_EXEC(MT_UNF_HDMI_UnRegCallbackFunc(MT_UNF_HDMI_ID_0, &g_hdmi_callback));
    MT_EXEC(MT_UNF_HDMI_DeInit());
    MB_ZERO(g_hdmi_callback);
    SINGULARITY_CHECK(true);
}

void HDMI::hdmi_callback(int event, void *_data)
{
    auto thiz { static_cast<HDMI *>(_data) };

    switch(event)
    {
        case MT_UNF_HDMI_EVENT_HOTPLUG:
        {
            DEBUG_MSG("HDMI EVENT: Hotplug:\n");
            thiz->hdmi_init();
            return;
        }

        case MT_UNF_HDMI_EVENT_NO_PLUG:
            DEBUG_MSG("HDMI EVENT: No plug\n");
            break;

        case MT_UNF_HDMI_EVENT_EDID_FAIL:
            DEBUG_MSG("HDMI EVENT: EDID Fail\n");
            break;

        case MT_UNF_HDMI_EVENT_HDCP_FAIL:
            DEBUG_MSG("HDMI EVENT: HDCP Fail\n");
            break;

        case MT_UNF_HDMI_EVENT_HDCP_SUCCESS:
            DEBUG_MSG("HDMI EVENT: HDCP Success\n");
            break;

        case MT_UNF_HDMI_EVENT_RSEN_CONNECT:
            DEBUG_MSG("HDMI EVENT: RSEN Connect\n");
            break;

        case MT_UNF_HDMI_EVENT_RSEN_DISCONNECT:
            DEBUG_MSG("HDMI EVENT: RSEN Disconnect\n");
            break;

        case MT_UNF_HDMI_EVENT_HDCP_USERSETTING:
            DEBUG_MSG("HDMI EVENT: HDCP Usersetting\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY:
            DEBUG_MSG("HDMI EVENT: CEC Standby\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_OK:
            DEBUG_MSG("HDMI EVENT: CEC OK\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_UP:
            DEBUG_MSG("HDMI EVENT: CEC Up\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_DOWN:
            DEBUG_MSG("HDMI EVENT: CEC Down\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_LEFT:
            DEBUG_MSG("HDMI EVENT: CEC Left\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_RIGHT:
            DEBUG_MSG("HDMI EVENT: CEC Right\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_UP:
            DEBUG_MSG("HDMI EVENT: CEC Vol+\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_DOWN:
            DEBUG_MSG("HDMI EVENT: CEC Vol-\n");
            break;

        case MT_UNF_HDMI_EVENT_TV_SEND_CEC_MUTE:
            DEBUG_MSG("HDMI EVENT: CEC Mute\n");
            break;
    }
}

void HDMI::hdmi_init()
{
    MT_UNF_HDMI_ATTR_S hdmi_attr;
    MB_ZERO(hdmi_attr);
    MT_UNF_EDID_BASE_INFO_S sink_cap;
    MB_ZERO(sink_cap);
    MT_UNF_HDMI_STATUS_S hdmi_status;
    MB_ZERO(hdmi_status);
#ifdef MT_HDCP_SUPPORT
    static mt_u8 u8FirstTimeSetting = MT_TRUE;
#endif
    MT_EXEC(MT_UNF_HDMI_GetStatus(MT_UNF_HDMI_ID_0, &hdmi_status));

    if(MT_FALSE == hdmi_status.bConnected)
    {
        DEBUG_MSG("\tNO CONNECTION!" << endl);
        return;
    }

    MT_EXEC(MT_UNF_HDMI_GetAttr(MT_UNF_HDMI_ID_0, &hdmi_attr));
    auto ret = MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_0, &sink_cap);

    if(ret == MT_SUCCESS)
    {
        //hdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
        if(MT_TRUE == sink_cap.bSupportHdmi)
        {
            hdmi_attr.bEnableHdmi = MT_TRUE;

            if(MT_TRUE != sink_cap.stColorSpace.bYCbCr444)
            {
                hdmi_attr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
            }
        }
        else
        {
            hdmi_attr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
            //读取到了edid，并且不支持hdmi则进入dvi模式
            //read real edid ok && sink not support hdmi,then we run in dvi mode
            hdmi_attr.bEnableHdmi = MT_FALSE;
        }
    }
    else
    {
        //when get capability fail,use default mode
        hdmi_attr.bEnableHdmi = MT_FALSE;
    }

    if(MT_TRUE == hdmi_attr.bEnableHdmi)
    {
        hdmi_attr.bEnableAudio = MT_TRUE;
        hdmi_attr.bEnableVideo = MT_TRUE;
        hdmi_attr.bEnableAudInfoFrame = MT_TRUE;
        hdmi_attr.bEnableAviInfoFrame = MT_TRUE;
    }
    else
    {
        hdmi_attr.bEnableAudio = MT_FALSE;
        hdmi_attr.bEnableVideo = MT_TRUE;
        hdmi_attr.bEnableAudInfoFrame = MT_FALSE;
        hdmi_attr.bEnableAviInfoFrame = MT_FALSE;
        hdmi_attr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

#ifdef MT_HDCP_SUPPORT

    if(u8FirstTimeSetting == MT_TRUE)
    {
        u8FirstTimeSetting = MT_FALSE;

        if(g_HDCPFlag == MT_TRUE)
        {
            hdmiAttr.bHDCPEnable = MT_TRUE; //Enable HDCP
        }
        else
        {
            hdmiAttr.bHDCPEnable = MT_FALSE;
        }
    }
    else
    {
        //HDCP Enable use default setting!!
    }

#endif
    MT_EXEC(MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_0));
    MT_EXEC(MT_UNF_HDMI_SetAttr(MT_UNF_HDMI_ID_0, &hdmi_attr));
    /* MT_UNF_HDMI_SetAttr must before MT_UNF_HDMI_Start! */
    MT_EXEC(MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0));
#ifndef NDEBUG
    DEBUG_MSG("\tEnableHdmi: " << dec << (int)hdmi_attr.bEnableHdmi <<
              "\n\tbEnableVideo: " << hdmi_attr.bEnableVideo <<
              "\n\tenVidOutMode: " << hdmi_attr.enVidOutMode <<
              "\n\tenDeepColorMode: " << hdmi_attr.enDeepColorMode <<
              "\n\tbxvYCCMode: " << hdmi_attr.bxvYCCMode <<
              "\n\tbEnableAudio: " << hdmi_attr.bEnableAudio <<
              "\n\tbEnableAviInfoFrame: " << hdmi_attr.bEnableAviInfoFrame <<
              "\n\tbEnableAudInfoFrame: " << hdmi_attr.bEnableAudInfoFrame <<
              "\n\tbEnableSpdInfoFrame: " << hdmi_attr.bEnableSpdInfoFrame <<
              "\n\tbEnableMpegInfoFrame: " << hdmi_attr.bEnableMpegInfoFrame <<
              endl);
    MT_UNF_HDMI_STATUS_S status;
    MT_UNF_HDMI_GetStatus(MT_UNF_HDMI_ID_0, &status);
    DEBUG_MSG("HDMI Status: " <<
              "\n\tAuthed: " << status.bAuthed <<
              "\n\tConnected: " << status.bConnected <<
              "\n\tPower ON: " << status.bSinkPowerOn <<
              "\n"
             );
#endif
    MT_UNF_HDMI_CEC_CMD_S cec_cmd;
    MB_ZERO(cec_cmd);
    cec_cmd.enSrcAdd = MT_UNF_CEC_LOGICALADD_TUNER_1;
    cec_cmd.enDstAdd = MT_UNF_CEC_LOGICALADD_TV;
    cec_cmd.u8Opcode = CEC_OPCODE_IMAGE_VIEW_ON;
    MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_0, &cec_cmd);
}

}
