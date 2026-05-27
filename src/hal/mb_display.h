#pragma once

#include "common/mb_types.h"

#include <memory>

namespace mb {

class Display
{
private:
    struct Data;
    std::unique_ptr<Data> m_p;

public:
    enum class Quadrant
    {
        Q1,
        Q2,
        Q3,
        Q4,
        Fullscreen
    };

    Display();
    ~Display();

    static void clear();

    void take_snapshot();

    void resize_video_to_quadrant(Quadrant _q);

    static Display *get_instance();

    Color_Standard get_color_standard();
    Resolution_Standard get_resolution_standard();
    Aspect_Mode get_aspect_mode();
    void set_aspect_mode(Aspect_Mode _mode);
    void set_color_standard(Color_Standard _color_standard);
    void set_resolution_standard(Resolution_Standard _resoluton_standard);
    void set_cvbs_off();
    void set_cvbs_on();
    void set_all_display_settings(Resolution_Standard _resolution_standard, Aspect_Mode _aspect_mode, Color_Standard _color_standard);
};

} // namespace mb
