#include "mb_player.h"

// Use as reference: https://raw.githubusercontent.com/videolan/vlc/refs/heads/master/modules/codec/cc.c
//
// IMPORTANT: This implementation uses Unicode codepoints (uint16_t) internally for character storage.
// - EIA-608 special characters are mapped to UTF-8 strings in Eia608ParseExtended()
// - UTF-8 strings are decoded to Unicode codepoints when written to the buffer
// - Unicode codepoints are encoded back to UTF-8 when publishing to the display layer
// This approach ensures proper handling of special characters (ç, ú, é, ã, ♪, etc.) in all modes,
// especially POPUP mode where characters are buffered before display.

namespace mb {

static const struct
{
    Player::eia608_color_t  i_color;
    Player::eia608_font_t   i_font;
    int             i_column;
} pac2_attribs[] =

{
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_GREEN,   Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_GREEN,   Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_BLUE,    Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_BLUE,    Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_CYAN,    Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_CYAN,    Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_RED,     Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_RED,     Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_YELLOW,  Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_YELLOW,  Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_MAGENTA, Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_MAGENTA, Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_ITALICS,           0 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE_ITALICS, 0 },

    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,           0 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,         0 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,           4 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,         4 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,           8 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,         8 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,          12 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,        12 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,          16 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,        16 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,          20 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,        20 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,          24 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,        24 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_REGULAR,          28 },
    { Player::EIA608_COLOR_WHITE,   Player::EIA608_FONT_UNDERLINE,        28 },
};

static constexpr auto BLANK = 0x20; // Space character for blank positions

// Decode UTF-8 sequence to Unicode codepoint
// Returns the codepoint and advances the pointer
inline uint16_t utf8_decode(const char*& _text)
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(_text);
    uint8_t first = *p;
    
    // ASCII (0x00-0x7F)
    if (first < 0x80)
    {
        _text++;
        return first;
    }
    
    // 2-byte sequence (0xC0-0xDF)
    if ((first & 0xE0) == 0xC0 && p[1])
    {
        uint16_t codepoint = ((first & 0x1F) << 6) | (p[1] & 0x3F);
        _text += 2;
        return codepoint;
    }
    
    // 3-byte sequence (0xE0-0xEF)
    if ((first & 0xF0) == 0xE0 && p[1] && p[2])
    {
        uint16_t codepoint = ((first & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        _text += 3;
        return codepoint;
    }
    
    // Invalid or unsupported (4-byte sequences)
    _text++;
    return '?';
}

// Encode Unicode codepoint to UTF-8
// Returns number of bytes written
inline int utf8_encode(uint16_t codepoint, char* output)
{
    if (codepoint < 0x80)
    {
        output[0] = static_cast<char>(codepoint);
        return 1;
    }
    else if (codepoint < 0x800)
    {
        output[0] = static_cast<char>(0xC0 | (codepoint >> 6));
        output[1] = static_cast<char>(0x80 | (codepoint & 0x3F));
        return 2;
    }
    else
    {
        output[0] = static_cast<char>(0xE0 | (codepoint >> 12));
        output[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        output[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
        return 3;
    }
}

inline bool is_valid_text(const char* _text)
{
    while(true)
    {
        auto c = *_text;
        if (c == 0)
        {
            return false;
        }

        // Check if it is a valid printable ASCII char A-Z, 0-9, etc
        if (c >= '!' and c <= '~')
        {
            return true;
        }

        _text++;
    }
}

void Player::publish()
{
    const auto &fn = m_cc_callback;

    if (fn)
    {
        int active_line = 0;
        Event_CC data;
        auto screen = &m_screen[m_screen_idx];
        char utf8_buffer[EIA608_SCREEN_COLUMNS * 3 + 1]; // Max 3 bytes per char + null

        for (int row = 0; row < EIA608_SCREEN_ROWS; row++)
        {
            if (screen->row_used[row])
            {
                // Convert uint16_t codepoints to UTF-8 string
                int utf8_pos = 0;
                int first_non_blank = -1;
                int last_non_blank = -1;
                
                // Find range of non-blank characters
                for (int col = 0; col < EIA608_SCREEN_COLUMNS; col++)
                {
                    uint16_t codepoint = screen->characters[row][col];
                    if (codepoint != 0 && codepoint != BLANK)
                    {
                        if (first_non_blank == -1) first_non_blank = col;
                        last_non_blank = col;
                    }
                }
                
                if (first_non_blank >= 0)
                {
                    // Convert characters to UTF-8
                    for (int col = first_non_blank; col <= last_non_blank; col++)
                    {
                        uint16_t codepoint = screen->characters[row][col];
#ifndef NDEBUG
                        if (codepoint > 0x7F)
                        {
                            DEBUG("[" << col << "]=U+" << std::hex << std::setw(4) << std::setfill('0') << (int)codepoint << std::dec << " ");
                        }
#endif
                        if (codepoint == 0 || codepoint == BLANK)
                        {
                            utf8_buffer[utf8_pos++] = ' ';
                        }
                        else
                        {
                            int bytes = utf8_encode(codepoint, &utf8_buffer[utf8_pos]);
#ifndef NDEBUG
//                             if (codepoint > 0x7F)
//                             {
//                                 std::cout << __FILE__ << ":" << __LINE__ << "-> ";
//                                 for (int b = 0; b < bytes; b++)
//                                 {
//                                     std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)utf8_buffer[utf8_pos + b] << " ";
//                                 }
//                                 std::cout << std::dec << "\n";
//                             }
#endif
                            utf8_pos += bytes;
                        }
                    }
                }
                utf8_buffer[utf8_pos] = '\0';
                
                auto text = utf8_buffer;
                int x = first_non_blank >= 0 ? first_non_blank : 0;

                // Remove trailing spaces
                int len = strlen(text);
                while (len > 0 && text[len - 1] == ' ')
                {
                    len--;
                }
                utf8_buffer[len] = '\0';

                // Publish if we have valid text
                if (len > 0 && is_valid_text(text))
                {
                    data.lines[active_line].x = x;
                    data.lines[active_line].y = row;
                    data.lines[active_line].text = std::string_view(text);

#ifndef NDEBUG
                    // Debug: Log UTF-8 characters being published
                    bool has_multibyte = false;
                    for (int i = 0; text[i]; i++)
                    {
                        if (static_cast<uint8_t>(text[i]) >= 0x80)
                        {
                            has_multibyte = true;
                            break;
                        }
                    }
                    if (has_multibyte)
                    {
                        DEBUG("CC UTF-8 text (row=" << row << ", x=" << x << "): \"" << text << "\"\n");
                    }
#endif

                    if (++active_line >= MBGUI_CC_MAX_LINES)
                    {
                        break;
                    }
                }
                else if (static_cast<uint8_t>(*text) == 0xe2) // HACK
                {
                    data.lines[active_line].text = "♪";
                }                
#ifndef NDEBUG
                else if (len > 0)
                {
                    DEBUG("CC invalid text (row=" << row << "): \"" << text << "\" - hex: ");
                    for(size_t i = 0; i < strlen(text); i++)
                    {
                        DEBUG(" " << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)text[i]);
                    }
                    DEBUG(" " << std::dec << "\n");
                }
#endif
            }
        }

        if (active_line > 0)
        {
            fn(std::move(data));
        }
    }
}

void Player::Eia608Cursor(int dx)
{
    m_cursor.column += dx;

    if (m_cursor.column < 0)
    {
        m_cursor.column = 0;
    }
    else if (m_cursor.column > EIA608_SCREEN_COLUMNS - 1)
    {
        m_cursor.column = EIA608_SCREEN_COLUMNS - 1;
    }
}

int Player::Eia608GetWritingScreenIndex()
{
    switch (m_mode)
    {
        case EIA608_MODE_POPUP:    // Non displayed screen
            return 1 - m_screen_idx;

        case EIA608_MODE_ROLLUP_2: // Displayed screen
        case EIA608_MODE_ROLLUP_3:
        case EIA608_MODE_ROLLUP_4:
        case EIA608_MODE_PAINTON:
            return m_screen_idx;

        default:
            /* It cannot happen, else it is a bug */
            mb_assert(false);
            return 0;
    }
}

void Player::Eia608Write(const uint8_t c)
{
    const int i_row = m_cursor.row;
    const int i_column = m_cursor.column;
    eia608_screen_t *screen;

    if (m_mode == EIA608_MODE_TEXT)
    {
        return;
    }

    screen = &m_screen[Eia608GetWritingScreenIndex()];
    
#ifndef NDEBUG
    uint16_t old_value = screen->characters[i_row][i_column];
    if (old_value > 0x7F && c < 0x80)
    {
        DEBUG("WARNING: Overwriting multibyte char U+" << std::hex << std::setw(4) << std::setfill('0') << (int)old_value
              << " with ASCII 0x" << std::setw(2) << (int)c << std::dec 
              << " at [" << i_row << "][" << i_column << "]\n");
    }
    else if (c > 0x7F)
    {
        DEBUG("Eia608Write(uint8_t): Writing byte 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)c 
              << std::dec << " at [" << i_row << "][" << i_column << "] mode=" << (int)m_mode << "\n");
    }
#endif
    
    screen->characters[i_row][i_column] = static_cast<uint16_t>(c);
    screen->colors[i_row][i_column] = m_color;
    screen->fonts[i_row][i_column] = m_font;
    screen->row_used[i_row] = true;
    Eia608Cursor(1);
}

void Player::Eia608Write(const char *c, bool replace_previous)
{
    /* Extended characters in EIA-608 replace the previous character.
     * For example: 'c' + Extended command = 'ç' (c-cedilla)
     * When replace_previous=true, move cursor back before writing. */

    if (replace_previous)
    {
        Eia608Cursor(-1);
    }

#ifndef NDEBUG
    // Debug: Print received characters
    // std::cout << __FILE__ << ":" << __LINE__ << " bytes: ";
    // for (const char* p = c; *p; p++)
    // {
    //     std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)*p << ' ';
    // }
    // std::cout << std::dec << "\n";
    DEBUG(" at row=" << (int)m_cursor.row 
              << " col=" << (int)m_cursor.column 
              << " mode=" << (int)m_mode 
              << " screen=" << Eia608GetWritingScreenIndex()
              << " replace=" << (replace_previous ? "yes" : "no") << "\n");
#endif

    // Decode UTF-8 and write as Unicode codepoints
    while (*c)
    {
        const char* start = c;
        uint16_t codepoint = utf8_decode(c);
        
#ifndef NDEBUG
        // Debug: Show decoded codepoint
        int bytes_consumed = c - start;
        //DEBUG("Decoded codepoint: U+" << std::hex << std::setw(4) << std::setfill('0') << (int)codepoint
        //          << std::dec << " (" << bytes_consumed << " byte" << (bytes_consumed > 1 ? "s" : "") << ")\n");
#endif
        
        // Write the full Unicode codepoint to the buffer
        const int i_row = m_cursor.row;
        const int i_column = m_cursor.column;
        eia608_screen_t *screen;

        if (m_mode != EIA608_MODE_TEXT && i_column < EIA608_SCREEN_COLUMNS)
        {
            screen = &m_screen[Eia608GetWritingScreenIndex()];
            screen->characters[i_row][i_column] = codepoint;
            screen->colors[i_row][i_column] = m_color;
            screen->fonts[i_row][i_column] = m_font;
            screen->row_used[i_row] = true;
            
#ifndef NDEBUG
            // Debug: Verify what was stored
            uint16_t stored = screen->characters[i_row][i_column];
            if (stored != codepoint)
            {
                DEBUG("WARNING: Codepoint mismatch! Wrote U+" << std::hex << std::setw(4) << std::setfill('0') << (int)codepoint
                      << " but read back U+" << std::setw(4) << (int)stored << std::dec << "\n");
            }
            else if (codepoint > 0x7F)
            {
                //DEBUG("Stored U+" << std::hex << std::setw(4) << std::setfill('0') << (int)codepoint
                //      << std::dec << " at [" << i_row << "][" << i_column << "]\n");
            }
#endif
            
            Eia608Cursor(1);
        }
    }
}

void Player::Eia608Erase()
{
    const int i_row = m_cursor.row;
    const int i_column = m_cursor.column - 1;
    eia608_screen_t *screen;

    if (m_mode == EIA608_MODE_TEXT)
    {
        return;
    }

    if (i_column < 0)
    {
        return;
    }

    screen = &m_screen[Eia608GetWritingScreenIndex()];
    /* FIXME do we need to reset row_used/colors/font ? */
    screen->characters[i_row][i_column] = BLANK;
    Eia608Cursor(-1);
}

void Player::Eia608ClearScreenRowX(int i_screen, int i_row, int x)
{
    auto screen = &m_screen[i_screen];

    if (x == 0)
    {
        screen->row_used[i_row] = false;
    }
    else
    {
        screen->row_used[i_row] = false;

        for (int i = 0; i < x; i++)
        {
            if (screen->characters[i_row][i] != BLANK ||
                    screen->colors[i_row][i] != EIA608_COLOR_DEFAULT ||
                    screen->fonts[i_row][i] != EIA608_FONT_REGULAR)
            {
                screen->row_used[i_row] = true;
                break;
            }
        }
    }

    for (; x < EIA608_SCREEN_COLUMNS + 1; x++)
    {
        screen->characters[i_row][x] = BLANK;
        screen->colors[i_row][x] = EIA608_COLOR_DEFAULT;
        screen->fonts[i_row][x] = EIA608_FONT_REGULAR;
    }
}

void Player::Eia608ClearScreenRow(int i_screen, int i_row)
{
    Eia608ClearScreenRowX(i_screen, i_row, 0);
}

void Player::Eia608ClearScreen(int i_screen)
{
    for (int i = 0; i < EIA608_SCREEN_ROWS; i++)
    {
        Eia608ClearScreenRow(i_screen, i);
    }
}

void Player::Eia608EraseScreen(bool _displayed)
{
    Eia608ClearScreen(_displayed ? m_screen_idx : (1 - m_screen_idx));
}

void Player::Eia608EraseToEndOfRow()
{
    if (m_mode == EIA608_MODE_TEXT)
    {
        return;
    }

    Eia608ClearScreenRowX(Eia608GetWritingScreenIndex(), m_cursor.row, m_cursor.column);
}

void Player::Eia608RollUp()
{
    if (m_mode == EIA608_MODE_TEXT)
    {
        return;
    }

    const int i_screen = Eia608GetWritingScreenIndex();
    auto screen = &m_screen[i_screen];
    int keep_lines;

    /* Window size */
    if (m_mode == EIA608_MODE_ROLLUP_2)
    {
        keep_lines = 2;
    }
    else if (m_mode == EIA608_MODE_ROLLUP_3)
    {
        keep_lines = 3;
    }
    else if (m_mode == EIA608_MODE_ROLLUP_4)
    {
        keep_lines = 4;
    }
    else
    {
        return;
    }

    /* Reset the cursor */
    m_cursor.column = 0;

    /* Erase lines above our window */
    for (int i = 0; i < m_cursor.row - keep_lines; i++)
    {
        Eia608ClearScreenRow(i_screen, i);
    }

    /* Move up */
    for (int i = 0; i < keep_lines - 1; i++)
    {
        const int i_row = m_cursor.row - keep_lines + i + 1;

        if (i_row < 0)
        {
            continue;
        }

        mb_assert(i_row + 1 < EIA608_SCREEN_ROWS);
        memcpy(screen->characters[i_row], screen->characters[i_row + 1], sizeof(*screen->characters));
        memcpy(screen->colors[i_row], screen->colors[i_row + 1], sizeof(*screen->colors));
        memcpy(screen->fonts[i_row], screen->fonts[i_row + 1], sizeof(*screen->fonts));
        screen->row_used[i_row] = screen->row_used[i_row + 1];
    }

    /* Reset current row */
    Eia608ClearScreenRow(i_screen, m_cursor.row);
}

Player::eia608_status_t Player::Eia608ParseSingle(const uint8_t dx)
{
    mb_assert(dx >= 0x20);
    
    // EIA-608 Basic Character Set remapping
    // Only specific positions outside the standard ASCII printable set are remapped
    // Reference: CEA-608-E Section 6.4.1
    const char* replacement = nullptr;
    
    switch (dx)
    {
        case 0x2A: replacement = "á"; break;  // LATIN SMALL LETTER A WITH ACUTE
        case 0x5C: replacement = "é"; break;  // LATIN SMALL LETTER E WITH ACUTE (backslash position)
        case 0x5E: replacement = "í"; break;  // LATIN SMALL LETTER I WITH ACUTE (caret position)
        case 0x5F: replacement = "ó"; break;  // LATIN SMALL LETTER O WITH ACUTE (underscore position)
        case 0x60: replacement = "ú"; break;  // LATIN SMALL LETTER U WITH ACUTE (grave accent position)
        case 0x7B: replacement = "ç"; break;  // LATIN SMALL LETTER C WITH CEDILLA (left brace position)
        case 0x7C: replacement = "÷"; break;  // DIVISION SIGN (pipe position)
        case 0x7D: replacement = "Ñ"; break;  // LATIN CAPITAL LETTER N WITH TILDE (right brace position)
        case 0x7E: replacement = "ñ"; break;  // LATIN SMALL LETTER N WITH TILDE (tilde position)
        case 0x7F: replacement = "█"; break;  // SOLID BLOCK (delete position)
        default:
            // No replacement needed - use the character as-is
            Eia608Write(dx);
            return EIA608_STATUS_CHANGED;
    }
    
    // Write the replacement character
    Eia608Write(replacement);
    return EIA608_STATUS_CHANGED;
}

Player::eia608_status_t Player::Eia608ParseTextAttribute(uint8_t d2)
{
    const int i_index = d2 - 0x20;
    mb_assert(d2 >= 0x20 && d2 <= 0x2f);
    m_color = pac2_attribs[i_index].i_color;
    m_font  = pac2_attribs[i_index].i_font;
    Eia608Cursor(1);
    return EIA608_STATUS_DEFAULT;
}

Player::eia608_status_t Player::Eia608ParseExtended(uint8_t d1, uint8_t d2)
{
    mb_assert(d2 >= 0x20 && d2 <= 0x3f);
    mb_assert(d1 == 0x11 || d1 == 0x12 || d1 == 0x13);

    uint16_t command = (d1 << 8) | d2;

    switch (command)
    {
        case 0x1130: Eia608Write("®", false); break;
        case 0x1131: Eia608Write("°", false); break;
        case 0x1132: Eia608Write("½", false); break;
        case 0x1133: Eia608Write("¿", false); break;
        case 0x1134: Eia608Write("™", false); break;
        case 0x1135: Eia608Write("¢", false); break;
        case 0x1136: Eia608Write("£", false); break;
        case 0x1137: Eia608Write("♪", false); break;
        case 0x1138: Eia608Write("à", false); break;
        case 0x1139: Eia608Write(" ", false); break;
        case 0x113a: Eia608Write("è", false); break; 
        case 0x113b: Eia608Write("â", false); break;
        case 0x113c: Eia608Write("ê", false); break;
        case 0x113d: Eia608Write("î", false); break;
        case 0x113e: Eia608Write("ô", false); break;
        case 0x113f: Eia608Write("û", false); break;
        case 0x1220: Eia608Write("Á", true); break;
        case 0x1221: Eia608Write("É", true); break;
        case 0x1222: Eia608Write("Ó", true); break;
        case 0x1223: Eia608Write("Ú", true); break;
        case 0x1224: Eia608Write("Ü", true); break;
        case 0x1225: Eia608Write("ü", true); break;
        case 0x1226: Eia608Write("ú", true); break;
        case 0x1227: Eia608Write("¡", true); break;
        case 0x1228: Eia608Write("©", true); break;
        case 0x1229: Eia608Write("ç", true); break;  // c-cedilla lowercase (added - was missing)
        case 0x122a: Eia608Write("℠", true); break;
        case 0x122b: Eia608Write("©", true); break;
        case 0x1230: Eia608Write("À", true); break;
        case 0x1231: Eia608Write("Â", true); break;
        case 0x1232: Eia608Write("Ç", true); break;
        case 0x1233: Eia608Write("È", true); break;
        case 0x1234: Eia608Write("Ê", true); break;
        case 0x1235: Eia608Write("Ë", true); break;
        case 0x1236: Eia608Write("ë", true); break;
        case 0x1237: Eia608Write("Î", true); break;
        case 0x1238: Eia608Write("Ï", true); break;
        case 0x1239: Eia608Write("ï", true); break;
        case 0x123a: Eia608Write("Ô", true); break;
        case 0x123b: Eia608Write("Ù", true); break;
        case 0x123c: Eia608Write("ù", true); break; 
        case 0x123d: Eia608Write("Û", true); break;
        case 0x1320: Eia608Write("Ã", true); break;
        case 0x1321: Eia608Write("ã", true); break;
        case 0x1322: Eia608Write("Í", true); break;
        case 0x1323: Eia608Write("Ì", true); break;
        case 0x1325: Eia608Write("Ò", true); break;
        case 0x1326: Eia608Write("ò", true); break;
        case 0x1327: Eia608Write("Õ", true); break;
        case 0x1328: Eia608Write("õ", true); break;
        case 0x132b: Eia608Write("\\", true); break;
        case 0x132c: Eia608Write("^", true); break;
        case 0x132d: Eia608Write("_", true); break;
        case 0x132e: Eia608Write("¦", true); break;
        case 0x132f: Eia608Write("~", true); break;
        case 0x1330: Eia608Write("Ä", true); break;
        case 0x1331: Eia608Write("ä", true); break;
        case 0x1332: Eia608Write("Ö", true); break;
        case 0x1333: Eia608Write("ö", true); break;
        case 0x1334: Eia608Write("ß", true); break;
        case 0x1335: Eia608Write("¥", true); break;
        case 0x1336: Eia608Write("¤", true); break;
        case 0x1337: Eia608Write("|", true); break;
        case 0x1338: Eia608Write("Å", true); break;
        case 0x1339: Eia608Write("å", true); break;
        default:
#ifndef NDEBUG
            DEBUG("EIA608 Extended char not mapped: 0x" << std::hex << std::setw(4) << std::setfill('0') << (int)command 
                  << " (d1=0x" << std::setw(2) << (int)d1 << " d2=0x" << std::setw(2) << (int)d2 << ")" << std::dec << "\n");
#endif
            DEBUG_MSG(HAL, WARN, "Char not found: " << hex << setw(2) << setfill('0') << (int)d1 << " " << setw(2) << (int)d2 << "\n");
            return EIA608_STATUS_DEFAULT;
    }

    return EIA608_STATUS_CHANGED;
}

Player::eia608_status_t Player::Eia608ParseCommand0x14(uint8_t d2)
{
    eia608_status_t i_status = EIA608_STATUS_DEFAULT;
    CC_Mode proposed_mode;

    switch (d2)
    {
        case 0x20:  /* Resume caption loading */
            m_mode = EIA608_MODE_POPUP;
            // Clear the non-displayed buffer to avoid showing old characters
            Eia608EraseScreen(false);
            break;

        case 0x21:  /* Backspace */
            Eia608Erase();
            i_status = EIA608_STATUS_CHANGED;
            break;

        case 0x22:  /* Reserved */
        case 0x23:
            break;

        case 0x24:  /* Delete to end of row */
            Eia608EraseToEndOfRow();
            break;

        case 0x25:  /* Rollup 2 */
        case 0x26:  /* Rollup 3 */
        case 0x27:  /* Rollup 4 */
            if (m_mode == EIA608_MODE_POPUP || m_mode == EIA608_MODE_PAINTON)
            {
                Eia608EraseScreen(true);
                Eia608EraseScreen(false);
                i_status = static_cast<eia608_status_t>(EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_CLEARED);
            }

            if (d2 == 0x25)
            {
                proposed_mode = EIA608_MODE_ROLLUP_2;
            }
            else if (d2 == 0x26)
            {
                proposed_mode = EIA608_MODE_ROLLUP_3;
            }
            else
            {
                proposed_mode = EIA608_MODE_ROLLUP_4;
            }

            if (proposed_mode != m_mode)
            {
                m_mode = proposed_mode;
                m_cursor.column = 0;
                m_cursor.row = m_row_rollup;
            }

            break;

        case 0x28:  /* Flash on */
            /* TODO */
            break;

        case 0x29:  /* Resume direct captionning */
            m_mode = EIA608_MODE_PAINTON;
            break;

        case 0x2a:  /* Text restart */
            /* TODO */
            break;

        case 0x2b: /* Resume text display */
            m_mode = EIA608_MODE_TEXT;
            break;

        case 0x2c: /* Erase displayed memory */
            Eia608EraseScreen(true);
            i_status = static_cast<eia608_status_t>(EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_CLEARED);
            break;

        case 0x2d: /* Carriage return */
            Eia608RollUp();
            i_status = EIA608_STATUS_CHANGED;
            break;

        case 0x2e: /* Erase non displayed memory */
            Eia608EraseScreen(false);
            break;

        case 0x2f: /* End of caption (flip screen if not paint on) */
            if (m_mode != EIA608_MODE_PAINTON)
            {
                m_screen_idx = 1 - m_screen_idx;
            }

            m_mode = EIA608_MODE_POPUP;
            m_cursor.column = 0;
            m_cursor.row = 0;
            m_color = EIA608_COLOR_DEFAULT;
            m_font = EIA608_FONT_REGULAR;
            i_status = static_cast<eia608_status_t>(EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_ENDED);
            break;
    }

    return i_status;
}

bool Player::Eia608ParseCommand0x17(uint8_t d2)
{
    switch (d2)
    {
        case 0x21:  /* Tab offset 1 */
        case 0x22:  /* Tab offset 2 */
        case 0x23:  /* Tab offset 3 */
            Eia608Cursor(d2 - 0x20);
            break;
    }

    return false;
}

bool Player::Eia608ParsePac(uint8_t d1, uint8_t d2)
{
    static const int pi_row[] =
    {
        11, -1, 1, 2, 3, 4, 12, 13, 14, 15, 5, 6, 7, 8, 9, 10
    };
    const int i_row_index = ((d1 << 1) & 0x0e) | ((d2 >> 5) & 0x01);
    mb_assert(d2 >= 0x40 && d2 <= 0x7f);

    if (pi_row[i_row_index] <= 0)
    {
        return false;
    }

    /* Row */
    if (m_mode != EIA608_MODE_TEXT)
    {
        m_cursor.row = pi_row[i_row_index] - 1;
    }

    m_row_rollup = pi_row[i_row_index] - 1;

    /* Column */
    if (d2 >= 0x60)
    {
        d2 -= 0x60;
    }
    else if (d2 >= 0x40)
    {
        d2 -= 0x40;
    }

    m_cursor.column = pac2_attribs[d2].i_column;
    m_color = pac2_attribs[d2].i_color;
    m_font  = pac2_attribs[d2].i_font;
    return false;
}

Player::eia608_status_t Player::Eia608ParseData(uint8_t d1, uint8_t d2)
{
    eia608_status_t i_status = EIA608_STATUS_DEFAULT;

#ifndef NDEBUG
    // Debug: Log all EIA-608 commands for troubleshooting
    // if (d1 >= 0x20 || (d1 >= 0x11 && d1 <= 0x17))
    // {
    //     std::cout << __FILE__ << ":" << __LINE__ << " EIA608ParseData: d1=0x" << std::hex << std::setw(2) << std::setfill('0') << (int)d1 
    //           << " d2=0x" << std::setw(2) << (int)d2 << std::dec
    //           << " row=" << (int)m_cursor.row << " col=" << (int)m_cursor.column;
    //     if (d1 >= 0x20 && d2 >= 0x20)
    //     {
    //         std::cout << " [chars: '" << (char)d1 << "' '" << (char)d2 << "']";
    //     }
    //     std::cout << "\n";
    // }
#endif

    if (d1 >= 0x18 && d1 <= 0x1f)
    {
        d1 -= 8;
    }

#define ON( d2min, d2max, cmd ) do { if( d2 >= d2min && d2 <= d2max ) i_status = cmd; } while(0)

    switch (d1)
    {
        case 0x11:
            ON(0x20, 0x2f, Eia608ParseTextAttribute(d2));
            ON(0x30, 0x3f, Eia608ParseExtended(d1, d2));
            break;

        case 0x12:
        case 0x13:
            ON(0x20, 0x3f, Eia608ParseExtended(d1, d2));
            break;

        case 0x14: case 0x15:
            ON(0x20, 0x2f, Eia608ParseCommand0x14(d2));
            break;

        case 0x17:
            ON(0x21, 0x23, (eia608_status_t)Eia608ParseCommand0x17(d2));
            ON(0x2e, 0x2f, Eia608ParseTextAttribute(d2));
            break;
    }

    if (d1 == 0x10)
    {
        ON(0x40, 0x5f, (eia608_status_t)Eia608ParsePac(d1, d2));
    }
    else if (d1 >= 0x11 && d1 <= 0x17)
    {
        ON(0x40, 0x7f, (eia608_status_t)Eia608ParsePac(d1, d2));
    }

#undef ON

    if (d1 >= 0x20)
    {
        i_status = Eia608ParseSingle(d1);

        if (d2 >= 0x20)
        {
            i_status = static_cast<eia608_status_t>(i_status | Eia608ParseSingle(d2));
        }
    }

    /* Ignore changes occurring to doublebuffer */
    if (m_mode == EIA608_MODE_POPUP && i_status == EIA608_STATUS_CHANGED)
    {
        i_status = EIA608_STATUS_DEFAULT;
    }

    return i_status;
}

static unsigned Eia608ParseChannel(const uint8_t d[2])
{
    /* Check odd parity */
    static const int p4[16] =
    {
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
    };

    if (p4[d[0] & 0xf] == p4[d[0] >> 4] ||
            p4[d[1] & 0xf] == p4[ d[1] >> 4])
    {
        return -1;
    }

    const int d1 = d[0] & 0x7f;

    if (d1 >= 0x10 && d1 <= 0x1f)
    {
        return 1 + ((d1 & 0x08) != 0);
    }
    else if (d1 < 0x10)
    {
        return 3;
    }

    return 0;
}

Player::eia608_status_t Player::Eia608Parse(const uint8_t data[2])
{
    const uint8_t d1 = data[0] & 0x7f; /* Removed parity bit */
    const uint8_t d2 = data[1] & 0x7f;
    eia608_status_t i_screen_status = EIA608_STATUS_DEFAULT;

    if (d1 == 0 && d2 == 0)
    {
        return EIA608_STATUS_DEFAULT;    /* Ignore padding (parity check are sometimes invalid on them) */
    }

    // Ignore channel
    // auto i_channel = Eia608ParseChannel( data );
    // if( i_channel > 1 )
    //     return EIA608_STATUS_DEFAULT;

    if (d1 >= 0x10)
    {
        if (d1 >= 0x20 ||
                d1 != m_last_d1 || d2 != m_last_d2)  /* Command codes can be repeated */
        {
            i_screen_status = Eia608ParseData(d1, d2);
        }

        m_last_d1 = d1;
        m_last_d2 = d2;
    }
    else if ((d1 >= 0x01 && d1 <= 0x0E) || d1 == 0x0F)
    {
        /* XDS block / End of XDS block */
    }

    return i_screen_status;
}

void Player::parse_cc(unsigned char *_data, size_t _length)
{
    unsigned i_field = 0;
    auto offset = 7;

    if(_data[0] == 'G' and _data[1] == 'A' and _data[2] == '9' and _data[3] == '4')
    {
       offset = 7;
    }
    else
    {
         offset = 4;
    }

    _data += offset;
    _length -= offset;
    while (_length >= 3)
    {
        //if (_data[0] == 0xfc and _data[1] == 0x94 and _data[2] == 0x2F)
        //{
        //    DEBUG("Data: " << hex << setw(2) << setfill('0') << (int)_data[0] << " " << setw(2) << (int)_data[1] << " " << setw(2)<<            (int)_data[2] << "\n");
        //    DEBUG("Publish\n");
        //}
        if ((_data[0] & 0x04) /* Valid bit */)
        {
            /* Mask off the specific i_field bit, else some sequences can be lost. */
            if (m_standard  == EIA608 and
                    (_data[0] & 0x03) == i_field)
            {
                eia608_status_t i_status = Eia608Parse(&_data[1]);

                /* a caption is ready or removed, process its screen */
                /*
                 * In case of rollup/painton with 1 packet/frame, we need
                 * to update on Changed status.
                 * Batch decoding might be incorrect if those in
                 * large number of commands (mp4, ...) then.
                 * see CEAv1.2zero.trp tests */
                if (i_status & (EIA608_STATUS_DISPLAY | EIA608_STATUS_CHANGED))
                {
                    publish();
                }
            }

            else if( m_standard == CEA708 )//and
            //            (_data[0] & 0x03) >= 2 )
            {
                DEBUG("\nCEA708 STANDARD\n");
                //CEA708_DTVCC_Demuxer_Push( p_sys->p_dtvcc, i_spupts, _data );
            }
        }

        _length -= 3;
        _data += 3;
    }
}

void Player::set_cc_callback(CC_Callback _callback)
{
    auto enabled = _callback.operator bool();
    m_cc_callback = std::move(_callback);
    m_cc_enabled = enabled;
}

}
