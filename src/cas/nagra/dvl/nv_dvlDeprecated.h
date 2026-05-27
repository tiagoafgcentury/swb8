/**
** @file nv_dvlDeprecated.h
**
** @brief
**   DVR/VOD Library containing deprecated APIs.
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


#ifndef NV_DVL_DEPRECATED_H
#define NV_DVL_DEPRECATED_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*																			  */
/*								 INCLUDE FILES								  */
/*																			  */
/******************************************************************************/

#include "nv_common.h"

/******************************************************************************/
/*																			  */
/*									   TYPES								  */
/*																			  */
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
 * @deprecated
 *   Use dvlDebugControlRole() instead.
 * @brief
 *   Defines the debug traces outputted by the module xModule.
 *   Debug of any role is outputted. Use the function
 *   dvlDebugControlRole() to control also the debug role.
 * @param   xModule
 *   Concerned module, or MOD_ALL to control all modules
 * @param   xVerbosity
 *   Verbosity that this module must use for debug
 *   (from VERBOSITY_LOW to VERBOSITY_MAX)
 * @param  xClass
 *   Class(es) that this module must output, or CLASS_ALL to
 *   output any class
 * @remarks
 * -# Replaces the previous control performed for the concerned module(s).
 *   The previous control may have been hard-coded through dbginit file,
 *   or done through a previous call to a dvlDebugControlXxx() function.
*/
NAGRA_CA_API void dvlDebugControlX
(
  TDvlDebugModule  xModule,
  TDvlDebugVerbosity  xVerbosity,
  TDvlDebugClass32    xClass
);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @deprecated
 *    This function is obsolete, call dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the buffer allocated by the DVR/VOD
 *    Library and containing the VOD playback output EMM and metadata.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxVodPlaybackOutputEmm
 *            Pointer to the buffer containing the VOD playback output EMM
 *            to release.
 *  @param  pxGenericMetadata
 *            Pointer to the buffer to release, containing the generic metadata.
 *  @param  pxSpecificMetadata
 *            Pointer to the buffer to release, containing the specific metadata.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @see ::dvlStartVodPlayback()
 *  @see ::dvlReleaseBuffer()
 */
NAGRA_CA_API TDvlStatus dvlReleaseVodPlaybackOutputEmm
(
  const TUnsignedInt8* pxVodPlaybackOutputEmm,
  const TUnsignedInt8* pxGenericMetadata,
  const TUnsignedInt8* pxSpecificMetadata

);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the buffer allocated by the DVR/VOD
 *    Library and containing the recording output Credentials.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxRecordOutputCredentials
 *            Pointer to the buffer containing the record output Credentials
 *            to release.
 *  @param  pxProfileMetadata
 *            Pointer to the buffer to release, containing the metadata related
 *            to the Profile.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @see ::dvlStartRecord()
*/
NAGRA_CA_API TDvlStatus dvlReleaseRecordOutputCredentials
(
  const TUnsignedInt8* pxRecordOutputCredentials,
  const TUnsignedInt8* pxProfileMetadata
);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the buffer allocated by the DVR/VOD
 *    Library and containing the playback output Credentials.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxPlaybackOutputCredentials
 *            Pointer to the buffer containing the playback output Credentials
 *            to release.
 *  @param  pxGenericMetadata
 *            Pointer to the buffer to release, containing the generic metadata
 *  @param  pxSpecificMetadata
 *            Pointer to the buffer to release, containing the specific metadata
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @see ::dvlStartPlayback()
*/
NAGRA_CA_API TDvlStatus dvlReleasePlaybackOutputCredentials
(
  const TUnsignedInt8* pxPlaybackOutputCredentials,
  const TUnsignedInt8* pxGenericMetadata,
  const TUnsignedInt8* pxSpecificMetadata
);

/**
 *  @deprecated
 *    This function is deprecated!
 *  @ingroup groupRecord
 *  @brief
 *    This function sends a record session request to the DVR/VOD Library.\n
 *    When this function sets \c DVL_RECORD_GRANTED into \c *pxRecordingStatus,
 *    the corresponding content key has been properly set into the chipset
 *    stream cipher and the STB application can start storing the content on
 *    the HDD.\n
 *    This function also allows appending the current record to a previous
 *    recorded content, by passing as argument the Credentials related to
 *    the previous recorded content to which the current recording shall be
 *    appended.\n
 *    If the append is authorized , the current content will be recorded
 *    with same content key as the event to which it is appended. The output
 *    Credentials then apply to both contents, the previously recorded one and
 *    the currently recorded one; the STB application may then consider the
 *    two records as only one.\n
 *    If the append fails (record status \c DVL_RECORD_DENIED_APPENDING_DENIED
 *    is returned in \c *pxRecordingStatus), no content key is set into the
 *    stream cipher of the chipset. In that case, the STB application may
 *    still try a standalone recording (without passing any input Credentials).\n
 *    If the function returned with the record status DVL_RECORD_GRANTED, the
 *    STB application shall copy/store the content of the output Credentials
 *    buffer on the HDD and then release it by calling the function
 *    \c dvlReleaseBuffer().\n
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
 *  @param[in] xTSid
 *            Identifier of transport stream to be recorded.
 *  @param[in] xProfileSize
 *            Size in Bytes of the input profile. Shall be set to 0 if the
 *            default hard coded operator profile has to be used.
 *  @param[in] pxProfile
 *            Mandatory Profile containing the recording and usage rules of the
 *            content. Shall be set to \c NULL if the default hard coded
 *            operator profile has to be used.
 *  @param[in] xSpecificMetadataSize
 *            Size of the optional specific metadata to be stored in the output
 *            Credentials. Shall be set to zero in case no specific related
 *            metadata are passed to the DVR/VOD Library.
 *  @param[in] pxSpecificMetadata
 *            Optional specific metadata to be stored in the output Credentials.
 *            Shall be set to NULL in case no specific metadata are passed to
 *            the DVR/VOD Library.
 *  @param[in] xCollectionId
 *            Unique identifier of the collection to which the content belongs.
 *            The collection identifier is used for recorded content appending.
 *  @param[in] xInputCredentialsSize
 *            Size of the optional input Credentials.
 *            Shall be set to zero in case no input Credentials is passed to
 *            the DVR/VOD Library.
 *  @param[in] pxInputCredentials
 *            Optional Credentials of the content to which the current recording
 *            should be appended.
 *            Should be set to \c NULL if the current recording is not intended
 *            to be appended to any other content.
 *  @param[in] xRecordErrorCB
 *            Callback used by the DVR/VOD Library to report a recording error.
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
 *  @param[out] pxGenericMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            generic metadata. The input pointer shall not be \c NULL if
 *            \c ppxGenericMetadata is not \c NULL.
 *  @param[out] ppxGenericMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional generic metadata. This buffer can
 *            be \c NULL if the caller is not interested in retrieving generic
 *            metadata.
 *            Once the function returns, \c *ppxGenericMetadata may be NULL if
 *            the Credentials contain no generic metadata or if the input
 *            Credentials are not valid (function returns
 *            \c DVL_ERROR_BAD_PARAMETER).
 *            The buffer pointed by *ppxGenericMetadata shall be released once no
 *            longer used by the STB application, by calling the dedicated
 *            function \c dvlReleaseBuffer().
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
 *    -# The appending result depends on various parameters, according to the
 *      operator's wishes. The Profile shall basically have an explicit append
 *      authorization and it may be subject to a certain time span after the
 *      first recording.
*/
NAGRA_CA_API TDvlStatus dvlStartRecord
(
        TTransportSessionId   xTSid,
        TSize                 xProfileSize,
  const TUnsignedInt8*       pxProfile,
        TSize                 xSpecificMetadataSize,
  const TUnsignedInt8*       pxSpecificMetadata,
        TUnsignedInt32        xCollectionId,
        TSize                 xInputCredentialsSize,
  const TUnsignedInt8*       pxInputCredentials,
        TDvlRecordErrorCB     xRecordErrorCB,
        TSize*               pxOutputCredentialsSize,
  const TUnsignedInt8**     ppxOutputCredentials,
        TDvlRecordStatus*    pxRecordingStatus,
        TSize*               pxGenericMetadataSize,
  const TUnsignedInt8**     ppxGenericMetadata,
        TDvlHandle**        ppxRecordHandle
);

/**
 * @deprecated
 *   This structure is used with deprecated functions dvlStartPlayback and dvlStartVodPlayback
 * @ingroup groupPlayback
 * @brief
 *    Playback error callback.
 *
 *    This callback is called by the DVR/VOD Library if an error occurs during
 *    a playback session. In that case, the DVR/VOD Library sets the
 *    corresponding error status in the parameter \c xPlaybackStatus.
 *
 *    If the callback is called by the DVR/VOD Library, the STB Application
 *    shall consider the playback has no more allowed. It shall initiate a
 *    message that will then stop the concerned playback and return from the
 *    callback.
 *    Once the STB application runs in its context again, it shall call
 *    \c dvlStopPlayback() with the corresponding playback handle.
 *
 *    This callback can only be called by the DVR/VOD Library as long as the
 *    playback handle is valid (i.e. as long as the STB Application has not
 *    called \c dvlStopPlayback() with that playback handle).
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param[out] pxPlaybackHandle
 *            Pointer to the handle of the concerned playback session.
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
typedef void (*TDvlPlaybackErrorCB)
(
  TDvlHandle*      pxPlaybackHandle,
  TDvlAccessStatus  xPlaybackStatus
);

/**
 *  @ingroup groupPlayback
 *  @deprecated
 *     This function is deprecated!
 *  @brief
 *    This function sends a playback request to the DVR/VOD library, given one
 *    input Credentials.\n
 *    When this function returns with the status \c DVL_ACCESS_GRANTED set in
 *    \c *pxPlaybackStatus, that means that the corresponding content key has
 *    been correctly set in the chipset stream cipher and the STB application
 *    can start playing back the content.\n
 *    If the function returns with the playback status \c DVL_ACCESS_GRANTED,
 *    the STB application should copy/store the content of the output
 *    Credentials buffer and then release it by calling the function
 *    \c dvlReleaseBuffer(). That output Credentials should replace any one
 *    already existing for that content. Nevertheless, if the application
 *    does not store credentials, it can ignore this output buffer.\n
 *    Once the function returned with the playback status \c DVL_ACCESS_GRANTED,
 *    the DVR/VOD Library may call the passed playback error callback at any
 *    time. The main reason is that the playback may be subject to a time limit
 *    and the playback is suddenly denied. In that case, the STB application
 *    shall stop the current playback session.\n
 *    When the function returns with error status \c DVL_NO_ERROR, it does not
 *    necessarily mean the playback is successful. The STB Application still
 *    needs to check the value set in the playback status \c *pxPlaybackStatus
 *    parameter.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xTSid
 *            Identifier of transport stream to be played back.
 *  @param[in] xInputCredentialsSize
 *            Size in Bytes of the mandatory input Credentials.
 *  @param[in] pxInputCredentials
 *            Pointer to the input Credentials related to the content
 *            to play back.
 *            The DVR/VOD Library won't modify that input buffer. But the STB
 *            application shall replace on the content storage the input
 *            Credentials by the output Credentials.
 *  @param[in] xPlaybackErrorCB
 *            Callback used by the DVR/VOD Library to report any playback error.
 *  @param[out] pxOutputCredentialsSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output Credentials.
 *  @param[out] ppxOutputCredentials
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the output Credentials.
 *            The input pointer shall not be \c NULL. \n
 *            The STB application shall replace the input Credentials on the
 *            content storage by this output Credentials and then release the
 *            buffer by calling \c dvlReleaseBuffer().
 *  @param[out] pxPlaybackStatus
 *            Pointer where the DVR/VOD Library will store the playback status.
 *            If \c pxPlaybackStatus is greater or equal to
 *            \c DVL_ACCESS_DENIED, the content key has not been set into the
 *            chipset stream cipher and the content cannot be played back.
 *  @param[out] pxGenericMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional generic metadata. The input pointer shall not be \c NULL
 *            if \c ppxGenericMetadata is not NULL.
 *  @param[out] ppxGenericMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional generic metadata. This buffer can
 *            be NULL if the caller is not interested in retrieving generic
 *            metadata.
 *            Once the function returns, \c *ppxGenericMetadata may be NULL if
 *            the input Credentials contain no generic metadata or if they are
 *            not valid (function returns \c DVL_ERROR_BAD_PARAMETER).
 *            If returned, the buffer pointed by \c *ppxGenericMetadata shall
 *            be released once no longer used by the STB application, by
 *            calling \c ::dvlReleaseBuffer().
 *  @param[out] pxSpecificMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional specific metadata. The input pointer shall not be \c NULL
 *            if \c ppxSpecificMetadata is not \c NULL.
 *  @param[out] ppxSpecificMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional specific metadata. This buffer can
 *            be NULL if the caller is not interested in retrieving specific
 *            metadata.
 *            Once the function returns, \c *ppxSpecificMetadata may be \c NULL
 *            if the input Credentials contain no specific metadata or if they
 *            are not valid (function returns \c DVL_ERROR_BAD_PARAMETER).
 *            The buffer pointed by \c *ppxSpecificMetadata shall be released
 *            once no longer used by the STB application, by calling
 *            \c ::dvlReleaseBuffer().
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
 *  @remark
 *    -# If the function returns \c DVL_NO_ERROR and \c *pxPlaybackStatus
 *    contains \c DVL_ACCESS_GRANTED, the STB application shall copy/store
 *    the content of the output Credentials buffer on the HDD and then release
 *    it by calling the function \c dvlReleaseBuffer().
*/
NAGRA_CA_API TDvlStatus dvlStartPlayback
(
        TTransportSessionId   xTSid,
        TSize                 xInputCredentialsSize,
  const TUnsignedInt8*       pxInputCredentials,
        TDvlPlaybackErrorCB   xPlaybackErrorCB,
        TSize*               pxOutputCredentialsSize,
  const TUnsignedInt8**     ppxOutputCredentials,
        TDvlAccessStatus*    pxPlaybackStatus,
        TSize*               pxGenericMetadataSize,
  const TUnsignedInt8**     ppxGenericMetadata,
        TSize*               pxSpecificMetadataSize,
  const TUnsignedInt8**     ppxSpecificMetadata,
        TDvlHandle**        ppxPlaybackHandle
);

/**
 *  @ingroup groupPlayback
 *  @deprecated
 *     This function is deprecated!
 *  @brief
 *    This function is responsible for stopping the playback session, which
 *    consists in the cleanup of the session's resources and
 *    the removal of the content key from the chipset stream cipher.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *    A valid playback session must be running, handled by the parameter
 *    pxPlaybackHandle.
 *
 *  @post
 *    None
 *
 *  @param[in] pxPlaybackHandle
 *            Handle of the playback session to stop.
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
NAGRA_CA_API TDvlStatus dvlStopPlayback
(
  TDvlHandle*  pxPlaybackHandle
);

/**
 *  @ingroup groupPlayback
 *  @deprecated
 *     This function is deprecated!
 *  @brief
 *    This function sends a VOD playback request to the DVR/VOD library, given
 *    two input Credentials, an EMM and an ECM.<br>
 *    When this function returns with the status \c DVL_ACCESS_GRANTED set in
 *    \c *pxPlaybackStatus, that means that the corresponding content key has
 *    been correctly set in the chipset stream cipher and the STB application
 *    can start playing back the content.<br>
 *    If the function returns with the playback status \c DVL_ACCESS_GRANTED,
 *    the STB application shall copy/store the content of the output
 *    Credentials buffer containing the EMM on the HDD and then release it by
 *    calling the function \c dvlReleaseBuffer(). That output EMM shall replace
 *    any one already existing for that content.<br>
 *    Once the function returned with the playback status \c DVL_ACCESS_GRANTED,
 *    the DVR/VOD Library may call the passed playback error callback at any
 *    time. The main reason is that the playback may be subject to a time limit
 *    and the playback is suddenly denied. In that case, the STB application
 *    shall stop the current VOD playback session.<br>
 *    When the function returns with error status \c DVL_NO_ERROR, it does not
 *    necessarily mean the playback is successful. The STB Application still
 *    needs to check the value set in the playback status \c *pxPlaybackStatus
 *    parameter.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param[in] xTSid
 *            Identifier of transport stream to be played back.
 *  @param[in] xInputEmmSize
 *            Size in Bytes of the mandatory input EMM.
 *  @param[in] pxInputEmm
 *            Pointer to the input EMM related to the VOD content to play back.
 *            The DVR/VOD Library won't modify that input buffer. But the STB
 *            application shall replace on the content storage the input
 *            EMM by the output EMM contained in \c *ppxOutputEmm.
 *  @param[in] xEcmSize
 *            Size in Bytes of the mandatory ECM.
 *  @param[in] pxEcm
 *            Pointer to the ECM related to the VOD content to play back.
 *            The DVR/VOD Library won't modify that input buffer.
 *  @param[in] xPlaybackErrorCB
 *            Callback used by the DVR/VOD Library to report any playback error.
 *  @param[out] pxOutputEmmSize
 *            Pointer where the DVR/VOD Library will store the Size in Bytes of
 *            the buffer containing the output EMM.
 *  @param[out] ppxOutputEmm
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            memory that will contain the output EMM.
 *            The input pointer shall not be \c NULL.
 *            The STB application shall replace the input EMM on the
 *            content storage by this output EMM and then release the
 *            buffer by calling \c dvlReleaseBuffer().
 *  @param[out] pxVodPlaybackStatus
 *            Pointer where the DVR/VOD Library will store the playback status.
 *            If \c pxPlaybackStatus is lower than \c DVL_ACCESS_DENIED, the
 *            content key has been correctly set into the chipset stream cipher
 *            and the stream can be played back.
 *            If \c pxPlaybackStatus is greater or equal to \c DVL_ACCESS_DENIED,
 *            the content key has not been set into the chipset stream cipher
 *            and the content cannot be played back.
 *  @param[out] pxGenericMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional generic metadata. The input pointer shall not be \c NULL
 *            if \c ppxGenericMetadata is not \c NULL.
 *  @param[out] ppxGenericMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional generic metadata. This buffer can
 *            be \c NULL if the caller is not interested in retrieving generic
 *            metadata.
 *            Once the function returns, \c *ppxGenericMetadata may be \c NULL
 *            if the input Credentials (ECM or EMM) contain no generic metadata
 *            or if they are not valid (function returns
 *            \c DVL_ERROR_BAD_PARAMETER).<br>
 *            The buffer pointed by \c *ppxGenericMetadata shall be released
 *            once no longer used by the STB application, by calling
 *            \c dvlReleaseBuffer().
 *  @param[out] pxSpecificMetadataSize
 *            Pointer where the DVR/VOD Library will store the size of the
 *            optional specific metadata. The input pointer shall not be
 *            \c NULL if \c ppxSpecificMetadata is not \c NULL.
 *  @param[out] ppxSpecificMetadata
 *            Pointer where the DVR/VOD Library will store the pointer to the
 *            buffer containing the optional specific metadata. This buffer can
 *            be \c NULL if the caller is not interested in retrieving specific
 *            metadata.<br>
 *            Once the function returns, \c *ppxSpecificMetadata may be \c NULL
 *            if the input Credentials (ECM or EMM) contain no specific
 *            metadata or if the input Credentials are not valid (function
 *            returns \c DVL_ERROR_BAD_PARAMETER).
 *            The buffer pointed by \c *ppxSpecificMetadata shall be released
 *            once no more used by the STB application, by calling the
 *            dedicated function \c dvlReleaseBuffer().
 *  @param[out] ppxVodPlaybackHandle
 *            Pointer where the DVR/VOD Library will store the VOD playback
 *            session handler.
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
 *    -# If the function returns \c DVL_NO_ERROR and \c *pxPlaybackStatus
 *    contains \c DVL_ACCESS_GRANTED, the STB application shall copy/store the
 *    content of the output Credentials buffer on the HDD and then release it
 *    by calling the function \c dvlReleaseBuffer().
*/
NAGRA_CA_API TDvlStatus dvlStartVodPlayback
(
        TTransportSessionId   xTSid,
        TSize                 xInputEmmSize,
  const TUnsignedInt8*       pxInputEmm,
        TSize                 xEcmSize,
  const TUnsignedInt8*       pxEcm,
        TDvlPlaybackErrorCB   xPlaybackErrorCB,
        TSize*               pxOutputEmmSize,
  const TUnsignedInt8**     ppxOutputEmm,
        TDvlAccessStatus*    pxVodPlaybackStatus,
        TSize*               pxGenericMetadataSize,
  const TUnsignedInt8**     ppxGenericMetadata,
        TSize*               pxSpecificMetadataSize,
  const TUnsignedInt8**     ppxSpecificMetadata,
        TDvlHandle**        ppxVodPlaybackHandle
);

/**
 *  @deprecated
 *      This function is deprecated.
 *  @ingroup groupPlayback
 *  @brief
 *    This function is responsible for stopping the VOD playback session, which
 *    consists in the cleanup of the VOD playback session related resources and
 *    the removal of the VOD content key from the chipset stream cipher.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *    A valid VOD playback session must be running, handled by the parameter
 *    pxVodPlaybackHandle.
 *
 *  @post
 *    None
 *
 *  @param[in] pxVodPlaybackHandle
 *            Handle of the playback session to stop.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            The VOD handle is invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
*/
NAGRA_CA_API TDvlStatus dvlStopVodPlayback
(
  TDvlHandle*  pxVodPlaybackHandle
);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the array allocated by the DVR/VOD
 *    Library containing information about neighbourhood Set-Top Boxes secrets.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxNeighbourhoodInfoItems
 *            Pointer to the buffer containing the array to release.
 *
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR
 *            The function execution failed.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *
 *  @see ::dvlHomeNetworkInformationRequest()
*/
NAGRA_CA_API TDvlStatus dvlReleaseNeighbourhoodInfoItems
(
  const TDvlNeighbourhoodInfo*  pxNeighbourhoodInfoItems
);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the buffer allocated by the DVR/VOD
 *    Library and containing the Credentials metadata related to the Profile and
 *    to the content.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxCredentialsUnion
 *            Pointer to the Credentials union structure to release.
 *  @param  pxCredentialsGenericMetadata
 *            Pointer to the buffer to release, containing the Credentials
 *            generic metadata.
 *  @param  pxCredentialsSpecificMetadata
 *            Pointer to the buffer to release, containing the Credentials
 *            specific metadata.
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
NAGRA_CA_API TDvlStatus dvlReleaseCredentialInformation
(
  const TCredentialsUnion* pxCredentialsUnion,
  const TUnsignedInt8*     pxCredentialsGenericMetadata,
  const TUnsignedInt8*     pxCredentialsSpecificMetadata
);

/**
 * @deprecated
 *   Use dvlReleaseBuffer() instead.
 *  @brief
 *    A call to this function releases the buffer allocated by the DVR/VOD
 *    Library and containing the authenticated data.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @post
 *    None
 *
 *  @param  pxAuthenticatedData
 *            Pointer to the buffer to release, containing the authenticated
 *            data.
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
NAGRA_CA_API TDvlStatus dvlReleaseAuthenticatedData
(
  const TUnsignedInt8*     pxAuthenticatedData
);

#ifdef __cplusplus
}
#endif

#endif /* NV_DVL_DEPRECATED_H */

/* END OF FILE */
