#include <lvgl.h>
#include "common/mb_globals.h"
#include "mb_osd_fade_canvas.h"

namespace {

constexpr auto PI = 3.141592653589793f;
constexpr auto VIDEO_WIDTH = (DISPLAY_WIDTH / 2) + 20;
constexpr auto VIDEO_HEIGHT = (DISPLAY_HEIGHT / 2) + 20;
constexpr auto VIDEO_FADE_WIDTH = VIDEO_HEIGHT / 4;
constexpr auto VIDEO_FADE_HEIGHT = VIDEO_HEIGHT / 4;
constexpr lv_opa_t VIDEO_BASE_OPA = 100;

lv_opa_t video_map[VIDEO_WIDTH * VIDEO_HEIGHT];
bool video_map_ready = false;

constexpr lv_opa_t INFO_BASE_OPA = 200;
constexpr auto INFO_OPA_FAC = INFO_BASE_OPA / 2;
constexpr auto INFO_BOTTOM_START_X1 = 0;
constexpr auto INFO_BOTTOM_START_Y1 = ((DISPLAY_HEIGHT / 4) * 3);
constexpr auto INFO_BOTTOM_START_X2 = DISPLAY_WIDTH;
constexpr auto INFO_BOTTOM_START_Y2 = ((DISPLAY_HEIGHT / 4) * 3) + (DISPLAY_HEIGHT / 8);
constexpr auto INFO_TOP_START_X1 = 0;
constexpr auto INFO_TOP_START_Y1 = DISPLAY_HEIGHT / 8;
constexpr auto INFO_TOP_START_X2 = DISPLAY_WIDTH;
constexpr auto INFO_TOP_START_Y2 = DISPLAY_HEIGHT / 4;
constexpr auto INFO_WIDTH_TOP = INFO_TOP_START_X2 - INFO_TOP_START_X1;
constexpr auto INFO_HEIGHT_TOP = INFO_TOP_START_Y2 - INFO_TOP_START_Y1;
constexpr auto INFO_WIDTH_BOTTOM = INFO_BOTTOM_START_X2 - INFO_BOTTOM_START_X1;
constexpr auto INFO_HEIGHT_BOTTOM = INFO_BOTTOM_START_Y2 - INFO_BOTTOM_START_Y1;

lv_opa_t info_curve[INFO_HEIGHT_TOP];
bool info_map_ready = false;

}

namespace mb {

Fade_Canvas::Fade_Canvas()
{
}

Fade_Canvas::~Fade_Canvas()
{
    lv_draw_buf_destroy(draw_buf);
}

void Fade_Canvas::prepare_masks()
{
    if (!info_map_ready)
    {
        for (int i = 0; i < INFO_HEIGHT_TOP; i++)
        {
            info_curve[i] = roundf(cosf((static_cast<float>(i + INFO_HEIGHT_TOP) / static_cast<float>(INFO_HEIGHT_TOP)) * PI) * INFO_OPA_FAC) + INFO_OPA_FAC;
        }

        info_map_ready = true;
        DEBUG_MSG(OSD, DEBUG, "Info map ready\n");
    }
    else if (!video_map_ready)
    {
        constexpr auto OPA_FAC = (255 - VIDEO_BASE_OPA) / 2;
        constexpr lv_opa_t RANDOM_DELTA = 20;
        lv_opa_t curve_w[VIDEO_FADE_WIDTH];
        lv_opa_t curve_h[VIDEO_FADE_HEIGHT];

        for (int x = 0; x < VIDEO_FADE_WIDTH; x++)
        {
            curve_w[x] = roundf(cosf((static_cast<float>(x) / static_cast<float>(VIDEO_FADE_WIDTH)) * PI) * OPA_FAC) + OPA_FAC + VIDEO_BASE_OPA;
        }

        for (int y = 0; y < VIDEO_FADE_HEIGHT; y++)
        {
            curve_h[y] = roundf(cosf((static_cast<float>(y + VIDEO_FADE_HEIGHT) / static_cast<float>(VIDEO_FADE_HEIGHT)) * PI) * OPA_FAC) + OPA_FAC + VIDEO_BASE_OPA;
        }

        auto fuzz = [](lv_opa_t v)
        {
            auto delta = std::min<int>(255 - v, RANDOM_DELTA);
            return v + ((random() % delta) - (delta / 2));
        };

        memset(video_map, VIDEO_BASE_OPA - 1, sizeof(video_map));

        for (int y = 0; y < VIDEO_FADE_HEIGHT; y++)
        {
            lv_opa_t *row = video_map;
            row += (VIDEO_HEIGHT - VIDEO_FADE_HEIGHT + y) * VIDEO_WIDTH;

            for (int x = 0; x < VIDEO_WIDTH; x++)
            {
                row[x] = fuzz(VIDEO_BASE_OPA);
            }
        }

        for (int y = 0; y < VIDEO_FADE_HEIGHT; y++)
        {
            lv_opa_t *row = video_map;
            row += (VIDEO_HEIGHT - VIDEO_FADE_HEIGHT + y) * VIDEO_WIDTH;

            for (int x = 0; x < VIDEO_WIDTH; x++)
            {
                lv_opa_t v = fuzz(curve_h[y]);

                if (v > row[x])
                {
                    row[x] = v;
                }
            }
        }

        for (int y = 0; y < VIDEO_HEIGHT; y++)
        {
            lv_opa_t *row = video_map;
            row += y * VIDEO_WIDTH;

            for (int x = 0; x < VIDEO_FADE_WIDTH; x++)
            {
                lv_opa_t v = fuzz(curve_w[x]);

                if (v > row[x])
                {
                    row[x] = v;
                }
            }
        }

        constexpr auto BLUR_COMPENSATE_DIAG = 200;

        for (int i = 3; i < VIDEO_FADE_HEIGHT / 1.5; i++)
        {
            for (int j = -1; j < 2; j++)
            {
                lv_opa_t *row = video_map;
                row += (VIDEO_HEIGHT - i - j) * VIDEO_WIDTH;

                if (row[i] < BLUR_COMPENSATE_DIAG)
                {
                    row[i] = roundf((row[i] + BLUR_COMPENSATE_DIAG) / 2.0);
                }
            }
        }

        video_map_ready = true;
        DEBUG_MSG(OSD, DEBUG, "Video map ready\n");
    }
}

Fade_Canvas::Fade_Canvas_Ptr_t Fade_Canvas::make_video_mask(lv_obj_t *_parent)
{
    while (!video_map_ready)
    {
        prepare_masks();
    }

    constexpr auto start_x = 0;
    constexpr auto start_y = 0;
    // Prepare Canvas
    auto base_color = lv_color_make(0x00, 0x00, 0x00);
    auto result = std::make_unique<Fade_Canvas>();
    result->canvas = lv_canvas_create(_parent);
    auto canvas = result->canvas;
    auto draw_buf = result->draw_buf = lv_draw_buf_create(VIDEO_WIDTH, VIDEO_HEIGHT, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA, 0);
    lv_canvas_set_draw_buf(canvas, draw_buf);
    lv_canvas_fill_bg(canvas, base_color, VIDEO_BASE_OPA);
    // Draw base black rect
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = lv_color_make(0x00, 0x00, 0x00);
    lv_area_t coords = {start_x, start_y, VIDEO_FADE_WIDTH, VIDEO_HEIGHT};
    lv_draw_rect(&layer, &dsc, &coords);
    coords = {start_x, VIDEO_HEIGHT - VIDEO_FADE_HEIGHT, VIDEO_WIDTH, VIDEO_HEIGHT};
    lv_draw_rect(&layer, &dsc, &coords);
    lv_canvas_finish_layer(canvas, &layer);

    // Blur by taking the average of the surrounging 10 pixels. i.e. XX = SUM(PP) / 49
    //
    // PP PP PP PP PP PP PP
    // PP PP PP PP PP PP PP
    // PP PP PP PP PP PP PP
    // PP PP PP XX PP PP PP
    // PP PP PP PP PP PP PP
    // PP PP PP PP PP PP PP
    // PP PP PP PP PP PP PP

    for (int y = 5; y < VIDEO_HEIGHT - 10; y++)
    {
        auto r0 = video_map + ((y - 5) * VIDEO_WIDTH);
        auto r1 = video_map + ((y - 4) * VIDEO_WIDTH);
        auto r2 = video_map + ((y - 3) * VIDEO_WIDTH);
        auto r3 = video_map + ((y - 2) * VIDEO_WIDTH);
        auto r4 = video_map + ((y - 1) * VIDEO_WIDTH);
        auto r5 = video_map + ((y + 0) * VIDEO_WIDTH);
        auto r6 = video_map + ((y + 1) * VIDEO_WIDTH);
        auto r7 = video_map + ((y + 2) * VIDEO_WIDTH);
        auto r8 = video_map + ((y + 3) * VIDEO_WIDTH);
        auto r9 = video_map + ((y + 4) * VIDEO_WIDTH);
        auto r10 = video_map + ((y + 5) * VIDEO_WIDTH);

        for (int x = 5; x < VIDEO_WIDTH - 10; x++)
        {
            auto v0 = r0[x - 5] + r0[x - 4] + r0[x - 3] + r0[x - 2] + r0[x - 1] + r0[x + 0] + r0[x + 1] + r0[x + 2] + r0[x + 3] + r0[x + 4] + r0[x + 5];
            auto v1 = r1[x - 5] + r1[x - 4] + r1[x - 3] + r1[x - 2] + r1[x - 1] + r1[x + 0] + r1[x + 1] + r1[x + 2] + r1[x + 3] + r1[x + 4] + r1[x + 5];
            auto v2 = r2[x - 5] + r2[x - 4] + r2[x - 3] + r2[x - 2] + r2[x - 1] + r2[x + 0] + r2[x + 1] + r2[x + 2] + r2[x + 3] + r2[x + 4] + r2[x + 5];
            auto v3 = r3[x - 5] + r3[x - 4] + r3[x - 3] + r3[x - 2] + r3[x - 1] + r3[x + 0] + r3[x + 1] + r3[x + 2] + r3[x + 3] + r3[x + 4] + r3[x + 5];
            auto v4 = r4[x - 5] + r4[x - 4] + r4[x - 3] + r4[x - 2] + r4[x - 1] + r4[x + 0] + r4[x + 1] + r4[x + 2] + r4[x + 3] + r4[x + 4] + r4[x + 5];
            auto v5 = r5[x - 5] + r5[x - 4] + r5[x - 3] + r5[x - 2] + r5[x - 1] + r5[x + 0] + r5[x + 1] + r5[x + 2] + r5[x + 3] + r5[x + 4] + r5[x + 5];
            auto v6 = r6[x - 5] + r6[x - 4] + r6[x - 3] + r6[x - 2] + r6[x - 1] + r6[x + 0] + r6[x + 1] + r6[x + 2] + r6[x + 3] + r6[x + 4] + r6[x + 5];
            auto v7 = r7[x - 5] + r7[x - 4] + r7[x - 3] + r7[x - 2] + r7[x - 1] + r7[x + 0] + r7[x + 1] + r7[x + 2] + r7[x + 3] + r7[x + 4] + r7[x + 5];
            auto v8 = r8[x - 5] + r8[x - 4] + r8[x - 3] + r8[x - 2] + r8[x - 1] + r8[x + 0] + r8[x + 1] + r8[x + 2] + r8[x + 3] + r8[x + 4] + r8[x + 5];
            auto v9 = r9[x - 5] + r9[x - 4] + r9[x - 3] + r9[x - 2] + r9[x - 1] + r9[x + 0] + r9[x + 1] + r9[x + 2] + r9[x + 3] + r9[x + 4] + r9[x + 5];
            auto v10 = r10[x - 5] + r10[x - 4] + r10[x - 3] + r10[x - 2] + r10[x - 1] + r10[x + 0] + r10[x + 1] + r10[x + 2] + r10[x + 3] + r10[x + 4] + r10[x + 5];
            auto v = std::min((v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10) / 121, 255);
            lv_canvas_set_px(canvas, start_x + x, start_y + y, base_color, v);
        }
    }

    return result;
}


std::tuple<Fade_Canvas::Fade_Canvas_Ptr_t, Fade_Canvas::Fade_Canvas_Ptr_t> Fade_Canvas::make_info_mask(lv_obj_t *_parent)
{
    while (!video_map_ready)
    {
        prepare_masks();
    }

    // Prepare Canvas
    auto base_color = lv_color_make(0x00, 0x00, 0x00);
    auto result = std::make_tuple(std::make_unique<Fade_Canvas>(), std::make_unique<Fade_Canvas>());
    std::get<0>(result)->canvas = lv_canvas_create(_parent);
    std::get<1>(result)->canvas = lv_canvas_create(_parent);
    auto top_canvas = std::get<0>(result)->canvas;
    auto bottom_canvas = std::get<1>(result)->canvas;
    auto top_draw_buf = std::get<0>(result)->draw_buf = lv_draw_buf_create(INFO_WIDTH_TOP, INFO_HEIGHT_TOP, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA, 0);
    auto bottom_draw_buf = std::get<1>(result)->draw_buf = lv_draw_buf_create(INFO_WIDTH_BOTTOM, INFO_HEIGHT_BOTTOM, LV_COLOR_FORMAT_NATIVE_WITH_ALPHA, 0);
    lv_canvas_set_draw_buf(top_canvas, top_draw_buf);
    lv_canvas_set_draw_buf(bottom_canvas, bottom_draw_buf);
    lv_canvas_fill_bg(top_canvas, base_color, INFO_BASE_OPA);
    lv_canvas_fill_bg(bottom_canvas, base_color, INFO_BASE_OPA);

    for (int y = 0; y < INFO_HEIGHT_TOP; y++)
    {
        for (int x = 0; x < INFO_WIDTH_TOP; x++)
        {
            lv_canvas_set_px(top_canvas, x, y, base_color, info_curve[INFO_HEIGHT_TOP - y - 1]);
            lv_canvas_set_px(bottom_canvas, x, y, base_color, info_curve[y]);
        }
    }

    return result;
}

} // namespace mb
