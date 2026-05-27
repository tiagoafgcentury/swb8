#include "mb_diseqc.h"
#include "common/mb_globals.h"
#include <array>

#include <aui_nim.h>
#include "mb_ali_globals.h"

#include <sstream>

namespace mb {

struct DiseqC_Controller::Data
{
    aui_hdl hdl {};
};

DiseqC_Controller::DiseqC_Controller():
    m_p(std::make_unique<Data>())
{
    unsigned long nim_dev = 0;
    ALI_EXEC(aui_find_dev_by_idx(AUI_MODULE_NIM, nim_dev, &m_p->hdl));
}

DiseqC_Controller::~DiseqC_Controller()
{
}

bool DiseqC_Controller::output(uint8_t _pin)
{
    DEBUG_MSG(HAL, DEBUG, "DiseqC output: " << dec << (int)_pin << "\n");
    std::array<uint8_t, 4> vec = {0xE0, 0x10, 0x38, 0xF0};

    if(_pin < 1 || _pin > 4)
    {
        DEBUG_MSG(HAL, ERROR, "Porta inválida <1-4>\n");
        return false;
    }

    vec[3] = 0xF0 + (_pin - 1) * 4;

    if(aui_diseqc_oper(m_p->hdl, AUI_DISEQC_MODE_BYTES, vec.data(), vec.size(), nullptr, nullptr) != 0)
    {
        DEBUG_MSG(HAL, ERROR, "aui_diseqc_oper failed\n");
        return false;
    }

    return true;
}

static std::vector<uint8_t> split_by_character(const std::string &str, char delimiter)
{
    std::vector<uint8_t> result;
    std::stringstream ss(str);
    std::string token;
    result.reserve(str.size() / 2);

    while(std::getline(ss, token, delimiter))
    {
        result.push_back(static_cast<uint8_t>(std::stoi(token, nullptr, 16)));
    }

    return result;
}

bool DiseqC_Controller::command(std::string _line)
{
    auto elements = split_by_character(_line, ' ');
#ifndef NDEBUG
    DEBUG_MSG_NL(HAL, DEBUG, "");

    for(const auto e : elements)
    {
        DEBUG_MSG_NL(HAL, DEBUG, hex << setw(2) << setfill('0') << static_cast<int>(e) << " ");
    }

    DEBUG_MSG(HAL, DEBUG, ": " << elements.size() << "\n");
#endif

    if(aui_diseqc_oper(m_p->hdl, AUI_DISEQC_MODE_BYTES, elements.data(), elements.size(), nullptr, nullptr) != 0)
    {
        return false;
    }

    return true;
}

};// namespace mb
