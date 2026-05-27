/**
** @file nv_pvrvod.h
**
** @brief
**   DVR/VOD Library Content recording, migrating, playback & LCM functions.
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


#ifndef NV_PVRVOD_H
#define NV_PVRVOD_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/
#include "nv_acd.h"

/******************************************************************************/
/*                                                                            */
/*                                     TYPES                                  */
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
 *  @ingroup groupRecord
 *  @brief
 *    This function creates a legacy LCM. This LCM does not contain any content
 *    key, instead it contains a special marker indicating, when it is played
 *    back by the DVL, that the SEC shall be set in legacy mode. This function
 *    is intended to be called during a legacy DVR migration phase and the
 *    returned LCM is expected to be stored persistently and associated to
 *    legacy contents. Once this is done, the returned LCM buffer shall be
 *    released with the function \c dvlReleaseBuffer().
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xSpecificMetadataSize
 *            Size of the optional specific metadata to be stored in the output
 *            LCM. Shall be set to zero in case no specific related
 *            metadata are passed to the DVR/VOD Library.
 *  @param[in] pxSpecificMetadata
 *            Optional specific metadata to be stored in the output Credentials.
 *            Shall be set to NULL in case no specific metadata are passed to
 *            the DVR/VOD Library.
 *  @param[out] pxOutputCredentialsSize
 *            Returned length of the output Credentials. This value shall be
 *            considered valid only if the function returned DVL_NO_ERROR.
 *  @param[out] ppxOutputCredentials
 *            Returned address of the memory where the output Credentials will
 *            be stored. This value shall be considered valid and released only
 *            if the  function returned DVL_NO_ERROR. To release this buffer,
 *            the STB application shall call \c dvlReleaseBuffer().
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlCreateLegacyLcm
(
        TSize                 xSpecificMetadataSize,
  const TUnsignedInt8*       pxSpecificMetadata,
        TSize*               pxOutputCredentialsSize,
  const TUnsignedInt8**     ppxOutputCredentials
);

/**
 *  @ingroup groupPlayback
 *  @brief
 *    This function loads the entitlement related to the prmSyntax OR contentId
 *    supplied in \c pxPrmSyntax OR \c pxContentId to the DVR/VOD Playback session.
 *
 *    Input parameters prmSyntax and contentId are exclusive.
 *    Meaning if prmSyntax is set, xContentIdSize shall be set to 0 and pxContentId shall be set to NULL.
 *    On the other hand, if contentId is set, xPrmSyntaxSize shall be set to 0 and pxPrmSyntax shall be set to NULL.
 *    If prmSyntax and contentId are not compliant with if this rule the DVR/VOD library shall return \c DVL_ERROR_BAD_PARAMETER.
 *
 *    - When this function returns with the error status \c DVL_NO_ERROR, it does not
 *      necessarily mean the playback is successful. The Application still
 *      needs to check the value set in the status \p *pxAccessStatus
 *      parameter.
 *    - When this function returns with the status \c DVL_ACCESS_GRANTED set in
 *      \p *pxAccessStatus, that means that the corresponding entitlement was found
 *      and the related content key has been correctly set in the descrambler
 *      so that the application can start playing back the content.
 *    - Else, that mean the entitlement associated to the prmSyntax OR contentId was not found,
 *      has expired or was invalid. In this case, the Application should try to request new entitlement
 *      and wait for a notification DVL_ACCESS_GRANTED from \c xAccessChangeNotificationCallback()
 *
 *    Once the function returned with DVL_NO_ERROR, the DVR/VOD Library may
 *    call the playback notification callback \p xAccessChangeNotificationCallback
 *    at any time.
 *
 *    parameter.<br>
 *    @remarks
 *    - Note that this function can be called several times during a Playback session
 *      in order to load several keys (only for entitlement related to the prmSyntax).
 *      It enables to manage key change smoothly. The keys
 *      table is managed in circular buffer mode, that means that when the table is full,
 *      the oldest key is replaced by the new key. Default size of this buffer is 2: that
 *      means that at most 2 entitlements are loaded simultaneously and older entitlements
 *      related DVL resources (like callback) are freed.
 *    - After opening a playback session with \c dvlPlaybackOpenSession(), you shall not mix
 *      call to \c dvlPlaybackSetEntitlements() and \c dvlPlaybackSetRelatedEntitlements().
 *    - if prmSyntax is valid and ppxKeyId is not NULL, keyId fields will be set even
 *      if the entitlement is missing.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] pxPlaybackHandle
 *            Identifier of the playback session previously created with dvlPlaybackOpenSession().
 *  @param[in] xPrmSyntaxSize
 *            Size in Bytes of the buffer containing the prm Syntax related to
 *            the entitlement to set.
 *  @param[in] pxPrmSyntax
 *            Pointer of the buffer containing the prmSyntax. Syntax related to
 *            the entitlement to set. <br>
 *  @param[in] xContentIdSize
 *            Size of the contentId related to the Entitlement to set.
 *  @param[in] pxContentId
 *            Pointer to the contentId, as string, related to the Entitlement to set.
 *  @param[in] pxApplicationPrivateData
 *            Pointer to Application Private Data allowing Application to store
 *             any contextual information
 *  @param[out] pxAccessStatus
 *            Pointer where the DVR/VOD Library will store the playback status.
 *            If \c *pxAccessStatus is greater or equal to
 *            \c DVL_ACCESS_DENIED, the content key has not been set
 *            and the content cannot be played back.
 *  @param[in] xAccessChangeNotificationCallback
 *            Callback that will be called if AccessStatus change.
 *            Main reasons for a Access Status to change are:
 *            - entitlement Expiration
 *            - access granted following the importation of new entitlements.
 *  @param[out] pxKeyIdSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output keyId.
 *            The input pointer shall not be NULL if ppxKeyId is not NULL.
 *            If no KeyId is present, the KeyId size is set to zero.
 *  @param[out] ppxKeyId
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the KeyId. <br>
 *            This buffer can be \c NULL if the caller is not interested in retrieving KeyId.<br>
 *            Else:
 *            - If no KeyId is present, *ppxKeyId will be set to NULL.
 *            - else the application shall release the buffer by calling \c dvlReleaseBuffer().
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 */
NAGRA_CA_API TDvlStatus  dvlPlaybackSetRelatedEntitlements
(
        TDvlHandle*                pxPlaybackHandle,
        TSize                       xPrmSyntaxSize,
  const TUnsignedInt8*             pxPrmSyntax,
        TSize                       xContentIdSize,
  const TUnsignedInt8*             pxContentId,
        void*                      pxApplicationPrivateData,
        TDvlAccessStatus*          pxAccessStatus,
        TDvlPlaybackNotificationCB  xAccessChangeNotificationCallback,
        TSize*                     pxKeyIdSize,
  const TUnsignedInt8**            ppxKeyId
);

#ifdef __cplusplus
}
#endif

#endif /* NV_PVRVOD_H */

/* END OF FILE */
