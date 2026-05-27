#include "hal/mb_gpio.h"

#include "common/mb_globals.h"

#include <aui_gpio.h>
#include "mb_ali_globals.h"

#include <atomic>

namespace {

std::atomic<int> s_use_count { 0 };

}

namespace mb {

struct GPIO::Data
{
    aui_hdl handle { nullptr };

    static long unsigned int callback(void *)
    {
        DEBUG_MSG(HAL, DEBUG, "GPIO Init\n");
        return 0;
    }
};

GPIO::GPIO(GPIO_NUM _num, bool _readonly):
    m_p(std::make_unique<Data>())
{
    if(s_use_count.fetch_add(1, std::memory_order_release) == 0)
    {
        ALI_EXEC(aui_gpio_init(GPIO::Data::callback, nullptr));
    }

    if(!valid_io(_num))
    {
        DEBUG_MSG(HAL, ERROR, "Erro, I/O " << static_cast<int>(_num) << " Inválida\n");
        return;
    }

    m_valid = true;
    aui_gpio_attr gpio_attr;
    MB_ZERO(gpio_attr);
    gpio_attr =
    {
        .uc_dev_idx = static_cast<int>(_num),
        .io = (_readonly ? AUI_GPIO_O_DIR : AUI_GPIO_I_DIR),
        .value_out = AUI_GPIO_VALUE_LOW
    };
    ALI_EXEC(aui_gpio_open(&gpio_attr, &(m_p->handle)));
}

GPIO::~GPIO()
{
    if(m_valid)
    {
        ALI_EXEC(aui_gpio_close(m_p->handle));
    }

    if(s_use_count.fetch_sub(1, std::memory_order_release) == 1)
    {
        ALI_EXEC(aui_gpio_deinit());
    }
}

void* GPIO::get_handle()
{
    return m_p->handle;
}

uint8_t GPIO::read()
{
    aui_gpio_value value;

    if(!m_valid)
    {
        DEBUG_MSG(HAL, ERROR, "Erro, I/O inválida\n");
        return 0;
    }

    ALI_EXEC(aui_gpio_get_value(m_p->handle, &value));
    return value;
}

void GPIO::write(uint8_t _value)
{
    if(!m_valid)
    {
        DEBUG_MSG(HAL, ERROR, "Erro, I/O inválida\n");
        return;
    }

    ALI_EXEC(aui_gpio_set_value(m_p->handle, _value ? AUI_GPIO_VALUE_HIGH : AUI_GPIO_VALUE_LOW));
}

#ifdef CJSON_VERSION_MAJOR
// Lista leds
cJSON *GPIO::list_led()
{
    auto result = cJSON_CreateObject();
    auto leds = cJSON_AddArrayToObject(result, "ledList");
    // Led vermelho
    auto item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "gpio", static_cast<int>(GPIO_NUM::LED_RED));
    cJSON_AddStringToObject(item, "name", "Led Vermelho");
    cJSON_AddItemToArray(leds, item);
    // Led verde
    item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "gpio", static_cast<int>(GPIO_NUM::LED_GREEN));
    cJSON_AddStringToObject(item, "name", "Led Verde");
    // Resultado final com a lista
    cJSON_AddItemToArray(leds, item);
    return result;
}

// Lista de teclas
cJSON *GPIO::list_button()
{
    auto result = cJSON_CreateObject();
    auto buttons = cJSON_AddArrayToObject(result, "buttonsList");
    // Led vermelho
    auto led = cJSON_CreateObject();
    cJSON_AddNumberToObject(led, "gpio", static_cast<int>(GPIO_NUM::POWER_BUTTON));
    cJSON_AddStringToObject(led, "name", "Tecla power");
    cJSON_AddItemToArray(buttons, led);
    // Led verde
    led = cJSON_CreateObject();
    cJSON_AddNumberToObject(led, "gpio", static_cast<int>(GPIO_NUM::CHUP_BUTTON));
    cJSON_AddStringToObject(led, "name", "Tecla CHUP");
    cJSON_AddItemToArray(buttons, led);
    // Led verde
    led = cJSON_CreateObject();
    cJSON_AddNumberToObject(led, "gpio", static_cast<int>(GPIO_NUM::CHDOWN_BUTTON));
    cJSON_AddStringToObject(led, "name", "Tecla CHDOWN");
    // Resultado final com a lista
    cJSON_AddItemToArray(buttons, led);
    return result;
}

#endif // CJSON_VERSION_MAJOR

bool GPIO::valid_io(GPIO_NUM io)
{
    return  valid_io_led(io) || valid_io_button(io);
}

bool GPIO::valid_io_led(GPIO_NUM io)
{
    return  io == GPIO_NUM::LED_GREEN ||
            io == GPIO_NUM::LED_RED;
}

bool GPIO::valid_io_button(GPIO_NUM io)
{
    return  io == GPIO_NUM::POWER_BUTTON ||
            io == GPIO_NUM::CHUP_BUTTON ||
            io == GPIO_NUM::CHDOWN_BUTTON;
}

} // namespace mb
