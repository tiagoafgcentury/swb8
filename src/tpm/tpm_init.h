#ifndef __TPM_INIT_H__
#define __TPM_INIT_H__

#include "aui_common.h"

struct tpm_nim_init_para
{
    unsigned long ul_nim_type;
    unsigned long ul_freq;
    unsigned long ul_symb_rate;
    unsigned long ul_polar;
    unsigned long ul_src;
    unsigned long ul_freq_band;
    unsigned long ul_device;
    unsigned long ul_nim_type_id;
    unsigned long ul_demod_i2c_type_id;
    unsigned long ul_demod_i2c_addr;
    unsigned long ul_demod_qpsk_cfg;
    unsigned long ul_tuner_i2c_type_id;
    unsigned long ul_tuner_i2c_addr;
    unsigned long ul_tuner_type_id;
    unsigned long ul_nim_std; // added by vedic   value for DVBT
    unsigned int qam_mode;
    long plp_index;              // DVBT2
    long plp_id;                 // DVBT2 and DVBS2
    unsigned long ul_plsn_value; // DVBS2
    long tsn;                    // DVB-S2X
    unsigned long stream_id;
    unsigned char plhs;
    unsigned long plhs_seq_31_0;
    unsigned long plhs_seq_63_32;
    unsigned long plhs_seq_89_64;
    unsigned char priority;
    unsigned char lnbftype;
};

struct tpm_tsi_init_para
{
    // unsigned long ul_hw_init_val;
    unsigned long ul_tsi_id;
    // unsigned long ul_input_src;
    unsigned long ul_tis_port_idx; // tsi channel id
    unsigned long ul_dmx_idx;
    unsigned long ul_nim_index;
    unsigned long ul_transport_type;
};

struct tpm_tsg_init_para
{
    unsigned long ul_tsg_clk;
    unsigned long ul_bit_rate;
    unsigned char raw_packet_size;
};

struct tpm_dmx_create_av_para
{
    int dmx_id;
    unsigned long video_encode_type;
    unsigned long front_end;
    unsigned long nim_chipid;
    unsigned short video_pid;
    unsigned short audio_pid;
    unsigned short audio_desc_pid;
    unsigned short pcr_pid;
    unsigned long data_path;
};

struct tpm_dis_init_para
{
    unsigned long ul_dis_type;
    unsigned long ul_dis_layer;
};

struct tpm_audio_init_para
{
    unsigned long ul_audio_type;
    unsigned long ul_volume; // deprecated
};

struct tpm_deca_init_para
{
    unsigned long ul_audio_type;
};

struct tpm_snd_init_para
{
    unsigned long ul_volume; // deprecated
};

enum api_app_decv_chg_mode
{
    /** show black screen when change program */
    API_CHG_BLACK,
    /** show the last picture of the last program on screen before the new program begins to
       play */
    API_CHG_STILL,
};

typedef void (*api_decv_callback)(void *p_user_hld, unsigned int para1, unsigned int para2);

struct tpm_decv_init_para
{
    unsigned short video_id;
    unsigned long ul_video_type;
    unsigned long ul_preview_enable;
    enum api_app_decv_chg_mode ul_chg_mode;
    api_decv_callback callback;
    void *puser_data;
    unsigned long ul_dis_layer;
};

struct tpm_modules_init_para
{
    struct tpm_nim_init_para init_para_nim;
    struct tpm_tsi_init_para init_para_tsi;
    struct tpm_tsg_init_para init_para_tsg;
    struct tpm_dmx_create_av_para dmx_create_av_para;
    struct tpm_dis_init_para init_para_dis;
    struct tpm_audio_init_para init_para_audio;
    struct tpm_decv_init_para init_para_decv;
    // struct tpm_deca_init_para init_para_deca;
    struct tpm_snd_init_para init_para_snd;
};

struct ali_aui_hdls
{
    aui_hdl nim_hdl;
    aui_hdl tsi_hdl;
    aui_hdl tsg_hdl;
    aui_hdl kl_hdl[2];
    aui_hdl dsc_hdl;
    aui_hdl dmx_hdl;
    aui_hdl decv_hdl;
    aui_hdl deca_hdl;
    aui_hdl snd_hdl;
    aui_hdl dis_hdl;
    aui_hdl av_hdl;
};

/**
 *   @brief      open and init tsi device
 *   @param[in]  *para               tsi devce attribute.
 *   @param[in]  *aui_nim_hdl        tsi devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsi_init(struct tpm_tsi_init_para *para, aui_hdl *p_hdl);

/**
 *   @brief      open and init tsi device and init TS stream pass cic
 *   @param[in]  *para               tsi devce attribute.
 *   @param[in]  *aui_nim_hdl        tsi devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsi_init_cic(struct tpm_tsi_init_para *para, aui_hdl *p_hdl);

/**
 *   @brief      set  tsi signal source and signal route
 *   @param[in]  *para               tsi devce attribute.
 *   @param[in]  *aui_nim_hdl        tsi devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsi_init_extend(struct tpm_tsi_init_para *para, aui_hdl *p_hdl);

/**
 *   @brief      open,init and enable dis device
 *   @param[in]  *init_para_decv     dis devce attribute.
 *   @param[in]  *aui_nim_hdl        dis devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_dis_init(struct tpm_dis_init_para init_para_dis, aui_hdl *p_hdl);

/**
 *   @brief      open and init decv device
 *   @param[in]  *init_para_decv     decv devce attribute.
 *   @param[in]  *aui_nim_hdl        decv devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_decv_init(struct tpm_decv_init_para init_para_decv, aui_hdl *p_hdl);

/**
 *   @brief      open and init deca device
 *   @param[in]  init_para_audio     deca devce attribute.
 *   @param[in]  *aui_nim_hdl        deca devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_audio_init(struct tpm_audio_init_para init_para_audio,
                   aui_hdl *p_hdl_deca,
                   aui_hdl *p_hdl_snd);

/**
 *   @brief      open and init tsg device
 *   @param[in]  *para               tsg devce attribute.
 *   @param[in]  *aui_nim_hdl        tsg devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsg_init(struct tpm_tsg_init_para *para, aui_hdl *p_hdl);

/**
 *   @brief      init related parameters to test stream with nim
 *   @param[in]  *init_para_decv     command line arguement count
 *   @param[in]  **argv              command line arguement.
 *   @param[out] *init_para          store arguement from command line arguements extracted
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int ali_aui_init_para_for_test_nim(unsigned long *argc,
                                   char **argv,
                                   struct tpm_modules_init_para *init_para);

/**
 *   @brief      init related parameters to test stream with tsg
 *   @param[in]  *init_para_decv     command line arguement count
 *   @param[in]  **argv              command line arguement.
 *   @param[out] *init_para          store arguement from command line arguements extracted
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int ali_aui_init_para_for_test_tsg(unsigned long *argc,
                                   char **argv,
                                   struct tpm_modules_init_para *init_para);

/**
 *   @brief      create av stream,this funtion init all related device
 *   @param[in]  *para               av function attribute.
 *   @param[in]  *aui_nim_hdl        av function handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_av_init(struct ali_aui_hdls *p_handles, aui_hdl *p_hdl);

/**
 *   @brief      open nim device and lock signal
 *   @param[in]  *para               nim devce attribute.
 *   @param[in]  *aui_nim_hdl        nim devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int nim_connect(struct tpm_nim_init_para *para, aui_hdl *aui_nim_hdl);

/**
 *   @brief      get signal info after lock signal
 *   @param[in]  *para               nim devce attribute.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int nim_get_signal_info(struct tpm_nim_init_para *para);

/**
 *   @brief      close and deinit nim device
 *   @param[in]  hdl                 nim devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_nim_deinit(aui_hdl hdl);

/**
 *   @brief      close  tsi device
 *   @param[in]  hdl                 tsi devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsi_deinit(aui_hdl hdl);

/**
 *   @brief      close  tsg device
 *   @param[in]  hdl                 tsg devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_tsg_deinit(aui_hdl hdl);

/**
 *   @brief      stop and close dmx device
 *   @param[in]  hdl                 dmx devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_dmx_deinit(aui_hdl hdl);

/**
 *   @brief      stop snd device
 *   @param[in]  hdl                 snd devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_snd_deinit(aui_hdl hdl_deca, aui_hdl hdl_snd);

/**
 *   @brief      stop dis device
 *   @param[in]  *p_hdl              dis devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_dis_deinit(aui_hdl *p_hdl);

/**
 *   @brief      stop and close decv device
 *   @param[in]  hdl                 decv devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_decv_deinit(aui_hdl hdl);

/**
 *   @brief      close ali chipset related device
 *   @param[in]  hdl                 snd devce handler.
 *   @return     AUI_RTN_SUCCESS     or error code
 */
int tpm_deinit(struct ali_aui_hdls *handles);

void get_audio_description_pid(unsigned short *pid);

void test_stream_with_nim_help(char *item);

int set_dmx_for_create_av_stream(int dmx_id,
                                 unsigned short video_pid,
                                 unsigned short audio_pid,
                                 unsigned short audio_desc_pid,
                                 unsigned short pcr_pid,
                                 aui_hdl *dmx_hdl);
void test_live_reg();

#endif

