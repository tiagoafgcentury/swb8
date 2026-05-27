/**
**  @file ca_dsc.h
**
**  @brief
**   This file defines the generic descrambler API used by the different
**   NagraVision modules and software to access the hardware descrambling
**   capabilities of the set-top box.
**
**  @ingroup DSC
**
**  $Revision: 1.34 $
**
** COPYRIGHT:
**   1998-2002, 2006, 2007 NagraVision S.A.
**
*/

/*
** REMARK:
**    Comments in this file use special tags to allow automatic API
**    documentation generation in HTML format, using the
**    GNU-General Public Licensed Doxygen tool.
**    For more information about Doxygen, please check www.doxygen.org
**
** REFERENCES:
**    [EMI] Nagravision, System Architecture, Encryption Method Indicator,
**          Specification
*/

/**
 *  @defgroup DSC Descrambler
 *  @brief
 *    Descrambler component
 *
 *    This component is the interface to the descrambler driver.
*/


#ifndef CA_DSC_H
#define CA_DSC_H

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#ifdef DSC_WRAP
#include "dscwrap.h"
#endif

#include "ca_defs.h"
#include "ca_defsx.h"
#include "ca_mpeg.h"


/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    Identifier of DSC channel.
 *
 *  @see DSC_CHANNEL_ID_INVALID
*/
typedef struct SDscChannelId* TDscChannelId;

/**
 *  @brief
 *    Invalid value for TDscChannelId.
 *
 *  @see TDscChannelId
*/
#define DSC_CHANNEL_ID_INVALID ((TDscChannelId) NULL)


/**
 *  @brief
 *    Return values of the DSC methods.
 *
*/
typedef enum
{
  DSC_NO_ERROR,
  /**<  Method call was successful.
   */
  DSC_ERROR_NO_MORE_RESOURCES,
  /**<  No more descrambler resources are available.
   */
  DSC_ERROR_UNKNOWN_ID,
  /**<  The descrambler resource is not open.
   */
  DSC_ERROR_BAD_PID,
  /**<  The given PID is out of range.
   */
  DSC_ERROR_BAD_KEY_LENGTH,
  /**<  The key length is not supported by the hardware.
   */
  DSC_ERROR_NOT_IMPLEMENTED,
  /**<  The function is not implemented.
   */
  DSC_ERROR,
  /**<  Other error.
   */
  DSC_ERROR_WRONG_EMI,
  /**<  The given EMI is not supported by the DSC
   */
  LAST_DSC_STATUS
  /**< Number of types. Internal use only.
   */
} TDscStatus;


/**
 *  @brief
 *    Descrambling key set in the descrambler channel.
*/
typedef TUnsignedInt8 TDscKey;


/******************************************************************************/
/*                                                                            */
/*                             FUNCTION PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    This function opens a descrambler channel.
 *
 *  @param[out] pxDscChannelId
 *                Identificator of the opened channel. This identificator is a
 *                parameter to every function accessing the channel.
 *
 *  @param[in] xTransportSessionId
 *               Transport session identifier selecting which transport should
 *               be descrambled.
 *
 *  @param[in] xMpegStreamType
 *               MPEG stream type as defined in table 2-35 of ISO/IEC 13818-1.
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_NO_MORE_RESOURCES
 *             No more resources are available.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @remarks
 *   -# This function shall not return any error about the MPEG stream type.
 *      This type is not handled by the CA software. MPEG stream types defined
 *      in ca_mpeg.h will no longer evolve and are maintained for backward
 *      compatibility reasons only.
*/
TDscStatus dscOpen
(
  TDscChannelId*        pxDscChannelId,
  TTransportSessionId    xTransportSessionId,
  TMpegStreamType        xMpegStreamType
);


/**
 *  @brief
 *    This function closes a descrambler channel.
 *
 *  @param[in] xDscChannelId
 *               Channel to close.
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The descrambler ID is not currently opened.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
*/
TDscStatus dscClose
(
  TDscChannelId   xDscChannelId
);


/**
 *  @brief
 *    This function sets the PID to descramble on a particular channel.
 *
 *  @param[in] xDscChannelId
 *               Channel holding the PID to descramble.
 *
 *  @param[in] xPid
 *               PID to descramble.
 *
 *  @retval DSC_NO_ERROR
 *            Success.
 *
 *  @retval DSC_ERROR_UNKNOWN_ID
 *            The descrambler ID is not currently opened.
 *
 *  @retval DSC_ERROR_BAD_PID
 *            This PID value is not allowed.
 *
 *  @retval DSC_ERROR
 *            Other error.
 *
*/
TDscStatus dscSetPid
(
  TDscChannelId   xDscChannelId,
  TPid            xPid
);


/**
 *  @brief
 *    Set the keys on the specified channel, in non-protected mode, i.e. CWs in
 *    clear-text form.
 *
 *  @param[in] xDscChannelId
 *               Channel for which the keys must be updated.
 *
 *  @param[in] xOddKeyLength
 *               Length in bytes of the odd key
 *
 *  @param[in] pxOddKey
 *               Buffer containing the odd key
 *
 *  @param[in] xEvenKeyLength
 *               Length in bytes of the even key
 *
 *  @param[in] pxEvenKey
 *               Buffer containing the even key
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The descrambler ID is not currently opened.
 *
 *  @retval  DSC_ERROR_BAD_KEY_LENGTH
 *             The key length is not supported by the hardware.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @remarks
 *   -# If one of the key is not available, the given pointer is set to \c NULL
 *      and the corresponding length to 0. The driver should not change the key
 *      in this case.
*/
TDscStatus dscSetKeys
(
        TDscChannelId   xDscChannelId,
        TSize           xOddKeyLength,
  const TDscKey*        pxOddKey,
        TSize           xEvenKeyLength,
  const TDscKey*        pxEvenKey
);


/**
 *  @brief
 *    Set protected keys on the specified channel. Keys are directly ciphered
 *    with the descrambler chipset key (i.e. key ladder level 0)
 *
 *  @param[in] xDscChannelId
 *               Channel for which the keys must be updated.
 *
 *  @param[in] xOddProtectedKeyLength
 *               Length in bytes of the ciphered odd key
 *
 *  @param[in] pxOddProtectedKey
 *               Buffer containing the ciphered odd key
 *
 *  @param[in] xEvenProtectedKeyLength
 *               Length in bytes of the ciphered even key
 *
 *  @param[in] pxEvenProtectedKey
 *               Buffer containing the even key
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The descrambler ID is not currently opened.
 *
 *  @retval  DSC_ERROR_BAD_KEY_LENGTH
 *             The key length is not supported by the hardware.
 *
 *  @retval  DSC_ERROR_NOT_IMPLEMENTED
 *             The function is not implemented.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @remarks
 *   -# If one of the protected keys is not available, the given pointer is set
 *      to \c NULL and the corresponding length to 0. The driver should not
 *      change the key in this case
 *   .
 *   -# This function is mutually exclusive with \c dscSet1LevelProtectedKeys
 *      and its implementation depends on the key ladder level.
*/
TDscStatus dscSet0LevelProtectedKeys
(
        TDscChannelId    xDscChannelId,
        TSize            xOddProtectedKeyLength,
  const TUnsignedInt8*  pxOddProtectedKey,
        TSize            xEvenProtectedKeyLength,
  const TUnsignedInt8*  pxEvenProtectedKey
);


/**
 *  @brief
 *    Set protected keys on the specified channel. Keys are ciphered with the
 *    intermediate key \c pxL1ProtectingKey which is itself ciphered with the
 *    descrambler chipset key (i.e. key ladder level 1)
 *
 *    The key \c pxL1ProtectingKey complies with the format of the 128-bit input
 *    key defined "TDES Key Format" section.
 *
 *  @param[in] xDscChannelId
 *               Channel for which the keys must be updated.
 *
 *  @param[in] xL1ProtectingKeyLength
 *               Length in bytes of the level 1 ciphered key
 *
 *  @param[in] pxL1ProtectingKey
 *               Level 1 ciphered key protecting the odd and even keys
 *
 *  @param[in] xOddProtectedKeyLength
 *               Length in bytes of the ciphered odd key
 *
 *  @param[in] pxOddProtectedKey
 *               Buffer containing the odd key
 *
 *  @param[in] xEvenProtectedKeyLength
 *               Length in bytes of the ciphered even key
 *
 *  @param[in] pxEvenProtectedKey
 *               Buffer containing the even key
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The descrambler ID is not currently opened.
 *
 *  @retval  DSC_ERROR_BAD_KEY_LENGTH
 *             The key length is not supported by the hardware.
 *
 *  @retval  DSC_ERROR_NOT_IMPLEMENTED
 *             The function is not implemented.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @remarks
 *   -# If one of the protected keys is not available, the given pointer is set
 *      to \c NULL and the corresponding length to 0. The driver should not
 *      change the key in this case
 *   .
 *   -# This function is mutually exclusive with \c dscSet0LevelProtectedKeys
 *      and its implementation depends on the key ladder level.
*/
TDscStatus dscSet1LevelProtectedKeys
(
        TDscChannelId    xDscChannelId,
        TSize            xL1ProtectingKeyLength,
  const TUnsignedInt8*  pxL1ProtectingKey,
        TSize            xOddProtectedKeyLength,
  const TUnsignedInt8*  pxOddProtectedKey,
        TSize            xEvenProtectedKeyLength,
  const TUnsignedInt8*  pxEvenProtectedKey
);


/**
 *  @brief
 *    This function provides the descrambler channel with the metadata
 *    associated to the program. The program metadata is conveyed into the ECM
 *    stream from the head-end to set-top box. It consists in a collection of
 *    TLV descriptors complying with the format defined "CAK, Metadata Format,
 *    Specification". The copy protection information is a typical usage of
 *    program metadata.
 *
 *  @param[in] xDscChannelId
 *               Descrambler channel identifier
 *
 *  @param[in] xMetadataLength
 *             	 Length in bytes of the metadata buffer.
 *
 *  @param[in] pxMetadata
 *               Buffer containing the metadata descriptors
 *
 *  @retval  DSC_NO_ERROR
 *             Success.
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The descrambler ID is not currently opened.
 *
 *  @retval  DSC_ERROR_NOT_IMPLEMENTED
 *             The function is not implemented.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @remarks
 *   -# This function is called only if access to the content is granted. In
 *      such a case, it is called just before setting the keys into the
 *      descrambler (\c dscSetKeys, \c dscSet1LevelProtectedKeys or
 *      dscSet0LevelProtectedKeys), even if there is no metadata. In this latter
 *      case, \c pxMetadata is set to \c NULL and \c xMetadataLength is set to 0.
*/
TDscStatus dscSetMetadata
(
        TDscChannelId       xDscChannelId,
        TSize               xMetadataLength,
  const TUnsignedInt8*     pxMetadata
);


/**
 *  @brief
 *    This function set the EMI identifying the decryption method to be used to
 *    descramble the elementary stream.
 *
 *    Refer to the Nagravision Encryption Method Indicator Specification [EMI]
 *    for the definition of all EMI.
 *
 *  @param[in] xDscChannelId
 *    Pointer to the stream decryption session handler identifying the session

 *  @param[in] xDecryptionEmi
 *    Encryption Method to be used to descramble the elementary stream
 *
 *  @retval  DSC_NO_ERROR
 *             The decryption EMI was successfully set
 *
 *  @retval  DSC_ERROR_UNKNOWN_ID
 *             The given descrambler channel ID is not currently opened.
 *
 *  @retval  DSC_ERROR_NOT_IMPLEMENTED
 *             The function is not implemented.
 *
 *  @retval  DSC_ERROR
 *             Other error.
 *
 *  @retval  DSC_ERROR_WRONG_EMI
 *             The given decryption EMI is not supported by the DSC
 *
 *  @remarks
 *   -# This function is called just before \c dscSetMetadata or just before
 *      setting the keys into the descrambler (\c dscSetKeys,
 *      \c dscSet1LevelProtectedKeys or \c dscSet0LevelProtectedKeys) if
 *      \c dscSetMetadata is not called.
 *   -# The EMI set through this function relies on PMT descriptors (DVB
 *      scrambling_descriptor or custom descriptor). If the PMT does not convey
 *      any scrambling algorithm signalization, \c dscSetDecryptionEmi() is not
 *      called and it is assumed the program is scrambled with the default
 *      scrambling algorithm used on this network.
 *
*/
TDscStatus dscSetDecryptionEmi
(
  TDscChannelId    xDscChannelId,
  TUnsignedInt16   xDecryptionEmi
);

#endif /* CA_DSC_H */

/* ca_dsc.h */
