#include "mb_dvb_century.h"
#include "common/mb_globals.h"
#include "common/mb_assert.h"
#include "mb_events.h"

#include <string.h>
#include <algorithm>


#define GET_2BYTE( cp ) ( ( ( (unsigned short)(*cp) ) << 8 ) | ( ( (unsigned short)(*(cp+1)) ) ) )
#define GET_3BYTE( cp ) ( ( ( (unsigned long)(*cp) )<< 16 ) | ( ( (unsigned long)(*(cp+1)) ) <<  8 ) | ( ( (unsigned long)(*(cp+2)) ) ) )
#define GET_4BYTE( cp ) ( ( ( (unsigned long)(*cp) )<< 24 ) | ( ( (unsigned long)(*(cp+1)) ) << 16 ) | ( ( (unsigned long)(*(cp+2)) ) << 8 ) | ( (unsigned long)(*(cp+3)) ) )

#define SECTION_LENGTH( h ) ( ( ( *( h + 1 ) & 0x0F ) << 8 ) | ( * ( h + 2 ) ) )
#define GET_DSMCC_MSG_ID(p)  (((unsigned short)(*(p+SECTION_HEADER_SIZE + 2)) << 8 ) | ((unsigned short)(*(p+SECTION_HEADER_SIZE + 3))))

#define _BITMASKREFPOS  ( 0x80000000 )
#define _ISRCVDBLOCK( blknum ) ( ( g_OTA_Mod_Info.blkchkbits[ blknum / 32 ] ) & ( _BITMASKREFPm_ota_event_callbackOS >> ( blknum % 32 ) ) )
#define _SETNEWBLOCK( blknum ) ( ( g_OTA_Mod_Info.blkchkbits[ blknum / 32 ] ) |= ( _BITMASKREFPOS >> ( blknum % 32 ) ) )

#define SECTION_HEADER_SIZE 8
#define DSMMSG_HEADER_SIZE  12
#define DSIMSG_HEADER_SIZE  20
#define DIIMSG_HEADER_SIZE  20
#define DDBMSG_HEADER_SIZE  6

#define MAX_DDB_MSGSIZE 4066

#define OTA_DSMCC_MSGID_DSI     0x1006  // DSI
#define OTA_DSMCC_MSGID_DII     0x1002  // DII
#define OTA_DSMCC_MSGID_DDB     0x1003  // DDB

namespace mb {

Century::Century(const uint8_t *_data, size_t _size)
{
    mb_assert(table_id() == _data[0]);

    if(table_id() != _data[0])
    {
        return;
    }

    // Verifica mensagem
    unsigned short messageid = 0;
    messageid = GET_DSMCC_MSG_ID(_data);

    if(messageid == OTA_DSMCC_MSGID_DSI)
    {
        dsi_process((uint8_t *)_data, _size);
        return;
    }
}

void Century::dsi_process(uint8_t *section, size_t _size)
{
#ifdef __DEBUG_DSI__
    auto _p = section;

    for(auto counter = 0 ; counter < _size ; ++ counter)
    {
        if(counter % 10 == 0)
        {
            std::cout << "\n";
        }

        DEBUG_MSG(hex << (int)*_p);
        ++_p;
    }

    std::cout << "\n";
#endif
    auto ptr = section + SECTION_HEADER_SIZE + 2;
    unsigned short messageid = GET_2BYTE(ptr);
    auto section_length = SECTION_LENGTH(section);
    ptr =  section + SECTION_HEADER_SIZE;

    if(section_length <= SECTION_HEADER_SIZE + DSMMSG_HEADER_SIZE + DSIMSG_HEADER_SIZE)
    {
        DEBUG_MSG(DVB, ERROR, "[msAPI_OAD_ProcessDSI : dsmcc_section_length = 0x" << hex << section_length << "\n");
        return;
    }

    // DSM MSG Header
    ptr += 4;   // protocolDiscriminator, dsmccType, messageId pass
    ptr += 5;   // trans_id, reserved pass
    auto adaptLength = *ptr++;
    ptr += 2;
    // DSI Payload
    ptr += adaptLength;
    ptr += 20;  //serverId pass
    auto compatibilityLength = GET_2BYTE(ptr);
    ptr += 2;
    ptr += compatibilityLength;
    auto privateDataLength = GET_2BYTE(ptr);
    ptr += 2;

    if(privateDataLength < 2)
    {
        DEBUG_MSG(DVB, ERROR, "msAPI_OAD_ProcessDSI : privateDataLength = 0x" << hex << privateDataLength << "\n");
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Here is privateDataByte
    ////////////////////////////////////////////////////////////////////////////
    auto number_of_groups = GET_2BYTE(ptr);
    ptr += 2;

    for(auto count = 0; count < number_of_groups; count++)
    {
        unsigned long group_id = GET_4BYTE(ptr);
        ptr += 8;
        Compatibility_Descriptor_Proccess(ptr);
        // compatibilityLength
        compatibilityLength = GET_2BYTE(ptr);
        ptr += (compatibilityLength + 2);
        // GroupInfoLength
        auto groupInfoLength = GET_2BYTE(ptr);
        ptr += (groupInfoLength + 2);
        // privateDataLength
        privateDataLength = GET_2BYTE(ptr);
        ptr += (privateDataLength + 2);
    }
}

int Century::Compatibility_Descriptor_Proccess(unsigned char *pPrivateData)
{
    unsigned short  compatibilityLength, descriptorCount;
    compatibilityLength = GET_2BYTE(pPrivateData);
    pPrivateData += 2;

    if(compatibilityLength < 2)
    {
        DEBUG_MSG(DVB, ERROR, "CompatibilityDescriptor_Proccess : compatibilityLength = 0x" << hex << compatibilityLength << "\n");
        return -1;
    }

    descriptorCount = GET_2BYTE(pPrivateData);
    pPrivateData += 2;

    for(int i = 0; i < descriptorCount; i ++)
    {
        auto descriptorType = *pPrivateData++;
        auto descriptorLength = *pPrivateData++;

        switch(descriptorType)
        {
            case 0x01://HW Version.
            case 0x02://SW version.
            {
                OTA_File file;
                file.type = OTA_Type::Century_DSI;
#ifdef OTA_UPGRADE_DEBUG_ON
                specifierType = *pPrivateData++;
#else
                pPrivateData++;
#endif
                file.product_id = GET_3BYTE(pPrivateData);
                pPrivateData += 3;
                file.sw_min = GET_2BYTE(pPrivateData);
                pPrivateData += 2;
                file.sw_current = GET_2BYTE(pPrivateData);
                pPrivateData += 2;
                pPrivateData++;
                _ota_files.push_back(std::move(file));
            }
            break;

            default:
            {
                pPrivateData += descriptorLength;
                DEBUG_MSG(DVB, ERROR, "Error !! Wrong descriptorType = 0x" << hex << descriptorType << "\n");
                break;
            }
        }
    }

    return 0;
}

} // namespace mb
