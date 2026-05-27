/**
  @file ngwm_ree.h

  @brief
  This file defines the NexGuard Watermark REE Interface.

  @details

  It defines functions required to control the watermark TA from the REE

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
  - <b> 1.0.1 - 27-Apr-2020 </b>
    -  Changed copyright and trademarks to Nagravision S.A.
  - <b> 1.0.0 - 26-Feb-2017 </b>
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
  @addtogroup g_wm_ree
  @brief Describe the NexGuard Watermark REE Interface.
*/

#ifndef NGWM_REE_H
#define NGWM_REE_H

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
  @defgroup g_wm_ree    NexGuard Watermark REE Interface
*/

/** @{ */
#define NGWMREEAPI_VERSION_MAJOR       1
#define NGWMREEAPI_VERSION_MEDIUM      0
#define NGWMREEAPI_VERSION_MINOR       1
/** @} */

#define NGWMREEAPI_VERSION_INT       \
    NGWM_INTERFACE_VERSION_INT(NGWMREEAPI_VERSION_MAJOR, NGWMREEAPI_VERSION_MEDIUM, NGWMREEAPI_VERSION_MINOR)

#define NGWMREEAPI_VERSION_STRING    \
    NGWM_INTERFACE_VERSION_STRING(NGWMREEAPI_, NGWMREEAPI_VERSION_MAJOR, NGWMREEAPI_VERSION_MEDIUM, NGWMREEAPI_VERSION_MINOR)


/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_wm_ree
  @brief
  Result returned by NexGuard watermark REE interface functions.
*/

typedef enum
{
  NGWM_REE_SUCCESS,
  /**< The operation has completed successfully. */
  NGWM_REE_ERROR,
  /**< An unexpected error has occurred while processing the operation. */
  NGWM_REE_NOT_APPLICABLE,
  /**< The operation is not applicable */
  NGWM_REE_DENIED,
  /**< The operation is denied because of privilege violation */
  NGWM_REE_LAST_STATUS
  /**< Not used. */
}
TNgwmReeResult;


/**
  @ingroup g_wm_ree
  @brief
  This structure defines the NexGuard watermark REE interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    NexGuard watermark REE interface version number.
    @details
    Assign it to the ::NGWMREEAPI_VERSION_INT result.
  */

  uint32_t (*configure)
  (
    const uint8_t* pxConfig,
          uint32_t  xConfigSize
  );
  /**<
    @brief
    Configures watermark IP cores on all media pipelines.

    @details
    This function configures all watermark IP core instances with the given
    configuration.

    @param[in] pxConfig
    Buffer containing the watermark configuration.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NGWM_REE_SUCCESS
    The operation completed successfully

    @retval ::NGWM_REE_ERROR
    The operation failed

    @retval ::NGWM_REE_DENIED
    The operation is denied because of privilege violation (e.g. commands originated
    from the REE are no longer allowed)
  */

  uint32_t (*configureByPipe)
  (
          uint32_t    xPipeSourceId,
    const uint8_t*   pxConfig,
          uint32_t    xConfigSize
  );
  /**<
    @brief
    Configures watermark IP cores related to a given pipeline.

    @details
    This function configures watermark IP core instances associated to a given
    pipeline. The pipeline is identified by the pipe source ID.

    @param[in] xPipeSourceId
    Identifier of to the pipeline source element from which the content to be
    watermarked is coming from.

    @param[in] pxConfig
    Buffer containing the watermark configuration.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NGWM_REE_SUCCESS
    The operation completed successfully

    @retval ::NGWM_REE_ERROR
    The operation failed

    @retval ::NGWM_REE_NOT_APPLICABLE
    The operation is not applicable. For instance, the operation is called for
    a pipe without any IP core embedder.

    @retval ::NGWM_REE_DENIED
    The operation is denied because of privilege violation (e.g. commands originated
    from the REE are no longer allowed)
  */

} INgwmRee;


/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_wm_ree
  @brief
  Provide the watermark REE interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the NexGuard watermark REE interface structure must remain
  accessible as long as the NexGuard TA is running.

  @details
  This function is used by a rich client to retrieve the NexGuard watermark REE
  interface structure.

  @return
  A constant pointer to the NexGuard watermark REE interface structure.

  @see ::INgwmRee
*/

const INgwmRee* ngwmGetReeInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NGWM_REE_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
