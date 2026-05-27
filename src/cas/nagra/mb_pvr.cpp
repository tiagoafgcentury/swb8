#include "mb_pvr.h"

#include "aui_fs.h"
#include "mb_acs_reencrypt.h"
#include "hal/mb_system.h"

#include <sys/statvfs.h>
#include <aui_dsc.h>
#include <aui_dmx.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_pvr.h>
#include <aui_fs.h>
#include <aui_snd.h>

#include <atomic>
#include <unistd.h>
#include <time.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace mb {

struct PVR_Cas::Data
{

    aui_hdl     rec_hdl;
    aui_hdl     ply_hdl;
    Data()
    {
        clear();
    }

    void clear()
    {
        MB_ZERO(rec_hdl);
        MB_ZERO(ply_hdl);
    }
};

PVR_Cas::PVR_Cas():
    m_p(std::make_unique<Data>()),
    m_state(State::Idle),
    m_acs_reencrypt(std::make_unique<ACS_Reencrypt>())
{
}

PVR_Cas::~PVR_Cas()
{
    pvr_deinit();
}

int PVR_Cas::pvr_init()
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    m_p->clear();
    memset(&ini_param, 0, sizeof(ini_param));
    ini_param.max_rec_number    = max_record_number;
    ini_param.max_play_number   = max_play_number;
    ini_param.trim_record_ptm   = 0;
    ini_param.continuous_tms_en = 0;
    ini_param.debug_level       = AUI_PVR_DEBUG_ALL; //AUI_PVR_DEBUG_NONE;
    STRCPY(ini_param.dvr_path_prefix, "PVR");
    STRCPY(ini_param.info_file_name, "info.dvr");
    STRCPY(ini_param.info_file_name_new, "info3.dvr");
    STRCPY(ini_param.ts_file_format, "dvr");
    STRCPY(ini_param.ts_file_format_new, "ts");
    STRCPY(ini_param.ps_file_format, "mpg");
    STRCPY(ini_param.test_file1, "test_wr1.dvr");
    STRCPY(ini_param.test_file2, "test_wr2.dvr");
    STRCPY(ini_param.storeinfo_file_name, "storeinf.dvr");
    ini_param.record_min_len = 15;      // in seconds, recomment to 15s, the record will be
    // deleted if shorter that this limit
    ini_param.tms_time_max_len  = 7200; // in seconds, recomment to 2h(7200);
    ini_param.tms_file_min_size = 2;    // in MBytes,  recomment to 10M
    ini_param.prj_mode          = AUI_PVR_DVBS2;
    ini_param.cache_addr        = reinterpret_cast<unsigned int>(pvr_buffer.data());
    ini_param.cache_size        = pvr_buffer_len;
    ini_param.scramble_vobu_time_len = 600;
	ini_param.h264_vobu_time_len = 600;
    //ini_param.event_callback     = pvr_evnt_callback;

    DEBUG_MSG(CAS, INFO, "pvr_buffer is: " <<  ini_param.cache_addr << " len: " << ini_param.cache_size << "\n");
    ret = aui_pvr_init(&ini_param);
    if(ret != AUI_RTN_SUCCESS)
    {
        return 1;
    }

    DEBUG_MSG(CAS, INFO, "aui_pvr_init ret : 0x" << hex << ret << "\n");
    return 0;
}

void PVR_Cas::pvr_deinit()
{
    aui_pvr_deinit();
}

std::string PVR_Cas::search_pvr_mount_point()
{
    std::ifstream mounts("/proc/mounts");
    std::string line;
    m_mount_point = "";
    while (std::getline(mounts, line))
    {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;

        if (iss >> device >> mountpoint >> fstype)
        {
            // Filtra apenas montagens em /mnt
            if (mountpoint.find(USB_PATH) == 0)
            {
                m_mount_point = mountpoint;
                m_filesystem_type = fstype;
            }
        }
    }

    return m_mount_point;
}

std::string PVR_Cas::pvr_mount_point()
{
    return m_mount_point;
}

std::string PVR_Cas::pvr_filesystem_type()
{
    return m_filesystem_type;
}

void PVR_Cas::pvr_get_disk_size(std::string disk_mount)
{
    struct statvfs fs_status;
    memset(&fs_status, 0, sizeof(struct statvfs));

    if(statvfs(disk_mount.c_str(), &fs_status) != 0)
    {
        DEBUG_MSG(CAS, ERROR, "\nstatvfs error\n");
        return;
    }

    // Tamanhos em bytes
    unsigned long long total = fs_status.f_blocks * fs_status.f_frsize;
    unsigned long long free = fs_status.f_bfree * fs_status.f_frsize;
    unsigned long long available = fs_status.f_bavail * fs_status.f_frsize;
    unsigned long long used = total - free;
    DEBUG_MSG(CAS, INFO, "Montagem: " << disk_mount << "\n");
    DEBUG_MSG(CAS, INFO, "Total     : " << dec << total / (1024 * 1024) << " MB\n");
    DEBUG_MSG(CAS, INFO, "Usado     : " << dec << used / (1024 * 1024) << " MB\n");
    DEBUG_MSG(CAS, INFO, "Disponível: " << dec << available / (1024 * 1024) << " MB\n");
}

AUI_RTN_CODE PVR_Cas::pvr_attach(std::string disk_name, PVR_Record_Mode disk_mode)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_pvr_disk_attach_info p_apart_param;
    memset(&p_apart_param, 0, sizeof(aui_pvr_disk_attach_info));
    strncpy(p_apart_param.mount_name, disk_name.c_str(), AUI_PVR_MOUNT_NAME_LEN_MAX);

    switch(disk_mode)
    {
        case PVR_Record_Mode::PVR_REC_AND_TMS_DISK:
            p_apart_param.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
            p_apart_param.disk_mode = 0; //USB
            break;

        default:
        case PVR_Record_Mode::PVR_TMS_ONLY_DISK:
            p_apart_param.disk_usage = AUI_PVR_TMS_ONLY_DISK;
            p_apart_param.disk_mode = 0; //USB
            break;
    }

    p_apart_param.sync        = 1;
    p_apart_param.init_list   = 1;
    p_apart_param.check_speed = 0;
    ret = aui_pvr_disk_attach(&p_apart_param);
    if (ret == AUI_RTN_SUCCESS)
    {
        m_mount_point = disk_name;
    }

    pvr_get_disk_size(disk_name);

    DEBUG_MSG(CAS, INFO, "aui_pvr_disk_attach ret : 0x" << hex << ret << "\n");

    return ret;
}


void PVR_Cas::pvr_callback(aui_hdl handle, aui_evnt_pvr msg_type, unsigned int msg_code, void *user_data)
{
    auto thiz = static_cast<PVR_Cas*>(user_data);

    switch (msg_type)
    {
        case AUI_EVNT_PVR_MSG_PLAY_NEAR_END:
            if (thiz->function_state() == Function_State::Timeshift)
            {
                thiz->pvr_play_pause();

            }
            break;
        case AUI_EVNT_PVR_END_DATAEND:
        case AUI_EVNT_PVR_END_REVS:
        case AUI_EVNT_PVR_END_TMS:
        case AUI_EVNT_PVR_MSG_PLAY_STOP:
            printf("\n*** CHEGOU AQUI!!! msg_type: %d\n", msg_type);
            break;

        default:
        {
            ACS_Reencrypt::acs_reen_param acs_reen;
            MB_ZERO(acs_reen);

            acs_reen.block_size   = TS_DEFAULT_BLOCK_SIZE;
            acs_reen.acs_enc_algo = AUI_DSC_ALGO_AES; // default AES algo encrypt
            acs_reen.dynamic_key = 0; // ACS reencrypt fixed key flow (pvr no-block mode)
            thiz->m_acs_reencrypt->acs_reencrypt_fixed_key_callback(handle, msg_type, msg_code, &acs_reen);
            break;
        }
    }
}

bool PVR_Cas::pvr_play_open(pvr_playback_param *param)
{
    aui_ply_param st_app;

    memset(&st_app, 0, sizeof(st_app));
    st_app.index        = param->record_index;
    st_app.dmx_id       = AUI_DMX_ID_SW_DEMUX0;
    st_app.live_dmx_id  = AUI_DMX_ID_DEMUX0;
    st_app.fn_callback  = pvr_callback;
    st_app.user_data    = this;
    st_app.preview_mode = 0;
    st_app.speed        = AUI_PVR_PLAY_SPEED_1X;
    DEBUG_MSG(CAS, DEBUG, "start_mode : 0x" << hex << param->start_mode << "\n");
    st_app.start_mode = param->start_mode;
    st_app.start_pos  = 0;
    st_app.start_time = 0;
    st_app.state      = AUI_PVR_PLAY_STATE_PLAY;

    if(not param->filename.empty())
    {
        STRCPY(st_app.path, param->filename.c_str());
    }

    printf("\n**************** st_app.path: %s ****************\n", st_app.path);

    aui_pvr_set(NULL, AUI_PVR_BLOCK_MODE, 0, 0, 0);

    if(AUI_RTN_SUCCESS != aui_pvr_ply_open(&st_app, &m_p->ply_hdl))
    {
        DEBUG_MSG(CAS, ERROR, "aui_pvr_ply_open fail!\n");
        return false;
    }

    m_state.store(State::Started, std::memory_order_release);
    return true;
}


bool PVR_Cas::pvr_play_play()
{
    m_state.store(State::Starting, std::memory_order_release);

    if(AUI_RTN_SUCCESS != aui_pvr_ply_state_change(m_p->ply_hdl, AUI_PVR_PLAY_STATE_PLAY, 0))
    {
        return false;
    }

    ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
    fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;
    m_state.store(State::Started, std::memory_order_relaxed);
    return true;
}

bool PVR_Cas::pvr_play_pause()
{
    m_state.store(State::Pausing, std::memory_order_release);

    if(AUI_RTN_SUCCESS != aui_pvr_ply_state_change(m_p->ply_hdl, AUI_PVR_PLAY_STATE_PAUSE, 0))
    {
        return false;
    }

    m_state.store(State::Paused, std::memory_order_relaxed);
    return true;
}

bool PVR_Cas::pvr_play_forward(PVR_Speed_Forward pvr_speed)
{
    aui_player_speed _pvr_speed;

    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Started == state or State::Starting == state or State::Error == state))
    {
        switch(pvr_speed)
        {
            case PVR_Speed_Forward::PVR_SPEED_FASTFORWARD_2:
                _pvr_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
                break;

            case PVR_Speed_Forward::PVR_SPEED_FASTFORWARD_4:
                _pvr_speed = AUI_PLAYER_SPEED_FASTFORWARD_4;
                break;

            case PVR_Speed_Forward::PVR_SPEED_FASTFORWARD_8:
                _pvr_speed = AUI_PLAYER_SPEED_FASTFORWARD_8;
                break;

            case PVR_Speed_Forward::PVR_SPEED_FASTFORWARD_16:
                _pvr_speed = AUI_PLAYER_SPEED_FASTFORWARD_16;
                break;

            case PVR_Speed_Forward::PVR_SPEED_FASTFORWARD_32:
                _pvr_speed = AUI_PLAYER_SPEED_FASTFORWARD_32;
                break;

            case PVR_Speed_Forward::PVR_SPEED_NORMAL:
            default:
                _pvr_speed = AUI_PLAYER_SPEED_NORMAL;
                break;
        }

        DEBUG_MSG(CAS, DEBUG, "FORWARD SPEED: " << _pvr_speed << "\n");
        if(AUI_RTN_SUCCESS != aui_pvr_ply_state_change(m_p->ply_hdl, AUI_PLY_STATE_SPEED, _pvr_speed))
        {
            return false;
        }
    }

    return true;
}

bool PVR_Cas::pvr_play_rewind(PVR_Speed_Rewind pvr_speed)
{
    aui_player_speed _pvr_speed;

    auto state = m_state.load(std::memory_order_acquire);
    if(m_p and (State::Started == state or State::Starting == state or State::Error == state))
    {
        switch(pvr_speed)
        {
            case PVR_Speed_Rewind::PVR_SPEED_FASTREWIND_2:
                _pvr_speed = AUI_PLAYER_SPEED_FASTREWIND_2;
                break;

            case PVR_Speed_Rewind::PVR_SPEED_FASTREWIND_4:
                _pvr_speed = AUI_PLAYER_SPEED_FASTREWIND_4;
                break;

            case PVR_Speed_Rewind::PVR_SPEED_FASTREWIND_8:
                _pvr_speed = AUI_PLAYER_SPEED_FASTREWIND_8;
                break;

            case PVR_Speed_Rewind::PVR_SPEED_FASTREWIND_16:
                _pvr_speed = AUI_PLAYER_SPEED_FASTREWIND_16;
                break;

            case PVR_Speed_Rewind::PVR_SPEED_FASTREWIND_32:
                _pvr_speed = AUI_PLAYER_SPEED_FASTREWIND_32;
                break;

            case PVR_Speed_Rewind::PVR_SPEED_NORMAL:
            default:
                _pvr_speed = AUI_PLAYER_SPEED_NORMAL;
                break;
        }

        DEBUG_MSG(CAS, DEBUG, "REWIND SPEED: " << _pvr_speed << "\n");
        if(AUI_RTN_SUCCESS != aui_pvr_ply_state_change(m_p->ply_hdl, AUI_PLY_STATE_SPEED, _pvr_speed))
        {
            return false;
        }
    }

    return true;
}

bool PVR_Cas::pvr_play_close()
{
    m_state.store(State::Closing, std::memory_order_release);
    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync      = TRUE;
    st_apsp.vpo_mode  = 0;

    if(AUI_RTN_SUCCESS != aui_pvr_ply_close(m_p->ply_hdl, &st_apsp))
    {
        return false;
    }

    m_state.store(State::Idle, std::memory_order_relaxed);
    return true;
}

unsigned long PVR_Cas::pvr_record_open(pvr_record_param *param)
{

    aui_record_prog_param st_arp;
    AUI_RTN_CODE ret          = AUI_RTN_SUCCESS;
    std::string labelname;
    memset(&st_arp, 0, sizeof(st_arp));
    st_arp.dmx_id               = param->dmx_id;
    st_arp.is_tms_record        = param->rec_mode;
    st_arp.pid_info.audio_count = param->audio_pid_count;
    st_arp.h264_flag            = param->video_type;
    st_arp.fn_callback          = pvr_callback;
    st_arp.user_data            = this;
    st_arp.pid_info.video_pid   = param->video_pid;
    st_arp.pid_info.pcr_pid     = param->pcr_pid;
    st_arp.rec_type             = AUI_PVR_REC_TYPE_TS;
    st_arp.ca_mode              = (param->is_ca_mode == 1 ? 1 : 0);
    size_t pos                  = param->filename.find(':');

    if(pos != std::string::npos)
    {
        // Se encontrou ':', extrai a parte após o separador
        labelname = param->filename.substr(pos + 1);
    }

    if(param->video_pid == AUI_INVALID_PID)
    {
        st_arp.av_flag = 0;
    }
    else
    {
        st_arp.av_flag = 1;
    }

    for(int i = 0; i < param->audio_pid_count; i++)
    {
        st_arp.pid_info.audio_pid[i]  = param->audio_pid[i];
        st_arp.pid_info.audio_type[i] = static_cast<aui_deca_stream_type>(param->audio_type[i]);
        st_arp.pid_info.audio_desc_pid[i] = 0x1FFF;

        if(param->audio_desc_pid)
        {
            st_arp.pid_info.audio_desc_pid[i] = param->audio_desc_pid[i];
        }
    }

    //ENCRYPT
    st_arp.is_reencrypt           = 1;
    st_arp.rec_special_mode       = AUI_PVR_ACS_MULTI_RE_ENCRYPTION;
    st_arp.is_scrambled           = 0;

    //FTA
    //st_arp.is_reencrypt           = 0;
    //st_arp.rec_special_mode       = AUI_PVR_NONE;
    //st_arp.is_scrambled           = 0;
    aui_pvr_set(NULL, AUI_PVR_BLOCK_MODE, 0, 0, 0);

    if(not labelname.empty())
    {
        st_arp.label_init_param = new aui_pvr_label_init_param;
        strncpy(st_arp.label_init_param->label, labelname.c_str(), AUI_PVR_LABEL_NAME_LEN_MAX - 1);
    }

    if(param->filename.empty())
    {
        sprintf(st_arp.folder_name, "/Century_%d", rand()%1000);
    }
    else
    {
        sprintf(st_arp.folder_name, "/%s", param->filename.c_str());
    }

    m_record_filename = st_arp.folder_name;
    DEBUG_MSG(CAS, DEBUG, "pvr recorder filename: " << st_arp.folder_name << "\n");
    ret = aui_pvr_rec_open(&st_arp, &m_p->rec_hdl);
    DEBUG_MSG(CAS, DEBUG, "pvr recorder ret: " << ret << "\n");

    if(st_arp.label_init_param)
    {
        delete st_arp.label_init_param;
        st_arp.label_init_param = NULL;
    }

    if(ret == AUI_RTN_SUCCESS)
    {

        aui_pvr_rec_state_change(m_p->rec_hdl, AUI_PVR_REC_STATE_RECORDING);

/*
        char rec_name[255];
        sprintf(rec_name, "%s", m_record_filename.c_str());
        ret = aui_pvr_set(  m_p->mp_hdl,
                            AUI_PVR_NV_CREDENTIAL_DATA,
                            static_cast<unsigned int>(rec_name[0]),
                            static_cast<unsigned int>(m_credential.data[0]),
                            static_cast<unsigned int>(m_credential.size)
        );

        if(ret != AUI_RTN_SUCCESS)
        {
            DEBUG_MSG(CAS, ERROR, "Saving the credential data fail!\n");
        }
        else
        {
            DEBUG_MSG(CAS, DEBUG, "Saving the credential data success!\n");
        }
*/
    }
    else
    {
        m_state.store(State::Error, std::memory_order_release);
        DEBUG_MSG(CAS, ERROR, "aui_pvr_rec_open fail\n");
        m_p->rec_hdl = 0;
    }

    m_state.store(State::Started, std::memory_order_release);
    return ret;
}

unsigned long PVR_Cas::pvr_record_close()
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    m_state.store(State::Closing, std::memory_order_release);
    ret              = aui_pvr_rec_close(m_p->rec_hdl, 1);

    if(ret != AUI_RTN_SUCCESS)
    {
        m_p->rec_hdl = 0;
        return 1;
    }

    m_state.store(State::Idle, std::memory_order_relaxed);
    return 0;
}

void PVR_Cas::pvr_clear_tms()
{
    DEBUG_MSG(CAS, DEBUG, "************ pvr_clear_tms ************\n");
    aui_pvr_clear_tms();
}

void PVR_Cas::pvr_record_pause()
{
    unsigned int duration = 0;
    m_state.store(State::Pausing, std::memory_order_release);
    aui_pvr_rec_state_change(m_p->rec_hdl, AUI_PVR_REC_STATE_PAUSE);
    aui_pvr_get(m_p->rec_hdl, AUI_PVR_REC_TIME_S, &duration, 0, 0);
    m_state.store(State::Paused, std::memory_order_relaxed);
    DEBUG_MSG(CAS, DEBUG, "************pvr record pause at [" << duration << "\n");
}

void PVR_Cas::pvr_record_resume()
{
    unsigned int duration = 0;
    aui_pvr_rec_state_change(m_p->rec_hdl, AUI_PVR_REC_STATE_RECORDING);
    aui_pvr_get(m_p->rec_hdl, AUI_PVR_REC_TIME_S, &duration, 0, 0);
    m_state.store(State::Started, std::memory_order_release);
    DEBUG_MSG(CAS, DEBUG, "************pvr record resume at [" << duration << "\n");
}

unsigned long PVR_Cas::pvr_record_desc(pvr_record_param *record_param)
{
    DEBUG_MSG(CAS, DEBUG, "filename: " << record_param->filename << "\n");
    DEBUG_MSG(CAS, DEBUG, "vpid: " << dec << record_param->video_pid << " vtype: " << record_param->video_type << "\n");
    DEBUG_MSG(CAS, DEBUG, "acount :" << dec << record_param->audio_pid_count << "\n");

    if((record_param->audio_pid_count <= 0) || (AUI_MAX_PVR_AUDIO_PID < record_param->audio_pid_count))
    {
        DEBUG_MSG(CAS, DEBUG, "audio count is error!\n");
        return 1;
    }
    else
    {
        for(auto i = 0; i < record_param->audio_pid_count; i++)
        {
            DEBUG_MSG(CAS, DEBUG, "apid: " << dec << record_param->audio_pid << " vtype: " << dec << record_param->audio_type << " desc: " << record_param->audio_desc_pid << "\n");
        }
    }

    DEBUG_MSG(CAS, DEBUG, "pcr_pid: " << dec << record_param->pcr_pid << "\n");
    DEBUG_MSG(CAS, DEBUG, "encrypt_type: " << dec << record_param->encrypt_type << "\n");
    DEBUG_MSG(CAS, DEBUG, "do_decrypt: " << dec << record_param->do_decrypt << " fixed_key: " << dec << record_param->key_fixed << "\n");

    DEBUG_MSG(CAS, DEBUG, "start recording....\n");

    if(0 != pvr_record_open(record_param))
    {
        DEBUG_MSG(CAS, ERROR, "ali_pvr_record_open failed\n");
        return 1;
    }

    return 0;
}

unsigned long PVR_Cas::pvr_record(pvr_record_param *record_param)
{
    DEBUG_MSG(CAS, DEBUG, "filename: " << record_param->filename << "\n");
    DEBUG_MSG(CAS, DEBUG, "vpid: " << dec << record_param->video_pid << " vtype: " << dec << record_param->video_type << "\n");
    DEBUG_MSG(CAS, DEBUG, "acount :" << dec << record_param->audio_pid_count << "\n");

    if((record_param->audio_pid_count <= 0) || (AUI_MAX_PVR_AUDIO_PID < record_param->audio_pid_count))
    {
        DEBUG_MSG(CAS, ERROR, "\n audio count is error!\n");
        return 1;
    }
    else
    {
        for(auto i = 0; i < record_param->audio_pid_count; i++)
        {
            DEBUG_MSG(CAS, DEBUG, "apid: " << dec << record_param->audio_pid[i] << " atype: " << dec << record_param->audio_type[i] << "\n");
        }
    }

    DEBUG_MSG(CAS, DEBUG, "pcr_pid: " << dec << record_param->pcr_pid << "\n");
    DEBUG_MSG(CAS, DEBUG, "encrypt_type: " << dec << record_param->encrypt_type << "\n");
    DEBUG_MSG(CAS, DEBUG, "do_decrypt: " << dec << record_param->do_decrypt << " fixed_key: " << dec << record_param->key_fixed << "\n");

    DEBUG_MSG(CAS, DEBUG, "start recording....\n");

    if(0 != pvr_record_open(record_param))
    {
        DEBUG_MSG(CAS, ERROR, "ali_pvr_record_open failed\n");
        return 1;
    }

    return 0;
}

unsigned long PVR_Cas::pvr_record_ts(pvr_record_param *record_param)
{
    return pvr_record(record_param);
}

unsigned long PVR_Cas::pvr_record_dss(pvr_record_param *record_param)
{
    return pvr_record(record_param);
}

void PVR_Cas::pvr_audio_get(aui_pvr_pid_info *aui_pid_info)
{
    auto state = m_state.load(std::memory_order_acquire);
    if(State::Started == state || State::Paused == state)
    {
        aui_pvr_get(m_p->rec_hdl, AUI_PVR_PID_INFO, (unsigned int *)&aui_pid_info, 0, 0);
    }
}

void PVR_Cas::pvr_audio_set(aui_pvr_pid_info aui_pid_info, uint8_t aud_idx)
{
    auto state = m_state.load(std::memory_order_acquire);
    if(State::Started == state || State::Paused == state)
    {
        aui_pvr_set(m_p->rec_hdl, AUI_PVR_AUDIO_PID_CHANGE, aui_pid_info.audio_pid[aud_idx], aui_pid_info.audio_desc_pid[aud_idx], 0);
    }
}

unsigned int PVR_Cas::get_pvr_record_current_time()
{
    auto state = m_state.load(std::memory_order_relaxed);
    if(State::Started == state || State::Paused == state)
    {
        unsigned int param = 0;
        aui_pvr_get(m_p->rec_hdl, AUI_PVR_REC_TIME_S, &param, 0, 0);
        //DEBUG_MSG("get current record time : " << dec << param << "\n");
        return param;
    }

    return 0;
}

unsigned int PVR_Cas::get_pvr_timeshift_rec_current_time()
{
    auto state = m_state.load(std::memory_order_relaxed);
    if(State::Started == state || State::Paused == state)
    {

        unsigned int param = 0;
        #if 0
        aui_pvr_set(m_p->rec_hdl, AUI_PVR_REC_TMS_TO_REC, (unsigned int)&param, 0, 0);
        #else
        unsigned int starttime = 0;
        unsigned int duration = 0;
        aui_pvr_get(m_p->rec_hdl, AUI_PVR_REC_TIME_TMS, &starttime, 0, 0);
        aui_pvr_get(m_p->rec_hdl, AUI_PVR_REC_TIME_S, &duration, 0, 0);
        param = (duration > starttime) ? (duration - starttime) : 0;
        #endif
        DEBUG("get_pvr_timeshift_rec_current_time : " << dec << param << "\n");
        return param;
    }

    return 0;
}

unsigned int PVR_Cas::get_pvr_timeshift_play_current_time()
{
    auto state = m_state.load(std::memory_order_relaxed);
    if(State::Started == state || State::Paused == state)
    {
        unsigned int capbility = 0;
        unsigned int current_begin;
        unsigned int param = 0;
        unsigned int duration = 0;
        aui_pvr_get(m_p->ply_hdl, AUI_PVR_PLAY_TIME_S, &param, 0, 0);
        aui_pvr_get(m_p->ply_hdl, AUI_PVR_REC_TIME_S, &duration, 0, 0);
        aui_pvr_get(m_p->ply_hdl, AUI_PVR_REC_CAPBILITY, (unsigned int *)&capbility, 0, 0);
        current_begin = (duration > capbility) ? (duration - capbility) : 0;
        param      = (param == (current_begin - 1)) ? current_begin : param;
        //DEBUG_MSG(CAS, DEBUG, "get_pvr_timeshift_play_current_time : " << dec << param << "\n");
        //DEBUG_MSG(CAS, DEBUG, "get_pvr_timeshift_duration_current_time : " << dec << duration << "\n");
        return param;
    }

    return 0;
}

std::string PVR_Cas::pvr_record_filename()
{
    return m_record_filename;
}

unsigned int PVR_Cas::get_pvr_player_current_time()
{
    auto state = m_state.load(std::memory_order_relaxed);
    if(State::Started == state || State::Paused == state)
    {
        unsigned int param = 0;
        aui_pvr_get(m_p->ply_hdl, AUI_PVR_PLAY_TIME_S, &param, 0, 0);
        //DEBUG_MSG("get current playback time : " << dec << param << "\n");
        return param;
    }

    return 0;
}

unsigned int PVR_Cas::get_pvr_player_total_time()
{
    auto state = m_state.load(std::memory_order_relaxed);
    if(State::Started == state || State::Paused == state)
    {
        unsigned int param = 0;
        aui_pvr_get(m_p->ply_hdl, AUI_PVR_ITEM_DURATION, &param, 0, 0);
        //DEBUG_MSG("get play item duration : " << param << "\n");
        return param;
    }

    return 0;
}

} // namespace mb
