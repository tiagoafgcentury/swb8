#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

#include "mb_events.h"
#include "common/mb_types.h"
#include "common/mb_lineup.h"

namespace mb {

class Player
{
public:
    enum class State
    {
        Idle,
        Opened,
        Closing,
        Starting,
        Started,
        Paused,
        Stopping,
        Stopped,
        Error,
    };

    typedef std::function < void(Event_CC &&) > CC_Callback;

protected:
    struct Data;
    std::unique_ptr<Data> m_p;

    std::atomic<State> m_state;
#if __cplusplus >= 201703L
    static_assert(std::atomic<State>::is_always_lock_free);
#endif

protected: // CC Stuff
    std::atomic<bool> m_cc_enabled { false };
    CC_Callback m_cc_callback;

    void parse_cc(unsigned char *_data, size_t _length);

public: // CC Stuff
    typedef enum
    {
        EIA608_COLOR_WHITE = 0,
        EIA608_COLOR_GREEN = 1,
        EIA608_COLOR_BLUE = 2,
        EIA608_COLOR_CYAN = 3,
        EIA608_COLOR_RED = 4,
        EIA608_COLOR_YELLOW = 5,
        EIA608_COLOR_MAGENTA = 6,
        EIA608_COLOR_USERDEFINED = 7
    } eia608_color_t;
    static constexpr auto EIA608_COLOR_DEFAULT = EIA608_COLOR_WHITE;

    typedef enum
    {
        EIA608_FONT_REGULAR    = 0x00,
        EIA608_FONT_ITALICS    = 0x01,
        EIA608_FONT_UNDERLINE  = 0x02,
        EIA608_FONT_UNDERLINE_ITALICS = EIA608_FONT_UNDERLINE | EIA608_FONT_ITALICS
    } eia608_font_t;

    static constexpr auto EIA608_SCREEN_ROWS = 15u;
    static constexpr auto EIA608_SCREEN_COLUMNS = 32u;

private: // CC Stuff
    enum CC_Mode
    {
        EIA608_MODE_POPUP = 0,
        EIA608_MODE_ROLLUP_2 = 1,
        EIA608_MODE_ROLLUP_3 = 2,
        EIA608_MODE_ROLLUP_4 = 3,
        EIA608_MODE_PAINTON = 4,
        EIA608_MODE_TEXT = 5
    };

    enum CC_Standard
    {
        EIA608,
        CEA708,
    };

    enum eia608_status_t
    {
        EIA608_STATUS_DEFAULT         = 0x00,
        EIA608_STATUS_CHANGED         = 0x01, /* current screen has been altered */
        EIA608_STATUS_CAPTION_ENDED   = 0x02, /* screen flip */
        EIA608_STATUS_CAPTION_CLEARED = 0x04, /* active screen erased */
        EIA608_STATUS_DISPLAY         = EIA608_STATUS_CAPTION_CLEARED | EIA608_STATUS_CAPTION_ENDED,
    };

    struct eia608_screen_t // A CC buffer
    {
        uint16_t characters[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS + 1];
        eia608_color_t colors[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS + 1];
        eia608_font_t fonts[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS + 1]; // Extra char at the end for a 0
        int row_used[EIA608_SCREEN_ROWS]; // Any data in row?
    };

    eia608_screen_t m_screen[2];
    int m_screen_idx = 0;

    CC_Standard m_standard = EIA608;
    CC_Mode m_mode = EIA608_MODE_POPUP;

    uint8_t m_last_d1 = 0;
    uint8_t m_last_d2 = 0;
    eia608_color_t m_color = EIA608_COLOR_WHITE;
    eia608_font_t m_font = EIA608_FONT_REGULAR;

    struct Cursor_Pos
    {
        uint8_t column = 0;
        uint8_t row = 0;
    };

    Cursor_Pos m_cursor;
    uint8_t m_row_rollup = 0;

    eia608_status_t Eia608Parse(const uint8_t data[2]);
    eia608_status_t Eia608ParseData(uint8_t d1, uint8_t d2);
    eia608_status_t Eia608ParseTextAttribute(uint8_t d2);
    void Eia608Cursor(int dx);
    eia608_status_t Eia608ParseSingle(const uint8_t dx);
    void Eia608Write(const uint8_t c);
    void Eia608Write(const char *c, bool replace_previous = false);
    void Eia608Erase();
    int Eia608GetWritingScreenIndex();
    eia608_status_t Eia608ParseExtended(uint8_t d1, uint8_t d2);
    eia608_status_t Eia608ParseCommand0x14(uint8_t d2);
    bool Eia608ParseCommand0x17(uint8_t d2);
    bool Eia608ParsePac(uint8_t d1, uint8_t d2);
    void Eia608EraseScreen(bool _displayed);
    void Eia608ClearScreenRowX(int i_screen, int i_row, int x);
    void Eia608ClearScreenRow(int i_screen, int i_row);
    void Eia608ClearScreen(int i_screen);
    void Eia608EraseToEndOfRow();
    void Eia608RollUp();

    void publish();

public:
    Player();
    virtual ~Player();

    void change_audio(const Service::AudioPid &_audio);
    void open(const Service &_service);
    virtual void close();

    virtual void start();
    virtual void stop();
    //virtual void pause();
    //virtual void resume();
    virtual void stop_closed_caption();
    virtual void start_closed_caption();
    virtual uint64_t get_player_stc();

    auto state() const
    {
        return m_state.load(std::memory_order_acquire);
    }

    void set_cc_callback(CC_Callback _callback);

#ifndef NDEBUG
    void get_player_status();
#endif
};

std::string_view to_str(Player::State _state);

} // namespace mb
