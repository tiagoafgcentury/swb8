#include "mb_dvb_eitv.h"
#include "common/mb_globals.h"
#include "common/mb_assert.h"

#include <string.h>
#include <algorithm>

namespace mb {

EiTV::EiTV(const uint8_t *_data, size_t _size)
{
    mb_assert(table_id() == _data[0]);

    if(table_id() != _data[0])
    {
        return;
    }

    auto end { _data + _size };
    /*                                                                                                                            2022-07-08
    1103100280010005FF0000B1000000010FE200000000000047868C00002E00 04 0109 FF819282 E1B5030600 0109 FF819282 E1B6030100 0109 FF819282 B5 1CE97800 0109 FF819282 B6 1CE97800 0004 0000000004A5011F021D42355F4C69737461446543*61
    1103100280010005FF0000DE000000010FE200000000000047868C00004400 06 0109 FF819282 E1B5030900 0109 FF819282 E1B6030900 0109 FF819282 E1B7030B00 0109 FF819282 B5 1CE97800 0109 FF819282 B6 1CE97800 0109 FF819282 B7 1CE97800 0005
    1103100280010005FF000105000000010FE200000000000047868C00004400 06 0109 FF819282 E1B5030D00 0109 FF819282 E1B6030D00 0109 FF819282 E1B7030D00 0109 FF819282 B5 1CE97800 0109 FF819282 B6 1CE97800 0109 FF819282 B7 1CEAC600 0006
     * */
    auto num_files = _data[39];
    // Filoeds inverted because of endiness
    struct EiTVData
    {
        uint16_t _0901;
        uint16_t _81FF;
        uint16_t _8292;
        uint8_t product0;
        uint8_t product1;
        uint8_t ver_major;
        uint8_t ver_minor;
        uint8_t ver_patch;
    } __attribute__((packed));
    const size_t header_size = 40;
    _ota_files.reserve(num_files / 2);
    auto p = reinterpret_cast<const EiTVData *>(_data + header_size);
    std::vector<uint16_t> channel_lists;
    auto c = _data + header_size + (sizeof(EiTVData) * num_files);
    mb_assert(c < end);

    if(c < end)
    {
        for(auto i = 0; i < num_files; i++)
        {
            if(p[i]._8292 == 0x8292)
            {
                uint8_t product;

                if(p[i].product0 == 0xE1)
                {
                    product = p[i].product1;
                }
                else
                {
                    product = p[i].product0;
                }

                OTA_File *ota { nullptr };
                auto it = std::find_if(_ota_files.begin(), _ota_files.end(), [product](const auto & o)
                {
                    return o.product == product;
                });

                if(it == _ota_files.end())
                {
                    _ota_files.emplace_back();
                    ota = &_ota_files[_ota_files.size() - 1];
                    ota->type = OTA_Type::EiTV;
                    ota->table_id = _data[0];
                    ota->product = product;
                }
                else
                {
                    ota = &_ota_files[std::distance(_ota_files.begin(), it)];
                }

                if(p[i].product0 == 0xE1)
                {
                    ota->version = (p[i].ver_major << 8) | p[i].ver_minor;
                }
                else
                {
                    auto version = ((p[i].ver_major & 0b00001111) << 8) | p[i].ver_minor;
                    ota->channel_list_version = version;
                    auto it = std::find(channel_lists.begin(), channel_lists.end(), version);

                    if(it == channel_lists.end())
                    {
                        channel_lists.push_back(version);
                    }
                };
            }
        }

        auto cl = channel_lists.begin();

        for(; c < end; c++)
        {
            if(strcmp((char *)c, ".dat") == 0)
            {
                auto start = (char *)c;

                while(isprint(*start))
                {
                    --start;
                }

                start++;

                for(auto &o : _ota_files)
                {
                    if(o.channel_list_version == *cl)
                    {
                        o.channel_list = start;
                    }
                }

                cl++;

                if(cl == channel_lists.end())
                {
                    break;
                }

                c += 4;
            }
        }
    }
}

} // namespace mb
