/**
** @file ca_mpeg.h
**
** @brief
**   This file defines some constants in relation with MPEG standard.
**   It is used by both the DMX and the PSI interfaces.
**
**  $Id: //CAK/components/glbapi/TAGS/GLBAPI_1_2_1/src/ca_mpeg.h#1 $
**
** COPYRIGHT:
**   2001-2014 Nagravision S.A.
**
**
*/

#ifndef CA_MPEG_H
#define CA_MPEG_H

#include "ca_defs.h"

/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    MPEG-2 stream type as defined in ISO/IEC 13818-1.
 *
 *    Beware of using stream type values defined in this document. They are
 *    deprecated and will no longer evolve. They are maintained in this document
 *    for backward compatibility reasons only. New types introduced in the
 *    ISO/IEC standard have to be defined by the decoder application.
*/
typedef TUnsignedInt8  TMpegStreamType;

/**
 *  @brief
 *    MPEG-2 Packet Identifier.
 *
 *  @see PID_NULL, PID_INVALID
*/
typedef TUnsignedInt16 TPid;


/**
 *  @brief
 *    MPEG-2 Program Number.
 *
*/
typedef TUnsignedInt16 TProgramNumber;


/**
 *  @brief
 *    Conditional Access System Identifier.
*/
typedef TUnsignedInt16 TCaSystemId;

/******************************************************************************/
/*                                                                            */
/*                           CONSTANTS DEFINITIONS                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    MPEG PID of NULL packets.
 *
 *  @see TPid
*/
#define PID_NULL  ((TPid) 0x1FFF)


/**
 *  @brief
 *    Invalid MPEG PID.
 *
 *  @see TPid
*/
#define PID_INVALID ((TPid) 0xFFFF)


/**
 *  @brief
 *    Defines the maximum valid value for an MPEG PID. Mpeg Pids are stored on
 *    13 bits.
 *
 *  @see TPid
*/
#define PID_MAX_VALUE ((TPid) 0x1FFF)


/**
 *  @brief
 *    MPEG program number of the NIT.
 *
 *  @see TProgramNumber
*/
#define PROGRAM_NUMBER_NIT ((TProgramNumber) 0)



/** @cond APA_NO_EXPORT */
/**
 *  @brief
 *    Size of the header of an MPEG section: table identifier + section syntax
 *    indicator + size.
 *
*/
#define MPEG_SECTION_HEADER_SIZE  3
/** @endcond */



/**
 *  @brief
 *    Deprecated (Reserved)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_RESERVED          ((TMpegStreamType)0x00)
/**
 *  @brief
 *    Deprecated (ISO/IEC 11172 Video)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_VIDEO             ((TMpegStreamType)0x01)
/**
 *  @brief
 *    Deprecated (H.262 | ISO/IEC 13818-2 Video)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_VIDEO_13818       ((TMpegStreamType)0x02)
/**
 *  @brief
 *    Deprecated (ISO/IEC 11172 Audio)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_AUDIO             ((TMpegStreamType)0x03)
/**
 *  @brief
 *    Deprecated (ISO/IEC 13818-3 Audio)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_AUDIO_13818       ((TMpegStreamType)0x04)
/**
 *  @brief
 *    Deprecated (H.222.0 | ISO/IEC 13818-1 Private section)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_PRIVATE_SECTIONS  ((TMpegStreamType)0x05)
/**
 *  @brief
 *    Deprecated (H.222.0 | ISO/IEC 13818-1 PES packets containing private data.)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_PRIVATE_DATA      ((TMpegStreamType)0x06)
/**
 *  @brief
 *    Deprecated (ISO/IEC 13522 MHEG)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_MHEG              ((TMpegStreamType)0x07)
/**
 *  @brief
 *    Deprecated (H.222.0 | ISO/IEC 13818-1 DSM-CC)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_DSMCC             ((TMpegStreamType)0x08)
/**
 *  @brief
 *    Deprecated (H.222.0 | ISO/IEC 13818-1/11172-1 auxiliary)
 *
 *  @see TMpegStreamType
 *
 *  @deprecated
*/
#define     MPEG_TYPE_AUXILIARY         ((TMpegStreamType)0x09)

#endif /* CA_MPEG_H */
