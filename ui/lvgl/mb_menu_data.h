#pragma once

#include <array>
#include <vector>
#include <lvgl.h>
#include <string.h>
#include <string>
#include "common/mb_lineup.h"

#define MAX_CHANNEL_INFO_VIEW 4
#define MAX_LINE_LIST_VIEW 15

namespace mb {

struct Sub_Menu_View
{
    Sub_Menu_View(lv_obj_t *_box, lv_obj_t *_rect, lv_obj_t *_channel_name, lv_obj_t *_channel_number):
        box(_box), rect(_rect), channel_name(_channel_name), channel_number(_channel_number)
    {
    }

    uint16_t x = 0;

    lv_obj_t *box { nullptr };
    lv_obj_t *rect { nullptr };
    lv_obj_t *channel_name { nullptr };
    lv_obj_t *channel_number { nullptr };
};

struct Sub_Menu_Data
{
    Sub_Menu_Data(std::string_view _channelName, std::string_view _channelNumber, bool _bFavorite, Service_ID_t _service_id, Transponder_Id _transponder_id):
        channel_name(_channelName), channel_number(_channelNumber), is_favorite(_bFavorite), service_id(_service_id), transponder_id(_transponder_id)
    {
    }

    uint16_t x = 0;

    std::string channel_name;
    std::string channel_number;
    bool is_favorite;
    Service_ID_t service_id;
    Transponder_Id transponder_id;
};

struct Channel_Data
{
    enum Channel_List_Type
    {
        MY_TV_CHANNELS,
        MY_RADIO_CHANNELS,
        ALL_TV_CHANNELS,
        ALL_RADIO_CHANNELS,
        MAX_CHANNEL_TYPE
    };

    std::array<std::vector<Sub_Menu_Data>, MAX_CHANNEL_TYPE> channel_list;

    auto size() const
    {
        return channel_list.size();
    }

    auto size(Channel_List_Type _listType) const
    {
        return channel_list[_listType].size();
    }

    template<typename... _Args>
    void emplace_back(Channel_List_Type listType, _Args &&... __args)
    {
        channel_list[listType].emplace_back(__args...);
    }
};

struct All_Channel_Data
{
    std::vector<Sub_Menu_Data> channel_list;

    auto size() const
    {
        return channel_list.size();
    }

    template<typename... _Args>
    void emplace_back(_Args &&... __args)
    {
        channel_list.emplace_back(__args...);
    }
};

struct Menu_Home
{
    enum Categories
    {
        FAVORITOS,
        REGIONAIS,
        ENTRETENIMENTO,
        NOTICIAS,
        EDUCACAO,
        RELIGIOSO,
        AGRONEGOCIO,
        TV_PUBLICA,
        COMPRAS,
        OUTROS_CANAIS_TV,
        TODOS_CANAIS_TV,
        TODAS_AS_RADIOS,
        MAX_CATEGORIES
    };

    std::array<std::string, MAX_CATEGORIES> category_title_name;
    std::array<lv_obj_t *, MAX_CATEGORIES> menu_title;
    std::array<std::vector<Sub_Menu_Data>, MAX_CATEGORIES> sub_menu_data;
    std::array<uint16_t, MAX_CATEGORIES> sub_menu_count;

    uint16_t number_of_categories_founds;
    uint16_t number_of_services;

    auto size() const
    {
        return sub_menu_data.size();
    }

    auto size(Categories _category) const
    {
        return sub_menu_data[_category].size();
    }

    template<typename... _Args>
    void emplace_back(Categories category, _Args &&... __args)
    {
        sub_menu_data[category].emplace_back(__args...);
    }

    std::vector<Sub_Menu_View> sub_menu_view;

    void emplace_view_back(lv_obj_t *_box, lv_obj_t *_rect, lv_obj_t *_chName, lv_obj_t *_chNumber)
    {
        sub_menu_view.emplace_back(_box, _rect, _chName, _chNumber);
    }
};

struct Menu_Data
{
    Menu_Data(lv_obj_t *_m_sel, lv_obj_t *_img, lv_obj_t *_imgSel, lv_obj_t *_m_lbl):
        sel(_m_sel), img(_img), img_sel(_imgSel), lbl(_m_lbl)
    {
    }

    lv_obj_t *sel { nullptr };
    lv_obj_t *img { nullptr };
    lv_obj_t *img_sel { nullptr };
    lv_obj_t *lbl { nullptr };
};

struct Signal_Data
{
    lv_obj_t *prg_bar_quality{ nullptr };
    lv_obj_t *lbl_quality { nullptr };
    lv_obj_t *prg_bar_strength { nullptr };
    lv_obj_t *lbl_strength { nullptr };
    lv_obj_t *lbl_snr { nullptr };
};

} // namespace mb
