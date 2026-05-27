/**
** @file nv_import.h
**
** @brief
**   DVR/VOD Library Entitlements importation management functions.
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


#ifndef NV_IMPORT_H
#define NV_IMPORT_H

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
/*                                     TYPES                                  */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupImport
 *  @brief
 *    This enumeration defines the possible status returned by an importation
 *    request.<br>
*/
typedef enum
{
  DVL_ENTITLEMENTS_IMPORT_NO_ERROR = 0,
  /**<
     *    The importation request has successfully terminated.
     *    All entitlements were imported successfully.
    */
  DVL_ENTITLEMENTS_IMPORT_ERROR_MEMORY_FULL,
  /**<
     *    At least one entitlement was not stored successfully in DVL cache memory
     *    because not enough space is available in DVL cache.
    */
  DVL_ENTITLEMENTS_IMPORT_ERROR_NOT_ALL_IMPORTED,
  /**<
     *    At least one entitlement was not imported
    */
  DVL_ENTITLEMENTS_IMPORT_ERROR,
  /**<
     *    The importation request failed.
     *    No entitlement was imported successfully.
    */
  LAST_DVL_ENTITLEMENTS_IMPORT_STATUS
  /**<
     * Last DVL importation status. This status is never returned by the DVR/VOD Library
    */
} TDvlEntitlementsImportStatus;

/**
 *  @ingroup groupImport
 *  @brief
 *    Definition of the Handle that uniquely identifies an
 *    importation session.
 *
 *  @remark
 *    -# The type \c TDvlImportHandle is opaque to client applications to prevent
 *    external access. A pointer on a \c TDvlImportHandle is returned when calling
 *     \c dvlImportEntitlements(). \n
 *    The type \c TDvlImportHandle should always be used as a pointer by a client
 *    application.
*/
typedef struct SDvlImportHandle TDvlImportHandle;

/**
 *  @ingroup groupImport
 *  @brief
 *    Definition of the Structure that describes an
 *    Entitlement to be stored by DVL in cache or persistent storage.
 *
*/
typedef struct
{
	TSize  firstCredentialsSize;
	/**<
     *   First credential size <br>
     *   Size in bytes of the mandatory first credential/entitlement to import.
     *   Shall not be set to 0.
    */
  TUnsignedInt8*  firstCredentials;
  /**<
     *   First credential buffer.<br>
     *   Pointer to the mandatory first credential/entitlement to import.
     *   - Credential importation => type DVL EMM
     *   - Entitlement importation => DVL EMM + DVL ECM concatenated.
     *   Shall not be NULL.
   */
  TSize  secondCredentialsSize;
  /**<
     *   Second credential size <br>
     *   Size in bytes of the optional second credential to import
     *   Shall be 0 if no second credential is supplied.
   */
  TUnsignedInt8*  secondCredentials;
  /**<
     *   Second credential  buffer.<br>
     *   Pointer to the optional second credential to import => type DVL ECM.
     *   Shall be NULL if no second credential is supplied.
   */
  TSize prmSyntaxSize;
  /**<
     *   PrmSyntax size <br>
     *   Size in bytes of the prm syntax related to the entitlement.
     *   Shall be 0 if no prmSyntax is supplied.
   */
  TUnsignedInt8* pPrmSyntax;
  /**<
     *   PrmSyntax buffer <br>
     *   Pointer to the prm syntax related to the entitlement.
     *   Shall be NULL if no prmSyntax is supplied. <br>
     *   Supplying prmSyntax permits to optimize license descrambling
     *   when several licenses are supplied and one is needed immediately.
   */
} TDvlEntitlementImport;


/**
 *  @ingroup groupImport
 *  @brief
 *    Importation Notification callback when importing entitlements.
 *
 *    This callback is called by the DVR/VOD Library when the importation of
 *    all entitlements is finished.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[out] pxHandle
 *           Pointer to the related handle returned by \c dvlImportEntitlements()
 *  @param[out] importStatus
 *            DVL importation status returned by the DVR/VOD Library.
 *
 *  @remark
 *    -# This function is called in the DVR/VOD Library context. Therefore,
 *       its execution should be as short as possible. It must not block the
 *       calling task.
 *    -# As consequence of previous remark, no other function of this API
 *       shall be called directly inside the notification callback.
 *
*/
typedef void (*TDvlEntitlementsImportNotificationCB)
(
  TDvlImportHandle*             pxHandle,
  TDvlEntitlementsImportStatus   xImportStatus
);


/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupImport
 *  @brief
 *    This function imports one or several Entitlements supplied by the Application. \n
 *    Note that Application Data should be a base64 encoded string or binary data.
 *    Importation is asynchronous, so this function returns immediately. When all entitlements are imported in DVL, the callback is called in
 *    order to notify the Application that importation is terminated.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running. <br>
 *
 *  @post
 *   The Entitlements start being processed asynchronously by DVL in order to be stored in its cache or
 *   persistent storage.
 *
 *  @param[in] xEntitlementNumber
 *            number of the entitlements to import. <br>
 *            Shall not be 0.
 *  @param[in] ppxEntitlementsArray
 *            Pointer to the array containing the Entitlements to import. <br>
 *            Shall not be \c NULL.
 *  @param[out] ppxHandle
 *            Pointer where the DVR/VOD Library will store the Handle related to the importation request.
 *            This handle will be supplied in related notification callbacks.
 *  @param[in] xImportNotificationCB
 *            Callback which will be called when all entitlements have been processed. <br>
 *            Shall not be \c NULL.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus  dvlImportEntitlements
(
  TUnsignedInt32                         xEntitlementNumber,
  TDvlEntitlementImport**              ppxEntitlementsArray,
  TDvlImportHandle**                   ppxHandle,
  TDvlEntitlementsImportNotificationCB   xImportNotificationCB
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function release an importation session launched with \c dvlImportEntitlements().
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running. <br>
 *
 *  @post
 *   If not terminated, the importation is aborted and related callback will not be called.
 *   Then, the related resources are released
 *
 *  @param[in] pxHandle
 *            Pointer to the Handle related to the importation request.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlEntitlementsImportationRelease
(
       TDvlImportHandle*  pxHandle
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function creates a JSON payload for a DVS request.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *
 *
 *  @param[in] xPrmSyntaxSize
 *            Size in bytes of the prmSyntax buffer\n
 *            Can be 0 if no prmSyntax is supplied.
 *  @param[in] pxPrmSyntax
 *            pointer to a buffer containing the prmSyntax<br>
 *            Can be NULL if no prmSyntax is supplied.
 *  @param[in] xClientProtectedPrivateDataSize
 *            Size in bytes of the buffer containing the Protected Private Data. <br>
 *            Can be 0 if no protected private data.
 *  @param[in] pxClientProtectedPrivateData
 *            Pointer to the buffer containing the Protected Private Data. <br>
 *            Those Private data are inserted in opaque data of any communication with the Server
 *            and must be used to transmit sensible information like the session token <br>
 *            Can be NULL if no protected private data.
 *  @param[in] xClientClearPrivateDataSize
 *            Size in bytes of the buffer containing the Clear Private Data. <br>
 *            Can be 0 if no clear private data.
 *  @param[in] pxClientClearPrivateData
 *            Pointer to the buffer containing the Clear Private Data. <br>
 *            Those Private data are inserted in clear during any communication with the Server.
 *            These data can be processed by the server which can return an answer in the response.
 *            This answer can be fetched via the parameter \c pxServerPrivateDataString of the related
 *            \c xCommunicationNotificationCB callback. <br>
 *            Can be NULL if no clear private data is supplied.
 *  @param[out] pxJSONPayloadForServerSize
 *            Pointer where the function will store the size in bytes of the generated payload
 *  @param[out] ppxJSONPayloadForServer
 *            Pointer in which the function will store the address of the buffer containing
 *            the generated payload to supply to the license Server.\n
 *            Shall not be \c NULL.\n
 *            When no more used, this buffer shall be released by Application with \c dvlReleaseBuffer().
 *
 *  @retval DVL_NO_ERROR
 *             The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *             One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *             The function execution failed.
 */
NAGRA_CA_API TDvlStatus  dvlCreateJSONPayloadForServer
(
        TSize                                  xPrmSyntaxSize,
        TChar*                                pxPrmSyntax,
        TSize                                  xClientProtectedPrivateDataSize,
        TChar*                                pxClientProtectedPrivateData,
        TSize                                  xClientClearPrivateDataSize,
        TChar*                                pxClientClearPrivateData,
        TSize*                                pxJSONPayloadForServerSize,
        TChar**                              ppxJSONPayloadForServer
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function extracts DVS status from a JSON response from license server.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *  @post
 *
 *
 *  @param[in] xJsonResponseSize
 *            Size in bytes of JSON Response supplied by the license Server.
 *  @param[in] pxJsonResponse
 *            Address of the buffer containing the JSON response supplied by the license Server.\n
 *            Shall not be \c NULL.
 *  @param[out] pxStatus
 *            Pointer where the function will store the status in response.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlJSONResponseGetStatus
(
        TSize                                xJsonResponseSize,
        TUnsignedInt32*                     pxJsonResponse,
        TUnsignedInt32*                     pxStatus
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function extracts LCIs from a JSON response from license server.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *  @post
 *
 *
 *  @param[in] xJsonResponseSize
 *            Size in bytes of JSON Response supplied by the license Server.
 *  @param[in] pxJsonResponse
 *            Address of the buffer containing the JSON response supplied by the license Server.\n
 *            Shall not be \c NULL.
 *  @param[out] pxLciNumber
 *            Pointer where the function will store the number of LCI in response.
 *  @param[out] pppxLciArray
 *            Pointer where the DVL will store the address of the array containing
 *            pointers to the LCIs buffers found in response in hexadecimal binary format.\n
 *            This array shall be released with \c dvlReleaseBuffer which will
 *            release the array with all its items.
 *            This array might be empty if no LCI are found in response\n
 *            Shall not be \c NULL.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlJSONResponseGetLCIs
(
        TSize                                xJsonResponseSize,
        TUnsignedInt32*                     pxJsonResponse,
        TUnsignedInt32*                     pxLciNumber,
  const TDvlBuffer***                     pppxLciArray
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function extracts HDE from a JSON response from license server.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *  @post
 *
 *
 *  @param[in] xJsonResponseSize
 *            Size in bytes of JSON Response supplied by the license Server.
 *  @param[in] pxJsonResponse
 *            Address of the buffer containing the JSON response supplied by the license Server.\n
 *            Shall not be \c NULL.
 *  @param[out] pxHdeSize
 *            Pointer where the function will store the size in bytes of the HDE supplied in response
 *  @param[out] ppxHde
 *            address of the buffer containing HDE supplied in response\n
 *            Shall not be \c NULL.\n
 *            If HDE is supplied, this buffer shall be released with \c dvlReleaseBuffer().
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlJSONResponseGetHDE
(
        TSize                                xJsonResponseSize,
        TUnsignedInt32*                     pxJsonResponse,
        TSize*                              pxHdeSize,
        TChar**                            ppxHde
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function extracts entitlements from a JSON response from license server.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *
 *
 *  @param[in] xJsonResponseSize
 *            Size in bytes of JSON Response supplied by the license Server.
 *  @param[in] pxJsonResponse
 *            Address of the buffer containing the JSON response supplied by the license Server.\n
 *            Shall not be \c NULL.
 *  @param[out] pxEntitlementsNumber
 *            Pointer where the DVL will store the number of LCI in response.
 *  @param[out] pppxEntitlementsArray
 *            Pointer where the DVL will store the adress of the array containing
 *            Entitlements found in DVS response.\n
 *            This buffer shall be released with \c dvlReleaseBuffer() which will
 *            release the array with all its items. \n
 *            This array might be empty\n
 *            Adress of Array not be \c NULL.
 *
 *  @retval DVL_NO_ERROR
 *             The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *             One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *             The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlJSONResponseGetEntitlements
(
        TSize                                xJsonResponseSize,
        TUnsignedInt32*                     pxJsonResponse,
        TUnsignedInt32*                     pxEntitlementsNumber,
  const TDvlEntitlementImport***          pppxEntitlementsArray
);

/**
 *  @ingroup groupImport
 *  @brief
 *    This function extracts private data from a JSON response from license server.
 *
 *
 *  @pre
 *
 *  @post
 *
 *
 *  @param[in] xJsonResponseSize
 *            Size in bytes of JSON Response supplied by the license Server.
 *  @param[in] pxJsonResponse
 *            Address of the buffer containing the JSON response supplied by the license Server.\n
 *            Shall not be \c NULL.
 *  @param[out] pxPrivateDataSize
 *            Pointer where the function will store the size in bytes of the privatedata supplied in response
 *  @param[out] ppxPrivateData
 *            address of the buffer containing privatedata supplied in response\n
 *            Shall not be \c NULL.\n
 *            If PrivateData are supplied, this buffer shall be released with \c dvlReleaseBuffer().
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlJSONResponseGetPrivateData
(
        TSize                                xJsonResponseSize,
        TUnsignedInt32*                     pxJsonResponse,
        TSize*                              pxPrivateDataSize,
        TChar**                            ppxPrivateData
);

#ifdef __cplusplus
}
#endif

#endif /* NV_IMPORT_H */

/* END OF FILE */
