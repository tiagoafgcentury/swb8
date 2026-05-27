/**
  @file nv_acd.h

  @brief
  This file defines types and constants that are common to different Nagra product
  application interfaces.

  This file must not contain definitions linked to the driver level.


  COPYRIGHT:
    2016-2017 Nagravision S.A.
*/

/* ========================================================================== */
/*                         DOCUMENTATION ORGANISATION                         */
/* -------------------------------------------------------------------------- */
/* This file is always part of other documentation and has no independent     */
/* documentation.                                                             */
/* ========================================================================== */

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_defs_common
  @brief
  Describe common definitions used by product interfaces.
*/

#ifndef NV_ACD_H
#define NV_ACD_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */


/* ========================================================================== */
/*                                 DEFINITIONS                                */
/* ========================================================================== */


/* ========================================================================== */
/*                                    TYPES                                   */
/* ========================================================================== */

/**
  @ingroup g_defs_common
  @brief
  Define user intent value.
*/

typedef enum
{
  NV_USER_INTENT_WATCH                        = 0,
  /**< The descrambling session is used to display video on the screen. Content may come from
       any sources (e.g. live, VOD, PVR recorded content, timeshift review buffer) */
  NV_USER_INTENT_RECORD                       = 1,
  /**< The descrambling session is used for the rescrambling of content with DVL (for PVR) */
  NV_USER_INTENT_EXPORT                       = 2,
  /**< The descrambling session is used for the rescrambling of content with a link protection for export (DTCP-IP, NAGRA LPL...) */
  NV_USER_INTENT_RECORD_TIMESHIFT                    = 3
  /**< The descrambling session is used for the rescrambling of content with DVL (for time shift record use case) */
} TNvUserIntent;

#ifdef __cplusplus
}
#endif

#endif /* defined NV_ACD_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
