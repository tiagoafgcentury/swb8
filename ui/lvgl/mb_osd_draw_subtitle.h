#pragma once

#include "common/mb_globals.h"
#include "common/mb_types.h"
#include "mb_events.h"
#include "mb_osd.h"

#include <memory>
#include <map>
#include <queue>

/* List of different SEGMENT TYPES */
/* According to EN 300-743, table 2 */
#define DVBSUB_ST_PAGE_COMPOSITION      0x10
#define DVBSUB_ST_REGION_COMPOSITION    0x11
#define DVBSUB_ST_CLUT_DEFINITION       0x12
#define DVBSUB_ST_OBJECT_DATA           0x13
#define DVBSUB_ST_DISPLAY_DEFINITION    0x14
//#define DVBSUB_ST_ALTERNATE_CLUT        0x16
#define DVBSUB_ST_ENDOFDISPLAY          0x80
#define DVBSUB_ST_STUFFING              0xff
/* List of different OBJECT TYPES */
/* According to EN 300-743, table 6 */
#define DVBSUB_OT_BASIC_BITMAP          0x00
#define DVBSUB_OT_BASIC_CHAR            0x01
#define DVBSUB_OT_COMPOSITE_STRING      0x02
/* Pixel DATA TYPES */
/* According to EN 300-743, table 9 */
#define DVBSUB_DT_2BP_CODE_STRING       0x10
#define DVBSUB_DT_4BP_CODE_STRING       0x11
#define DVBSUB_DT_8BP_CODE_STRING       0x12
#define DVBSUB_DT_24_TABLE_DATA         0x20
#define DVBSUB_DT_28_TABLE_DATA         0x21
#define DVBSUB_DT_48_TABLE_DATA         0x22
#define DVBSUB_DT_END_LINE              0xf0
/* List of different Page Composition Segment state */
/* According to EN 300-743, 7.2.1 table 3 */
#define DVBSUB_PCS_STATE_ACQUISITION    0x01
#define DVBSUB_PCS_STATE_CHANGE         0x02
/* According to EN 300-743, 7.2.8 table 33 */
#define DVBSUB_ST_BITDEPTH_8BIT         0x00
#define DVBSUB_ST_BITDEPTH_10BIT        0x01
/* According to EN 300-743, 7.2.8 table 34 */
#define DVBSUB_ST_COLORIMETRY_CDS       -1
#define DVBSUB_ST_COLORIMETRY_SDR_709   0x00
#define DVBSUB_ST_COLORIMETRY_SDR_2020  0x01
#define DVBSUB_ST_COLORIMETRY_HDR_PQ    0x02
#define DVBSUB_ST_COLORIMETRY_HDR_HLG   0x03

#define bs_align_0(s) bs_write_align(s, 0)
#define bs_align_1(s) bs_write_align(s, 1)

namespace mb {

class Bitstream
{

public:

    // Construtor default
    Bitstream()
        : m_data(nullptr),
          m_size(0),
          m_bytePos(0),
          m_bitPos(0)
    {}

    // Construtor com buffer
    Bitstream(const uint8_t* data, size_t size)
        : m_data(data),
          m_size(size),
          m_bytePos(0),
          m_bitPos(0)
    {}

    // Proíbe cópia
    Bitstream(const Bitstream&) = delete;
    Bitstream& operator=(const Bitstream&) = delete;

    // Permite move
    Bitstream(Bitstream&&) noexcept = default;
    Bitstream& operator=(Bitstream&&) noexcept = default;


    //Bitstream() : m_data(nullptr), m_size(0), m_bitPos(0) {}

    //Bitstream(const uint8_t* data, size_t size)
    //    : m_data(data), m_size(size), m_bytePos(0), m_bitPos(0) {}

    bool eof() const
    {
        return m_bytePos >= m_size;
    }

    size_t bitpos() const
    {
        return m_bytePos * 8 + m_bitPos;
    }

    void skip(size_t bits)
    {
        m_bytePos += (m_bitPos + bits) / 8;
        m_bitPos   = (m_bitPos + bits) % 8;

        if (m_bytePos > m_size)
            m_bytePos = m_size, m_bitPos = 0;
    }

    uint32_t readBits(uint8_t count)
    {
        uint32_t result = 0;
        for (uint8_t i = 0; i < count; ++i)
        {
            if (eof()) return result;

            result <<= 1;
            result |= read1();
        }

        return result;
    }

    uint32_t read1()
    {
        if (eof())
        {
            return 0;
        }

        uint8_t bit = (m_data[m_bytePos] >> (7 - m_bitPos)) & 1;

        if (++m_bitPos == 8) {
            m_bitPos = 0;
            ++m_bytePos;
        }

        return bit;
    }

    void align()
    {
        if (m_bitPos != 0)
        {
            m_bitPos = 0;
            m_bytePos++;
        }
    }

    // Exponential-Golomb (UE)
    uint_fast32_t readUE()
    {
        unsigned zeros = 0;

        while (!eof() && read1() == 0)
        {
            zeros++;
        }

        if (zeros == 0)
            return 0;

        uint32_t suffix = readBits(zeros);
        return (1u << zeros) - 1 + suffix;
    }

    // Exponential-Golomb signed (SE)
    int_fast32_t readSE()
    {
        uint_fast32_t ue = readUE();
        return (ue & 1) ? (ue + 1) / 2 : -int_fast32_t(ue / 2);
    }

    const uint8_t* data() const
    {
        return m_data;
    }

    size_t size() const
    {
        return m_size;
    }

    size_t bytePos() const
    {
        return m_bytePos;
    }

    uint8_t bitPosInsideByte() const
    {
        return m_bitPos;
    }

private:
    const uint8_t* m_data;
    size_t m_size;
    size_t m_bytePos;
    uint8_t m_bitPos;  // 0..7 (bit dentro do byte)
};

class OSD_Draw_Subtitle: public OSD
{

private:

enum video_color_range_t
{
    COLOR_RANGE_UNDEF,
    COLOR_RANGE_FULL,
    COLOR_RANGE_LIMITED,
#define COLOR_RANGE_STUDIO COLOR_RANGE_LIMITED
#define COLOR_RANGE_MAX    COLOR_RANGE_LIMITED
};

/* The object definition gives the position of the object in a region [7.2.5] */
struct dvbsub_objectdef_t
{
    int id        = 0;
    int type      = 0;
    int x         = 0;
    int y         = 0;
    int fg_pc     = 0;
    int bg_pc     = 0;

    std::string psz_text;
};

struct RGBA
{
    uint8_t r  = 0;
    uint8_t g  = 0;
    uint8_t b  = 0;
    uint8_t a  = 0;
};

/* The entry in the palette CLUT */
struct dvbsub_color_t
{
    uint8_t r  = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a  = 0;
};

/* The displays dimensions [7.2.1] */
struct dvbsub_display_t
{
    uint8_t  id           = 0;
    uint8_t  version      = 0;

    uint16_t width_minus1  = 0;
    uint16_t height_minus1 = 0;

    bool     b_windowed      = false;

    int x      = 0;
    int y      = 0;
    int max_x  = 0;
    int max_y  = 0;
};

/* [7.2.4] */
struct dvbsub_clut_t
{
    uint8_t id = 0;
    uint8_t version = 0;

    std::array<dvbsub_color_t, 4>  c2b {};
    std::array<dvbsub_color_t, 16> c4b {};
    std::array<dvbsub_color_t, 256> c8b {};

    video_color_range_t color_range {};
    int dynamic_range_and_colour_gamut = 0;

    dvbsub_clut_t() = default;

    dvbsub_clut_t(const dvbsub_clut_t&) = delete;
    dvbsub_clut_t& operator=(const dvbsub_clut_t&) = delete;

    dvbsub_clut_t(dvbsub_clut_t&&) noexcept = default;
    dvbsub_clut_t& operator=(dvbsub_clut_t&&) noexcept = default;
};


/* The Region is an area on the image [7.2.3]
 * with a list of the object definitions associated and a CLUT */
struct dvbsub_region_t
{
    int id          = 0;
    int version     = 0;
    int x           = 0;
    int y           = 0;
    int width       = 0;
    int height      = 0;
    int level_comp  = 0;
    int depth       = 0;
    int clut        = 0;

    // antes: dvbsub_objectdef_t *p_object_defs
    std::vector<dvbsub_objectdef_t> p_object_defs;

};

/* The object definition gives the position of the object in a region */
struct dvbsub_regiondef_t
{
    int id = 0;
    int x  = 0;
    int y  = 0;
};


/* The page defines the list of regions [7.2.2] */
struct dvbsub_page_t
{
    int id       = 0;
    int timeout  = 0;   // segundos
    int state    = 0;
    int version  = 0;

    std::vector<dvbsub_regiondef_t> p_region_defs;
};

struct decoder_sys_t
{

    decoder_sys_t() = default;

    // proíbe cópia
    decoder_sys_t(const decoder_sys_t&) = delete;
    decoder_sys_t& operator=(const decoder_sys_t&) = delete;

    // permite move (SEM noexcept)
    decoder_sys_t(decoder_sys_t&&) = default;
    decoder_sys_t& operator=(decoder_sys_t&&) = default;

    Bitstream bs;

    int id               = 0;
    int ancillary_id     = 0;
    uint64_t pts         = 0;

    int spu_position     = 0;
    int spu_x            = 0;
    int spu_y            = 0;

    bool b_page            = false;

    // buffer de pixels (antes uint8_t*)
    std::vector<uint8_t> p_pixbuf;

    std::vector<std::unique_ptr<dvbsub_page_t>> p_page;

    std::vector<std::unique_ptr<dvbsub_region_t>> p_regions;

    std::vector<std::unique_ptr<dvbsub_clut_t>> p_cluts;

    // Mantidos como objetos diretos
    dvbsub_display_t display {};
    dvbsub_clut_t    default_clut {};
};

struct ByteSpan
{
    uint8_t* data;
    size_t size;

    uint8_t& operator[](size_t i) { return data[i]; }
    const uint8_t& operator[](size_t i) const { return data[i]; }

    uint8_t* begin() { return data; }
    uint8_t* end() { return data + size; }

    ByteSpan subspan(size_t off, size_t count = SIZE_MAX) const {
        if (off > size) return { data, 0 };
        size_t new_size = std::min(size - off, count);
        return ByteSpan{ data + off, new_size };
    }
};

struct PendingSubtitlePage
{
    uint64_t pts = 0;
    std::unique_ptr<decoder_sys_t> page;

    PendingSubtitlePage() = default;

    PendingSubtitlePage(PendingSubtitlePage&&) = default;
    PendingSubtitlePage& operator=(PendingSubtitlePage&&) = default;

    PendingSubtitlePage(const PendingSubtitlePage&) = delete;
    PendingSubtitlePage& operator=(const PendingSubtitlePage&) = delete;
};


    // parsing
    void decode_segment(decoder_sys_t *p_sys);
    void decode_page_composition(decoder_sys_t *p_sys, uint16_t );
    void decode_region_composition(decoder_sys_t *p_sys, uint16_t );
    void decode_object(decoder_sys_t *p_sys, uint16_t );
    void decode_display_definition(decoder_sys_t *p_sys, uint16_t );
    void decode_clut(decoder_sys_t *p_sys, uint16_t length);
    void free_all(decoder_sys_t *p_sys);

    void dvbsub_render_pdata( decoder_sys_t *, dvbsub_region_t *, int, int, uint8_t *, int );
    void dvbsub_pdata2bpp(Bitstream& bs, ByteSpan p, int width, int& offset);
    void dvbsub_pdata4bpp(Bitstream& bs, ByteSpan p, int width, int& offset);
    void dvbsub_pdata8bpp(Bitstream& bs, ByteSpan p, int width, int& offset);
    void show_image(decoder_sys_t *p_sys);
    void queue_page_for_presentation(decoder_sys_t&& p_sys);

    dvbsub_color_t ycbcr_to_rgba(int y, int cb, int cr, int t);

    static constexpr auto width = 720;
    static constexpr auto height = 576;

    lv_obj_t *m_main_box { nullptr };
    lv_obj_t* m_parent { nullptr };
    lv_obj_t* m_img_obj { nullptr };
    std::unique_ptr<uint8_t[]> m_buf;
    lv_timer_t *m_hide_timer { nullptr };
    lv_timer_t *m_pts_timer { nullptr };
    uint32_t m_hide_delay_ms = 5000;
    static void hide_timer_callback(lv_timer_t *);
    uint32_t m_pts_delay_ms = 100;
    static void pts_timer_callback(_lv_timer_t *);

    std::queue<PendingSubtitlePage> m_pending_pages;
    uint64_t m_pts_stc_offset = 0;
    bool m_pts_base_valid = false;

    std::vector<uint32_t> m_pixels;
    lv_img_dsc_t m_dsc {};

public:
    OSD_Draw_Subtitle(OSD *_parent);
    virtual ~OSD_Draw_Subtitle();

    void create_subtitle();
    void show_subtitle(const Event_Subtitle_Image &_event);
    void render(const Event_Subtitle_Image& img, lv_coord_t x = 0, lv_coord_t y = 0);
    void clear();
    void presentation_subtitle_osd();

};

} // namespace mb
