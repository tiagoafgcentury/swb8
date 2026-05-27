#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_tsg.h>
#include <aui_nim.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_av.h>
#include <aui_common.h>
#include <aui_misc.h>
#include <aui_otp.h>
#include <sys/stat.h>

#include "tpm_init.h"
#include "tpm_api.h"


#define NIM_TIMEOUT_MS 1000

static unsigned long s_play_mode = 0;
aui_nim_demod_type _to_aui_nim_demod_type(const char *input);

// Global flag of play state
static unsigned long *g_data_path = NULL;

static struct tpm_modules_init_para g_init_play_para;

static unsigned short static_ad_pid = AUI_INVALID_PID;

static struct ali_aui_hdls *s_p_hdls = NULL;

unsigned long tpm_service_play(unsigned long para_num, char **service_para)
{
    memset(&g_init_play_para, 0, sizeof(g_init_play_para));

    if (s_p_hdls == NULL)
    {
        s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
    }

    if (NULL == s_p_hdls)
    {
        printf("malloc fail\n");
        return -1;
    }

    MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
    /*init all used device in this mode*/
    //ali_aui_init_para_for_test_nim(argc, argv, &g_init_play_para);
    ali_aui_init_para_for_test_nim(&para_num, service_para, &g_init_play_para);

    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    if (aui_nim_init(NULL))
    {
        printf("\nnim init error\n");
        goto err_live;
    }

    /*open and init display device*/
    if (tpm_dis_init(g_init_play_para.init_para_dis, &s_p_hdls->dis_hdl))
    {
        printf("\r\n tpm_dis_init failed!");
        goto err_live;
    }

    printf("AUI DIS opened\n");

    // Rivaille.Zhu: change nim_connect location due to masoic issue
    // when live stream playing starts to execute in aui sample test.
    if (nim_connect(&g_init_play_para.init_para_nim, &s_p_hdls->nim_hdl))
    {
        printf("\nnim connect error\n");
        goto err_live;
    }

    printf("nim connect success\n");

    /*if (tpm_tsi_init(&g_init_play_para.init_para_tsi, &s_p_hdls->tsi_hdl)) {
        printf("\r\n tpm_tsi_init failed!");
        goto err_live;
    }
    printf("AUI TSI opened\n");*/

    /*init decv device */
    if (tpm_decv_init(g_init_play_para.init_para_decv, &s_p_hdls->decv_hdl))
    {
        printf("\r\n tpm_decv_init failed!");
        goto err_live;
    }

    printf("AUI DECV opened\n");

    /*init deva device*/
    if (tpm_audio_init(
                g_init_play_para.init_para_audio, &s_p_hdls->deca_hdl, &s_p_hdls->snd_hdl))
    {
        printf("\r\n tpm_audio_init failed!");
        goto err_live;
    }

    printf("AUI audio opened[%08x]\n", (int)s_p_hdls->deca_hdl);

    /*set video and audio  synchronous ways,signal from nim,set to AUI_STC_AVSYNC_PCR.
     *signal from tsg,set to AUI_STC_AVSYNC_AUDIO*/
    if (aui_decv_sync_mode(s_p_hdls->decv_hdl, AUI_STC_AVSYNC_PCR))
    {
        printf("Set AUI_STC_AVSYNC_PCR fail\n");
        goto err_live;
    }

    /* In TDS, create av streams should be done after audio and video init
       because DMX needs the right format for audio and video
    */
    /*set vpid,apid,pcr and create av stream*/
    if (set_dmx_for_create_av_stream(g_init_play_para.dmx_create_av_para.dmx_id,
                                     g_init_play_para.dmx_create_av_para.video_pid,
                                     g_init_play_para.dmx_create_av_para.audio_pid,
                                     g_init_play_para.dmx_create_av_para.audio_desc_pid,
                                     g_init_play_para.dmx_create_av_para.pcr_pid,
                                     &s_p_hdls->dmx_hdl))
    {
        printf("\r\n set dmx failed!");
        goto err_live;
    }

    printf("AUI DMX opened[%08x]\n", (int)s_p_hdls->dmx_hdl);
    aui_dmx_data_path path;
    MEMSET(&path, 0, sizeof(aui_dmx_data_path));
    path.data_path_type                           = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    g_init_play_para.dmx_create_av_para.data_path = (unsigned long)path.data_path_type;

    if (aui_dmx_data_path_set(s_p_hdls->dmx_hdl, &path))
    {
        printf("\r\n aui_dmx_data_path_set failed\n");
        goto err_live;
    }

    printf("dmx data path set %d\n", path.data_path_type);

    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices
     *before enabling DMX AV stream*/
    if (0 == s_play_mode)
    {
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL))
        {
            printf("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_live;
        }
    }
    else if (1 == s_play_mode)
    {
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            printf("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
        }
    }
    else
    {
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
        {
            printf("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
        }
    }

    if (nim_get_signal_info(&g_init_play_para.init_para_nim))
    {
        goto err_live;
    }

    g_data_path = &g_init_play_para.dmx_create_av_para.data_path;
    return AUI_RTN_SUCCESS;
err_live:
    printf("play error\n");
    g_data_path = NULL;
    tpm_deinit(s_p_hdls);
    FREE(s_p_hdls);
    s_p_hdls = NULL;
    return AUI_RTN_FAIL;
}

unsigned long tpm_service_stop(void)
{
    if (NULL == s_p_hdls)
    {
        printf("Is not playing state, no need to stop\n");
        return AUI_RTN_SUCCESS;
    }

    tpm_deinit(s_p_hdls);
    FREE(s_p_hdls);
    s_p_hdls      = NULL;
    static_ad_pid = AUI_INVALID_PID;
    g_data_path = NULL;
    return AUI_RTN_SUCCESS;
}

void get_audio_description_pid(unsigned short *ad_pid)
{
    if (!ad_pid)
    {
        return;
    }

    *ad_pid = static_ad_pid;
}

unsigned char uc_mute_flag = 0;

// E.g. sby pm,time,NEC,60df609F,60df9867,01-01,23:59:00
unsigned long tpm_power_off(void)
{
    aui_standby_setting setting;
    struct tm rtc_tm;
    struct tm alarm_tm;
    char *tailptr = NULL;
    int ret       = 0;
    aui_hdl p_hdl_snd;
    unsigned char uc_mute_enable = 0;
    memset(&setting, 0, sizeof(setting));
    setting.state = AUI_POWER_PMU_STANDBY;
    setting.display_mode = AUI_PANNEL_SHOW_OFF;
    //setting.display_mode = AUI_PANNEL_SHOW_ON;
    setting.standby_key[0].protocol = AUI_IR_TYPE_NEC;
    setting.standby_key[0].code     = 0x60F73BC4; // RED key of ALi IR remote
    setting.standby_key[1].protocol = AUI_IR_TYPE_NEC;
    setting.standby_key[1].code     = 0x60DF9867; // SLEEP key of ALi IR remote
    setting.wakeup_time = NULL;
    AUI_PRINTF("\n");
    AUI_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    if (setting.state == AUI_POWER_PMU_STANDBY)
    {
        //====================end
        AUI_PRINTF("PMU standby\n");
    }
    else
    {
        AUI_PRINTF("PM(fast) standby\n");
    }

    // for gpio mute set
    if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &p_hdl_snd))
    {
        aui_attr_snd attr_snd;
        MEMSET(&attr_snd, 0, sizeof(aui_attr_snd));
        ret = aui_snd_open(&attr_snd, &p_hdl_snd);
    }

    ret = aui_snd_mute_get(p_hdl_snd, &uc_mute_enable);
    AUI_PRINTF("snd mute status:%d\n", uc_mute_enable);

    if (uc_mute_enable == 0)
    {
        ret          = aui_snd_mute_set(p_hdl_snd, 1);
        uc_mute_flag = 1;
    }
    else
    {
        uc_mute_flag = 0;
    }

    AUI_PRINTF("Display mode: %s\n", intmap_bname(setting.display_mode, sby_mode_map));
    AUI_PRINTF("Wake up IR key: %s %#X %#X\n",
               intmap_bname(setting.standby_key[0].protocol, ir_prot_map),
               setting.standby_key[0].code,
               setting.standby_key[1].code);

    if (setting.wakeup_time != NULL)
    {
        AUI_PRINTF("RTC time:     %04u-%02u-%02u %02u:%02u:%02u\n",
                   rtc_tm.tm_year + 1900,
                   rtc_tm.tm_mon + 1,
                   rtc_tm.tm_mday,
                   rtc_tm.tm_hour,
                   rtc_tm.tm_min,
                   rtc_tm.tm_sec);
        AUI_PRINTF("Wake up time:      %02u-%02u %02u:%02u:%02u\n",
                   setting.wakeup_time->tm_mon + 1,
                   setting.wakeup_time->tm_mday,
                   setting.wakeup_time->tm_hour,
                   setting.wakeup_time->tm_min,
                   setting.wakeup_time->tm_sec);
    }
    else
    {
        AUI_PRINTF("Wake up time is not set.\n");
    }

    AUI_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    AUI_PRINTF("\n");

    if (setting.state == AUI_POWER_PM_STANDBY)
    {
        //jobs_before_str();
    }

    ret = aui_standby_set_state(setting);

    if (setting.state == AUI_POWER_PM_STANDBY)
    {
        //jobs_after_resume();
    }

    if (uc_mute_flag == 1)
    {
        // for gpio mute set
        if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &p_hdl_snd))
        {
            aui_attr_snd attr_snd;
            MEMSET(&attr_snd, 0, sizeof(aui_attr_snd));
            ret = aui_snd_open(&attr_snd, &p_hdl_snd);
        }

        ret = aui_snd_mute_set(p_hdl_snd, 0);
    }

    return ret;
}

unsigned long tpm_write_hdcp_key(unsigned char *hdcpKeyData, int key_len)
{
    int ret = 0;
    FILE *fp = NULL;

    if (key_len != HDCP_ENC_KEY_LEN)
    {
        printf("get filename file len error. key_len=%d\n", key_len);
        return -1;
    }

    fp = fopen(HDCP_KEY_SAVE_PATH, "wb+");

    if (fp == NULL)
    {
        return -1;
    }

    fwrite(hdcpKeyData, 1, key_len, fp);
    fclose(fp);
    return ret;
}

unsigned long tpm_write_hdcp_decrypt_key(unsigned char *hdcp_protect, int hdcp_protect_len)
{
    int ret = 0;
    FILE *fp = NULL;

    if (0 == hdcp_protect_len)
    {
        printf("not need write hdcp_protect data.\n");
        return -1;
    }

    if ((0 != hdcp_protect_len) && (16 != hdcp_protect_len))
    {
        printf("error hdcp_protect data len.\n");
        return -1;
    }

#if 1 // Can be done only one time
    ret = aui_otp_write((0x59) * 4, hdcp_protect, hdcp_protect_len);

    if (ret != 0)
    {
        printf("\naui_otp_write ---- Error\n");
    }

#endif
    return ret;
}

lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color)
{
    lv_obj_t *m_rect = lv_obj_create(bgd);
    lv_obj_set_size(m_rect, w, h);
    lv_obj_set_scrollbar_mode(m_rect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(m_rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(m_rect, color, 0);
    lv_obj_set_style_radius(m_rect, 0, 0);
    lv_obj_align(m_rect, LV_ALIGN_DEFAULT, x, y);
    lv_obj_set_style_pad_all(m_rect, 0, LV_PART_MAIN);
    return m_rect;
}

lv_obj_t *set_label_text(lv_obj_t *bgd, const char *text, int x, int y, lv_font_t *font, lv_color_t fontColor)
{
    lv_obj_t *label;
    label = lv_label_create(bgd);
    lv_obj_set_style_text_font(label,  font, 0);
    lv_obj_set_style_text_color(label, fontColor, 0);
    lv_obj_align(label, LV_ALIGN_DEFAULT, x, y);
    lv_label_set_text(label, text);
    return label;
}

bool tpm_set_lnb_power(int lnb_power)
{
    int nim_dev = 0;
    int power   = lnb_power ? AUI_NIM_LNB_POWER_ON : AUI_NIM_LNB_POWER_OFF;
    aui_hdl hdl = NULL;
    struct aui_nim_attr nim_attr;
    nim_attr.ul_nim_id    = 0;
    nim_attr.en_dmod_type = _to_aui_nim_demod_type("S");
    nim_attr.en_std = AUI_NIM_STD_DVBS;

    if (aui_nim_init(NULL))
    {
        aui_nim_de_init(NULL);
        printf("init error\n");
        return false;
    }

    if (aui_nim_open(&nim_attr, &hdl))
    {
        printf("open error\n");
        return false;
    }

    if (aui_find_dev_by_idx(AUI_MODULE_NIM, (unsigned long)nim_dev, &hdl) != AUI_RTN_SUCCESS)
    {
        printf("AUI_MODULE_NIM %d is not opened\n", nim_dev);
        return false;
    }

    printf("AUI_MODULE_NIM %d is opened\n", nim_dev);

    if (aui_nim_lnb_power_set(hdl, power))
    {
        printf("aui_nim_lnb_power_set error\n");
        return -1;
    }

    printf("aui_nim_lnb_power_set success = %d\n", power);
    return true;
}
