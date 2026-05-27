#pragma once

#include <cinttypes>
#include <string>

namespace tpm {

enum class Partition
{
    boot_total_area,
    uboot,
    dtb,
    bootenv,
    deviceinfo,
    boot_logo,
    see_bl,
    see,
    kernel,
    uboot_redundant,
    persistent_mem,
    db2,
    db2_redundant,
    db3,
    db3_redundant,
    roothash,
    rootfs,
    upg_onepackage,
    mnt_app,
    mnt_vfs,
    UNKNOWN
};

struct Partition_Info
{
    uint8_t dev { 0 };
    uint32_t size { 0 };
    uint32_t erasesize { 0 };
    std::string name;
};

void init_partition_map();

}
