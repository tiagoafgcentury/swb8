#pragma once

#include <memory>
#include <tuple>

namespace mb {

class Fade_Canvas
{
public:
    Fade_Canvas();
    ~Fade_Canvas();

    static void prepare_masks();

    typedef std::unique_ptr<Fade_Canvas> Fade_Canvas_Ptr_t;

    static Fade_Canvas_Ptr_t make_video_mask(lv_obj_t *_parent);

    /**
     * Cria fades top e bottom, respectivamente
     */
    static std::tuple<Fade_Canvas_Ptr_t, Fade_Canvas_Ptr_t> make_info_mask(lv_obj_t *_parent);

    lv_draw_buf_t *draw_buf { nullptr };
    lv_obj_t *canvas { nullptr };
};

} // namespace mbgui
