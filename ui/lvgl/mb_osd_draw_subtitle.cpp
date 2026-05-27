// mb_osd_draw_subtitle.cpp
#include "mb_osd_draw_subtitle.h"
#include "common/mb_globals.h"
#include "mb_menu_resources.h"
#include "mb_osd_fonts.h"
#include "tasks/mb_task.h"
#include "tasks/mb_task_player.h"
#include <unordered_set>


#define ARGB(a,r,g,b) (((unsigned)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))

namespace mb {

OSD_Draw_Subtitle::OSD_Draw_Subtitle(OSD *_parent)
    : OSD(_parent)
{
}

OSD_Draw_Subtitle::~OSD_Draw_Subtitle()
{
    DELETE_OBJ(m_main_box);
    DELETE_TIMER(m_hide_timer);
    DELETE_TIMER(m_pts_timer);
}

void OSD_Draw_Subtitle::create_subtitle()
{
    m_main_box = create_rect(
        get_main_screen(OSD_Layer::CLOSED_CAPTION_LAYER),
        0, 0,
        width, height,
        OSD_COLOR_BLACK
    );

    lv_obj_null_on_delete(&m_main_box);
    lv_obj_set_style_bg_opa(m_main_box, LV_OPA_TRANSP, 0);
    lv_obj_center(m_main_box);

    m_img_obj = lv_img_create(m_main_box);
    lv_obj_null_on_delete(&m_img_obj);
    lv_obj_add_flag(m_img_obj, LV_OBJ_FLAG_HIDDEN);
    m_hide_timer = lv_timer_create(hide_timer_callback, m_hide_delay_ms, this);
    lv_timer_set_repeat_count(m_hide_timer, -1);

    m_pts_timer = lv_timer_create(pts_timer_callback, m_pts_delay_ms, this);
    lv_timer_set_repeat_count(m_pts_timer, -1);

}

void OSD_Draw_Subtitle::pts_timer_callback(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Draw_Subtitle *>(lv_timer_get_user_data(_timer));
    thiz->presentation_subtitle_osd();
}

void OSD_Draw_Subtitle::presentation_subtitle_osd()
{
    //uint64_t stc = Task::s_task_player->get_stc();

    if (!m_pending_pages.empty())
    {
        auto &pending = m_pending_pages.front();

        #warning REVER O CALCULO DO PTS PARA EXIBICAO DO SUBTITLE
        //if (((stc - pending.pts) & ((1ULL << 33) - 1)) < (1ULL << 32))
        {
            show_image(pending.page.get());
            m_pending_pages.pop();
        }
    }
}

void OSD_Draw_Subtitle::hide_timer_callback(lv_timer_t *_timer)
{
    auto thiz = static_cast<OSD_Draw_Subtitle *>(lv_timer_get_user_data(_timer));
    thiz->clear();
}

void OSD_Draw_Subtitle::clear()
{
    if (m_img_obj)
    {
        lv_obj_add_flag(m_img_obj, LV_OBJ_FLAG_HIDDEN);
    }

    m_buf.reset();
    std::memset(&m_dsc, 0, sizeof(m_dsc));
}

void OSD_Draw_Subtitle::show_subtitle(const Event_Subtitle_Image &_event)
{
    if (_event.data.size() <= 0)
    {
        clear();
        return;
    }

    decoder_sys_t p_sys = {};

    const uint8_t* data = _event.data.data();
    size_t size = _event.data.size();

    if (size < 10)
    {
        return;
    }

    const uint8_t* payload_ptr = data + 10;
    size_t payload_len = size - 10;

    // Inicializa bitstream
    p_sys.bs = Bitstream(payload_ptr, payload_len);

    p_sys.pts = _event.pts;
    // Deve começar com 0x20
    if (p_sys.bs.readBits(8) != 0x20)
    {
        DEBUG("\ninvalid data identifier\n");
        return;
    }

    // subtitle stream id deve ser 0
    if (p_sys.bs.readBits(8) != 0)
    {
        DEBUG("\ninvalid subtitle stream id\n");
        return;
    }

    p_sys.b_page = false;

    if (payload_len < 3)
    {
        return;
    }

    // Processa todos os segmentos até encontrar end marker
    uint8_t i_sync_byte = p_sys.bs.readBits(8);

    while (i_sync_byte == 0x0F) // Sync
    {
        decode_segment(&p_sys);
        i_sync_byte = p_sys.bs.readBits(8);
    }

    // Deve terminar com 0x3F
    if ((i_sync_byte & 0x3F) != 0x3F)
    {
        DEBUG("\nend marker not found (corrupted subtitle ?)\n");
        return;
    }

    /* Check if the page is to be displayed */
    if( p_sys.p_page.size() > 0 && p_sys.b_page )
    {
        queue_page_for_presentation(std::move(p_sys));
        //show_image(&p_sys);
    }
}

void OSD_Draw_Subtitle::queue_page_for_presentation(decoder_sys_t&& p_sys)
{
    if (!m_pts_base_valid)
    {
        uint64_t stc = Task::s_task_player->get_stc();

        // Alinha PTS com STC
        m_pts_stc_offset =
            (p_sys.pts - stc) & ((1ULL << 33) - 1);

        m_pts_base_valid = true;
    }

    PendingSubtitlePage p;
    p.pts  = p_sys.pts;
    p.page = std::make_unique<decoder_sys_t>(std::move(p_sys));
    m_pending_pages.push(std::move(p));
}



OSD_Draw_Subtitle::dvbsub_color_t OSD_Draw_Subtitle::ycbcr_to_rgba(int y, int cr, int cb, int t)
{
    y -= 16;
    cb -= 128;
    cr -= 128;

    double r_val = 1.164 * y + 1.596 * cr;
    double g_val = 1.164 * y - 0.392 * cb - 0.813 * cr;
    double b_val = 1.164 * y + 2.017 * cb;

    dvbsub_color_t px;
    px.r = static_cast<uint8_t>(std::max(0, std::min(255, (int)r_val)));
    px.g = static_cast<uint8_t>(std::max(0, std::min(255, (int)g_val)));
    px.b = static_cast<uint8_t>(std::max(0, std::min(255, (int)b_val)));

    // Alpha is already inverted by the caller (255 - t)
    // Just pass through the value as-is
    px.a = static_cast<uint8_t>(std::max(0, std::min(255, t)));

    return px;
}

void OSD_Draw_Subtitle::show_image(decoder_sys_t *p_sys)
{

    uint16_t img_width = 0;
    uint16_t img_height = 0;

    for (auto& rptr : p_sys->p_regions)
    {
        auto* r = rptr.get();

        dvbsub_clut_t *active_clut = nullptr;

        for (auto& uptr : p_sys->p_cluts)
        {
            if (uptr && uptr->id == r->clut)
            {
                active_clut = uptr.get();
                break;
            }
        }

        if (!active_clut)
        {
            active_clut = &p_sys->default_clut; // fallback
        }

        img_width = r->width;
        img_height = r->height;
        auto compute_size = (r->width * r->height);
        m_pixels.resize(compute_size);

        for (int i = 0; i < compute_size; i++)
        {
            int color_idx = p_sys->p_pixbuf[i];
            std::array<dvbsub_color_t, 256> clut {};
            const dvbsub_clut_t* src = active_clut;
            switch (r->depth)
            {
                case 1: // 2bpp
                    std::copy(src->c2b.begin(), src->c2b.end(), clut.begin());
                    break;

                case 2: // 4bpp
                    std::copy(src->c4b.begin(), src->c4b.end(), clut.begin());
                    break;

                case 3: // 8bpp
                    clut = src->c8b;
                    break;
            }

            if (clut.size() > 0)
            {
                RGBA rgba = {clut[color_idx].r, clut[color_idx].g, clut[color_idx].b, clut[color_idx].a};
                m_pixels[i] = 0;
                m_pixels[i] |= rgba.a << 24;
                m_pixels[i] |= rgba.r << 16;
                m_pixels[i] |= rgba.g << 8;
                m_pixels[i] |= rgba.b << 0;
            }
            else
            {
                m_pixels[i] = 0x808080FF;
            }
        }
    }

    if (p_sys->p_page.empty())
    {
        DEBUG("Nenhuma página DVB disponível\n");
        return;
    }

    const auto& page = p_sys->p_page.front();
    if (!page)
    {
        DEBUG("Página inválida\n");
        return;
    }

    DEBUG("Page Version: " << static_cast<int>(page->version) << ", Timeout: " << static_cast<int>(page->timeout) << "s\n");

    int x = 0;
    int y = 0;

    if (!page->p_region_defs.empty())
    {
        for (const auto& region_def : page->p_region_defs)
        {

            DEBUG(" Region ID: " << static_cast<int>(region_def.id)
                << ", X: " << region_def.x
                << ", Y: " << region_def.y << "\n");
        }

        // Usa a primeira região como referência
        x = page->p_region_defs.front().x;
        y = page->p_region_defs.front().y;
    }
    else
    {
        DEBUG("Página não possui regiões\n");
    }

    /* Reinicia timer de ocultar caso tenha ocorrido alteração */
    const uint32_t new_hide_delay_ms = page->timeout * 1000;

    if (m_hide_delay_ms != new_hide_delay_ms)
    {
        DEBUG("Tempo de ocultar legenda de " << m_hide_delay_ms
            << "ms para " << new_hide_delay_ms << "ms\n");

        m_hide_delay_ms = new_hide_delay_ms;
        lv_timer_pause(m_hide_timer);
        lv_timer_set_period(m_hide_timer, m_hide_delay_ms);
        lv_timer_reset(m_hide_timer);
        lv_timer_resume(m_hide_timer);
    }

    lv_obj_set_pos(m_img_obj, x, y);
    lv_obj_remove_flag(m_img_obj, LV_OBJ_FLAG_HIDDEN);
    lv_timer_reset(m_hide_timer);

    if (img_width == 0 || img_height == 0 || m_pixels.empty())
    {
        lv_obj_add_flag(m_img_obj, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    m_dsc.header.w = img_width;
    m_dsc.header.h = img_height;
    m_dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
    m_dsc.data_size = (img_width * img_height * 4);
    m_dsc.data = reinterpret_cast<const uint8_t*>(m_pixels.data());

    DEBUG("Image description: W=" << img_width <<
          ", H=" << img_height <<
          ", Color Format=" << m_dsc.header.cf <<
          ", Size=" << m_dsc.data_size << "\n");

    lv_img_set_src(m_img_obj, &m_dsc);
    lv_obj_remove_flag(m_img_obj, LV_OBJ_FLAG_HIDDEN);
    lv_timer_reset(m_hide_timer);

}

void OSD_Draw_Subtitle::decode_segment(decoder_sys_t *p_sys)
{

    // Leitura do tipo, id da página e tamanho do segmento
    const uint8_t  type    = p_sys->bs.readBits(8);
    const uint16_t page_id = p_sys->bs.readBits(16);
    const uint16_t size    = p_sys->bs.readBits(16);

    // Verifica se o segmento pertence a uma das páginas válidas
    if (page_id != 0x02 && page_id != 0x02)
    {
        p_sys->bs.skip(size * 8);
        return;
    }

    // Processa o tipo de segmento
    switch (type)
    {
        case DVBSUB_ST_PAGE_COMPOSITION:
            decode_page_composition(p_sys, size);
            break;

        case DVBSUB_ST_REGION_COMPOSITION:
            decode_region_composition(p_sys, size);
            break;

        case DVBSUB_ST_CLUT_DEFINITION:
            decode_clut(p_sys, size);
            break;

        case DVBSUB_ST_OBJECT_DATA:
            decode_object(p_sys, size);
            break;

        case DVBSUB_ST_DISPLAY_DEFINITION:
            decode_display_definition(p_sys, size);
            break;

        //case DVBSUB_ST_ALTERNATE_CLUT:
            //alternative_CLUT(p_sys, size);
            //break;

        case DVBSUB_ST_ENDOFDISPLAY:
            p_sys->bs.skip(size * 8);
            break;

        case DVBSUB_ST_STUFFING:
            p_sys->bs.skip(size * 8);
            break;

        default:
            p_sys->bs.skip(size * 8);
            break;
    }
}

void OSD_Draw_Subtitle::free_all(decoder_sys_t *p_sys)
{
    /*free( p_sys->p_display ); No longer malloced */
    p_sys->p_cluts.clear();
    p_sys->p_regions.clear();
    p_sys->p_page.clear();
}

void OSD_Draw_Subtitle::decode_clut(decoder_sys_t *p_sys, uint16_t segment_length)
{
    uint16_t processed_length = 0;
    int id, version;

    id      = p_sys->bs.readBits(8);
    version = p_sys->bs.readBits(4);

    /* Procura CLUT existente */
    dvbsub_clut_t* p_clut = nullptr;

    for (auto& uptr : p_sys->p_cluts)
    {
        if (uptr && uptr->id == id)
        {
            p_clut = uptr.get();
            break;
        }
    }

    /* Se versão já conhecida, apenas descarta o segmento */
    if (p_clut && p_clut->version == version)
    {
        p_sys->bs.skip(8 * segment_length - 12);
        return;
    }

    /* Cria CLUT se não existir */
    if (!p_clut)
    {
        auto new_clut = std::make_unique<dvbsub_clut_t>();
        p_clut = new_clut.get();
        p_sys->p_cluts.push_back(std::move(new_clut));
    }

    /* === RESET EXPLÍCITO PARA O DEFAULT (SEM COPY ASSIGNMENT) === */
    p_clut->version = version;
    p_clut->id      = id;

    p_clut->color_range = p_sys->default_clut.color_range;
    p_clut->dynamic_range_and_colour_gamut =
        p_sys->default_clut.dynamic_range_and_colour_gamut;

    p_clut->c2b = p_sys->default_clut.c2b;
    p_clut->c4b = p_sys->default_clut.c4b;
    p_clut->c8b = p_sys->default_clut.c8b;

    /* Reserved bits */
    p_sys->bs.skip(4);
    processed_length = 2;

    /* Parse CLUT entries */
    while (processed_length < segment_length)
    {
        uint8_t y, cb, cr, t;
        uint_fast8_t cid  = p_sys->bs.readBits(8);
        uint_fast8_t type = p_sys->bs.readBits(3);

        p_sys->bs.skip(4);

        p_clut->color_range =
            p_sys->bs.readBits(1) ? COLOR_RANGE_FULL : COLOR_RANGE_LIMITED;

        if (p_clut->color_range == COLOR_RANGE_FULL)
        {
            y  = p_sys->bs.readBits(8);
            cr = p_sys->bs.readBits(8);
            cb = p_sys->bs.readBits(8);
            t  = p_sys->bs.readBits(8);
            processed_length += 6;
        }
        else
        {
            y  = p_sys->bs.readBits(6) << 2;
            cr = p_sys->bs.readBits(4) << 4;
            cb = p_sys->bs.readBits(4) << 4;
            t  = p_sys->bs.readBits(2) << 6;
            processed_length += 4;
        }

        /* Transparência total */
        if (y == 0)
        {
            cr = cb = 0;
            t  = 0xff;
        }

        /* Atualiza entradas conforme o tipo */
        if ((type & 0x04) && (cid < 4))
            p_clut->c2b[cid] = ycbcr_to_rgba(y, cr, cb, 255 - t);

        if ((type & 0x02) && (cid < 16))
            p_clut->c4b[cid] = ycbcr_to_rgba(y, cr, cb, 255 - t);

        if (type & 0x01)
            p_clut->c8b[cid] = ycbcr_to_rgba(y, cr, cb, 255 - t);
    }
}

void OSD_Draw_Subtitle::decode_page_composition(decoder_sys_t *p_sys, uint16_t segment_len)
{
    const uint8_t timeout = p_sys->bs.readBits(8);
    const uint8_t version = p_sys->bs.readBits(4);
    const uint8_t state   = p_sys->bs.readBits(2);

    p_sys->bs.skip(2);  // reserved

    // Obter página atual (se existir)
    dvbsub_page_t* page =
        p_sys->p_page.empty() ? nullptr : p_sys->p_page.back().get();

    // ============================
    // Estado especial: PAGE_CHANGE
    // ============================
    if (state == DVBSUB_PCS_STATE_CHANGE)
    {
        free_all(p_sys);
        page = nullptr;
    }
    else if (state != DVBSUB_PCS_STATE_ACQUISITION &&
             state != DVBSUB_PCS_STATE_CHANGE)
    {
        //DEBUG("\n didn't receive an acquisition page yet\n");
        return;
    }

    // ============================
    // Checagem de versão
    // ============================
    if (page && page->version == version)
    {
        p_sys->bs.skip(8 * (segment_len - 2));
        return;
    }
    else if (page)
    {
        // limpar regiões
        page->p_region_defs.clear();
    }

    // ============================
    // Criar nova página se necessário
    // ============================
    if (!page)
    {
        auto new_page = std::make_unique<dvbsub_page_t>();
        p_sys->p_page.push_back(std::move(new_page));
        page = p_sys->p_page.back().get();
    }

    page->version = version;
    page->timeout = timeout;
    p_sys->b_page = true;

    // ============================
    // Número de regiões
    // ============================
    const int n_regions = (segment_len - 2) / 6;

    if (n_regions == 0)
    {
        return;
    }

    page->p_region_defs.resize(n_regions);

    for (int i = 0; i < n_regions; i++)
    {
        auto& reg = page->p_region_defs[i];

        reg.id = p_sys->bs.readBits(8);
        p_sys->bs.skip(8); // reserved
        reg.x  = p_sys->bs.readBits(16);
        reg.y  = p_sys->bs.readBits(16);
    }
}

void OSD_Draw_Subtitle::decode_region_composition(decoder_sys_t* p_sys, uint16_t seg_len)
{
    const int region_id     = p_sys->bs.readBits(8);
    const int version       = p_sys->bs.readBits(4);

    dvbsub_region_t* region = nullptr;
    for (auto& r : p_sys->p_regions)
    {
        if (r->id == region_id)
        {
            region = r.get();
            break;
        }
    }

    if (region && region->version == version)
    {
        p_sys->bs.skip(8 * (seg_len - 1) - 4);
        return;
    }

    if (!region)
    {
        auto new_region = std::make_unique<dvbsub_region_t>();
        new_region->id = region_id;
        new_region->version = version;

        p_sys->p_regions.push_back(std::move(new_region));
        region = p_sys->p_regions.back().get();
    }

    // ---- Atributos da região ----
    region->id      = region_id;
    region->version = version;

    const bool fill_bg = p_sys->bs.readBits(1);
    p_sys->bs.skip(3);

    const int width  = p_sys->bs.readBits(16);
    const int height = p_sys->bs.readBits(16);

    const int level_comp = p_sys->bs.readBits(3);
    const int depth      = p_sys->bs.readBits(3);
    p_sys->bs.skip(2);

    const int clut = p_sys->bs.readBits(8);

    const int bg_8 = p_sys->bs.readBits(8);
    const int bg_4 = p_sys->bs.readBits(4);
    const int bg_2 = p_sys->bs.readBits(2);
    p_sys->bs.skip(2);

    // Limpar lista antiga de objetos
    region->p_object_defs.clear();

    // Se tamanho mudou → alocar buffer novo
    if (region->width != width || region->height != height)
    {
        p_sys->p_pixbuf.assign(width * height, 0);
    }

    // Preencher fundo quando necessário
    if (fill_bg)
    {
        const uint8_t bg =
            (depth == 1) ? bg_2 :
            (depth == 2) ? bg_4 :
                           bg_8;

        std::fill(p_sys->p_pixbuf.begin(), p_sys->p_pixbuf.end(), bg);
    }

    // Atualizar parâmetros
    region->width      = width;
    region->height     = height;
    region->level_comp = level_comp;
    region->depth      = depth;
    region->clut       = clut;

    // ---- Ler lista de objetos ----
    int processed = 10;   // bytes já processados

    while (processed < seg_len)
    {
        dvbsub_objectdef_t obj;

        obj.id   = p_sys->bs.readBits(16);
        obj.type = p_sys->bs.readBits(2);

        p_sys->bs.skip(2);            // provider
        obj.x = p_sys->bs.readBits(12);
        p_sys->bs.skip(4);
        obj.y = p_sys->bs.readBits(12);

        processed += 6;

        if (obj.type == DVBSUB_OT_BASIC_CHAR ||
            obj.type == DVBSUB_OT_COMPOSITE_STRING)
        {
            obj.fg_pc = p_sys->bs.readBits(8);
            obj.bg_pc = p_sys->bs.readBits(8);
            processed += 2;
        }
        region->p_object_defs.push_back(std::move(obj));
    }
}

void OSD_Draw_Subtitle::decode_display_definition(decoder_sys_t* p_sys, uint16_t seg_len)
{
    const uint16_t segment_bits = seg_len * 8;
    uint16_t processed_bits = 4;   // já lemos 4 bits da versão

    // ------------------------------------------------------
    // Lê versão
    // ------------------------------------------------------
    const int version = p_sys->bs.readBits(4);

    // Se não mudou, ignore segmento inteiro
    if (p_sys->display.version == version)
    {
        p_sys->bs.skip(segment_bits - 4);
        return;
    }

    p_sys->display.version = version;

    // ------------------------------------------------------
    // Lê parâmetros principais
    // ------------------------------------------------------
    p_sys->display.b_windowed = p_sys->bs.readBits(1);
    p_sys->bs.skip(3);  // reserved
    processed_bits += 4;

    p_sys->display.width_minus1  = p_sys->bs.readBits(16);
    p_sys->display.height_minus1 = p_sys->bs.readBits(16);
    processed_bits += 32;

    // ------------------------------------------------------
    // Se windowed, há coordenadas extras
    // ------------------------------------------------------
    if (p_sys->display.b_windowed)
    {
        p_sys->display.x     = p_sys->bs.readBits(16);
        p_sys->display.max_x = p_sys->bs.readBits(16);
        p_sys->display.y     = p_sys->bs.readBits(16);
        p_sys->display.max_y = p_sys->bs.readBits(16);

        processed_bits += 64;
    }
    else
    {
        // Preenche valores padrão quando não há janela
        p_sys->display.x     = 0;
        p_sys->display.y     = 0;
        p_sys->display.max_x = p_sys->display.width_minus1;
        p_sys->display.max_y = p_sys->display.height_minus1;
    }

    DEBUG("Display Definition: W=" << (p_sys->display.width_minus1 + 1) <<
          ", H=" << (p_sys->display.height_minus1 + 1) <<
          ", Windowed=" << p_sys->display.b_windowed <<
          ", X=" << p_sys->display.x <<
          ", Y=" << p_sys->display.y <<
          ", MaxX=" << p_sys->display.max_x <<
          ", MaxY=" << p_sys->display.max_y << "\n");

    lv_obj_set_size(m_main_box,
                  p_sys->display.width_minus1 + 1,
                  p_sys->display.height_minus1 + 1);
    lv_obj_center(m_main_box);

    // ------------------------------------------------------
    // Verificação final
    // ------------------------------------------------------
    if (processed_bits != segment_bits)
    {
        DEBUG("\n processed length " << (processed_bits / 8) << " bytes != segment length " << seg_len << " bytes n");
    }
}

void OSD_Draw_Subtitle::decode_object(decoder_sys_t *p_sys, uint16_t segment_len)
{

    const uint32_t total_bits = segment_len * 8;
    const int obj_id = p_sys->bs.readBits(16);
    p_sys->bs.skip(4);    // version
    const int coding_method = p_sys->bs.readBits(2);

    if (coding_method > 1)
    {
        DEBUG("\n Unknown DVB object coding " << coding_method << "not supported\n");
        p_sys->bs.skip(total_bits - 22);  // remove payload
        return;
    }

    // ============================
    // Verifica se o objeto aparece em algum region
    // ============================
    dvbsub_region_t* region = nullptr;
    for (auto& rptr : p_sys->p_regions)
    {
        auto* r = rptr.get();

        uint16_t size = static_cast<uint16_t>(r->p_object_defs.size());
        for (auto i = 0; i < size; i++)
        {
            if (r->p_object_defs[i].id == obj_id)
            {
                region = r;
                break;
            }
        }
        if (region) break;
    }

    // Se não aparecer em nenhuma region, descarta
    if (!region)
    {
        p_sys->bs.skip(total_bits - 22);
        return;
    }

    // ============================
    // Flags adicionais
    // ============================
    p_sys->bs.skip(1);  // non_modify_color
    p_sys->bs.skip(1);  // reserved

    // ==========================================================
    // MODO 0: pixel data (run-length coded, top & bottom field)
    // ==========================================================
    if (coding_method == 0)
    {
        const int top_len    = p_sys->bs.readBits(16);
        const int bottom_len = p_sys->bs.readBits(16);

        size_t byte_pos = p_sys->bs.bitpos() / 8;
        const uint8_t* base = p_sys->bs.data();
        uint8_t* top_ptr    = const_cast<uint8_t*>(base + byte_pos);
        uint8_t* bottom_ptr = top_ptr + top_len;

        // total bytes de payload = segment_len - header(2 bytes) - flags(1 byte) - sizes(4 bytes)
        p_sys->bs.skip(total_bits - 56);

        const uint8_t* end = p_sys->bs.data() + p_sys->bs.size();
        // Sanity check
        if (segment_len < (top_len + bottom_len + 7) ||
            (top_ptr + top_len + bottom_len) > end)
        {
            DEBUG("\nCorrupted DVB object pixel data \n");
            return;
        }

        // Render para todas as regiões onde o objeto aparece
        for (auto& rptr : p_sys->p_regions)
        {
            dvbsub_region_t* r = rptr.get();

            uint16_t size = static_cast<uint16_t>(r->p_object_defs.size());
            for (auto i = 0; i < size; i++)
            {
                auto& obj = r->p_object_defs[i];

                if (obj.id != obj_id)
                    continue;

                dvbsub_render_pdata(p_sys, r, obj.x, obj.y, top_ptr, top_len);

                if (bottom_len > 0)
                {
                    dvbsub_render_pdata(p_sys, r, obj.x, obj.y + 1, bottom_ptr, bottom_len);
                }
                else
                {
                    // bottom = cópia do top
                    dvbsub_render_pdata(p_sys, r, obj.x, obj.y + 1, top_ptr, top_len);
                }
            }
        }

        return;
    }
    else
    {
        const int num_codes = p_sys->bs.readBits(8);
        size_t byte_pos = p_sys->bs.bitpos() / 8;
        const uint8_t* base = p_sys->bs.data();
        uint8_t* ptr = const_cast<uint8_t*>(base + byte_pos);
        const uint8_t* end = p_sys->bs.data() + p_sys->bs.size();

        // sanity check
        if (segment_len < (num_codes * 2 + 4) ||
            (ptr + num_codes * 2) > end)
        {
            DEBUG("\nCorrupted DVB object pixel data \n");
            return;
        }

        for (auto& rptr : p_sys->p_regions)
        {
            auto* r = rptr.get();

            for (auto& obj : r->p_object_defs)
            {
                if (obj.id != obj_id)
                    continue;

                // Ajusta tamanho da string
                obj.psz_text.resize(num_codes);

                // Copia os caracteres
                for (int j = 0; j < num_codes; j++)
                {
                    obj.psz_text[j] = static_cast<char>(
                        p_sys->bs.readBits(16) & 0xFF);
                }
            }
        }
    }

}

void OSD_Draw_Subtitle::dvbsub_render_pdata(decoder_sys_t *p_sys, dvbsub_region_t *p_region,
                                              int _x, int _y, uint8_t *p_field, int _field)
{
    // Sanity check
    if (p_sys->p_pixbuf.empty())
    {
        DEBUG("\nregion " << p_region->id <<" has no pixel buffer! \n");
        return;
    }

    if (_x < 0 || _y < 0 ||
        _x >= p_region->width ||
        _y >= p_region->height)
    {
        DEBUG("\ninvalid offset x: " << _x << " y: " << _y << "\n");
        return;
    }

    Bitstream bs = Bitstream(p_field, _field);

    int offset = 0;

    // Ponteiro inicial da linha
    uint8_t* p_pixbuf = p_sys->p_pixbuf.data() + _y * p_region->width;

    while (!bs.eof())
    {
        if (_y >= p_region->height)
            return;

        uint8_t code = bs.readBits(8);

        // Span da linha inteira
        ByteSpan full_row{ p_pixbuf, static_cast<size_t>(p_region->width) };
        ByteSpan row = full_row.subspan(static_cast<size_t>(_x));

        switch (code)
        {
        case 0x10: // 2bpp
            dvbsub_pdata2bpp(bs, row, p_region->width - _x, offset);
            break;

        case 0x11: // 4bpp
            dvbsub_pdata4bpp(bs, row, p_region->width - _x, offset);
            break;

        case 0x12: // 8bpp
            dvbsub_pdata8bpp(bs, row, p_region->width - _x, offset);
            break;

        case 0x20:
        case 0x21:
        case 0x22:
            // Map tables (não usados)
            break;

        case 0xF0: // End of line
            offset = 0;
            _y += 2;

            if (_y >= p_region->height)
                return;

            p_pixbuf = p_sys->p_pixbuf.data() + _y * p_region->width;
            break;
        }
    }
}

void OSD_Draw_Subtitle::dvbsub_pdata2bpp(Bitstream &bs, ByteSpan p, int width, int& offset)
{
    bool stop = false;

    while (!stop && !bs.eof())
    {
        int count = 0;
        int color = bs.readBits(2);

        if (color != 0)
        {
            count = 1;
        }
        else
        {
            if (bs.readBits(1) == 1)  // Switch1
            {
                count = 3 + bs.readBits(3);
                color = bs.readBits(2);
            }
            else
            {
                if (bs.readBits(1) == 0)  // Switch2
                {
                    switch (bs.readBits(2))  // Switch3
                    {
                        case 0x00: stop = true;  break;
                        case 0x01: count = 2;    break;
                        case 0x02:
                            count = 12 + bs.readBits(4);
                            color = bs.readBits(2);
                            break;
                        case 0x03:
                            count = 29 + bs.readBits(8);
                            color = bs.readBits(2);
                            break;
                    }
                }
                else
                {
                    count = 1; // 1 pixel color 0
                }
            }
        }

        if (count == 0) continue;

        if (offset + count > width)
            break;

        ByteSpan dst = p.subspan(offset, count);
        std::fill(dst.begin(), dst.end(), static_cast<uint8_t>(color));

        offset += count;

    }

    bs.align();
}

void OSD_Draw_Subtitle::dvbsub_pdata4bpp(Bitstream &bs, ByteSpan p, int width, int& offset)
{
    bool stop = false;

    while (!stop && !bs.eof())
    {
        int count = 0;
        int color = bs.readBits(4);

        if (color != 0)
        {
            count = 1;
        }
        else
        {
            if (bs.readBits(1) == 0)  // Switch1
            {
                count = bs.readBits(3);
                if (count != 0)
                {
                    count += 2;
                }
                else stop = true;
            }
            else
            {
                if (bs.readBits(1) == 0)  // Switch2
                {
                    count = 4 + bs.readBits(2);
                    color = bs.readBits(4);
                }
                else
                {
                    switch (bs.readBits(2))  // Switch3
                    {
                        case 0x0: count = 1; break;
                        case 0x1: count = 2; break;
                        case 0x2:
                            count = 9 + bs.readBits(4);
                            color = bs.readBits(4);
                            break;
                        case 0x3:
                            count = 25 + bs.readBits(8);
                            color = bs.readBits(4);
                            break;
                    }
                }
            }
        }

        if (count == 0) continue;

        if (offset + count > width)
            break;

        ByteSpan dst = p.subspan(offset, count);
        std::fill(dst.begin(), dst.end(), static_cast<uint8_t>(color));

        offset += count;

    }

    bs.align();
}

void OSD_Draw_Subtitle::dvbsub_pdata8bpp(Bitstream &bs, ByteSpan p, int width, int& offset)
{
    bool stop = false;

    while (!stop && !bs.eof())
    {
        int count = 0;
        int color = bs.readBits(8);

        if (color != 0)
        {
            count = 1;
        }
        else
        {
            if (bs.readBits(1) == 0)  // Switch1
            {
                count = bs.readBits(7);
                if (count == 0) stop = true;
            }
            else
            {
                count = bs.readBits(7);
                color = bs.readBits(8);
            }
        }

        if (count == 0) continue;

        if (offset + count > width)
            break;

        ByteSpan dst = p.subspan(offset, count);
        std::fill(dst.begin(), dst.end(), static_cast<uint8_t>(color));

        offset += count;

        }

    bs.align();
}

} // namespace mb
