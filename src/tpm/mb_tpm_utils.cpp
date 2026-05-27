#include "mb_tpm_utils.h"

#include <unordered_map>

#include <cstring>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <string_view>

#include "common/mb_hash.h"

namespace tpm {

char dump_buffer[8192];

std::string_view to_str(const Partition_Info &_info)
{
    snprintf(dump_buffer, sizeof(dump_buffer), "%s - Dev: %d, Size: %d, Erase Size: %d", _info.name.c_str(), _info.dev, _info.size, _info.erasesize);
    return dump_buffer;
}

std::string_view to_str(const Partition _part)
{
    switch(_part)
    {
#define PART_TO_STR(P) case Partition:: P : return # P;
            PART_TO_STR(boot_total_area);
            PART_TO_STR(uboot);
            PART_TO_STR(dtb);
            PART_TO_STR(bootenv);
            PART_TO_STR(deviceinfo);
            PART_TO_STR(boot_logo);
            PART_TO_STR(see_bl);
            PART_TO_STR(see);
            PART_TO_STR(kernel);
            PART_TO_STR(uboot_redundant);
            PART_TO_STR(persistent_mem);
            PART_TO_STR(db2);
            PART_TO_STR(db2_redundant);
            PART_TO_STR(db3);
            PART_TO_STR(db3_redundant);
            PART_TO_STR(roothash);
            PART_TO_STR(rootfs);
            PART_TO_STR(upg_onepackage);
            PART_TO_STR(mnt_app);
            PART_TO_STR(mnt_vfs);

        case Partition::UNKNOWN:
            break;
    }

    return "UNKNOWN";
}

Partition from_str(const std::string_view _part)
{
    using namespace mb;

    switch(mb::l_strhash(_part))
    {
#define PART_FROM_STR(P) case # P ## _lhash : return Partition:: P;
            PART_FROM_STR(boot_total_area);
            PART_FROM_STR(uboot);
            PART_FROM_STR(dtb);
            PART_FROM_STR(bootenv);
            PART_FROM_STR(deviceinfo);
            PART_FROM_STR(boot_logo);
            PART_FROM_STR(see_bl);
            PART_FROM_STR(see);
            PART_FROM_STR(kernel);
            PART_FROM_STR(uboot_redundant);
            PART_FROM_STR(persistent_mem);
            PART_FROM_STR(db2);
            PART_FROM_STR(db2_redundant);
            PART_FROM_STR(db3);
            PART_FROM_STR(db3_redundant);
            PART_FROM_STR(roothash);
            PART_FROM_STR(rootfs);
            PART_FROM_STR(upg_onepackage);
            PART_FROM_STR(mnt_app);
            PART_FROM_STR(mnt_vfs);
    }

    return Partition::UNKNOWN;
}

std::unordered_map<Partition, Partition_Info> g_partition_map;

void init_partition_map()
{
    /*
     * mtd0: 00080000 00010000 "boot_total_area"
     * mtd1: 00050000 00010000 "uboot"
     */
    auto mtd = open("/proc/mtd", O_RDONLY);

    if(mtd)
    {
        char buffer[8192];
        ssize_t r;
        r = read(mtd, buffer, sizeof(buffer));
        close(mtd);

        if(r > 0)
        {
            buffer[r] = 0;
        }
        else
        {
            return;
        }

        auto start = buffer;
        auto p = start;
        auto eof = start + r;
#define FIND_NEXT(CHAR) \
    while (*p != CHAR)              \
    {                               \
        p++;                        \
        if (p >= eof)               \
        {                           \
            goto END_OF_FILE;       \
        }                           \
    }
        // Ignore first line
        {
            FIND_NEXT('\n');
            p++;
        }

        while(p < eof)
        {
            Partition_Info info;
            info.dev = strtol(p + 3, nullptr, 10);
            FIND_NEXT(' ');
            p++;
            info.size = strtoll(p, nullptr, 16);
            FIND_NEXT(' ');
            p++;
            info.erasesize = strtoll(p, nullptr, 16);
            FIND_NEXT('"');
            start = p + 1;
            FIND_NEXT('\n');
            info.name = std::string(start, p - start - 1);
            p++;
            auto part = from_str(info.name);
            g_partition_map[part] = std::move(info);
        }

END_OF_FILE:

        for(const auto &info : g_partition_map)
        {
            printf("Part: %s\n", to_str(info.second).data());
        }
    }
}

}
