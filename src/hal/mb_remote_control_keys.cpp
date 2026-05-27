#include "mb_remote_control_keys.h"

namespace mb {

#warning "Implementar função to_int e to_str para converter as teclas do controle remoto para inteiro e string, respectivamente. Isso é necessário para processar a sequência de teclas corretamente."
int to_int(Remote_Control_Key _key)
{
    switch(_key)
    {
        case Remote_Control_Key::KEY_0:
            return 0;
        case Remote_Control_Key::KEY_1:
            return 1;
        case Remote_Control_Key::KEY_2:
            return 2;
        case Remote_Control_Key::KEY_3:
            return 3;   
        case Remote_Control_Key::KEY_4:
            return 4;
        case Remote_Control_Key::KEY_5:
            return 5;
        case Remote_Control_Key::KEY_6:
            return 6;
        case Remote_Control_Key::KEY_7:
            return 7;
        case Remote_Control_Key::KEY_8:
            return 8;   
        case Remote_Control_Key::KEY_9:
            return 9;
        default:
            return -1;
    }
}

char to_char(Remote_Control_Key _key)
{
    auto key_int = to_int(_key);
    if (key_int >= 0 && key_int <= 9)    {
        return '0' + key_int;
    }
    return '\0';
}

std::string_view to_str(Remote_Control_Key _key)
{
    switch(_key)
    {
#define RK(KEY) case Remote_Control_Key::KEY: return # KEY
            RK(KEY_0);
            RK(KEY_1);
            RK(KEY_2);
            RK(KEY_3);
            RK(KEY_4);
            RK(KEY_5);
            RK(KEY_6);
            RK(KEY_7);
            RK(KEY_8);
            RK(KEY_9);
            RK(KEY_BLUE);
            RK(KEY_CC);
            RK(KEY_CHDOWN);
            RK(KEY_CHUP);
            RK(KEY_FWD);
            RK(KEY_RED);
            RK(KEY_GREEN);
            RK(KEY_ORANGE);
            RK(KEY_YELLOW);
            RK(KEY_REV);
            RK(KEY_REC);
            RK(KEY_SELECT);
            RK(KEY_INFO);
            RK(KEY_LAST);
            RK(KEY_LR);
            RK(KEY_MENU);
            RK(KEY_MUTE);
            RK(KEY_NEXT);
            RK(KEY_PAUSE);
            RK(KEY_PLAY);
            RK(KEY_POWER);
            RK(KEY_PREV);
            RK(KEY_GUIDE);
            RK(KEY_SLEEP);
            RK(KEY_STOP);
            RK(KEY_TVRADIO);
            RK(KEY_VOLDOWN);
            RK(KEY_VOLTAR);
            RK(KEY_VOLUP);
            RK(KEY_PLUS);
            RK(KEY_OK);
            RK(KEY_UPGRADE);
            RK(FRONT_PANEL_0);
    }

    return "<UNKNOWN>";
}

}
