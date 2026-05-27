/**
  @file nv_defs.h

  @brief
  This file defines the general types and constants used by the
  Nagra product interfaces.

  It focuses on defining fixed-size integer types and boolean types.

  COPYRIGHT:
    2013 - 2015 Nagravision S.A.
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
  @addtogroup g_defs
  @brief
  Describe all common definitions used by product interfaces.

  @details

  Basic definitions include all basic types used by Nagra product that are
  used and may sometimes be left undefined by compilation tools.

  Common definitions are common Nagra specific definitions.
*/

/**
  @defgroup g_defs_basics                 Basic definitions
  @brief
  Describe basic definitions used by product interfaces.
  @ingroup g_defs

  @details

  The Nagra product interfaces uses the exact-width integer types introduced
  with the C99 version of the C language. If these definitions are not
  supported by the compilation tools, they must be locally remapped on the
  best suitable supported integer types. By defining @c \_NV_REMAP_DEFS_ at
  compilation time, a default remapping is defined.

  @warning
  Please ensure that the compilation flags used defining the default types are
  the same than those used for generating the Nagra product libraries.
*/

/**
  @defgroup g_defs_common                 Common definitions
  @brief
  Describe common definitions used by product interfaces.
  @ingroup g_defs

  @details

  The following definitions are Nagra-specific and commonly used in the
  present Nagra interfaces.
*/

#ifndef NV_DEFS_H
#define NV_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include <stddef.h>
#include <limits.h>

#ifdef CA_INCLUDE_OBFUSCATE_SYMBOLS
#include "symbols.h"
#endif /* CA_INCLUDE_OBFUSCATE_SYMBOLS */

#include "nv_types.h"

/* ========================================================================== */
/*                                 DEFINITIONS                                */
/* ========================================================================== */

#ifndef NV_PUBLIC_API
#define NV_PUBLIC_API
#endif

/* Management of interface version within function tables. */

#define NV_QUOTE(x)                 #x
#define NV_QUOTE_EXPAND(x)          NV_QUOTE(x)
#define NV_INTERFACE_VERSION_STRING( head, major, medium, minor ) \
    #head NV_QUOTE_EXPAND(major) "." NV_QUOTE_EXPAND(medium) "." NV_QUOTE_EXPAND(minor)
#define NV_INTERFACE_VERSION_INT( major, medium, minor )  ( (major)<<16 | (medium)<<8 | (minor) )

/* ========================================================================== */
/*                                    TYPES                                   */
/* ========================================================================== */

/**
  @addtogroup g_defs_basics
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Boolean types
  Boolean type is aliased to C99 @c bool type. Associated constant FALSE and
  TRUE are redirected to the C99 definitions.
  @{
*/

typedef bool                   bool_t;

/**
  @brief
  False boolean value.
	@hideinitializer
*/
# ifndef FALSE
#   define FALSE               ((bool_t)false)
# endif

/**
  @brief
  True boolean value.
	@hideinitializer
*/
# ifndef TRUE
#   define TRUE                ((bool_t)(!FALSE))
# endif

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* MIN, MAX redefinition. */

/**
  @addtogroup g_defs_basics
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name MIN and MAX macro-instruction
  @{
*/

/**
  @brief
  Provide the minimal value between @c a and @c b.

  @hideinitializer
*/
#ifndef MIN
# define MIN(a,b) ( ( (a) < (b) ) ? (a) : (b) )
#endif

/**
  @brief
  Provide the maximal value between @c a and @c b.

  @hideinitializer
*/
#ifndef MAX
# define MAX(a,b) ( ( (a) > (b) ) ? (a) : (b) )
#endif

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/


/**
  @ingroup g_defs_common
  @brief
  Define a data memory buffer.

  @details
  The buffer type includes a memory area address and the size of this memory
  area.
*/

typedef struct
{
  void*       data;
  /**< Address of the buffer data memory area. */
  uint32_t    size;
  /**< Size of the buffer data memory area. */
}
TNvBuffer;

/**
  @ingroup g_defs_common
  @brief
  Define a generic identifier.

  @details
  A generic identifier is implemented as a buffer.
  The memory data area may contain a numeric or string identifier depending
  on the context of use.
*/

typedef TNvBuffer TNvIdentifier;

/**
  @ingroup g_defs_common
  @brief
  Define a generic handle.

  @details
  A generic opaque handle can be freely adapted and mapped on any
  implementation.
*/

typedef void* TNvHandle;

/**
  @ingroup g_defs_common
  @brief
  Define a session identifier.

  @details
  A generic session identifier to be used for all session identification and
  handling.
*/

typedef uint32_t TNvSession;

/**
  @ingroup g_defs_common
  @brief
  Invalid session identifier value.
*/

#define NV_SESSION_INVALID      ((uint32_t)(-1))

/**
  @ingroup g_defs_common
  @brief
  Define a string of byte characters.

  @details
  Defines a null-terminated string as a pointer to characters.
*/

typedef char* TNvString;

#ifdef __cplusplus
}
#endif

#endif /* defined NV_DEFS_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
