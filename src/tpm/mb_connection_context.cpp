#include "mb_connection_context.h"

#include "mb_task_http_server.h"

#include "../hal/mb_gpio.h"
#include <aui_gpio.h>

namespace mb {

GPIO gpio_button_power(GPIO_NUM::POWER_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chup(GPIO_NUM::CHUP_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chdown(GPIO_NUM::CHDOWN_BUTTON, AUI_GPIO_I_DIR);


bool wait_remote_control_context::test()
{
    // Testa se alguma tecla foi digitada, se sim,
    // retorna true e encerra o processamento
    if(Task_HTTP_Server::s_last_key_pressed.has_value())
    {
        return true;
    }

    // Se não, roda o teste da classe mãe que testa se
    // o time-out expirou.
    return delayed_context::test();
}

bool wait_key_panel_context::test()
{
    // Testa se alguma tecla foi digitada, se sim,
    // retorna true e encerra o processamento
    if(!(gpio_button_power.read() && gpio_button_chdown.read() && gpio_button_chup.read()))
    {
        return true;
    }

    // Se não, roda o teste da classe mãe que testa se
    // o time-out expirou.
    return delayed_context::test();
}

wait_get_system_info::wait_get_system_info()
{
    if(not(Task_HTTP_Server::s_nuid.has_value() and
            Task_HTTP_Server::s_caid.has_value() and
            Task_HTTP_Server::s_scua.has_value()))
    {
        //Task::post_event_cas_fingerprint_get();
    }
}

bool wait_get_system_info::test()
{
    // Testa se alguma tecla foi digitada, se sim,
    // retorna true e encerra o processamento
    if(Task_HTTP_Server::s_nuid.has_value() and
            Task_HTTP_Server::s_caid.has_value() and
            Task_HTTP_Server::s_scua.has_value())
    {
        return true;
    }

    // Se não, roda o teste da classe mãe que testa se
    // o time-out expirou.
    return delayed_context::test();
}


} // namespace mb
