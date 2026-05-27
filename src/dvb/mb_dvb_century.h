#pragma once

#include <cstdint>

#include "mb_dvb_ota.h"

namespace mb {

class Century final : public OTA
{
public:
    explicit Century(const uint8_t *_data, size_t _size);
    ~Century() {}

    virtual Table_ID_t table_id() const override
    {
        return DSI_TABLE_ID;
    }

private:

    struct OTA_DATA_INFO_t
    {
        unsigned int PRODUCT_ID: 24;
        unsigned long GroupId;
        unsigned short SoftwareVersion;
        unsigned short minVersion;
        unsigned short OTAPid;
        unsigned char UpgradeType;/*00 : Normal    01 : Forced*/
    };

    enum OTA_STATUS
    {
        e_OTA_STATUS_NORMAL = 0,
        e_OTA_STATUS_CHECK_SIGNAL,
        e_OTA_STATUS_SHOW_VERSION,
        e_OTA_STATUS_DOWNLOADING,
        e_OTA_STATUS_DOWNLOAD_PAUSE,
        e_OTA_STATUS_DOWNLOAD_FAIL,
        e_OTA_STATUS_DOWNLOAD_NOINFO,
        e_OTA_STATUS_DOWNLOAD_SUCCESS,
        e_OTA_STATUS_NOSIGNAL,
        e_OTA_STATUS_WRITE,
        e_OTA_STATUS_NONE,
    };

    static constexpr auto CUSTOMER_OUI = 0x0011F5;

    struct OTA_GROUP_INFO_t
    {
        OTA_DATA_INFO_t *ModuleInfo;
        unsigned char ModuleCnt;
        unsigned long GroupId;
    };

    struct OTA_MODULE_INFO_t
    {
        unsigned char   *moduleData;    // Pointer to raw module data

        unsigned long   groupId;        // Transaction Id of DII
        unsigned long   downloadId;
        unsigned short  blockSize;      // Max size of blocks in this Module

        unsigned short  moduleId;       // Id of this module
        unsigned long   moduleSz;       // Size of this module
        unsigned char   version;        // Version of this module

        unsigned short  chkarraycnt;    // Number of bytes in 'blkchkbits'
        unsigned long   *blkchkbits;    // Bit flag marking received blocks

        unsigned short  totBlocks;      // Total number of blocks in module
        unsigned short  rcvBlocks;      // Total received blocks in module
    };

    void dsi_process(uint8_t *_data, size_t _size);

    int  Compatibility_Descriptor_Proccess(unsigned char *pPrivateData);

};

};
