/**
  @file ngwm_hal.h

  @brief
  This file defines the NexGuard Watermark Hardware Abstraction Layer (HAL) Interface.

  @details

  It defines functions required to write NexGuard IP Core registers

  This interface is implemented by the SoC vendor

  COPYRIGHT:
    2020 Nagravision S.A.
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
/*                               CHANGE HISTORY                               */
/* ========================================================================== */

/** @page p_wmk_history Changes history
  - <b> 1.3.1 - 27-Apr-2020 </b>
    -  Changed copyright and trademarks to Nagravision S.A.
  - <b> 1.3.0 - 17-Oct-2018 </b>
    -  Added INgwmHal::getCoreVersion and INgwmHal::getErrorCode
  - <b> 1.2.0 - 21-Jun-2018 </b>
    -  Added INgwmHal::allocate, INgwmHal::free and INgwmHal::log
  - <b> 1.1.0 - 19-Mar-2018 </b>
    -  Added INgwmHal::set24bitMode
  - <b> 1.0.0 - 09-Nov-2017 </b>
    -  First issue
*/

/** @mainpage Overview
  - @subpage p_wmk_history

  <hr>
  Copyright &copy; 2017-2020 Nagravision S.A. All rights reserved. @n
  CH-1033 Cheseaux, Switzerland @n
  Tel: +41 21 7320311  Fax: +41 21 7320100 @n
  http://www.nagra.com

  All trademarks and registered trademarks are the property of their respective owners.

  This document is supplied with an understanding that the notice(s) herein or
  any other contractual agreement(s) made that instigated the delivery of a hard
  copy, electronic copy, facsimile or file transfer of this document are
  strictly observed and maintained.

  The information contained in this document is subject to change without notice.

  <b>Security Policy of Nagravision Kudelski Group</b> @n
  Any recipient of this document, without exception, is subject to a Non-Disclosure
  Agreement (NDA) and access authorization.
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_wm_hal
  @brief Describe the NexGuard Watermark HAL Interface.
*/

#ifndef NGWM_HAL_H
#define NGWM_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "ngwm_defs.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @defgroup g_wm_hal    NexGuard Watermark HAL Interface
*/

/** @{ */
#define NGWMHALAPI_VERSION_MAJOR       1
#define NGWMHALAPI_VERSION_MEDIUM      3
#define NGWMHALAPI_VERSION_MINOR       1
/** @} */

#define NGWMHALAPI_VERSION_INT       \
    NGWM_INTERFACE_VERSION_INT(NGWMHALAPI_VERSION_MAJOR, NGWMHALAPI_VERSION_MEDIUM, NGWMHALAPI_VERSION_MINOR)

#define NGWMHALAPI_VERSION_STRING    \
    NGWM_INTERFACE_VERSION_STRING(WMTAAPI_, NGWMHALAPI_VERSION_MAJOR, NGWMHALAPI_VERSION_MEDIUM, NGWMHALAPI_VERSION_MINOR)



/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_wm_hal
  @brief
  Result returned by NexGuard watermark HAL interface functions.
*/

typedef enum
{
  NGWM_HAL_SUCCESS,
  /**< The operation has completed successfully. */
  NGWM_HAL_ERROR,
  /**< An unexpected error has occured while processing the operation. */
  NGWM_HAL_LAST_STATUS
  /**< Not used. */
}
TNgwmHalResult;


/**
  @ingroup g_wm_hal
  @brief
  This structure defines the NexGuard watermark HAL interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    NexGuard watermark HAL interface version number.
    @details
    Assign it to the ::NGWMHALAPI_VERSION_INT result.
  */

  TNgwmHalResult (*setSeed)
  (
    TNgwmEmbedder   pxEmbedder,
    uint32_t         xSeed
  );
  /**<
    @brief
    Set the seed to be used for watermarking

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xSeed
    Seed to be to configure in the hardware embedder

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*setOperatorId)
  (
    TNgwmEmbedder   pxEmbedder,
    uint8_t          xOperatorId
  );
  /**<
    @brief
    Set the operator ID to be used for watermarking

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xOperatorId
    Operator ID to configure in the hardware embedder

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*setSettings)
  (
          TNgwmEmbedder   pxEmbedder,
    const uint8_t*        pxSettings,
          uint32_t         xSize
  );
  /**<
    @brief
    Set the settings to be used for watermarking

    Data referenced by @c pxSettings parameter is encapsulated in a TLV descriptor complying with
    the format below. It may actually convey an array of settings for potentially
    several output types.

    @code
      tag                   8 uimsbf  h44
      length                8 uimsbf
      technology_type       8 uimsbf
      technology_version   16 uimsbf
      settings_nb           8 uimsbf
      for(i=0; i<N; i++){
        setting_type        8 uimsbf
        setting_data     21*8 uimsbf
      }
    @endcode

    Settings conveyed in the array, and also the number of entries in the array,
    are SoC specific and depend on the IP core integration model.
    If the SoC is able to configure IP cores per output type (e.g.
    SDR8, HDR10, etc), the settings array contains an entry for each
    supported output type. Otherwise, the settings array contains one single entry
    applicable to any output.

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] pxSettings
    Buffer containing the settings to configure in the hardware embedder.

    @param[in] xSize
    Size in bytes of the pxSettings array

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*setDeviceId)
  (
          TNgwmEmbedder   pxEmbedder,
    const uint8_t*        pxDeviceId,
          uint8_t          xSizeInBits
  );
  /**<
    @brief
    Set the device ID to be used for watermarking

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] pxDeviceId
    Device ID to configure in the hardware embedder

    @param[in] xSizeInBits
    Size in bits of the device ID (e.g. typically 32 bits)

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*setTimeCode)
  (
    TNgwmEmbedder   pxEmbedder,
    uint16_t         xTimeCode
  );
  /**<
    @brief
    Set the timecode to be used for watermarking.

    The value of the timecode provided to this function is already a 16-bit value
    using a 15-minute resolution over a year.

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xTimeCode
    Timecode to configure in the hardware embedder

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*enableService)
  (
    TNgwmEmbedder   pxEmbedder,
    bool             xIsEnabled
  );
  /**<
    @brief
    Activate/deactivate watermark embedding.

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xIsEnabled
    When this boolean is set to TRUE, watermark embedding is activated. Otherwise
    it is deactivated.

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*setStubEmbedding)
  (
    TNgwmEmbedder   pxEmbedder,
    bool             xIsEnabled
  );
  /**<
    @brief
    Enables or disables the stub debug test pattern.

    The stub pattern is a visible mark used for test purposes. It is easier to
    check the integration when embedding a stub pattern than using the real
    imperceptible watermark.

    Note that the stub is only visible when the watermark embedding is turned on.

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xIsEnabled
    When this boolean is set to TRUE, the stub test pattern is activated. Otherwise
    it is deactivated.

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*set24bitMode)
  (
    TNgwmEmbedder   pxEmbedder,
    bool             xIsEnabled
  );
  /**<
    @brief
    Enables or disables the 24-bit payload mode.

    By default, the 24-bit payload mode is disabled and the IP core works in
    56-bit payload mode.

    @param[in] pxEmbedder
    Embedder object handle related to IP core to be configured

    @param[in] xIsEnabled
    When this boolean is set to TRUE, the 24-bit payload mode is enabled.
    Otherwise it is disabled and the IP core works in 56-bit mode.

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  void* (*allocate)
  (
    size_t    xSize
  );
  /**<
    @brief
    Allocates @c xSize bytes of memory.

    @param[in]      xSize
    Number of bytes to allocate

    @return
    Returns a pointer to the allocated buffer when the operation succeeds.
    Returns @c NULL otherwise.
  */

  void (*free)
  (
    void*     pxMem
  );
  /**<
    @brief
    Frees the allocated memory referenced by @c pxMem.

    @param[in]      pxMem
    A pointer to the memory to be freed.
  */

  void (*log)
  (
    const char*  pxMessage
  );
  /**<
    @brief
    This function logs a NULL-terminated string message.

    @param[in] pxMessage
    Null-terminated message string to be logged.
  */

  TNgwmHalResult (*getCoreVersion)
  (
    TNgwmEmbedder   pxEmbedder,
    uint8_t         pxVersion[2]
  );
  /**<
    @brief
    Reads the IP core version

    @param[in] pxEmbedder
    Embedder object handle

    @param[out] pxVersion
    A 2-byte array where to copy the IP core version

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

  TNgwmHalResult (*getErrorCode)
  (
    TNgwmEmbedder   pxEmbedder,
    uint8_t*        pxErrorCode
  );
  /**<
    @brief
    Reads the IP core error code

    @param[in] pxEmbedder
    Embedder object handle

    @param[out] pxErrorCode
    Pointer to a 1-byte variable where to store the IP core error code.

    @retval ::NGWM_HAL_SUCCESS
    The operation completed successfully

    @retval ::NGWM_HAL_ERROR
    The operation failed
  */

} INgwmHal;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_wm_hal
  @brief
  Provide the watermark HAL interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the NexGuard watermark HAL interface structure must remain
  accessible as long as the NexGuard TA is running.

  @details
  This function is used by the NexGuard TA to retrieve the NexGuard watermark HAL
  interface structure.

  @return
  A constant pointer to the NexGuard watermark HAL interface structure.

  @see ::INgwmHal
*/

const INgwmHal* ngwmGetHalInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NGWM_HAL_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
