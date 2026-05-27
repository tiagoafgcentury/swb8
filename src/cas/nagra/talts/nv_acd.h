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

#include "nv_types.h"

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
  NV_USER_INTENT_RECORD_TIMESHIFT             = 3,
  /**< The descrambling session is used for the rescrambling of content with DVL (for time shift record use case) */
  NV_USER_INTENT_EXPORT_XCODE                 = 4
  /**< The descrambling session is used to enable the transcoding of content before rescrambling of content with DVL for another device */
} TNvUserIntent;

/**
  @ingroup g_defs_common
  @brief
  Define UUID binary representation.

  @details
  UUID represented as a raw 16-bytes array (128 bits).
*/

typedef uint8_t TNvBinUuid[16];

/**
  @ingroup g_defs_common
  @brief
  Define UUID character representation.

  @details
  UUID represented as a character array (not null-terminated)
  for holding its canonical form: @n
  xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx @n
*/

typedef uint8_t TNvCharUuid[36];

/**
  @ingroup g_defs_common
  @brief
  Define binary representation of a hash on 256 bits (output of SHA256 for instance)
*/

typedef uint8_t TNvHash256[32];

#ifdef __cplusplus
}
#endif

#endif /* defined NV_ACD_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
