#include "hal/mb_demux.h"
#include "common/mb_globals.h"
#include "common/mb_assert.h"

#include "mb_ali_globals.h"
#include "mb_dvb_subtitle_parser.h"
#include <aui_dmx.h>

#include <atomic>
#include <mutex>
#include <thread>

//#define USE_MEMORY_POOL
#define USE_MALLOC_QUEUE

#ifdef USE_MALLOC_QUEUE
    #include <queue>
#endif

static std::atomic_bool s_read_data { false };

namespace mb {

std::mutex s_global_mutex_lock;

aui_hdl g_ali_demux_hnd;
aui_attr_dmx g_ali_demux_attr;

struct Demux::Data
{
};

struct MemoryPoolEntry
{
    size_t size { 0 };
    uint8_t *data { nullptr };

    MemoryPoolEntry(size_t _size, uint8_t *_data):
        size(_size),
        data(_data)
    {
    }

    MemoryPoolEntry(MemoryPoolEntry &_other) = delete;
    MemoryPoolEntry(MemoryPoolEntry &&_other) = delete;
    MemoryPoolEntry(const MemoryPoolEntry &_other) = delete;

    ~MemoryPoolEntry()
    {
        free(data);
    }

    std::tuple<size_t, uint8_t *> release()
    {
        auto t_size = size;
        auto t_data = data;
        size = 0;
        data = nullptr;
        return {t_size, t_data};
    }
};

struct Demux::Channel::Data
{
    aui_hdl hnd_channel { nullptr };
    aui_hdl hnd_filter { nullptr };
    std::atomic<bool> m_running { false };
    std::atomic<bool> m_has_data { false };

    Data()
    {
    }

    ~Data()
    {
    }

    std::mutex queue_lock;
    std::queue<MemoryPoolEntry> m_pool_data;

    /**
     *   Function pointer to specify the callback function registered by the function
     *   #aui_dmx_reg_sect_call_back and used to get the section data
     *
     *   @note   About the parameters:
     *   - @b filter_handle             = Input parameter as #aui_hdl handle of
     *                                   the DMX filter already opened by the
     *                                   function #aui_dmx_filter_open
     *   - @b puc_section_data_buf_addr = Input parameter as a pointer to the
     *                                   start address of section data buffer.
     *   - @b Caution: This buffer contains a complete section table, it is not
     *                                   the buffer to TS data.
     *   - @b ul_section_data_len       = Input parameter as the length of the
     *                                   section data (the maximun value of a
     *                                   section data will be 4096 bytes)
     *   - @b pv_usr_data               = Input parameter as a pointer to the
     *                                   user private parameters
     *   - @b pv_reserved               = Input parameter not used currently
     *                                   then user can ignore it
     *
     *   @warning  @b 1. As all callback functions are "fired" in the same event loop thread,
     *   the application should do the minimum job with the current callback
     *   otherwise the other callback functions cannot be "fired" quite quickly.
     *
     *   @warning  @b 2. In the callback functions of DMX Module, the following operations are
     *   not allowed because a deadlock may occur (the application should
     *   always try to avoid deadlock in callback functions):
     *   - Create/Destroy DMX channels or DMX filters
     *   - Stop/Close the DMX device
     *   - Wait for another callback
     */
    static long section_callback(aui_hdl /*filter_handle*/, unsigned char *puc_section_data_buf_addr,
                                 unsigned long ul_section_data_len, void *pv_usr_data, void *)
    {
        auto _ = std::lock_guard(s_global_mutex_lock);
        auto thiz = static_cast<Data *>(pv_usr_data);

        if (thiz->m_running.load(std::memory_order_acquire))
        {
            auto _ = std::lock_guard(thiz->queue_lock);

            if (thiz->m_pool_data.size() < 50)
            {
                auto p = static_cast<uint8_t *>(malloc(ul_section_data_len));
                memcpy(p, puc_section_data_buf_addr, ul_section_data_len);
                thiz->m_pool_data.emplace(ul_section_data_len, p);
                thiz->m_has_data.store(true, std::memory_order_relaxed);
            }
        }
        else
        {
            DEBUG_MSG(HAL, DEBUG, "Discard: not running\n");
        }

        return 0;
    }

    static bool is_pes_packet(const uint8_t* data, size_t size)
    {
        return (size > 4 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01 && data[3] == 0xBD);
    }


    static uint64_t extract_pts(const uint8_t* data, size_t size)
    {
        if (size < 14)
        {
            return 0;
        }
        if ((data[7] & 0x80) && (data[9] & 0x20))
        {
            // Extrai 33 bits do PTS
            uint64_t pts = 0;
            pts |= uint64_t(data[9] & 0x0E) << 29;  // 3 bits
            pts |= uint64_t(data[10]) << 22;         // 8 bits
            pts |= uint64_t(data[11] & 0xFE) << 14;  // 7 bits
            pts |= uint64_t(data[12]) << 7;          // 8 bits
            pts |= uint64_t(data[13] & 0xFE) >> 1;   // 7 bits

            // PTS já está em 90 kHz, mantêm 33 bits
            return pts & ((1ULL << 33) - 1);
        }
        return 0;
    }

    static long pes_data_callback(aui_hdl /*filter_handle*/, unsigned char *pes_data, unsigned long pes_len, void *pv_usr_data)
    {
        auto thiz = static_cast<Demux::Channel *>(pv_usr_data);
        if (pes_len > 184)
        {
            auto p = static_cast<uint8_t *>(malloc(pes_len));
            memcpy(p, pes_data, pes_len);
            if(is_pes_packet(p, pes_len))
            {
                uint64_t pts = extract_pts(p, pes_len) & ((1ULL << 33) - 1);
                if (pts > 0)
                {
                    thiz->m_subtitle_parser.parse_subtitle_segment(&p[4], pes_len-4, pts);
                }
            }
            free(p);
        }
        return 0;
    }
};

Demux::Demux():
    m_p(std::make_unique<Data>())
{
    MB_ZERO(g_ali_demux_attr);
    g_ali_demux_attr.uc_dev_idx = m_demux_id;
    ALI_EXEC(aui_dmx_open(&g_ali_demux_attr, &g_ali_demux_hnd));
    ALI_EXEC(aui_dmx_start(g_ali_demux_hnd, &g_ali_demux_attr));
}

Demux::~Demux()
{
    stop();
    ALI_EXEC(aui_dmx_stop(g_ali_demux_hnd, &g_ali_demux_attr));
    ALI_EXEC(aui_dmx_close(g_ali_demux_hnd));
}

HAL_NATIVE_HANDLE Demux::get_native_handle()
{
    return g_ali_demux_hnd;
}

Demux::Channel::Channel(TABLE_REQUIRE_NAME_TYPE_PARAM Demux *_parent, PID_t _pid, TS_Filter_Table_Ptr _filters, bool _crc_check, Data_Type _data_type):
    m_parent(_parent),
    m_p(std::make_unique<Data>()),
    m_table_id(_filters->value[0]),
    m_pid(_pid),
    m_use_count(0)
{
    m_service_id = 0;
    if(m_table_id == PMT_TABLE_ID)
    {
        m_service_id = (_filters->value[3] << 8) | (_filters->value[4]);
    }
    //a = *std::array<>
    //a[0] -> std::optional[{ .pos = 2, .val = 61 }]
    //a[1] -> std::optional[{ .pos = 3, .val = a8 }]
#ifndef NDEBUG
    m_table_name = _table_name;
#endif
    inc_use_count();
    aui_attr_dmx_channel attr_channel;
    MB_ZERO(attr_channel);
    aui_attr_dmx_filter attr_filter;
    MB_ZERO(attr_filter);
    attr_channel.us_pid = _pid;
    attr_filter.usr_data = m_p.get();

    if(_data_type == Data_Type::SECTION)
    {
        attr_channel.dmx_data_type = AUI_DMX_DATA_SECT;
        ALI_EXEC(aui_dmx_channel_open(g_ali_demux_hnd, &attr_channel, &m_p->hnd_channel));
        ALI_EXEC(aui_dmx_channel_start(m_p->hnd_channel, &attr_channel));
        ALI_EXEC(aui_dmx_filter_open(m_p->hnd_channel, &attr_filter, &m_p->hnd_filter));
        constexpr unsigned char continue_capture_flag = 1u;
        ALI_EXEC(aui_dmx_filter_mask_val_cfg(m_p->hnd_filter,
                                            _filters->mask,
                                            _filters->value,
                                            _filters->reverse,
                                            _filters->size,
                                            _crc_check ? 1 : 0,
                                            continue_capture_flag));
        ALI_EXEC(aui_dmx_reg_sect_call_back(m_p->hnd_filter, Data::section_callback));
        ALI_EXEC(aui_dmx_filter_start(m_p->hnd_filter, &attr_filter));
    }
    else if (_data_type == Data_Type::PES)
    {
        attr_channel.dmx_data_type = AUI_DMX_DATA_PES;
        attr_channel.dmx_channel_pes_callback_support = 1;
        ALI_EXEC(aui_dmx_channel_open(g_ali_demux_hnd, &attr_channel, &m_p->hnd_channel));
        ALI_EXEC(aui_dmx_channel_start(m_p->hnd_channel, &attr_channel));
        ALI_EXEC(aui_dmx_filter_open(m_p->hnd_channel, &attr_filter, &m_p->hnd_filter));
        ALI_EXEC(aui_dmx_reg_pes_call_back(m_p->hnd_filter, Demux::Channel::Data::pes_data_callback, this));
        ALI_EXEC(aui_dmx_filter_start(m_p->hnd_filter, &attr_filter));
    }

    m_p->m_running.store(true, std::memory_order_release);
}

Demux::Channel::Channel(Demux::Channel &&_other):
    m_parent(_other.m_parent),
    m_p(std::move(_other.m_p)),
    m_table_id(_other.m_table_id),
    m_pid(_other.m_pid),
    m_use_count(_other.m_use_count.load(std::memory_order_acquire))
#ifndef NDEBUG
    , m_table_name(_other.m_table_name)
#endif
{
    if(m_table_id == PMT_TABLE_ID)
    {
        m_service_id = _other.m_service_id;
    }
}

Demux::Channel::~Channel()
{
    if (s_read_data.load(std::memory_order_acquire))
    {
        DEBUG_MSG(HAL, ERROR, "WRONG!\n");
    }

    m_p->m_running.store(false, std::memory_order_release);
    aui_attr_dmx_filter attr_filter;
    MB_ZERO(attr_filter);
    ALI_EXEC(aui_dmx_reg_sect_call_back(m_p->hnd_filter, nullptr));
    ALI_EXEC(aui_dmx_filter_stop(m_p->hnd_filter, &attr_filter));
    ALI_EXEC(aui_dmx_filter_close(&m_p->hnd_filter));
    aui_attr_dmx_channel attr_channel;
    MB_ZERO(attr_channel);
    ALI_EXEC(aui_dmx_channel_stop(m_p->hnd_channel, &attr_channel));
    ALI_EXEC(aui_dmx_channel_close(&m_p->hnd_channel));
    auto _ = std::lock_guard(m_p->queue_lock);

    while (!m_p->m_pool_data.empty())
    {
        m_p->m_pool_data.pop();
    }
}

void Demux::Channel::pause()
{
    m_p->m_running.store(false, std::memory_order_release);
}

void Demux::start(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, TS_Filter_Table_Ptr _filters, bool _crc_check, Data_Type _data_type)
{
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [ = ](const auto & c)
    {
        Service_ID_t _service_id = c.service_id();
        if (_filters->value[0] == PMT_TABLE_ID)
        {
            _service_id = (_filters->value[3] << 8) | _filters->value[4];
        }
        return c.pid() == _pid && c.table_id() == _filters->value[0] && c.service_id() == _service_id;
    });

    if (it == m_channels.end())
    {
        m_channels.emplace_back(TABLE_REQUIRE_NAME_PARAM this, _pid, _filters, _crc_check, _data_type);
    }
    else
    {
        it->inc_use_count();
    }
}

void Demux::pause(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Table_ID_t _table_id, Service_ID_t _service_id)
{
    auto it = std::find_if(std::begin(m_channels),
                           std::end(m_channels),
                           [_pid, _table_id, _service_id](const auto & c)
    {
        return c.pid() == _pid && c.table_id() == _table_id && c.service_id() == _service_id;
    }

                          );

    if (it != std::end(m_channels))
    {
        auto use_count = it->use_count();

        if (use_count == 1)
        {
            it->pause();
        }
    }
    else
    {
        mb_assert(false);
    }
}

void Demux::stop(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Table_ID_t _table_id, Service_ID_t _service_id)
{
    auto it = std::find_if(std::begin(m_channels),
                           std::end(m_channels),
                           [_pid, _table_id, _service_id](const auto & c)
    {
        return c.pid() == _pid && c.table_id() == _table_id && c.service_id() == _service_id;
    }

                          );

    if (it != std::end(m_channels))
    {
        auto use_count = it->dec_use_count();

        if (use_count <= 1)
        {
            auto _ = std::lock_guard(s_global_mutex_lock);
            mb_assert(strcmp(_table_name, it->table_name()) == 0);
            mb_assert(!s_read_data.load(std::memory_order_acquire));
            m_channels.erase(it);
        }
    }
    else
    {
        DEBUG_MSG(HAL, DEBUG, _table_name << " PID: " << (int)_pid << " Table Id: " << (int)_table_id << " not started\n");
    }
}

void Demux::stop()
{
    m_channels.clear();
}

void Demux::read_data(std::chrono::milliseconds _timeout)
{
    s_read_data.store(true, std::memory_order_release);

    std::vector<std::reference_wrapper<Channel>> snapshot;
    snapshot.reserve(m_channels.size());

    {
        auto _ = std::lock_guard(s_global_mutex_lock);
        for (auto &c : m_channels)
            snapshot.emplace_back(c);
    }

    for (auto &c : snapshot)
    {
        c.get().read_data(_timeout);
    }

    s_read_data.store(false, std::memory_order_release);
}


bool Demux::is_in_read_data() const
{
    return s_read_data.load(std::memory_order_acquire);
}

void Demux::Channel::read_data(std::chrono::milliseconds _timeout)
{
    if (m_p->m_has_data.load(std::memory_order_relaxed))
    {
        auto end = now() + _timeout;
#ifndef NDEBUG
        bool failed_at_least_once = false;
#endif

        while (now() < end && m_p->m_running.load(std::memory_order_acquire))
        {
            size_t size { 0 };
            uint8_t *data { nullptr };

            if (m_p->queue_lock.try_lock())
            {
                auto _ = std::lock_guard(m_p->queue_lock, std::adopt_lock);

                if (not m_p->m_pool_data.empty())
                {
                    std::tie(size, data) = m_p->m_pool_data.front().release();
                    m_p->m_pool_data.pop();
                }
                else
                {
                    m_p->m_has_data.store(false, std::memory_order_relaxed);
                }
            }
            else
            {
#ifndef NDEBUG

                // If we fail once, we can ignore, but if we failed repeatedly
                // then we have to report it
                if (failed_at_least_once)
                {
                    DEBUG_MSG(HAL, DEBUG, "Demux::Channel::read_data - Failed to aquire lock\n");
                }
                else
                {
                    failed_at_least_once = true;
                }

#endif
                std::this_thread::yield();
                continue;
            }

            if (data)
            {
                parse_data(&data, size);

                if (data)
                {
                    free(data);
                }
            }
            else
            {
                break;
            }
        }
    }
}

} // namespace mb
