#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <cstring>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "dvb/mb_dvb_types.h"

#ifdef MBGUI_SAT_MONITOR
#include <chrono>
#endif

namespace mb {

class Satellite_Delivery_System_Descriptor;
class S2_Satellite_Delivery_System_Descriptor;

typedef uint8_t  Section_Number_t;
typedef uint16_t Bouquet_ID_t;
typedef uint16_t Event_ID_t;
typedef uint32_t Frequency_t;
typedef uint16_t NID_t;
typedef uint16_t ONID_t;
typedef uint16_t PID_t;
typedef uint8_t  PAT_Ver_t;
typedef uint16_t Program_Number_t;
typedef uint16_t Service_ID_t;
typedef uint16_t EIT_Service_ID_t;
typedef uint32_t Symbol_Rate_t;
typedef uint8_t  Table_ID_t;
typedef uint16_t TS_ID_t;
typedef uint32_t Viewer_Channel_t;
typedef uint8_t  Volume_t;
typedef uint8_t  Zone_ID_t;
typedef uint16_t Segment_ID_t;
typedef uint16_t General_Order_t;
typedef uint16_t Order_By_Type_t;
typedef uint64_t Order_In_Full_t;
typedef uint64_t Order_In_Favorite_t;
typedef uint16_t Satellite_ID_t;

// Nagra
typedef std::string NAGRA_NUID_t;
typedef std::string NAGRA_CAID_t;
typedef std::string NAGRA_SCUA_t;
typedef std::string CAK_Version_t;
typedef std::string Project_Info_t;
typedef std::string Chipset_Type_t;
typedef std::string Chipset_Revision_t;

enum class Polarity
{
    Horizontal,
    Vertical,
    Left,
    Right,
    UNDEFINED
};
std::string_view to_str(Polarity _type);

enum class DVB_Mode
{
    DVBS,
    DVBS2,
    DVBS2X,
    DVBT,
    DVBT2,
    UNDEFINED
};
std::string_view to_str(DVB_Mode _type);

enum class Modulation_Type
{
    Default,
    QAM_4,
    QAM_4_NR,
    QAM_16,
    QAM_32,
    QAM_64,
    QAM_128,
    QAM_256,
    QAM_512,
    BPSK,
    QPSK,
    DQPSK,
    PSK_8,
    APSK_16,
    APSK_32,
    APSK_64,
    APSK_128,
    APSK_256,
    APSK_L_8,
    APSK_L_16,
    APSK_L_32,
    APSK_L_64,
    APSK_L_128,
    APSK_L_256,
    VSB_8,
    VSB_16,
    AUTO,
    UNDEFINED
};
std::string_view to_str(Modulation_Type _type);

enum class Band
{
    C,
    Ku,
    UNDEFINED
};
std::string_view to_str(Band _type);

enum class LNBF_Type
{
    Mono,
    Multi,
    Universal,
    UNDEFINED
};
std::string_view to_str(LNBF_Type _type);

enum class LNBF_Position
{
    Normal,
    Inverted
};
std::string_view to_str(LNBF_Position _type);

enum class DiseqC_Type
{
    None,
    DiseqC_1_0,
    DiseqC_1_1,
    LNBF_Switch
};
std::string_view to_str(DiseqC_Type _type);

enum class FEC_Rate
{
    AUTO,
    FEC_1_2,
    FEC_2_3,
    FEC_3_4,
    FEC_4_5,
    FEC_5_6,
    FEC_6_7,
    FEC_7_8,
    FEC_8_9,
    FEC_9_10,
    FEC_1_4,
    FEC_1_3,
    FEC_2_5,
    FEC_3_5,
    FEC_5_9,
    FEC_7_9,
    FEC_4_15,
    FEC_7_15,
    FEC_8_15,
    FEC_11_15,
    FEC_13_18,
    FEC_9_20,
    FEC_11_20,
    FEC_23_36,
    FEC_25_36,
    FEC_11_45,
    FEC_13_45,
    FEC_14_45,
    FEC_26_45,
    FEC_28_45,
    FEC_29_45,
    FEC_31_45,
    FEC_32_45,
    FEC_77_90,
    UNDEFINED
};

std::string_view to_str(FEC_Rate _fec_rate);

enum class Video_Codec
{
    None,
    MPEG2,
    MPEG4,
    H264,
    HEVC,
    UNDEFINED
};
std::string_view to_str(Video_Codec _codec);

enum class Audio_Codec
{
    None,
    AAC,
    MP1,
    MP2,
    AC3,
    UNDEFINED
};
std::string_view to_str(Audio_Codec _codec);

enum class Network_Policies
{
    TVRO = 1, // Same as Claro
    Sky = 2,
    Generic = 3,
    Nagra_Test_Streams = 99
};

enum Network_Id : NID_t
{
    Network_Id_Generic = 0,
    Network_Id_Claro = 45626,
    Network_Id_Sky = 162,
    Network_Id_Nagra = 0x31,
};

enum class Satellite_Operator
{
    Claro = 1 ,
    Sky = 2,
    Generic = 3,
};
std::string_view to_str(Satellite_Operator _operator);

enum class Lineup_Origin
{
    LO_DATABASE,
    LO_SATELLITE,
};
std::string_view to_str(Lineup_Origin _lineup_origin);

enum class Aspect_Mode
{
    AUTO = 0,
    PILLBOX_16x9,
    PANSCAN_16x9,
    LETTERBOX_16x9,
    FULLSCREEN_16x9,
    PANSCAN_4x3,
    LETTERBOX_4x3,
    FULLSCREEN_4x3
};
std::string_view to_str(Aspect_Mode _aspect_mode);

enum class Color_Standard
{
    None,
    NTSC_60,
    PAL_M_50,
    PAL_M_60,
    PAL_N_50,
    UNDEFINED
};
std::string_view to_str(Color_Standard _color_standart);

enum class Resolution_Standard
{
    _480i_60Hz,
    _480p_60Hz,
    _720p_60Hz,
    _1080i_30Hz,
    _1080p_60Hz,
};
std::string_view to_str(Resolution_Standard _resolution);

// Status locais que devem se tornar globais
enum class Clock_Type
{
    Auto,
    Manual,
    Timezone,
};
std::string_view to_str(Clock_Type _clock_status);

enum class Timezone_Mode
{
    Auto,
    F_Noronha_UTC_2,
    Brasilia_UTC_3,
    Amazonas_UTC_4,
    Acre_UTC_5,
};
std::string_view to_str(Timezone_Mode _timezone_mode);

enum class Language_Mode
{
    Portugues,
    Ingles,
};
std::string_view to_str(Language_Mode _language_mode);

enum class Message_Categories
{
    Program_Access,
    Program_Access_Denied,
    Event_Popup,
    Event_Finger_Print,
    Event_CAK_Reset,
    COUNT
};
std::string_view to_str(Message_Categories _category);

enum class CC_Type
{
    Disabled,
    Closed_Caption,
    Subtitle,
    COUNT
};
std::string_view to_str(CC_Type _cc_type);

enum class Channel_List_Type
{
    MY_TV_CHANNELS,
    MY_RADIO_CHANNELS,
    ALL_TV_CHANNELS,
    ALL_RADIO_CHANNELS,
    COUNT,
};
inline Channel_List_Type increment_value(Channel_List_Type _value, int _increment)
{
    return static_cast<Channel_List_Type>((static_cast<int>(_value) + static_cast<int>(Channel_List_Type::COUNT) + _increment) % static_cast<int>(Channel_List_Type::COUNT));
}
std::string_view to_str(Channel_List_Type _channel_list_type);

enum class Schedule_Operation
{
    RECORD = 0,
    REMIND
};

enum class Schedule_Repeat
{
    ONCE = 0,
    DAILY,
    WEEKLY
};

enum class Schedule_Status
{
    PAUSED = 0,
    ACTIVE
};

enum class Subtitle_Font_Size
{
    Small,
    Medium,
    Large
};

enum class Subtitle_Font_Color
{
    White,
    Yellow,
    Black
};

enum class Subtitle_Background_Color
{
    None,
    Grey,
    Yellow
};

template <typename  T>
class ET
{
private:
    T m_value;

public:
    explicit ET(): m_value(T{}) {}
    explicit ET(T _v): m_value(_v) {}

    bool is_null() const
    {
        return m_value == 0;
    }

    bool operator==(const ET<T> &_other) const
    {
        return m_value == _other.m_value;
    }
    bool operator!=(const ET<T> &_other) const
    {
        return m_value != _other.m_value;
    }
    bool operator>(const ET<T> &_other) const
    {
        return m_value > _other.m_value;
    }
    bool operator<(const ET<T> &_other) const
    {
        return m_value < _other.m_value;
    }
};

class DVB_Table_Section
{
public:
    typedef uint16_t Size_Type;

private:
    struct Ptr;
    std::shared_ptr<Ptr> m_p;

public:
    DVB_Table_Section() {}
    DVB_Table_Section(const uint8_t *_data, Size_Type _size);

    void assign(uint8_t *_data, Size_Type _size);

    DVB_Table_Section(const DVB_Table_Section &_other);
    DVB_Table_Section(DVB_Table_Section &&_other);

    void operator=(DVB_Table_Section &&_other);
    void operator=(const DVB_Table_Section &_other);

    const uint8_t *data() const;
    Size_Type size() const;
    bool empty() const;

    void reset()
    {
        m_p.reset();
    }

    operator bool() const
    {
        return not empty();
    }
};

constexpr PAT_Ver_t INVALID_PAT_VERSION = 0xFF;

struct Transponder_Id
{
public:
    typedef uint64_t Id_Type;

private:
    static constexpr auto POLARITY_BITMAP = 0b111;
    static constexpr auto FREQUENCY_BITMAP = 0xFFFFFFFFLL - POLARITY_BITMAP;

    Id_Type m_frequency { 0 };

public:
    constexpr Transponder_Id()
    {
    }

    constexpr Transponder_Id(const Transponder_Id &_other):
        m_frequency(_other.m_frequency)
    {}

    constexpr Transponder_Id(Transponder_Id &&_other):
        m_frequency(_other.m_frequency)
    {}

    constexpr Transponder_Id(Frequency_t _frequency, Polarity _polarity, Satellite_ID_t _satellite_id)
    {
        set_frequency(_frequency, _polarity, _satellite_id);
    }

    constexpr void set_frequency(Frequency_t _frequency, Polarity _polarity, Satellite_ID_t _satellite_id)
    {
        m_frequency = ((static_cast<uint64_t>(_frequency) & FREQUENCY_BITMAP) + static_cast<uint64_t>(_polarity)) | (static_cast<uint64_t>(_satellite_id) << 32);
    }

    constexpr void set_id(Id_Type _id)
    {
        m_frequency = _id;
    }

    Id_Type id() const
    {
        return m_frequency;
    }

    Frequency_t frequency() const
    {
        return static_cast<Frequency_t>(m_frequency & FREQUENCY_BITMAP);
    }

    Polarity polarity() const
    {
        return static_cast<Polarity>(m_frequency & POLARITY_BITMAP);
    }

    Satellite_ID_t satellite_id() const
    {
        return static_cast<Satellite_ID_t>(m_frequency >> 32);
    }

    void set_satellite_id(Satellite_ID_t _satellite_id)
    {
        m_frequency = (m_frequency & 0xFFFFFFFFLL) | (static_cast<uint64_t>(_satellite_id) << 32);
    }

    constexpr void operator=(const Transponder_Id &_other)
    {
        m_frequency = _other.m_frequency;
    }

    constexpr bool operator==(const Transponder_Id &_other) const
    {
        return m_frequency == _other.m_frequency;
    }

    constexpr bool operator!=(const Transponder_Id &_other) const
    {
        return m_frequency != _other.m_frequency;
    }

    constexpr bool operator<(const Transponder_Id &_other) const
    {
        return m_frequency < _other.m_frequency;
    }

    constexpr bool operator>(const Transponder_Id &_other) const
    {
        return m_frequency > _other.m_frequency;
    }
};

template<typename T>
T &operator<<(T &_stream, Transponder_Id _tp)
{
    auto f = std::to_string(_tp.frequency() / 1000) + "/";
    const char *polarity = nullptr;

    switch(_tp.polarity())
    {
        case Polarity::Horizontal:
            polarity = "H";
            break;

        case Polarity::Vertical:
            polarity = "V";
            break;

        case Polarity::Left:
            polarity = "L";
            break;

        case Polarity::Right:
            polarity = "R";
            break;

        case Polarity::UNDEFINED:
            polarity = "U";
            break;
    }

    auto satellite_id = std::to_string(_tp.satellite_id()) +"/";
    _stream << satellite_id + f + polarity;
    return _stream;
}

enum class OTA_Type
{
    EiTV,
    Skyworth,
    Century_DSI,
};
std::string_view to_str(OTA_Type _ota_type);

struct OTA_Config
{
    Transponder_Id transponder;
    OTA_Type type;
    PID_t pid { 0 };
    Table_ID_t table_id { 0 };
};

constexpr auto OTA_CONFIGS_COUNT = 3;
extern std::array<OTA_Config, OTA_CONFIGS_COUNT> g_ota_configs;

struct OTA_File
{
    OTA_Type type;
    PID_t pid { 0 };
    Table_ID_t table_id { 0 };
    uint8_t product { 0 };
    uint16_t version { 0 };
    uint16_t channel_list_version { 0 };
    std::string channel_list;
    uint32_t product_id { 0 };
    uint16_t sw_current { 0 };
    uint16_t sw_min { 0 };
};

struct CA_Info
{
    CA_Type type { CA_Type::None };
    uint16_t id { 0 };
    PID_t pid;
};

struct DVB_Subtitle_Info
{
    std::string iso639_language_code;
    uint8_t subtitling_type { 0 };
    uint16_t composition_page_id { 0 };
    uint16_t ancillary_page_id { 0 };
};

#define MBGUI_HASH_COMBINE 1

inline void hash_combine(std::size_t &) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t &seed, const T &v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

} // namespace mb

namespace std {
template<>
struct hash<mb::Transponder_Id> : public __hash_base<size_t, mb::Transponder_Id::Id_Type>
{
    size_t operator()(const mb::Transponder_Id &__val) const noexcept
    {
        return std::_Hash_impl::hash(__val.id());
    }
};

template<>
struct hash<mb::OTA_File>
{
    size_t operator()(const mb::OTA_File &f) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, static_cast<int>(f.type), f.pid,  f.table_id,
                         f.product, f.version, f.channel_list_version, f.channel_list,
                         f.product_id, f.sw_current, f.sw_min);
        return result;
    }
};

template<>
struct hash<mb::CA_Info>
{
    size_t operator()(const mb::CA_Info &c) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, c.type, c.id, c.pid);
        return result;
    }
};

template<>
struct hash<vector<mb::CA_Info>>
{
    size_t operator()(const vector<mb::CA_Info> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &c : v)
        {
            mb::hash_combine(result, c);
        }

        return result;
    }
};

template<>
struct hash<mb::DVB_Subtitle_Info>
{
    size_t operator()(const mb::DVB_Subtitle_Info &s) const noexcept
    {
        size_t result { 0 };
        mb::hash_combine(result, s.iso639_language_code, s.subtitling_type, s.composition_page_id, s.ancillary_page_id);
        return result;
    }
};

template<>
struct hash<vector<mb::DVB_Subtitle_Info>>
{
    size_t operator()(const vector<mb::DVB_Subtitle_Info> &v) const noexcept
    {
        size_t result { 0 };

        for(const auto &s : v)
        {
            mb::hash_combine(result, s);
        }

        return result;
    }
};

} // namespace std
