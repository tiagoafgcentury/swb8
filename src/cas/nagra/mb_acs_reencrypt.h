#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include "common/mb_globals.h"

#include "aui_common.h"
#include "aui_common_list.h"
#include "aui_pvr.h"
#include "aui_dsc.h"
#include "aui_dmx.h"
#include <aui_trng.h>

#define TS_DEFAULT_BLOCK_SIZE (47 * 1024)
#define TRANSPORT_SESSION_ID_PLAY ( 42 ) // Definido no CAS mb_nagra.h

namespace mb {

class ACS_Reencrypt
{
public:

    struct acs_reen_param
    {
        uint16_t dynamic_key;
        uint16_t block_size;
        aui_dsc_algo acs_enc_algo;
    };

    struct key_info
    {
        uint8_t key[16];
        uint8_t iv[16];
        aui_dsc_parity_mode key_pattern;
    };

    struct rec_reencrypt_dev
    {
        aui_hdl trng_hdl;
        aui_hdl pvr_rec_hdl;
        aui_hdl channel_reen_hdl; // Reencrypt channel hdl.
        aui_hdl ts_record_dmx_hdl;
        aui_hdl ts_encrypt_dsc_hdl;    // dsc handle to encrypt recording data
        aui_hdl kinfo_encrypt_dsc_hdl; // dsc handle to encrypt key info
        uint8_t uc_dev_idx;
        uint8_t is_busy;
        uint8_t iv[16];
        uint8_t origin_key[16];
        uint16_t pid_list[16];
        uint8_t rec_item_idx;
        uint16_t pid_num;
        uint16_t key_change_cnt;
        uint16_t last_key_block_count;
        uint16_t next_key_block_count;
        aui_pvr_store_info_data_single *pStoreInfo;
    };

    struct ply_decrypt_dev
    {
        aui_hdl pvr_ply_hdl;
        aui_hdl ts_playback_dmx_hdl;
        aui_hdl ts_decrypt_dsc_hdl;    // dsc handle to decrypt recording data
        aui_hdl kinfo_decrypt_dsc_hdl; // dsc handle to decrypt key info
        aui_hdl av_hdl;
        unsigned char is_busy;
        unsigned char uc_dev_idx;
        unsigned char first_key_change_flag;
        unsigned short pid_num;
        unsigned short pid_list[16];
        unsigned int next_time; // Next change key time.
        unsigned int pre_time;  // Pre change key time.
        unsigned int rec_item_idx;
        aui_pvr_store_info_data *pabStoreInfo;
        aui_pvr_store_info_header StoreInfoHead;
    };

private:
    static constexpr auto MAX_LIVE_DECRYPT_DSC_COUNT = 4;
    static constexpr auto ACS_DSC_INDEX_BASE = 10;
    static constexpr auto ACS_TS_ENCRYPT_DSC_IDX_OFFSET = 0;
    static constexpr auto ACS_KINFO_ENCRYPT_DSC_IDX_OFFSET = 1;
    static constexpr auto ACS_TS_DECRYPT_DSC_IDX_OFFSET = 2;
    static constexpr auto ACS_KINFO_DECRYPT_DSC_IDX_OFFSET = 3;
    static constexpr auto ACS_DSC_NUM_FOR_ONE_REC = 2;
    static constexpr auto ACS_DSC_NUM_FOR_ONE_PLY = 2;
    static constexpr auto ACS_DSC_NUM_FOR_ONE_REC_PLY = 4;
    static constexpr auto ACS_MAX_DSC_INDEX = 16;

    static unsigned char get_fixed_key[16];

    rec_reencrypt_dev g_acs_rec_fixed_key_hdl[AUI_PVR_MAX_RECORD_NUM] = { 0 };
    ply_decrypt_dev g_acs_ply_fixed_key_hdl[AUI_PVR_MAX_PLAYBACK_NUM] = { 0 };

    uint8_t acs_dsc_idx(uint8_t _dev_idx, uint8_t _offset);
    uint16_t crypto_pids_check(uint16_t *_pid_list, uint16_t _pid_num);
    int acs_crypto_pids_set(aui_pvr_crypto_pids_param *_p_pids_param);
    int acs_rec_fixed_key_encrypt_dsc_init(rec_reencrypt_dev *_rec_reencrypt_hdl, aui_pvr_crypto_general_param *_p_param, acs_reen_param *_acs_reen);
    int acs_rec_fixed_key_encrypt_dsc_deinit(rec_reencrypt_dev *_rec_reencrypt_hdl);
    int acs_ply_fixed_key_decrypt_dsc_init(ply_decrypt_dev *_ply_decrypt_hdl, acs_reen_param *_acs_reen);
    int acs_ply_fixed_key_decrypt_dsc_deinit(ply_decrypt_dev *_ply_decrypt_hdl);
    int acs_rec_fixed_key_start(aui_hdl _handle, aui_pvr_crypto_general_param *_p_param, acs_reen_param *_acs_reen);
    int acs_rec_fixed_key_stop(aui_hdl _handle);
    int acs_ply_fixed_key_start(aui_hdl _handle, aui_pvr_crypto_general_param *_p_param, acs_reen_param *_acs_reen);
    int acs_ply_fixed_key_stop(aui_hdl _handle);

protected:
    struct Data;
    std::unique_ptr<Data> m_p;

public:
    ACS_Reencrypt();
    virtual ~ACS_Reencrypt();
    void acs_reencrypt_fixed_key_callback(aui_hdl _handle, uint32_t _msg_type, uint32_t _msg_code, acs_reen_param *_acs_reen);

};

} // namespace mb
