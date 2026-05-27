/**
** @file nv_pmm.h
**
** @brief
**   DVR/VOD Library PRM Management Message functions.
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


#ifndef NV_PMM_H
#define NV_PMM_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                                    INCLUDE FILES                           */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupPMM
 *  @brief
 *    This function allows the STB application to send to the DVR/VOD Library
 *    PRM Management Messages sent by the Head End.<br>
 *    The DVR/VOD Library processes the message in the function itself. That
 *    means the STB application can release the buffer once the function
 *    returns.<br>
 *    The buffer pointed by \c pxPrmManagementMessage shall remain valid
 *    during the whole function processing. Once the function returns, the STB
 *    application can release the pointed memory buffer.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xPrmManagementMessageSize
 *            Size of the PRM Management Message.
 *  @param[in] pxPrmManagementMessage
 *            Pointer to the buffer of Bytes containing the PRM Management
 *            message.
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed. The management message could not
 *            be processed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlPostPrmManagementMessage
(
        TSize             xPrmManagementMessageSize,
  const TUnsignedInt8*   pxPrmManagementMessage
);

/**
 *  @ingroup groupPMM
 *  @brief
 *    This function allows the STB application to retrieve private data that are
 *    contained in a ciphered message.
 *    The DVR/VOD Library decrypts the input message, verifies the signature
 *    that it contains and returns the private data in plaintext.<br>
 *    The function returns only authenticated data. If the decryption or the
 *    signature verification fails, it returns an error status and the pointer
 *    to the data is \c NULL.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xCipheredMessageSize
 *            Size of the input ciphered message.
 *  @param[in] pxCipheredMessage
 *            Pointer to the buffer of Bytes containing the ciphered message.
 *  @param[out] pxAuthenticatedDataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            authenticated data contained in the ciphered message.
 *            The input pointer shall not be NULL.
 *  @param[out] ppxAuthenticatedData
 *            Pointer where the DVR/VOD Library will store the pointer to
 *            the buffer containing the authenticated data.
 *            This buffer cannot be NULL.
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
NAGRA_CA_API TDvlStatus dvlGetAuthenticatedMessage
(
        TSize             xCipheredMessageSize,
  const TUnsignedInt8*   pxCipheredMessage,
        TSize*           pxAuthenticatedDataSize,
  const TUnsignedInt8** ppxAuthenticatedData
);

#ifdef __cplusplus
}
#endif

#endif /* NV_PMM_H */

/* END OF FILE */
