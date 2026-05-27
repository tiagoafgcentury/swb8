/**
  @file nv_tfl.h

  @brief
  Common Nagra trusted framework layers interface definitions.

  @details
  This file provides the common definitions to all trusted framework layer
  interfaces i.e. the communication interfaces and the trusted application
  interfaces.

  COPYRIGHT:
    2013 - 2016 Nagravision S.A.
*/

/*
   ==========================================================================
   IMPORTANT REMARK :
   ==========================================================================

   Comments in this file use special tags to allow automatic API
   documentation generation in HTML format, using the GNU-General Public
   Licensed Doxygen tool.
   For more information about Doxygen, please check www.doxygen.org

   Depending on the platform, the CHM file may not open properly if it is
   stored on a network drive. So either the file should be moved on a local
   drive or add the following registry entry on Windows platform (regedit):
   [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003

   ==========================================================================
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_tfl_defs
  @brief
  Common definitions specific to the Nagra trusted framework layer interfaces.

  @details

  The Nagra <b>Trusted Framework Layers</b> includes several interfaces running
  in rich and trusted execution environment. It introduces common trusted
  definitions to both rich and trusted execution environment.
  Please make sure to have read the documentation pages for a complete
  description of the interface constraints and requirements.
*/

#ifndef NV_TFL_H
#define NV_TFL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_defs.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_tfl_defs
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Nagra trusted framework layers interfaces version
  @brief
  Define the version number of the Nagra trusted framework layers interfaces.

  @details
  This version has to be included in the each Nagra trusted framework layer
  interface structure -- i.e. returned by nvGetTrustedSessionInterface() and
  nvGetTrustedApplicationContext(). To do so, use the Nagra trusted framework
  layers macro-instruction ::TFLAPI_VERSION_INT to put it in the right format.

  @{
*/

/** @brief Nragra trusted framework layers version major number. */
#define TFLAPI_VERSION_MAJOR        1
/** @brief Nagra trusted framework layers version medium number. */
#define TFLAPI_VERSION_MEDIUM       0
/** @brief Nagra trusted framework layers version minor number. */
#define TFLAPI_VERSION_MINOR        21

/**
  @brief Nagra trusted framework layers interface version formatted as a single integer.
  @hideinitializer
*/
#define TFLAPI_VERSION_INT        \
    NV_INTERFACE_VERSION_INT(TFLAPI_VERSION_MAJOR, TFLAPI_VERSION_MEDIUM, TFLAPI_VERSION_MINOR)
/**
  @brief Nagra trusted framework layer interface version formatted as a string.
  @hideinitializer
*/
#define TFLAPI_VERSION_STRING     \
    NV_INTERFACE_VERSION_STRING(TFLAPI_, TFLAPI_VERSION_MAJOR, TFLAPI_VERSION_MEDIUM, TFLAPI_VERSION_MINOR)

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_tfl_defs

  @brief
  Values for Nagra trusted framework layers operation result.

  @details
  This enumeration provides all the possible values that can be returned by a
  Nagra trusted framework layers operation.
*/

typedef enum {
    NV_TRUSTED_SUCCESS,
	/**< The trusted operation has been successful. */
    NV_TRUSTED_ERROR_CLIENT_MEMORY,
	/**< The trusted operation failed: The client environment runs out of memory. */
    NV_TRUSTED_ERROR_TRUSTED_MEMORY,
	/**< The trusted operation failed: The trusted environment runs out of memory. */
    NV_TRUSTED_ERROR_COMMUNICATION,
	/**< The trusted operation failed: The non-trusted environment failed to reach the trusted environment. */
    NV_TRUSTED_ERROR_BAD_PARAMETER,
	/**< The trusted operation failed: A bad parameter has been provided. */
    NV_TRUSTED_ERROR_BLOCK_TOO_SHORT,
	/**< The trusted operation failed: An output block has a size too short to hold the expected result. */
    NV_TRUSTED_ERROR_INVALID_OPERATION,
	/**< The trusted operation failed: The operation results with inconsistent state. */
    NV_TRUSTED_ERROR_NOT_SUPPORTED,
	/**< The trusted operation failed: The requested operation is not supported. */
    NV_TRUSTED_ERROR_SECURITY,
	/**< The trusted operation failed: The operation was not authorized. */
    NV_TRUSTED_ERROR,
	/**< The trusted operation failed: The operation results with an unknown error. */
    NV_TRUSTED_RESULT_MAX
	/**< Enumeration management value - Unused. */
} TNvTrustedResult;

/**
  @ingroup g_tfl_defs
  @brief
  Types of memory blocks used to communicate with the trusted execution
  environment.

  @details
  This enumeration provides all the possible values that define the type of a
  memory block provided to the trusted environment.

  Refer to @ref s_tfl_invoke_outputs section for further details.

  @see ::TNvTrustedBlock.
*/

typedef enum
{
  NV_TRUSTED_BLOCK_INPUT,
  /**<
    The memory block is provided for input only. It is filled by caller and
    must not be modified by the trusted operation. The related size of the
    memory block provides the amount of input data.
  */
  NV_TRUSTED_BLOCK_OUTPUT,
  /**<
    The memory block is provided for result output only. It has no relevant
    values before operation and can be filled by the trusted operations. The
    related size of the memory block is the total size available and can be
    adjusted down to the relevant filled value.
    Refer to @ref s_tfl_invoke_outputs section for further details.
  */
  NV_TRUSTED_BLOCK_INPUT_OUTPUT,
  /**<
    The memory block is provided for input and result output. It is filled by
    caller and can be modified by the trusted operations. The related size of
    the memory block provides the amount of input data. It can also be
    adjusted down to the relevant filled value.
    Refer to @ref s_tfl_invoke_outputs section for further details.
  */
  NV_TRUSTED_BLOCK_DIRECTION_MAX
  /**< Enumeration management value - Unused */
}
TNvTrustedBlockDirection;

/**
  @ingroup g_tfl_defs
  @brief
  Memory blocks used to communicate with the trusted execution environment.

  This structure describes a memory block intended to be exchanged with the
  trusted environment. A memory block is primarily described by its address and
  size as well as its exchange direction. Refer to ::TNvTrustedBlockDirection
  for details on exchange direction. Memory block allocation is managed by the
  client of the trusted environment.

  Refer to @ref s_tfl_invoke_outputs section for further details.

  @see ::TNvTrustedBlockDirection, ::INvTrustedSession::command(),
       nvTrustedSessionCommandEntry().
*/

typedef struct
{
  void*    pAddr;
  /**< Address of the memory block. The size may be @c NULL for an output only block. */
  uint32_t size;
  /**< Size of the memory block.
       This size may be modified during invocation operation for an output or an input/output block. */
  uint32_t direction;
  /**< Exchange direction of the memory block; refer to ::TNvTrustedBlockDirection. */
}
TNvTrustedBlock;

#ifdef __cplusplus
}
#endif

#endif /* defined NV_TFL_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
