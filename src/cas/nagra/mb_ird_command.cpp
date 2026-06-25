#include "mb_ird_command.h"

#include <fcntl.h>
#include <unistd.h>
#include <queue>

#include "common/mb_globals.h"

#include "tasks/mb_task.h"

namespace mb {

Popup_Handler s_popup_handler {};
Finger_Print_Handler s_fingerprint_handler {};

std::map<uint8_t, Popup_Message> s_popup_cache {};

namespace {

constexpr auto MAX_SEQUENCE_COUNT = 11;

uint32_t s_last_sequences[MAX_SEQUENCE_COUNT];
unsigned int s_last_sequences_pos = 0;

void init_last_sequences() __attribute__((constructor));

void init_last_sequences()
{
    auto fd = open(MBGUI_NAGRA_IRD_CACHE_FILE, O_RDONLY);

    if(fd > 0)
    {
        auto p = reinterpret_cast<uint8_t *>(s_last_sequences);
        auto sz = sizeof(s_last_sequences);

        do
        {
            auto r = read(fd, p, sz);

            if(r <= 0)
            {
                close(fd);
                goto EXIT_CLEAR_CACHE;
            }

            p += r;
            sz -= r;
        }
        while(sz);

        close(fd);

        for(s_last_sequences_pos = 0; s_last_sequences_pos < MAX_SEQUENCE_COUNT; s_last_sequences_pos++)
        {
            if(s_last_sequences[s_last_sequences_pos] == 0xffffffff)
            {
                return;
            }
        }

        s_last_sequences_pos = 0;
        return;
    }

EXIT_CLEAR_CACHE:
    memset(s_last_sequences, 0xFF, sizeof(s_last_sequences));
}

void flush_last_sequences()
{
    auto fd = open(MBGUI_NAGRA_IRD_CACHE_FILE, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if(fd > 0)
    {
        auto p = reinterpret_cast<uint8_t *>(s_last_sequences);
        auto sz = sizeof(s_last_sequences);

        do
        {
            auto r = write(fd, p, sz);

            if(r < 0)
            {
                break;
            }

            p += r;
            sz -= r;
        }
        while(sz);
    }

    close(fd);
}

}

void process_ird_command(uint8_t *_data, size_t _size)
{
    DEBUG_MSG(CAS, INFO, "Data: " << hex);

    for(auto i = 0u; i < _size; i++)
    {
        DEBUG_MSG_NL(CAS, INFO, HEXBYTE(_data[i]));
    }

    DEBUG_MSG_NL(CAS, INFO, endl);
    auto sequence_number = __builtin_bswap32(*reinterpret_cast<uint32_t *>(_data + 2));

    for(int i = 0; i < MAX_SEQUENCE_COUNT; i++)
    {
        if(s_last_sequences[i] == sequence_number)
        {
            DEBUG_MSG(CAS, INFO, "IRD Command already processed: " << sequence_number << "\n");
            return;
        }
    }

    s_last_sequences[s_last_sequences_pos] = sequence_number;
    s_last_sequences_pos = (s_last_sequences_pos + 1) % MAX_SEQUENCE_COUNT;
    s_last_sequences[s_last_sequences_pos] = 0xffffffff;
    flush_last_sequences();
    uint8_t command_id = _data[6];
    bool addressing_mode = _data[7] & 0b10000000;
    uint8_t operation = _data[7] & 0b00111111;
    _data += 8;
    _size -= 8;
#ifndef NDEBUG
    uint8_t tag = _data[0];
    uint8_t length = _data[1];
    DEBUG_MSG(CAS, INFO, "IRD New Command: " << dec << (int)sequence_number << " " << (int)tag << " " << (int)length << " " << (int)command_id << " " << (int)operation << "\n");
#endif

    uint64_t zone_id = 0;
    if(addressing_mode)
    {
        if(_size < 6)
        {
            DEBUG_MSG(CAS, ERROR, "Invalid IRD command: truncated zone_id\n");
            return;
        }

        zone_id =
            (static_cast<uint64_t>(_data[0]) << 40) |
            (static_cast<uint64_t>(_data[1]) << 32) |
            (static_cast<uint64_t>(_data[2]) << 24) |
            (static_cast<uint64_t>(_data[3]) << 16) |
            (static_cast<uint64_t>(_data[4]) << 8)  |
            (static_cast<uint64_t>(_data[5]));

        DEBUG_MSG(CAS, INFO, "IRD Command addressed to zone: 0x" << std::hex << zone_id << std::dec << "\n");

        _data += 6;
        _size -= 6;
    }

    //switch(static_cast<Nagra_Commands>(command_id))
    switch(command_id)
    {
        case RESET_PIN_CODE:
            DEBUG_MSG(CAS, INFO, "IRD Command: RESET PIN CODE\n");
            //Task::post_event_pin_reset();
            break;

        case STB_RESET:
            break;

        case FINGERPRINTING:
            printf("\n ************************************* \n");
            DEBUG_MSG(CAS, INFO, "FINGER PRINT COMMAND RECEIVED!");
            printf("\n ************************************* \n");
            if(operation == 0x00)
            {
                parse_finger_print(_data, _size);
            }
            break;

        case MAIL:
            break;

        case FORCE_IDENTIFICATION:
            break;

        case FORCE_STAND_BY:
            break;

        case ZONE_ID:
        {
            if(operation == 0x00)
            {
                if(_size < 6)
                {
                    DEBUG_MSG(CAS, ERROR, "Invalid SET ZONE ID payload\n");
                    break;
                }

                Zone_ID_t zone_id = __builtin_bswap32(*reinterpret_cast<uint32_t*>(_data + 2));
                Segment_ID_t segment_id = __builtin_bswap32(*reinterpret_cast<uint32_t*>(_data + 2));
                DEBUG_MSG(CAS, INFO, "IRD Command: SET ZONE ID: " << zone_id << "\n");
                Task::post_event_lineup_save_zone_id(zone_id, segment_id);
            }

            break;
        }

        case REPORT_SYSTEM_INFORMATION:
            break;

        case POPUP_MESSAGE:
            parse_osd_popup_message(_data, _size);
            break;

        default:
            DEBUG_MSG(CAS, INFO, "COMMAND_ID UNKNOWN: " << dec << (int)command_id << "\n");
            break;
    }
}

void set_popup_handler(Popup_Handler cb)
{
    s_popup_handler = std::move(cb);
}

void set_fingerprint_handler(Finger_Print_Handler cb)
{
    s_fingerprint_handler = std::move(cb);
}

void parse_finger_print(uint8_t* data, size_t size)
{
    if (!data || size < 2)
        return;

    Finger_Print_Message fp;

    fp.enabled = true;
    fp.fingerprint_id = data[0];
    fp.flags          = data[1];
    fp.blink  = (fp.flags & 0x01);
    fp.scroll = (fp.flags & 0x02);

    if(size >= 4)
    {
        fp.duration_seconds = (static_cast<uint16_t>(data[2]) << 8) | static_cast<uint16_t>(data[3]);
    }

    if(size >= 6)
    {
        fp.y_position = (static_cast<uint16_t>(data[4]) << 8) | static_cast<uint16_t>(data[5]);
    }

    constexpr size_t TEXT_OFFSET = 6;

    if(size > TEXT_OFFSET)
    {
        std::string text;

        for(size_t i = TEXT_OFFSET; i < size; i++)
        {
            uint8_t c = data[i];

            if(c >= 32 && c <= 126)
            {
                text.push_back(static_cast<char>(c));
            }
        }

        if(!text.empty())
        {
            fp.has_text = true;
            fp.text = text;
        }
    }

    DEBUG_MSG(CAS, INFO,
        "FINGERPRINT:"
        << " id=" << dec << (int)fp.fingerprint_id
        << " flags=0x" << hex << (int)fp.flags
        << " duration=" << dec << fp.duration_seconds
        << " y=" << fp.y_position
        << " blink=" << fp.blink
        << " scroll=" << fp.scroll
        << " text=\"" << fp.text << "\"\n"
    );

    if(s_fingerprint_handler)
    {
        s_fingerprint_handler(fp);
    }

    if(s_popup_handler)
    {
        Event_Display_Message event;

        std::stringstream ss;

        ss << "{";
        ss << "\"message\":\"" << fp.text << "\",";
        ss << "\"type\":\"fingerprint\",";
        ss << "\"blink\":" << (fp.blink ? "true" : "false") << ",";
        ss << "\"scroll\":" << (fp.scroll ? "true" : "false") << ",";
        ss << "\"y_position\":" << fp.y_position << ",";
        ss << "\"style\":" << (int)fp.style;
        ss << "}";

        event.message  = ss.str();
        event.category = Message_Categories::Event_Finger_Print;

        if(fp.duration_seconds > 0)
        {
            event.timeout = std::chrono::milliseconds(fp.duration_seconds * 1000);
        }
        else
        {
            event.timeout = std::chrono::milliseconds(10000);
        }

        s_popup_handler(event);
    }
}

void parse_osd_popup_message(uint8_t* data, size_t size)
{

    if (!data || size < 17)
        return;

    constexpr size_t HEADER_SIZE = 16;
    constexpr size_t CRC_SIZE    = 1;

    if (size <= HEADER_SIZE + CRC_SIZE)
        return;

    uint8_t popup_id = data[0];

    uint8_t total_segments = (data[1] >> 2) & 0x3F;
    uint8_t persistence    = data[1] & 0x03;

    uint8_t segment_number = (data[2] >> 2) & 0x3F;

    if (segment_number >= total_segments)
        return;

    size_t text_offset = HEADER_SIZE;
    size_t text_len    = size - HEADER_SIZE - CRC_SIZE;

    std::string text(
        reinterpret_cast<char*>(data + text_offset),
        text_len
    );

    // Cache do popup
    auto& popup = s_popup_cache[popup_id];

    if (popup.segments.empty())
    {
        popup.popup_id       = popup_id;
        popup.total_segments = total_segments;
        popup.persistence    = persistence;
        popup.segments.resize(total_segments);
    }

    popup.segments[segment_number] = text;

    // Concatena apenas os segmentos recebidos
    std::string full_message;
    for (const auto& seg : popup.segments)
    {
        if (!seg.empty())
            full_message += seg;
    }

    if (full_message.empty())
        return;

    if (s_popup_handler)
    {
        Event_Display_Message event;

        event.message  = full_message;
        event.category = Message_Categories::Event_Popup;
        event.timeout = std::chrono::milliseconds(10000);

        s_popup_handler(event);
    }

    // Limpa cache (popup substitui anterior)
    s_popup_cache.erase(popup_id);

}




} // namespace mb
