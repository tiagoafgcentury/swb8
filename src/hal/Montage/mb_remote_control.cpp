#include "mb_remote_control.h"
#include "mb_globals.h"

#include "mt_unf_ir.h"
#undef min
#undef max

#define CODE_KEY_POWER      0x23dcef06
#define CODE_KEY_MUTE       0x639cef06
#define CODE_KEY_REC        0x32cdef06
#define CODE_KEY_STOP       0x2fd0ef06
#define CODE_KEY_PLAY       0x728def06
#define CODE_KEY_PAUSE      0x6e91ef06
#define CODE_KEY_REV        0x36c9ef06
#define CODE_KEY_FWD        0x30cfef06
#define CODE_KEY_PREV       0x6798ef06
#define CODE_KEY_NEXT       0x629def06
#define CODE_KEY_GREEN      0x7b84ef06
#define CODE_KEY_YELLOW     0x26d9ef06
#define CODE_KEY_CHUP       0x35caef06
#define CODE_KEY_CHDOWN     0x2dd2ef06
#define CODE_KEY_VOLDOWN    0x6699ef06
#define CODE_KEY_SELECT     0x31ceef06
#define CODE_KEY_VOLUP      0x3ec1ef06
#define CODE_KEY_RED        0x7689ef06
#define CODE_KEY_BLUE       0x6996ef06
#define CODE_KEY_MENU       0x6a95ef06
#define CODE_KEY_CC         0x7e81ef06
#define CODE_KEY_INFO       0x3cc3ef06
#define CODE_KEY_VOLTAR     0x3ac5ef06
#define CODE_KEY_LR         0x7a85ef06
#define CODE_KEY_SLEEP      0x7c83ef06
#define CODE_KEY_1          0x6d92ef06
#define CODE_KEY_2          0x6c93ef06
#define CODE_KEY_3          0x33ccef06
#define CODE_KEY_4          0x718eef06
#define CODE_KEY_5          0x708fef06
#define CODE_KEY_6          0x37c8ef06
#define CODE_KEY_7          0x758aef06
#define CODE_KEY_8          0x748bef06
#define CODE_KEY_9          0x3bc4ef06
#define CODE_KEY_TVRADIO    0x7788ef06
#define CODE_KEY_0          0x7887ef06
#define CODE_KEY_LAST       0x7d82ef06
#define CODE_KEY_ORANGE     0x6897ef06

namespace mb {

Remote_Control::Remote_Control()
{
    MT_EXEC(MT_UNF_IR_Init());
    MT_EXEC(MT_UNF_IR_EnableRepKey(MT_TRUE));
    MT_EXEC(MT_UNF_IR_SetRepKeyTimeoutAttr(400));
    MT_EXEC(MT_UNF_IR_SetFetchMode(0));
    MT_EXEC(MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC));
    set_standby_wakeup();
}

Remote_Control::~Remote_Control()
{
    MT_EXEC(MT_UNF_IR_DeInit());
}

void Remote_Control::set_standby_wakeup()
{
    ir_wavefilter_config_s wavefiler;
    MB_ZERO(wavefiler);
    wavefiler.irda_wfilt_channel = 1;
    wavefiler.irda_wfilt_channel_cfg[0].protocol = IRDA_NEC;
    wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
    wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = CODE_KEY_POWER;
    MT_EXEC(MT_UNF_IR_SetWaveFilter(wavefiler));
}

void Remote_Control::read_keys()
{
    mt_u64 key_code;
    char name[64];
    MT_UNF_KEY_STATUS_E status;
    uint read_timeout { 1u };
    auto ret { MT_UNF_IR_GetValueWithProtocol(&status, &key_code, name, sizeof(name), read_timeout) };

    if(MT_SUCCESS == ret)
    {
        if(status != MT_UNF_KEY_STATUS_DOWN)
        {
            return;
        }

        Remote_Control_Key key;

        switch(key_code)
        {
#define CK(K) case CODE_ ## K: key = Remote_Control_Key::  K; break
                CK(KEY_POWER);
                CK(KEY_MUTE);
                CK(KEY_REC);
                CK(KEY_STOP);
                CK(KEY_PLAY);
                CK(KEY_PAUSE);
                CK(KEY_REV);
                CK(KEY_FWD);
                CK(KEY_PREV);
                CK(KEY_NEXT);
                CK(KEY_GREEN);
                CK(KEY_YELLOW);
                CK(KEY_CHUP);
                CK(KEY_CHDOWN);
                CK(KEY_VOLDOWN);
                CK(KEY_SELECT);
                CK(KEY_VOLUP);
                CK(KEY_RED);
                CK(KEY_BLUE);
                CK(KEY_MENU);
                CK(KEY_CC);
                CK(KEY_INFO);
                CK(KEY_VOLTAR);
                CK(KEY_LR);
                CK(KEY_SLEEP);
                CK(KEY_1);
                CK(KEY_2);
                CK(KEY_3);
                CK(KEY_4);
                CK(KEY_5);
                CK(KEY_6);
                CK(KEY_7);
                CK(KEY_8);
                CK(KEY_9);
                CK(KEY_TVRADIO);
                CK(KEY_0);
                CK(KEY_LAST);
                CK(KEY_ORANGE);

            default:
                DEBUG_MSG("RC Key not handled: 0x" << setfill('0') << setw(8) << hex << key_code << endl);
        }

        if(m_key_handler)
        {
            m_key_handler(key);
        }
    }
}


} // namespace mb
