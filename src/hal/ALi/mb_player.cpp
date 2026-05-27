#include "../mb_player.h"
#include "../mb_demux.h"

#include "mb_ali_globals.h"
#include <aui_av.h>
#include <aui_vbi.h>
#include <aui_stc.h>

#include "common/mb_globals.h"

#include <atomic>
#include <unistd.h>

#include <thread>
#include <chrono>
using namespace std::chrono_literals;

namespace {

#ifndef NDEBUG
    mb::PID_t s_current_video_pid = 0;
#endif

aui_deca_stream_type to_ali_audio_codec(mb::Audio_Codec _codec)
{
    using namespace mb;

    switch (_codec)
    {
        case Audio_Codec::AAC:
            return AUI_DECA_STREAM_TYPE_AAC_LATM;

        case Audio_Codec::MP1:
            return AUI_DECA_STREAM_TYPE_MPEG1;

        case Audio_Codec::MP2:
            return AUI_DECA_STREAM_TYPE_MPEG2;

        case Audio_Codec::AC3:
            return AUI_DECA_STREAM_TYPE_AC3;

        case Audio_Codec::None:
            DEBUG_MSG(HAL, WARN, "No Audio Codec\n");
            break;

        case Audio_Codec::UNDEFINED:
            DEBUG_MSG(HAL, ERROR, "Unknown Audio Codec\n");
            break;
    }

    return AUI_DECA_STREAM_TYPE_INVALID;
}

aui_decv_format to_ali_video_codec(mb::Video_Codec _codec)
{
    using namespace mb;

    switch (_codec)
    {
        case Video_Codec::MPEG2:
            return AUI_DECV_FORMAT_MPEG;

        case Video_Codec::MPEG4:
        case Video_Codec::H264:
            return AUI_DECV_FORMAT_AVC;

        case Video_Codec::HEVC:
            return AUI_DECV_FORMAT_HEVC;

        case Video_Codec::None:
            DEBUG_MSG(HAL, WARN, "No Video Codec\n");
            break;

        case Video_Codec::UNDEFINED:
            DEBUG_MSG(HAL, ERROR, "Unknown Video Codec\n");
            break;
    }

    return AUI_DECV_FORMAT_INVALID;
}

std::string_view ali_to_str(aui_deca_stream_type _type)
{
    switch (_type)
    {
#define DECA_TYPE_STR(TYPE) \
case AUI_DECA_STREAM_TYPE_ ## TYPE: return # TYPE;
            DECA_TYPE_STR(AAC_LATM);
            DECA_TYPE_STR(MPEG1);
            DECA_TYPE_STR(MPEG2);
            DECA_TYPE_STR(AC3);

        default:
            break;
    }

    return "UNDEFINED";
}

std::string_view ali_to_str(aui_decv_format _type)
{
    switch (_type)
    {
#define DECV_TYPE_STR(TYPE) \
case AUI_DECV_FORMAT_ ## TYPE: return # TYPE;
            DECV_TYPE_STR(MPEG);
            DECV_TYPE_STR(AVC);
            DECV_TYPE_STR(HEVC);

        default:
            break;
    }

    return "UNDEFINED";
}

}

namespace mb {

std::string_view to_str(Player::State _state)
{
    switch (_state)
    {
        case Player::State::Idle:
            return "Idle";

        case Player::State::Opened:
            return "Opened";

        case Player::State::Closing:
            return "Closing";

        case Player::State::Starting:
            return "Starting";

        case Player::State::Started:
            return "Started";

        case Player::State::Stopping:
            return "Stopping";

        case Player::State::Stopped:
            return "Stopped";

        case Player::State::Error:
            return "Error";
    }

    return "<UNDEFINED>";
};

struct Player::Data
{
    aui_attrAV attr_av;
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;
    aui_attr_decv attr_decv;
    aui_hdl hnd_stc;
    uint8_t num_callbacks;

    Data()
    {
        clear();
    }

    void clear()
    {
        MB_ZERO(attr_av);
        MB_ZERO(attr_deca);
        MB_ZERO(attr_snd);
        MB_ZERO(attr_decv);
        MB_ZERO(hnd_stc);
    }

    /**
        This callback type is for the event
        <b><em> "First frame of a program showed on the screen" </em></b>.\n
        The callback function will be called once while changing the channel, and
        the input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL
    */
    static void aui_decv_callback_first_showed(void *pv_user_hld, unsigned int, unsigned int)
    {
        auto thiz = static_cast<Player *>(pv_user_hld);
        DEBUG_MSG(HAL, INFO, "Player event: first_showed\n");
        thiz->m_state.store(State::Started, std::memory_order_release);
    }

    /**
        This callback type is for the event
        <b><em> "Switching mode done after setting display mode" </em></b>.\n
        The callback function will be called once after switching mode done, and
        the input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL

        @note  At present, this callback type is intended @a only for
        - TS playback
        - Projects based on <b> Linux OS OS </b>
    */
    static void aui_decv_callback_mode_switch_ok(void */*pv_user_hld*/, unsigned int, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: mode_switch_ok\n");
    }

    /**
        This callback type is for the event
        <b><em> "Head of the first I-frame parsed" </em></b>.\n
        The callback function will be called once while changing the channel, and
        the input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL
    */
    static void aui_decv_callback_first_head_parsed(void */*pv_user_hld*/, unsigned int, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: first_head_parsed\n");
    }

    /**
        This callback type is for the event
        <b><em> "First I-frame decoded" </em></b>.\n
        The callback function will be called once while changing the channel, and
        the input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL
    */
    static void aui_decv_callback_first_i_frame_decoded(void */*pv_user_hld*/, unsigned int, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: first_i_frame_decoded\n");
    }

    /**
        The callback function that belong to this callback type will return all
        Video information of the pictures on the screen changed during playtime, e.g.
        - Height and/or Width
        - Frame Rate
        - Active Format Description (AFD)
        - Sample Aspect Ratio

        and the input parameters @b para1 and @b para2 will get different values
        depends of the underlying operating system as below:
        - @b para1 = Pointer to the struct #aui_decv_info_cb,
                    i.e. new (changed) video information
        - @b para2 = NULL
    */
    static void aui_decv_callback_info_changed(void */*pv_user_hld*/, unsigned int para1, unsigned int)
    {
        auto info = static_cast<aui_decv_info_cb *>(reinterpret_cast<void *>(para1));
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: info_changed:");

        switch (info->flag)
        {
            case 0:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tDimensions of the video pictures changed\n\t(i.e. the field #pic_width and/or #pic_height of this struct changed)");
                break;

            case 1:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tFrame rate of the video pictures changed\n\t(i.e. the field #frame_rate of this struct changed, where the unit is @a fps*1000)");
                break;

            case 2:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tActive Format Description (AFD) of the video picture changed\n\t(i.e. the field #active_format of this struct changed)");
                break;

            case 3:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tSample Aspect Ratio (SAR) of the video pictures changed\n\t(i.e. the field #sar_width and/or #sar_height of this struct changed)");
                break;
        }

        DEBUG_MSG_NL(HAL, DEBUG, dec
                  << "\n\tPic Width: " << info->pic_width
                  << "\n\tPic Height: " << info->pic_height
                  << "\n\tActive Format: " << (int) info->active_format
                  << "\n\tFrame Rate:" << info->frame_rate
                  << "\n\tSAR Width: " << info->sar_width
                  << "\n\tSAR Height: " << info->sar_height
                  << endl);
    }

    /**
        This callback type is for the event
        <b><em> "Error occurred" </em></b>.\n
        The callback function will return an error type as defined in the enum
        #aui_decv_error_type during playtime. The input parameters @b para1 and @b para2
        will get the following values:
        - @b para1 = Flag to indicate the error type occurred (the integer values
                     and related meaning of this flag are defined in the enum #aui_decv_error_type)
        - @b para2 = NULL
    */
    static void aui_decv_callback_error(void */*pv_user_hld*/, unsigned int para1, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);

        switch (static_cast<aui_decv_error_type>(para1))
        {
            case AUI_DECV_ERROR_NO_DATA:
                DEBUG_MSG(HAL, INFO, "Player event: NO DATA: video input data buffer of the video decoder is underflow (i.e. the remaining data in the buffer is less than one frame)" << endl);
                break;

            case AUI_DECV_ERROR_HARDWARE:
                DEBUG_MSG(HAL, INFO, "Player event: HARDWARE: hardware related to the video decoder has failed to decode a frame" << endl);
                break;

            case AUI_DECV_ERROR_SYNC:
                DEBUG_MSG(HAL, DEBUG, "Player event: SYNC: frame lost the synchronization with either the audio or the PCR" << endl);
                break;

            case AUI_DECV_ERROR_FRAME_DROP:
                DEBUG_MSG(HAL, INFO, "Player event: SYNC: frame dropped because it lost the synchronization with either the audio or the PCR" << endl);
                break;

            case AUI_DECV_ERROR_RESOLUTION:
                DEBUG_MSG(HAL, INFO, "Player event: SYNC: video resolution is not supported" << endl);
                break;
        }
    }

    /**
        This callback type is for the event
        <b><em> "Video state changed" </em></b>.\n
        The callback function is called when the state of video decoder changed from
        @b Decoding to <b> No data Status </b> or @a vice-versa. The input parameters
        @b para1 and @b para2 will get the following values:
        - @b para1 = Flag to indicate the current state (the integer values and related
                     meaning of this flag are defined in the enum #aui_decv_state_type)
        - @b para2 = NULL
    */
    static void aui_decv_callback_state_change(void *pv_user_hld, unsigned int para1, unsigned int)
    {
        auto thiz = static_cast<Player *>(pv_user_hld);
        auto state = static_cast<aui_decv_state_type>(para1);
        DEBUG_MSG(HAL, DEBUG, "Player event: state_change:");

        switch (state)
        {
            case AUI_DECV_STATE_NODATA:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tNO DATA: video decoder is not getting data to decode by at least <b> 500 ms" << endl);
                thiz->m_state.store(State::Error, std::memory_order_release);
                break;

            case AUI_DECV_STATE_DECODING:
                DEBUG_MSG_NL(HAL, DEBUG, "\n\tDECODING: video decoder is decoding" << endl);
                auto expected = State::Error;
                thiz->m_state.compare_exchange_strong(expected, State::Idle, std::memory_order_release);
                break;
        }
    }

    /**
        This callback type is for the event
        <b><em> "Group of Pictures" </em></b>\n
        The callback function is called when the group of pictures starts to be
        decoded.\n
        The input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL

        @note This callback type is intended for mornitoring I-Frame events
    */
    static void aui_decv_callback_monitor_gop(void */*pv_user_hld*/, unsigned int, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: monitor_gop\n");
    }

    /**
        This callback type is for the event
        <b><em> "Frame showed on the screen" </em></b>\n
        The callback function is called when a frame is showed on the screen.\n
        The input parameters @b para1 and @b para2 will get the following values:
        - @b para1 = NULL
        - @b para2 = NULL
    */
    static void aui_decv_callback_frame_displayed(void */*pv_user_hld*/, unsigned int, unsigned int)
    {
        //auto thiz = static_cast<Player*>(pv_user_hld);
        DEBUG_MSG(HAL, DEBUG, "Player event: frame_displayed\n");
    }

    /**
     T h*is callback type is for the event
     <b><em> "User data for closed captions in video stream parsed" </em></b>.\n
     The input parameters @b para1 and @b para2 of the callback function which
     belong to this callback type will take the following values:
     - @b para1 = The length of the memory block where the user data are stored
     - @b para2 = Pointer to the first memory location from which the user data
     are stored
     */
    static void aui_decv_callback_user_data(void *pv_user_hld,
                                        unsigned int length,
                                        unsigned int ptr)
    {
        if (!pv_user_hld || length <= 4 || ptr == 0)
            return;

        auto *thiz = static_cast<Player *>(pv_user_hld);
        unsigned char *data = reinterpret_cast<unsigned char *>(static_cast<uintptr_t>(ptr));
        thiz->parse_cc(data, length);
    }

    void register_deca_callbacks()
    {
        struct aui_st_deca_io_reg_callback_para cb_param;
        memset(&cb_param, 0, sizeof(cb_param));
        cb_param.en_cb_type = AUI_DECA_CB_MONITOR_NEW_FRAME;
        cb_param.p_cb       = aui_deca_monitorcb_newframe;
        cb_param.pv_param   = this;
        ALI_EXEC(aui_deca_set(attr_av.pv_hdl_deca, AUI_DECA_REG_CB, &cb_param));
        cb_param.en_cb_type = AUI_DECA_CB_MONITOR_START;
        cb_param.p_cb       = aui_deca_monitorcb_start;
        cb_param.pv_param   = this;
        ALI_EXEC(aui_deca_set(attr_av.pv_hdl_deca, AUI_DECA_REG_CB, &cb_param));
        cb_param.en_cb_type = AUI_DECA_CB_MONITOR_STOP;
        cb_param.p_cb       = aui_deca_monitorcb_stop;
        cb_param.pv_param   = this;
        ALI_EXEC(aui_deca_set(attr_av.pv_hdl_deca, AUI_DECA_REG_CB, &cb_param));
        cb_param.en_cb_type = AUI_DECA_CB_MONITOR_DECODE_ERR;
        cb_param.p_cb       = aui_deca_monitorcb_error;
        cb_param.pv_param   = this;
        ALI_EXEC(aui_deca_set(attr_av.pv_hdl_deca, AUI_DECA_REG_CB, &cb_param));
        cb_param.en_cb_type = AUI_DECA_CB_MONITOR_OTHER_ERR;
        cb_param.p_cb       = aui_deca_monitorcb_error_other;
        cb_param.pv_param   = this;
        ALI_EXEC(aui_deca_set(attr_av.pv_hdl_deca, AUI_DECA_REG_CB, &cb_param));
    }

    static void aui_deca_monitorcb_newframe([[maybe_unused]] void *p1, [[maybe_unused]] void *p2, void *_thiz)
    {
        //auto thiz = static_cast<Sound *>(_thiz);
        //DEBUG_MSG("Sound event: newframe\n");
    }

    static void aui_deca_monitorcb_start([[maybe_unused]] void *p1, [[maybe_unused]] void *p2, void *_thiz)
    {
        //auto thiz = static_cast<Sound *>(_thiz);
        DEBUG_MSG(HAL, DEBUG, "Sound event: started\n");
        //thiz->m_state.store(State::Started, std::memory_order_relaxed);
    }

    static void aui_deca_monitorcb_stop([[maybe_unused]] void *p1, [[maybe_unused]] void *p2, void *_thiz)
    {
        //auto thiz = static_cast<Sound *>(_thiz);
        DEBUG_MSG(HAL, DEBUG, "Sound event: stopped\n");
        //thiz->m_state.store(State::Stopped, std::memory_order_relaxed);
    }

    static void aui_deca_monitorcb_error([[maybe_unused]] void *p1, [[maybe_unused]] void *p2, void *_thiz)
    {
        //auto thiz = static_cast<Sound *>(_thiz);
        DEBUG_MSG(HAL, DEBUG, "Sound event: error\n");
        //thiz->m_state.store(State::Error, std::memory_order_relaxed);
    }

    static void aui_deca_monitorcb_error_other([[maybe_unused]] void *p1, [[maybe_unused]] void *p2, void *_thiz)
    {
        //auto thiz = static_cast<Sound *>(_thiz);
        DEBUG_MSG(HAL, DEBUG, "Sound event: error - other\n");
        //thiz->m_state.store(State::Error, std::memory_order_relaxed);
    }
};

Player::Player():
    m_state(State::Idle)
{
}

Player::~Player()
{
    stop();
    close();
}

void Player::change_audio(const Service::AudioPid &_audio)
{
    switch (m_state.load(std::memory_order_acquire))
    {
        case State::Started:
        case State::Error:
        {
            auto audio_codec = to_ali_audio_codec(_audio.codec);
            aui_dmx_stream_pid pid_list;
            MB_ZERO(pid_list);
            ALI_EXEC(aui_deca_set(m_p->attr_av.pv_hdl_deca, AUI_DECA_PREPARE_CHANGE_AUD_TRACK, nullptr));
            ALI_EXEC(aui_deca_stop(m_p->attr_av.pv_hdl_deca, nullptr));
            pid_list.ul_pid_cnt      = 1;
            pid_list.stream_types[0] = AUI_DMX_STREAM_AUDIO;
            pid_list.aus_pids_val[0] = _audio.pid;
            ALI_EXEC(aui_deca_type_set(m_p->attr_av.pv_hdl_deca, audio_codec));
            ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_SET_CHANGE_AUD_STREM, (void *)&pid_list));
            ALI_EXEC(aui_deca_start(m_p->attr_av.pv_hdl_deca, NULL));
            break;
        }

        default:
            break;
    }
}

void Player::open(const Service &_service)
{
    m_p = std::make_unique<Data>();
    const Service::AudioPid *audio_service { nullptr };
    const auto &audio_pids = _service.audio_pids();

    if (!audio_pids.empty())
    {
        audio_service = &_service.current_audio();
    }

    auto audio_codec = audio_service ? to_ali_audio_codec(audio_service->codec) : AUI_DECA_STREAM_TYPE_INVALID;
    auto video_codec = to_ali_video_codec(_service.video_codec());
#ifndef NDEBUG
    DEBUG_MSG(HAL, INFO, "Player:Open: " << dec <<
              "\n\tName: '" << _service.name() << "'"
              "\n\tVPid: " << _service.video_pid() << " " << ali_to_str(video_codec) << " " << (int)video_codec <<
              "\n\tPCR Pid: " << _service.pcr_pid() <<
              "\n\tType: " << (int)_service.service_type()
             );

    if (audio_service)
    {
        DEBUG_MSG_NL(HAL, INFO, "\n\tAPid: " << audio_service->pid << " " << ali_to_str(audio_codec) << " " << (int)audio_codec);
    }

    DEBUG_MSG_NL(HAL, INFO, "\n");
#endif
    aui_dmx_stream_pid pid_list;
    MB_ZERO(pid_list);
    //m_p->clear();
    aui_dmx_data_path p_dmx_data_path_info;
    MB_ZERO(p_dmx_data_path_info);

    /************ AUDIO ************/
    if (audio_service)
    {
        auto idx = pid_list.ul_pid_cnt;
        pid_list.ul_pid_cnt += 1;
        pid_list.aus_pids_val[idx] = audio_service->pid;
        pid_list.stream_types[idx] = AUI_DMX_STREAM_AUDIO;
        m_p->attr_av.st_av_info.b_audio_enable       = 1;
        m_p->attr_av.st_av_info.en_audio_stream_type = audio_codec;
        m_p->attr_av.st_av_info.en_spdif_type        = AUI_SND_OUT_MODE_DECODED;
        m_p->attr_av.st_av_info.ui_audio_pid         = audio_service->pid;
    }

    /************ VIDEO ************/
    if (_service.video_pid())
    {
        auto idx = pid_list.ul_pid_cnt;
        pid_list.ul_pid_cnt += 2;
        pid_list.aus_pids_val[idx] = _service.video_pid();
        pid_list.aus_pids_val[idx + 1] = _service.pcr_pid();
        pid_list.stream_types[idx] = AUI_DMX_STREAM_VIDEO;
        pid_list.stream_types[idx + 1] = AUI_DMX_STREAM_PCR;
        m_p->attr_av.st_av_info.b_video_enable       = 1;
        m_p->attr_av.st_av_info.b_pcr_enable         = 1;
        m_p->attr_av.st_av_info.ui_video_pid         = _service.video_pid();
        m_p->attr_av.st_av_info.ui_pcr_pid           = _service.pcr_pid();
        m_p->attr_av.st_av_info.en_video_stream_type = video_codec;
    }

    // Set Callbacks
    auto i = 0;
    m_p->num_callbacks = 0;
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_FIRST_SHOWED, Player::Data::aui_decv_callback_first_showed, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_MODE_SWITCH_OK, Player::Data::aui_decv_callback_mode_switch_ok, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_FIRST_HEAD_PARSED, Player::Data::aui_decv_callback_first_head_parsed, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_FIRST_I_FRAME_DECODED, Player::Data::aui_decv_callback_first_i_frame_decoded, this};
    //m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_USER_DATA_PARSED, Player::Data::aui_decv_callback_user_data, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_INFO_CHANGED, Player::Data::aui_decv_callback_info_changed, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_ERROR, Player::Data::aui_decv_callback_error, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_STATE_CHANGE, Player::Data::aui_decv_callback_state_change, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_MONITOR_GOP, Player::Data::aui_decv_callback_monitor_gop, this};
    m_p->attr_decv.callback_nodes[i++] = {AUI_DECV_CB_FRAME_DISPLAYED, Player::Data::aui_decv_callback_frame_displayed, this};
    m_p->num_callbacks = i;
    p_dmx_data_path_info.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    m_p->attr_av.st_av_info.b_dmx_enable = 1;

    ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_STREAM_CREATE_AV, &pid_list));
    ALI_EXEC(aui_dmx_data_path_set(g_ali_demux_hnd, &p_dmx_data_path_info));
    ALI_EXEC(aui_deca_open(&m_p->attr_deca, &m_p->attr_av.pv_hdl_deca));
    ALI_EXEC(aui_snd_open(&m_p->attr_snd, &m_p->attr_av.pv_hdl_snd));
    ALI_EXEC(aui_decv_open(&m_p->attr_decv, &m_p->attr_av.pv_hdl_decv));
    ALI_EXEC(aui_decv_decode_format_set(m_p->attr_av.pv_hdl_decv, video_codec));
    ALI_EXEC(aui_deca_type_set(m_p->attr_av.pv_hdl_deca, audio_codec));
    ALI_EXEC(aui_stc_open(&m_p->hnd_stc));

    m_p->register_deca_callbacks();
    m_state.store(State::Opened, std::memory_order_relaxed);
#ifndef NDEBUG
    s_current_video_pid = _service.video_pid();
#endif
}

void Player::start_closed_caption()
{
    mb_assert(m_p);
    if (m_p)
    {
        m_p->attr_decv.callback_nodes[m_p->num_callbacks] = {AUI_DECV_CB_USER_DATA_PARSED, Player::Data::aui_decv_callback_user_data, this};
        ALI_EXEC(aui_decv_set(m_p->attr_av.pv_hdl_decv, AUI_DECV_SET_REG_CALLBACK, (void *)&m_p->attr_decv.callback_nodes[m_p->num_callbacks]));
        ALI_EXEC(aui_decv_user_data_type_set(m_p->attr_av.pv_hdl_decv, AUI_DECV_USER_DATA_ALL));
    }
}

void Player::stop_closed_caption()
{
    if (m_p)
    {
        ALI_EXEC(aui_decv_set(m_p->attr_av.pv_hdl_decv, AUI_DECV_SET_UNREG_CALLBACK, (void *)&m_p->attr_decv.callback_nodes[m_p->num_callbacks]));
    }
}

void Player::close()
{
    m_state.store(State::Closing, std::memory_order_release);

    if (m_p)
    {
        ALI_EXEC(aui_decv_close(&m_p->attr_decv, &m_p->attr_av.pv_hdl_decv));
        ALI_EXEC(aui_snd_close(m_p->attr_av.pv_hdl_snd));
        ALI_EXEC(aui_deca_close(m_p->attr_av.pv_hdl_deca));
        ALI_EXEC(aui_stc_close(m_p->hnd_stc));
        m_p.reset();
    }

    m_state.store(State::Idle, std::memory_order_relaxed);
}

void Player::start()
{
    mb_assert(m_p);
    mb_assert(state() == State::Opened or state() == State::Stopped);

    if (m_p)
    {
        m_state.store(State::Starting, std::memory_order_release);
        ALI_EXEC(aui_deca_start(m_p->attr_av.pv_hdl_deca, &m_p->attr_deca));
        ALI_EXEC(aui_snd_start(m_p->attr_av.pv_hdl_snd, &m_p->attr_snd));
        ALI_EXEC(aui_decv_start(m_p->attr_av.pv_hdl_decv));
        ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_STREAM_ENABLE, nullptr));

        if (m_p->attr_av.st_av_info.ui_video_pid == 0) // Radio - We don't get first video frame
        {
            m_state.store(State::Started, std::memory_order_relaxed);
        }
        else
        {
            // Wait for first video frame event
        }
    }
}

void Player::stop()
{
    if (m_p)
    {
        auto state = m_state.load(std::memory_order_acquire);

        if (State::Started == state or State::Starting == state or State::Error == state)
        {
            m_state.store(State::Stopping, std::memory_order_release);
            ALI_EXEC(aui_deca_stop(m_p->attr_av.pv_hdl_deca, &m_p->attr_deca));
            ALI_EXEC(aui_snd_stop(m_p->attr_av.pv_hdl_snd, &m_p->attr_snd));
            ALI_EXEC(aui_decv_stop(m_p->attr_av.pv_hdl_decv));
            ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_STREAM_DISABLE, nullptr));
            m_state.store(State::Stopped, std::memory_order_relaxed);
        }
    }
}
#if 0
void Player::pause()
{
    if (m_p)
    {
        auto state = m_state.load(std::memory_order_acquire);
        if (state != State::Started)
        {
            return;
        }
        m_state.store(State::Paused, std::memory_order_release);
        ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_STREAM_DISABLE, nullptr));
        ALI_EXEC(aui_deca_stop(m_p->attr_av.pv_hdl_deca, nullptr));
        ALI_EXEC(aui_decv_stop(m_p->attr_av.pv_hdl_decv));
    }
}

void Player::resume()
{
    if (m_p)
    {
        auto state = m_state.load(std::memory_order_acquire);

        if (State::Paused == state)
        {
            m_state.store(State::Starting, std::memory_order_release);
            ALI_EXEC(aui_deca_start(m_p->attr_av.pv_hdl_deca, &m_p->attr_deca));
            ALI_EXEC(aui_decv_start(m_p->attr_av.pv_hdl_decv));
            ALI_EXEC(aui_dmx_set(g_ali_demux_hnd, AUI_DMX_STREAM_ENABLE, nullptr));
            m_state.store(State::Started, std::memory_order_relaxed);
        }
    }
}
#endif

uint64_t Player::get_player_stc()
{
    unsigned long _stc_27m = 0;
    if (m_p)
    {
        ALI_EXEC(aui_stc_get(m_p->hnd_stc, &_stc_27m));
    }

    // Converte de 27 MHz → 90 kHz
    uint64_t stc_90k = _stc_27m / 300;

    // Mantém em 33 bits (wrap MPEG)
    return stc_90k & ((1ULL << 33) - 1);
}

#ifndef NDEBUG
void Player::get_player_status()
{
    aui_st_dmx_get_ts_pkg_cnt_by_pid pid_list;
    MB_ZERO(pid_list);
    pid_list.ul_pid = s_current_video_pid;

    if (aui_dmx_get(g_ali_demux_hnd, AUI_DMX_RCV_TS_PKG_CNT_GET_BY_PID, &pid_list) == 0)
    {
        DEBUG_MSG(HAL, INFO, "Player Pid List: " << dec << s_current_video_pid << " pkg cnt: " << pid_list.ul_ts_pkg_cnt << "\n");
    }
}

#endif

} // namespace mb
