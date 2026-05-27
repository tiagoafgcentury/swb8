#pragma once

#include "common/mb_globals.h"
#include "mb_osd.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"

#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <map>

namespace mb {

class MB_OSD_Enhanced_Keys {
public:
    using KeyCallback = std::function<void()>;
    enum class Orientation { Horizontal, Vertical };

    struct Key {
        std::string label_text;
        std::string header_text;
        int id;
        bool enabled = true;
        bool visible = true;
        bool marked = false;
        KeyCallback callback = nullptr;
        std::map<Remote_Control_Key, KeyCallback> mapped_callbacks;
        
        // LVGL Objects
        lv_obj_t* btn_obj = nullptr;
        lv_obj_t* label_obj = nullptr;
        lv_obj_t* header_box_obj = nullptr;
        lv_obj_t* header_label_obj = nullptr;
    };

    struct Group {
        int x = 0, y = 0;
        int rows = 0, cols = 0;
        int key_width = 220, key_height = 50;
        int x_spacing = 10, y_spacing = 10;
        bool visible = true;
        bool enabled = true;
        Orientation orientation = Orientation::Horizontal;
        lv_align_t label_align = LV_ALIGN_CENTER;
        
        lv_color_t bg_color = OSD_COLOR_GREY_MEDIUM;
        lv_color_t selected_color = OSD_COLOR_ORANGE;
        lv_color_t marked_color = OSD_COLOR_ORANGE;
        lv_color_t disabled_text_color = OSD_COLOR_GREY_DARK;
        
        std::vector<Key> keys;
        int last_selected_idx = -1; // Local index within this group
    };

private:
    lv_obj_t* m_parent = nullptr;
    std::vector<Group> m_groups;
    size_t m_active_group_idx = 0;
    int m_selected_key_idx = 0; // Local index in active group

    lv_color_t m_global_selected_color = OSD_COLOR_ORANGE;
    lv_font_t* m_font = font_25;
    lv_font_t* m_font_sb = font_semi_25;

public:
    MB_OSD_Enhanced_Keys() {
        add_group(); // Ensure at least one group exists
    }

    MB_OSD_Enhanced_Keys(lv_obj_t* parent) : m_parent(parent) {
        add_group(); // Ensure at least one group exists
    }

    virtual ~MB_OSD_Enhanced_Keys() { clear(); }

    void set_parent(lv_obj_t* parent) { m_parent = parent; }

    size_t add_group() {
        m_groups.emplace_back();
        return m_groups.size() - 1;
    }

    // --- Configuration Setters for Current Group ---
    void set_group_pos(int x, int y) { m_groups.back().x = x; m_groups.back().y = y; }
    void set_group_grid(int rows, int cols) { m_groups.back().rows = rows; m_groups.back().cols = cols; }
    void set_group_orientation(Orientation orient) { m_groups.back().orientation = orient; }
    void set_group_key_size(int width, int height) { m_groups.back().key_width = width; m_groups.back().key_height = height; }
    void set_group_align(lv_align_t align) { m_groups.back().label_align = align; }
    void set_group_colors(lv_color_t bg, lv_color_t prev_sel) {
        m_groups.back().bg_color = bg;
        m_groups.back().marked_color = prev_sel;
    }
    void set_group_selected_color(lv_color_t color) {
        m_groups.back().selected_color = color;
    }
    void set_group_selected_color(size_t group_idx, lv_color_t color) {
        if (group_idx < m_groups.size()) {
            m_groups[group_idx].selected_color = color;
        }
    }
    void set_global_selected_color(lv_color_t color) {
        m_global_selected_color = color;
    }
    void set_group_disabled_text_color(size_t group_idx, lv_color_t color) {
        if (group_idx < m_groups.size()) {
            m_groups[group_idx].disabled_text_color = color;
        }
    }
    void set_group_disabled_text_color(lv_color_t color) {
        m_groups.back().disabled_text_color = color;
    }
    void set_group_spacing(int x_s, int y_s) { 
        m_groups.back().x_spacing = x_s; 
        m_groups.back().y_spacing = y_s; 
    }
    void set_group_enabled(size_t group_idx, bool enabled) {
        if (group_idx < m_groups.size()) {
            m_groups[group_idx].enabled = enabled;
        }
    }
    bool get_group_enabled(size_t group_idx) const {
        if (group_idx < m_groups.size()) {
            return m_groups[group_idx].enabled;
        }
        return false;
    }

    // --- Key Management ---
    size_t add_key(std::string_view label, KeyCallback cb = nullptr) {
        Key k;
        k.label_text = label;
        k.callback = cb;
        m_groups.back().keys.push_back(k);
        return m_groups.back().keys.size() - 1;
    }

    size_t add_key_with_header(std::string_view label, std::string_view header_text, const std::map<Remote_Control_Key, KeyCallback>& key_callbacks) {
        Key k;
        k.label_text = label;
        k.header_text = header_text;
        k.mapped_callbacks = key_callbacks;
        m_groups.back().keys.push_back(k);
        return m_groups.back().keys.size() - 1;
    }

    size_t add_key(std::string_view label, const std::map<Remote_Control_Key, KeyCallback>& key_callbacks) {
        Key k;
        k.label_text = label;
        k.mapped_callbacks = key_callbacks;
        m_groups.back().keys.push_back(k);
        return m_groups.back().keys.size() - 1;
    }

    KeyCallback get_selected_mapped_callback(Remote_Control_Key key) const {
        if (m_active_group_idx >= m_groups.size()) {
            return nullptr;
        }
        const auto& group = m_groups[m_active_group_idx];
        if (group.keys.empty()) {
            return nullptr;
        }
        if (m_selected_key_idx < 0 || static_cast<size_t>(m_selected_key_idx) >= group.keys.size()) {
            return nullptr;
        }

        const auto& callbacks = group.keys[m_selected_key_idx].mapped_callbacks;
        auto it = callbacks.find(key);
        if (it == callbacks.end()) {
            return nullptr;
        }
        return it->second;
    }

    std::string get_selected_label() const {
        if (m_active_group_idx >= m_groups.size()) {
            return "";
        }
        const auto& group = m_groups[m_active_group_idx];
        if (group.keys.empty()) {
            return "";
        }
        if (m_selected_key_idx < 0 || static_cast<size_t>(m_selected_key_idx) >= group.keys.size()) {
            return "";
        }

        return group.keys[m_selected_key_idx].label_text;
    }

    size_t get_selected_group() const {
        return m_active_group_idx;
    }

    int get_selected_key() const {
        return m_selected_key_idx;
    }

    size_t get_group_size(size_t group_idx) const {
        if (group_idx < m_groups.size()) {
            return m_groups[group_idx].keys.size();
        }
        return 0;
    }

    void set_selected_group(size_t group_idx) {
        if (group_idx < m_groups.size()) {
            if (m_groups[group_idx].keys.empty()) {
                return;
            }
            int restore_idx = m_groups[group_idx].last_selected_idx;
            if (restore_idx >= 0 && static_cast<size_t>(restore_idx) < m_groups[group_idx].keys.size() &&
                m_groups[group_idx].keys[restore_idx].enabled && m_groups[group_idx].keys[restore_idx].visible) {
                set_selected_key(group_idx, restore_idx);
                return;
            }
            for (size_t k = 0; k < m_groups[group_idx].keys.size(); ++k) {
                if (m_groups[group_idx].keys[k].enabled && m_groups[group_idx].keys[k].visible) {
                    set_selected_key(group_idx, k);
                    return;
                }
            }
            set_selected_key(group_idx, 0);
        }
    }

    void set_label(size_t group_idx, size_t key_idx, std::string_view label) {
        if (group_idx < m_groups.size() && key_idx < m_groups[group_idx].keys.size()) {
            m_groups[group_idx].keys[key_idx].label_text = label;
            if (m_groups[group_idx].keys[key_idx].label_obj) {
                lv_label_set_text(m_groups[group_idx].keys[key_idx].label_obj, m_groups[group_idx].keys[key_idx].label_text.c_str());
            }
        }
    }

    lv_obj_t* get_label(size_t group_idx, size_t key_idx) const {
        if (group_idx < m_groups.size() && key_idx < m_groups[group_idx].keys.size()) {
            return m_groups[group_idx].keys[key_idx].label_obj;
        }
        return nullptr;
    }

    void set_key_enabled(size_t group_idx, size_t key_idx, bool enabled) {
        if (group_idx < m_groups.size() && key_idx < m_groups[group_idx].keys.size()) {
            m_groups[group_idx].keys[key_idx].enabled = enabled;
            bool is_selected = (m_active_group_idx == group_idx && m_selected_key_idx == static_cast<int>(key_idx));
            update_key_visual(group_idx, static_cast<int>(key_idx), is_selected);
        }
    }

    void set_key_marked(size_t group_idx, size_t key_idx, bool marked) {
        if (group_idx < m_groups.size() && key_idx < m_groups[group_idx].keys.size()) {
            m_groups[group_idx].keys[key_idx].marked = marked;
            bool is_selected = (group_idx == m_active_group_idx && static_cast<int>(key_idx) == m_selected_key_idx);
            update_key_visual(group_idx, static_cast<int>(key_idx), is_selected);
        }
    }

    void next_marked(size_t group_idx) {
        if (group_idx >= m_groups.size()) return;
        auto& group = m_groups[group_idx];
        for (size_t i = 0; i < group.keys.size(); ++i) {
            if (group.keys[i].marked) {
                group.keys[i].marked = false;
                update_key_visual(group_idx, static_cast<int>(i), group_idx == m_active_group_idx && static_cast<int>(i) == m_selected_key_idx);
                size_t next = (i + 1) % group.keys.size();
                group.keys[next].marked = true;
                update_key_visual(group_idx, static_cast<int>(next), group_idx == m_active_group_idx && static_cast<int>(next) == m_selected_key_idx);
                return;
            }
        }
    }

    void prev_marked(size_t group_idx) {
        if (group_idx >= m_groups.size()) return;
        auto& group = m_groups[group_idx];
        for (size_t i = 0; i < group.keys.size(); ++i) {
            if (group.keys[i].marked) {
                group.keys[i].marked = false;
                update_key_visual(group_idx, static_cast<int>(i), group_idx == m_active_group_idx && static_cast<int>(i) == m_selected_key_idx);
                size_t prev = (i + group.keys.size() - 1) % group.keys.size();
                group.keys[prev].marked = true;
                update_key_visual(group_idx, static_cast<int>(prev), group_idx == m_active_group_idx && static_cast<int>(prev) == m_selected_key_idx);
                return;
            }
        }
    }

    int get_key_marked(size_t group_idx) const {
        if (group_idx >= m_groups.size()) return -1;
        const auto& group = m_groups[group_idx];
        for (size_t i = 0; i < group.keys.size(); ++i) {
            if (group.keys[i].marked) return static_cast<int>(i);
        }
        return -1;
    }

    void set_group_visible(size_t group_idx, bool visible) {
        if (group_idx < m_groups.size()) {
            m_groups[group_idx].visible = visible;
            if (!m_parent) {
                return;
            }

            auto& group = m_groups[group_idx];
            for (auto& key : group.keys) {
                if (!key.btn_obj) {
                    continue;
                }
                if (visible && key.visible) {
                    lv_obj_clear_flag(key.btn_obj, LV_OBJ_FLAG_HIDDEN);
                    if (key.header_box_obj) lv_obj_clear_flag(key.header_box_obj, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(key.btn_obj, LV_OBJ_FLAG_HIDDEN);
                    if (key.header_box_obj) lv_obj_add_flag(key.header_box_obj, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
    }

    void set_selected_key(size_t group_idx, size_t key_idx) {
        if (group_idx >= m_groups.size()) {
            return;
        }

        auto& group = m_groups[group_idx];
        if (key_idx >= group.keys.size()) {
            return;
        }

        size_t old_group_idx = m_active_group_idx;
        int old_key_idx = m_selected_key_idx;

        m_active_group_idx = group_idx;
        m_selected_key_idx = static_cast<int>(key_idx);
        m_groups[group_idx].last_selected_idx = static_cast<int>(key_idx);

        if (!m_parent) {
            return;
        }

        update_key_visual(old_group_idx, old_key_idx, false);
        update_key_visual(m_active_group_idx, m_selected_key_idx, true);
    }

    // --- Layout and Rendering ---
    void draw() {
        if (!m_parent) {
            return;
        }
        for (auto& group : m_groups) {
            // Cleanup existing
            for (auto& key : group.keys) {
                if (key.btn_obj) { lv_obj_del(key.btn_obj); key.btn_obj = nullptr; }
                if (key.header_box_obj) { lv_obj_del(key.header_box_obj); key.header_box_obj = nullptr; }
            }

            for (size_t i = 0; i < group.keys.size(); ++i) {
                auto& key = group.keys[i];

                // Calculate Grid Position
                int row = 0, col = 0;
                if (group.orientation == Orientation::Horizontal) {
                    col = (group.cols > 0) ? (i % group.cols) : i;
                    row = (group.cols > 0) ? (i / group.cols) : 0;
                } else {
                    row = (group.rows > 0) ? (i % group.rows) : i;
                    col = (group.rows > 0) ? (i / group.rows) : 0;
                }

                int x = group.x + (col * (group.key_width + group.x_spacing));
                int y = group.y + (row * (group.key_height + group.y_spacing));

                // Determine background color
                lv_color_t color = group.bg_color;
                if (&group == &m_groups[m_active_group_idx] && (int)i == m_selected_key_idx) {
                    color = group.selected_color;
                } else if (key.marked) {
                    color = group.marked_color;
                }

                key.btn_obj = mb::OSD::create_rect(m_parent, x, y, group.key_width, group.key_height, color);
                lv_obj_set_style_radius(key.btn_obj, group.key_height / 2, 0);
                
                lv_color_t text_color = key.enabled ? OSD_COLOR_WHITE : group.disabled_text_color;
                auto font = (&group == &m_groups[m_active_group_idx] && (int)i == m_selected_key_idx) ? m_font_sb : m_font;
                key.label_obj = mb::OSD::set_label_text(key.btn_obj, key.label_text, 0, 0, font, text_color);
                auto left_margin = group.label_align == LV_ALIGN_LEFT_MID ? 20 : 0;
                lv_obj_align(key.label_obj, group.label_align, left_margin, 0);

                if (!key.header_text.empty()) {
                    key.header_box_obj = mb::OSD::create_rect(m_parent, x, y - group.key_height + 8, group.key_width, group.key_height - 8, OSD_COLOR_BLACK);
                    key.header_label_obj = mb::OSD::set_label_text(key.header_box_obj, key.header_text, 0, 0, m_font_sb, OSD_COLOR_WHITE);
                    lv_obj_align(key.header_label_obj, LV_ALIGN_CENTER, 0, 0);
                }

                if (!group.visible || !key.visible) {
                    lv_obj_add_flag(key.btn_obj, LV_OBJ_FLAG_HIDDEN);
                    if (key.header_box_obj) lv_obj_add_flag(key.header_box_obj, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
    }

    // --- Navigation Logic ---
    void next(bool inside_group = true) { move_focus(1, 0, inside_group); }
    void prev(bool inside_group = true) { move_focus(-1, 0, inside_group); }
    
    void next_line(bool inside_group = false) { 
        int step = m_groups[m_active_group_idx].cols;
        move_focus(step > 0 ? step : 1, 0, inside_group); 
    }

    void prev_line(bool inside_group = false) { 
        int step = m_groups[m_active_group_idx].cols;
        move_focus(step > 0 ? -step : -1, 0, inside_group); 
    }

    void next_group() { move_group(1); }
    void prev_group() { move_group(-1); }

private:
    void move_group(int dir) {
        if (m_groups.empty() || m_groups.size() == 1) return;
        // Save current position in the current group
        m_groups[m_active_group_idx].last_selected_idx = m_selected_key_idx;
        size_t start_g = m_active_group_idx;
        size_t g_idx = start_g;
        do {
            g_idx = (g_idx + m_groups.size() + dir) % m_groups.size();
            if (m_groups[g_idx].visible && m_groups[g_idx].enabled && !m_groups[g_idx].keys.empty()) {
                // Try to restore previously selected key in this group
                int restore_idx = m_groups[g_idx].last_selected_idx;
                if (restore_idx >= 0 && static_cast<size_t>(restore_idx) < m_groups[g_idx].keys.size()
                    && m_groups[g_idx].keys[restore_idx].enabled && m_groups[g_idx].keys[restore_idx].visible) {
                    set_selected_key(g_idx, restore_idx);
                    return;
                }
                // Fallback: select first enabled/visible key
                for (size_t k = 0; k < m_groups[g_idx].keys.size(); ++k) {
                    if (m_groups[g_idx].keys[k].enabled && m_groups[g_idx].keys[k].visible) {
                        set_selected_key(g_idx, k);
                        return;
                    }
                }
            }
        } while (g_idx != start_g);
    }

    void move_focus(int key_delta, int group_delta, bool inside_group) {
        if (m_groups.empty()) {
            return;
        }

        size_t old_group_idx = m_active_group_idx;
        int old_key_idx = m_selected_key_idx;
        size_t g_idx = m_active_group_idx;
        int k_idx = m_selected_key_idx;

        // 1. Try moving within the current group
        if (key_delta != 0) {
            auto& g = m_groups[g_idx];
            if (g.keys.empty()) {
                return;
            }
            int start_k = k_idx;
            do {
                int next_k = k_idx + key_delta;
                if (!inside_group && (next_k < 0 || next_k >= static_cast<int>(g.keys.size()))) {
                    break;
                }
                k_idx = (next_k + static_cast<int>(g.keys.size())) % static_cast<int>(g.keys.size());
                if (g.keys[k_idx].enabled && g.keys[k_idx].visible) {
                    m_selected_key_idx = k_idx;
                    g.last_selected_idx = k_idx;
                    update_key_visual(old_group_idx, old_key_idx, false);
                    update_key_visual(m_active_group_idx, m_selected_key_idx, true);
                    return;
                }
            } while (k_idx != start_k);
        }

        // 2. If not inside_group, try moving to the next visible group
        if (!inside_group) {
            size_t start_g = g_idx;
            int dir = (key_delta >= 0) ? 1 : -1;
            do {
                g_idx = (g_idx + dir + m_groups.size()) % m_groups.size();
                if (m_groups[g_idx].visible && m_groups[g_idx].enabled && !m_groups[g_idx].keys.empty()) {
                    auto& g = m_groups[g_idx];
                    int candidate = (dir > 0) ? 0 : static_cast<int>(g.keys.size() - 1);
                    int start_k = candidate;
                    do {
                        if (g.keys[candidate].enabled && g.keys[candidate].visible) {
                            m_active_group_idx = g_idx;
                            m_selected_key_idx = candidate;
                            update_key_visual(old_group_idx, old_key_idx, false);
                            update_key_visual(m_active_group_idx, m_selected_key_idx, true);
                            return;
                        }
                        candidate = (candidate + dir + g.keys.size()) % g.keys.size();
                    } while (candidate != start_k);
                }
            } while (g_idx != start_g);
        }
    }

    void update_key_visual(size_t group_idx, int key_idx, bool selected) {
        if (group_idx >= m_groups.size()) {
            return;
        }
        auto& group = m_groups[group_idx];
        if (key_idx < 0 || static_cast<size_t>(key_idx) >= group.keys.size()) {
            return;
        }

        auto& key = group.keys[key_idx];
        if (!key.btn_obj || !key.label_obj) {
            return;
        }

        lv_color_t bg_color = group.bg_color;
        if (selected) {
            bg_color = group.selected_color;
        } else if (key.marked) {
            bg_color = group.marked_color;
        }

        lv_color_t text_color = key.enabled ? OSD_COLOR_WHITE : group.disabled_text_color;
        auto font = selected ? m_font_sb : m_font;

        lv_obj_set_style_bg_color(key.btn_obj, bg_color, 0);
        lv_obj_set_style_bg_opa(key.btn_obj, LV_OPA_MAX, 0);
        lv_obj_set_style_text_color(key.label_obj, text_color, 0);
        lv_obj_set_style_text_font(key.label_obj, font, 0);
    }

    void clear() {
        for (auto& g : m_groups) {
            for (auto& k : g.keys) {
                if (k.btn_obj && lv_obj_is_valid(k.btn_obj)) lv_obj_del(k.btn_obj);
                if (k.header_box_obj && lv_obj_is_valid(k.header_box_obj)) lv_obj_del(k.header_box_obj);
            }
        }
        m_groups.clear();
    }
};

} // namespace mb

#if 0 
/*
 * ============================================================================
 * EXAMPLES AND USE CASES
 * ============================================================================
 *
 * USE CASE 1: Split Navigation (Toolbar and Grid)
 * ---------------------------------------------
 * mb::MB_OSD_Enhanced_Keys menu(parent);
 * * // Group 0: Top Horizontal Toolbar
 * menu.set_group_pos(50, 20);
 * menu.set_group_grid(1, 4);
 * menu.add_key("Home"); menu.add_key("Search"); menu.add_key("Apps"); menu.add_key("Settings");
 * * // Group 1: Main 3x3 Grid
 * menu.add_group();
 * menu.set_group_pos(50, 100);
 * menu.set_group_grid(3, 3);
 * menu.set_group_colors(OSD_COLOR_BLUE, OSD_COLOR_CYAN);
 * for(int i=0; i<9; ++i) menu.add_key("Item " + std::to_string(i));
 * * menu.draw();
 * * // Interaction:
 * // menu.next(true);  -> Cycles ONLY within current group (Toolbar OR Grid)
 * // menu.next(false); -> Cycles keys, but jumps from Toolbar to Grid when reaching the end.
 *
 * USE CASE 2: Dynamic Visibility
 * ------------------------------
 * // Hide the sidebar group (index 2)
 * menu.set_group_visible(2, false); 
 * // Navigation functions will now automatically skip all keys in Group 2.
 *
 * USE CASE 3: Mapped Callbacks
 * ----------------------------
 * mb::MB_OSD_Enhanced_Keys menu(parent);
 * std::map<Remote_Control_Key, mb::MB_OSD_Enhanced_Keys::KeyCallback> callbacks = {
 *     { Remote_Control_Key::KEY_OK, []() { /* handle OK */ } },
 *     { Remote_Control_Key::KEY_CHUP, []() { /* handle up */ } },
 *     { Remote_Control_Key::KEY_CHDOWN, []() { /* handle down */ } }
 * };
 * menu.add_key("Item", callbacks);
 * menu.draw();
 *
 * // In your event handler:
 * auto cb = menu.get_selected_mapped_callback(event.key);
 * if (cb) { cb(); }
 *
 * USE CASE 4: Modifying Labels
 * ----------------------------
 * mb::MB_OSD_Enhanced_Keys menu(parent);
 * menu.add_key("Old Label");
 * menu.draw();
 * 
 * // Later on, update the text the same way as mb_osd_keys (referencing group and key indices)
 * menu.set_label(0, 0, "New Label");
 * lv_obj_t* label_obj = menu.get_label(0, 0); // access the lvgl label object if needed
 *
 * USE CASE 5: Keys with Headers
 * -----------------------------
 * mb::MB_OSD_Enhanced_Keys menu(parent);
 * menu.add_key_with_header("LNB Freq", "Universal"); // Will draw a black box with "Universal" above the key
 * menu.add_key_with_header("Port", "1");
 * menu.draw();
 */
#endif


