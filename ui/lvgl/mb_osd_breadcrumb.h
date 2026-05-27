#pragma once

#include "common/mb_globals.h"
#include "common/mb_lineup.h"
#include "mb_osd.h"
#include "mb_osd_translate.h"
#include "mb_menu_resources.h"

#include <vector>
#include <string>

namespace mb {

class Osd_Breadcrumb : public OSD
{
public:
    typedef std::vector<std::string> Vector;
    typedef Vector::size_type Position;

private:
    static constexpr auto width = DISPLAY_WIDTH;
    static constexpr auto heigth = DISPLAY_HEIGHT;
    static constexpr auto breadcrumb_width = width - 163 - 100;
    static constexpr auto breadcrumb_heigth = 50;
    static constexpr auto breadcrumb_x = 90;
    static constexpr auto breadcrumb_y = 42;

    Vector m_names;
    lv_obj_t *m_breadcrumb_box { nullptr };

    lv_obj_t *m_breadcrumb_label { nullptr };
    lv_obj_t *m_breadcrumb_last { nullptr };
    lv_obj_t *m_breadcrumb_logo { nullptr };

    void draw();

public:
    Osd_Breadcrumb(OSD *_parent);
    virtual ~Osd_Breadcrumb();
    void remove_name();
    void remove_name(bool redraw);
    void add_name(std::string_view _name);
    void add_name(std::string_view _name, bool redraw);
    void add_name(std::vector<std::string_view> _name_list);
    void replace_last_name(std::string_view _new_name);
    bool is_empty();
    void clear();
    void update(std::string_view _name);
    void init(lv_obj_t *_parent);
    bool is_initialized();
    void debug_print() const;

    Position get_position()
    {
        return m_names.size();
    }

    void clear_from_position(const Position &_it)
    {
        if (m_names.size() > _it)
        {
            m_names.resize(_it);
            draw();
        }
    }

    static Osd_Breadcrumb s_instance;
};

}
