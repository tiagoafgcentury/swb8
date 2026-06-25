#pragma once

#include "mb_types.h"
#include "mb_assert.h"

#include <vector>
#include <mutex>
#include <map>

namespace mb {

struct Transponder;

enum class Service_Category
{
    Undefined = 0,
    Religioso = 2,
    Novos_Canais = 4,
    Entretenimento = 8,
    Agronegocio = 9,
    Educacao = 10,
    TV_Publica = 11,
    Regionais = 12,
    Radios = 13,
    Noticias = 15,
    Compras = 16,
};

struct Satellite
{
    uint16_t id { 0 };
    std::string name;
    Band band;
    LNBF_Type type;
    LNBF_Position position;
    DiseqC_Type switch_type;
    uint8_t switch_pos;
    float orbital { 0 };
    bool is_mandatory { false };
    Network_Policies network_policies { Network_Policies::Generic };

    bool operator!=(const Satellite &_other) const
    {
        return id != _other.id or
               name != _other.name or
               band != _other.band or
               type != _other.type or
               position != _other.position or
               switch_type != _other.switch_type or 
               switch_pos != _other.switch_pos;
    }
};

struct ScheduleEntry
{
    uint16_t id { 0 };
    Service_ID_t service_id;
    std::chrono::time_point<std::chrono::system_clock> time_to_start;
    std::chrono::time_point<std::chrono::system_clock> time_to_end;
    Schedule_Operation operation;
    Schedule_Repeat repeat;
    Schedule_Status status;
};

class Service
{
public:
    Service() = default; // Creates an empty/invalid sentinel (service_id == 0)
    Service(const Service &) = default;
    Service &operator=(const Service &) = default;

    explicit Service(Transponder_Id _transponder_id, Service_ID_t _service_id):
        m_service_id(_service_id),
        m_transponder_id(_transponder_id),
        m_satellite_id(_transponder_id.satellite_id())
    {
        mb_assert(_service_id != 0);
        mb_assert(_transponder_id.frequency() != 0);
    }

    Service(Service &&other):
        m_service_type(other.m_service_type),
        m_service_id(other.m_service_id),
        m_transponder_id(other.m_transponder_id),
        m_name(std::move(other.m_name)),
#ifdef MBGUI_SAT_MONITOR
        m_service_provider_name(std::move(other.m_service_provider_name)),
#endif
        m_pcr_pid(other.m_pcr_pid),
        m_video_pid(other.m_video_pid),
        m_video_codec(other.m_video_codec),
        m_pmt_pid(other.m_pmt_pid),
        m_epg_pid(other.m_epg_pid),
        m_ca_types(other.m_ca_types),
        m_regionalizacao(other.m_regionalizacao),
        m_viewer_channel(other.m_viewer_channel),
        m_bouquet_id(other.m_bouquet_id),
        m_is_favorite(other.m_is_favorite),
        m_category(other.m_category),
        m_zones(std::move(other.m_zones)),
        m_order_in_full(other.m_order_in_full),
        m_order_in_favorite(other.m_order_in_favorite),
#ifdef MBGUI_SAT_MONITOR
        m_playable(other.m_playable),
        m_time_to_start(other.m_time_to_start),
#endif
        m_audio_pids(std::move(other.m_audio_pids)),
        m_current_audio(other.m_current_audio),
        m_subtitle_pid(other.m_subtitle_pid),
        m_dvb_subtitle_present(other.m_dvb_subtitle_present),
        m_satellite_id(other.m_satellite_id)

    {
    }

    Service &operator=(Service &&other)
    {
        m_service_type = other.m_service_type;
        m_service_id = other.m_service_id;
        m_transponder_id = other.m_transponder_id;
        m_name = std::move(other.m_name);
        m_pcr_pid = other.m_pcr_pid;
        m_video_pid = other.m_video_pid;
        m_video_codec = other.m_video_codec;
        m_pmt_pid = other.m_pmt_pid;
        m_epg_pid = other.m_epg_pid;
        m_ca_types = other.m_ca_types;
        m_regionalizacao = other.m_regionalizacao;
        m_viewer_channel = other.m_viewer_channel;
        m_bouquet_id = other.m_bouquet_id;
        m_is_favorite = other.m_is_favorite;
        m_category = other.m_category;
        m_zones = std::move(other.m_zones);
        m_order_in_full = other.m_order_in_full;
        m_order_in_favorite = other.m_order_in_favorite;
        m_audio_pids = std::move(other.m_audio_pids);
        m_current_audio = other.m_current_audio;
        m_subtitle_pid = other.m_subtitle_pid;
        m_dvb_subtitle_present = other.m_dvb_subtitle_present;
        m_satellite_id = other.m_satellite_id;
        return *this;
    }

    struct AudioPid
    {
        static constexpr uint8_t LANG_SIZE = 4;

        AudioPid(PID_t _pid, const char *_lang, Audio_Codec _codec);

        PID_t pid { 0 };
        char lang[LANG_SIZE] { 0, 0, 0, 0 };
        Audio_Codec codec { Audio_Codec::None };
    };

private:
    static AudioPid s_empty_audio_pid;

    Service_Type m_service_type { Service_Type::none };

    Service_ID_t m_service_id { 0 };
    Transponder_Id m_transponder_id;
    std::string m_name;
    PID_t m_pcr_pid { 0 };
    PID_t m_video_pid { 0 };
    Video_Codec m_video_codec { Video_Codec::None };
    PID_t m_pmt_pid { 0 };
    PID_t m_epg_pid { 0 };

    std::vector<CA_Info> m_ca_types;
    std::vector<DVB_Subtitle_Info> m_dvb_subtitles;

    Regionalizacao m_regionalizacao { Regionalizacao::Undefined };
    Viewer_Channel_t m_viewer_channel { 0 };
    Bouquet_ID_t m_bouquet_id { 0 };
    bool m_is_favorite { true };
    Service_Category m_category { Service_Category::Undefined };

    std::set<Zone_ID_t> m_zones;

    Order_In_Full_t m_order_in_full { 0 };
    Order_In_Favorite_t m_order_in_favorite { 0 };

#ifdef MBGUI_SAT_MONITOR
    bool m_playable { false };
    std::chrono::milliseconds m_time_to_start{ 0 };
#endif

    std::vector<AudioPid> m_audio_pids;
    uint8_t m_current_audio { 0 };

    PID_t m_subtitle_pid { 0 };
    bool m_dvb_subtitle_present { false };

    uint16_t m_satellite_id { 0 };

public:
    Service_Type service_type() const
    {
        return m_service_type;
    }

    void set_service_type(Service_Type _service_type)
    {
        m_service_type = _service_type;
    }

    Service_ID_t service_id() const
    {
        return m_service_id;
    }

    void set_service_id(Service_ID_t _service_id)
    {
        m_service_id = _service_id;
    }

    Transponder_Id transponder_id() const
    {
        return m_transponder_id;
    }

    bool operator==(const Service &_other) const
    {
        return m_service_id == _other.m_service_id && m_transponder_id == _other.m_transponder_id;
    }

    bool operator!=(const Service &_other) const
    {
        return !(*this == _other);
    }

    void set_transponder_id(Transponder_Id _transponder_id)
    {
        m_transponder_id = _transponder_id;
    }

    std::string_view name() const
    {
        return m_name;
    }

    void set_name(std::string &&_name)
    {
        m_name = std::move(_name);
    }

    void set_name(std::string_view _name)
    {
        m_name = _name;
    }

    bool check_pids_are_valid() const;
    void reset_pmt_pids();

    PID_t pcr_pid() const
    {
        return m_pcr_pid;
    }

    void set_pcr_pid(PID_t _pcr_pid)
    {
        m_pcr_pid = _pcr_pid;
    }

    PID_t video_pid() const
    {
        return m_video_pid;
    }

    void set_video_pid(PID_t _video_pid)
    {
        m_video_pid = _video_pid;
    }

    Video_Codec video_codec() const
    {
        return m_video_codec;
    }

    void set_video_codec(Video_Codec _video_codec)
    {
        m_video_codec = _video_codec;
    }

    PID_t pmt_pid() const
    {
        return m_pmt_pid;
    }

    void set_pmt_pid(PID_t _pmt_pid)
    {
        m_pmt_pid = _pmt_pid;
    }

    PID_t epg_pid() const
    {
        return m_epg_pid;
    }

    void set_epg_pid(PID_t _epg_pid)
    {
        m_epg_pid = _epg_pid;
    }

    const std::vector<CA_Info> &ca_types() const
    {
        return m_ca_types;
    }

    bool has_nagra_cas() const
    {
        for(const auto &ca : ca_types())
        {
            if(ca.type == CA_Type::Nagra)
            {
                return true;
            }
        }

        return false;
    }

    void add_ca_types(CA_Info _ca_type)
    {
        m_ca_types.push_back(_ca_type);
    }

    void set_ca_types(std::vector<CA_Info> &&_ca_types)
    {
        m_ca_types = std::move(_ca_types);
    }

    const std::vector<DVB_Subtitle_Info> &dvb_subtitle() const
    {
        return m_dvb_subtitles;
    }

    void set_dvb_subtitle_present(bool _dvb_subtitle_present)
    {
        m_dvb_subtitle_present = _dvb_subtitle_present;
    }

    bool has_dvb_subtitle() const
    {
        return m_dvb_subtitle_present;
    }

    void set_dvb_subtitle(std::vector<DVB_Subtitle_Info> &&_dvb_subtitle)
    {
        m_dvb_subtitles = std::move(_dvb_subtitle);
    }

    Regionalizacao regionalizacao() const
    {
        return m_regionalizacao;
    }

    void set_regionalizacao(Regionalizacao _regionalizacao)
    {
        m_regionalizacao = _regionalizacao;
    }

    Viewer_Channel_t viewer_channel() const
    {
        return m_viewer_channel;
    }

    void set_viewer_channel(Viewer_Channel_t _viewer_channel)
    {
        m_viewer_channel = _viewer_channel;
    }

    Bouquet_ID_t bouquet_id() const
    {
        return m_bouquet_id;
    }

    void set_bouquet_id(Bouquet_ID_t _bouquet_id)
    {
        m_bouquet_id = _bouquet_id;
    }

    std::string_view bouquet_name() const;
    void set_bouquet_name(std::string_view _bouquet_name);
    static void set_bouquet_name(Bouquet_ID_t _bouquet_id, std::string_view _bouquet_name);

    bool is_favorite() const
    {
        return m_is_favorite;
    }

    void set_is_favorite(bool _is_favorite)
    {
        m_is_favorite = _is_favorite;
    }

    void toggle_is_favorite()
    {
        m_is_favorite = !m_is_favorite;
    }

    Service_Category category() const
    {
        return m_category;
    }

    const std::set<Zone_ID_t> &zones() const
    {
        return m_zones;
    }

    void set_zones(std::set<Zone_ID_t> &&_zones)
    {
        m_zones = std::move(_zones);
    }

    void insert_zone(Zone_ID_t _zone_id)
    {
        m_zones.insert(_zone_id);
    }

    Order_In_Full_t get_order_in_full() const
    {
        return m_order_in_full;
    }

    void set_order_in_full(Order_In_Full_t _order_in_full)
    {
        m_order_in_full = _order_in_full;
    }

    Order_In_Favorite_t get_order_in_favorite() const
    {
        return m_order_in_favorite;
    }

    void set_order_in_favorite(Order_In_Favorite_t _order_in_favorite)
    {
        m_order_in_favorite = _order_in_favorite;
    }

#ifdef MBGUI_SAT_MONITOR
    bool playable() const
    {
        return m_playable;
    }

    std::chrono::milliseconds time_to_start() const
    {
        return m_time_to_start;
    }
#endif

    const std::vector<AudioPid> &audio_pids() const
    {
        return m_audio_pids;
    }

    void clear_audio_pids()
    {
        m_audio_pids.clear();
    }

    void add_audio_pid(AudioPid _audio_pid)
    {
        m_audio_pids.emplace_back(std::move(_audio_pid));
    }

    template<typename... Args>
    void add_audio_pid(Args... args)
    {
        m_audio_pids.emplace_back(args...);
    }

    const AudioPid &current_audio() const
    {
        if(m_current_audio < m_audio_pids.size())
        {
            return m_audio_pids[m_current_audio];
        }
        else if(m_audio_pids.size())
        {
            return m_audio_pids[0];
        }
        else
        {
            return s_empty_audio_pid;
        }
    }

    void set_current_audio_index(size_t index)
    {
        if(index < m_audio_pids.size())
        {
            m_current_audio = index;
        }
    }

    auto current_audio_index() const
    {
        return m_current_audio;
    }

    const AudioPid &next_audio();

    void sort_audio_pids()
    {
        std::sort(m_audio_pids.begin(), m_audio_pids.end(),
                  [](const auto & rhs, const auto & lhs)
        {
            return rhs.pid < lhs.pid;
        }
                 );
    }

    PID_t subtitle_pid() const
    {
        return m_subtitle_pid;
    }

    void set_subtitle_pid(PID_t _subtitle_pid)
    {
        m_subtitle_pid = _subtitle_pid;
    }

    uint16_t satellite_id() const
    {
        return m_satellite_id;
    }

    void set_satellite_id(uint16_t _satellite_id)
    {
        m_satellite_id = _satellite_id;
        m_transponder_id.set_satellite_id(_satellite_id);
    }

    void map_service_type(const Transponder *_tp);
};

struct Transponder
{
    constexpr Transponder() {}
    constexpr Transponder(const Transponder &other):
        transponder_id(other.transponder_id),
        symbol_rate(other.symbol_rate),
        dvb_mode(other.dvb_mode),
        transport_stream_id(other.transport_stream_id),
        original_network_id(other.original_network_id),
        network_id(other.network_id),
        is_home_channel(other.is_home_channel),
#ifdef MBGUI_SAT_MONITOR
        failed_to_lock(other.failed_to_lock),
#endif
        satellite_id(other.satellite_id),
        pat_version_number(other.pat_version_number)
    {
    }

    constexpr Transponder(Transponder &&other):
        transponder_id(other.transponder_id),
        symbol_rate(other.symbol_rate),
        dvb_mode(other.dvb_mode),
        transport_stream_id(other.transport_stream_id),
        original_network_id(other.original_network_id),
        network_id(other.network_id),
        is_home_channel(other.is_home_channel),
#ifdef MBGUI_SAT_MONITOR
        failed_to_lock(other.failed_to_lock),
#endif
        satellite_id(other.satellite_id),
        pat_version_number(other.pat_version_number)
    {
    }

    constexpr Transponder &operator=(Transponder &&other)
    {
        transponder_id = other.transponder_id;
        symbol_rate = other.symbol_rate;
        dvb_mode = other.dvb_mode;
        transport_stream_id = other.transport_stream_id;
        original_network_id = other.original_network_id;
        network_id = other.network_id;
        is_home_channel = other.is_home_channel;
#ifdef MBGUI_SAT_MONITOR
        failed_to_lock = other.failed_to_lock;
#endif
        satellite_id = other.satellite_id;
        pat_version_number = other.pat_version_number;
        return *this;
    }

   constexpr Transponder &operator=(const Transponder &other)
    {
        transponder_id = other.transponder_id;
        symbol_rate = other.symbol_rate;
        dvb_mode = other.dvb_mode;
        transport_stream_id = other.transport_stream_id;
        original_network_id = other.original_network_id;
        network_id = other.network_id;
        is_home_channel = other.is_home_channel;
#ifdef MBGUI_SAT_MONITOR
        failed_to_lock = other.failed_to_lock;
#endif
        satellite_id = other.satellite_id;
        pat_version_number = other.pat_version_number;
        return *this;
    }

    Transponder(Transponder_Id _transponder_id, Symbol_Rate_t _symbol_rate, DVB_Mode _dvb_mode,
                TS_ID_t _transport_stream_id, ONID_t _original_network_id, NID_t _network_id, bool _is_home_channel):

        transponder_id(_transponder_id),
        symbol_rate(_symbol_rate),
        dvb_mode(_dvb_mode),
        transport_stream_id(_transport_stream_id),
        original_network_id(_original_network_id),
        network_id(_network_id),
        is_home_channel(_is_home_channel),
        satellite_id(_transponder_id.satellite_id())
    {
    }

    Transponder_Id transponder_id;
    Symbol_Rate_t symbol_rate { 0 };
    DVB_Mode dvb_mode { DVB_Mode::UNDEFINED };
    TS_ID_t transport_stream_id { 0 };
    ONID_t original_network_id { 0 };
    NID_t network_id { 0 };
    bool is_home_channel { false };
    Satellite_ID_t satellite_id { 0 };
#ifdef MBGUI_SAT_MONITOR
    bool failed_to_lock { true };
#endif

    PAT_Ver_t pat_version_number { INVALID_PAT_VERSION };

    bool operator==(const Transponder &_other) const
    {
        return transponder_id == _other.transponder_id && satellite_id == _other.satellite_id;
    }

    bool operator!=(const Transponder &_other) const
    {
        return transponder_id != _other.transponder_id || satellite_id != _other.satellite_id;
    }

    bool operator==(const Transponder_Id &_other) const
    {
        return transponder_id == _other;
    }

    bool operator!=(const Transponder_Id &_other) const
    {
        return transponder_id != _other;
    }

    void set_transponder(Transponder_Id _transponder_id, Symbol_Rate_t _symbol_rate, DVB_Mode _dvb_mode,
                            TS_ID_t _transport_stream_id, ONID_t _original_network_id, NID_t _network_id, bool _is_home_channel,
                            uint16_t _satellite_id = 0)
    {
        transponder_id = _transponder_id;
        transponder_id.set_satellite_id(_satellite_id);
        symbol_rate = _symbol_rate;
        dvb_mode = _dvb_mode;
        transport_stream_id = _transport_stream_id;
        original_network_id = _original_network_id;
        network_id = _network_id;
        is_home_channel = _is_home_channel;
        satellite_id = _satellite_id;
    }

    void set_satellite_id(uint16_t _satellite_id)
    {
        satellite_id = _satellite_id;
        transponder_id.set_satellite_id(_satellite_id);
    }
};

inline bool operator==(const Transponder_Id &_lhs, const Transponder &_rhs)
{
    return _rhs.transponder_id == _lhs;
}

typedef std::vector<Satellite> Satellite_List;
typedef std::vector<Transponder> Transponder_List;
typedef std::vector<Service> Service_List;

struct OTA_TS_PID
{
    Transponder transponder;
    PID_t pid { 0 };
    uint16_t software_version { 0 };
    uint8_t download_mode { 0 };
    uint8_t factory_reset_flag { 0 };
};
typedef std::vector<OTA_TS_PID> OTA_TS_PID_List;

struct Lineup
{
    Lineup();
    Service_List services;
    Service_List hidden_services;
    Transponder_List transponders;
    Satellite_List m_satellites;
    Channel_List_Type m_channel_list_type { Channel_List_Type::MY_TV_CHANNELS };
    bool m_lineup_table_empty = true;

    std::tuple<Service, Channel_List_Type> m_last_service;
    std::tuple<Service, Channel_List_Type> m_current_service;
    std::tuple<Service, Channel_List_Type> m_last_tv_service;
    std::tuple<Service, Channel_List_Type> m_last_radio_service;

    void clear();
    void clear_satellite(uint16_t _satellite_id);
    void filter_lineup();
    void set_table_not_empty();
    void set_channel_list_type(Channel_List_Type _list_type);
    Channel_List_Type get_channel_list_type();
    void set_favorite_by_viewer_channel(Viewer_Channel_t _viewer_channel, bool _is_favorite);
    void set_current_service(Service *_current_service);
    Service *get_service_by_viewer_channel(Viewer_Channel_t _viewer_channel);
    Service *get_service(Service_ID_t _service_id, Transponder_Id _transponder_id);
    void toggle_tv_radio_channel_list_type();
    Service *get_current_service();
    Service *get_next_service();
    Service *get_previous_service();
    void delete_current_service();
    void clone_current_service();
    Service_ID_t get_last_available_service_id();
    Service_ID_t get_max_service_id();
    Viewer_Channel_t get_last_available_viewer_channel();
    Viewer_Channel_t get_max_viewer_channel();
    std::string get_sattelite_name(Satellite_ID_t satellite_id);

    struct Channel_Info
    {
        Channel_Info(const Service &_srv);

        std::string         channel_name;
        Viewer_Channel_t    viewer_channel;
        bool                favorite;
        Transponder_Id      transponder_id;
        Service_ID_t        service_id;
        Order_In_Full_t     order_in_full;
        Order_In_Favorite_t order_in_favorite;
    };

    std::vector<Channel_Info> get_list(Channel_List_Type _type);
    std::vector<Channel_Info> get_tv_list();
    std::vector<Channel_Info> get_radio_list();
    std::vector<Channel_Info> get_favorite_tv_list();
    std::vector<Channel_Info> get_favorite_radio_list();
    int  get_current_channel_index();
    int  get_next_channel_index_by_type(int base, int direction);
    void preview_channel(int index);
    void channel_change_by_index(int index);
    Service *get_next_channel(int _direction, const std::vector<Channel_Info> &_srvs);
    Service *get_first_service();
    Service *get_last_service();
    void channel_change(Service *_srv);
    bool channel_change_by_viewer_channel(Viewer_Channel_t _viewer_channel);
    void swap_last_service();
    Order_In_Full_t get_max_order_in_full();
    Order_In_Favorite_t get_max_order_in_favorite();
    std::map<std::string, std::string> get_current_lock_info();
    void load_satellite_list();
    bool is_tv_service(Service_Type _type) const
    {
        return _type == Service_Type::advanced_codec_hd_digital_television_service
               or _type == Service_Type::advanced_codec_sd_digital_television_service
               or _type == Service_Type::digital_television_service;
    }
    bool is_radio_service(Service_Type _type) const
    {
        return _type == Service_Type::digital_radio_sound_service;
    }

    const Transponder *get_transponder(const Transponder_Id &_id) const
    {
        auto it = std::find_if(transponders.cbegin(), transponders.cend(), [_id](const auto & t)
        {
            return t.transponder_id == _id;
        });

        if(it != transponders.cend())
        {
            return &*it;
        }

        return nullptr;
    }

    Transponder *get_transponder(const Transponder_Id &_id)
    {
        auto it = std::find_if(transponders.begin(), transponders.end(), [_id](const auto & t)
        {
            return t.transponder_id == _id;
        });

        if(it != transponders.end())
        {
            return &*it;
        }

        return nullptr;
    }
};

class Lineup_Mutex_Ref
{
private:
    bool m_is_locked { false };
    Lineup_Mutex_Ref();

public:
    ~Lineup_Mutex_Ref();

    static Lineup_Mutex_Ref get_current_lineup();
    static bool is_empty();
    Lineup *operator->();
    Lineup *get();
    void reset();
};

inline size_t get_current_lineup_size()
{
    auto current_lineup = Lineup_Mutex_Ref::get_current_lineup();
    return current_lineup->services.size();
}

#ifdef MBGUI_SAT_MONITOR
std::string serialize(const Lineup *_line_up);
#endif

Audio_Codec audio_codec_from_stream_type(uint8_t _stream_type);

} // namespace mb
