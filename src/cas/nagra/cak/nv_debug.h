/**
  @file nv_debug.h

  @brief
  This file defines the Nagra debug interface.

  @details

  It defines the function structure providing the required features as well as
  the error messages to manage.

  This interface is realized by the device platform implementer.
  It provides to the Nagra client a debug service.

  COPYRIGHT:
    2014 - 2016 Nagravision S.A.
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
  @addtogroup g_debug
  @brief Describe the Nagra debug interface of Nagra clients.

  @details
  The Nagra <b>Debug</b> interface introduces definition for providing printing
  service for log and services for performance measurement. Please make sure
  to have read the documentation pages for a complete description of the
  interface constraints and requirements.
*/

#ifndef NV_DEBUG_H
#define NV_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_defs.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_debug
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Nagra debug interface version
  @brief
  Define the version number of the Nagra debug interface.

  @details
  This version has to be included in the debug interface structure returned by
  nvGetDebugInterface(). To do so, use the debug macro ::DEBUGAPI_VERSION_INT to
  put it in the right format.

  @{
*/

/** @brief Nagra debug interface version major number. */
#define DEBUGAPI_VERSION_MAJOR       1
/** @brief Nagra debug interface version medium number. */
#define DEBUGAPI_VERSION_MEDIUM      1
/** @brief Nagra debug interface version minor number. */
#define DEBUGAPI_VERSION_MINOR       0

/**
  @brief Nagra debug interface version formatted as a single integer.
  @hideinitializer
*/
#define DEBUGAPI_VERSION_INT       \
    NV_INTERFACE_VERSION_INT(DEBUGAPI_VERSION_MAJOR, DEBUGAPI_VERSION_MEDIUM, DEBUGAPI_VERSION_MINOR)
/**
  @brief Nagra debug interface version formatted as a string.
  @hideinitializer
*/
#define DEBUGAPI_VERSION_STRING    \
    NV_INTERFACE_VERSION_STRING(DEBUGAPI_, DEBUGAPI_VERSION_MAJOR, DEBUGAPI_VERSION_MEDIUM, DEBUGAPI_VERSION_MINOR)

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_debug
  @brief
  This structure defines the Nagra debug interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    Nagra debug interface version number.
    @details
    Assign it to the ::DEBUGAPI_VERSION_INT result.
  */

  void (*log)
  (
    const char*  pxMessage
  );
  /**<
    @brief
    This function logs a NULL-terminated string message.

    @details
    When the Nagra client is built with log capabilities, logging is used for
    execution traces and for dumping some management data.
    The logged messages can be directly printed on a console or recorded in a
    file or anything like suitable to the targeted platform. It must allow the
    platform to deliver back the full log to Nagra support in case of
    integration issue.

    @param[in] pxMessage
    Null-terminated message string to be logged.
  */

  uint32_t (*getTickCount)
  (
    void
  );
  /**<
    @brief
    Returns the current tick count in microseconds.

    @details
    This function returns a current tick count.
    This will be used to make performance measurement.
    The ticks value must be provided in microseconds

    @return
    Current tick value.
  */

  int (*getChar)
  (
    void
  );
  /**<
    @brief
    Reads the next character from the standard input.

    @details
    This function reads the next character from the standard input. Characters
    are buffered in a FIFO. If there is no character pending in the buffer, the
    function blocks and waits until it receives a character.

    When the buffer is full, overrun characters are simply discarded.

    @return
    It returns the character read as an unsigned char cast to an int on success
    or EOF on failure.
  */

} INvDebug;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_debug
  @brief
  Provide the Nagra debug interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Nagra debug interface structure must remain
  accessible as long as the Nagra client is running.

  @details

  This function is used by the Nagra client to retrieve the Nagra debug
  interface structure.

  This function should be called once during Nagra client initialization but
  it cannot be definitively assumed. Therefore the address of the structure
  and the memory allocated to it must be valid as long as the Nagra client
  library is loaded and running.

  @return
  A constant pointer to the Nagra debug interface structure.

  @see ::INvDebug.
*/

const INvDebug* nvGetDebugInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_DEBUG_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
