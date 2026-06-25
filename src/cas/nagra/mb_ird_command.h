#pragma once

#include <common/mb_types.h>
#include "mb_events.h"

#include <functional>
#include <map>
#include <string>
#include <vector>



namespace mb {

enum Nagra_Commands
{
    RESET_PIN_CODE          = 0x12,
    STB_RESET               = 0x54,
    FINGERPRINTING          = 0x56,
    MAIL                    = 0xC0,
    FORCE_IDENTIFICATION    = 0xC2,
    FORCE_STAND_BY          = 0xC9,
    ZONE_ID                 = 0xD8,
    REPORT_SYSTEM_INFORMATION = 0xE2,
    POPUP_MESSAGE           = 0xCF,
};

enum Nagra_Command_Operations
{
    TVKEY_OSD_MESSAGE = 0x02,
};

struct Popup_Message
{
    uint8_t popup_id;
    uint8_t total_segments;
    uint8_t persistence;
    std::vector<std::string> segments;
};

struct Finger_Print_Message
{
    bool enabled {false};
    uint8_t fingerprint_id {0};
    uint8_t flags {0};
    uint16_t duration_seconds {0};
    uint16_t x_position {0};
    uint16_t y_position {0};
    uint8_t transparency {0};
    uint8_t style {0};
    bool blink {false};
    bool scroll {false};
    bool has_text {false};
    std::string text;
};

using Popup_Handler = std::function<void(const Event_Display_Message&)>;

using Finger_Print_Handler = std::function<void(const Finger_Print_Message&)>;

void set_popup_handler(Popup_Handler cb);
void set_fingerprint_handler(Finger_Print_Handler cb);

extern Popup_Handler s_popup_handler;
extern Finger_Print_Handler s_fingerprint_handler;

extern std::map<uint8_t, Popup_Message> s_popup_cache;

void process_ird_command(uint8_t *data, size_t size);
void parse_finger_print(uint8_t* data, size_t size);
void parse_osd_popup_message(uint8_t* data, size_t size);

} // namespace mb
