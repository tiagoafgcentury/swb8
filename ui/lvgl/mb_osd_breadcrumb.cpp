#include "mb_osd_breadcrumb.h"
#include "mb_osd_fonts.h"

namespace mb {

Osd_Breadcrumb Osd_Breadcrumb::s_instance(nullptr);

Osd_Breadcrumb::Osd_Breadcrumb(OSD *_parent):
    OSD(_parent)
{
}

Osd_Breadcrumb::~Osd_Breadcrumb()
{
}

void Osd_Breadcrumb::init(lv_obj_t *_parent)
{
    if (_parent)
    {
        m_breadcrumb_box = create_rect(_parent, breadcrumb_x, breadcrumb_y, breadcrumb_width, breadcrumb_heigth, OSD_COLOR_BLACK);
    }
    else
    {
        m_breadcrumb_box = create_rect(get_main_screen(OSD_Layer::MAIN_MENU), breadcrumb_x, breadcrumb_y, breadcrumb_width, breadcrumb_heigth, OSD_COLOR_BLACK);
    }

    lv_obj_null_on_delete(&m_breadcrumb_box);
    m_breadcrumb_logo = load_image(m_breadcrumb_box, LOGO_MENU_CENTURY_C_CINZA, 0, 0, breadcrumb_heigth, breadcrumb_heigth);
    lv_obj_null_on_delete(&m_breadcrumb_logo);
}

bool Osd_Breadcrumb::is_initialized()
{
    return m_breadcrumb_box != nullptr;
}

void Osd_Breadcrumb::draw()
{
    if (m_names.size() == 0)
    {
        return;
    }

    if (not m_breadcrumb_box) // Janela já foi destruída
    {
        return;
    }

    auto items = m_names;
    bool add_3_dots = false;
    do
    {
        // Monta texto temporário para medir largura real
        std::string temp_label;
        for (size_t i = 0; i < items.size() - 1; ++i)
        {
            temp_label += items[i] + " > ";
        }

        lv_point_t size;
        lv_text_get_size(&size,
                        temp_label.c_str(),
                        font_25,
                        0,
                        0,
                        LV_COORD_MAX,
                        LV_TEXT_FLAG_NONE);

        // Se couber na largura disponível, sai do loop
        if (size.x < (breadcrumb_width - 120))
        {
            break;
        }

        add_3_dots = true;

        if (items.size() > 2)
            items.erase(items.begin() + 1);
        else
            break;

    }
    while (true);

    if (add_3_dots)
    {
        items.insert(items.begin() + 1, "...");
    }

    // //#warning "DEBUG: Imprime os itens do caminho de migalhas no console"
    // std::cout << TERM_RED_BOLD << "Breadcrumb items: ";
    // for (const auto &item : items)
    // {
    //     std::cout << item << " | ";
    // }
    // std::cout << TERM_RESET << std::endl;

    // Separa o último nome do caminho de migalhas
    auto last = items.back();
    // Separa os primeiros nomes do caminho de migalhas
    std::vector<std::string> firsts;

    for (auto it = items.begin(); it != items.end() - 1; it++)
    {
        firsts.push_back(*it);
    }

    // Texto caminho de migalhas
    std::string label = "";
    for (auto it = firsts.begin(); it != firsts.end(); it++)
    {
        label = label + *it + " > ";
    }

    // Texto inicial caminho de migalhas
    update_or_create_label(m_breadcrumb_box, &m_breadcrumb_label, label, 70, 10, font_25, OSD_COLOR_WHITE);
    DEBUG_MSG(OSD, DEBUG, "\n");

    // Texto final do caminho de migalhas, em negrito
    lv_point_t pos;
    lv_text_get_size(&pos, label.c_str(), font_25, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    update_or_create_label(m_breadcrumb_box, &m_breadcrumb_last, last, 70 + pos.x, 10, font_semi_25, OSD_COLOR_WHITE);
    DEBUG_MSG(OSD, DEBUG, last << "\n");
    DEBUG_MSG(OSD, DEBUG, "pos.x: " << pos.x << "\n\n");
}

void Osd_Breadcrumb::update(std::string_view _name)
{
    if (!m_breadcrumb_box)
    {
        DEBUG_MSG(OSD, ERROR, "breadcrumb não iniciado!\n");
        return;
    }

    if (m_names.size() > 0)
    {
        m_names.pop_back();
    }
    m_names.emplace_back(_name);
    draw();
}

void Osd_Breadcrumb::remove_name()
{
    if (m_names.size() > 0)
    {
        m_names.pop_back();
    }
    draw();
}

void Osd_Breadcrumb::remove_name(bool redraw)
{
    if (m_names.size() > 0)
    {
        m_names.pop_back();
    }
    if (redraw)
    {
        draw();
    }
}

void Osd_Breadcrumb::add_name(std::string_view _name)
{
    m_names.emplace_back(_name);
    draw();
}

void Osd_Breadcrumb::add_name(std::string_view _name, bool redraw)
{
    m_names.emplace_back(_name);
    if (redraw)
    {
        draw();
    }
}

void Osd_Breadcrumb::add_name(std::vector<std::string_view> _name_list)
{
    for (const auto &name : _name_list)
    {
        m_names.emplace_back(name);
    }
    draw();
}

void Osd_Breadcrumb::replace_last_name(std::string_view _new_name)
{
    if (m_names.size() > 0)
    {
        m_names.pop_back();
    }
    m_names.emplace_back(_new_name);
    draw();
}

bool Osd_Breadcrumb::is_empty()
{
    return m_names.empty();
}

void Osd_Breadcrumb::clear()
{
    m_names.clear();
    DELETE_OBJ(m_breadcrumb_label);
    DELETE_OBJ(m_breadcrumb_last);
    DELETE_OBJ(m_breadcrumb_logo);
    DELETE_OBJ(m_breadcrumb_box);
}

void Osd_Breadcrumb::debug_print() const
{
    DEBUG_MSG(OSD, DEBUG, "===== Breadcrumb Debug =====\n");

    if (m_names.empty())
    {
        DEBUG_MSG(OSD, DEBUG, "Lista vazia\n");
        return;
    }

    int index = 0;
    for (const auto& name : m_names)
    {
        DEBUG_MSG(OSD, DEBUG, "[" << index++ << "] " << name << "\n");
    }

    DEBUG_MSG(OSD, DEBUG, "Total itens: " << m_names.size() << "\n");
    DEBUG_MSG(OSD, DEBUG, "============================\n");
}


} // namespace mb

