#include "mb_globals.h"

#include "dvb/mb_dvb_utc_mjd.h"

#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <cctype>

namespace mb {

const std::array<const std::string_view, 6> COMPATIBLE_VIDEO_FORMATS = { ".mp4", ".mov", ".m4a", ".3gp", ".3g2", ".mj2" };
const std::array<const std::string_view, 3> COMPATIBLE_AUDIO_FORMATS = { ".aac", ".wma", ".mp3" };
const std::array<const std::string_view, 2> COMPATIBLE_PHOTO_FORMATS = { ".jpg", ".png" };
const std::array<const std::string_view, 1> COMPATIBLE_RECORDING_FORMATS = { ".ts" };

int8_t g_timezone_offset { -3 };
uint8_t g_clock_set { 0 };
bool g_factory_reset_enabled { false };
bool g_production_final_test { false };

std::string cat(const char *_origin)
{
    std::string result;
    auto src = scoped_var(open(_origin, O_RDONLY), close);

    if(src < 0)
    {
        DEBUG_MSG(COMMON, ERROR, "Error opening " << _origin << ": " << strerror(errno) << "\n");
    }
    else
    {
        struct stat statbuf;

        if(fstat(src, &statbuf) == 0)
        {
            result.resize(statbuf.st_size);
            auto p = result.data();
            const auto end = p + result.size();
            ssize_t rd = 0;

            while(true)
            {
                rd = read(src, p, end - p);

                if(rd == 0)
                {
                    goto EXIT_CAT;
                }
                else if(rd < 0)
                {
                    DEBUG_MSG(COMMON, ERROR, "Error reading " << _origin << ": " << strerror(errno) << "\n");
                    goto EXIT_CAT;
                }

                p += rd;
            }
        }
    }

EXIT_CAT:
    return result;
}

std::string sanitize_filename(std::string_view _name)
{
    std::string result;
    result.reserve(_name.size());

    for(unsigned char c : _name)
    {
        if(c == ' ')
        {
            result.push_back('_');
        }
        else if(std::isprint(c))
        {
            switch(c)
            {
                case '\\':
                case '/':
                case ':':
                case '*':
                case '?':
                case '"':
                case '<':
                case '>':
                case '|':
                    break;
                default:
                    result.push_back(c);
                    break;
            }
        }
    }

    return result;
}

void cat(const char *_origin, const char *_dest)
{
    auto src = scoped_var(open(_origin, O_RDONLY), close);

    if(src < 0)
    {
        DEBUG_MSG(COMMON, ERROR, "Error opening " << _origin << ": " << strerror(errno) << "\n");
    }
    else
    {
        auto dst = scoped_var(open(_dest, O_WRONLY), close);

        if(dst < 0)
        {
            DEBUG_MSG(COMMON, ERROR, "Error opening " << _dest << ": " << strerror(errno) << "\n");
        }
        else
        {
            char buffer[8192];
            ssize_t rd = 0, wr = 0;

            while(true)
            {
                rd = read(src, buffer, sizeof(buffer));

                if(rd == 0)
                {
                    return;
                }
                else if(rd < 0)
                {
                    DEBUG_MSG(COMMON, ERROR, "Error reading " << _origin << ": " << strerror(errno) << "\n");
                    return;
                }

                auto p = buffer;

                while(rd > 0)
                {
                    wr = write(dst, p, rd);

                    if(wr < 0)
                    {
                        DEBUG_MSG(COMMON, ERROR, "Error writting " << _dest << ": " << strerror(errno) << "\n");
                        return;
                    }

                    p += wr;
                    rd -= wr;
                }
            }
        }
    }
}

#ifndef NDEBUG
bool debuggerIsAttached()
{
    char buf[4096];
    auto status_fd = scoped_var(open("/proc/self/status", O_RDONLY), close);

    if(status_fd == -1)
    {
        return false;
    }

    const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);

    if(num_read <= 0)
    {
        return false;
    }

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const auto tracer_pid_ptr = strstr(buf, tracerPidString);

    if(!tracer_pid_ptr)
    {
        return false;
    }

    for(const char *characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr)
    {
        if(isspace(*characterPtr))
        {
            continue;
        }
        else
        {
            return isdigit(*characterPtr) != 0 && *characterPtr != '0';
        }
    }

    return false;
}

#endif

std::string convert_iso8859_1(const uint8_t *_data, size_t _size)
{
   if (_size == 0)
    {
        return std::string{};
    }

    if (_data[0] == 0x15)
    {
        return std::string{reinterpret_cast<const char *>(_data + 1), _size - 1};
    }

    if (_data[0] > 0 and _data[0] < 0x20)
    {
        _data++;
        _size--;
    }

    std::string result;
    auto end { _data + _size };
    size_t new_size { _size };

    for(auto ch = _data; ch < end; ++ch)
    {
        if(*ch >= 0x80)
        {
            new_size++;
        }
    }

    result.reserve(new_size);

    for(; _data < end; ++_data)
    {
        auto ch = *_data;

        if(ch < 0x80)
        {
            result.push_back(ch);
        }
        else
        {
            result.push_back(0xc0 | ch >> 6);
            result.push_back(0x80 | (ch & 0x3f));
        }
    }

    return result;
}

size_t utf_charcount(std::string_view _str)
{
    auto p { _str.data() };
    auto end { p + _str.size() };
    size_t result { 0 };

    while(p < end)
    {
        if((*p & 0b10000000) == 0)
        {
            result++;
            p++;
        }
        else if((*p & 0b11000000) == 0b11000000)
        {
            result += 2;
            p += 2;
        }
        else if((*p & 0b11100000) == 0b11100000)
        {
            result += 3;
            p += 3;
        }
        else if((*p & 0b11110000) == 0b11110000)
        {
            result += 4;
            p += 4;
        }
    }

    return result;
}

bool run_process(const char *file, const char *const argv[])
{
    const auto pid = fork();

    switch(pid)
    {
        case -1: // Error
        {
            DEBUG_MSG(COMMON, ERROR, "fork() error:" << strerror(errno) << "\n");
            return false;
        }

        case 0: // Child
        {
            DEBUG_MSG(COMMON, DEBUG, "child started: '" << argv[0] << "'\n");
            execv(file, const_cast<char *const *>(argv));
            break;
        }

        default: // Parent
        {
            DEBUG_MSG(COMMON, DEBUG, "child pid: '" << pid << "'\n");
            break;
        }
    }

    return true;
};

} // namespace mb
