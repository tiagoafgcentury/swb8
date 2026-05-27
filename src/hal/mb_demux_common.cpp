#include "mb_demux.h"
#include "common/mb_globals.h"
#include "common/mb_hash.h"
#include "common/mb_types.h"

namespace mb {

void Demux::Channel::parse_data(uint8_t **_data, size_t _data_size)
{
    if(*_data && _data_size)
    {
        auto table_id = *_data[0];

        switch(table_id)
        {
            case PAT_TABLE_ID:
            {
                uint16_t stream_id;
                memcpy(&stream_id, *_data + 1, sizeof(stream_id));
                if(stream_id == 0x1BD)
                {
                    DEBUG("\n******************************************************\n");
                    DEBUG("\n* CHEGUEI AQUI!!!!!!!*\n");
                    DEBUG("\n******************************************************\n");
                }
                else
                {
                    if(m_parent->m_pat_callback)
                    {
                        m_parent->m_pat_callback(pid(), PAT(*_data, _data_size));
                    }
                }

                break;
            }

            case CAT_TABLE_ID:
            {
                if(m_parent->m_cat_callback)
                {
                    m_parent->m_cat_callback(pid(), CAT(_data, _data_size));
                }

                break;
            }

            case PMT_TABLE_ID:
            {
                if(m_parent->m_pmt_callback)
                {
                    m_parent->m_pmt_callback(pid(), PMT(_data, _data_size));
                }

                break;
            }

            case SDT_TABLE_ID_ACTUAL:
            case SDT_TABLE_ID_OTHER:
            {
                if(m_parent->m_sdt_callback)
                {
                    m_parent->m_sdt_callback(pid(), SDT(*_data, _data_size));
                }

                break;
            }

            case NIT_TABLE_ID_ACTUAL:
            case NIT_TABLE_ID_OTHER:
            {
                if(m_parent->m_nit_callback)
                {
                    m_parent->m_nit_callback(pid(), NIT(*_data, _data_size));
                }

                break;
            }

            case BAT_TABLE_ID:
            {
                if(m_parent->m_bat_callback)
                {
                    m_parent->m_bat_callback(pid(), BAT(*_data, _data_size));
                }

                break;
            }

            case TDT_TABLE_ID:
            {
                if(m_parent->m_tdt_callback)
                {
                    m_parent->m_tdt_callback(pid(), TDT(*_data, _data_size));
                }

                break;
            }

            case TOT_TABLE_ID:
            {
                if(m_parent->m_tot_callback)
                {
                    m_parent->m_tot_callback(pid(), TOT(*_data, _data_size));
                }

                break;
            }

            case EIT_TABLE_ID_PF_ACTUAL:
            case EIT_TABLE_ID_PF_OTHER:
                goto EIT_TABLE;

            case DSI_TABLE_ID:
            {
                if(m_parent->m_ota_callback)
                {
                    m_parent->m_ota_callback(pid(), Century(*_data, _data_size));
                }

                break;
            }

            case SKYWORTH_OTA_HEADER_TABLE_ID:
            {
                if(m_parent->m_ota_callback)
                {
                    m_parent->m_ota_callback(pid(), SKW(*_data, _data_size));
                }

                break;
            }

            default:
            {
                if((table_id >= EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW && table_id <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH) ||
                        (table_id >= EIT_TABLE_ID_SCHEDULE_OTHER_LOW && table_id <= EIT_TABLE_ID_SCHEDULE_OTHER_HIGH))
                {
                    goto EIT_TABLE;
                }

                DEBUG_MSG(DVB, WARN, "Unhandled table: 0x" << hex << (int)table_id << "\n");
                break;
EIT_TABLE:

                [[unlikely]] if(EIT::check_service_filter(*_data) and m_parent->m_eit_callback)
                {
                    m_parent->m_eit_callback(pid(), EIT(*_data, _data_size));
                }

                break;
            }
        }
    }
}

} // namespace mb
