/**
  @file ngwm_types.h

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

#ifndef NGWM_TYPES_H
#define NGWM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include <stddef.h>
#include <limits.h>

#ifndef _NG_REMAP_DEFS_

/*
  This should be the preferred case unless the targeted platform tool chain
  does not support fixed-size integer introduced in C99 standard.
*/
#include <stdint.h>
#include <stdbool.h>

#else  /* _NV_REMAP_DEFS_ */

#define INT8_MIN        CHAR_MIN
#define INT8_MAX        CHAR_MAX
#define UINT8_MAX       UCHAR_MAX
#define INT16_MIN       SHRT_MIN
#define INT16_MAX       SHRT_MAX
#define UINT16_MAX      USHRT_MAX
#define INT32_MIN       LONG_MIN
#define INT32_MAX       LONG_MAX
#define UINT32_MAX      ULONG_MAX

typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
typedef ptrdiff_t       intptr_t;

#ifndef __cplusplus     /* Boolean is a basic C++ type */
#ifndef bool
typedef uint8_t         bool;
#endif
#ifndef false
#define false           0
#endif
#ifndef true
#define true            ((bool)(!false))
#endif
#endif                  /* __cplusplus */

#endif  /* _NG_REMAP_DEFS_ */

#ifndef FALSE
#define FALSE           ((bool)false)
#endif

#ifndef TRUE
#define TRUE            ((bool)true)
#endif

#ifdef __cplusplus
}
#endif

#endif /* NGWM_TYPES_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
