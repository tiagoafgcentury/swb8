#include "mb_hash.h"

#include "mb_globals.h"
#include "SHA256.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace mb {

std::tuple<File_Hash, bool> file_hash(std::string_view _file_name)
{
    auto fd = open(_file_name.data(), O_RDONLY);

    if(fd <= 0)
    {
        DEBUG_MSG(COMMON, ERROR, "Error opening " << _file_name << " for hash: " << strerror(errno) << "\n");
        return {0, false};
    }

    struct stat statbuf;

    if(fstat(fd, &statbuf) != 0)
    {
        DEBUG_MSG(COMMON, ERROR, "Error stating " << _file_name << " for hash: " << strerror(errno) << "\n");
        return {0, false};
    }

    char buffer[statbuf.st_size];
    auto read_ptr = &buffer[0];
    auto remain = statbuf.st_size;

    while(remain > 0)
    {
        auto size = read(fd, read_ptr, remain);

        if(size < 0)
        {
            DEBUG_MSG(COMMON, ERROR, "Unable to read " << _file_name << ": " << strerror(errno) << "\n");
            break;
        }

        remain -= size;
        read_ptr += size;
    }

    close(fd);

    if(remain == 0)
    {
        return {SHA256(buffer, statbuf.st_size), true};
    }
    else
    {
        DEBUG_MSG(COMMON, ERROR, "Unable to read all " << _file_name << " " << remain << "\n");
    }

    return {0, false};
}

bool check_file_hash(std::string_view _file_name)
{
    auto hash_file_name = std::string(_file_name) + ".hash";
    auto fd = open(hash_file_name.c_str(), O_RDONLY);

    if(fd <= 0)
    {
        DEBUG_MSG(COMMON, ERROR, "Error opening " << hash_file_name << " for hash: " << strerror(errno) << "\n");
        return false;
    }

    char buffer[1024];
    auto size = read(fd, buffer, 1024);
    close(fd);

    for(auto pos = 0; pos < size; pos++)
    {
        if(isspace(buffer[pos]))
        {
            buffer[pos] = 0;
            break;
        }
    }

    auto [cur_file_hash, success] = file_hash(_file_name);
    return success and strncasecmp(buffer, cur_file_hash.c_str(), std::min<int>(size, cur_file_hash.size())) == 0;
}

void create_file_hash(std::string_view _file_name)
{
    auto hash_file_name = std::string(_file_name) + ".hash";
    auto [cur_file_hash, success] = file_hash(_file_name);

    if(success)
    {
        char buffer[1024];
        auto size = snprintf(buffer, sizeof(buffer), "%s  %s\n", cur_file_hash.c_str(), _file_name.data());
        unlink(hash_file_name.c_str());
        auto fd = creat(hash_file_name.c_str(), S_IRUSR);

        if(fd <= 0)
        {
            DEBUG_MSG(COMMON, ERROR, "Error opening " << hash_file_name << " for hash: " << strerror(errno) << "\n");
            return;
        }

        auto p = buffer;

        while(size > 0)
        {
            auto wrote = write(fd, p, size);
            size -= wrote;
            p += wrote;
        }

        close(fd);
    }
}

} // namespace mb
