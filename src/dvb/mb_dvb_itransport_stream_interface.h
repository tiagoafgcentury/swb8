#pragma once

#include <cstdint>
#include <vector>

#include "mb_dvb_transport_stream.h"

namespace mb {

class ITransport_Stream_Interface
{
private:
    SI_Table *_parent { nullptr };

protected:
    std::vector<Transport_Stream> _transport_streams;

    void parse_transport_streams(const uint8_t *_data, size_t _size);

public:
    explicit ITransport_Stream_Interface(SI_Table *parent);
    virtual ~ITransport_Stream_Interface() {};

    virtual Table_ID_t table_id() const
    {
        return _parent->table_id();
    }
    auto move_transport_streams()
    {
        return std::move(_transport_streams);
    }
};

};
