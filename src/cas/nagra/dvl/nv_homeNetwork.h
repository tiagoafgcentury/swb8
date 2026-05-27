/**
** @file nv_homeNetwork.h
**
** @brief
**   DVR/VOD Library Home network functions.
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


#ifndef NV_HOME_NETWORK_H
#define NV_HOME_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                 INCLUDE FILES                                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                     TYPES                                                  */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup groupHomeNetwork
 *  @brief
 *    This enumeration gives information about the type of secret.
*/
typedef enum
{
  DVL_SECRET_TYPE_SEC = 0,
    /**<
     *    The secrets are chipset based.
   */
  DVL_SECRET_TYPE_OTP,
    /**<
     *    The secrets are OTP based.
   */
  DVL_SECRET_TYPE_CERT,
    /**<
     *    The secrets are NOCS3 chipset based.
   */
  LAST_DVL_SECRET_TYPE
    /**<
     *    Last DVL key type.
     *    This key type is never returned by the DVR/VOD Library.
   */
} TDvlSecretsType;

/**
 *  @ingroup groupHomeNetwork
 *  @brief
 *    This structure contains information about the secrets that
 *    the DVR/VOD Library possesses and that allows it to
 *    playback content recorded by a neighbourhood Set-Top Box.
*/
typedef struct
{
  TUnsignedInt32  irdSerialNumber;
    /**<
     *    This field contains the Nagra unique Identifier of the Set-Top Box
     *    of which the DVR/VOD Library possesses some secrets.
   */
  TDvlSecretsType secretsType;
    /**<
     *    This field contains the type of the secrets.
   */
  TUnsignedInt32  expirationDate;
    /**<
     *    This field contains the expiryDate of the secret.<br>
     *    For LCI 0x0200 expiryDate of the secret.<br>
     *    For LCI 0x0100 expiryDate is default value 0xFFFFFFFF.
   */
} TDvlNeighbourhoodInfo;


/**
 *  @ingroup groupHomeNetwork
 *  @brief
 *    This function allows the STB application to request information about
 *    the Home Networking.
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *
 *  @param[out] pxHomeNetworkIdValue
 *            Nagra unique identifier of the Home Network to which the STB
 *            belongs.
 *            If the Set-Top Box does not belong to any Home Network,
 *            the DVR/VOD Library returns 0xFFFFFFFF as
 *            \c *pxHomeNetworkIdValue.
 *  @param[out] xHomeNetworkIdString
 *            NULL-terminated and printable string containing the Nagra unique
 *            Home Network Identifier.
 *            If the Set-Top Box doesn't belong to any Home Network,
 *            the string will contain "42 9496 7295 96".
 *  @param[out] pxHomeNetworkVersion
 *            Version of the Home Network to which the STB belongs.
 *            If the STB doesn't belong to any Home Network, the DVR/VOD
 *            Library returns 0xFF as \c pxHomeNetworkVersion.
 *  @param[out] pxNumberOfNeighbourhoodInfoItems
 *            Number of neighbourhood information items contained in
 *            \c ppxNeighbourhoodInfoItems.
 *  @param[out] ppxNeighbourhoodInfoItems
 *            Array containing information about neighbourhood Set-Top Boxes
 *            secrets.
 *            The DVR/VOD Library possesses some secrets allowing to playback
 *            the content recorded by any of the neighbourhood Set-Top Boxes
 *            listed in this array. This buffer shall be released by calling
 *            the function \c dvlReleaseBuffer().
 *            This array might be empty, as is does not contain the identifier
 *            of the local Set-Top Box on which runs this particular instance
 *            of DVR/VOD Library. This array may contain more than once the
 *            information item related to the same neightbourhood Set-Top Box,
 *            once per secret type (see \c TDvlSecretsType).
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
NAGRA_CA_API TDvlStatus dvlHomeNetworkInformationRequest
(
  TUnsignedInt32*                pxHomeNetworkIdValue,
  TDvlUniqueIdString             xHomeNetworkIdString,
  TUnsignedInt8*                 pxHomeNetworkVersion,
  TUnsignedInt8*                 pxNumberOfNeighbourhoodInfoItems,
  const TDvlNeighbourhoodInfo**  ppxNeighbourhoodInfoItems
);

#ifdef __cplusplus
}
#endif

#endif /* End of NV_HOME_NETWORK_H */
