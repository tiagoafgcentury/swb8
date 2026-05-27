
#include "mb_acs_reencrypt.h"

#include "hal/mb_system.h"

#include <aui_ca_pvr.h>
#include <aui_dsc.h>
#include <aui_dmx.h>
#include <aui_pvr.h>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern "C" {
#include "cas/nagra/cak/ca_cak.h"
#include "cas/nagra/cak/ca_sec.h"
#include "cas/nagra/cak/ca_sec_util.h"
}

namespace mb {

extern "C"
{
    AUI_RTN_CODE aui_dsc_cak_fd_to_hdl_get(int fd, aui_hdl *new_handle);
}


struct ACS_Reencrypt::Data
{

    aui_hdl     mp_hdl;
    Data()
    {
        clear();
    }

    void clear()
    {
        MB_ZERO(mp_hdl);
    }
};

unsigned char ACS_Reencrypt::get_fixed_key[16];

ACS_Reencrypt::ACS_Reencrypt():
    m_p(std::make_unique<Data>())
{
    memset(g_acs_rec_fixed_key_hdl, 0, sizeof(g_acs_rec_fixed_key_hdl));
    memset(g_acs_ply_fixed_key_hdl, 0, sizeof(g_acs_ply_fixed_key_hdl));

    static bool key_is_set = false;
    if (not key_is_set)
    {
        key_is_set = true;
        auto fingerprint = System::get_board_fingerprint();
        DEBUG_MSG(HAL, DEBUG, "Senha: " << fingerprint << "\n");

        auto str = fingerprint.data();
        for (int i = 0; i < 16; i++)
        {
            char hex[3];
            hex[0] = *str++;
            hex[1] = *str++;
            hex[2] = 0;
            get_fixed_key[i] = strtol(hex, nullptr, 16);
        }
    }
}

ACS_Reencrypt::~ACS_Reencrypt()
{
}

void ACS_Reencrypt::acs_reencrypt_fixed_key_callback(aui_hdl _handle, uint32_t _msg_type, uint32_t _msg_code, acs_reen_param *_acs_reen)
{

    switch (_msg_type)
    {
        case AUI_EVNT_PVR_MSG_REC_START_OP_STARTDMX:
            acs_rec_fixed_key_start(_handle, (aui_pvr_crypto_general_param *)_msg_code, _acs_reen);
            break;

        case AUI_EVNT_PVR_MSG_REC_STOP_OP_STOPDMX:
            acs_rec_fixed_key_stop(_handle);
            break;

        case AUI_EVNT_PVR_MSG_PLAY_START_OP_STARTDMX:
            acs_ply_fixed_key_start(_handle, (aui_pvr_crypto_general_param *)_msg_code, _acs_reen);
            break;
        case AUI_EVNT_PVR_MSG_PLAY_STOP_OP_STOPDMX:
            acs_ply_fixed_key_stop(_handle);
            break;

        case AUI_EVNT_PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        case AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS:
            acs_crypto_pids_set((aui_pvr_crypto_pids_param *)_msg_code);
            break;

        case AUI_EVNT_PVR_MSG_REC_DATA_UPDATE:
            // fixed key case not need to handle this event
            break;

        case AUI_EVNT_PVR_MSG_BLOCK_MODE_DECRYPT:
            // fixed key case not need to handle this event
            break;
        case AUI_EVNT_PVR_MSG_PLAY_STOP:
        default:;
    }
}

uint8_t ACS_Reencrypt::acs_dsc_idx(uint8_t _dev_idx, uint8_t _offset)
{
    return (ACS_DSC_INDEX_BASE + (ACS_DSC_NUM_FOR_ONE_REC_PLY * _dev_idx) + _offset);
}

uint16_t ACS_Reencrypt::crypto_pids_check(uint16_t *_pid_list, uint16_t _pid_num)
{
    uint16_t valid_pid_num = 0;
    uint16_t i, j;

    for (i = 0; i < _pid_num; i++)
    {
        if ((0 == _pid_list[i]) || ((_pid_list[i] & AUI_INVALID_PID) == AUI_INVALID_PID))
        {
            continue;
        }

        for (j = 0; j < valid_pid_num; j++)
        {
            if (_pid_list[i] == _pid_list[j])
            {
                _pid_list[i] = AUI_INVALID_PID;
                break;
            }
        }

        if (j >= valid_pid_num)
        {
            _pid_list[valid_pid_num++] = _pid_list[i];
        }
    }

    return valid_pid_num;
}

int ACS_Reencrypt::acs_crypto_pids_set(aui_pvr_crypto_pids_param *_p_pids_param)
{

    aui_pvr_pid_info *pid_info = NULL;
    uint16_t *pid_list;
    uint16_t pid_list_size;
    uint16_t pid_num = 0;
    uint16_t i;

    if (nullptr == _p_pids_param)
    {
        DEBUG_MSG(HAL, ERROR, "p_pids_param NULL.\n");
        return -1;
    }

    pid_info = _p_pids_param->pid_info;
    if (nullptr == pid_info)
    {
        DEBUG_MSG(HAL, ERROR, "pid_info NULL.\n");
        return -1;
    }

    pid_list = _p_pids_param->pid_list;
    if (nullptr == pid_list)
    {
        DEBUG_MSG(HAL, ERROR, "pid_list NULL.\n");
        return -1;
    }

    pid_list_size = _p_pids_param->pid_num;

    if ((pid_info->video_pid != AUI_INVALID_PID) && (pid_num < pid_list_size))
    {
        pid_list[pid_num++] = pid_info->video_pid;
    }

    for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++)
    {
        pid_list[pid_num++] = pid_info->audio_pid[i];
    }

    for (i = 0; (i < pid_info->ttx_pid_count) && (pid_num < pid_list_size); i++)
    {
        pid_list[pid_num++] = pid_info->ttx_pids[i];
    }

    for (i = 0; (i < pid_info->ttx_subt_pid_count) && (pid_num < pid_list_size); i++)
    {
        pid_list[pid_num++] = pid_info->ttx_subt_pids[i];
    }

    for (i = 0; (i < pid_info->subt_pid_count) && (pid_num < pid_list_size); i++)
    {
        pid_list[pid_num++] = pid_info->subt_pids[i];
    }

    for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++)
    {
        if (pid_info->audio_desc_pid[i] != AUI_INVALID_PID)
        {
            pid_list[pid_num++] = pid_info->audio_desc_pid[i];
        }
    }

    pid_info->pat_pid = 0x1fff;
    pid_info->cat_pid = 0x1fff;
    pid_info->sdt_pid = 0x1fff;
    pid_info->nit_pid = 0x1fff;
    pid_info->eit_pid = 0x1fff;
	pid_info->pmt_pid = 0x1fff;

    // Delete the repeated pids, make the pid list clear and easy to read.
    _p_pids_param->pid_num = crypto_pids_check(pid_list, pid_num);
    if (_p_pids_param->pid_num <= 0)
    {
        DEBUG_MSG(HAL, ERROR, "pid_num: " << _p_pids_param->pid_num << "\n");
        return -1;
    }

    return 0;
}

/**
 *    Function called by pvr_acs_rec_fixed_key_start() in ACS-Reencrypt
 *    fixed key flow, need to config dsc which used to encrypt recording data
 *    and set a fixed key to encrypt the recording data.
 */
int ACS_Reencrypt::acs_rec_fixed_key_encrypt_dsc_init(struct rec_reencrypt_dev *_rec_reencrypt_hdl,
                                                  aui_pvr_crypto_general_param */*_p_param*/,
                                                  acs_reen_param *_acs_reen)
{
    int i = 0;
    aui_attr_dsc attr;

    // clang-format off
    unsigned char iv[16] = {
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    };

    // clang-format on

    memset(&attr, 0, sizeof(aui_attr_dsc));
    /****************** Config DSC which encrypt TS data **********************/
    attr.uc_dev_idx = acs_dsc_idx(_rec_reencrypt_hdl->uc_dev_idx, ACS_TS_ENCRYPT_DSC_IDX_OFFSET);
    attr.uc_algo      = _acs_reen->acs_enc_algo; // AES/DES
    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
    (void)iv;
    attr.uc_mode              = AUI_DSC_WORK_MODE_IS_ECB;
    attr.dsc_data_type        = AUI_DSC_DATA_TS;

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->pvr_rec_hdl <<"]: dsc_open dev_idx = "<< (int)attr.uc_dev_idx << ", algo = " << (int)attr.uc_algo << ", data_type = "<< (int)attr.dsc_data_type << "\n");

    if (aui_dsc_open(&attr, &_rec_reencrypt_hdl->ts_encrypt_dsc_hdl))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->ts_encrypt_dsc_hdl <<"]: aui_dsc_open error\n");
        return -1;
    }

    // Set TS MODE to the scrambler which encrypt recording data.
    aui_dsc_process_attr process_attr;
    memset(&process_attr, 0, sizeof(aui_dsc_process_attr));
    process_attr.ul_block_size = _acs_reen->block_size;
    process_attr.process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;

    if (aui_dsc_process_attr_set(_rec_reencrypt_hdl->ts_encrypt_dsc_hdl, &process_attr))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->ts_encrypt_dsc_hdl <<"]: aui_dsc_process_attr_set failed\n");
        return -1;
    }

    // Set 128 bits random key to the scrambler which encrypt recording data before RECORD
    // start.
    attr.puc_key        = get_fixed_key;
    attr.ul_key_len     = 16 * 8;
    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_EVEN;
    attr.dsc_key_type   = AUI_DSC_HOST_KEY_SRAM;
    attr.uc_algo        = _acs_reen->acs_enc_algo;
    attr.en_residue     = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
    attr.en_parity      = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used
    attr.en_en_de_crypt = AUI_DSC_ENCRYPT;
    attr.pus_pids       = _rec_reencrypt_hdl->pid_list;
    attr.ul_pid_cnt     = _rec_reencrypt_hdl->pid_num;
    (void)iv;
    attr.uc_mode       = AUI_DSC_WORK_MODE_IS_ECB;
    attr.dsc_data_type = AUI_DSC_DATA_TS;

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->pvr_rec_hdl <<"] a aui_dsc_attach_key_info2dsc() key:" << hex << setfill('0'));
    for (i = 0; i < 16; i++)
    {
        DEBUG_MSG(HAL, DEBUG, " 0x" << setw(2) << (int)attr.puc_key[i]);
    }
    DEBUG_MSG(HAL, DEBUG, "\n");

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->pvr_rec_hdl <<"] pid_num: " << _rec_reencrypt_hdl->pid_num << "\n");

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->pvr_rec_hdl <<"] PID:");
    for (i = 0; i < _rec_reencrypt_hdl->pid_num; i++)
    {
        DEBUG_MSG(HAL, DEBUG, " 0x" << attr.pus_pids[i]);
    }
    DEBUG_MSG(HAL, DEBUG, "\n");
    if (aui_dsc_attach_key_info2dsc(_rec_reencrypt_hdl->ts_encrypt_dsc_hdl, &attr))
    {
        DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _rec_reencrypt_hdl->pvr_rec_hdl <<"]: aui_dsc_attach_key_info2dsc error\n\n");
        aui_dsc_close(_rec_reencrypt_hdl->ts_encrypt_dsc_hdl);
        return -1;
    }

    return 0;
}

/**
 *   Function called by pvr_acs_rec_fixed_key_stop() in ACS-Reencrypt
 *   fixed key flow, need to release dsc resource.
 */
int ACS_Reencrypt::acs_rec_fixed_key_encrypt_dsc_deinit(rec_reencrypt_dev *_rec_reencrypt_hdl)
{
    if (_rec_reencrypt_hdl->ts_encrypt_dsc_hdl)
    {
        if (aui_dsc_close(_rec_reencrypt_hdl->ts_encrypt_dsc_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << _rec_reencrypt_hdl->pvr_rec_hdl << "] aui_dsc_close ts_record_dsc fail\n");
            return -1;
        }
        _rec_reencrypt_hdl->ts_encrypt_dsc_hdl = NULL;
    }

    return 0;
}

int ACS_Reencrypt::acs_rec_fixed_key_start(aui_hdl _handle, aui_pvr_crypto_general_param *_p_param, acs_reen_param *_acs_reen)
{
    rec_reencrypt_dev *rec_reencrypt_hdl = nullptr;
    aui_attr_dmx attr_dmx;
    aui_dmx_data_path data_path_info;
    aui_hdl valid_dsc_hdl[MAX_LIVE_DECRYPT_DSC_COUNT] = { 0 };
    unsigned int valid_live_dsc_num                   = 0;

    for (int i = 0; i < AUI_PVR_MAX_RECORD_NUM; i++)
    {
        if (g_acs_rec_fixed_key_hdl[i].is_busy == 0)
        {
            g_acs_rec_fixed_key_hdl[i].is_busy = 1;
            rec_reencrypt_hdl                  = &g_acs_rec_fixed_key_hdl[i];
            rec_reencrypt_hdl->uc_dev_idx      = i;
            break;
        }
    }

    if (nullptr == rec_reencrypt_hdl)
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY] Error: no idle rec reencrypt device left\n");
        return -1;
    }

    rec_reencrypt_hdl->pvr_rec_hdl = _handle;
    memcpy(rec_reencrypt_hdl->pid_list, _p_param->pid_list, (sizeof(uint16_t) * _p_param->pid_num));
    rec_reencrypt_hdl->pid_num = _p_param->pid_num;
    int _crypto_fd = secGetCryptoFdFromSessID(TRANSPORT_SESSION_ID_PLAY, SEC_DECRYPT);
    if (_crypto_fd > 0)
    {
        if(aui_dsc_cak_fd_to_hdl_get(_crypto_fd, valid_dsc_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY ] aui_dsc_cak_fd_to_hdl_get failed!\n");
        }
        else
        {
            valid_live_dsc_num = 1;
        }
    }
    DEBUG_MSG(HAL, INFO, "valid_live_dsc_num: "<< valid_live_dsc_num << "\n");
    DEBUG_MSG(HAL, INFO, "_crypto_fd: "<< _crypto_fd << "\n");
    DEBUG_MSG(HAL, INFO, "valid_dsc_hdl: "<< hex << valid_dsc_hdl << "\n");

    if (acs_rec_fixed_key_encrypt_dsc_init(rec_reencrypt_hdl, _p_param, _acs_reen))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< rec_reencrypt_hdl->pvr_rec_hdl <<"] pvr_acs_rec_fixed_key_encrypt_dsc_init failed\n");
        goto ERROR_EXIT;
    }

    memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
    attr_dmx.uc_dev_idx = _p_param->dmx_id;
    if (aui_find_dev_by_idx( AUI_MODULE_DMX, _p_param->dmx_id, &rec_reencrypt_hdl->ts_record_dmx_hdl))
    {
        if (aui_dmx_open(&attr_dmx, &rec_reencrypt_hdl->ts_record_dmx_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< rec_reencrypt_hdl->pvr_rec_hdl << "]: dmx open("<< _p_param->dmx_id << ") failed\n");
            goto ERROR_EXIT;
        }
        aui_dmx_start(rec_reencrypt_hdl->ts_record_dmx_hdl, &attr_dmx);
    }

    memset(&data_path_info, 0, sizeof(aui_dmx_data_path));
    if (valid_live_dsc_num)
    {
        // The recording stream is a scrambled stream
        data_path_info.data_path_type = AUI_DMX_DATA_PATH_REEN_REC;
        data_path_info.p_hdl_de_dev   = valid_dsc_hdl[0];
    }
    else
    {
        // The recording stream is a FTA stream
        data_path_info.data_path_type = AUI_DMX_DATA_PATH_EN_REC;
        data_path_info.p_hdl_de_dev   = NULL;
    }

    data_path_info.dsc_type     = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
    data_path_info.p_hdl_en_dev = rec_reencrypt_hdl->ts_encrypt_dsc_hdl;
    if (aui_dmx_data_path_set(rec_reencrypt_hdl->ts_record_dmx_hdl, &data_path_info))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< rec_reencrypt_hdl->ts_record_dmx_hdl <<"] aui_dmx_data_path_set error!\n");
        goto ERROR_EXIT;
    }

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< rec_reencrypt_hdl->pvr_rec_hdl << "]: pvr_encrypt_dsc_set return.\n");

    if (valid_live_dsc_num > 1)
    {
        for (int i = 1; i < (int)valid_live_dsc_num; i++)
        {
            memset(&data_path_info, 0, sizeof(aui_dmx_data_path));

            DEBUG_MSG(HAL, INFO,"[FIXED KEY]: ADD one more descrambler!!!!\n");
            data_path_info.data_path_type = AUI_DMX_DATA_PATH_REEN_REC_ADD_DESCRAMBLER;

            // Binding the 2nd descrambler for audio
            data_path_info.p_hdl_de_dev = valid_dsc_hdl[i];
            data_path_info.dsc_type     = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
            data_path_info.p_hdl_en_dev = rec_reencrypt_hdl->ts_encrypt_dsc_hdl;

            if (aui_dmx_data_path_set(rec_reencrypt_hdl->ts_record_dmx_hdl, &data_path_info))
            {
                DEBUG_MSG(HAL, ERROR,"[FIXED KEY]: aui_dmx_data_path_set ADD one more Descrambler error! ret: "<< rec_reencrypt_hdl->pvr_rec_hdl << "\n");
                goto ERROR_EXIT;
            }
        }
    }

    DEBUG_MSG(HAL, INFO,"[FIXED KEY]: pvr_encrypt_dsc_set return. ret: "<< rec_reencrypt_hdl->pvr_rec_hdl << "\n");

    return 0;

ERROR_EXIT:
    acs_rec_fixed_key_stop(_handle);
    return -1;
}

int ACS_Reencrypt::acs_rec_fixed_key_stop(aui_hdl _handle)
{
    rec_reencrypt_dev *rec_reencrypt_hdl = nullptr;

    for (int i = 0; i < AUI_PVR_MAX_RECORD_NUM; i++)
    {
        if (g_acs_rec_fixed_key_hdl[i].pvr_rec_hdl == _handle)
        {
            g_acs_rec_fixed_key_hdl[i].is_busy = 0;
            rec_reencrypt_hdl                  = &g_acs_rec_fixed_key_hdl[i];
            rec_reencrypt_hdl->uc_dev_idx      = 0;
            rec_reencrypt_hdl->pvr_rec_hdl     = nullptr;
        }
    }

    if (nullptr == rec_reencrypt_hdl)
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY] Error: can not find rec reencrypt device\n");
        return -1;
    }

    if (rec_reencrypt_hdl->ts_record_dmx_hdl)
    {
        rec_reencrypt_hdl->ts_record_dmx_hdl = NULL;
    }

    /************************* Deinit DSC ****************************/
    if (acs_rec_fixed_key_encrypt_dsc_deinit(rec_reencrypt_hdl))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<<rec_reencrypt_hdl->pvr_rec_hdl<<"] pvr_acs_fixed_key_dsc_deinit err\n");
        return -1;
    }

    return 0;
}

/**
 *    Function called by pvr_acs_ply_fixed_key_start() in ACS-Reencrypt
 *    fixed key flow, need to config dsc and set the fixed key which the same as
 *    recording to decrypt the recording data.
 */
int ACS_Reencrypt::acs_ply_fixed_key_decrypt_dsc_init(ply_decrypt_dev *_ply_decrypt_hdl, acs_reen_param *_acs_reen)
{
    aui_attr_dsc attr;
    unsigned int i = 0;

    // clang-format off
    unsigned char iv[16] = {
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    };
    // clang-format on

    /****************** Config DSC which decrypt TS data **********************/
    memset(&attr, 0, sizeof(aui_attr_dsc));
    attr.uc_dev_idx = acs_dsc_idx(_ply_decrypt_hdl->uc_dev_idx, ACS_TS_DECRYPT_DSC_IDX_OFFSET);
    attr.uc_algo      = _acs_reen->acs_enc_algo;
    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
    attr.dsc_data_type= AUI_DSC_DATA_TS;
    attr.uc_mode      = AUI_DSC_WORK_MODE_IS_ECB;
    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] dev_idx = "<< (int)attr.uc_dev_idx << ", algo = " << (int)attr.uc_algo << ", data_type = "<< (int)attr.dsc_data_type << ", block_size = " << _acs_reen->block_size <<"\n");

    if (aui_dsc_open(&attr, &_ply_decrypt_hdl->ts_decrypt_dsc_hdl))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] aui_dsc_open error\n");
        return -1;
    }

    aui_dsc_process_attr process_attr;
    memset(&process_attr, 0, sizeof(aui_dsc_process_attr));
    process_attr.ul_block_size = _acs_reen->block_size;
    process_attr.process_mode = AUI_DSC_PROCESS_MODE_TS_DECRYPT;

    if (aui_dsc_process_attr_set(_ply_decrypt_hdl->ts_decrypt_dsc_hdl, &process_attr))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] aui_dsc_process_attr_set failed\n");
        return -1;
    }

    attr.puc_key        = get_fixed_key;
    attr.ul_key_len     = 16 * 8;
    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_EVEN; // AUI_DSC_KEY_PATTERN_ODD_EVEN;
    attr.dsc_key_type   = AUI_DSC_HOST_KEY_SRAM;
    attr.uc_algo        = _acs_reen->acs_enc_algo;
    attr.en_residue     = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
    // TS playback should always use auto parity mode
    attr.en_parity      = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    attr.en_en_de_crypt = AUI_DSC_DECRYPT;
    attr.pus_pids       = _ply_decrypt_hdl->pid_list;
    attr.ul_pid_cnt     = _ply_decrypt_hdl->pid_num;
    (void)iv;
    attr.dsc_data_type = AUI_DSC_DATA_TS;
    attr.uc_mode       = AUI_DSC_WORK_MODE_IS_ECB;

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] data_type: " << attr.dsc_data_type << ", key_len = " << (int)attr.ul_key_len << ", key_pattern = " << (int)attr.ul_key_pattern << "\n");

    DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] aui_dsc_attach_key_info2dsc() key:" << hex << setfill('0'));
    for (i = 0; i < 16; i++)
    {
        DEBUG_MSG(HAL, DEBUG, " 0x" << setw(2) << (int)attr.puc_key[i]);
    }
    DEBUG_MSG(HAL, DEBUG, "\n");

    DEBUG_MSG(HAL, DEBUG, "[CHANGE KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] pid_num: " << _ply_decrypt_hdl->pid_num << "\n");

    DEBUG_MSG(HAL, DEBUG, "[CHANGE KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] PID:");
    for (i = 0; i < _ply_decrypt_hdl->pid_num; i++)
    {
        DEBUG_MSG(HAL, DEBUG, "0x" << attr.pus_pids[i]);
    }
    DEBUG_MSG(HAL, DEBUG, "\n");

    if (aui_dsc_attach_key_info2dsc(_ply_decrypt_hdl->ts_decrypt_dsc_hdl, &attr))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x"<< _ply_decrypt_hdl->pvr_ply_hdl <<"] aui_dsc_attach_key_info2dsc error\n");
        aui_dsc_close(_ply_decrypt_hdl->ts_decrypt_dsc_hdl);
        return -1;
    }

    return 0;
}

int ACS_Reencrypt::acs_ply_fixed_key_decrypt_dsc_deinit(ply_decrypt_dev *_ply_decrypt_hdl)
{
    if (_ply_decrypt_hdl->ts_decrypt_dsc_hdl)
    {
        if (aui_dsc_close(_ply_decrypt_hdl->ts_decrypt_dsc_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << _ply_decrypt_hdl->pvr_ply_hdl << "] aui_dsc_close ts_playback_dsc fail\n");
            return -1;
        }
        _ply_decrypt_hdl->ts_decrypt_dsc_hdl = NULL;
    }

    return 0;
}

/**
 *    Function called when receive event AUI_EVNT_PVR_MSG_PLAY_START_OP_STARTDMX in
 *    ACS-Reencrypt fixed key flow, need to do the following steps:
 *      1. config dsc.
 *      2. dmx data path set.
 */
int ACS_Reencrypt::acs_ply_fixed_key_start(aui_hdl _handle, aui_pvr_crypto_general_param *_p_param, acs_reen_param *_acs_reen)
{
    ply_decrypt_dev *ply_decrypt_hdl = NULL;
    aui_attr_dmx attr_dmx;
    int dmx_id = AUI_DMX_ID_SW_DEMUX0;
    aui_dmx_data_path data_path_info;

    for (int i = 0; i < AUI_PVR_MAX_PLAYBACK_NUM; i++)
    {
        if (g_acs_ply_fixed_key_hdl[i].is_busy == 0)
        {
            g_acs_ply_fixed_key_hdl[i].is_busy = 1;
            ply_decrypt_hdl                    = &g_acs_ply_fixed_key_hdl[i];
            ply_decrypt_hdl->uc_dev_idx        = i;
            break;
        }
    }

    if (nullptr == ply_decrypt_hdl)
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY] Error: no idle ply reencrypt device left\n");
        return -1;
    }
    ply_decrypt_hdl->pvr_ply_hdl = _handle;

    memcpy(ply_decrypt_hdl->pid_list, _p_param->pid_list, sizeof(unsigned short) *_p_param->pid_num);
    ply_decrypt_hdl->pid_num = _p_param->pid_num;

    /************************* Config DSC ****************************/
    if (acs_ply_fixed_key_decrypt_dsc_init(ply_decrypt_hdl, _acs_reen) != 0)
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] pvr_decrypt_dsc_set err\n");
        goto ERROR_EXIT;
    }

    /************************* Set DMX data path ****************************/
    memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
    attr_dmx.uc_dev_idx = dmx_id;
    if (aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_id, &ply_decrypt_hdl->ts_playback_dmx_hdl))
    {
        if (aui_dmx_open(&attr_dmx, &ply_decrypt_hdl->ts_playback_dmx_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] dmx("<< dmx_id << ") open failed\n");
            goto ERROR_EXIT;
        }
        DEBUG_MSG(HAL, DEBUG, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] aui_dmx_open("<< dec << dmx_id << ") success\n");
    }

    /*
     After getting the DMX device handle, we started connecting DMX and DSC. That means tell
     dmx which dsc device will be used for decryption playback.
     */
    memset(&data_path_info, 0, sizeof(aui_dmx_data_path));
    data_path_info.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY; // In this case, TS stream decryption.
    data_path_info.dsc_type     = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
    data_path_info.p_hdl_de_dev = ply_decrypt_hdl->ts_decrypt_dsc_hdl;
    data_path_info.p_hdl_en_dev = nullptr;
    if (aui_dmx_data_path_set(ply_decrypt_hdl->ts_playback_dmx_hdl, &data_path_info))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] aui_dmx_data_path_set error!\n");
        goto ERROR_EXIT;
    }

    return 0;

ERROR_EXIT:
    acs_ply_fixed_key_stop(_handle);
    return -1;
}

int ACS_Reencrypt::acs_ply_fixed_key_stop(aui_hdl _handle)
{
    ply_decrypt_dev *ply_decrypt_hdl = nullptr;

    for (int i = 0; i < AUI_PVR_MAX_PLAYBACK_NUM; i++)
    {
        if (g_acs_ply_fixed_key_hdl[i].pvr_ply_hdl == _handle)
        {
            g_acs_ply_fixed_key_hdl[i].is_busy = 0;
            ply_decrypt_hdl                    = &g_acs_ply_fixed_key_hdl[i];
            ply_decrypt_hdl->uc_dev_idx        = 0;
            ply_decrypt_hdl->pvr_ply_hdl       = nullptr;
        }
    }

    if (nullptr == ply_decrypt_hdl)
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY] Error: can not find ply reencrypt device\n");
        return -1;
    }

    /********************** Deinit DSC *************************/
    if (acs_ply_fixed_key_decrypt_dsc_deinit(ply_decrypt_hdl))
    {
        DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] pvr_acs_fixed_key_ply_deinit fail\n");
        return -1;
    }

    if (ply_decrypt_hdl->ts_playback_dmx_hdl)
    {
        if (aui_dmx_close(ply_decrypt_hdl->ts_playback_dmx_hdl))
        {
            DEBUG_MSG(HAL, ERROR, "[FIXED KEY 0x" << ply_decrypt_hdl->pvr_ply_hdl << "] aui_dmx_close fail\n");
            return -1;
        }
        ply_decrypt_hdl->ts_playback_dmx_hdl = NULL;
    }

    return 0;
}

} // namespace mb
