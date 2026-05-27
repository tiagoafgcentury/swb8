/**
** @file nv_opaque.h
**
** @brief
**   DVR/VOD Library Opaque data management functions.
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


#ifndef NV_OPAQUE_H
#define NV_OPAQUE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*..........................................................................  */
/*...........................  INCLUDE FILES................................  */
/*..........................................................................  */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                     TYPES                                  */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Definition of the Handle that uniquely identifies an
 *    Opaque Data Object.
 *
 *  @remark
 *    -# The type TOdmHandle is opaque to client applications to prevent
 *    external access. A pointer on a TOdmHandle is returned when calling
 *    dvlOpaqueDataMessageCreate and shall be returned when releasing the
 *    Opaque Data Message later.
 *    The type TOdmHandle should always be used as a pointer by a client
 *    application.
*/
typedef struct SOdmHandle TOdmHandle;


/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupODM
 *  @brief
 *    This function enables the STB application to request the creation of
 *    an Opaque Data. This function supplies an Opaque Data Message Handle.
 *
 *    The Application can use Opaque Data setters to add fields in the Opaque Data.
 *    The Opaque Data can then be used to convey information needed by the Head-End
 *    in order to generate an entitlement.
 *
 *    The Application can retrieve the Opaque Data buffer thanks to dvlOpaqueDataMessageGet().
 *
 *    When the Application does not need the Opaque Data object anymore, it shall use the
 *    dvlOpaqueDataMessageRelease() function to free resources allocated by DVL.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *
 *  @param[out]	ppxOpaqueDataMessageHandle
 *             Pointer where the DVR/VOD Library will store the Opaque Data Message Handle.
 *             The input pointer shall not be NULL.
 *
 *  @retval DVL_NO_ERROR
 *            The function was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlOpaqueDataMessageCreate
(
        TOdmHandle**     ppxOpaqueDataMessageHandle
);

/**
 *  @ingroup groupODM
 *  @brief
 *    This function allows to release a OpaqueDataMessage Handle when it is no
 *    more used by Application.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in]  pxOpaqueDataMessageHandle
 *             Pointer to the OpaqueDataMessage Handle previously created with dvlOpaqueDataMessageCreate().
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlOpaqueDataMessageRelease
(
          TOdmHandle*     pxOpaqueDataMessageHandle
);

/**
 *  @ingroup groupODM
 *  @brief
 *    This function adds/updates Application Data inside the OpaqueDataMessage.
 *    The function dvlOpaqueDataMessageGet() allow to retrieve the
 *    corresponding OpaqueDataMessage buffer.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in]  pxOpaqueDataMessageHandle
 *             Pointer to the OpaqueDataMessage Handle previously created with dvlOpaqueDataMessageCreate().
 *  @param[in] xApplicationDataSize
 *             Length of the Application Data
 *  @param[in]	pxApplicationData
 *             Pointer to the Application Data buffer. Note that Application Data should be a
 *             base64 encoded string in order to be a JSON compatible string.
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlOpaqueDataMessageSetApplicationData
(
        TOdmHandle*     pxOpaqueDataMessageHandle,
        TSize	           xApplicationDataSize,
  const TUnsignedInt8* 	pxApplicationData
);

/**
 *  @ingroup groupODM
 *  @brief
 *    This function adds/updates PRM Signalization Data inside the Opaque Data Message.
 *    The function dvlOpaqueDataMessageGet() enable to retrieve the
 *    corresponding Opaque Data Message buffer.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in]  pxOpaqueDataMessageHandle
 *             Pointer to the Opaque Data Message Handle previously created with dvlOpaqueDataMessageCreate().
 *  @param[in] xPrmSignalizationSize
 *             Length of the PRM Signalization Data
 *  @param[in]	pxPrmSignalization
 *             Pointer to the PRM Signalization Data buffer.
 *             Note that this buffer shall be a base64 URL safe encoded string.
 *
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlOpaqueDataMessageSetPrmSignalization
(
        TOdmHandle*     pxOpaqueDataMessageHandle,
        TSize	           xPrmSignalizationSize,
  const TUnsignedInt8* 	pxPrmSignalization
);

/**
 *  @ingroup groupODM
 *  @brief
 *    This function supplies the address of the buffer containing the Opaque Data Message (base64).
 *    The DVR/VOD Library processes the message in the function itself.
 *    That means the STB application can release the buffer once the function returns.
 *    The buffer pointed by pxOpaqueDataMessage shall remain valid during the
 *    whole function processing. Once the function returns, the STB application
 *    can release the pointed memory buffer.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in]  pxOpaqueDataMessageHandle
 *             Pointer to the Opaque Data Message Handle previously created with dvlOpaqueDataMessageCreate().
 *  @param[out] pxOpaqueDataMessageSize
 *             Pointer to the length of the output Message. This size is set to zero
 *             by the DVR/VOD Library in case the returned status is not equal to DVL_NO_ERROR.
 *  @param[out]	ppxOpaqueDataMessage
 *             This pointer will contain the address of the memory where the
 *             output Credentials will be stored if the function returns DVL_NO_ERROR.
 *             In case the function returns any other status, the returned address
 *             will be set to NULL.
 *             The memory pointed by *ppxOpaqueDataMessage shall be released by the STB
 *             application by calling the dedicated function dvlReleaseBuffer().
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlOpaqueDataMessageGet
(
        TOdmHandle*      pxOpaqueDataMessageHandle,
        TSize* 	         pxOpaqueDataMessageSize,
  const TUnsignedInt8** ppxOpaqueDataMessage
);

#ifdef __cplusplus
}
#endif

#endif /* NV_OPAQUE_H */

/* END OF FILE */
