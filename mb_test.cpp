#include <cassert>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

#include <lvgl/lvgl.h>

using namespace std::chrono_literals;
using std::chrono::steady_clock;


#define MB_ZERO(var) memset(&var, 0, sizeof(var))

#ifndef NDEBUG

#define DEBUG_MSG(MSG) do { using namespace std; cerr << MSG; } while(false)

#define PRINT_DEBUG_MSG(MSG) do { using namespace std; std::stringstream __ss; __ss << MSG; Application::print_message(__ss.str()); } while(false)

#ifdef PRINT_TABLES
#define DEBUG_MSG_TABLES(MSG) DEBUG_MSG(MSG)
#else
#define DEBUG_MSG_TABLES(MSG) do { } while(false)
#endif

#else

#define DEBUG_MSG(MSG) do { } while(false)
#define DEBUG_MSG_TABLES(MSG) do { } while(false)

#endif


constexpr auto DEFAULT_SELECTOR = static_cast<uint32_t>(LV_PART_MAIN) | static_cast<uint32_t>(LV_STATE_DEFAULT);
const auto DISPLAY_WIDTH = 1280;
const auto DISPLAY_HEIGHT = 720;

#define OSD_COLOR_WHITE                 lv_color_hex(0xF1F1F1)
#define OSD_COLOR_BLACK                 lv_color_hex(0x000000)
#define OSD_COLOR_BLACK_LIGHT           lv_color_hex(0x373435)
#define OSD_COLOR_GREY                  lv_color_hex(0x737373)
#define OSD_COLOR_GREY_MEDIUM           lv_color_hex(0x484848)
#define OSD_COLOR_GREY_DARK             lv_color_hex(0x232323)
#define OSD_COLOR_BLUE                  lv_color_hex(0x098DE9)
#define OSD_COLOR_RED                   lv_color_hex(0xFF0000)
#define OSD_COLOR_GREEN                 lv_color_hex(0x008000)
#define OSD_COLOR_ORANGE                lv_color_hex(0xFF6D1E)
#define OSD_COLOR_ORANGE_DARK           lv_color_hex(0xFF4000)
#define OSD_COLOR_YELLOW                lv_color_hex(0xFFFF00)

#define obj_del(obj) { if ( obj ) { lv_obj_del(obj); obj = nullptr; } }

lv_timer_t  *m_timer        = { nullptr };
lv_obj_t    *slider         = { nullptr };
lv_obj_t    *slider_label   = { nullptr };

lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color);
static void process_timer_cb(lv_timer_t *tm);

void ask4Update(bool start);
void updatingApp(bool start);
void updateFinished(bool start);

void process_timer_cb(lv_timer_t *tm)
{
    static int step  = 0;
    static int progress  = 0;
    std::cout << "process_timer_cb, step = " << step << "\n";

    if(step == 4)
    {
        ask4Update(false);
        updatingApp(true);
        progress  = 0;
    }
    else if(step > 4 && step <= 14)
    {
        progress += 10;
        lv_label_set_text_fmt(slider_label, "%d%%", progress);
        lv_slider_set_value(slider, progress, LV_ANIM_ON);
    }
    else if(step == 15)
    {
        updatingApp(false);
        updateFinished(true);
    }
    // Loop infinito de redesign
    else if(step == 20)
    {
        updateFinished(false);
        ask4Update(true);
        step = 0;
    }

    // Finaliza callback
    ++step;
}

void ask4Update(bool start)
{
    static lv_obj_t *mainBox  = { nullptr };
    static lv_obj_t *label      = { nullptr };
    lv_obj_t        *btn        = { nullptr };
    lv_obj_t        *label_btn  = { nullptr };
    auto width = DISPLAY_WIDTH / 3;
    auto heigth = DISPLAY_HEIGHT / 4;

    if(start)
    {
        // Cria área da tela
        mainBox = create_rect(lv_screen_active(), 0, 0, width, heigth, OSD_COLOR_GREY);
        lv_obj_set_style_radius(mainBox, 20, 0);
        lv_obj_align(mainBox, LV_ALIGN_CENTER, 0, 0);
        // Imprime mensagem
        label = lv_label_create(mainBox);
        lv_label_set_text(label, "Start update?");
        lv_obj_set_style_text_color(label, OSD_COLOR_BLACK, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
        // Cria botão
        btn = lv_button_create(mainBox);
        lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
        lv_obj_remove_flag(btn, LV_OBJ_FLAG_PRESS_LOCK);
        label = lv_label_create(btn);
        lv_label_set_text(label, "Start");
        lv_obj_center(label);
    }
    else
    {
        obj_del(label_btn);
        obj_del(btn);
        obj_del(label);
        obj_del(mainBox);
    }
}

void updatingApp(bool start)
{
    static lv_obj_t *mainBox  = { nullptr };
    static lv_obj_t *label      = { nullptr };
    auto width = DISPLAY_WIDTH / 3;
    auto heigth = DISPLAY_HEIGHT / 4;

    if(start)
    {
        // Cria área da tela
        mainBox = create_rect(lv_screen_active(), 0, 0, width, heigth, OSD_COLOR_GREY);
        lv_obj_set_style_radius(mainBox, 20, 0);
        lv_obj_align(mainBox, LV_ALIGN_CENTER, 0, 0);
        // Imprime mensagem
        label = lv_label_create(mainBox);
        lv_label_set_text(label, "Updating...");
        lv_obj_set_style_text_color(label, OSD_COLOR_BLACK, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
        /*Create a slider in the center of the display*/
        slider = lv_slider_create(mainBox);
        lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, 0);
        /*Create a label below the slider*/
        slider_label = lv_label_create(mainBox);
        lv_obj_align(slider_label, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_label_set_text(slider_label, "0%");
    }
    else
    {
        obj_del(label);
        obj_del(mainBox);
    }
}

void updateFinished(bool start)
{
    static lv_obj_t *mainBox  = { nullptr };
    static lv_obj_t *label      = { nullptr };
    auto width = DISPLAY_WIDTH / 3;
    auto heigth = DISPLAY_HEIGHT / 4;

    if(start)
    {
        // Cria área da tela
        mainBox = create_rect(lv_screen_active(), 0, 0, width, heigth, OSD_COLOR_GREY);
        lv_obj_set_style_radius(mainBox, 20, 0);
        lv_obj_align(mainBox, LV_ALIGN_CENTER, 0, 0);
        // Imprime mensagem
        label = lv_label_create(mainBox);
        lv_label_set_text(label, "Updating finished with success!!!");
        lv_obj_set_style_text_color(label, OSD_COLOR_RED, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        obj_del(label);
        obj_del(mainBox);
    }
}

lv_obj_t *create_rect(lv_obj_t *bgd, int x, int y, int w, int h, lv_color_t color)
{
    lv_obj_t *m_rect = lv_obj_create(bgd);
    lv_obj_set_size(m_rect, w, h);
    lv_obj_set_scrollbar_mode(m_rect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(m_rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(m_rect, color, 0);
    lv_obj_set_style_radius(m_rect, 0, DEFAULT_SELECTOR);
    lv_obj_align(m_rect, LV_ALIGN_DEFAULT, x, y);
    return m_rect;
}

int main()
{
    lv_init();
    auto display = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(display, "/dev/fb0");
    lv_display_trigger_activity(NULL);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_COVER, LV_PART_MAIN);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_ARGB8888);
    lv_linux_fbdev_set_force_refresh(display, true);
    // Desenha tela inicial
    ask4Update(true);
    //updatingApp(true);
    // Inicia timer para apagar tela
    m_timer = lv_timer_create(process_timer_cb, 1000, nullptr);
    lv_timer_set_repeat_count(m_timer, 1000000);

    while(true)
        lv_timer_handler();

    lv_deinit();
    return 0;
}
