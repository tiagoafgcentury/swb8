#include <mt_unf_avplay.h>
#include <mt_unf_sound.h>
#include <mt_unf_demux.h>

#undef max
#undef min

#include "mb_player_hls.h"

#include <curl/curl.h>
#include "mb_globals.h"
#include "mb_application.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

#define TS_BUFFER_SIZE      0x20000
#define TS_DATA_SIZE        (188 * 512)

void set_audio_attr(mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode);

namespace mb {

struct CURL_HND
{
    explicit CURL_HND(const char *_url, bool _verbose = false)
    {
        hnd = curl_easy_init();
        //curl_easy_setopt(hnd, CURLOPT_DNS_SERVERS, "8.8.8.8,8.8.4.4");
        mb_assert(hnd);
        curl_easy_setopt(hnd, CURLOPT_URL, _url);

        if(_verbose)
        {
            curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(hnd, CURLOPT_DEBUGDATA, this);
#ifndef NDEBUG
            curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, curl_trace);
#endif
        }

#ifdef SKIP_PEER_VERIFICATION
        /*
         * If you want to connect to a site who is not using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you.
         */
        curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
        /*
         * If the site you are connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl will refuse to connect. You can skip
         * this check, but this will make the connection less secure.
         */
        curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    }

    ~CURL_HND()
    {
        curl_easy_cleanup(hnd);
    }

    CURL *hnd;

#ifndef NDEBUG
    static int curl_trace(CURL *handle, curl_infotype type,
                          const char *data, size_t /*size*/,
                          void */*userp*/)
    {
        (void)handle; /* prevent compiler warning */

        switch(type)
        {
            case CURLINFO_TEXT:
                DEBUG_MSG("CURL Info: " << data << "\n");

            /* FALLTHROUGH */
            default: /* in case a new one is introduced to shock us */
                return 0;

            case CURLINFO_HEADER_OUT:
                DEBUG_MSG("H> " << data);
                return 0;

            case CURLINFO_DATA_OUT:
                //DEBUG_MSG("D> " << data);
                return 0;

            case CURLINFO_SSL_DATA_OUT:
                return 0;

            case CURLINFO_HEADER_IN:
                DEBUG_MSG("H< " << data);
                return 0;

            case CURLINFO_DATA_IN:
                //DEBUG_MSG("D< " << data);
                return 0;

            case CURLINFO_SSL_DATA_IN:
                return 0;
        }
    }

#endif
};

static const size_t s_stream_queue_size = 5;

Player_HLS::Player_HLS(Application *_parent):
    Player(),
    m_stream_queue(s_stream_queue_size),
    m_stream_queue_done(s_stream_queue_size),
    m_parent(_parent)
{
    auto demux_id = m_parent->get_demux().demux_id();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_multi_handle = curl_multi_init();
    mb_assert(m_multi_handle);
    curl_multi_setopt(m_multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
    MT_EXEC(MT_UNF_DMX_AttachTSPort(demux_id, MT_UNF_DMX_PORT_RAM_0));
    MT_EXEC(MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, TS_BUFFER_SIZE, &m_buff_handle));
    MT_UNF_AVPLAY_ATTR_S avplay_attr;
    MB_ZERO(avplay_attr);
    MT_EXEC(MT_UNF_AVPLAY_GetDefaultConfig(&avplay_attr, MT_UNF_AVPLAY_STREAM_TYPE_ES));
    avplay_attr.u32DemuxId = demux_id;
    MT_EXEC(MT_UNF_AVPLAY_Create(&avplay_attr, &m_avhnd));
}

Player_HLS::~Player_HLS()
{
    auto demux_id = m_parent->get_demux().demux_id();
    MT_EXEC(MT_UNF_DMX_DestroyTSBuffer(m_buff_handle));
    MT_EXEC(MT_UNF_DMX_DetachTSPort(demux_id));
    curl_global_cleanup();
}

void Player_HLS::open(const char *_url)
{
    mb_assert(m_state == State::DoneM3U || m_state == State::Init);
    m_state = State::Start;
    m_curl_m3u = std::make_unique<CURL_HND>(_url, false);
    auto hnd { m_curl_m3u->hnd };
    /* send all data to this function  */
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _curl_write_m3u_callback);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, _curl_header_m3u_callback);
    curl_easy_setopt(hnd, CURLOPT_HEADERDATA, this);
    curl_multi_add_handle(m_multi_handle, hnd);
    m_still_running++;
    process_events(std::chrono::milliseconds(0));
}

void Player_HLS::close()
{
    Player::close();
}

size_t Player_HLS::curl_header_m3u_callback(char *contents, size_t size, size_t nmemb)
{
    if(contents && size * nmemb > 1)
    {
        switch(tolower(contents[0]))
        {
            case 'h':
                if(m_http_status_code == 0)
                {
                    boost::cmatch what;
                    boost::regex re("^HTTP/[1-3].[1-3]*\\s+([1-5][0-9][0-9])\\s+.*");

                    if(boost::regex_match(contents, what, re))
                    {
                        DEBUG_MSG("HTTP Code: '" << what[1] << "'\n");
                        m_http_status_code = atoi(what[1].begin());
                    }
                }

                break;

            case 'l':
            {
                boost::cmatch what;
                boost::regex re("^Location:\\s(.*)");

                if(boost::regex_match(contents, what, re))
                {
                    std::string url = what[1];
                    boost::trim(url);
                    DEBUG_MSG("Location: '" << url << "'\n");
                    m_parent->post_event([this, url]()
                    {
                        open(url);
                        return false;
                    });
                }
            }
        }
    }

    return size * nmemb;
}

size_t Player_HLS::curl_write_m3u_callback(void *contents, size_t size, size_t nmemb)
{
    auto hnd { m_curl_m3u->hnd };

    if(m_http_status_code == 0)
    {
        auto ret { curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &m_http_status_code) };
        mb_assert(ret == CURLE_OK);
        mb_assert(m_http_status_code / 100 == 2);
    }

    if(m_http_status_code / 100 == 2)
    {
        char *p { static_cast<char *>(contents) };
        auto end { p + (size * nmemb) };
        auto line_start { p };

        while(p < end)
        {
            switch(*p)
            {
                case '\r':
                    *p = 0;
                    p++;
                    break;

                case '\n':
                    *p = 0;

                    if(strlen(line_start) >  1)
                    {
                        parse_m3u_line(line_start);
                    }

                    p++;
                    line_start = p;
                    break;

                default:
                    p++;
            }
        }
    }

    return size * nmemb;
}

void Player_HLS::parse_m3u_line(const char *line)
{
    if(m_state == State::Start)
    {
        if(strcasecmp(line, "#EXTM3U") != 0)
        {
            DEBUG_MSG("Header mismatch: '" << line << "'\n");
        }
        else
        {
            m_state = State::StartM3U;
        }
    }
    else if(*line == '#')
    {
        /*
        boost::cmatch what;
        boost::regex re("^#EXT(.*):(.*)");
        if(boost::regex_match(line, what, re))
        {
            if(what.size() == 3)
            {
                DEBUG_MSG("Header: EXT" << what[1] << " = " << what[2] << "\n");
            }
        }
        */
    }
    else if(m_state == State::StartM3U)
    {
        char *m3u_url { nullptr };
        curl_easy_getinfo(m_curl_m3u->hnd, CURLINFO_EFFECTIVE_URL, &m3u_url);
        m_final_m3u_url = m3u_url;
        std::string url { line };
        boost::trim(url);
        push_queue(std::move(url));
    }
    else
    {
        DEBUG_MSG("Ignore: '" << line << "'\n");
    }
}

void Player_HLS::process_events(const std::chrono::milliseconds &_timeout)
{
    if(m_still_running)
    {
        /* wait for activity, timeout or "nothing" */
        auto ret { curl_multi_wait(m_multi_handle, NULL, 0, _timeout.count(), NULL) };

        switch(ret)
        {
            case CURLM_CALL_MULTI_PERFORM:
            case CURLM_OK:
                ret = curl_multi_perform(m_multi_handle, &m_still_running);
                mb_assert(ret == CURLM_OK);

                while(true)
                {
                    int msgs_in_queue { 0 };
                    CURLMsg *msg  = curl_multi_info_read(m_multi_handle, &msgs_in_queue);

                    if(!msg)
                    {
                        break;
                    }

                    if(msg->msg == CURLMSG_DONE)
                    {
                        if(m_curl_ts && msg->easy_handle == m_curl_ts->hnd)
                        {
                            m_curl_ts.reset();
                            start_next();
                        }
                        else if(m_curl_m3u && msg->easy_handle == m_curl_m3u->hnd)
                        {
                            m_curl_m3u.reset();
                            m_state = State::DoneM3U;
                        }
                    }
                }

                break;

            default:
                DEBUG_MSG("CURL Error code: " << ret << endl);
                m_still_running = 0;
                break;
        }

        m_parent->post_event(std::bind(&Player_HLS::process_events, this, _timeout));
    }
    else
    {
        DEBUG_MSG("Finished\n");
    }
}

void Player_HLS::push_queue(std::string &&_url)
{
    for(const auto &u : m_stream_queue)
    {
        if((!u.empty()) && strcmp(u.c_str(), _url.c_str()) == 0)
        {
            //DEBUG_MSG("Ignore: " << _url << "'\n");
            return;
        }
    }

    for(const auto &u : m_stream_queue_done)
    {
        if((!u.empty()) && strcmp(u.c_str(), _url.c_str()) == 0)
        {
            //DEBUG_MSG("Ignore: " << _url << "'\n");
            return;
        }
    }

    DEBUG_MSG("Add: '" << _url << "'\n");
    m_stream_queue.push_back(std::move(_url));
    start_next();
}

void Player_HLS::start()
{
    // Audio
    {
        MT_UNF_AUDIOTRACK_ATTR_S audio_attr;
        MB_ZERO(audio_attr);
        MT_EXEC(MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &audio_attr));
        auto audio_codec { to_montage_audio_codec(Audio_Codec::AAC) };
        MT_EXEC(MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &audio_attr, &m_atrackHnd));
        MT_EXEC(MT_UNF_SND_Attach(m_atrackHnd, m_avhnd));
        set_audio_attr(m_avhnd, audio_codec, HA_AUDIO_ID_TRUEHD == audio_codec ? HD_DEC_MODE_THRU : HD_DEC_MODE_RAWPCM);
        //auto audio_pid = 1;
        //MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &audio_pid));
    }
    // Video
    {
        MT_UNF_VCODEC_ATTR_S vdec_attr;
        MB_ZERO(vdec_attr);
        MT_EXEC(MT_UNF_AVPLAY_GetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdec_attr));
        vdec_attr.enType = static_cast<MT_UNF_VCODEC_TYPE_E>(to_montage_video_codec(Video_Codec::H264));
        vdec_attr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        vdec_attr.u32ErrCover = 100;
        vdec_attr.u32Priority = 3;
        vdec_attr.u32UseDescInfoFlag = 1;
        vdec_attr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VDEC, &vdec_attr));
        //uint32_t video_pid = 0;
        //MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &video_pid));
        //MT_EXEC(MT_UNF_AVPLAY_SetAttr(m_avhnd, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &video_pid));
    }
    MT_EXEC(MT_UNF_AVPLAY_Start(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL));
    MT_EXEC(MT_UNF_AVPLAY_Start(m_avhnd, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL));
    Player::m_state = Player::State::Started;
}

void Player_HLS::start_next()
{
    if((!m_curl_ts) && (!m_stream_queue.empty()))
    {
        auto url { std::move(m_stream_queue.front()) };
        m_stream_queue.pop_front();
        m_curl_ts = std::make_unique<CURL_HND>(url.c_str());
        DEBUG_MSG("Start: '" << url << "'\t" << m_stream_queue.size() << "\n");
        m_stream_queue_done.push_back(std::move(url));
        auto hnd { m_curl_ts->hnd };
        /* send all data to this function  */
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _curl_write_ts_callback);
        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, this);
        curl_multi_add_handle(m_multi_handle, hnd);

        if(Player::state() < Player::State::Started)
        {
            setup();
            start();
        }

        m_still_running++;

        if(m_stream_queue.size() <= 1 && m_state != State::StartM3U)
        {
            open(m_final_m3u_url);
        }
    }
}

#define TS_SYNC_BYTE 0x47
#define TS_PACK_LEN 188

size_t Player_HLS::curl_write_ts_callback(void *contents, size_t size, size_t nmemb)
{
    size_t total_size { size * nmemb };
    MT_UNF_STREAM_BUF_S buffer;
    auto p { static_cast<uint8_t *>(contents) };

    while(total_size > 0)
    {
        MT_EXEC(MT_UNF_DMX_GetTSBuffer(m_buff_handle, TS_DATA_SIZE, &buffer, 10));
        auto sz { std::min<size_t>(buffer.u32Size, total_size) };
        memcpy(buffer.pu8Data, p, sz);
        p += sz;
        total_size -= sz;
        MT_EXEC(MT_UNF_DMX_PutTSBuffer(m_buff_handle, sz));
    }

    /*
    {
        uint32_t total_size = size * nmemb;
        auto p = static_cast<uint8_t*>(contents);
        auto end = p + total_size - (TS_PACK_LEN * 5);
        for(;p < end; p++)
        {
            if(p[TS_PACK_LEN * 0] == TS_SYNC_BYTE
               && p[TS_PACK_LEN * 1] == TS_SYNC_BYTE
               //&& p[TS_PACK_LEN * 2] == TS_SYNC_BYTE
               //&& p[TS_PACK_LEN * 3] == TS_SYNC_BYTE
               //&& p[TS_PACK_LEN * 4] == TS_SYNC_BYTE
            )
            {
                uint16_t pid = ((p[1] & 0b00011111) << 8) | p[2];
                DEBUG_MSG("PID: " << dec << pid << "\n");
            }
        }
    }
    */
    return size * nmemb;
}

}
