#include "mb_dvb_tables.h"
#include "common/mb_globals.h"

#include <algorithm>

namespace mb {

ITransport_Stream_Interface::ITransport_Stream_Interface(SI_Table *parent):
    _parent(parent)
{
}

void ITransport_Stream_Interface::parse_transport_streams(const uint8_t *_data, size_t _size)
{
    const uint16_t Transport_Stream_Size { (16 + 16 + 4 + 12) / 8 };

    for(uint16_t stream_length = 0; _size > 2; _data += stream_length, _size -= stream_length)
    {
        stream_length = ((_data[4] & 0b00001111) << 8) | _data[5];
        stream_length += Transport_Stream_Size;

        if(stream_length > _size)
        {
            DEBUG_MSG(DVB, ERROR, "Stream Length greater then available data!" << endl);
            return;
        }

        _transport_streams.emplace_back(_parent, _data, _size);
    }
}

} // namespace mb
