#pragma once

#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

namespace mb {

class MB_OSD_Keys
{
public:
    using KeyCallback = std::function<void()>;

private:
    struct Group {
        int lines = 0;
        int columns = 0;
        int key_width = 0;
        int key_height = 0;
        int x_spacing = 0;
        int y_spacing = 0;
        int key_x = 0;
        int key_y = 0;
        size_t start_index = 0;
        size_t count = 0;
        bool visible = true;
    };

    std::vector<Group> m_groups;
    std::vector<bool> m_keys_visible;

    std::vector<lv_obj_t *> m_keys;
    std::vector<lv_obj_t *> m_labels;
    std::vector<std::string> m_label_texts;
    std::vector<lv_obj_t *> m_headers;
    std::vector<lv_obj_t *> m_header_box;
    std::vector<std::string> m_header_text;
    std::vector<KeyCallback> m_key_callbacks;
    std::vector<uint8_t> m_set_indices;

    // Fontes padrão
    lv_font_t *m_font = font_25;
    lv_font_t *m_font_sb = font_semi_25;

    // Default layout values for new groups
    int m_offset_x = 0;
    int m_offset_y = 0;
    int m_default_key_width = 0;
    int m_default_key_height = 0;
    int m_default_x_spacing = 0;
    int m_default_y_spacing = 0;
    int m_default_key_x = 0;
    int m_default_key_y = 0;

    size_t m_selected = 0;
    lv_obj_t *m_main { nullptr };
    lv_color_t m_background_color = OSD_COLOR_GREY_MEDIUM;
    lv_color_t m_selected_color = OSD_COLOR_ORANGE;

    bool m_align_center = false;

    size_t m_previous_selected = -1;
    std::list<size_t> m_disabled;

    // Desenha as teclas na horizontal ou vertical
    enum Orientation
    {
        Horizontal,
        Vertical,
    };
    Orientation m_orientation = Orientation::Horizontal;

public:
    MB_OSD_Keys(int _offset_x, int _offset_y, int _key_width, int _key_heigth, int _spacing, int _key_x, int _key_y):
        m_offset_x(_offset_x),
        m_offset_y(_offset_y),
        m_default_key_width(_key_width),
        m_default_key_height(_key_heigth),
        m_default_x_spacing(_spacing),
        m_default_key_x(_key_x),
        m_default_key_y(_key_y)
    {
        clear();
    }

    virtual ~MB_OSD_Keys()
    {
        clear();
    }

    void operator= (std::initializer_list<std::string_view> _init)
    {
        for(const auto &label : _init)
        {
            add_label(label);
        }
    }

    // Adds a new group and makes it the current one for subsequent operations
    void add_group()
    {
        Group g;
        g.key_width = m_default_key_width;
        g.key_height = m_default_key_height;
        g.x_spacing = m_default_x_spacing;
        g.y_spacing = m_default_y_spacing; // Using x spacing as default if not specified
        g.key_x = m_default_key_x;
        g.key_y = m_default_key_y;
        g.start_index = m_keys.size();
        g.count = 0;
        g.lines = 0;
        g.columns = 0;
        g.visible = true;
        m_groups.push_back(g);
    }

    void set_group_visible(size_t group_index, bool visible)
    {
        if (group_index < m_groups.size())
        {
            m_groups[group_index].visible = visible;
            // Apply visibility to existing objects if they exist
            // (Only relevant if called after draw, but draw handles re-creation/flagging)
            // If objects exist, we update flags
            const auto& g = m_groups[group_index];
            for (size_t i = 0; i < g.count; ++i)
            {
                size_t key_idx = g.start_index + i;
                update_key_visibility(key_idx);
            }
        }
    }

    void set_key_visible(size_t key_index, bool visible)
    {
        if (key_index < m_keys_visible.size())
        {
            m_keys_visible[key_index] = visible;
            update_key_visibility(key_index);
        }
    }

    void update_key_visibility(size_t key_index)
    {
        if (key_index >= m_keys.size()) return;

        bool visible = m_keys_visible[key_index];
        // Check group visibility
        for (const auto& g : m_groups)
        {
            if (key_index >= g.start_index && key_index < g.start_index + g.count)
            {
                if (!g.visible) visible = false;
                break;
            }
        }

        if (m_keys[key_index])
        {
            if (visible) lv_obj_clear_flag(m_keys[key_index], LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(m_keys[key_index], LV_OBJ_FLAG_HIDDEN);
        }
        
        // Also headers
        if (key_index < m_headers.size() && m_headers[key_index])
        {
             if (visible) lv_obj_clear_flag(m_headers[key_index], LV_OBJ_FLAG_HIDDEN);
             else lv_obj_add_flag(m_headers[key_index], LV_OBJ_FLAG_HIDDEN);
        }
        if (key_index < m_header_box.size() && m_header_box[key_index])
        {
             if (visible) lv_obj_clear_flag(m_header_box[key_index], LV_OBJ_FLAG_HIDDEN);
             else lv_obj_add_flag(m_header_box[key_index], LV_OBJ_FLAG_HIDDEN);
        }
    }

    void set_lines(int _lines)
    {
        if (!m_groups.empty()) m_groups.back().lines = _lines;
    }

    void set_columns(int _columns)
    {
        if (!m_groups.empty()) m_groups.back().columns = _columns;
    }

    void set_spacing(int _spacing)
    {
        if (!m_groups.empty()) m_groups.back().x_spacing = _spacing;
    }

    void set_spacing(int _x_spacing, int _y_spacing)
    {
        if (!m_groups.empty()) {
            m_groups.back().x_spacing = _x_spacing;
            m_groups.back().y_spacing = _y_spacing;
        }
    }

    void set_width(int _key_width)
    {
        if (!m_groups.empty()) m_groups.back().key_width = _key_width;
        // Also update default for future groups? No, specific setters apply to current.
    }

    void set_height(int _key_height)
    {
        if (!m_groups.empty()) m_groups.back().key_height = _key_height;
    }

    void set_x(int _key_x)
    {
        if (!m_groups.empty()) m_groups.back().key_x = _key_x;
    }

    void set_y(int _key_y)
    {
        if (!m_groups.empty()) m_groups.back().key_y = _key_y;
    }

    void set_back_color(lv_color_t _back_color)
    {
        m_background_color = _back_color;
    }

    void set_selected_color(lv_color_t _selected_color)
    {
        m_selected_color = _selected_color;
    }

    void set_background(lv_obj_t *_bkg)
    {
        m_main = _bkg;
    }

    void set_horizontal()
    {
        m_orientation = Orientation::Horizontal;
    }

    void set_vertical()
    {
        m_orientation = Orientation::Vertical;
    }

    void set_fonts(lv_font_t *_font, lv_font_t *_font_sb)
    {
        m_font = _font;
        m_font_sb = _font_sb;
    }

    void set_label(size_t _index, std::string_view _label)
    {
        mb_assert(_index < m_labels.size());

        if (_index >= m_labels.size())
        {
            return;
        }

        m_label_texts[_index] = _label;

        if (m_labels[_index])
        {
            lv_label_set_text(m_labels[_index], _label.data());
        }
    }

    lv_obj_t *get_label(size_t _index)
    {
        mb_assert(_index < m_labels.size());

        if (_index > m_labels.size())
        {
            return nullptr;
        }

        return m_labels[_index];
    }

    lv_obj_t *get_button(size_t _index)
    {
        mb_assert(_index < m_labels.size());

        if (_index > m_labels.size())
        {
            return nullptr;
        }

        return m_keys[_index];
    }

    void safe_delete_obj(lv_obj_t* &obj)
    {
        if (obj)
        {
            if (lv_obj_is_valid(obj))
            {
                lv_obj_del(obj);
            }
            obj = nullptr;
        }
    }

    template <typename T>
    void delete_objects(std::vector<T *> &_objects)
    {
        for (auto &obj : _objects)
        {
            safe_delete_obj(obj);
        }
        _objects.clear();
    }

    void clear()
    {
        delete_objects(m_labels);
        delete_objects(m_keys);
        delete_objects(m_headers);
        delete_objects(m_header_box);

        m_label_texts.clear();
        m_disabled.clear();
        m_header_text.clear();
        m_keys_visible.clear();
        m_groups.clear();
        m_key_callbacks.clear();
        m_set_indices.clear();
        m_selected = 0;
        m_previous_selected = -1;

        // Re-create default group
        add_group();
    }

    void draw()
    {
        for (const auto& group : m_groups)
        {
            // Calculate base position for the group
            auto x_base = group.key_x - m_offset_x;
            auto y_base = group.key_y - m_offset_y;

            for (size_t k = 0; k < group.count; ++k)
            {
                size_t i = group.start_index + k;
                if (i >= m_keys.size()) break;

                int x = x_base;
                int y = y_base;

                if (m_orientation == Orientation::Horizontal)
                {
                    if (group.columns && (k >= (size_t)group.columns))
                    {
                        x = x_base + ((k % group.columns) * group.x_spacing);
                        int ys = group.y_spacing ? group.y_spacing : (2 * group.key_height);
                        y = y_base + ((k / group.columns) * ys);
                    }
                    else
                    {
                        x = x_base + (k * group.x_spacing);
                    }
                }
                else
                {
                    y = y_base + (k * group.x_spacing); // Vertical uses x_spacing? Based on old code: y = ... + (i * m_x_spacing)
                }

                // Cleanup existing objects if any
                safe_delete_obj(m_labels[i]);
                safe_delete_obj(m_keys[i]);
                if (i < m_headers.size()) DELETE_OBJ(m_headers[i]);
                if (i < m_header_box.size()) DELETE_OBJ(m_header_box[i]);

                auto background = m_previous_selected == i ? m_selected_color : m_background_color;
                m_keys[i] = mb::OSD::create_rect(m_main, x, y, group.key_width, group.key_height, background);
                lv_obj_set_style_radius(m_keys[i], group.key_height / 2, mb::DEFAULT_SELECTOR);

                if (m_previous_selected == i)
                {
                    lv_obj_set_style_bg_opa(m_keys[i], LV_OPA_50, 0);
                }

                // Check disabled
                auto font_color = OSD_COLOR_WHITE;
                for (auto d : m_disabled)
                {
                    if (i == d)
                    {
                        font_color = OSD_COLOR_GREY_DARK;
                    }
                }

                m_labels[i] = mb::OSD::set_label_text(m_keys[i], m_label_texts[i], 0, 0, m_font, font_color);

                if (m_orientation == Orientation::Vertical and not m_align_center)
                {
                    lv_obj_align(m_labels[i], LV_ALIGN_LEFT_MID, 20, 0);
                    lv_label_set_long_mode(m_labels[i], LV_LABEL_LONG_DOT);
                    lv_obj_set_width(m_labels[i], group.key_width - 40);
                    lv_obj_set_height(m_labels[i], group.key_height - 20);
                }
                else
                {
                    lv_obj_center(m_labels[i]);
                }

                // Check header
                if (i < m_header_text.size())
                {
                    m_header_box[i] = mb::OSD::create_rect(m_main, x, y - group.key_height + 8, group.key_width, group.key_height - 8, OSD_COLOR_BLACK);
                    m_headers[i] = mb::OSD::set_label_text(m_header_box[i], m_header_text[i], 0, 0, m_font_sb, OSD_COLOR_WHITE);
                    lv_obj_align(m_headers[i], LV_ALIGN_CENTER, 0, 0);
                }

                // Check visibility
                update_key_visibility(i);
            }
        }
    }

    void set_align_center()
    {
        m_align_center = true;
    }

    void select(size_t _index)
    {
        // Verifica se recebeu um parâmetro válido
        if (_index >= m_labels.size())
        {
            return;
        }

        // Deseleciona o anterior
        unselect();

        // Atualiza campo com parâmetro válido
        m_selected = _index;

        // Se valor iniciado, coloca a cor padrão
        select();
    }

    void select()
    {
        if (m_selected < m_keys.size() && m_keys[m_selected]) {
            lv_obj_set_style_bg_color(m_keys[m_selected], m_selected_color, mb::DEFAULT_SELECTOR);
            if(m_selected < m_labels.size() && m_labels[m_selected])
                lv_obj_set_style_text_font(m_labels[m_selected], m_font_sb, mb::DEFAULT_SELECTOR);
            lv_obj_set_style_bg_opa(m_keys[m_selected], LV_OPA_MAX, 0);
        }
    } 

    void unselect()
    {
        if (m_selected < m_keys.size() && m_keys[m_selected]) {
            auto background = m_selected == m_previous_selected ? m_selected_color : m_background_color;
            lv_obj_set_style_bg_color(m_keys[m_selected], background, mb::DEFAULT_SELECTOR);
            if(m_selected < m_labels.size() && m_labels[m_selected])
                lv_obj_set_style_text_font(m_labels[m_selected], m_font, mb::DEFAULT_SELECTOR);
            // Ajusta opacidade
            auto opa = m_selected == m_previous_selected ? LV_OPA_50 : LV_OPA_MAX;
            lv_obj_set_style_bg_opa(m_keys[m_selected], opa, 0);
        }
    }

    size_t get_size()
    {
        return m_keys.size();
    }

    void add_label(std::string_view _label)
    {
        add_label(_label, nullptr, 0);
    }

    void add_label(std::string_view _label, KeyCallback _function, uint8_t _set_index)
    {
        m_label_texts.emplace_back(_label);
        m_keys.push_back(nullptr);
        m_labels.push_back(nullptr);
        m_keys_visible.push_back(true);
        m_key_callbacks.push_back(_function);
        m_set_indices.push_back(_set_index);
        
        if (!m_groups.empty()) {
            m_groups.back().count++;
        }
    }

    KeyCallback get_callback(size_t index)
    {
        if (index < m_key_callbacks.size()) {
            return m_key_callbacks[index];
        }
        return nullptr;
    }

    KeyCallback get_selected_callback()
    {
        return get_callback(m_selected);
    }

    uint8_t get_set_index(size_t index)
    {
        if (index < m_set_indices.size()) {
            return m_set_indices[index];
        }
        return 0;
    }

    void add_header(std::string_view _header)
    {
        m_header_text.emplace_back(_header);
        m_header_box.emplace_back(nullptr);
        m_headers.push_back(nullptr);
    }

    auto get_selected()
    {
        return m_selected;
    }

    size_t get_enabled_count()
    {
        size_t count = 0;
        for(size_t i=0; i<m_keys.size(); ++i) {
             bool disabled = false;
             for (auto d : m_disabled) if (i == d) disabled = true;
             if (!disabled && is_visible(i)) count++;
        }
        return count;
    }

    void swap_disabled(size_t _index)
    {
        if (_index > m_keys.size())
        {
            return;
        }

        auto d = m_disabled;

        if (std::find(d.begin(), d.end(), _index) == d.end())
        {
            m_disabled.push_back(_index);
            if(m_labels[_index])
                lv_obj_set_style_text_color(m_labels[_index], OSD_COLOR_GREY_DARK, mb::DEFAULT_SELECTOR);
        }
        else
        {
            m_disabled.remove(_index);
            if(m_labels[_index])
                lv_obj_set_style_text_color(m_labels[_index], OSD_COLOR_WHITE, mb::DEFAULT_SELECTOR);
        }
    }

    void set_enabled(size_t _index)
    {
        auto d = m_disabled;

        if (std::find(d.begin(), d.end(), _index) != d.end())
        {
            m_disabled.remove(_index);
            if(_index < m_labels.size() && m_labels[_index])
                lv_obj_set_style_text_color(m_labels[_index], OSD_COLOR_WHITE, mb::DEFAULT_SELECTOR);
        }
    }

    void set_disabled(size_t _index)
    {
        auto d = m_disabled;

        if (std::find(d.begin(), d.end(), _index) == d.end())
        {
            m_disabled.push_back(_index);
            if(_index < m_labels.size() && m_labels[_index])
                lv_obj_set_style_text_color(m_labels[_index], OSD_COLOR_GREY_DARK, mb::DEFAULT_SELECTOR);
        }
    }

    void set_disable_all()
    {
        for (size_t i = 0 ; i < m_keys.size() ; i++)
        {
            set_disabled(i);
        }
    }

    void set_enable_all()
    {
        for (size_t i = 0 ; i < m_keys.size() ; i++)
        {
            set_enabled(i);
        }
    }
    
    void remove_disabled(size_t _index)
    {
        auto d = m_disabled;

        if (std::find(d.begin(), d.end(), _index) != d.end())
        {
            m_disabled.remove(_index);
            if(_index < m_labels.size() && m_labels[_index])
                lv_obj_set_style_text_color(m_labels[_index], OSD_COLOR_WHITE, mb::DEFAULT_SELECTOR);
        }
    }

    void set_previously_selected(size_t _index)
    {
        // Verifica se recebeu um parâmetro válido
        if (_index >= m_labels.size())
        {
            return;
        }

        size_t old_prev = m_previous_selected;

        // Atualiza campo com parâmetro válido
        m_previous_selected = _index;

        // Se valor iniciado, coloca a cor padrão
        if (m_keys[m_previous_selected])
        {
            if (old_prev < m_keys.size() && m_keys[old_prev] && old_prev != m_selected)
            {
                lv_obj_set_style_bg_color(m_keys[old_prev], m_background_color, mb::DEFAULT_SELECTOR);
                lv_obj_set_style_bg_opa(m_keys[old_prev], LV_OPA_MAX, 0);
            }

            if (m_previous_selected < m_keys.size() && m_keys[m_previous_selected] && m_previous_selected != m_selected)
            {
                lv_obj_set_style_bg_color(m_keys[m_previous_selected], m_selected_color, mb::DEFAULT_SELECTOR);
                lv_obj_set_style_bg_opa(m_keys[m_previous_selected], LV_OPA_50, 0);
            }

            select();
        }
    }

    void next()
    {
        if (get_enabled_count() > 0)
        {
            unselect();
            m_selected = get_next(m_selected);
            select();
        }
    }

    void previous()
    {
        if (get_enabled_count() > 0)
        {
            unselect();
            m_selected = get_previous(m_selected);
            select();
        }
    }

    bool is_first_enabled()
    {
        auto previous = get_previous(m_selected);
        return previous < m_selected ? false : true;
    }

    bool is_last_enabled()
    {
        auto next = get_next(m_selected);
        return next > m_selected ? false : true;
    }

    // Helper to find group for an index
    const Group* get_group_for_index(size_t index) const {
        for (const auto& g : m_groups) {
            if (index >= g.start_index && index < g.start_index + g.count) return &g;
        }
        return nullptr;
    }

    // Helper to check if a key is effectively visible (both key and group)
    bool is_visible(size_t index) const {
        if (index >= m_keys_visible.size() || !m_keys_visible[index]) return false;
        const Group* g = get_group_for_index(index);
        return g && g->visible;
    }

    void next_line()
    {
        if (get_enabled_count() > 0)
        {
            unselect();
            
            size_t current = m_selected;
            const Group* g = get_group_for_index(current);
            if (g) {
                // Try moving down within group
                size_t offset_in_group = current - g->start_index;
                size_t next_in_group = offset_in_group + g->columns;
                
                if (next_in_group < g->count) {
                    m_selected = g->start_index + next_in_group;
                } else {
                    // Try to jump to next visible group
                    bool found = false;
                    size_t group_idx = 0;
                    // Find current group index
                    for(size_t i=0; i<m_groups.size(); ++i) {
                         if (&m_groups[i] == g) { group_idx = i; break; }
                    }
                    
                    // Look for next visible group
                    for (size_t i = group_idx + 1; i < m_groups.size(); ++i) {
                        if (m_groups[i].visible && m_groups[i].count > 0) {
                            m_selected = m_groups[i].start_index;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        // Loop back to start? Or stay? Current implementation of next_line() loops.
                        // (m_selected + m_columns) % m_keys.size();
                        // Let's loop back to first visible group.
                        for (size_t i = 0; i <= group_idx; ++i) {
                             if (m_groups[i].visible && m_groups[i].count > 0) {
                                m_selected = m_groups[i].start_index;
                                break;
                            }
                        }
                    }
                }
            } else {
                // Fallback
                m_selected = (m_selected + 1) % m_keys.size();
            }
            
            // Adjust if hidden/disabled
            // User requirement implies simple grid navigation.
            m_selected = get_next((m_keys.size() + m_selected - 1) % m_keys.size());
            
            select();
        }
    }

    void previous_line()
    {
        if (get_enabled_count() > 0)
        {
            unselect();
            
            size_t current = m_selected;
            const Group* g = get_group_for_index(current);
            if (g) {
                size_t offset_in_group = current - g->start_index;
                if (offset_in_group >= (size_t)g->columns) {
                    m_selected = current - g->columns;
                } else {
                    // Move to previous visible group
                    bool found = false;
                    size_t group_idx = 0;
                    for(size_t i=0; i<m_groups.size(); ++i) {
                         if (&m_groups[i] == g) { group_idx = i; break; }
                    }
                    
                    // Look for prev visible group
                    for (int i = (int)group_idx - 1; i >= 0; --i) {
                        if (m_groups[i].visible && m_groups[i].count > 0) {
                             // Go to last line of that group? Or first? usually last line.
                             // Actually, let's go to last element of that group to be safe, 
                             // or calculate column match?
                             // Simpler: Go to start of that group or end?
                             // If I go up from top of group B, I expect to be at bottom of group A.
                             size_t last_in_prev = m_groups[i].start_index + m_groups[i].count - 1;
                             m_selected = last_in_prev;
                             // Maybe adjust to be in the same column?
                             // Complex if columns differ. Keep it simple: last element.
                             found = true;
                             break;
                        }
                    }
                    if (!found) {
                        // Loop to last group
                         for (int i = (int)m_groups.size() - 1; i >= (int)group_idx; --i) {
                            if (m_groups[i].visible && m_groups[i].count > 0) {
                                m_selected = m_groups[i].start_index + m_groups[i].count - 1;
                                break;
                            }
                        }
                    }
                }
            } else {
                if (m_selected > 0) m_selected--;
                else m_selected = m_keys.size() - 1;
            }

            m_selected = get_previous((m_selected + 1) % m_keys.size());

            select();
        }
    }

    void next_in_group()
    {
        if (m_keys.empty()) return;

        const Group* g = get_group_for_index(m_selected);
        if (!g || g->count == 0) return;

        size_t start = g->start_index;
        size_t end = start + g->count;
        
        if (m_selected < start || m_selected >= end) return;

        size_t current = m_selected;
        for (size_t i = 0; i < g->count; ++i) {
            current++;
            if (current >= end) current = start;

            bool disabled = false;
            for (auto d : m_disabled) {
                if (current == d) {
                    disabled = true;
                    break;
                }
            }

            if (!disabled && is_visible(current)) {
                if (current != m_selected) {
                    unselect();
                    m_selected = current;
                    select();
                }
                return;
            }
        }
    }

    void previous_in_group()
    {
        if (m_keys.empty()) return;

        const Group* g = get_group_for_index(m_selected);
        if (!g || g->count == 0) return;

        size_t start = g->start_index;
        size_t end = start + g->count;
        
        if (m_selected < start || m_selected >= end) return;

        size_t current = m_selected;
        for (size_t i = 0; i < g->count; ++i) {
            if (current == start) current = end - 1;
            else current--;

            bool disabled = false;
            for (auto d : m_disabled) {
                if (current == d) {
                    disabled = true;
                    break;
                }
            }

            if (!disabled && is_visible(current)) {
                if (current != m_selected) {
                    unselect();
                    m_selected = current;
                    select();
                }
                return;
            }
        }
    }

    void go_to_last_enabled()
    {
        if (get_enabled_count() > 0)
        {
            // Varre todas as teclas
            for (int i = m_keys.size() - 1 ; i >= 0; i--)
            {
                // Verifica se a tecla não está desabilitada e é visível
                if (std::find(m_disabled.begin(), m_disabled.end(), i) == m_disabled.end() && is_visible(i))
                {
                    unselect();
                    m_selected = i;
                    select();
                    break;
                }
            }
        }
    }

    void go_to_first_enabled()
    {
        if (get_enabled_count() > 0)
        {
            // Varre todas as teclas
            for (size_t i = 0 ; i < m_keys.size(); i++)
            {
                // Verifica se a tecla não está desabilitada e é visível
                if (std::find(m_disabled.begin(), m_disabled.end(), i) == m_disabled.end() && is_visible(i))
                {
                    unselect();
                    m_selected = i;
                    select();
                    break;
                }
            }
        }
    }

    size_t get_previous(size_t sel)
    {
        if (m_keys.empty()) return sel;

        size_t result = sel;
        for (size_t i = 0; i < m_keys.size(); ++i)
        {
            result = (m_keys.size() + result - 1) % m_keys.size();

            bool disabled = false;
            for (auto d : m_disabled) if (result == d) disabled = true;

            if (!disabled && is_visible(result))
            {
                return result;
            }
        }
        return sel; // All disabled/invisible
    }

    size_t get_previous_recursive(size_t sel)
    {
        // Kept for ABI compatibility if referenced elsewhere, but delegates to iterative logic
        // Actually, internal usage was only for recursive calls.
        // next_line / previous_line calls get_previous_recursive(..., 0).
        // Let's replace usage in next_line/previous_line and remove this or redirect.
        return get_previous(sel);
    }

    size_t get_next(size_t sel)
    {
        if (m_keys.empty()) return sel;

        size_t result = sel;
        for (size_t i = 0; i < m_keys.size(); ++i)
        {
            result = (result + 1) % m_keys.size();

            bool disabled = false;
            for (auto d : m_disabled) if (result == d) disabled = true;

            if (!disabled && is_visible(result))
            {
                return result;
            }
        }
        return sel; // All disabled/invisible
    }

    size_t get_next_recursive(size_t sel)
    {
        return get_next(sel);
    }

    bool is_last_line()
    {
        const Group* g = get_group_for_index(m_selected);
        if (!g) return true;

        bool last_in_group = false;
        size_t idx_in_group = m_selected - g->start_index;

        if (m_orientation == Orientation::Horizontal)
        {
            if (g->columns == 0) last_in_group = true;
            else last_in_group = (idx_in_group / g->columns) >= static_cast<size_t>(g->lines - 1);
        }
        else
        {
             if (g->lines == 0) last_in_group = true;
             else last_in_group = (idx_in_group / g->lines) == 0; 
        }

        if (!last_in_group) return false;

        // Check if there are visible groups after this one
        size_t group_idx = 0;
        for(size_t i=0; i<m_groups.size(); ++i) { if (&m_groups[i] == g) { group_idx = i; break; } }

        for (size_t i = group_idx + 1; i < m_groups.size(); ++i) {
            if (m_groups[i].visible && m_groups[i].count > 0) return false;
        }

        return true;
    }

    bool is_first_line()
    {
        const Group* g = get_group_for_index(m_selected);
        if (!g) return true;

        bool first_in_group = false;
        size_t idx_in_group = m_selected - g->start_index;

        if (m_orientation == Orientation::Horizontal)
        {
            if (g->columns == 0) first_in_group = true;
            else first_in_group = (idx_in_group / g->columns) == 0;
        }
        else
        {
             if (g->lines == 0) first_in_group = true;
             else first_in_group = (idx_in_group / g->lines) == 0;
        }

        if (!first_in_group) return false;

        // Check if there are visible groups before this one
        size_t group_idx = 0;
        for(size_t i=0; i<m_groups.size(); ++i) { if (&m_groups[i] == g) { group_idx = i; break; } }

        for (int i = (int)group_idx - 1; i >= 0; --i) {
            if (m_groups[i].visible && m_groups[i].count > 0) return false;
        }

        return true;
    }

    void hide()
    {
        // Hide all
        for (auto &k : m_keys) if (k) lv_obj_add_flag(k, LV_OBJ_FLAG_HIDDEN);
        for (auto &h : m_headers) if (h) lv_obj_add_flag(h, LV_OBJ_FLAG_HIDDEN);
        for (auto &hb : m_header_box) if (hb) lv_obj_add_flag(hb, LV_OBJ_FLAG_HIDDEN);
    }

    void show()
    {
        // Show all that should be visible
        for(size_t i=0; i<m_keys.size(); ++i) {
            update_key_visibility(i);
        }
    }
};

}

/*
 * Examples of use:
 *
 * // Single Group
 * MB_OSD_Keys keys(0, 0, 100, 50, 10, 10, 10);
 * keys.set_lines(2);
 * keys.set_columns(2);
 * keys.add_label("Key 1", nullptr, 0);
 * keys.add_label("Key 2", nullptr, 1);
 * keys.add_label("Key 3", nullptr, 2);
 * keys.add_label("Key 4", nullptr, 3);
 * keys.draw();
 *
 * 
 * 
 * 
 * // Double Group
 * MB_OSD_Keys keys(0, 0, 100, 50, 10, 10, 10);
 *
 * // First Group
 * keys.set_lines(1);
 * keys.set_columns(2);
 * keys.add_label("G1-K1");
 * keys.add_label("G1-K2");
 *
 * // Second Group
 * keys.add_group();
 * keys.set_x(10);
 * keys.set_y(100); // Position below first group
 * keys.set_lines(1);
 * keys.set_columns(2);
 * keys.add_label("G2-K1");
 * keys.add_label("G2-K2");
 *
 * keys.draw();
 */
