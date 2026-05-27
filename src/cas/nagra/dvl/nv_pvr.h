/**
** @file nv_pvr.h
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


#ifndef NV_PVR_H
#define NV_PVR_H

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

/**
 *  @brief
 *    Definition of the DVR/VOD Library Handle that uniquely identifies a
 *    recording or playback session.
 *
 *  @remark
 *    -# The type TDvlHandle is opaque to client applications to prevent
 *    external access. A pointer on a TDvlHandle is returned when starting a
 *    record or a playback session and shall be returned when stopping the
 *    session later. This pointer on a TDvlHandle is also given by the DVR/VOD
 *    library in all the error call-backs. The type TdvlHandle should always
 *    be used as a pointer by a client application.
*/
typedef struct SDvlHandle TDvlHandle;

/**
 *  @ingroup groupRecord
 *  @brief
 *    This enumeration defines the possible status returned by a recording
 *    request.<br>
 *    For easy usage, each DENIED value is greater than DVL_RECORD_DENIED.
 *    The client application is encouraged to use this comparison to be
 *    compatible with the future recording status Nagravision could add.
*/
typedef enum
{
  DVL_RECORD_GRANTED = 0,
    /**<
     *    The recording request has successfully terminated.
     *    The content key is now set in the chipset stream cipher and the STB
     *    application can start storing the content on the HDD.
    */
  DVL_RECORD_DENIED = 100,
    /**<
     *    The usage rules associated to this content deny recording it.
     *    No key has been set into the chipset stream cipher and the STB
     *    application shall not store the content on the HDD.
     */
  DVL_RECORD_DENIED_APPENDING_DENIED,
    /**<  The DVR/VOD Library could not append the current recording with the
     *    content related to the input credentials.
     *    The DVR/VOD Library did NOT set any key into the chipset and the STB
     *    application shall not store the content on the HDD.
    */
  DVL_RECORD_DENIED_BAD_PROFILE,
    /**<
     *    Deprecated.
     *    The input Profile of the request is invalid. No key has been set into
     *    the chipset stream cipher and the STB application shall not store the
     *    content on the HDD.
     */
  DVL_RECORD_DENIED_BAD_INPUT_CREDENTIALS,
    /**<
     *    The input Credentials of the request provided for appending purpose
     *    is invalid. No key has been set into the chipset stream cipher and
     *    the STB application shall not store the content on the HDD.
     */
  DVL_RECORD_DENIED_WRONG_TSID,
    /**<
     *    The transport session identifier is unknown by the SEC at that time.
     *    No key has been set into the chipset stream cipher and the STB
     *    application shall not store the content on the HDD.
     */
  DVL_RECORD_DENIED_LIMIT_EXCEEDED,
    /**<
     *    Deprecated.
     *    This status is not returned by the DVL.
     */
  DVL_RECORD_DENIED_INVALID_TIME,
    /**<
     *    The internal time reference is not valid. The DVR/VOD Library is not
     *    authorized to record any content. No key has been set into the
     *    chipset stream cipher and the STB application shall not store the
     *    content on the HDD.
     */
  DVL_RECORD_DENIED_INVALID_EMI,
    /**<
     *    The EMI planned to encrypt the content is not supported by the SCR.
     *    No key has been set into the chipset stream cipher and the STB
     *    application shall not store the content on the HDD.
     */
   LAST_DVL_RECORD_STATUS
    /**<
     *    Last DVR/VOD Library Record status value. This value is never
     *    returned by the DVR/VOD Library.
     */
} TDvlRecordStatus;

/**
 *  @ingroup groupPlayback
 *  @brief
 *    Possible access statuses associated to a provided content.<br>
 *    For easy usage, each DENIED value is greater than DVL_ACCESS_DENIED.
 *    The client application is encouraged to use this comparison to be
 *    compatible with the future access status Nagravision could add.
*/
typedef enum
{
  DVL_ACCESS_GRANTED = 0,
    /**<
     *    The playback of the content is granted.
    */
  DVL_ACCESS_DENIED = 100,
    /**<
     *    The playback of the content is denied.
     */
  DVL_ACCESS_DENIED_CONTENT_EXPIRED,
    /**<
     *    The content's usage rules deny its playback because it is expired.
     */
  DVL_ACCESS_DENIED_INVALID_CREDENTIALS,
    /**<
     *    The provided content Credentials are invalid.
     */
  DVL_ACCESS_DENIED_WRONG_TSID,
    /**<
     *    The transport session identifier is unknown by the SEC at that time.
     *    This status only applies to a playback request.
     */
  DVL_ACCESS_DENIED_LIMIT_EXCEEDED,
    /**<
     *    Deprecated.
     *    This status is not returned by the DVL.
     */
  DVL_ACCESS_DENIED_INVALID_TIME,
    /**<
     *    The internal time reference is not valid. The DVR/VOD Library is not
     *    authorized to play back any content.
     */
  DVL_ACCESS_DENIED_UNKNOWN_STB,
    /**<
     *    The access to the content is denied because it has been
     *    recorded by an unknown set-top box.
     */
  DVL_ACCESS_DENIED_UNKNOWN_HOME_NETWORK,
    /**<
     *    The access to the content is denied because it has been
     *    recorded by a set-top box belonging to an unknow Home Network.
     */
  DVL_ACCESS_DENIED_UNKNOWN_NUID,
    /**<
     *    The access to the content is denied because it has been
     *    recorded by a set-top box with an unknown NUID.
     */
  DVL_ACCESS_DENIED_INVALID_EMI,
    /**<
     *    The EMI used to protect this content is not supported by the SEC
     *    implementation.
     */
  DVL_ACCESS_DENIED_INVALID_KEY_PROTECTION,
    /**<
     *    The keys needed to decipher the content have been protected using a
     *    mechanism not supported by the SEC implementation.
     */
  DVL_ACCESS_DENIED_MISSING_CREDENTIALS,
    /**<
     *    The entitlements needed to decipher the content are not available.
     */
  DVL_ACCESS_DENIED_UNKNOWN_HOME_DOMAIN,
    /**<
     *    The access to the content is denied because it has been
     *    generated for an unknown Home Domain.
     */
  LAST_DVL_ACCESS_STATUS
    /**<
     *    Last DVL Playback status value.
     *    This value is never returned by the DVR/VOD Library.
     */
} TDvlAccessStatus;

/**
 *  @ingroup groupRecord
 *  @brief
 *    This enum defines the two different types of viewing windows:
 *    one relative to the recording date and the other one relative
 *    to the first visualization date.
*/
typedef enum
{
  VIEWING_WINDOW_RELATIVE_TO_CREATION_DATE = 0,
    /**<
     *    The viewing window is relative to the creation date.
     *    In case of a LCM, it corresponds to the recording date.
    */
  VIEWING_WINDOW_RELATIVE_TO_VISUALIZATION,
    /**<
     *    The viewing window is relative to the first visualization date.
    */
  LAST_VIEWING_WINDOW_RELATIVITY
    /**<
     *    Last viewing window relativity.
     *    This value is never returned, nor used by the DVR/VOD Library.
    */
} TViewingWindowRelativity;

/**
 * @ingroup groupRecord
 * @brief
 *    Record error callback.
 *
 *    This callback is called by the DVR/VOD Library if an error occurs during
 *    a record session. In that case, the DVR/VOD Library sets the error status
 *    in the parameter \c xRecordStatus.
 *
 *    If the callback is called by the DVR/VOD Library, the STB Application
 *    shall consider the recording has failed. It shall initiate a message that
 *    will then stop the recording and return from the callback. Once the STB
 *    application runs in its context again, it shall call \c dvlStopRecord().
 *
 *    This callback can only be called by the DVR/VOD Library as long as the
 *    recording handle is valid (i.e. as long as the STB Application has not
 *    called \c dvlStopRecord() with that recording handle).
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[in] pxRecordHandle
 *            Pointer to the handle of the concerned recording session.
 *  @param[in] xRecordStatus
 *            DVL Record status returned by the DVR/VOD Library.
 *
 *  @remark
 *    -# This function is called in the DVR/VOD Library context. Therefore,
 *       its execution should be as short as possible. It must not block the
 *       calling task.
 *    -# As consequence of previous remark, no other function of this API
 *       shall be called directly inside the notification callback.
 *
*/
typedef void (*TDvlRecordErrorCB)
(
  TDvlHandle*      pxRecordHandle,
  TDvlRecordStatus  xRecordStatus
);

/**
 *  @ingroup groupRecord
 *  @brief
 *    This structure contains the details of the recording parameters used by
 *    the DVL. To ease the management of default values and API compatibility,
 *    this structure SHALL only be allocated by the DVL by calling
 *    dvlCreateRecordSessionParameters and never by the application itself.
 *    When this structure is released by calling dvlReleaseBuffer, the memory
 *    pointed by pInputCredentials and pSpecificMetadata is untouched.
*/
typedef struct
{
  TTransportSessionId       tsId;
    /**<
     *    Identifier of transport stream to be recorded.
    */
  TDvlRecordErrorCB         pErrorCB;
    /**<
     *    Callback used by the DVR/VOD Library to report a recording error.
    */
  TUnsignedInt32            collectionId;
    /**<
     *    Unique identifier of the collection to which the content belongs.
     *    The collection identifier is used for recorded content appending
    */
  TSize                     inputCredentialsSize;
    /**<
     *    Size of the optional input Credentials.
     *    Shall be set to zero in case no input Credentials is pased to
     *    the DVR/VOD Library.
    */
  TUnsignedInt8*            pInputCredentials;
    /**<
     *    Optional Credentials of the content to which the current recording
     *    should be appended.
     *    Should be set to \c NULL if the current recording is not intended
     *    to be appended to any other content.
    */
  TSize                     specificMetadataSize;
    /**<
     *    Size of the optional specific metadata to be stored in the output
     *    Credentials. Shall be set to zero in case no specific related
     *    metadata are passed to the DVR/VOD Library.
    */
  TUnsignedInt8*            pSpecificMetadata;
    /**<
     *    Optional specific metadata to be stored in the output Credentials.
     *    Shall be set to NULL in case no specific metadata are passed to
     *    the DVR/VOD Library.
    */
  TViewingWindowRelativity  viewingWindowRelativity;
    /**<
     *    Relativity of the viewing window.
     *    @see ::TViewingWindowRelativity
    */
  TCaDuration               viewingWindowDuration;
    /**<
     *    Number of seconds the content recorded can be played back.
     *    The viewing window can be relative to the recording date or to the
     *    content first visualization date.
    */
  TCaDuration               retentionDuration;
    /**<
     *    Number of seconds the content recorded can stay stored on the HDD. It
     *    corresponds to the number of seconds after which the content expires.
    */
  TUnsignedInt16            emi;
    /**<
     *    EMI used for content encryption
    */
  TNvUserIntent  userIntent;
  /**<  Application user intent.<br>
    *   This user intent will be set in SEC.
    *   User Intent feature is only available in DVL Dual Library
    *   with SEC version 6.3.0 onwards
    */
  TBoolean secureMediaPathEnabled;
  /**<  Application defined SMP option.<br>
    *   Middleware sets the secure media path.
    *   Secure Media Path feature is only available in DVL Dual Library
	*   @warning
    *   For device without secure media path support, DVL Dual library delivery
    *   is not built with secure media path feature and the value of this field
    *   is ignored by DVL Dual library.
    */
} TDvlRecordSessionParameters;

/**
 * @ingroup groupPlayback
 * @brief
 *    Playback Notification callback on an Entitlement.
 *
 *    This callback is called by the DVR/VOD Library if a change of access status occurs during
 *    a playback session. In that case, the DVR/VOD Library sets the
 *    corresponding access status in the parameter \c xPlaybackStatus.
 *    It also set the privateData pointer with the value supplied when calling \p dvlPlaybackSessionSetEntitlements()
 *    or \c dvlPlaybackSessionSetRelatedEntitlements.
 *
 *    If the callback is called by the DVR/VOD Library with access no more granted, the Application
 *    shall consider the playback is no more allowed. It shall initiate a
 *    message that will then take proper action of the concerned playback and return from the
 *    callback.
 *
 *    This callback can only be called by the DVR/VOD Library as long as the
 *    playback handle is valid (i.e. as long as the Application has not
 *    called \c dvlPlaybackCloseSession() with this playback handle).
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[out] pxPlaybackHandle
 *            Pointer to the handle of the concerned playback session.
 *  @param[out] pxPrivateData
 *            Pointer to Application Private Data allowing Application to store
 *             any contextual information
 *  @param[out] xPlaybackStatus
 *            DVL playback status returned by the DVR/VOD Library.
 *
 *  @remark
 *    -# This function is called in the DVR/VOD Library context. Therefore,
 *       its execution should be as short as possible. It must not block the
 *       calling task.
 *    -# As consequence of previous remark, no other function of this API
 *       shall be called directly inside the notification callback.
 *
*/
typedef void (*TDvlPlaybackNotificationCB)
(
  TDvlHandle*      pxPlaybackHandle,
  void *           pxPrivateData,
  TDvlAccessStatus  xPlaybackStatus
);



/**
 *  @ingroup groupPlayback
 *  @brief
 *    Structure used when calling the dvlPlaybackSetEntitlements() request.
 *    It contains input credentials and Application Private Data that will be
 *    returned in dvlPlaybackNotificationCallback. <br>
 *    Current known Entitlement format for Playback are:
 *    - One Credentials buffer required to perform the playback
 *    - Two Credentials buffer required to perform the playback
*/
typedef struct
{
	TSize  firstCredentialsSize;
	/**<  1st credentials size.<br>
	  * Size in bytes of the mandatory first input credential
    */
  TUnsignedInt8*  firstCredentials;
  /**<  1st credentials buffer.<br>
    *    Pointer to the  first input Credentials related to the content to play back.
    *    During a call to dvlPlaybackSetEntitlements(), the DVR/VOD Library won't modify that buffer. But the STB
    *      application shall replace on the content storage this input
    *    Credentials by the output Credentials when output credentials is supplied.<br>
    */
  TSize  secondCredentialsSize;
  /**<  2nd credentials size.<br>
    *   Size in bytes of the second input credential.
    *   This field is only used when 2 credentials buffer are required, else it shall be 0.
    */
  TUnsignedInt8*  secondCredentials;
  /**<  2nd credentials buffer.<br>
    *    Pointer to the second input Credentials (type DVL ECM)
    *      related to the content to play back.
    *    During a call to dvlPlaybackSetEntitlements(), the DVR/VOD Library won't modify that structure. But the STB
    *      application shall replace on the content storage the input
    *    Credentials by the output Credentials when output credentials is supplied.<br>
    *    This field is only used when 2 credentials are needed, else it shall be NULL.
    */
  TDvlPlaybackNotificationCB   playbackNotificationCB;
  /**<  Entitlement Notification Callback.<br>
    *      Callback used by the DVR/VOD Library to report any playback error on the related Entitlement.
    */
  void* privateData;
  /**<  Application private data for NotificationCallback.<br>
    *   The Application can put any information at this address
    *   in order to be able to manage correctly the callback.
    */

  TNvUserIntent  userIntent;
  /**<  Application user intent.<br>
    *   This user intent will be set in SEC.
    *   User Intent feature is only available in DVL Dual Library
    *   with SEC version from 6.3.0 onwards.
    */
  TBoolean secureMediaPathEnabled;
  /**<  Application defined SMP option.<br>
    *   Middleware sets the secure media path.
    *   Secure Media Path feature is only available in DVL Dual Library.
	*   @warning
    *   For device without secure media path support, DVL Dual library delivery
    *   is not built with secure media path feature and the value of this field
    *   is ignored by DVL Dual library.
    */
} TDvlEntitlementsParameters;


/******************************************************************************/
/*                          DVR/VOD API LIBRARY                               */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupPlaybackRecordInfo
 *  @brief
 *    This function returns the Key Slot ID.
 *
 *  @note
 *    This API is available for DVL Dual Library only.
 *
 *  @pre
 *    Handle should be valid
 *
 *  @post
 *    None
 *
 *  @param[in] pxHandle
 *            Pointer to the session.
 *
 *  @param[out] pxKeySlotId
 *            Pointer to return Key Slot ID.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The function execution failed due to bad parameter.
 *  @retval DVL_ERROR
 *            The function execution failed due to some other error.
*/
NAGRA_CA_API TDvlStatus dvlGetKeySlotIdentifier
(
    TDvlHandle      *pxHandle,
    TUnsignedInt16  *pxKeySlotId
);

/**
 *  @ingroup groupRecord
 *  @brief
 *    This function allocates a TDvlRecordSessionParameters structure,
 *    initializes it with default values and returns it to the application.
 *    At this time and if the result of the function is DVL_NO_ERROR, the
 *    structure contains all the input parameters of a
 *    recording session with their default values hardcoded in the DVR/VOD
 *    Library. The application SHALL then modify those values according to
 *    content usage rules and then start recording sessions by calling
 *    \c dvlStartRecordEx.<br>
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
 *  @param[out] ppxRecordParameters
 *            Returned pointer to the structure allocated by the DVL and
 *            containing the default recording parameters.
 *            If the method fails, the retuned pointer is NULL.
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
 *    -# The TTransportSessionId attribute of the structure is initialized by
 *      default to TRANSPORT_SESSION_INVALID and SHALL be configured by the
 *      application.
 *    -# The TDvlRecordErrorCB attribute of the structure is initialized by
 *      default to NULL and SHALL be configured by the application.
 *    -# The rest of the attributes of the structure CAN be modified by the
 *      application and have the following default values:
 *     (collectionId : 0xFFFFFFFF), (inputCredentialsSize : 0),
 *     (pInputCredentials : NULL), (specificMetadataSize : 0),
 *     (pSpecificMetadata : 0), (viewingWindowRelativity : VIEWING_WINDOW_RELATIVE_TO_CREATION_DATE),
 *     (viewingWindowDuration : 0xFFFFFFFF), (retentionDuration : 0xFFFFFFFF)
 *     (emi : default value set at DVL compilation time)
*/
NAGRA_CA_API TDvlStatus dvlCreateRecordSessionParameters
(
        TDvlRecordSessionParameters** ppxRecordParameters
);

/**
 *  @ingroup groupRecord
 *  @brief
 *    This function sends a record session request to the DVR/VOD Library.
 *    When this function sets \c DVL_RECORD_GRANTED into \c *pxRecordingStatus,
 *    the corresponding content key has been properly set into the chipset
 *    stream cipher and the STB application can start storing the content on
 *    the HDD.<br>
 *    This function also allows appending the current record to a previous
 *    recorded content, by passing as argument the Credentials related to
 *    the previous recorded content to which the current recording shall be
 *    appended. In append mode, the function ignores the specific metadata,
 *    viewing windows and retention duration specified in the the
 *    TDvlRecordSessionParameters.<br>
 *    - If the append is authorized, the current content will be recorded
 *    with same content key as the event to which it is appended. The output
 *    Credentials then apply to both contents, the previously recorded one and
 *    the currently recorded one; the STB application may then consider the
 *    two records as only one.<br>
 *    - If the append fails (record status \c DVL_RECORD_DENIED_APPENDING_DENIED
 *    is returned in \c *pxRecordingStatus), no content key is set into the
 *    stream cipher of the chipset. In that case, the STB application may
 *    still try a standalone recording (without passing any input
 *    Credentials).<br>
 *    - If the function returned with the record status DVL_RECORD_GRANTED, the
 *    STB application shall copy/store the content of the output Credentials
 *    buffer on the HDD and then release it by calling the function
 *    \c dvlReleaseBuffer().<br>
 *    Once the function returned with the record status \c DVL_RECORD_GRANTED,
 *    the DVR/VOD Library may call the passed record error callback at any
 *    time. The main reason is that the recording may be subject to a time
 *    limit and the recording is suddenly denied. In that case, the STB
 *    application shall stop the current recording session. When the function
 *    returns with error status \c DVL_NO_ERROR, it does not necessarily mean
 *    the recording is successful. The STB Application still needs to check the
 *    value set in the record status \c *pxRecordStatus parameter.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] pxRecordParameters
 *            Pointer on a record parameters structure allocated by the
 *            dvlCreateRecordSessionParameters function.
 *  @param[out] pxOutputCredentialsSize
 *            Pointer to the length of the output Credentials.
 *            This size is set to zero by the DVR/VOD Library in case the
 *            returned status is not equal to \c DVL_RECORD_GRANTED.
 *  @param[out] ppxOutputCredentials
 *            This pointer will contain the address of the memory where the
 *            output Credentials will be stored if the function returns
 *            DVL_RECORD_GRANTED. In case the function returns any other
 *            status, the returned address will be set to \c NULL. The memory
 *            pointed by \c *ppxOutputCredentials shall be released by the STB
 *            application by calling the dedicated function
 *            \c dvlReleaseBuffer().
 *  @param[out] pxRecordingStatus
 *            Pointer to the memory where the DVR/VOD Library will store the
 *            recording status.<br>
 *            If pxRecordingStatus is greater or equal to \c DVL_RECORD_DENIED,
 *            the content key has not been set into the chipset stream cipher
 *            and the content cannot be recorded.
 *  @param[out] ppxRecordHandle
 *            Pointer where the DVR/VOD Library will store the recording handle.
 *            The input pointer shall not be \c NULL.
 *            The value set by the DVR/VOD Library is significant only if
 *            \c *pxRecordingStatus is \c DVL_RECORD_GRANTED.
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
 *  @remark
 *    -# If the function returns \c DVL_NO_ERROR and \c *pxRecordStatus contains
 *      \c DVL_RECORD_GRANTED, the STB application shall copy/store the content
 *      of the output Credentials buffer on the HDD and then release it by
 *      calling the function \c dvlReleaseBuffer().
 *    -# The DVL itself supports several recording sessions at the same time,
 *      the underlying STB drivers might not. When they do, the
 *      TTransportSessionId used to start each record SHALL be configured
 *      accordingly.
*/
NAGRA_CA_API TDvlStatus dvlStartRecordEx
(
        TDvlRecordSessionParameters*  pxRecordParameters,
        TSize*                        pxOutputCredentialsSize,
  const TUnsignedInt8**              ppxOutputCredentials,
        TDvlRecordStatus*             pxRecordingStatus,
        TDvlHandle**                 ppxRecordHandle
);

/**
 *  @ingroup groupRecord
 *  @brief
 *   This function stops the current recording session handled by
 *   \c pxRecordHandle. The session has been started by \c dvlStartRecordEx().
 *
 *  @note
 *    In case of DVL Dual, the key set for recording session will be wiped off.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *    A valid recording session must be running, handled by the parameter
 *    \c pxRecordHandle.
 *
 *  @post
 *    None
 *
 *  @param[in] pxRecordHandle
 *            Recording session handle.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The input argument of the function is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlStopRecord
(
  TDvlHandle*  pxRecordHandle
);

/**
 *  @ingroup groupRecord
 *  @brief
 *   This function validates whether the input LCM created for TIMESHIFT can be
 *   converted for RECORD or not.  If it is allowed, it creates a new LCM for RECORD.
 *
 *  @note
 *    This API will be available for DVL Dual Library only.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in]  xInputLcmSize
 *                Input LCM buffer size.
 *
 *  @param[in]  pxInputLcm
 *                Buffer holding input LCM.
 *
 *  @param[in]  xRetentionDuration
 *                Number of seconds the content recorded can
 *                stay stored on the HDD. It corresponds to
 *                the number of seconds after which the content
 *                expires.
 *
 *  @param[out] pxOutputLcmSize
 *                Output LCM buffer size.
 *
 *  @param[out] ppxOutputLcm
 *                Buffer holding output LCM.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The input argument of the function is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/

NAGRA_CA_API TDvlStatus dvlConvertTimeshiftToRecording
(
        TSize                        xInputLcmSize,
  const TUnsignedInt8*              pxInputLcm,
        TCaDuration                  xRetentionDuration,
        TSize*                      pxOutputLcmSize,
  const TUnsignedInt8**            ppxOutputLcm
);

/**
 *  @ingroup groupPlayback
 *  @brief
 *    This function opens a Playback Session for the specified TSId.<br>
 *    The application can use this session to set keys embedded in Entitlements.<br>
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xTSid
 *            Identifier of transport session to be played back.
 *  @param[out] ppxPlaybackHandle
 *            Pointer where the DVR/VOD Library will store the playback session
 *            handler.
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
*/
NAGRA_CA_API TDvlStatus dvlPlaybackOpenSession
(
        TTransportSessionId   xTSid,
        TDvlHandle**        ppxPlaybackHandle
);

/**
 *  @ingroup groupPlayback
 *  @brief
 *    This function gives Entitlements to the DVR/VOD library.<br>
 *    - When this function returns with the status \c DVL_ACCESS_GRANTED set in
 *    \c *pxPlaybackStatus, that means that the corresponding content key has
 *    been correctly set in the chipset stream cipher and the STB application
 *    can start playing back the content.
 *    - If the function returns with the playback status \c DVL_ACCESS_GRANTED,
 *    the STB application should copy/store the content of the output
 *    Credentials buffer and then release it by calling the function
 *    \c dvlReleaseBuffer(). That output Credentials should replace any one
 *    already existing for that content. Nevertheless, if the application
 *    does not store credentials, it can ignore this output buffer.
 *
 *    Once the function returned with the playback status \c DVL_ACCESS_GRANTED,
 *    the DVR/VOD Library may call the playback error callback supplied in
 *    pxEntitlementsParameters at any time. The main reason is that the playback may be subject to a time limit
 *    and the playback is suddenly denied. In that case, the STB application
 *    shall stop the current playback session.<br>
 *    When the function returns with error status \c DVL_NO_ERROR, it does not
 *    necessarily mean the playback is successful. The STB Application still
 *    needs to check the value set in the playback status \c *pxPlaybackStatus
 *    parameter.<br>
 *    @remarks
 *    Note that this function can be called several times during a Playback session
 *    in order to load several keys. It enables to manage key change smoothly. The keys
 *    table is managed in circular buffer mode, that means that when the table is full,
 *    the oldest key is replaced by the new key. Default size of this buffer is 2: that
 *    means that at most 2 entitlements are loaded simultaneously and older entitlements
 *    related DVL resources (like callback) are freed.
 *    Moreover, If several Entitlements are set with the same KeyId, the Callback will not be called for former Entitlements
 *    since the key is overriden.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] pxPlaybackHandle
 *            Identifier of the playback session previously created with dvlPlaybackOpenSession().
 *  @param[in] pxEntitlementsParameters
 *            Structure containing Credentials and associated Notification Callback.
 *  @param[out] pxPlaybackStatus
 *            Pointer where the DVR/VOD Library will store the playback status.
 *            If \c pxPlaybackStatus is greater or equal to
 *            \c DVL_ACCESS_DENIED, the content key has not been set into the
 *            chipset stream cipher and the content cannot be played back.
 *  @param[out] pxOutputCredentialsSize
 *            Pointer where the DVR/VOD Library will store the size in bytes of the
 *            buffer containing the output credential.
 *  @param[out] ppxOutputCredentials
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the output Credentials.<br>
 *            - If the input pointer is \c NULL, the DVR/VOD library will not fill this field.
 *            - Else, the STB application shall replace all input Credentials on the
 *            content storage by the output Credentials and then release the
 *            buffer by calling \c dvlReleaseBuffer().
 *  @param[out] pxKeyIdSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output keyId.
 *            The input pointer shall not be NULL if ppxKeyId is not NULL.
 *            If no KeyId is present in the Entitlement, the KeyId size is set to zero.
 *  @param[out] ppxKeyId
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the KeyId. <br>
 *            This buffer can be \c NULL if the caller is not interested in retrieving KeyId.<br>
 *            Else:
 *            - If no KeyId is present in the Entitlement, *ppxKeyId will be set to NULL.
 *            - else the STB application shall release the buffer by calling \c dvlReleaseBuffer().
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
*/
NAGRA_CA_API TDvlStatus dvlPlaybackSetEntitlements
(
        TDvlHandle*                 pxPlaybackHandle,
        TDvlEntitlementsParameters* pxEntitlementsParameters,
        TDvlAccessStatus*           pxPlaybackStatus,
        TSize*                      pxOutputCredentialsSize,
  const TUnsignedInt8**            ppxOutputCredentials,
        TSize*                      pxKeyIdSize,
  const TUnsignedInt8**            ppxKeyId
);

/**
 *  @ingroup groupPlayback
 *  @brief
 *    This function is responsible for closing the playback session, which
 *    consists in the cleanup of the session's resources.
 *
 *  @note
 *    In case of DVL Dual, the key set for playback session will be wiped off.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *    A valid playback session created with dvlPlaybackOpenSession
 *    must be open, handled by the parameter pxPlaybackHandle.
 *
 *  @post
 *    None
 *
 *  @param[in] pxPlaybackHandle
 *            Handle of the playback session to close.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The playback handle is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlPlaybackCloseSession
(
        TDvlHandle*        pxPlaybackHandle
);

/**
 *  @ingroup groupMetadata
 *  @brief
 *    This function allows the DVR/VOD Library client to replace the specific
 *    metadata stored inside an LCM credential. The metadata can be read back
 *    by using \c dvlCredentialsInformationRequest(). The update credential
 *    returned by this function has to be released by calling
 *    \c dvlReleaseBuffer().
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xInLcmCredentialsSize
 *            Size in Bytes of the buffer containing the LCM Credentials to
 *            modify.
 *  @param[in] pxInLcmCredentials
 *            Pointer to the buffer containing the LCM Credentials to modified.
 *  @param[in] xSpecificMetadataSize
 *            Size of the specific metadata. The maximum size supported is
 *            63 bytes.
 *  @param[in] pxSpecificMetadata
 *            Pointer on the specific metadata.
 *  @param[out] pxOutLcmCredentialsSize
 *            Returned size in Bytes of the buffer containing the updated
 *            LCM Credentials.
 *  @param[out] ppxOutLcmCredentials
 *            Returned pointer to the buffer containing the updated
 *            LCM Credentials.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The input argument of the function is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlUpdateSpecificLcmMetadata
(
        TSize                 xInLcmCredentialsSize,
  const TUnsignedInt8*       pxInLcmCredentials,
        TSize                 xSpecificMetadataSize,
  const TUnsignedInt8*       pxSpecificMetadata,
        TSize*               pxOutLcmCredentialsSize,
  const TUnsignedInt8**     ppxOutLcmCredentials
);

/**
 *  @ingroup groupMetadata
 *  @brief
 *    This function allows the DVR/VOD Library client to replace the generic
 *    metadata stored inside an LCM credential. The metadata can be read back
 *    by using \c dvlCredentialsInformationRequest(). The update credential returned
 *    by this function has to be released by calling \c dvlReleaseBuffer().
 *
 *  @note
 *    For DVL Dual, generic metadata field of LCM contains the read-only Nagra Usage Rules (NUR).\n
 *    If called by the application, the DVL Dual Library returns DVL_ERROR_BAD_PARAMETER.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xInLcmCredentialsSize
 *            Size in Bytes of the buffer containing the LCM Credentials to
 *            modify.
 *  @param[in] pxInLcmCredentials
 *            Pointer to the buffer containing the LCM Credentials to modified.
 *  @param[in] xGenericMetadataSize
 *            Size of the generic metadata. The maximum size supported is
 *            63 bytes.
 *  @param[in] pxGenericMetadata
 *            Pointer on the generic metadata.
 *  @param[out] pxOutLcmCredentialsSize
 *            Returned size in Bytes of the buffer containing the updated
 *            LCM Credentials.
 *  @param[out] ppxOutLcmCredentials
 *            Returned pointer to the buffer containing the updated
 *            LCM Credentials.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The input argument of the function is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlUpdateGenericLcmMetadata
(
        TSize                 xInLcmCredentialsSize,
  const TUnsignedInt8*       pxInLcmCredentials,
        TSize                 xGenericMetadataSize,
  const TUnsignedInt8*       pxGenericMetadata,
        TSize*               pxOutLcmCredentialsSize,
  const TUnsignedInt8**     ppxOutLcmCredentials
);

#ifdef __cplusplus
}
#endif

#endif /* NV_PVR_H */

/* END OF FILE */
