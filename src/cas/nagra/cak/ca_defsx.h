/**
**  @file ca_defsx.h
**
**  @brief
**    This file defines specific types and constants used by the
**    NagraVision CA software.
**
**  $Id: //CAK/components/glbapi/TAGS/GLBAPI_1_2_1/src/ca_defsx.h#1 $
**
**  COPYRIGHT:
**    2004-2014 Nagravision S.A.
**/

#ifndef CA_DEFSX_H
#define CA_DEFSX_H

#include "ca_types.h"

/**
 *  @brief
 *    Transport session identifier.
 *
 *    An advanced device may manage more than one input stream. For example,
 *    a box may have a stream feed coming from a hard drive (DVR box) or a
 *    second tuner (for the PIP feature). <BR>
 *    In these cases, it is necessary to identify each of these streams
 *    individually. We will call these streams transport session. They are
 *    identified with a transport session identifier provided by the decoder
 *    manufacturer.<BR>
 *    The transport session identifier comes from this interface and is passed
 *    as is to the demultiplexer and descrambler drivers.<BR>
 *    ECMs as well as EMMs can be in any transport session
 *
 *  @see TRANSPORT_SESSION_ID_INVALID
*/
typedef TUnsignedInt32 TTransportSessionId;

/**
 *  @brief
 *    Defines an invalid transport session identifier.
 *
 *  @see  TTransportSessionId
*/
#define TRANSPORT_SESSION_ID_INVALID  ((TTransportSessionId) -1)

/**
 *  @brief
 *    Calendar time structure.
*/
typedef struct
{
  TUnsignedInt8  year;
    /**<  Years since 1900.
    */
  TUnsignedInt8  month;
    /**<  Month of the year (1 to 12).
    */
  TUnsignedInt8  day;
    /**<  Day of the month (1 to 31).
    */
  TUnsignedInt8  hour;
    /**<  Hours since midnight (0 to 23).
    */
  TUnsignedInt8  minute;
    /**<  Minute after the hour (0 to 59).
    */
  TUnsignedInt8  second;
    /**<  Second after the minute (0 to 59).
    */
} TCalendarTime;

/**
 *  @brief
 *    A duration in seconds.
*/
typedef TUnsignedInt32 TCaDuration;

/**
 * @brief
 *   Number of seconds since 1st of January 1970, 00:00:00
*/
typedef TUnsignedInt32    TUnixDate;

#endif /* CA_DEFSX_H */

/* end of file */
