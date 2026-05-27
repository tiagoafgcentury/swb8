/**
 ** @file ca_cakx.h
 **
 ** @brief
 **    Conditionnal Access Kernel external definitions.
 **
 **  COPYRIGHT:
 **    2014 Nagravision S.A.
 **
 ** CLASSIFICATION:
 **   CONFIDENTIAL
 **
 */

#ifndef CA_CAK_EXT_H
#define CA_CAK_EXT_H

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/


#include "ca_defs.h"


/******************************************************************************/
/*                                                                            */
/*                                   TYPES                                    */
/*                                                                            */
/******************************************************************************/

/**
 * @brief
 *   Number of internal counters available in
*/
#define NB_INTERNAL_COUNTERS (14)

/**
 * @brief
 *   CAK internal counters structure
*/
typedef struct
{
  TUnsignedInt32 emmInactivityCounter;
  /**< time elapsed since last processed EMM (in seconds).
  */
  TUnsignedInt32 internalCounters[NB_INTERNAL_COUNTERS];
  /**< Array of internal counters for debug purpose.
  */
} TCaDebugCounters;



/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 * @brief
 *   Gets the EMM inactivity counter and a set of CAK internal counters which
 *   shall be displayed in logs or in the UI for CAK debug purpose.
 *
 *
 * @return a structure containing counters related to EMM processing.
 */
NAGRA_CA_API TCaDebugCounters* caDebugGetEmmCounters
(
  void
);
#ifdef __cplusplus
}
#endif

#endif /* CA_CAK_EXT_H */

/* EOF */
