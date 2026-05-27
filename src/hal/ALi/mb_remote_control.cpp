#include "../mb_remote_control.h"
#include "../mb_remote_control_keys.h"
#include "common/mb_globals.h"

#include <atomic>

#include <aui_input.h>
#include "mb_ali_globals.h"

#define MB_USE_CALLBACK true

#define RC_DELAY        500
#define RC_INTERVAL     400
#define FP_DELAY        500
#define FP_INTERVAL     400

namespace mb {

GPIO gpio_button_power(GPIO_NUM::POWER_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chup(GPIO_NUM::CHUP_BUTTON, AUI_GPIO_I_DIR);
GPIO gpio_button_chdown(GPIO_NUM::CHDOWN_BUTTON, AUI_GPIO_I_DIR);

struct Remote_Control::Data
{
    Data()
    {
        s_this = this;
    }

    ~Data()
    {
        s_this = nullptr;
    }

    aui_hdl hdl { nullptr };

#ifdef MB_USE_CALLBACK
    static Data* s_this;
    std::atomic<Remote_Control_Key> m_key_buffer { Remote_Control_Key::FRONT_PANEL_0 };

    static void key_callback(aui_key_info *p_key_info, void *)
    {
        DEBUG_MSG(HAL, DEBUG, "Tecla recebida: 0x" << hex << setw(8) << (p_key_info->n_key_code) << "\n");
        switch (p_key_info->e_status)
        {
            case AUI_KEY_STATUS_PRESS:
            case AUI_KEY_STATUS_REPEAT:
                s_this->m_key_buffer.store(static_cast<Remote_Control_Key>(p_key_info->n_key_code & 0x0000FFFF), std::memory_order_release);
                break;

            default:
                break;
        }
    }

    static void gpio_callback(int gpio_index, aui_gpio_interrupt_type interrupt_type, void *pv_user_data)
    {
        DEBUG_MSG(HAL, DEBUG, "GPIO recebido: " << gpio_index << "\n");

        Remote_Control_Key key;

        switch (static_cast<GPIO_NUM>(gpio_index))
        {
            default:
                return;

            case GPIO_NUM::POWER_BUTTON:
                key = Remote_Control_Key::KEY_POWER;
                break;

            case GPIO_NUM::CHDOWN_BUTTON:
                key = Remote_Control_Key::KEY_CHDOWN;
                break;

            case GPIO_NUM::CHUP_BUTTON:
                key = Remote_Control_Key::KEY_CHUP;
                break;

        }

        s_this->m_key_buffer.store(key, std::memory_order_release);
    }
#endif
};

Remote_Control::Data* Remote_Control::Data::s_this = nullptr;

Remote_Control::Remote_Control():
    m_p{std::make_unique<Data>()}
{
    set_standby_wakeup();
    remote_control_init();
}

Remote_Control::~Remote_Control()
{
    ALI_EXEC(aui_key_close(m_p->hdl));
}

void Remote_Control::remote_control_init()
{
    if(m_p->hdl == nullptr)
    {
        ALI_EXEC(aui_key_open(0, nullptr, &(m_p->hdl)));
#ifdef MB_USE_CALLBACK
        aui_key_callback_register(m_p->hdl, Data::key_callback);
        aui_gpio_interrupt_attr attr;
        MB_ZERO(attr);
        attr.p_callback = Data::gpio_callback;
        attr.pv_user_data = m_p.get();
        attr.interrupt_type = AUI_GPIO_INTERRUPT_DISABLED;
        aui_gpio_interrupt_reg(gpio_button_power.get_handle(), &attr);
        aui_gpio_interrupt_reg(gpio_button_chup.get_handle(), &attr);
        aui_gpio_interrupt_reg(gpio_button_chdown.get_handle(), &attr);
#endif
    }

    mb_assert(m_p->hdl != nullptr);
    ALI_EXEC(aui_key_set_ir_rep_interval(m_p->hdl, RC_DELAY, RC_INTERVAL));
}

void Remote_Control::set_standby_wakeup()
{
}

void Remote_Control::read_keys()
{
#ifdef MB_USE_CALLBACK
    auto key = m_p->m_key_buffer.exchange(Remote_Control_Key::FRONT_PANEL_0, std::memory_order_relaxed);
    if (Remote_Control_Key::FRONT_PANEL_0 != key)
    {
        m_key_handler(key);
    }
#else
    aui_key_info key_info;
    MB_ZERO(key_info);
    AUI_RTN_CODE ret;
    ret = aui_key_key_get(m_p->hdl, &key_info);

    if(AUI_RTN_SUCCESS == ret)
    {
        if((key_info.e_status == AUI_KEY_STATUS_PRESS) && m_key_handler)
        {
            DEBUG_MSG(HAL, DEBUG, "Tecla recebida: 0x" << hex << setw(8) << key_info.n_key_code << endl);
            m_key_handler(static_cast<Remote_Control_Key>(key_info.n_key_code & 0x0000FFFF));
        }
        else if(key_info.e_status == AUI_KEY_STATUS_REPEAT && m_key_handler)
        {
            m_key_handler(static_cast<Remote_Control_Key>(key_info.n_key_code & 0x0000FFFF));
        }
    }
#endif
}
 
void Remote_Control::read_keys_front_panel()
{
    using Clock = std::chrono::steady_clock;

    // --- Estados estáticos ---
    static bool chup_pressed   = false;
    static bool chdown_pressed = false;
    static bool power_pressed  = false;
    static Clock::time_point chup_next_repeat;
    static Clock::time_point chdown_next_repeat;
    // -------------------------


    auto now = Clock::now();
    uint8_t ch_up_val = gpio_button_chup.read();
    if (ch_up_val == AUI_GPIO_VALUE_LOW)
    {
        if (!chup_pressed)
        {
            // PRESS
            chup_pressed = true;
            chup_next_repeat = now + std::chrono::milliseconds(FP_DELAY);
            m_key_handler(Remote_Control_Key::KEY_CHUP);
        }
        else if (now >= chup_next_repeat)
        {
            // REPEAT
            chup_next_repeat = now + std::chrono::milliseconds(FP_INTERVAL);
            m_key_handler(Remote_Control_Key::KEY_CHUP);
        }
    }
    else
    {
        chup_pressed = false;
    }

    uint8_t ch_down_val = gpio_button_chdown.read();
    if (ch_down_val == AUI_GPIO_VALUE_LOW)
    {
        if (!chdown_pressed)
        {
            // PRESS
            chdown_pressed = true;
            chdown_next_repeat = now + std::chrono::milliseconds(FP_DELAY);
            m_key_handler(Remote_Control_Key::KEY_CHDOWN);
        }
        else if (now >= chdown_next_repeat)
        {
            // REPEAT
            chdown_next_repeat = now + std::chrono::milliseconds(FP_INTERVAL);
            m_key_handler(Remote_Control_Key::KEY_CHDOWN);
        }
    }
    else
    {
        chdown_pressed = false;
    }

    uint8_t power_val = gpio_button_power.read();
    if (power_val == AUI_GPIO_VALUE_LOW)
    {
        if (!power_pressed)
        {
            power_pressed = true;
            m_key_handler(Remote_Control_Key::KEY_POWER);
        }
    }
    else
    {
        power_pressed = false;
    }
}

} // namespace mb
