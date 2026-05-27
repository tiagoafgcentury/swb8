#pragma once

#include <memory>
#include <cstring>

namespace mb {

class DiseqC_Controller
{
protected:
    struct Data;
    std::unique_ptr<Data> m_p;

public:
    DiseqC_Controller();
    virtual ~DiseqC_Controller();

    bool output(uint8_t _port);
    bool command(std::string _line);
};

} // namespace mb
