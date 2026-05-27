#pragma once

#include <inttypes.h>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <random>
#include <cstring>

#include "mb_assert.h"
#include "mb_debug.h"

#include "libgen.h"

#ifdef MBGUI_SAT_MONITOR
#include "mb_ip_sat_monitor.h"
#endif

#define MBGUI_SOCKET_DIR "/"
#define MBGUI_SOCKET_PATH_GUI MBGUI_SOCKET_DIR "mbgui"
#define MBGUI_SOCKET_PATH_CAS MBGUI_SOCKET_DIR "mbcas"

#define MBGUI_SOCKET_MAX_MESSAGES 10
#define MBGUI_SOCKET_MSGSIZE 8192

#define MBGUI_CC_MAX_LINES 4

#define MBGUI_STORAGE_PATH "/usr/mnt_app/mbgui/"
#define MBGUI_CACHE_PATH "/usr/mnt_vfs/mbgui/"

#define MBGUI_MANUFACTURE_DATE_FILE MBGUI_STORAGE_PATH "manufacture_date.txt"
#define MBGUI_LAST_ACTIVATION_DATE_FILE MBGUI_STORAGE_PATH "last_activation.txt"
#define MBGUI_TERMS_CONDITIONS_JSON_FILE MBGUI_STORAGE_PATH "terms_conditions.json"
#define MBGUI_TERMS_CONDITIONS_DATE_FILE MBGUI_STORAGE_PATH "terms_conditions_date.txt"
#define MBGUI_ZONE_ID_FILE MBGUI_STORAGE_PATH "zone.id"

#define MBGUI_LOCAL_DB MBGUI_CACHE_PATH "mbgui.db"
#define MBGUI_CONFIG_FILE MBGUI_CACHE_PATH "mbgui.cfg"


#if defined(MBCAS_CAK) and defined(MBCAS_USE_DIR_EXT)
#define MBCAS_EXTENSION "." MBCAS_CAK
#else
#define MBCAS_EXTENSION
#endif

#define MBGUI_NAGRA_PERSO_DATA_PATH "/usr/mnt_app" MBCAS_EXTENSION "/nagra/"
#define MBGUI_NAGRA_STORAGE_PATH "/usr/mnt_vfs/nagra/"
#define MBGUI_NAGRA_TRUSTED_STORAGE_PATH "/usr/mnt_app" MBCAS_EXTENSION "/nagra/"

#define MBGUI_NAGRA_PK_FILE "0303.pk"
#define MBGUI_NAGRA_PK_HASH "0303.sha256sum"
#define MBGUI_NAGRA_CASN_FILE "casn.txt"
#define MBGUI_NAGRA_CSC_FILE "csc.dat"
#define MBGUI_NAGRA_CSC_HASH "csc.sha256sum"
#define MBGUI_NAGRA_IRD_CACHE_FILE MBGUI_NAGRA_STORAGE_PATH "ird.cache"

#define MBGUI_CENTURY_WHASTAPP_NUMBER "(12) 3042-2700"
#define MBGUI_CENTURY_LINK_SAC "https://api.whatsapp.com/send?phone=+551230422700&text=Ol%C3%A1,%20preciso%20de%20atendimento"
#define MBGUI_CENTURY_HOMEPAGE "https://www.centurybr.com.br"
#define MBGUI_PRODUCT_NAME "MidiaBox"
#define MBGUI_MODEL_NAME "B8"

#define MBGUI_SKY_OTA_OUI 0x15C009

#define MBGUI_SKY_OTA_HW_CODE 0x08
#define MBGUI_SKY_OTA_MODEL 0x01

#define NAGRA_MAX_PK_SIZE 0x800
#define NAGRA_MAX_CSCD_SIZE 0x1000
#define NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA 0x9000
#define NAGRA_CSCD_START_ADDR_IN_BOOT_TOTAL_AREA 0x8000

#define USB_PATH (getenv("MBGUI_DEFAULT_USB_PATH") ? getenv("MBGUI_DEFAULT_USB_PATH") : "/mnt/usb/")

#define ONE_MEGA_BYTE 1024*1024

#define CONFIG_FILE_FINAL_TEST "config_teste_final.json"


#define MBGUI_NAGRA_TEST_STREAM_NETWORK_ID  0x31
#define MBGUI_NAGRA_UMASK                   0077

#define ZAPPING_TIMEOUT_MS  1000

#ifdef __clang__
// Disable Clang warning for IDE diagnostics
#pragma clang diagnostic ignored "-Wvla"
#endif

namespace {
constexpr auto DISPLAY_WIDTH = 1280;
constexpr auto DISPLAY_HEIGHT = 720;
constexpr auto ANIM_BAR_DURATION = 150;

constexpr auto NAGRA_STDIN_TIMEOUT = std::chrono::milliseconds(100);
}

constexpr const char *file_name(const char *_name)
{
    auto result { _name };
    bool get_next { false };

    for(auto p = _name, end = _name + std::char_traits<char>::length(_name); p < end; p++)
    {
        if(*p == '/')
        {
            get_next = true;
        }
        else if(get_next)
        {
            result = p;
            get_next = false;
        }
    }

    return result;
}

#define MB_ZERO(var) memset(&var, 0, sizeof(var))

namespace mb {

const extern std::array<const std::string_view, 6> COMPATIBLE_VIDEO_FORMATS;
const extern std::array<const std::string_view, 3> COMPATIBLE_AUDIO_FORMATS;
const extern std::array<const std::string_view, 2> COMPATIBLE_PHOTO_FORMATS;
const extern std::array<const std::string_view, 1> COMPATIBLE_RECORDING_FORMATS;

extern int8_t g_timezone_offset;
extern uint8_t g_clock_set;
extern bool g_factory_reset_enabled;
extern bool g_production_final_test;

std::string convert_iso8859_1(const uint8_t *_data, size_t _size);
size_t utf_charcount(std::string_view _str);
std::string sanitize_filename(std::string_view _name);

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g)
{
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

inline auto now()
{
    return std::chrono::steady_clock::now();
}

template<typename T>
auto time_elapsed(const T &_time_point)
{
    return decltype(_time_point)::clock::now() - _time_point;
}

constexpr auto LOCK_TIMEOUT = std::chrono::milliseconds
{
    5'000
};

constexpr auto START_PLAY_TIMEOUT = std::chrono::milliseconds
{
    4'500
};

constexpr auto SIGNAL_CHECK_INTERVAL = std::chrono::milliseconds
{
    250
};

constexpr auto MIN_RESTART_TIME = std::chrono::milliseconds
{
    15'000
};

inline time_t clock_to_time_t(const std::chrono::steady_clock::time_point &t)
{
    using namespace std::chrono;
    return system_clock::to_time_t(system_clock::now()
                                   + duration_cast<system_clock::duration>(t - steady_clock::now()));
};

inline time_t clock_to_time_t(const std::chrono::system_clock::time_point &t)
{
    using namespace std::chrono;
    return system_clock::to_time_t(t);
};

inline std::ostream& operator<<(std::ostream& _str, const std::chrono::time_point<std::chrono::system_clock> &_tp)
{
    auto t = std::chrono::system_clock::to_time_t(_tp);
    _str << std::put_time(std::localtime(&t), "%F %T");
    return _str;
}

template<class T, class Deleter>
class scoped_var
{
private:
    T m_val;
    Deleter m_deleter;

public:
    explicit scoped_var(T _val, Deleter _deleter):
        m_val(_val),
        m_deleter(_deleter)
    {
    }

    ~scoped_var()
    {
        m_deleter(m_val);
    }

    operator T()
    {
        return m_val;
    }

    template <typename U>
    bool operator>(const U &_val) const
    {
        return m_val > _val;
    }

    template <typename U>
    bool operator<(const U &_val) const
    {
        return m_val < _val;
    }

    template <typename U>
    bool operator!=(const U &_val) const
    {
        return m_val != _val;
    }

    template <typename U>
    bool operator==(const U &_val) const
    {
        return m_val == _val;
    }
};

bool run_process(const char *file, const char *const argv[]);


// =============================================================================
// deferred_call:
// --------------
// This struct enables us to implement deferred function calls simply in
// the defer() function below.  It forces a given function to automatically
// be called at the end of scope using move-only semantics.  Most
// commonly, the given function will be a lambda but that is not required.
// See the defer() function (below) for more on this
// =============================================================================
template <typename FUNC>
struct deferred_call
{
    // Disallow assignment and copy
    deferred_call(const deferred_call &that) = delete;
    deferred_call &operator=(const deferred_call &that) = delete;

    // Pass in a lambda

    deferred_call(FUNC &&f):
        m_func(std::forward<FUNC>(f)), m_is_owner(true)
    {
    }

    // Move constructor, since we disallow the copy
    deferred_call(deferred_call &&that):
        m_func(std::move(that.m_func)), m_is_owner(that.m_bOwner)
    {
        that.m_bOwner = false;
    }

    // Destructor forces deferred call to be executed
    ~deferred_call()
    {
        execute();
    }

    // Prevent the deferred call from ever being invoked
    bool cancel()
    {
        bool m_was_owner = m_is_owner;
        m_is_owner = false;
        return m_was_owner;
    }

    // Cause the deferred call to be invoked NOW
    bool execute()
    {
        const auto bWasOwner = m_is_owner;

        if(m_is_owner)
        {
            m_is_owner = false;
            m_func();
        }

        return bWasOwner;
    }

private:
    FUNC m_func;
    bool m_is_owner;
};


// -----------------------------------------------------------------------------
// defer:  Generic, deferred function calls
// ----------------------------------------
//      This function template the user the ability to easily set up any
//      arbitrary  function to be called *automatically* at the end of
//      the current scope, even if return is called or an exception is
//      thrown.  This is sort of a fire-and-forget.  Saves you from having
//      to repeat the same code over and over or from having to add
//      exception blocks just to be sure that the given function is called.
//
//      If you wish, you may cancel the deferred call as well as force it
//      to be executed BEFORE the end of scope.
//
// Example:
//      void Foo()
//      {
//          auto callOnException  = defer([]{ SomeGlobalFunction(); });
//          auto callNoMatterWhat = defer([pObj](pObj->SomeMemberFunction(); });
//
//          // Do dangerous stuff that might throw an exception ...
//
//          ...
//          ... blah blah blah
//          ...
//
//          // Done with dangerous code.  We can now...
//          //      a) cancel either of the above calls (i.e. call cancel()) OR
//          //      b) force them to be executed (i.e. call execute()) OR
//          //      c) do nothing and they'll be executed at end of scope.
//
//          callOnException.cancel();    // no exception, prevent this from happening
//
//          // End of scope,  If we had not canceled or executed the two
//          // above objects, they'd both be executed now.
//      }
// -----------------------------------------------------------------------------

template <typename F>
deferred_call<F> defer(F &&f)
{
    return deferred_call<F>(std::forward<F>(f));
}

std::string cat(const char *_origin);
void cat(const char *_origin, const char *_dest);

// trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s)
{
    ltrim(s);
    return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s)
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}

#ifndef NDEBUG
bool debuggerIsAttached();
#else
constexpr bool debuggerIsAttached()
{
    return false;
}
#endif

#ifndef NDEBUG
void dump_linux_capabilities();
#endif // NDEBUG

} // namespace mb
