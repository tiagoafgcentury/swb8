/**
  @file ngwm_defs.h

  @brief
  This file defines the general types and constants used by the
  NexGuard product interfaces.


  COPYRIGHT:
    2020 Nagravision S.A.
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @defgroup g_defs_common                 Common definitions
  @brief
  Describe common definitions
*/

#ifndef NGWM_DEFS_H
#define NGWM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include <stddef.h>
#include "ngwm_types.h"




/* ========================================================================== */
/*                                 DEFINITIONS                                */
/* ========================================================================== */

/* Management of interface version within function tables. */

#define NGWM_QUOTE(x)                 #x
#define NGWM_QUOTE_EXPAND(x)          NGWM_QUOTE(x)
#define NGWM_INTERFACE_VERSION_STRING( head, major, medium, minor ) \
    #head NGWM_QUOTE_EXPAND(major) "." NGWM_QUOTE_EXPAND(medium) "." NGWM_QUOTE_EXPAND(minor)
#define NGWM_INTERFACE_VERSION_INT( major, medium, minor ) \
    ( (uint32_t)( (major)<<16 | (medium)<<8 | (minor) ) )

/* ========================================================================== */
/*                                    TYPES                                   */
/* ========================================================================== */




/**
  @ingroup g_defs_common
  An opaque type to reference an embedder object. It can be freely adapted and
  mapped on any implementation.
*/

typedef void* TNgwmEmbedder;


#ifdef __cplusplus
}
#endif

#endif /* defined NGWM_DEFS_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
