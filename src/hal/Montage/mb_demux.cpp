#include "mb_demux.h"
#include "mb_globals.h"

#include <mt_common.h>
#include <mt_unf_demux.h>
#undef min
#undef max

#include <string.h>
#include <memory>
#include <array>
#include <atomic>
#include <algorithm>

#ifndef NDEBUG
#include <chrono>
#endif

#define INVALID_TABLE_ID (0xff)
#define CHANNEL_MAX_PROG    256
#define PROG_MAX_VIDEO      8
#define PROG_MAX_AUDIO      8
#define PROG_MAX_CA         8

#define SUBTDES_INFO_MAX 10
#define SUBTITLING_MAX 15
#define CAPTION_SERVICE_MAX 16
#define TTX_DES_MAX    10
#define TTX_MAX        15

#define SUBT_TYPE_DVB  (0x1)
#define SUBT_TYPE_SCTE (0x2)
#define SUBT_TYPE_BOTH (SUBT_TYPE_DVB | SUBT_TYPE_SCTE)

#define MAX_SECTION_LEN 4096
#define MAX_SECTION_NUM 256
#define MAX_PROGNAME_LENGTH 256

SINGULARITY_TOKEN

namespace mb {

Demux::Demux()
{
    SINGULARITY_CHECK(false);
    MT_EXEC(MT_UNF_DMX_Init());
#ifndef NDEBUG
    MT_UNF_DMX_CAPABILITY_S cap;
    MB_ZERO(cap);
    MT_EXEC(MT_UNF_DMX_GetCapability(&cap));
    DEBUG_MSG("DeMux CAP:" << dec <<
              "\n\tNUM IF Ports: " << cap.u32IFPortNum <<
              "\n\tNUM TS In Ports: " << cap.u32TSIPortNum <<
              "\n\tNUM TS Out Ports: " << cap.u32TSOPortNum <<
              "\n\tNUM TS RAM Ports: " << cap.u32RamPortNum <<
              "\n\tNUM Demux devices: " << cap.u32DmxNum <<
              "\n\tNUM Channels: " << cap.u32ChannelNum <<
              "\n\tNUM AV Channels: " << cap.u32AVChannelNum <<
              "\n\tNUM Filters: " << cap.u32FilterNum <<
              "\n\tNUM Keys: " << cap.u32KeyNum <<
              "\n\tNUM Rec Channels: " << cap.u32RecChnNum <<
              endl
             );
#endif
    MT_EXEC(MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_0));
}

Demux::~Demux()
{
    stop();
    MT_EXEC(MT_UNF_DMX_DetachTSPort(0));
    MT_EXEC(MT_UNF_DMX_DeInit());
    SINGULARITY_CHECK(true);
}

Demux::Channel::Channel(TABLE_REQUIRE_NAME_TYPE_PARAM Demux *_parent, uint32_t _pid, uint32_t _table_id, bool _crc_check):
    m_parent(_parent),
    m_table_id(_table_id),
    m_pid(_pid),
    m_use_count(0)
{
#ifndef NDEBUG
    m_table_name = _table_name;
    DEBUG_MSG("Start DEMUX: " << m_table_name << " " << hex << setfill('0') << setw(2) << _table_id << "\n");
#endif
    inc_use_count();
    MT_UNF_DMX_CHAN_ATTR_S chan_attr;
    MB_ZERO(chan_attr);
    MT_EXEC(MT_UNF_DMX_GetChannelDefaultAttr(&chan_attr));
    chan_attr.u32BufSize = _pid == EIT_TSPID ? 8192 : 1024;
    chan_attr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    chan_attr.enCRCMode = _crc_check ? MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD : MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    chan_attr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    MT_EXEC(MT_UNF_DMX_CreateChannel(_parent->demux_id(), &chan_attr, &m_channel_handle));
    MT_EXEC(MT_UNF_DMX_SetChannelPID(m_channel_handle, _pid));
    MT_UNF_DMX_FILTER_ATTR_S filter_attr;
    MB_ZERO(filter_attr);
    filter_attr.u32FilterDepth = 1;
    filter_attr.au8Match[0] = _table_id;
    MT_EXEC(MT_UNF_DMX_CreateFilter(_parent->demux_id(), &filter_attr, &m_filter_handle));
    MT_EXEC(MT_UNF_DMX_AttachFilter(m_filter_handle, m_channel_handle));
    MT_EXEC(MT_UNF_DMX_OpenChannel(m_channel_handle));
}

Demux::Channel::~Channel()
{
    DEBUG_MSG("Stop DEMUX: " << m_table_name << "\n");

    if(m_channel_handle || m_filter_handle)
    {
        auto chan { m_channel_handle };
        m_channel_handle = 0;
        auto filt { m_filter_handle };
        m_filter_handle = 0;
        MT_EXEC(MT_UNF_DMX_CloseChannel(chan));
        MT_EXEC(MT_UNF_DMX_DetachFilter(filt, chan));
        MT_EXEC(MT_UNF_DMX_DestroyFilter(filt));
        MT_EXEC(MT_UNF_DMX_DestroyChannel(chan));
    }
}

void Demux::start(TABLE_REQUIRE_NAME_TYPE_PARAM uint32_t _pid, uint32_t _table_id, bool _crc_check)
{
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [ = ](const auto & c)
    {
        return c.pid() == _pid && c.table_id() == _table_id;
    });

    if(it == m_channels.end())
    {
        m_channels.emplace_back(TABLE_REQUIRE_NAME_PARAM this, _pid, _table_id, _crc_check);
    }
    else
    {
        it->inc_use_count();
    }
}

void Demux::stop(TABLE_REQUIRE_NAME_TYPE_PARAM uint32_t _pid, uint32_t _table_id)
{
    auto it = std::find_if(std::begin(m_channels),
                           std::end(m_channels),
                           [_pid, _table_id](const auto & c)
    {
        return c.pid() == _pid && c.table_id() == _table_id;
    }
                          );
#ifndef NDEBUG
    uint64_t hash = _pid;
    hash = hash << 32;
    hash |= _table_id;
#endif

    if(it != std::end(m_channels))
    {
        auto use_count = it->dec_use_count();

        if(use_count <= 1)
        {
            mb_assert(strcmp(_table_name, it->table_name()) == 0);
            m_channels.erase(it);
        }
    }
    else
    {
        mb_assert(false);
    }
}

void Demux::stop()
{
    m_channels.clear();
}

void Demux::read_data(std::chrono::milliseconds _timeout)
{
    for(auto &chan : m_channels)
    {
        if(MT_UNF_DMX_CheckDataHandle(chan.channel_id(), _timeout.count()) == MT_SUCCESS)
        {
            chan.read_data(_timeout);
        }
    }
}

void Demux::Channel::read_data(std::chrono::milliseconds _timeout)
{
    const uint32_t num_requests { 4 };
    MT_UNF_DMX_DATA_S section[num_requests];
    MB_ZERO(section);
    uint32_t num_read { 0 };
    auto ret = MT_UNF_DMX_AcquireBuf(m_channel_handle, num_requests, &num_read, section, _timeout.count());

    switch(ret)
    {
        case MT_SUCCESS:
        {
            if(num_read > 0)
            {
                for(uint32_t i = 0; i < num_read; i++)
                {
                    switch(section[i].enDataType)
                    {
                        case MT_UNF_DMX_DATA_TYPE_WHOLE:
                        {
                            parse_data(section[i].pu8Data[0], section[i].u32Size);
                            break;
                        }

                        default:
                            DEBUG_MSG("Unhandled data type: 0x" << hex << (int)section[i].enDataType << "\n");
                    }
                }

                MT_UNF_DMX_ReleaseBuf(m_channel_handle, num_read, section);
            }

            return;
        }

        case MT_ERR_DMX_TIMEOUT:
        case MT_ERR_DMX_NOAVAILABLE_DATA:
        {
            //DEBUG_MSG("FAILED: MT_UNF_DMX_AcquireBuf(" << (int)m_channel_handle << "): time out" << endl);
            return;
        }

        default:
            DEBUG_MSG("FAILED: MT_UNF_DMX_AcquireBuf(" << dec << (int)m_channel_handle << "): " << mt_get_error_msg(ret) << endl);
            return;
    }
}

} // namespace mb
