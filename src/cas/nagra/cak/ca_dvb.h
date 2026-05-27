/**
** @file ca_dvb.h
**
** @brief
**   This file defines some constants and typesin relation with DVB standards.
**
**  $Id: //CAK/components/glbapi/TAGS/GLBAPI_1_2_1/src/ca_dvb.h#1 $
**
** COPYRIGHT:
**   2004-2014 Nagravision S.A.
**
*/

#ifndef CA_DVB_H
#define CA_DVB_H

#include "ca_defs.h"

/******************************************************************************/
/*                                                                            */
/*                           CONSTANTS DEFINITIONS                            */
/*                                                                            */
/******************************************************************************/

/* None */

/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    DVB Original Network Identifier (ONid).
*/
typedef TUnsignedInt16 TDvbOriginalNetworkId;

/**
 *  @brief
 *    DVB Transport Stream Identifier (TSid).
*/
typedef TUnsignedInt16 TDvbTransportStreamId;

/**
 *  @brief
 *    DVB Service Identifier (Service_id).
*/
typedef TUnsignedInt16 TDvbServiceId;


/**
 *  @brief
 *    DVB Triplet, that uniquely identifies a program on a DBV network.
*/
typedef struct
{
  TDvbOriginalNetworkId  originalNetworkId;
  TDvbTransportStreamId  transportStreamId;
  TDvbServiceId          serviceId;
} TDvbTriplet;

#endif /* CA_DVB_H */

/* End of file */
