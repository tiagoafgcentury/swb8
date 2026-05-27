#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <set>

#ifndef NDEBUG
#include <unordered_set>
#endif

#include "dvb/mb_dvb_tables.h"
#include "mb_dvb_subtitle_parser.h"

#include "hal/mb_hal_system_select.h"
#include "hal/mb_demux_macros.h"

namespace mb {

class Demux
{
public:
    typedef std::function < void(PID_t pid, BAT &&) > BAT_callback;
    typedef std::function < void(PID_t pid, CAT &&) > CAT_callback;
    typedef std::function < void(PID_t pid, EIT &&) > EIT_callback;
    typedef std::function < void(PID_t pid, NIT &&) > NIT_callback;
    typedef std::function < void(PID_t pid, OTA &&) > OTA_callback;
    typedef std::function < void(PID_t pid, PAT &&) > PAT_callback;
    typedef std::function < void(PID_t pid, PMT &&) > PMT_callback;
    typedef std::function < void(PID_t pid, SDT &&) > SDT_callback;
    typedef std::function < void(PID_t pid, TDT &&) > TDT_callback;
    typedef std::function < void(PID_t pid, TOT &&) > TOT_callback;

    Demux();
    ~Demux();

    enum class Data_Type
    {
        SECTION,
        PES,
    };

    struct TS_Filter_Table
    {
        uint8_t size { 0 };
        uint8_t *value { nullptr };
        uint8_t *mask { nullptr };
        uint8_t *reverse { nullptr };
    };
    typedef std::shared_ptr<TS_Filter_Table> TS_Filter_Table_Ptr;

    struct TS_Filter_Table_Id: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 1;

        TS_Filter_Table_Id(Table_ID_t _table_id)
        {
            buf_value = {_table_id};
            buf_mask = {0xFF};
            buf_reverse = {0x00};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_BAT_Bouquet_Id: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 5;

        TS_Filter_BAT_Bouquet_Id(Table_ID_t _table_id, const uint16_t _bouquet_id)
        {
            uint8_t id_high = (_bouquet_id >> 8);
            uint8_t id_low = (_bouquet_id & 0xFF);
            buf_value =     {_table_id, 0x00, 0x00, id_high,    id_low};
            buf_mask =      {0xFF,      0x00, 0x00, 0xFF,       0xFF};
            buf_reverse =   {0x00,      0xFF, 0xFF, 0x00,       0x00};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_OTA_Message_Id: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 12;

        TS_Filter_OTA_Message_Id(Table_ID_t _table_id, const uint16_t _message_id)
        {
            uint8_t id_high = (_message_id >> 8);
            uint8_t id_low = (_message_id & 0xFF);
            buf_value = {_table_id, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, id_high,  id_low};
            buf_mask =       {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,     0xFF};
            buf_reverse =    {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,     0x00};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_SDT_TS_Id: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 5;

        TS_Filter_SDT_TS_Id(Table_ID_t _table_id, const uint16_t _ts_id)
        {
            uint8_t id_high = (_ts_id >> 8);
            uint8_t id_low = (_ts_id & 0xFF);
            buf_value =     {_table_id, 0x00, 0x00, id_high,    id_low};
            buf_mask =      {0xFF,      0x00, 0x00, 0xFF,       0xFF};
            buf_reverse =   {0x00,      0xFF, 0xFF, 0x00,       0x00};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }
        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_PMT: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 5;

        TS_Filter_PMT(Table_ID_t _table_id, const uint16_t _prog_number)
        {
            uint8_t id_high = (_prog_number >> 8);
            uint8_t id_low = (_prog_number & 0xFF);
            buf_value =     {_table_id, 0x00, 0x00, id_high,    id_low};
            buf_mask =      {0xFF,      0x00, 0x00, 0xFF,       0xFF};
            buf_reverse =   {0x00,      0xFF, 0xFF, 0x00,       0x00};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_PMT_Next: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 6;

        TS_Filter_PMT_Next(Table_ID_t _table_id, const uint16_t _prog_number, uint8_t _version_number)
        {
            uint8_t id_high = (_prog_number >> 8);
            uint8_t id_low = (_prog_number & 0xFF);
            _version_number = _version_number << 1;
            _version_number = _version_number & 0b00111110;
            buf_value = {_table_id, 0x00, 0x00, id_high, id_low, _version_number};
            buf_mask = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0b00111110};
            buf_reverse = {0x00, 0x00, 0x00, 0x00, 0x00, 0b00111110};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    struct TS_Filter_CAT_Next: public TS_Filter_Table
    {
        static constexpr auto FILTER_MASK_SIZE = 6;

        TS_Filter_CAT_Next(Table_ID_t _table_id, uint8_t _version_number)
        {
            _version_number = _version_number << 1;
            _version_number = _version_number & 0b00111110;
            buf_value = {_table_id, 0x00, 0x00, 0x00, 0x00, _version_number};
            buf_mask = {0xFF, 0x00, 0x00, 0x00, 0x00, 0b00111110};
            buf_reverse = {0x00, 0x00, 0x00, 0x00, 0x00, 0b00111110};
            value = &buf_value[0];
            mask = &buf_mask[0];
            reverse = &buf_reverse[0];
            size = FILTER_MASK_SIZE;
        }

        std::array<uint8_t, FILTER_MASK_SIZE> buf_value;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_mask;
        std::array<uint8_t, FILTER_MASK_SIZE> buf_reverse;
    };

    void start(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, TS_Filter_Table_Ptr _filters, bool _crc_check, Data_Type _data_type);
    void pause(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Table_ID_t _table_id, Service_ID_t _service_id);
    void stop(TABLE_REQUIRE_NAME_TYPE_PARAM PID_t _pid, Table_ID_t _table_id, Service_ID_t _service_id);
    void stop();

    void clear_bat_callback()
    {
        m_bat_callback = {};
    }

    void set_bat_callback(BAT_callback _bat_callback)
    {
        mb_assert(!m_bat_callback);
        m_bat_callback = _bat_callback;
    }

    void clear_cat_callback()
    {
        m_cat_callback = {};
    }

    void set_cat_callback(CAT_callback _cat_callback)
    {
        mb_assert(!m_cat_callback);
        m_cat_callback = _cat_callback;
    }

    void clear_eit_callback()
    {
        m_eit_callback = {};
    }

    void set_eit_callback(EIT_callback _eit_callback)
    {
        mb_assert(!m_eit_callback);
        m_eit_callback = _eit_callback;
    }

    void clear_nit_callback()
    {
        m_nit_callback = {};
    }

    void set_nit_callback(NIT_callback _nit_callback)
    {
        mb_assert(!m_nit_callback);
        m_nit_callback = _nit_callback;
    }

    void clear_ota_callback()
    {
        m_ota_callback = {};
    }

    void set_ota_callback(OTA_callback _ota_callback)
    {
        mb_assert(!m_ota_callback);
        m_ota_callback = _ota_callback;
    }

    void clear_pat_callback()
    {
        m_pat_callback = {};
    }

    void set_pat_callback(PAT_callback _pat_callback)
    {
        mb_assert(!m_pat_callback);
        m_pat_callback = _pat_callback;
    }

    void clear_pmt_callback()
    {
        m_pmt_callback = {};
    }

    void set_pmt_callback(PMT_callback _pmt_callback)
    {
        mb_assert(!m_pmt_callback);
        m_pmt_callback = _pmt_callback;
    }

    void clear_sdt_callback()
    {
        m_sdt_callback = {};
    }
    void set_sdt_callback(SDT_callback _sdt_callback)
    {
        mb_assert(!m_sdt_callback);
        m_sdt_callback = _sdt_callback;
    }

    void clear_tdt_callback()
    {
        m_tdt_callback = {};
    }

    void set_tdt_callback(TDT_callback _tdt_callback)
    {
        mb_assert(!m_tdt_callback);
        m_tdt_callback = _tdt_callback;
    }

    void clear_tot_callback()
    {
        m_tot_callback = {};
    }

    void set_tot_callback(TOT_callback _tot_callback)
    {
        mb_assert(!m_tot_callback);
        m_tot_callback = _tot_callback;
    }

    void read_data(std::chrono::milliseconds _timeout);
    bool is_in_read_data() const;

    auto demux_id() const
    {
        return m_demux_id;
    }

    HAL_NATIVE_HANDLE get_native_handle();

private:
    uint8_t m_demux_id { 0 };

    struct Data;
    std::unique_ptr<Data> m_p;

    class Channel
    {
    private:
        Demux *m_parent { nullptr };

        struct Data;
        std::unique_ptr<Data> m_p;

        Table_ID_t m_table_id { 0 };
        PID_t m_pid { 0 };
        Service_ID_t m_service_id { 0 };

        std::atomic<int> m_use_count;

        DVB_Subtitle_Parser m_subtitle_parser;

#ifndef NDEBUG
        const char *m_table_name = nullptr;
#endif
        void parse_data(uint8_t **_data, size_t _data_size);
    public:
        Channel(TABLE_REQUIRE_NAME_TYPE_PARAM Demux *_parent, PID_t _pid, TS_Filter_Table_Ptr _filters, bool _crc_check, Data_Type _data_type);
        ~Channel();

        Channel(const Channel &) = delete;
        Channel &operator=(Channel const &) = delete;

        Channel(Channel &&_other);

        void read_data(std::chrono::milliseconds _timeout);
        void pause();

        auto pid() const
        {
            return m_pid;
        }

        auto table_id() const
        {
            return m_table_id;
        }

        auto service_id() const
        {
            return m_service_id;
        }

        auto inc_use_count()
        {
            return (m_use_count++);
        }

        auto dec_use_count()
        {
            return (m_use_count.fetch_sub(1));
        }

        auto use_count()
        {
            return (m_use_count.load(std::memory_order_acquire));
        }

#ifndef NDEBUG
        const char *table_name() const
        {
            return m_table_name;
        }
#endif
    };
    friend class Channel;

    std::list<Channel> m_channels;

    BAT_callback m_bat_callback;
    CAT_callback m_cat_callback;
    EIT_callback m_eit_callback;
    NIT_callback m_nit_callback;
    OTA_callback m_ota_callback;
    PAT_callback m_pat_callback;
    PMT_callback m_pmt_callback;
    SDT_callback m_sdt_callback;
    TDT_callback m_tdt_callback;
    TOT_callback m_tot_callback;
};

} // namespace mb
