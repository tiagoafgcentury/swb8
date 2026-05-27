/**
** @file nv_homeDomain.h
**
** @brief
**   DVR/VOD Library Home domain & network functions.
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


#ifndef NV_HOMEDOMAIN_H
#define NV_HOMEDOMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                                INCLUDE FILES                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                    TYPES                                   */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This structure contains information about a HomeDomain
 *    the DVR/VOD Library belongs and that allows it to
 *    playback Home domain content.
 *
 *
 *  @remark
 *    This structure is not fully defined yet!!!!!!!!!!
 *
*/
typedef struct
{
  TUnsignedInt8            homeDomainNameSize;
    /**<
     *    This field contains the Size of the HomeDomain name
     *    to which the DVL belongs
   */
  TUnsignedInt8*           homeDomainName;
    /**<
     *    This field contains the HomeDomain name
     *    to which the DVL belongs
   */
  TSize                    standardUsageRulesSize;
    /**<
     *     Size of the standard Usage Rules.
     */
  TUnsignedInt8*           pStandardUsageRules;
    /**<
     *     Standard Usage Rules.
     *     This buffer will be de-allocated by the DVR/VOD Library
     *     when the related TDvlHomeDomainInfo structure will be freed.
     */
  TSize                    operatorUsageRulesSize;
    /**<
     *     Size of the operator Usage Rules.
     */
  TUnsignedInt8*           pOperatorUsageRules;
    /**<
     *     Operator Usage Rules.
     *     This buffer will be de-allocated by the DVR/VOD Library
     *     when the related TDvlHomeDomainInfo structure will be freed.
     */
} TDvlHomeDomainInfo;

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This structure contains the details of the parameters used by
 *    the DVL to create HomeDomain content message.\n
 *    To ease the management of default values and API compatibility,
 *    this structure SHALL only be allocated by the DVL by calling
 *    \c dvlCreateHomeDomainContentMessageParameters and never by the application itself.\n
 *    When this structure is released by calling dvlReleaseBuffer, the memory
 *    pointed by pInputCredentials, pStandardUsageRules and is untouched.
*/
typedef struct
{
  TSize                    homeDomainNameSize;
    /**<
     *  Size of the Home Domain Name.
     */
  TUnsignedInt8*           pHomeDomainName;
    /**<
     *  Home Domain Name.
     */
  TSize                    inputCredentialsSize;
    /**<
     *  Size in Bytes of the input credentials.
     */
  TUnsignedInt8*           pInputCredentials;
    /**<
     *  Pointer to the buffer of Bytes containing the input Credentials.
     *  It shall contain a DVR LCM
     */
  TUnixDate                expirationDate;
    /**<
     *    Date at which the Home domain credentials will expire. Corresponds to the date after
     *    which no content can anymore be playback using that Credentials.
    */
  TSize                    standardUsageRulesSize;
    /**<
     *  Size of the optionnal standard Usage Rules. \n
     *  Shall be set to zero in case no standard related
     *  usage rules are passed to the DVR/VOD Library. Max supported value is 255.
     */
  TUnsignedInt8*           pStandardUsageRules;
    /**<
     *  Optionnal standard Usage Rules.
     */
  TSize                    operatorUsageRulesSize;
    /**<
     *  Size of the Optionnal operator Usage Rules. \n
     *  Shall be set to zero in case no operator related
     *  usage rules are passed to the DVR/VOD Library. Max supported value is 255.
     */
  TUnsignedInt8*           pOperatorUsageRules;
    /**<
     *  Optionnal operator Usage Rules.
     */
} TDvlHomeDomainContentMessageParameters;


/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    The values represent the detailed execution status of Home Domain API's.<br>
 *    For easy usage, each DVL_HD_ERROR_* value is greater than DVL_HD_ERROR.
 *    The client application is encouraged to use this comparison to be
 *    compatible with the future HomeDomain status Nagravision could add.
*/
typedef enum
{
  DVL_HD_SUCCESS = 0,
    /**<
     *    DVL is able to successfully process the HDE set by the STB application, or
     *    DVL is able to successfully generate HLCM for a specific home domain, depending
     *    on the Home Domain API called previously.
     */
  DVL_HD_ERROR = 100,
    /**<
     *    Error in processing Home Domain request.
     */
  DVL_HD_ERROR_TIME_VAL,
    /**<
     *    The internal time reference is not valid.
     *    This causes HDE processing or HLCM generation failure.
     */
  DVL_HD_ERROR_INVALID_INPUT,
    /**<
     *    The input parameters or the credentials to the request provided are invalid.
     *    This causes HDE processing or HLCM generation failure.
     */
  DVL_HD_ERROR_INVALID_HDE,
    /**<
     *    The input Home Domain Entitlement provided to DVL is invalid.
     *    DVL is unable to parse the HDE creation date or end date or renewal date.
     *    No Home Domain information fetched.
     */
  DVL_HD_ERROR_HDE_EXPIRED,
    /**<
     *    The input Home Domain Entitlement provided to DVL has expired.
     *    No HLCM generated.
     */
  DVL_HD_ERROR_HDE_NOT_FOUND,
    /**<
     *    No HDE found in DVL storage.
     */
  DVL_HD_ERROR_HDE_FETCH_FAILED,
    /**<
     *    DVL is unable to fetch the HDE from storage.
     */
  DVL_HD_ERROR_HDE_STORE_FAILED,
    /**<
     *    DVL is unable to store the HDE.
     */
  DVL_HD_ERROR_HDE_RELEASE_FAILED,
    /**<
     *    DVL is unable to release the HDE from storage.
     */
  DVL_HD_ERROR_HLCM_GENERATION_FAILED,
    /**<
     *    DVL is unable to generate HLCM from the input credentials provided.
     */
  DVL_HD_ERROR_INVALID_INPUT_LCM,
    /**<
     *    The input credentials provided for HLCM generation is invalid.
     *    No HLCM generated.
     */
  LAST_DVL_HD_STATUS
    /**<
     *    Last DVL Home Domain status value.
     *    This value is never returned by DVR/VOD Library.
     */
} TDvlHDStatus;

/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This function allows the STB application to request information about
 *    the Home Domain to which it belongs.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @param[out] pxHomeDomainsCreationDate
 *            creation time of the home domains entitlement expressed as an unsigned number of seconds
 *            elapsed since January 1st, 1970, at midnight, UTC. \n
 *            End of time expressed by this value is in 2106.
 *  @param[out] pxHomeDomainsExpirationDate
 *            UTC date at which the data of the Home domains cannot be used anymore,
 *            expressed as an unsigned number of seconds elapsed
 *            since January 1st, 1970, at midnight, UTC. \n
 *            End of time expressed by this value is in 2106.
 *  @param[out] pxHomeDomainsRenewalDate
 *            Date at which the Home domains SHOULD be renewed,
 *            expressed as an unsigned number of seconds elapsed
 *            since January 1st, 1970, at midnight, UTC. \n
 *            End of time expressed by this value is in 2106.
 *  @param[out] pxNumberOfHomeDomains
 *            Number of Home Domains to which the STB belongs
 *            It corresponds to the number of homeDomainInformation items contained in
 *            \c ppxHomeDomainInfoItems.
 *  @param[out] ppxHomeDomainInfoItems
 *            Array containing information about the Home Domain to which the Set Top Box belongs.\n
 *            This buffer shall be released by calling the function \c dvlReleaseBuffer().\n
 *            This array might be empty
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One of the input pointers of the function is invalid.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @see ::dvlReleaseBuffer()
*/
NAGRA_CA_API TDvlStatus dvlHomeDomainsInformationRequest
(
  TUnixDate*                  pxHomeDomainsCreationDate,
  TUnixDate*                  pxHomeDomainsExpirationDate,
  TUnixDate*                  pxHomeDomainsRenewalDate,
  TUnsignedInt8*              pxNumberOfHomeDomains,
  const TDvlHomeDomainInfo**  ppxHomeDomainInfoItems
);

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This function allows the STB application to send to the DVR/VOD Library
 *    a Home Network Entitlement sent by the Head End.<br>
 *    The DVR/VOD Library processes the entitlement in the function itself. That
 *    means the STB application can release the buffer once the function
 *    returns.<br>
 *    The buffer pointed by \p pxHde shall remain valid
 *    during the whole function processing. Once the function returns, the STB
 *    application can release the pointed memory buffer.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    Home domain information are updated
 *
 *  @param[in] xHdeSize
 *            Size of the Home Domain Entitlement.
 *  @param[in] pxHde
 *            Pointer to the buffer of Bytes containing the Home Domain
 *            Entitlement.
 *
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed. The entitlement could not
 *            be processed.
 *            It can also occur if the HomeDomainEntitlement is expired,
 *            invalid or older than a previously imported HDE
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.\n
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlPostHDE
(
  TSize                 xHdeSize,
  const TUnsignedInt8*  pxHde
);

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This function allocates a TDvlHomeDomainContentMessageParameters structure,
 *    initializes it with default values and returns it to the application.
 *    At this time and if the result of the function is DVL_NO_ERROR, the
 *    structure contains all the input parameters
 *    with their default values hardcoded in the DVR/VOD
 *    Library. The application SHALL then modify those values and then create
 *    a HomeDomain content message by calling
 *    \c dvlCreateHomeDomainContentMessage().<br>
 *    When no more used, the structure needs to be released by calling
 *    \c dvlReleaseBuffer().<br>
 *    Usage of this function to create the structure is mandatory to ensure
 *    backward compatibility with future evolutions of the DVL.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[out] ppxHlcmParameters
 *            Returned pointer to the structure allocated by the DVL and
 *            containing the default home domain credentials parameters.
 *            If the method fails, the returned pointer is NULL.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function was not able to allocate the structure to return.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            the function was called with a NULL pointer.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @remark
 *    -# The HlcmParameters attributes are initialized with following default values:
 *       (homeDomainNameSize: 0), (pHomeDomainName: NULL), (inputCredentialsSize: 0),
 *        (pInputCredentials:NULL), (standardUsageRulesSize: 0), (pStandardUsageRules: NULL),
 *        (operatorUsageRulesSize: 0), (pOperatorUsageRules: NULL), (expirationDate: 0)
 *    -# homeDomainNameSize, homeDomainName, inputCredentialsSize and inputCredentials
 *       are mandatory and SHALL be updated by the Application.
 *    -# Other attributes are optional and can be modified by the Application
 *    -# If Application does not modify default expirationDate, the input Credentials expiration date value
 *       will be used.
 *
*/
NAGRA_CA_API TDvlStatus dvlCreateHomeDomainContentMessageParameters
(
  TDvlHomeDomainContentMessageParameters**  ppxHlcmParameters
);

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This function allows the STB application to generate a HLCM
 *    (HomeDomain Credential Message).\n
 *    The DVR/VOD Library processes the input LCM and generate a HLCM
 *    for a specific Home Domain and containing LCM information.
 *    The HLCM can be deciphered by any device belonging to this Home Domain.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *    The DVR/VOD Library belo,ngs to the Home Domain
 *
 *  @post
 *
 *
 *  @param[in] pxHlcmParameters
 *            Pointer on a HLCM parameters structure allocated
 *            by the \c dvlCreateHomeDomainContentMessageParameters function
 *  @param[out] pxOutputCredentialsSize
 *            Pointer where the DVR/VOD Library will store the size in bytes
 *            of the buffer containing the output credential
 *  @param[out] ppxOutputCredentials
 *            Pointer where the DVR/VOD Library will store the pointer to the memory
 *            containing the HomeDomain Credential Message
 *  @retval DVL_NO_ERROR
 *            The message was successfully processed.
 *  @retval DVL_ERROR
 *            The function execution failed. The entitlement could not
 *            be processed.
 *            This error can also be raised if LCM or HomeDomainName is invalid
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlCreateHomeDomainContentMessage
(
  TDvlHomeDomainContentMessageParameters*  pxHlcmParameters,
  TSize*                                   pxOutputCredentialsSize,
  const TUnsignedInt8**                    ppxOutputCredentials
);

/**
 *  @ingroup groupHomeDomain
 *  @brief
 *    This function allows the STB application to get the execution status of
 *    previously executed Home domain API.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None.
 *
 *  @param[out] pxHDStatus
 *            Pointer where the DVR/VOD Library will store the execution status
 *            of previously executed Home Domain API.<br>
 *            \c dvlPostHDE<br>
 *            \c dvlHomeDomainsInformationRequest<br>
 *            \c dvlCreateHomeDomainContentMessageParameters<br>
 *            \c dvlCreateHomeDomainContentMessage<br>
 *
 *  @retval DVL_NO_ERROR
 *            Execution status of Home Domain API is successfully returned.
*/
NAGRA_CA_API TDvlStatus dvlGetLastHDStatus
(
  TDvlHDStatus*  pxHDStatus
);

#ifdef __cplusplus
}
#endif

#endif /* NV_HOMEDOMAIN_H */

/* END OF FILE */
