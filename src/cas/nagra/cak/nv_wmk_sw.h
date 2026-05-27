/**
  @file nv_wmk_sw.h

  @brief
  This file defines the Nagra watermark software overlay interface.

  @details
  It defines functions required to configure the watermark software overlay
  embedder.

  This interface is implemented by the entity who is in charge of integrating
  the software overlay embedder (e.g. middleware vendor)

  COPYRIGHT:
    2019 Nagravision S.A.
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_watermark
  @brief Describe the Nagra watermark software interface.
*/

#ifndef NV_WMK_SW_H
#define NV_WMK_SW_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_types.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @defgroup g_watermark    Nagra Watermark Software Interface
*/

/** @{ */
#define WMKSWAPI_VERSION_MAJOR       1
#define WMKSWAPI_VERSION_MEDIUM      0
#define WMKSWAPI_VERSION_MINOR       1
/** @} */

#define WMKSWAPI_VERSION_INT       \
    NV_INTERFACE_VERSION_INT(WMKSWAPI_VERSION_MAJOR, WMKSWAPI_VERSION_MEDIUM, WMKSWAPI_VERSION_MINOR)

#define WMKSWAPI_VERSION_STRING    \
    NV_INTERFACE_VERSION_STRING(WMKSWAPI_, WMKSWAPI_VERSION_MAJOR, WMKSWAPI_VERSION_MEDIUM, WMKSWAPI_VERSION_MINOR)


/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @brief
    Defines an invalid transport session identifier.

*/
#define NV_WMK_SW_TRANSPORT_SESSION_INVALID      ((uint32_t)(-1))


/**
  @ingroup g_watermark
  @brief
  Result returned by Nagra watermark software interface functions.
*/

typedef enum
{
  NV_WMK_SW_SUCCESS,
  /**< The operation has completed successfully. */
  NV_WMK_SW_ERROR,
  /**< An unexpected error has occurred while processing the operation. */
  NV_WMK_SW_NOT_APPLICABLE,
  /**< The operation is not applicable */
  NV_WMK_SW_LAST_STATUS
  /**< Not used. */
}
TNvWmkSwResult;


/**
  @ingroup g_watermark
  @brief
  This structure defines the Nagra watermark interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    Nagra watermark software interface version number.
    @details
    Assign it to the ::WMKSWAPI_VERSION_INT result.
  */

  uint32_t (*configure)
  (
    const uint8_t* pxConfig,
          uint32_t  xConfigSize
  );
  /**<
    @brief
    Configures all watermark software overlay embedders on all media pipelines.

    @details
    This function configures all watermark software overlay embedder instances
    with the given configuration.

    @param[in] pxConfig
    Buffer containing the watermark configuration. This buffer is no longer available
    after configure() returns.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NV_WMK_SW_SUCCESS
    The operation completed successfully

    @retval ::NV_WMK_SW_ERROR
    The operation failed
  */

  uint32_t (*configureByPipe)
  (
          uint32_t    xTransportSessionId,
    const uint8_t*   pxConfig,
          uint32_t    xConfigSize
  );
  /**<
    @brief
    Configures the watermark software overlay embedder related to a given pipeline.

    @details
    This function configures the watermark software overlay embedder instance
    associated to a given pipeline. The pipeline is identified by the transport
    session ID.

    @param[in] xTransportSessionId
    Identifier of the media pipeline associated to the decrypted stream to be
    watermarked.

    @param[in] pxConfig
    Buffer containing the watermark configuration. This buffer is no longer available
    after configureByPipe() returns.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NV_WMK_SW_SUCCESS
    The operation completed successfully

    @retval ::NV_WMK_SW_ERROR
    The operation failed

    @retval ::NV_WMK_SW_NOT_APPLICABLE
    The operation is not applicable. For instance, the operation is called for
    a pipe without any software overlay embedder.
  */

} INvWmkSw;

#ifdef __cplusplus
}
#endif

#endif /* NV_WMK_SW_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
