// Description: Tela de edição de satélite
#include "mb_osd_keyboard.h"
#include "mb_osd.h"
#include "mb_osd_fade_canvas.h"
#include "mb_osd_fonts.h"

#include <lvgl.h>

#include "mb_menu_resources.h"
#include "common/mb_globals.h"
#include "mb_events.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <string>
#include <src/misc/lv_color.h>
#include <src/misc/lv_types.h>
#include <src/misc/lv_color.h>
#include <time.h>
#include <tasks/mb_task.h>

#include <string>
#include <codecvt>
#include <locale>
#include <iterator>
namespace mb {

// Número de espaços ocupado pela tecla
const int OSD_Keyboard::s_kb_ctrl[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 3, 1,
    2, 3, 2, 2, 1,
    2, 6, 2, 1
};

// populate kb_nav_map
const std::pair<int, int> OSD_Keyboard::s_kb_nav_map[] =
{
    //  0/q       1/w       2/e       3/r       4/t       5/y       6/u       7/i       8/o       9/p       10 \n
    {11, 1},  {12, 2},  {13, 3},  {14, 4},  {15, 5},  {16, 6},  {17, 7},  {18, 8},  {19, 9},  {20, 0},  {0, 0},

    //  11/a      12/s      13/d       14/f      15/g      16/h      17/j     18/k      19/l      20/ç      21 \n
    {22, 12}, {23, 13}, {24, 14}, {25, 15}, {26, 16}, {27, 17}, {28, 18}, {29, 19}, {29, 20}, {29, 11}, {0, 0},

    //  22/z      23/x      24/c      25/v      26/b      27/n      28/m      29/Apagar                     30 \n
    {31, 23}, {31, 24}, {32, 25}, {32, 26}, {32, 27}, {33, 28}, {33, 29}, {34, 22},                     {0, 0},

    //  31 A-a              32 Espaço                     33 123              34, Áéô                       35 \n
    {37, 32},           {37, 33},                     {37, 34},           {37, 31},                     {0, 0},


    {-1, -1},           {0, 37},                      {-1, -1},                                        {0, 0 }
    //                      36 Salvar
};

OSD_Keyboard::OSD_Keyboard(OSD *_parent):
    OSD(_parent),
    m_kb_map_upper(
{
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",   "\n",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "Ç",   "\n",
    "Z", "X", "C", "V", "B", "N", "M", tr(__Apagar),    "\n",
    "A-a",    tr(__Espaco),  "123",    "Áéô",           "\n",
    " ", tr(__Salvar), " ",
}),
m_kb_map_lower(
{
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",   "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "ç",   "\n",
    "z", "x", "c", "v", "b", "n", "m", tr(__Apagar),    "\n",
    "A-a",    tr(__Espaco),  "123",    "Áéô",           "\n",
    " ", tr(__Salvar), " ",
}),
m_kb_map_num(
{
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",   "\n",
    ".", ",", ":", ";", "(", ")", "&", "@", "#", "%",   "\n",
    "_", "*", "\"", "/", "+", "!", "?", tr(__Apagar),   "\n",
    "A-a",    tr(__Espaco),  "123",    "Áéô",           "\n",
    " ", tr(__Salvar), " ",
}),
m_kb_map_special(
{
    "á", "â", "ã", "é", "ê", "í", "ó", "ô", "õ", "ú",   "\n",
    "Á", "Â", "Ã", "É", "Ê", "Í", "Ó", "Ô", "Õ", "Ú",   "\n",
    "-", "^", "~", "|", "$", "=", "'", tr(__Apagar),    "\n",
    "A-a",    tr(__Espaco),  "123",    "Áéô",           "\n",
    " ", tr(__Salvar), " ",
})
{
    for (auto &obj : m_key_box)
    {
        obj = nullptr;
    }

    for (auto &obj : m_key_label)
    {
        obj = nullptr;
    }
}

OSD_Keyboard::~OSD_Keyboard()
{
    DELETE_TIMER(m_timer);
    DELETE_OBJ(m_mainscreen);
    remove_focus();
}

// Processa tecla recebida
bool OSD_Keyboard::handle_event_remote_control(const Event_Remote_Control &_event)
{
    if (m_active_key < 0 || m_active_key >= s_keyboard_size)
    {
        m_active_key = 0;
    }

    switch (_event.key)
    {
        case Remote_Control_Key::KEY_0:
        case Remote_Control_Key::KEY_1:
        case Remote_Control_Key::KEY_2:
        case Remote_Control_Key::KEY_3:
        case Remote_Control_Key::KEY_4:
        case Remote_Control_Key::KEY_5:
        case Remote_Control_Key::KEY_6:
        case Remote_Control_Key::KEY_7:
        case Remote_Control_Key::KEY_8:
        case Remote_Control_Key::KEY_9:
        {
            auto key_char = to_char(_event.key);
            char key[2] = { static_cast<char>(key_char), '\0' };
            lv_textarea_add_text(m_textarea, key);
            break;
        }

        case Remote_Control_Key::KEY_PLUS:
        {
            lv_textarea_add_text(m_textarea, " ");
        }
        break;

        case Remote_Control_Key::KEY_CC:
        {
            lv_textarea_delete_char(m_textarea);
        }
        break;

        case Remote_Control_Key::KEY_LR:
        {
            m_active_kbd = static_cast<Active_Kbd>((static_cast<int>(m_active_kbd) + 1) % static_cast<int>(Active_Kbd::COUNT));
            create_keyboard();
        }
        break;

        case Remote_Control_Key::KEY_OK:
        {
            process_ok();
        }
        break;

        case Remote_Control_Key::KEY_VOLTAR:
        {
            Task::post_event(std::bind(m_callback, m_satellite));
        }
        break;

        case Remote_Control_Key::KEY_CHDOWN:
        {
            unselect();

            int next = s_kb_nav_map[m_active_key].first;
            if (next >= 0 && next < s_keyboard_size)
            {
                m_active_key = next;
            }
            select();
        }
        break;

        case Remote_Control_Key::KEY_CHUP:
        {
            unselect();

            for (auto i = 0 ; i < static_cast<int>(std::size(s_kb_nav_map)) ; i++)
            {
                if (!s_kb_nav_map[i].first && !s_kb_nav_map[i].second)
                {
                    continue;
                }

                if (s_kb_nav_map[i].first == m_active_key)
                {
                    m_active_key = i;
                    break;
                }
            }

            select();
        }
        break;

        case Remote_Control_Key::KEY_VOLUP:
        {
            unselect();
            int next = s_kb_nav_map[m_active_key].second;
            if (next >= 0 && next < s_keyboard_size)
            {
                m_active_key = next;
            }
            select();
        }
        break;

        case Remote_Control_Key::KEY_VOLDOWN:
        {
            unselect();

            for (auto i = 0 ; i < static_cast<int>(std::size(s_kb_nav_map)) ; i++)
            {
                if (!s_kb_nav_map[i].first && !s_kb_nav_map[i].second)
                {
                    continue;
                }

                if (s_kb_nav_map[i].second == m_active_key)
                {
                    m_active_key = i;
                    break;
                }
            }

            select();
        }
        break;

        default:
            break;
    }

    return true;
}

// Mostra a tela de seleção de satélite
void OSD_Keyboard::osd_keyboard(Osd_Keyboard_cb_t _callback, lv_area_t _area, Satellite _sat)
{
    m_callback = _callback;
    set_focus();
    m_satellite = std::move(_sat);
    // Cria a tela principal
    m_mainscreen = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), _area.x1, _area.y1, _area.x2, _area.y2, OSD_COLOR_BLACK);
    lv_obj_null_on_delete(&m_mainscreen);
    lv_obj_set_style_bg_opa(m_mainscreen, LV_OPA_90, 0);
    // Desenha linha
    m_line = create_rect(m_mainscreen, 0, 0, 3, _area.y2, OSD_COLOR_ORANGE);
    lv_obj_null_on_delete(&m_line);
    // Cria teclado
    create_keyboard();
    // Cria área de texto
    create_textarea();
    // Preenche área de edição
    lv_textarea_add_text(m_textarea, m_satellite.name.c_str());
}

void OSD_Keyboard::timer_cb(lv_timer_t *timer)
{
    OSD_Keyboard *thiz = static_cast<OSD_Keyboard *>(lv_timer_get_user_data(timer));
    if (!thiz)
    {
        return;
    }
    thiz->process_timer();
}

void OSD_Keyboard::process_timer()
{
    if(m_textarea)
    {
        auto text = lv_textarea_get_text(m_textarea);
        lv_textarea_set_text(m_textarea, text);
    }
    m_next_char = true;
    DELETE_TIMER(m_timer);
    m_last = "";
}

void OSD_Keyboard::process(std::string_view _c)
{
    std::string result { _c };
    std::array<std::u32string, 26> char_array =
    {
        U"aAãÃáÁàÀ",
        U"bB",
        U"cçCÇ",
        U"dD",
        U"eEéÉêÊ",
        U"fF",
        U"gG",
        U"hH",
        U"iIíÍ",
        U"jJ",
        U"kK",
        U"lL",
        U"mM",
        U"nN",
        U"oOóÓõÕôÔ",
        U"pP",
        U"qQ",
        U"rR",
        U"sS",
        U"tT",
        U"uUúÚ",
        U"vV",
        U"wW",
        U"xX",
        U"yY",
        U"zZ"
    };

    int pointer = 0;
    //static std::string last = "";

    if (_c != m_last)
    {
        pointer = 0;
        m_next_char = true;
    }
    else
    {
        m_next_char = false;
    }

    m_last = _c;

    // Verifica se timer do lvgl foi iniciado, se não foi, iniciar
    if (!m_timer)
    {
        pointer = 0;
        m_timer = lv_timer_create(timer_cb, s_timer_period, this);
    }
    else
    {
        lv_timer_reset(m_timer);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string s = converter.from_bytes(_c.data());

    for (const std::u32string &text : char_array)
    {
        auto pos = text.find_first_of(s);

        if (pos != std::string::npos)
        {
            auto p = (pointer + pos) % text.size();
            std::u32string secondPartUTF32 = text.substr(p, 1);
            result = converter.to_bytes(secondPartUTF32);
            pointer = (pointer + 1) % text.size();
            break;
        }
    }

    // Busca conteúdo da caixa de texto
    if (!m_textarea)
    {
        return;
    }
    std::string str = lv_textarea_get_text(m_textarea);

    if (!m_next_char && !str.empty())
    {
        str.pop_back();
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string utf32_str = converter.from_bytes(str);
        if (!utf32_str.empty())
        {
            utf32_str.pop_back();
        }
        str = converter.to_bytes(utf32_str);
    }

    str += result;
    lv_textarea_set_text(m_textarea, str.data());
    auto pos = lv_textarea_get_cursor_pos(m_textarea);
    if (pos > 0)
    {
        lv_textarea_set_cursor_pos(m_textarea, pos - 1);
    }
}

void OSD_Keyboard::process_ok()
{
    auto kbd = get_active_kbd();
    if (!kbd || m_active_key >= kbd->size())
    {
        return;
    }
    auto c = (*kbd)[m_active_key];

    if (c == tr(__Salvar))
    {
        m_satellite.name = lv_textarea_get_text(m_textarea);
        Task::post_event(std::bind(m_callback, m_satellite));
    }
    else if (c == tr(__Apagar))
    {
        std::string text = lv_textarea_get_text(m_textarea);
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::u32string utf32_text = converter.from_bytes(text);

        // Remove the last code point
        if (!utf32_text.empty())
        {
            utf32_text.resize(utf32_text.size() - 1);
        }

        // Convert back to UTF-8
        text = converter.to_bytes(utf32_text);
        lv_textarea_set_text(m_textarea, text.c_str());
    }
    else if (c == tr(__Espaco))
    {
        lv_textarea_add_text(m_textarea, " ");
    }
    else if (c == "A-a")
    {
        m_active_kbd = m_active_kbd == Active_Kbd::Lower ? Active_Kbd::Upper : Active_Kbd::Lower;
        create_keyboard();
    }
    else if (c == "123")
    {
        m_active_kbd = m_active_kbd != Active_Kbd::Num ? Active_Kbd::Num : Active_Kbd::Lower;
        create_keyboard();
    }
    else if (c == "Áéô")
    {
        m_active_kbd = m_active_kbd != Active_Kbd::Special ? Active_Kbd::Special : Active_Kbd::Lower;
        create_keyboard();
    }
    else
    {
        process(c);
    }
}

const OSD_Keyboard::Keyboard_t *OSD_Keyboard::get_active_kbd()
{
    switch (m_active_kbd)
    {
        case Active_Kbd::Upper:
            return &m_kb_map_upper;

        case Active_Kbd::Lower:
            return &m_kb_map_lower;

        case Active_Kbd::Special:
            return &m_kb_map_special;

        default:
            return &m_kb_map_num;
    }

    return nullptr;
}

void OSD_Keyboard::create_textarea()
{
    /*Create a text area. The keyboard will write here*/
    m_textarea = lv_textarea_create(m_mainscreen);
    lv_obj_null_on_delete(&m_textarea);
    lv_textarea_set_one_line(m_textarea, true);
    lv_obj_align(m_textarea, LV_ALIGN_TOP_LEFT, 30, 0);
    lv_obj_set_size(m_textarea, lv_pct(80), 50);
    lv_obj_add_state(m_textarea, LV_STATE_FOCUSED);
    lv_obj_set_style_radius(m_textarea, 25, DEFAULT_SELECTOR);
    lv_obj_set_style_border_color(m_textarea, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
    lv_obj_set_style_border_width(m_textarea, 2, DEFAULT_SELECTOR);
}

void OSD_Keyboard::create_keyboard()
{
    // Limpa teclado anterior
    for (auto &obj : m_key_box)
    {
        DELETE_OBJ(obj);
    }

    for (auto &obj : m_key_label)
    {
        DELETE_OBJ(obj);
    }

    // carrega teclado ativo
    const auto &kbd = *get_active_kbd();
    unsigned int i = 0;

    for (int line = 0 ; ; line++)
    {
        for (int column = 0 ; ;)
        {
            if (i >= std::size(s_kb_ctrl))
                break;

            if (i >= kbd.size())
                break;

            if (kbd[i].empty())
                break;

            if (!strcmp(kbd[i].data(), "\n"))
            {
                break;
            }

            int w = s_key_width * s_kb_ctrl[i] + (s_kb_ctrl[i] - 1) * 10;
            // Valores obtidos na tela de seleção de satélite
            auto x = 20 + column * (s_key_width + 10);
            auto y = 80 + line * (s_key_heigth + 10);

            if (kbd[i] == tr(__Salvar))
            {
                y += 40;
                m_key_box[i] = create_rect(m_mainscreen, x, y, w, s_key_heigth, OSD_COLOR_GREY_MEDIUM);
                lv_obj_set_style_radius(m_key_box[i], s_key_heigth / 2, DEFAULT_SELECTOR);
            }
            else
            {
                m_key_box[i] = create_rect(m_mainscreen, x, y, w, s_key_heigth, OSD_COLOR_BLACK);
                lv_obj_set_style_radius(m_key_box[i], s_key_heigth / 5, DEFAULT_SELECTOR);
            }

            m_key_label[i] = set_label_text(m_key_box[i], kbd[i], 0, 0, font_20, OSD_COLOR_WHITE);
            lv_obj_align(m_key_label[i], LV_ALIGN_CENTER, 0, 0);
            column += s_kb_ctrl[i];
            ++i;
        }

        if (i >= kbd.size())
            break;

        if (kbd[i].empty())
        {
            break;
        }

        i++;
    }

    if (m_active_key >= i)
    {
        m_active_key = 0;
    }
    select();
}

void OSD_Keyboard::select()
{

    if (m_active_key < 0 || m_active_key >= s_keyboard_size)
    {
        return;
    }

    if (!m_key_box[m_active_key])
    {
        return;
    }

    auto obj = m_key_box[m_active_key];
    lv_obj_set_style_bg_color(obj, OSD_COLOR_ORANGE, DEFAULT_SELECTOR);
}

void OSD_Keyboard::unselect()
{

    if (m_active_key < 0 || m_active_key >= s_keyboard_size)
    {
        return;
    }

    auto obj = m_key_box[m_active_key];
    if (!obj)
    {
        return;
    }
    const auto *kbd_ptr = get_active_kbd();
    if (!kbd_ptr || m_active_key >= kbd_ptr->size())
    {
        return;
    }

    const auto &kbd = *kbd_ptr;

    if (kbd[m_active_key] == tr(__Salvar))
    {
        lv_obj_set_style_bg_color(obj, OSD_COLOR_GREY_MEDIUM, DEFAULT_SELECTOR);
    }
    else
    {
        lv_obj_set_style_bg_color(obj, OSD_COLOR_BLACK, DEFAULT_SELECTOR);
    }
}

} // namespace mb
