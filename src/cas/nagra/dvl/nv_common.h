/**
** @file nv_common.h
**
** @brief
**   DVR/VOD Library Common external definitions.
**
**   The syntax of the comments of this file is compliant to the format used by
**   Doxygen to generate html documentation from source and header files.
**   Doxygen can be downloaded from www.doxygen.org and is provided under GNU
**   General Public License.
**
** $Revision:  $
**
** Copyright:
**   2006-2017 Nagra India
**
**
*/


#ifndef NV_COMMON_H
#define NV_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                              INCLUDE FILES                                 */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                     TYPES                                  */
/*                                                                            */
/******************************************************************************/

/**  @brief
 *    This enumeration defines the error statuses returned by
 *    the DVR/VOD Library functions.
*/
typedef enum
{
  DVL_NO_ERROR = 0,
    /**<
     *    The function terminated successfully.
    */
  DVL_ERROR,
    /**<
     *    An unspecified error happened during the function execution.
    */
  DVL_ERROR_BAD_PARAMETER,
    /**<
     *    One or more function arguments are missing. The request could not be
     *    successfully processed.
    */
  DVL_ERROR_DVL_NOT_RUNNING,
    /**<  The Nagra DVR/VOD Library is currently not running and the requested
     *    operation cannot be executed.
    */
  LAST_DVL_STATUS
    /**<
     *    Last DVL status.
     *    This status is never returned by the DVR/VOD Library.
    */
} TDvlStatus;

/**
 *
 *  @brief
 *    This structure describes a buffer
*/
typedef struct
{
  TUnsignedInt32  bufferSize;
    /**<
     *   Buffer size <br>
     *   Size in bytes of the buffer.
   */
  TUnsignedInt8* pBuffer;
    /**<
     *    This field contains the Buffer.
   */
} TDvlBuffer;


/**
 * @ingroup groupInitTerm
 * @brief
 *   UTC time importation callback.
 *
 *   This decoder manufacturer's function returns the current UTC time and date
 *   to the DVR VOD library. It will be called each time the DVR VOD Library
 *   needs this information.
 *
 * @note
 *   DVL Dual Library shall not use the UtcUnixTimeImportationCB provided by the application.\n
 *   The application can pass NULL in case of DVL Dual Library.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[out] pxUtcTime
 *    UTC Time.
 *
 *  @remark
 *    -# This function is called in the DVR/VOD Library context. Therefore,
 *       its execution should be as short as possible. It must not block the
 *       calling task.
 *    -# As consequence of previous remark, no other function of this API
 *       shall be called directly inside the notification callback.
*/
typedef void (*TUtcUnixTimeImportationCB)
(
  TUnixDate* pxUtcTime
);

/**
 * @ingroup groupInitTerm
 * @brief
 *   Pairing data importation callback.
 *
 *   This decoder manufacturer's function gets the pairing data
 *   (reconstruction), stored in the protected memory area of the decoder. The
 *   buffer is provided by this function. It should remain valid until the
 *   DVR/VOD Library shuts down.
 *
 *  @note
 *   DVL Dual Library shall not use the PairingImportationCB provided by the application.\n
 *   The application can pass NULL in case of DVL Dual Library.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[out] ppxNaspPairingData
 *            Pointer to the first byte of Nagravision-provided pairing
 *            data.
 *
 *  @remark
 *    -# This function is called in the DVR/VOD Library context. Therefore,
 *       its execution should be as short as possible. It must not block the
 *       calling task.
 *    -# As consequence of previous remark, no other function of this API
 *       shall be called directly inside the notification callback.
 *    -# DVR/VOD Library running in NOCS3 chipsets does not use the pairing data, hence
 *       this function is not used by \c dvlInitialize()
 *
*/
typedef void (*TPairingImportationCB)
(
  const TUnsignedInt8** ppxNaspPairingData
);

/**@ingroup groupDebug
 *  @brief
 *    DVL Debug module identifiers.
*/
typedef enum
{
  DVL_MOD_INIT = 1,       /**<  1: Initialization                  */
  DVL_MOD_MEM = 3,        /**<  3: Memory Allocator                */
  DVL_MOD_TLS = 5,        /**<  5: Tools (list, queue, ...)        */
  DVL_MOD_TLS_VTT,        /**<  6: Tools (vtt only)                */
  DVL_MOD_CRPT = 33,      /**< 33: Cryptographic algorithms        */
  DVL_MOD_UTC = 57,       /**< 57: UTCASH                          */
  DVL_MOD_DBG = 79,       /**< 79: Debug debugging                 */
  DVL_MOD_CASH = 82,      /**< 82: CA shell                        */
  DVL_MOD_DVL = 85,       /**< 85: DVR/VOD Library                 */
  DVL_MOD_SCE             /**< 86: Steady Clock Engine             */
} TDvlDebugModule;

/**@ingroup groupDebug
 * @brief
 *   Flag used to perform a control for every module
 *
 *  @see  TDvlDebugModule
*/
#define DVL_MOD_ALL    0xFFFFFFFF

/**@ingroup groupDebug
 * @brief
 *   DVL Verbosity level to control the amount of messages to display.
*/
typedef enum
{
  DVL_VERBOSITY_SILENT,
  /**< no output at all, except DBG_ALWAYS; default value */
  DVL_VERBOSITY_LOW,
  /**< global debug messages */
  DVL_VERBOSITY_MIDDLE,
  /**< global detailed debug messages */
  DVL_VERBOSITY_HIGH,
  /**< internal detailed debug messages */
  DVL_VERBOSITY_MAX
  /**< internal very detailed debug messages */
} TDvlDebugVerbosity;

/**@ingroup groupDebug
 * @brief
 *   DVL Debug classes.
*/
typedef TUnsignedInt32 TDvlDebugClass32;

/**@ingroup groupDebug
 * @brief
 *   all classes.
*/
#define DVL_CLASS_ALL  ((TDvlDebugClass32)(-1))

/**@ingroup groupDebug
 * @brief
 *   DVL Debug role (functionality)
*/
typedef TUnsignedInt32 TDvlDebugRole;

/**@ingroup groupDebug
 * @brief
 *   all roles
*/
#define DVL_DEBUG_ROLE_ALL   ((TDvlDebugRole)(-1))


/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupInitTerm
 *  @brief
 *    Initialize the DVR/VOD Library. This call may be done only once. No
 *    function of the DVR/VOD Library API shall be called before the end of the
 *    initialization stage. If the DVR/VOD Library is shutdown by a call to
 *    \c dvlTerminate(), it can be initialized again by calling
 *    \c dvlInitialize().
 *
 *  @pre
 *    The DVR/VOD Library must not be running.
 *
 *  @post
 *    The DVR/VOD Library may be started.
 *
 *  @param[in] xPairingImportationCB
 *            Callback function used by the DVR/VOD Library to get its pairing
 *            data block from the decoder OTP memory.
 *  @param[in] xUtcUnixTimeImportationCB
 *            Callback function used by the DVR/VOD Library to get the UTC time.
 *
 *  @note   The above parameters can be NULL in case of DVL Dual Library.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *            Either a callback pointer or the mandatory output pointers NULL.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @remark
 *    -# \c TPairingImportationCB()function is not used by DVR/VOD Library
 *       running on NOCS3 chipsets.
*/
NAGRA_CA_API TDvlStatus dvlInitialize
(
  TPairingImportationCB       xPairingImportationCB,
  TUtcUnixTimeImportationCB   xUtcUnixTimeImportationCB
);

/**
 *  @ingroup groupInitTerm
 *  @brief
 *   Terminate the DVR/VOD Library. This call may be made only once. No other
 *   function of the DVR/VOD Library API shall be called after the termination
 *   stage, except the initialization function \c dvlInitialize().
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    The DVR/VOD Library may be stopped.
 *
 *  @remark
 *    -# This function is synchronous. It means that the caller will be
 *       suspended as long as all the task have not performed their cleanup and
 *       have not terminated.
 *    -# During the \c dvlTerminate() call execution, no other functions of the
 *       DVR/VOD Library API may be called.
*/
NAGRA_CA_API void dvlTerminate
(
  void
);

/**
 *  @ingroup groupMemory
 *  @brief
 *    This function allows the DVR/VOD Library client application to release
 *    an output buffer returned by the DVL. It shall be called on each output
 *    buffer returned by the DVL.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] pxBuffer
 *            Pointer to the buffer to be released.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The input argument of the function is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlReleaseBuffer
(
  const TUnsignedInt8*     pxBuffer
);

/**@ingroup groupDebug
 * @brief
 *   This API enables the debug traces from DVL.
 * @note
 *   This API shall be used only after calling \c dvlInitialize().\n
 *   The Debug logs <B>can be controlled for the moment exclusively by using TDvlDebugVerbosity xVerbosity</B> and other parameters have no impact.
 * @param   xModule
 *   Concerned module, or \c MOD_ALL to control all modules
 * @param   xVerbosity
 *   Verbosity that this module must use for debug
 *   (from \c VERBOSITY_LOW to \c VERBOSITY_MAX)
 * @param  xClass
 *   Class(es) that this module must output, or \c CLASS_ALL to
 *   output any class
 * @param   xRole
 *   Role(s) of output that this module must output, or
 *   \c DEBUG_ROLE_ALL to output any role
 * @remarks
 * -# A call to this API, replaces the previous control performed.
 *   The previous control may have been hard-coded during the build,
 *   or done through a previous call to a \c dvlDebugControlRole() function.
*/
NAGRA_CA_API void dvlDebugControlRole
(
  TDvlDebugModule     xModule,
  TDvlDebugVerbosity  xVerbosity,
  TDvlDebugClass32    xClass,
  TDvlDebugRole       xRole
);

/** @}
*/ /* End of groupDebug */

#ifdef __cplusplus
}
#endif

#endif /* NV_COMMON_H */

/* END OF FILE */
