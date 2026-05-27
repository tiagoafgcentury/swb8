/**
** @file nv_relatedCredInfo.h
**
** @brief
**   DVR/VOD Library Related Credential information functions.
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


#ifndef NV_RELATEDCREDINFO_H
#define NV_RELATEDCREDINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                   TYPES                                    */
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
 *  @ingroup groupCredInfo
 *  @brief
 *    This function allows the Application to request information about
 *    DVR/VOD Library Credentials managed internally by DVL.
 *
 *    Input parameters prmSyntax and contentId are exclusive.
 *    Meaning if prmSyntax is set, xContentIdSize shall be set to 0 and pxContentId shall be set to NULL.
 *    On the other hand, if contentId is set xPrmSyntaxSize shall be set to 0 and pxPrmSyntax shall be set to NULL.
 *    If prmSyntax and contentId are not compliant with if this rule the DVR/VOD library will return \c DVL_ERROR_BAD_PARAMETER.
 *
 *    PrmSyntax is used to identify the related entitlements for which
 *    Application requests information.
 *    If prmSyntax is valid but related entitlements are not found, the function
 *    returns with DVL_NO_ERROR and will only fill KeyId and ContentId fields.
 *    Other fields will be set to NULL.
 *    If prmSyntax is invalid the DVR/VOD library will return \c DVL_ERROR_BAD_PARAMETER.
 *
 *    ContentId is used to identify the related entitlements for which
 *    Application requests information.
 *    If contentId is valid but related entitlements is not found, the function
 *    returns with DVL_NO_ERROR and will only fill ContentId fields.
 *    Other fields will be set to NULL.
 *    If contentId is invalid the DVR/VOD library will return \c DVL_ERROR_BAD_PARAMETER.
 *
 *    Some Credentials may contain operator specific data. As these data are
 *    operator dependant and are not interpreted by the DVR/VOD Library, these
 *    specific data are returned by this request in the form of a buffers of
 *    Bytes pointed by \c *ppxGenericMetadata  and \c *ppxSpecificMetadata.
 *    The STB manufacturer shall refer to the operator for the format and
 *    meaning of the information contained in these metadata.
 *    All output pointers shall be released by the function
 *    \c dvlReleaseBuffer().
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xPrmSyntaxSize
 *            Size of the prmSyntax related to the Entitlement to analyze.
 *  @param[in] pxPrmSyntax
 *            pointer to the prmSyntax related to the Entitlement to analyze.
 *  @param[in] xContentIdSize
 *            Size of the contentId related to the Entitlement to analyze.
 *  @param[in] pxContentId
 *            pointer to the contentId, as string, related to the Entitlement to analyze.
 *  @param[out] pxKeyIdSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output keyId.
 *            If no KeyId is present in the prmSyntax, the KeyId size is set to zero.
 *  @param[out] ppxKeyId
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the KeyId. <br>
 *            If no KeyId is present in the prmSyntax, *ppxKeyId will be set to NULL.
 *  @param[out] pxContentIdSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output contentId.
 *  @param[out] ppxContentIdData
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the ContentId.
 *  @param[out] ppxCredentialsInformation
 *            Pointer where the DVR/VOD Library will store the Credentials
 *            information.
 *            The input pointer shall not be NULL.
 *            If the related entitlement is not found, *ppxCredentialsInformation will be set to NULL.
 *  @param[out] pxGenericMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional generic metadata. The input pointer shall not be NULL
 *            if ppxGenericMetadata is not NULL.
 *  @param[out] ppxGenericMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional generic metadata. This buffer can
 *            be NULL if the caller is not interested in retrieving generic
 *            metadata.
 *            Once the function returns, \c *ppxGenericMetadata may be NULL if
 *            the input Credentials contain no generic metadata or if they are
 *            not valid (function returns \c DVL_ERROR_BAD_PARAMETER) or missing
 *            (*ppxCredentialsInformation NULL).\n
 *            If returned, the buffer pointed by \c *ppxGenericMetadata shall be
 *            released once no longer used by the STB application, by calling
 *            \c dvlReleaseBuffer().
 *  @param[out] pxSpecificMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional specific metadata. The input pointer shall not be \c NULL
 *            if \c ppxSpecificMetadata is not \c NULL.
 *  @param[out] ppxSpecificMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional specific metadata. This buffer can
 *            be NULL if the caller is not interested in retrieving specific
 *            metadata.
 *            Once the function returns, \c *ppxSpecificMetadata may be NULL if
 *            the input Credentials contain no specific metadata or if they are
 *            not valid (function returns \c DVL_ERROR_BAD_PARAMETER) or missing
 *            (*ppxCredentialsInformation NULL).\n
 *            If returned, the buffer pointed by \c *ppxSpecificMetadata shall be
 *            released once no longer used by the STB application, by calling
 *            \c dvlReleaseBuffer().
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
 *
 *  @see ::dvlReleaseBuffer()
*/
NAGRA_CA_API TDvlStatus dvlRelatedCredentialsInformationRequest
(
        TSize                 xPrmSyntaxSize,
  const TUnsignedInt8*       pxPrmSyntax,
        TSize                 xContentIdSize,
  const TUnsignedInt8*       pxContentId,
        TSize*               pxKeyIdSize,
  const TUnsignedInt8**     ppxKeyId,
        TSize*               pxContentIdSize,
  const TChar**             ppxContentIdData,
  const TVodEmmData**       ppxCredentialsInformation,
        TSize*               pxGenericMetadataSize,
  const TUnsignedInt8**     ppxGenericMetadata,
        TSize*               pxSpecificMetadataSize,
  const TUnsignedInt8**     ppxSpecificMetadata
);

#ifdef __cplusplus
}
#endif

#endif /* NV_RELATEDCREDINFO_H */

/* END OF FILE */
