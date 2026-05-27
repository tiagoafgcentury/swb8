#ifndef __AUI_TEST_STREAM_PLAY_NVCA_H_
#define __AUI_TEST_STREAM_PLAY_NVCA_H_

#include "lvgl.h"

#define HDCP_ENC_KEY_LEN    (288)
#define HDCP_KEY_SAVE_PATH    "/usr/mnt_app/hdcpkey"
#ifdef __cplusplus
extern "C" {
#endif
unsigned long tpm_service_play(unsigned long para_num, char **service_para);
unsigned long tpm_service_stop(void);
void get_audio_description_pid(unsigned short *ad_pid);
unsigned long tpm_write_hdcp_key(unsigned char *hdcpKeyData, int key_len);
unsigned long tpm_write_hdcp_decrypt_key(unsigned char *hdcp_protect, int hdcp_protect_len);
unsigned long tpm_power_off(void);
bool tpm_set_lnb_power(int lnb_power);

lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color);
lv_obj_t *set_label_text(lv_obj_t *bgd, const char *text, int x, int y, lv_font_t *font, lv_color_t fontColor);


#ifdef __cplusplus
}
#endif



#endif
