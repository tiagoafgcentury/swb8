#pragma once

#include <cinttypes>
#include <memory>
#include <vector>

#if __has_include(<cJSON.h>)
#include <cJSON.h>
#else
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#endif
#endif

namespace mb {

enum class GPIO_NUM
{
    TDI = 186,
    CHDOWN_BUTTON = 233,
    LED_GREEN = 234,
    CHUP_BUTTON = 235,
    LED_RED = 236,
    POWER_BUTTON = 241,
};

class GPIO
{
protected:
    struct Data;
    std::unique_ptr<Data> m_p;

private:
    bool m_valid = false;

public:
    GPIO(GPIO_NUM  _num, bool _readonly = true);
    ~GPIO();

    uint8_t read();
    void write(uint8_t _value);
    bool valid_io(GPIO_NUM);
    bool valid_io_led(GPIO_NUM);
    bool valid_io_button(GPIO_NUM);

    void* get_handle();

#ifdef CJSON_VERSION_MAJOR
    cJSON *list_led(void);
    cJSON *list_button(void);
#endif // CJSON_VERSION_MAJOR
};

} // namespace mb
