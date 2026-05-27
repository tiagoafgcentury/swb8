/**
  @file ngwm_ta.h

  @brief
  This file defines the NexGuard Watermark TA Interface.

  @details

  It defines functions required to control the watermark TA

  This interface is implemented by NexGuard and called the CAS/DRM TA through
  the SoC inter-TA communication framework.

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
  - <b> 1.0.4 - 27-Apr-2020 </b>
    -  Changed copyright and trademarks to Nagravision S.A.
  - <b> 1.0.3 - 01-Feb-2018 </b>
    -  Correcting the version number
  - <b> 1.0.2 - 24-Jan-2018 </b>
    - Added NGWM_TA_NOT_APPLICABLE and NGWM_TA_DENIED statuses
    - Added isTeeOrigin boolean
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
  @addtogroup g_wm_ta
  @brief Describe the NexGuard Watermark TA Interface.
*/

#ifndef NGWM_TA_H
#define NGWM_TA_H

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
  @defgroup g_wm_ta    NexGuard Watermark TA Interface
*/

/** @{ */
#define NGWMTAAPI_VERSION_MAJOR       1
#define NGWMTAAPI_VERSION_MEDIUM      0
#define NGWMTAAPI_VERSION_MINOR       4
/** @} */

#define NGWMTAAPI_VERSION_INT       \
    NGWM_INTERFACE_VERSION_INT(NGWMTAAPI_VERSION_MAJOR, NGWMTAAPI_VERSION_MEDIUM, NGWMTAAPI_VERSION_MINOR)

#define NGWMTAAPI_VERSION_STRING    \
    NGWM_INTERFACE_VERSION_STRING(NGWMTAAPI_, NGWMTAAPI_VERSION_MAJOR, NGWMTAAPI_VERSION_MEDIUM, NGWMTAAPI_VERSION_MINOR)


/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_wm_ta
  @brief
  Result returned by NexGuard watermark TA interface functions.
*/

typedef enum
{
  NGWM_TA_SUCCESS,
  /**< The operation has completed successfully. */
  NGWM_TA_ERROR,
  /**< An unexpected error has occurred while processing the operation. */
  NGWM_TA_NOT_APPLICABLE,
  /**< The operation is not applicable */
  NGWM_TA_DENIED,
  /**< The operation is denied because of privilege violation */
  NGWM_TA_LAST_STATUS
  /**< Not used. */
}
TNgwmTaResult;



/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_wm_ta
  @brief
  Configures watermark IP cores on all media pipelines.

  @details
  This function configures all watermark IP core instances with the given
  configuration.

  @param[in] xIsTeeOrigin
  When set to TRUE, this boolean indicates that the command is originated from
  a TEE trusted application. Otherwise, it indicates that the command is originated
  from the REE.

  @param[in] pxConfig
  Buffer containing the watermark configuration.

  @param[in] xConfigSize
  Size in bytes of the watermark configuration.

  @retval ::NGWM_TA_SUCCESS
  The operation completed successfully

  @retval ::NGWM_TA_ERROR
  The operation failed

  @retval ::NGWM_TA_DENIED
  The operation is denied because of privilege violation (e.g. commands originated
  from the REE are no longer allowed)
*/
uint32_t ngwmConfigure
(
        bool           xIsTeeOrigin,
  const uint8_t*      pxConfig,
        uint32_t       xConfigSize
);


/**
  @ingroup g_wm_ta
  @brief
  Configures watermark IP cores related to a given pipeline.

  @details
  This function configures watermark IP cores associated to a given
  pipeline.

  @param[in] xPipeSourceId
  Identifier of to the pipeline source element from which the content to be
  watermarked is coming from.

  @param[in] pxEmbedder
  Pointer to the embedder object associated to the IP core(s) to be configured.

  @param[in] xIsTeeOrigin
  When set to TRUE, this boolean indicates that the command is originated from
  a TEE trusted application. Otherwise, it indicates that the command is originated
  from the REE.

  @param[in] pxConfig
  Buffer containing the watermark configuration

  @param[in] xConfigSize
  Size in bytes of the watermark configuration

  @retval ::NGWM_TA_SUCCESS
  The operation completed successfully

  @retval ::NGWM_TA_ERROR
  The operation failed

  @retval ::NGWM_TA_NOT_APPLICABLE
  The operation is not applicable. For instance, the operation is called for
  a pipe without any IP core embedder.

  @retval ::NGWM_TA_DENIED
  The operation is denied because of privilege violation (e.g. commands originated
  from the REE are no longer allowed)
*/
uint32_t ngwmConfigureByPipe
(
        uint32_t         xPipeSourceId,
        TNgwmEmbedder   pxEmbedder,
        bool             xIsTeeOrigin,
  const uint8_t*        pxConfig,
        uint32_t         xConfigSize
);


#ifdef __cplusplus
}
#endif

#endif /* NGWM_TA_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
