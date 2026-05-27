#include "mb_player.h"

#include "mb_globals.h"
#include "mb_application.h"
#include "mb_demux.h"

#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_vo.h"

#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"

#ifdef DOLBYPLUS_HACODEC_SUPPORT
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif

#include <atomic>
#include <unistd.h>


SINGULARITY_TOKEN

namespace {

mb::Player *s_player { nullptr };

const auto s_player_listen_events =
{
    MT_UNF_AVPLAY_EVENT_EOS,                   /**<The end of stream (EOS) operation is performed, NULL*//**<CNcomment: EOS执行结束, NULL.*/
    MT_UNF_AVPLAY_EVENT_STOP,                  /**<The stop operation is performed, NULL*//**<CNcomment: STOP执行结束, NULL.*/
    //MT_UNF_AVPLAY_EVENT_RNG_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_BUF_STATE_E*//**<CNcomment: 媒体缓存队列状态变化, MT_UNF_AVPLAY_BUF_STATE_E.*/
    MT_UNF_AVPLAY_EVENT_NORM_SWITCH,           /**<Standard switch, MT_UNF_NORMCHANGE_PARAM_S*//**<CNcomment: 制式切换, MT_UNF_NORMCHANGE_PARAM_S .*/
    MT_UNF_AVPLAY_EVENT_FRAMEPACKING_CHANGE,   /**<*3D Frame packing change,MT_UNF_VIDEO_FRAME_PACKING_TYPE_E*//**<CNcomment: 3D帧类型变化, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E .*/
    //MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME,         /**<New video frame, MT_UNF_VO_FRAMEINFO_S*//**<CNcomment: 新视频帧, MT_UNF_VO_FRAMEINFO_S .*/
    //MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME,         /**<New audio frame, MT_UNF_AO_FRAMEINFO_S*//**<CNcomment: 新音频帧, MT_UNF_AO_FRAMEINFO_S .*/
    MT_UNF_AVPLAY_EVENT_NEW_USER_DATA,         /**<New video user data, MT_UNF_VIDEO_USERDATA_S*//**<CNcomment: 新视频用户数据, MT_UNF_VIDEO_USERDATA_S .*/
    MT_UNF_AVPLAY_EVENT_GET_AUD_ES,            /**<New audio ES data, MT_UNF_ES_BUF_S*//**<CNcomment: 新音频ES数据, MT_UNF_ES_BUF_S .*/
    MT_UNF_AVPLAY_EVENT_IFRAME_ERR,            /**<I frame decode error*//**<CNcomment: 解码I帧错误 .*/
    MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP,         /**<Pts Jump, MT_UNF_SYNC_PTSJUMP_PARAM_S * *//**<CNcomment: PTS跳变, MT_UNF_SYNC_PTSJUMP_PARAM_S * .*/
    MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE,      /**<Synchronization status change, MT_UNF_SYNC_STAT_PARAM_S * *//**<CNcomment: 同步状态变更, MT_UNF_SYNC_STAT_PARAM_S * .*/
    //MT_UNF_AVPLAY_EVENT_VID_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_EVENT_VID_BUF_STATE*//**<CNcomment: 视频缓存队列状态变化, MT_UNF_AVPLAY_EVENT_VID_BUF_STATE */
    //MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE,         /**<Status change of the media buffer queue, MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE*//**<CNcomment: 音频缓存队列状态变化, MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE */
    MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT,         /**<The video stream is unsupport*//**<CNcomment: 视频码流不支持*/
    MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO,         /**< frame error ratio *//**<CNcomment: 图像帧出错比例*/
    MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE,       /**< audio info change, MT_UNF_ACODEC_STREAMINFO_S *//**<CNcomment: 音频信息变化，MT_UNF_ACODEC_STREAMINFO_S*/
    MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT,         /**< unsupported audio *//**<CNcomment: 不支持的音频*/
    MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR,         /**< audio frame error *//**<CNcomment: 音频帧出错*/
    MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME,      /**<first video frame, MT_UNF_VO_FRAMEINFO_S*//**<CNcomment:First 视频帧, MT_UNF_VO_FRAMEINFO_S .*/
};

template<typename T>
void register_audio_codec(const char *type, const T &codecs)
{
    for(auto c : codecs)
    {
        char buffer[30];
        snprintf(buffer, sizeof(buffer) - 1, "libHA.AUDIO.%s.%s.so", c, type);
        auto ret { MT_UNF_AVPLAY_RegisterAcodecLib(buffer) };
        DEBUG_MSG("Register codec: '" << c << "': " << (ret == MT_SUCCESS ? "SUCCESS" : "FAILED") << "\n");
    }
}

}

void set_audio_attr(mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode)
{
    using namespace mb;
    MT_UNF_ACODEC_ATTR_S audio_attr;
    MB_ZERO(audio_attr);
    MT_EXEC(MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &audio_attr));
    audio_attr.enType = static_cast<HA_CODEC_ID_E>(enADecType);

    switch(audio_attr.enType)
    {
        case HA_AUDIO_ID_PCM:
        {
            /* set pcm wav format here base on pcm file 48k.raw */
            WAV_FORMAT_S wav_format;
            wav_format.nChannels = 2;
            wav_format.nSamplesPerSec = 48000;
            wav_format.wBitsPerSample = 16;
            wav_format.cbExtWord[0] = 0;
            wav_format.cbExtWord[1] = 1;
            HA_PCM_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &wav_format);
            DEBUG_MSG("PCM Format:"
                      "\n\tChannels: 2"
                      "\n\tSamples/s: 48000"
                      "\n\tBits/sample: 16"
                      "\n"
                     );
            break;
        }

        case HA_AUDIO_ID_MP2:
        {
            HA_MP2_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            break;
        }

        case HA_AUDIO_ID_AAC:
        {
            HA_AAC_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            break;
        }

        case HA_AUDIO_ID_MP3:
        {
            HA_MP3_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            break;
        }

        case HA_AUDIO_ID_AC3PASSTHROUGH:
        {
            HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            audio_attr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
            break;
        }

        case HA_AUDIO_ID_DTSPASSTHROUGH:
        {
            HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            audio_attr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
            break;
        }

        case HA_AUDIO_ID_TRUEHD:
        {
            HA_TRUEHD_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            mb_assert(HD_DEC_MODE_THRU == enMode);

            if(HD_DEC_MODE_THRU != enMode)
            {
                DEBUG_MSG("MLP only supports hbr Pass-through.\n");
                return;
            }

            audio_attr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
            DEBUG_MSG("TrueHD decoder(HBR Pass-through only.\n");
            break;
        }

        case HA_AUDIO_ID_DOLBY_TRUEHD:
        {
            TRUEHD_DECODE_OPENCONFIG_S config;
            HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(&config);
            HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            break;
        }

        case HA_AUDIO_ID_DOLBY_CONVERT:
        {
            TRUEHD_DECODE_OPENCONFIG_S config;
            HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(&config);
            HA_DOLBY_CONVERT_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            break;
        }

        case HA_AUDIO_ID_DTSHD:
        {
            DTSHD_DECODE_OPENCONFIG_S config;
            HA_DTSHD_DecGetDefalutOpenConfig(&config);
            HA_DTSHD_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            audio_attr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
            break;
        }

        case HA_AUDIO_ID_DTSM6:
        {
            DTSM6_DECODE_OPENCONFIG_S config;
            HA_DTSM6_DecGetDefalutOpenConfig(&config);
            HA_DTSM6_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            break;
        }

#ifdef DOLBYPLUS_HACODEC_SUPPORT

        case HA_AUDIO_ID_DOLBY_PLUS:
        {
            DOLBYPLUS_DECODE_OPENCONFIG_S config;
            HA_DOLBYPLUS_DecGetDefalutOpenConfig(&config);
            config.pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
            config.pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
            /* Dolby DVB Broadcast default settings */
            config.enDrcMode = DOLBYPLUS_DRC_RF;
            config.enDmxMode = DOLBYPLUS_DMX_SRND;
            HA_DOLBYPLUS_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            //audio_attr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
            break;
        }

#endif

        case HA_AUDIO_ID_DRA:
        {
            //       HA_DRA_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam));
            HA_DRA_DecGetOpenParam_MultichPcm(&(audio_attr.stDecodeParam));
            break;
        }

        case HA_AUDIO_ID_COOK:
        case HA_AUDIO_ID_AMRNB:
        case HA_AUDIO_ID_AMRWB:
        {
            HA_FFMPEG_DECODE_OPENCONFIG_S config;
            HA_FFMPEG_DecGetDefalutOpenConfig(&config);
            HA_FFMPEGC_DecGetDefalutOpenParam(&(audio_attr.stDecodeParam), &config);
            DEBUG_MSG("cook dec set ffmpeg dec param \n");
            break;
        }

        default:
            mb_assert(false);
            break;
    }

    MT_EXEC(MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &audio_attr));
}

namespace mb {

Player::Player()
{
    SINGULARITY_CHECK(false);
    s_player = this;
    m_state = State::Idle;
    MT_EXEC(MT_UNF_AVPLAY_Init());
    MT_EXEC(MT_UNF_SND_Init());
    MT_UNF_SND_ATTR_S snd_attr;
    MB_ZERO(snd_attr);
    MT_EXEC(MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_0, &snd_attr));
    MT_EXEC(MT_UNF_SND_Open(MT_UNF_SND_0, &snd_attr));
    MT_EXEC(MT_UNF_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL));
    MT_UNF_WINDOW_ATTR_S win_attr;
    MB_ZERO(win_attr);
    win_attr.enDisp = MT_UNF_DISPLAY1;
    win_attr.bVirtual = MT_FALSE;
    win_attr.stWinAspectAttr.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
    win_attr.stWinAspectAttr.bUserDefAspectRatio = MT_FALSE;
    win_attr.stWinAspectAttr.u32UserAspectWidth  = 0;
    win_attr.stWinAspectAttr.u32UserAspectHeight = 0;
    win_attr.bUseCropRect = MT_FALSE;
    win_attr.stInputRect.s32X = 0;
    win_attr.stInputRect.s32Y = 0;
    win_attr.stInputRect.s32Width = 0;
    win_attr.stInputRect.s32Height = 0;
    MT_EXEC(MT_UNF_VO_CreateWindow(&win_attr, &m_winhnd));
    MT_UNF_AVPLAY_ATTR_S avplay_attr;
    MB_ZERO(avplay_attr);
    MT_EXEC(MT_UNF_AVPLAY_GetDefaultConfig(&avplay_attr, MT_UNF_AVPLAY_STREAM_TYPE_TS));
    auto &demux = Application::get_demux();
    avplay_attr.u32DemuxId = demux.demux_id();
    MT_EXEC(MT_UNF_AVPLAY_Create(&avplay_attr, &m_avhnd));
    std::array<const char *, 2> audio_codecs = {"AMRWB", "AMRNB"};
    std::array<const char *, 14> audio_decode = {"MP3", "MP2", "AAC", "DOLBYTRUEHD",
                                                 "DRA", "TRUEHDPASSTHROUGH", "WMA", "COOK", "DOLBYPLUS", "DTSHD", "DTSM6",
                                                 "DTSPASSTHROUGH", "AC3PASSTHROUGH", "PCM"
                                                };
    register_audio_codec("codec", audio_codecs);
    register_audio_codec("decode", audio_decode);
}

Player::~Player()
{
    MT_EXEC(MT_UNF_SND_Close(MT_UNF_SND_0));
    MT_EXEC(MT_UNF_AVPLAY_Destroy(m_avhnd));
    m_avhnd = MT_INVALID_HANDLE;
    MT_EXEC(MT_UNF_SND_DeInit());
    MT_EXEC(MT_UNF_VO_DeInit());
    MT_EXEC(MT_UNF_AVPLAY_DeInit());
    s_player = nullptr;
    SINGULARITY_CHECK(true);
}

void Player::setup()
{
    mb_assert(state() == State::Idle);
    MT_EXEC(MT_UNF_AVPLAY_ChnOpen(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL));
    MT_EXEC(MT_UNF_AVPLAY_ChnOpen(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
    MT_EXEC(MT_UNF_VO_AttachWindow(m_winhnd, m_avhnd));
    MT_EXEC(MT_UNF_VO_SetWindowEnable(m_winhnd, MT_TRUE));

    for(auto e : s_player_listen_events)
    {
        MT_UNF_AVPLAY_RegisterEvent(m_avhnd, e, reinterpret_cast<MT_UNF_AVPLAY_EVENT_CB_FN>(av_player_event_callback));
    }
}

void Player::change_audio(const Service::AudioPid &_audio)
{
    if(m_state == State::Started)
    {
        MT_EXEC(MT_UNF_AVPLAY_Stop(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
        MT_EXEC(MT_UNF_SND_Detach(m_atrackHnd, m_avhnd));
        MT_EXEC(MT_UNF_SND_DestroyTrack(m_atrackHnd));
        m_atrackHnd = 0;
    }

    MT_UNF_AUDIOTRACK_ATTR_S audio_attr;
    MB_ZERO(audio_attr);
    MT_EXEC(MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &audio_attr));
    auto audio_codec = to_montage_audio_codec(_audio.codec);

    switch(audio_codec)
    {
        case HA_AUDIO_ID_AC3PASSTHROUGH:
        case HA_AUDIO_ID_EAC3PASSTHROUGH:
        case HA_AUDIO_ID_DTSPASSTHROUGH:
            audio_attr.b_spdif_mod = MT_TRUE;
            DEBUG_MSG("SPDIF Audio Passthrough");
            break;

        default:
            break;
    }

    MT_EXEC(MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &audio_attr, &m_atrackHnd));
    MT_EXEC(MT_UNF_SND_Attach(m_atrackHnd, m_avhnd));
    set_audio_attr(m_avhnd, audio_codec, HA_AUDIO_ID_TRUEHD == audio_codec ? HD_DEC_MODE_THRU : HD_DEC_MODE_RAWPCM);
    mt_u32 audio_pid { _audio.pid };
    DEBUG_MSG("Set audio pid: " << dec << audio_pid << "\n");
    MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &audio_pid));

    if(m_state == State::Started)
    {
        MT_EXEC(MT_UNF_AVPLAY_Start(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
    }
}

void Player::open(const Service &_service)
{
#ifndef NDEBUG
    DEBUG_MSG("Player:Open: " << dec <<
              "\n\tName: " << _service.name <<
              "\n\tVPid: " << _service.video_pid <<
              "\n\tPCR Pid: " << _service.pcr_pid <<
              "\n\tType: " << (int)_service.service_type <<
              "\n\tCodec: " << static_cast<int>(_service.video_codec)
             );
    char buffer[100];
    snprintf(buffer, 101, "Player:Open: Name: %s VPid: %d PCR Pid: %d Type: %d Codec: %d",
             _service.name.c_str(), _service.video_pid, _service.pcr_pid, (int)_service.service_type, static_cast<int>(_service.video_codec));
    PRINT_DEBUG_MSG(buffer);

    if(!_service.audio_pids.empty())
    {
        const auto &audio_service = _service.audio_pids[_service.current_audio];
        DEBUG_MSG("\n\tAPid: " << audio_service.pid);
    }

    DEBUG_MSG("\n");
#endif
    setup();

    /************ AUDIO ************/
    if(!_service.audio_pids.empty())
    {
        const auto &audio_service = _service.audio_pids[_service.current_audio];
        change_audio(audio_service);
    }

    /************ VIDEO ************/
    if(_service.video_pid)
    {
        MT_UNF_VCODEC_ATTR_S vdec_attr;
        MB_ZERO(vdec_attr);
        MT_EXEC(MT_UNF_AVPLAY_GetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdec_attr));
        vdec_attr.enType = static_cast<MT_UNF_VCODEC_TYPE_E>(to_montage_video_codec(_service.video_codec));
        vdec_attr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        vdec_attr.u32ErrCover = 100;
        vdec_attr.u32Priority = 3;
        vdec_attr.u32UseDescInfoFlag = 1;
        vdec_attr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdec_attr));
        mt_u32 video_pid { _service.video_pid };
        mt_u32 pcr_pid { _service.pcr_pid };
        MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &video_pid));
        MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &pcr_pid));
    }

    m_state = State::Opened;
}

void Player::tear_down()
{
    MT_EXEC(MT_UNF_VO_SetWindowEnable(m_winhnd, MT_FALSE));
    MT_EXEC(MT_UNF_VO_DetachWindow(m_winhnd, m_avhnd));

    //MT_EXEC(MT_UNF_SND_Detach(m_atrackHnd, m_avhnd));
    if(m_atrackHnd)
    {
        MT_EXEC(MT_UNF_SND_DestroyTrack(m_atrackHnd));
    }

    MT_EXEC(MT_UNF_AVPLAY_ChnClose(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD));
    MT_EXEC(MT_UNF_AVPLAY_ChnClose(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_VID));
    m_state = State::Idle;
    m_atrackHnd = 0;
}

void Player::close()
{
    mb_assert(state() != State::Started && state() != State::Starting);

    while(m_state != State::Stopped)
    {
        std::this_thread::yield();
    }

    for(auto e : s_player_listen_events)
    {
        MT_UNF_AVPLAY_UnRegisterEvent(m_avhnd, e);
    }

    m_state = State::Closing;
    tear_down();
}

int32_t Player::av_player_event_callback(uint32_t _handle, uint32_t _event, void *_param)
{
#ifndef NDEBUG
#define SNPRINTF(str, ...) \
    do { auto sz = snprintf(nullptr, 0, str, __VA_ARGS__);           \
        mem = malloc(sz + 1);                                        \
        text = static_cast<char*>(mem);                              \
        snprintf(const_cast<char*>(text), sz + 1, str, __VA_ARGS__); \
    } while (false)
    void *mem { nullptr };
    const char *text { nullptr };

    switch(_event)
    {
        case MT_UNF_AVPLAY_EVENT_EOS:
            text = "EOS - End of stream";
            break;

        case MT_UNF_AVPLAY_EVENT_STOP:
            text = "STOP";
            break;

        case MT_UNF_AVPLAY_EVENT_RNG_BUF_STATE:
            text = "Status change of the media buffer queue";
            break;

        case MT_UNF_AVPLAY_EVENT_NORM_SWITCH:
        {
            auto info = static_cast<MT_UNF_NORMCHANGE_PARAM_S *>(_param);
            SNPRINTF("Standard switch:"
                     "\n\tNew format: %d"
                     "\n\tWidth: %u"
                     "\n\tHeight: %u"
                     "\n\tProgressive: %d"
                     "\n\tFrame Rate: %.3f\n",
                     info->enNewFormat, info->u32ImageWidth, info->u32ImageHeight,
                     info->bProgressive, static_cast<double>(info->u32FrameRate) / 1000.0
                    );
            break;
        }

        case MT_UNF_AVPLAY_EVENT_FRAMEPACKING_CHANGE:
        {
            const char *detail { nullptr };

            if(_param)
            {
                auto info { static_cast<MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *>(_param) };

                switch(*info)
                {
                    case MT_UNF_FRAME_PACKING_TYPE_NONE:
                        detail = "Normal frame, not a 3D frame";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE:
                        detail = "Side by side";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM:
                        detail = "Top and bottom";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED:
                        detail = "Time interlaced: one frame for left eye, the next frame for right eye";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_FRAME_PACKING:
                        detail = "frame packing";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_3D_TILE:
                        detail = "Tile 3D";
                        break;

                    case MT_UNF_FRAME_PACKING_TYPE_BUTT:
                        detail = "Undefined";
                        break;
                }
            }
            else
            {
                detail = "No data";
            }

            SNPRINTF("3D Frame packing change: %s", detail);
            break;
        }

        case MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME:
            text = "New video frame";
            break;

        case MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME:
            text = "New audio frame";
            break;

        case MT_UNF_AVPLAY_EVENT_NEW_USER_DATA:
            text = "New video user data";
            break;

        case MT_UNF_AVPLAY_EVENT_GET_AUD_ES:
            text = "New audio ES data";
            break;

        case MT_UNF_AVPLAY_EVENT_IFRAME_ERR:
            text = "I frame decode error";
            break;

        case MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP:
            text = "Pts Jump";
            break;

        case MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE:
            text = "Synchronization status change";
            break;

        case MT_UNF_AVPLAY_EVENT_VID_BUF_STATE:
            text = "Status change of the media buffer queue";
            break;

        case MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE:
            text = "Status change of the media buffer queue";
            break;

        case MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT:
            text = "The video stream is unsupported";
            break;

        case MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO:
            text = "Frame error ratio";
            break;

        case MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE:
        {
            auto info { static_cast<MT_UNF_ACODEC_STREAMINFO_S *>(_param) };
            SNPRINTF("Audio info change:"
                     "\n\tCodec Type: %u"
                     "\n\tSample Rate: %u"
                     "\n\tBit Depth: %d"
                     "\n\tChannel: %u\n",
                     info->enACodecType, info->enSampleRate, info->enBitDepth, info->u32Channel
                    );
            break;
        }

        case MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT:
            text = "Unsupported audio";
            break;

        case MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR:
            text = "Audio frame error";
            break;

        case MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME:
        {
            auto info { static_cast<MT_UNF_VIDEO_FRAME_INFO_S *>(_param) };
            SNPRINTF("First video frame:"
                     "\n\tFrame Index: %u"
                     "\n\tSource Width: %u"
                     "\n\tSource Height: %u"
                     "\n\tSource PTS: %u"
                     "\n\tFrameRate: %.3f"
                     "\n\tVideoFormat: %d"
                     "\n\tProgressive: %d"
                     "\n\tField Mode: %d"
                     "\n\tDisplay Width: %u"
                     "\n\tDisplay Height: %u"
                     "\n\tError Level: %u%%"
                     "\n\tPicture Coding Type: %u"
                     "\n\tActive Format: %u"
                     "\n\tVid Format: %u\n",
                     info->u32FrameIndex, info->u32Width, info->u32Height, info->u32SrcPts,
                     static_cast<double>(info->stFrameRate.u32fpsInteger) + (static_cast<double>(info->stFrameRate.u32fpsDecimal) / 1000.0),
                     info->enVideoFormat, info->bProgressive,
                     info->enFieldMode, info->u32DisplayWidth, info->u32DisplayHeight,
                     info->u32ErrorLevel, info->picture_coding_type, info->active_format, info->vid_format);
            break;
        }

        case MT_UNF_AVPLAY_EVENT_BUTT:
        default:
            text = "Unknown event";
            break;
    }

    DEBUG_MSG("Player event: " << dec << _event << " = " << text << "\n");

    if(mem)
    {
        free(mem);
    }

#endif
    auto publish_event = [](Player::Events _event)
    {
        for(const auto &call_back : s_player->m_event_callbacks)
        {
            call_back(_event);
        }
    };

    switch(_event)
    {
        case MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME:
        {
            mb_assert(s_player->m_avhnd == _handle);
            s_player->m_state = State::Started;
            publish_event(Events::Started);
            break;
        }

        case MT_UNF_AVPLAY_EVENT_STOP:
            s_player->m_state = State::Stopped;
            publish_event(Events::Stopped);
            break;
    };

    return MT_SUCCESS;
}

void Player::start()
{
    mb_assert(state() == State::Opened || state() == State::Stopped);
    m_state = State::Starting;
    MT_EXEC(MT_UNF_AVPLAY_Start(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL));
    MT_EXEC(MT_UNF_AVPLAY_Start(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
}

void Player::stop()
{
    m_state = State::Stopping;
    MT_EXEC(MT_UNF_VO_ResetWindow(m_winhnd, MT_UNF_WINDOW_FREEZE_MODE_BLACK));

    if(m_avhnd)
    {
        if(m_atrackHnd)
        {
            MT_EXEC(MT_UNF_SND_Detach(m_atrackHnd, m_avhnd));
        }

        MT_EXEC(MT_UNF_AVPLAY_Stop(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL));
        MT_EXEC(MT_UNF_AVPLAY_Stop(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
    }
}

int Player::to_montage_audio_codec(Audio_Codec _codec)
{
    switch(_codec)
    {
        case Audio_Codec::AAC:
            return HA_AUDIO_ID_AAC;

        case Audio_Codec::MP1:
        case Audio_Codec::MP2:
            return HA_AUDIO_ID_MP2;

        case Audio_Codec::AC3:
            return HA_AUDIO_ID_DOLBY_CONVERT;

        case Audio_Codec::None:
            break;

        case Audio_Codec::Unknown:
            DEBUG_MSG("WARNING: Unknown Audio Codec\n");
            break;
    }

    mb_assert(false);
    return HA_AUDIO_ID_INVALID;
}

int Player::to_montage_video_codec(Video_Codec _codec)
{
    switch(_codec)
    {
        case Video_Codec::MPEG2:
            return MT_UNF_VCODEC_TYPE_MPEG2;

        case Video_Codec::MPEG4:
            return MT_UNF_VCODEC_TYPE_MPEG4;

        case Video_Codec::H264:
            return MT_UNF_VCODEC_TYPE_H264;

        case Video_Codec::HEVC:
            return MT_UNF_VCODEC_TYPE_HEVC;

        case Video_Codec::None:
            return MT_UNF_VCODEC_TYPE_BUTT;
    }

    mb_assert(false);
    return MT_UNF_VCODEC_TYPE_BUTT;
}

#ifndef NDEBUG
void Player::get_player_status()
{
    MT_UNF_AVPLAY_STATUS_INFO_S status_info;
    MB_ZERO(status_info);
    auto ret { MT_UNF_AVPLAY_GetStatusInfo(m_avhnd, &status_info) };

    if(MT_SUCCESS == ret)
    {
        DEBUG_MSG("Player status:" << dec <<
                  "\n\tSync status:"
                  "\n\t\tPTS of first audio frame: " << status_info.stSyncStatus.u64FirstAudPts <<
                  "\n\t\tPTS of last audio frame: " << status_info.stSyncStatus.u64LastAudPts <<
                  "\n\t\tPTS of first video frame: " << status_info.stSyncStatus.u64FirstVidPts <<
                  "\n\t\tPTS of last video frame: " << status_info.stSyncStatus.u64LastVidPts <<
                  "\n\t\tAud/Video frame diff: " << status_info.stSyncStatus.s64DiffAvPlayTime <<
                  "\n\t\tPlay time: " << status_info.stSyncStatus.u64PlayTime <<
                  "\n\t\tLocal time: " << status_info.stSyncStatus.u64LocalTime <<
                  "\n\tRun status: ");

        switch(status_info.enRunStatus)
        {
            case MT_UNF_AVPLAY_STATUS_STOP:
                DEBUG_MSG("STOP");
                break;

            case MT_UNF_AVPLAY_STATUS_PREPLAY:
                DEBUG_MSG("Pre-PLAY/Buffer");
                break;

            case MT_UNF_AVPLAY_STATUS_PLAY:
                DEBUG_MSG("PLAY");
                break;

            case MT_UNF_AVPLAY_STATUS_TPLAY:
                DEBUG_MSG("Trick PLAY");
                break;

            case MT_UNF_AVPLAY_STATUS_PAUSE:
                DEBUG_MSG("PAUSE");
                break;

            case MT_UNF_AVPLAY_STATUS_EOS:
                DEBUG_MSG("EOS");
                break;

            case MT_UNF_AVPLAY_STATUS_SEEK:
                DEBUG_MSG("SEEK");
                break;

            case MT_UNF_AVPLAY_STATUS_FREEZE:
                DEBUG_MSG("FREEZE");
                break;

            default:
                DEBUG_MSG("<UNDEFINED>");
                break;
        };

        DEBUG_MSG(
            "\n\tVid Frame Count: " << status_info.u32VidFrameCount <<
            "\n\tAud Frame Count: " << status_info.u32AuddFrameCount <<
            // "\n\tBuffer Status: " << status_info.stBufStatus <<
            "\n\tVid Error Frame Count: " << status_info.u32VidErrorFrameCount <<
            "\n"
        );
    }
    else
    {
        DEBUG_MSG("Player status: Failed\n");
    }
}

#endif

} // namespace mb
