#include "../mb_system.h"
#include "../mb_display.h"

#include "mb_gpio.h"

#include <aui_misc.h>
#include <aui_rtc.h>

#include "mb_ali_globals.h"

#include "common/mb_state_file.h"
#include "common/mb_globals.h"
#include "common/SHA256.h"

#include <iostream>
#include <sys/stat.h>
#include <filesystem>

constexpr auto STRAP_PIN = 0x1808d820u;

namespace mb {

struct System::Data
{
};

System *System::s_instance { nullptr };

static aui_hdl s_rtc_handler { nullptr };
static bool s_fake_standby_mode { false };

System::System(bool _production_final_test):
    m_p(std::make_unique<Data>()),
    m_hdmi(std::make_unique<HDMI>())
{
    mb_assert(s_instance == nullptr);
    s_instance = this;
    ALI_EXEC(aui_rtc_open(&s_rtc_handler));
    check_standby_mode(_production_final_test);
}

System::~System()
{
    ALI_EXEC(aui_rtc_close(s_rtc_handler));
    mb_assert(s_instance == this);
    s_instance = nullptr;
}

void System::poweroff_tpm()
{
    ALI_EXEC(aui_standby_set_ir(0x60f73bc4, AUI_IR_TYPE_NEC, 0, 0));
    ALI_EXEC(aui_standby_init());
    ALI_EXEC(aui_standby_enter());
}

void System::check_standby_mode(bool _production_final_test)
{
    State_File::App_State_File file;
    DEBUG_MSG(HAL, DEBUG, TERM_YELLOW_BOLD "Standby: " << (file.stand_by ? "ON" : "OFF") << "\n" TERM_RESET);
    s_fake_standby_mode = file.stand_by;

    if(_production_final_test)
    {
        s_fake_standby_mode = file.stand_by_in_production_mode;
    }

    if(s_fake_standby_mode)
    {
        DEBUG_MSG(HAL, DEBUG, "Fake Standby Mode\n");
        s_instance->fake_power_off();
        s_instance->set_led_color(LED_Color::RED);
    }
    else
    {
        s_instance->fake_power_on();
        s_instance->set_led_color(LED_Color::GREEN);
    }
}

bool System::stand_by(bool _production_final_test)
{
    State_File::App_State_File file;
    DEBUG_MSG(HAL, DEBUG, TERM_GREEN_BOLD "Standby: " << (file.stand_by ? "ON" : "OFF") << "\n" TERM_RESET);

    if(_production_final_test)
    {
        if(file.stand_by_in_production_mode == false)
        {
            s_instance->fake_power_off();
            s_fake_standby_mode = true;
        }
        else
        {
            s_instance->fake_power_on();
            s_fake_standby_mode = false;
        }
    }
    else
    {
        if(file.stand_by == false)
        {
            DEBUG_MSG(HAL, DEBUG, "Fake Standby Mode\n");
            s_instance->fake_power_off();
            s_fake_standby_mode = true;
        }
        else
        {
            s_instance->fake_power_on();
            s_fake_standby_mode = false;
        }
    }

    return s_fake_standby_mode;
}

void System::reboot()
{
    ALI_EXEC(aui_sys_reboot(2000));
}

void System::set_led_color(LED_Color _color)
{
    FILE *fp_red, *fp_green;
    fp_red = fopen("/sys/devices/platform/dtsleds/leds/red/brightness", "w");
    fp_green = fopen("/sys/devices/platform/dtsleds/leds/green/brightness", "w");

    if((fp_red == nullptr) && (fp_green == nullptr))
    {
        DEBUG_MSG(HAL, ERROR, "/sys/devices/platform/dtsleds/leds");
        return;
    }

    switch(_color)
    {
        case LED_Color::RED:
            fwrite("1", 1, 1, fp_red);
            fwrite("0", 1, 1, fp_green);
            break;

        case LED_Color::GREEN:
            fwrite("0", 1, 1, fp_red);
            fwrite("1", 1, 1, fp_green);
            break;

        case LED_Color::ORANGE:
            fwrite("1", 1, 1, fp_red);
            fwrite("1", 1, 1, fp_green);
            break;
    }

    fclose(fp_red);
    fclose(fp_green);
}

void System::fake_power_off()
{
    DEBUG_MSG(HAL, DEBUG, "Fake Power Off\n");
    //change_led_color();
    m_hdmi->hdmi_output_off();
    auto display = Display::get_instance();

    if(display)
    {
        display->set_cvbs_off();
    }
}

void System::fake_power_on()
{
    DEBUG_MSG(HAL, DEBUG, "Fake Power On\n");
    //change_led_color();
    m_hdmi->hdmi_output_on();
    auto display = Display::get_instance();

    if(display)
    {
        display->set_cvbs_on();
    }
}

const char *to_bin(uint8_t _value)
{
    static char buffer[9];

    for(int i = 0; i < 8; i++)
    {
        buffer[7 - i] = (_value & (1 << i)) ? '1' : '0';
    }

    buffer[8] = 0;
    return buffer;
}

System::Suppliers System::read_supplier()
{
#if 0
    /*
        You need disable the jtag function through strap pin before you set this TDI pin as a gpio.
        1. run "devmem 0x1808d820" to check the strap pin, if bit16 of the value is 1, then jtag function is enabled.                                                                                *
        2. run "devmem 0x1808d820 8 1" to write 1 to bit0, then read 0x1808d820 again to check bit16, if it is now 0, then jtag function is disabled.
        3. now you can set the gpio function of this pin by calling "aui_gpio_open" with "struct aui_gpio_attr = {.uc_dev_idx=186, .io=AUI_GPIO_I_DIR};", then call "aui_gpio_read" to get the value.
    */
    auto fd = open("/dev/mem", O_RDWR | O_SYNC);
    off_t page_size = getpagesize();
    off_t offset_in_page = (off_t)STRAP_PIN & (page_size - 1);
    auto map_base = mmap(nullptr,
                         page_size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED,
                         fd,
                         STRAP_PIN & ~(off_t)(page_size - 1));

    if(map_base == MAP_FAILED)
    {
        return Suppliers::UNKNOWN;
    }

    auto virt_addr = (uint8_t *)map_base + offset_in_page;

    if(((*(volatile uint32_t *)virt_addr) & 0x10000) == 0x10000)
    {
        *(volatile uint8_t *)virt_addr = 1;

        if(((*(volatile uint32_t *)virt_addr) & 0x10000) == 0x00000)
        {
            DEBUG_MSG("J-Tag Disabled sucessfully\n");
        }
        else
        {
            DEBUG_MSG("J-Tag Disable failed\n");
            return Suppliers::UNKNOWN;
        }
    }
    else
    {
        DEBUG_MSG("J-Tag Already Disabled.\n");
    }

    Suppliers result;
    GPIO gpio(GPIO_NUM::TDI);

    if(gpio.read() == 0)
    {
        result = Suppliers::Askey;
    }
    else
    {
        result = Suppliers::Skyworth;
    }

    munmap(map_base, page_size);
    close(fd);
    return result;
#endif
    return Suppliers::Askey;
}

bool System::set_system_time(UTC_MJD _time)
{
    aui_clock st_rtc_time = {};
    st_rtc_time.year  = _time.year();
    st_rtc_time.month = _time.month();
    st_rtc_time.date  = _time.day();
    st_rtc_time.day   = 0; // For ALi this is the Day of the Week
    st_rtc_time.hour  = _time.hour();
    st_rtc_time.min   = _time.minute();
    st_rtc_time.sec   = _time.second();
    return aui_rtc_set_time(s_rtc_handler, &st_rtc_time) == 0;
}

// Year, Month, Day, Hour, Minute, Second
UTC_MJD System::get_system_time()
{
    aui_clock st_rtc_time = {};
    auto ret = aui_rtc_get_time(s_rtc_handler, &st_rtc_time);

    if(AUI_RTN_SUCCESS == ret)
    {
        return UTC_MJD{st_rtc_time.year, st_rtc_time.month, st_rtc_time.date,
                       st_rtc_time.hour, st_rtc_time.min, st_rtc_time.sec};
    }

    return UTC_MJD{};
}

bool System::fake_stand_by_mode()
{
    return s_fake_standby_mode;
}

#define NAGRA_MAX_PK_SIZE 0x800
#define NAGRA_MAX_CSCD_SIZE 0x1000
#define NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA 0x9000
#define NAGRA_CSCD_START_ADDR_IN_BOOT_TOTAL_AREA 0x8000

std::string System::get_board_fingerprint()
{
    char buffer[NAGRA_MAX_PK_SIZE + NAGRA_MAX_CSCD_SIZE];

    auto fp = fopen("/dev/mtd0ro", "r");
    if (fp)
    {
        if (fseek(fp, NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA, SEEK_SET) == -1)
        {
            goto ERROR;
        }

        if (fread(buffer, 1, NAGRA_MAX_PK_SIZE, fp) != NAGRA_MAX_PK_SIZE)
        {
            goto ERROR;
        }

        if (fseek(fp, NAGRA_PK_START_ADDR_IN_BOOT_TOTAL_AREA, SEEK_SET) == -1)
        {
            goto ERROR;
        }

        if (fread(buffer + NAGRA_MAX_PK_SIZE, 1, NAGRA_MAX_CSCD_SIZE, fp) != NAGRA_MAX_CSCD_SIZE)
        {
            goto ERROR;
        }

        return {SHA256(buffer, sizeof(buffer))};
    }

ERROR:
    std::cerr << "Error reading fingerprint: " << strerror(errno) << std::endl;
    return {};
}

} // namespace mb
