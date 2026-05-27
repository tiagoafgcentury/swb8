/**
**  @file ca_sec.h
**
**  @brief
**   Secure Chipset management.
**
**   This file specifies the Secure Chipset API that allows Nagra CA
**   software to access security features of the MPEG chipset.
**
**  $Id: //CAK/components/secapi/TAGS/SECAPI_6_10_2/src/ca_sec.h#1 $
**
*/
/*
** REMARK:
**   Comments in this file use special tags to allow automatic API documentation
**   generation in HTML format, using the GNU-General Public Licensed Doxygen
**   tool. For more information about Doxygen, please check www.doxygen.org
**
**   Depending on the platform, the CHM file may not open properly if it is stored on a network drive.
**   So either the file should be moved on a local drive or add the following registry entry on
**   Windows platform (regedit):\n
**   [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003
*/


#ifndef CA_SEC_H
#define CA_SEC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the version number of the SEC API. This version has to be included in
 * the function table returned by secGetFunctionTable(). To do so, use the macro
 * #SECAPI_VERSION_INT to put it in the right format.
*/
#define SECAPI_VERSION_MAJOR  6
#define SECAPI_VERSION_MEDIUM 10
#define SECAPI_VERSION_MINOR  2

/** @page p_sec_history Changes history
  - <b> 6.10.2 - 15-Dec-2020 </b>
    - Updated opaque buffers diagram for 'cenc' full encryption (incremental mode)
      to fix and set the skipBlocks and cryptBlocks values to 0.
  .
  - <b> 6.10.1 - 18-Mar-2019 </b>
    - Reverted #TSecRsaPrivateEncrypt deprecation
    - Increased RSA minimum key size from 768 to 1024 bits
  .
  - <b> 6.9.0 - 04-Feb-2019 </b>
    - Added #TSecGetChipId
    - Deprecated #TSecRsaPrivateEncrypt
  .
  - <b> 6.8.2 - 27-Jul-2018 </b>
    - Fixed a misleading typo in the \ref p_sec_prot_buf section
  .
  - <b> 6.8.1 - 05-Mar-2018 </b>
    - Updated behavior of #TSecUnprotectKeystore to work in 2-step mode
    - Added clarification of key ID in PVR encryption mode
    - Updated the protected buffer policy section
  .
  - <b> 6.7.0 - 05-Feb-2018 </b>
    - Added user intent for transcode use case
  .
  - <b> 6.6.0 - 03-Oct-2017 </b>
    - Added generic MKL key ladder support for Conax
    - Added support of secure processor for Conax
  .
  - <b> 6.5.0 - 14-Jul-2017 </b>
    - Added an advanced opaque buffer structure to handle all Common Encrypytion
      scheme more efficiently
    - Removed the support of processOpaqueData in encryption mode. This function
      is now required in decryption mode only.
    - Added #TSecUnprotectKeystore
  .
  - <b> 6.4.0 - 14-Dec-2016 </b>
    - Added support of ETSI key ladder in legacy stream processing session
    - Updated RSA and DH key sizes to multiple of 32 bits
    - Specified that processContentUsageRules may be called with a NULL pointer
      when no usage rules are available.
    - Clarified management of protected buffers
  .
  - <b> 6.3.0 - 27-Sep-2016 </b>
    - Added user intent and PVR use case
    - Updated RSA and DH key sizes
    - Added support of DH from TEE
    - Clarified implementations of TSecAllocateBuffer in REE and TEE regarding
      protected buffers
    - Clarified definition of Transport Session ID
    - Clarified the way to allocate key slots
  .
  - <b> 6.2.0 - 28-Apr-2016 </b>
    - Added support of ETSI key ladder
    - Clarified usage of opaque buffers and defined a default opaque buffer type (TSecOpaqueBuffer)
    - Clarified usage of RAM2RAM and digest buffers. #TSecAllocateBuffer no longer required for these
      operations.
    - Clarified requirements related to protected buffers
    - Changed RSA and DH minimum key size to 1024 bits
    - Clarified usage of EMI
    - Fixed some typos
  .
  - <b> 6.1.0 - 09-Sep-2015 </b>
    - Added content usage rules processing
    - Added TEE privileged mode operations
  - <b> 6.0.1 - 27-Jul-2015 </b>
    - Internal documentation structure modifications
  - <b> 6.0.0 - 26-Jun-2015 </b>
    - Updated API to support a TEE architecture
    - Reworked documentation sections
    - Deprecated RSA key gen, SHA-1, SHA-384, ECDSA key gen, P160 and P384 curves,
      PKCS 1.5 and OAEP SHA-384 paddings
  .
  - <b> 5.1.2 - 14-Nov-2014 </b>
    - Included ca_types.h instead of ca_defs.h in order to comply with kernel
      mode support requirements
    - Removed NOCS references
    - Fixed some typos
  .
  - <b> 5.1.1 - 11-Jan-2013 </b>
    - Added secChipReset()
    - Clarified usage of SHA and HMAC context
    - Clarified management of buffers and forced usage of TSecAllocateBuffer()
      for RAM2RAM and digest functions.
    - Defined keySize parameter values for elliptic curves to be supported
    - Removed unwanted const qualifier on TSecFreeBuffer()
    - Clarified error management on inconsistent key ID and metadata parameters
  .
  - <b> 5.0.1 - 27-Sep-2012 </b>
    - Added support for stream processing
    - Replaced key parity by key ID in all secSetXxxKey functions
    - Added TSecAllocateBuffer() and TSecFreeBuffer() to comply with DMA
      buffer constraints.
    - Added support of RAM2RAM in-place operations
    - Added TSecUseLegacyKey() for specific DRM use case
  .
  - <b> 4.1.0 - 04-May-2012 </b>
    - Added TSecUseFlashProtKey() and TSecEncryptFlashProtKey()
  .
  - <b> 4.0.3 - 16-Mar-2012 </b>
    - Clarified elliptic curves to be supported and domain parameters size.
    - Clarified the list of EMIs to be supported
  .
  - <b> 4.0.2 - 22-Feb-2012 </b>
    - Added global overview
    - Using CRT parameters instead of d to define an RSA private key
    - Added function to compute CRT parameters dP, dQ and qInv from p, q and e.
    - Changed ECDSA signature output format to return explicitly return r and
      s components.
    - Changed TSecEcdhComputeKey() in order to consider the shared secret as
      the X-coordinates of the computed point.
    - Added support of SHA-384
    - Clarified RAM2RAM use cases to be supported. In particular it should
      be possible to change the key within the same RAM2RAM session.
    - Corrected description of #SEC_ERROR_BAD_PARAMETER on TSecSessionEncrypt()
      and TSecSessionDecrypt().
    - Corrected few typos
  .
  - <b> 3.1.0 - 31-Oct-2011 </b>
    - Removed support of DES
*/

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#ifdef SEC_WRAP_2_1
#include "sec_2_1w.h"
#endif

#ifdef SEC_GLUE_2_1
#include "sec_2_1g.h"
#endif

#include "ca_types.h"
#include "ca_defsx.h"

/******************************************************************************/
/*                                                                            */
/*                              VERSION TOOL                                  */
/*                                                                            */
/******************************************************************************/

#ifndef SEC_TOOL_VERSION
#define SEC_TOOL_STRINGIFY(s) SEC_TOOL_TOSTRING(s)
#define SEC_TOOL_TOSTRING(s)  #s

#define SEC_TOOL_VERSION_INT(a, b, c) (a<<16 | b<<8 | c)
#define SEC_TOOL_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define SEC_TOOL_VERSION(a, b, c) SEC_TOOL_VERSION_DOT(a, b, c)
#endif

#define SECAPI_VERSION_INTER SEC_TOOL_VERSION(SECAPI_VERSION_MAJOR, \
              SECAPI_VERSION_MEDIUM, \
              SECAPI_VERSION_MINOR)

#define SECAPI_VERSION_INT   SEC_TOOL_VERSION_INT(SECAPI_VERSION_MAJOR, \
            SECAPI_VERSION_MEDIUM, \
            SECAPI_VERSION_MINOR)

#define SECAPI_VERSION_STRING  "SECAPI_" SEC_TOOL_STRINGIFY(SECAPI_VERSION_INTER)




/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_chip_id
 *
 *  @brief
 *    Type used to accommodate the 32-bit NUID
*/
typedef TUnsignedInt8 TSecNuid[4];


/**
 *  @ingroup g_sec_chip_id
 *
 *  @brief
 *    Type used to accommodate the 64-bit NUID
*/
typedef TUnsignedInt8 TSecNuid64[8];


/**
 *  @brief
 *    Status code returned by functions of the SEC API.
 *
*/
typedef enum
{
  SEC_NO_ERROR,
   /**<  The intended operation was executed successfully.
    */
  SEC_ERROR,
   /**<  The function terminated abnormally. The intended operation failed.
    */
  SEC_ERROR_NOT_IMPLEMENTED,
   /**<  The function is not implemented.
    */
  SEC_ERROR_BAD_PARAMETER,
   /**<  Some of the parameters are missing or malformed.
    */
  SEC_ERROR_BAD_EMI,
   /**<  Given EMI is not supported
    */
  SEC_ERROR_BAD_USAGE,
   /**<  The usage is not allowed
    */
  SEC_ERROR_BAD_SIGNATURE,
   /**<  Verification of the signature failed
    */
  SEC_ERROR_BAD_PADDING,
   /**<  Verification of the padding occurring after a message decryption failed
    */
  SEC_ERROR_RULES_VIOLATION,
   /**< Usage rules are violated
    */
  LAST_SEC_STATUS
   /**<  Not used.
    */
} TSecStatus;


/**
 *  @brief
 *    Accepted hash types.
*/
typedef enum
{
  SEC_HASH_SHA1,
   /**<  SHA-1
    */
  SEC_HASH_SHA256,
   /**<  SHA-256
    */
  SEC_HASH_SHA384,
   /**<  @deprecated
    */
  LAST_SEC_HASH
   /**<  Internal use only
    */
}TSecHashType;


/**
  @ingroup g_sec_stream_processing
  @brief
    Structure defining ETSI key ladder profiles. Refer to @ref a_ref "[OMS]" for
    the detailed definition of each profile.
*/
typedef enum
{
  SEC_ETSI_SCTE_PROFILE_0,
  /**< Base profile */
  SEC_ETSI_SCTE_PROFILE_1,
  /**< Triple DES profile */
  SEC_ETSI_SCTE_PROFILE_1A,
  /**< Triple DES profile with module key derivation */
  SEC_ETSI_SCTE_PROFILE_2,
  /**< AES profile */
  SEC_ETSI_SCTE_PROFILE_2A,
  /**< AES encrypt profile with module key derivation */
  SEC_ETSI_SCTE_PROFILE_2B,
  /**< AES decrypt profile with module key derivation */
  SEC_ETSI_SCTE_LAST_PROFILE
  /**< Last enum entry */
} TSecEtsiProfile;

/**
  @ingroup g_sec_stream_processing
  @brief
    Type defining ciphers used at each stage of a key ladder
*/
typedef enum
{
  SEC_KLAD_CIPHER_TDES,
  /**< TDES keying option 2 cipher */
  SEC_KLAD_CIPHER_AES128,
  /**< AES-128 cipher */
  SEC_LAST_KLAD_CIPHER
  /**< Last enum entry */
} TSecKladCipher;


/**
  @ingroup g_sec_stream_processing
  @brief
    Structure defining the ETSI key ladder configuration.
*/
typedef struct
{
  TSecEtsiProfile   profile;
  /**<
    Profile defining the internal configuration of the ETSI key ladder
  */

  TUnsignedInt8     vendorId[2];
  /**<
    Identifier of CA vendors, network operators, and other entities using the
    ETSI key ladder
  */
  TUnsignedInt8     moduleId;
  /**<
    Module identifier required by profiles 1a, 2a and 2b
  */
  TSecKladCipher    cipher;
  /**<
    Cipher used at each of the 3 stages of the ETSI key ladder
  */
} TSecEtsiKladConfig;


/**
  @ingroup g_sec_stream_processing
  @brief
    Structure defining the key ladder configuration.
*/
typedef struct
{
  TUnsignedInt8     ownerId[2];
  /**<
    Identifier of the key ladder owner. The allocation strategy is based on the
    CA System ID start value defined by DVB (e.g. 0x0B00 for Conax).
  */
  TUnsignedInt8     rootKeyId;
  /**<
    Identifier of the key ladder root key
  */
  TSecKladCipher    cipher;
  /**<
    Cipher used at each of the 3 stages of the key ladder
  */
} TSecKladConfig;


/******************************************************************************/
/*                                                                            */
/*                             CHIPSET IDENTIFICATION                         */
/*                                                                            */
/******************************************************************************/

/**
  @ingroup g_sec_chip_id

  @brief
    This function returns a 4-byte array containing the 32-bit unique chipset
    identifier (NUID) that was programmed into the chipset's internal OTP
    memory by the chip manufacturer.

  @param[out] pxNuid
    Address of the array in which the function has to write the 4-byte NUID.
    Watch out, in order to avoid any alignment or endianness issues, the
    NUID has to be handled as an array of bytes and not as a 32-bit unsigned
    integer.

  @retval  SEC_NO_ERROR
    The NUID was read successfully.

  @retval  SEC_ERROR
    Found problems while trying to read the NUID.

*/
typedef TSecStatus (*TSecGetNuid)
(
  TSecNuid* pxNuid
);


/**
  @ingroup g_sec_chip_id

  @brief
    The prototype of this function is strictly identical to the callback
    TSecGetNuid. Its implementation is mandatory for backward compatibility
    with older client applications that are not aware of the function table
    TSecFunctionTable.
*/
TSecStatus secGetNuid
(
  TSecNuid* pxNuid
);


/**
  @ingroup g_sec_chip_id

  @brief
    This function returns an 8-byte array containing the 64-bit unique chipset
    identifier (NUID) that was programmed into the chipset's internal OTP
    memory by the chip manufacturer.

  @param[out] pxNuid
    Address of the array in which the function has to write the 8-byte NUID.
    Watch out, in order to avoid any alignment or endianness issues, the
    NUID has to be handled as an array of bytes and not as a 64-bit unsigned
    integer.

  @retval  SEC_NO_ERROR
    The NUID was read successfully.

  @retval  SEC_ERROR
    Found problems while trying to read the NUID.

*/
typedef TSecStatus (*TSecGetNuid64)
(
  TSecNuid64* pxNuid
);


/**
  @ingroup g_sec_chip_id

  @brief
    This function returns a null-terminated string containing the revision of
    the chipset (aka chipset cut).

    The internal format of this revision string is manufacturer dependent but
    has to be approved by Nagravision.

  @param[out] ppxChipsetRevision
    Pointer to be assigned to the buffer containing the chipset revision
    string. The SEC is in charge of allocating the buffer and must guarantee
    its integrity and never free it.

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR
    The operation failed.

*/
typedef TSecStatus (*TSecGetChipsetRevision)
(
  const TChar** ppxChipsetRevision
);


/**
  @ingroup g_sec_chip_id

  @brief
    The prototype of this function is strictly identical to the callback
    TSecGetChipsetRevision. Its implementation is mandatory for backward
    compatibility with older client applications that are not aware of the
    function table TSecFunctionTable.
*/
TSecStatus secGetChipsetRevision
(
  const TChar** ppxChipsetRevision
);


/**
  @ingroup g_sec_chip_id

  @brief
    This function returns a null-terminated string containing the extension of
    the chipset.

    The internal format of this extension string is manufacturer dependent but
    has to be approved by Nagravision.

  @param[out] ppxChipsetExtension
    Pointer to be assigned to the buffer containing the chipset extension
    string. The SEC is in charge of allocating the buffer and must guarantee
    its integrity and never free it.

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR
    The operation failed.

*/
typedef TSecStatus (*TSecGetChipsetExtension)
(
  const TChar** ppxChipsetExtension
);


/**
  @ingroup g_sec_chip_id

  @brief
    This function returns an 8-byte array containing the 64-bit unique chipset
    identifier realted to a given owner ID, that was programmed into the chipset's
    internal OTP memory by the chip manufacturer.

    If the chip ID is encoded on less than 64 bits, it has to be left padded
    with leading zeros.

  @param[in] pxOwnerId
    Owner of the chip ID. The allocation strategy of this value is based on the
    CA System ID start value defined by DVB (e.g. 0x0B00 for Conax).

  @param[out] pxChipId
    Address of the array in which the function has to write the 8-byte chip ID
    related to the given owner ID.
    Watch out, in order to avoid any alignment or endianness issues, the
    chip ID has to be handled as an array of bytes and not as a 64-bit unsigned
    integer.

  @retval  SEC_NO_ERROR
    The chip ID was read successfully.

  @retval  SEC_ERROR
    Found problems while trying to read the chip ID.

*/
typedef TSecStatus (*TSecGetChipId)
(
  TUnsignedInt8     pxOwnerId[2],
  TUnsignedInt8     pxChipId[8]
);


/******************************************************************************/
/*                                                                            */
/*                               CHIPSET CONFIGURATION                        */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_chip_config
 *
 *  @brief
 *    This function encrypts the global flash protection key with the chipset
 *    specific flash protection root key.
 *
 *    This function is intended to be used on the STB production line to
 *    encrypt the flash protection key before programming it in the flash
 *    SCS_Params_Area. Refer to the "Secure Chipset Integration Specification"
 *    of the chipset family concerned for further information about the process
 *    related to this key.
 *
 *  @param[in]  pxInput
 *    Buffer allocated by the caller containing the flash protection key to be
 *    encrypted.
 *
 *  @param[out]  pxOutput
 *    Buffer allocated by the caller where to write the encrypted flash protection
 *    key.
 *
 *  @param[in]  xSize
 *    Size in bytes of the flash protection key to be encrypted. It is equal to
 *    16 bytes for the time being.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if:
 *    - pxInput and/or pxOutput is NULL
 *    - xSize is different from 16
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    - The flash protection key is automatically decrypted, copied in RAM and
 *      authenticated by the secure chipset start-up process.
 *      This key must be then available for RAM2RAM operations from the boot
 *      context through BSD API (refer to BSD API documentation for further
 *      information) and after the OS is up and running through the SEC API
 *      TSecUseFlashProtKey().
*/
typedef TSecStatus (*TSecEncryptFlashProtKey)
(
  const TUnsignedInt8*     pxInput,
        TUnsignedInt8*     pxOutput,
        size_t              xSize
);


/**
 *  @ingroup g_sec_chip_config
 *
 *  @brief
 *    This function enables the TEE privileged mode. This operation is not reversible
 *    at all. Once enabled, Nagra CERT block is only accessible from the TEE.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
*/
typedef TSecStatus (*TSecEnableTeePrivilegedMode)
(
  void
);


/**
 *  @ingroup g_sec_chip_config
 *
 *  @brief
 *    This function gets the state of the TEE privileged mode.
 *
 *  @param[out]  pxEnabled
 *    This boolean is set to TRUE if the TEE privileged mode is enabled. It is
 *    set to FALSE otherwise.

 *  @retval  SEC_NO_ERROR
 *    The TEE privileged mode has been read successfully.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
*/
typedef TSecStatus (*TSecGetTeePrivilegedMode)
(
  TBoolean* pxEnabled
);


/******************************************************************************/
/*                                                                            */
/*                                  RESET                                     */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_reset
 *
 *  @brief
 *    This function performs a chipset reset. It must ensure the content of
 *    volatile memories (registers, caches, internal and external memories) has
 *    been cleared or is rendered inaccessible following the reset.
 *
 *    This function does not return.
 *
*/
void secChipReset
(
  void
);


/******************************************************************************/
/*                                                                            */
/*                                BLOCK CIPHER                                */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    Pointer to an opaque structure to be defined by the entity in charge of
 *    developing the SEC API and used as a handle for block cipher encryption
 *    and decryption sessions. The block cipher session may be either a RAM2RAM
 *    session or a stream session.
*/
typedef struct SSecCipherSession* TSecCipherSession;


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function asks the RAM2RAM crypto-engine to TDES encrypt a block of
 *    data stored in RAM with the RAM2RAM root key and write the result back in
 *    RAM. The choice of the encryption key and cryptographic algorithm is
 *    implicit.
 *
 *    If the caller provides the same input and output buffer (pxInput=pxOutput),
 *    the function must perform an in-place encryption.
 *
 *  @param[out] pxOutput
 *    Buffer allocated by the caller where to write the encrypted result
 *
 *  @param[in] pxInput
 *    Buffer containing the block of data to be encrypted.
 *
 *  @param[in] xDataSize
 *    Length in bytes of input and output data blocks to operate on. It shall
 *    be a multiple of 8.
 *
 *  @retval SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *    Also returned if xDataSize is not a multiple of 8 (TDES block cipher size).
 *
 *  @retval SEC_ERROR_NOT_IMPLEMENTED
 *    The function is not implemented
 *
 *  @see
 *    \ref s_sec_bc_rootkey
*/
typedef TSecStatus (*TSecEncryptData)
(
        TUnsignedInt8*  pxOutput,
  const TUnsignedInt8*  pxInput,
        TSize           xDataSize
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    The prototype of this function is strictly identical to the callback
 *    TSecEncryptData(). Its implementation is mandatory for backward
 *    compatibility with older client applications that are not aware of the
 *    function table TSecFunctionTable.
*/
TSecStatus secEncryptData
(
        TUnsignedInt8*  pxOutput,
  const TUnsignedInt8*  pxInput,
        TSize           xDataSize
);



/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function asks the RAM2RAM crypto-engine to TDES decrypt a block of
 *    data stored in RAM with the RAM2RAM root key and write the result back in
 *    RAM. The choice of the encryption key and cryptographic algorithm is
 *    implicit.
 *
 *    If the caller provides the same input and output buffer (pxInput=pxOutput),
 *    the function must perform an in-place encryption.
 *
*  @param[out] pxOutput
 *    Buffer allocated by the caller where to write the decrypted result
 *
 *  @param[in] pxInput
 *    Buffer containing the block of data to be decrypted.
 *
 *  @param[in] xDataSize
 *    Length in bytes of input and output data blocks to operate on. It shall
 *    be a multiple of 8.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *    Also returned if xDataSize is not a multiple of 8 (TDES block cipher size).
 *
 *  @retval  SEC_ERROR_NOT_IMPLEMENTED
 *    The function is not implemented.
 *
 *  @see
 *    \ref s_sec_bc_rootkey
*/
typedef TSecStatus (*TSecDecryptData)
(
        TUnsignedInt8*  pxOutput,
  const TUnsignedInt8*  pxInput,
        TSize           xDataSize
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    The prototype of this function is strictly identical to the callback
 *    TSecDecryptData(). Its implementation is mandatory for backward
 *    compatibility with older client applications that are not aware of the
 *    function table TSecFunctionTable.
*/
TSecStatus secDecryptData
(
        TUnsignedInt8*  pxOutput,
  const TUnsignedInt8*  pxInput,
        TSize           xDataSize
);



/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function opens a RAM2RAM encryption session and returns a session
 *    handle that will be used by other functions to set keys and encrypt data.
 *
 *    It is possible to change the encryption key and/or the encryption
 *    algorithm (EMI) during the same session. The protection of the encryption
 *    key (clear-text, key ladder, CERT) may also change during a session.
 *
 *  @param[out]  pxSession
 *    Session handle assigned by this function
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if pxSession is NULL.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @see
 *    - @ref p_sec_r2r "RAM2RAM"
*/
typedef TSecStatus (*TSecOpenRam2RamEncryptSession)
(
  TSecCipherSession*     pxSession
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function opens a RAM2RAM decryption session and returns a session
 *    handle that will be used by other functions to set keys and decrypt data.
 *
 *    It is possible to change the decryption key and/or the decryption
 *    algorithm (EMI) during the same session. The protection of the decryption
 *    key (clear-text, key ladder, CERT) may also change during a session.
 *
 *  @param[out]  pxSession
 *    Session handle assigned by this function
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if pxSession is NULL.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @see
 *    - @ref p_sec_r2r "RAM2RAM"
*/
typedef TSecStatus (*TSecOpenRam2RamDecryptSession)
(
  TSecCipherSession*     pxSession
);


/**
 *  @brief
 *    This function opens a stream encryption session and returns a session
 *    handle that will be used by other functions to set keys.
 *
 *    It is possible to change the encryption key and/or the encryption
 *    algorithm (EMI) during the same session. The protection of the encryption
 *    key (clear-text, key ladder, CERT) may also change during a session.
 *
 *  @param[out]  pxSession
 *    Session handle assigned by this function
 *
 *  @param[in]  xTransportSessionId
 *    Transport session ID associated to the streams to be encrypted. It is
 *    defined and provided by the application component in charge of managing
 *    the streams.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if:
 *    - pxSession is NULL
 *    - xTransportSessionId is invalid (#TRANSPORT_SESSION_ID_INVALID)
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    - Unlike RAM2RAM sessions, stream processing sessions do not directly
 *      handle data to be encrypted, it handles keys only. The relation between
 *      keys and streams is achieved thanks to the Transport Session ID and
 *      optionally the key ID provided by each secSetXxxKey() or secUseXxxKey()
 *      functions.
 *
 *  @see
 *    - @ref p_sec_stream_processing "Stream processing"
*/
typedef TSecStatus (*TSecOpenStreamEncryptSession)
(
  TSecCipherSession*     pxSession,
  TTransportSessionId     xTransportSessionId
);


/**
 *  @brief
 *    This function opens a stream decryption session and returns a session
 *    handle that will be used by other functions to set keys.
 *
 *    It is possible to change the decryption key and/or the decryption
 *    algorithm (EMI) during the same session. The protection of the decryption
 *    key (clear-text, key ladder, CERT) may also change during a session.
 *
 *  @param[out]  pxSession
 *    Session handle assigned by this function
 *
 *  @param[in]  xTransportSessionId
 *    Transport session ID associated to the streams to be decrypted. It is
 *    defined and provided by the application component in charge of managing
 *    the streams.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if:
 *    - pxSession is NULL
 *    - xTransportSessionId is invalid (#TRANSPORT_SESSION_ID_INVALID)
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    - Unlike RAM2RAM sessions, stream processing sessions do not directly
 *      handle data to be decrypted, it handles keys only. The relation between
 *      keys and streams is achieved thanks to the Transport Session ID and
 *      optionally the key ID provided by each secSetXxxKey() or secUseXxxKey()
 *      functions.
 *
 *  @see
 *    - @ref p_sec_stream_processing "Stream processing"
*/
typedef TSecStatus (*TSecOpenStreamDecryptSession)
(
  TSecCipherSession*     pxSession,
  TTransportSessionId     xTransportSessionId
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function closes any kind of block cipher session and frees the
 *    allocated resources.
 *
 *  @param[in]  xSession
 *    Handle of the session to be closed
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if xSession is NULL or does not correspond to a
 *    valid session.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    If it is called to close a stream processing session, this function must
 *    flush the related crypto-engine key table registers in order to
 *    immediately stop the decryption/encryption of the media streams that are
 *    potentially still flowing through this crypto-engine element.
*/
typedef TSecStatus (*TSecCloseSession)
(
  TSecCipherSession      xSession
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function sets a clear-text key in a crypto-engine for RAM2RAM or
 *    stream encryption/decryption operations.
 *
 *    The type of operation (encryption or decryption) is defined at session
 *    opening (TSecOpenXxxEncryptSession or TSecOpenXxxDecryptSession). The EMI
 *    provided as input parameter clearly identifies the cryptographic algorithm
 *    and chaining mode. At this stage the driver have all the information
 *    required to allocate hardware resources.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  xEmi
 *    Identifier of the cryptographic algorithm (TDES, AES, DVB-CSA) and block
 *    cipher modes (ECB, CBC, CTR) to be used by the crypto-engine.
 *
 *  @param[in]  xKeyIdSize
 *    Size in bytes of the key ID (pxKeyId). If no key ID is required (e.g.
 *    RAM2RAM session), xKeyIdSize is set to 0.
 *
 *  @param[in]  pxKeyId
 *    Identifier of the key provided:
 *    - If no key ID is required (e.g. RAM2RAM session), pxKeyId is set to NULL.
 *    - The key ID is typically used for MPEG transport stream decryption session
 *      (i.e. stream session associated to EMI 0x00XX) and indicates the parity
 *      of the keys. In such a case, the function must consider the parity of
 *      the key ID as the key polarity: for instance key ID = 0x00 => even
 *      key and key ID = 0x01 => odd key.
 *    - The key ID is also required to support key rotations with HLS (EMI
 *      0x4023) and MPEG/DASH (EMI 0x4024) standards. MPEG/DASH uses a key ID of
 *      16 bytes.
 *    - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
 *      of the key. For PVR raw encryption EMI, the key ID is always NULL.
 *
 *  @param[in]  xClearTextKeySize
 *    Size in bytes of the clear-text key:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *
 *  @param[in]  pxClearTextKey
 *    Buffer containing the clear-text key. The buffer is allocated by the
 *    caller. In case of TDES keying option 2, K1 and K2 are provided as follows:
 *    - K1=pxClearTextKey[0:7]
 *    - K2=pxClearTextKey[8:15]
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL clear-text key pointer
 *    - Size of key does not match the crypto algorithm defined by the EMI
 *    - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
 *      and vice-versa)
 *
 *  @retval  SEC_ERROR_BAD_EMI
 *    The EMI given as input parameter is not supported.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @remarks
 *    This function may be called at any time during a given session to change
 *    the EMI and/or the keys.
 *
 *  @see
 *    \ref s_sec_bc_clearkey
*/
typedef TSecStatus (*TSecSetClearTextKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi,
        size_t              xKeyIdSize,
        TUnsignedInt8*     pxKeyId,
        size_t              xClearTextKeySize,
  const TUnsignedInt8*     pxClearTextKey
);


/**
 *  @brief
 *    This function sets a protected content key in a 0-level key ladder
 *    crypto-engine for stream encryption/decryption operations. It is not
 *    intended to be used for RAM2RAM session.
 *
 *    The type of operation (encryption or decryption) is defined at session
 *    opening (TSecOpenStreamEncryptSession or TSecOpenStreamDecryptSession).
 *    The EMI provided as input parameter clearly identifies the cryptographic
 *    algorithm and chaining mode. At this stage the driver have all the
 *    information required to allocate hardware resources.
 *
 *    Beware that the key provided to the key ladder is protected with TDES-ECB
 *    keying option 2. This key ladder algorithm must not be confused with the
 *    crypto-engine algorithm signaled by the EMI.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  xKeyIdSize
 *    Size in bytes of the key ID (pxKeyId). If no key ID is required,
 *    xKeyIdSize is set to 0.
 *
 *  @param[in]  pxKeyId
 *    Identifier of the key provided:
 *    - If no key ID is required, pxKeyId is set to NULL.
 *    - The key ID is typically used for MPEG transport stream decryption session
 *      (i.e. stream session associated to EMI 0x00XX) and indicates the parity
 *      of the keys. In such a case, the function must consider the parity of
 *      the key ID as the key polarity: for instance key ID = 0x00 => even
 *      key and key ID = 0x01 => odd key.
 *    - The key ID is also required to support key rotations with HLS and MPEG-DASH
 *      standards. MPEG/DASH uses a key ID of 16 bytes.
 *    - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
 *      of the key. For PVR raw encryption EMI, the key ID is always NULL.
 *
 *  @param[in]  xEmi
 *    Identifier of the cryptographic algorithm (TDES, AES, DVB-CSA) and block
 *    cipher modes (ECB, CBC, CTR) to be used by the crypto-engine.
 *
 *  @param[in]  xCipheredContentKeySize
 *    Size in bytes of the ciphered content key:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *
 *  @param[in]  pxCipheredContentKey
 *    Buffer, allocated by the caller, containing the ciphered content key. It
 *    is equal to CipheredContentKey=TDES(ContentKey, RootKey).
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL key pointers
 *    - Size of content key does not match the crypto algorithm defined by the EMI
 *    - Unsupported protecting key size
 *    - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
 *      and vice-versa)
 *
 *  @retval  SEC_ERROR_BAD_EMI
 *    The EMI given as input parameter is not supported.
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This status is returned if xSession is not related to a stream session, i.e
 *    session opened with TSecOpenStreamEncryptSession() or
 *    TSecOpenStreamDecryptSession().
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @remarks
 *    This function may be called at any time during a given session to change
 *    the EMI and/or the keys.
 *
 *  @see
 *    \ref s_sec_bc_0kl_prot
*/
typedef TSecStatus (*TSecSet0LevelProtectedKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi,
        size_t              xKeyIdSize,
        TUnsignedInt8*     pxKeyId,
        size_t              xCipheredContentKeySize,
  const TUnsignedInt8*     pxCipheredContentKey
);


/**
 *  @brief
 *    This function sets protected keys required by the 1-level key ladder
 *    associated to the crypto-engine for stream encryption/decryption operations.
 *    It is not intended to be used for RAM2RAM session.
 *
 *    The type of operation (encryption or decryption) is defined at session
 *    opening (TSecOpenStreamEncryptSession or TSecOpenStreamDecryptSession).
 *    The EMI provided as input parameter clearly identifies the cryptographic
 *    algorithm and chaining mode. At this stage the driver have all the
 *    information required to allocate hardware resources.
 *
 *    Beware that keys provided to the key ladder are all protected with
 *    TDES-ECB keying option 2. This key ladder algorithm must not be confused
 *    with the crypto-engine algorithm signalled by the EMI.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  xKeyIdSize
 *    Size in bytes of the key ID (pxKeyId). If no key ID is required,
 *    xKeyIdSize is set to 0.
 *
 *  @param[in]  pxKeyId
 *    Identifier of the key provided:
 *    - If no key ID is required, pxKeyId is set to NULL.
 *    - The key ID is typically used for MPEG transport stream decryption session
 *      (i.e. stream session associated to EMI 0x00XX) and indicates the parity
 *      of the keys. In such a case, the function must consider the parity of
 *      the key ID as the key polarity: for instance key ID = 0x00 => even
 *      key and key ID = 0x01 => odd key.
 *    - The key ID is also required to support key rotations with HLS and MPEG-DASH
 *      standards. MPEG/DASH uses a key ID of 16 bytes.
 *    - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
 *      of the key. For PVR raw encryption EMI, the key ID is always NULL.
 *
 *  @param[in]  xEmi
 *    Identifier of the cryptographic algorithm (TDES, AES, DVB-CSA) and block
 *    cipher modes (ECB, CBC, CTR) to be used by the crypto-engine.
 *
 *  @param[in]  xCipheredContentKeySize
 *    Size in bytes of the ciphered content key:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *
 *  @param[in]  pxCipheredContentKey
 *    Buffer, allocated by the caller, containing the ciphered content key. It
 *    is equal to CipheredContentKey=TDES(ContentKey, L1ProtKey).
 *
 *  @param[in]  xCipheredProtectingKeySize
 *    Size in bytes of the intermediate level protecting keys (pxL1CipheredProtectingKey) used within the
 *    key ladder.
 *
 *  @param[in]  pxL1CipheredProtectingKey
 *    Buffer, allocated by the caller, containing the first ciphered protecting
 *    key fed into the key ladder. It is equal to L1CipheredProtKey=TDES(L1ProtKey,
 *    RootKey).
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL key pointers
 *    - Size of content key does not match the crypto algorithm defined by the EMI
 *    - Unsupported protecting key size
 *    - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
 *      and vice-versa)
 *
 *  @retval  SEC_ERROR_BAD_EMI
 *    The EMI given as input parameter is not supported.
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This status is returned if xSession is not related to a stream session, i.e.
 *    session opened with TSecOpenStreamEncryptSession() or
 *    TSecOpenStreamDecryptSession().
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @remarks
 *    This function may be called at any time during a given session to change
 *    the EMI and/or the keys.
 *
 *  @see
 *    \ref s_sec_bc_1kl_prot
*/
typedef TSecStatus (*TSecSet1LevelProtectedKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi,
        size_t              xKeyIdSize,
        TUnsignedInt8*     pxKeyId,
        size_t              xCipheredContentKeySize,
  const TUnsignedInt8*     pxCipheredContentKey,
        size_t              xCipheredProtectingKeySize,
  const TUnsignedInt8*     pxL1CipheredProtectingKey
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function sets protected keys required by the 2-level key ladder
 *    associated to the crypto-engine for RAM2RAM or stream encryption/decryption
 *    operations.
 *
 *    The type of operation (encryption or decryption) is defined at session
 *    opening (TSecOpenXxxEncryptSession or TSecOpenXxxDecryptSession). The EMI
 *    provided as input parameter clearly identifies the cryptographic algorithm
 *    and chaining mode. At this stage the driver have all the information
 *    required to allocate hardware resources.
 *
 *    Beware that keys provided to the key ladder are all protected with
 *    TDES-ECB keying option 2. This key ladder algorithm must not be confused
 *    with the crypto-engine algorithm signalled by the EMI.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  xKeyIdSize
 *    Size in bytes of the key ID (pxKeyId). If no key ID is required (e.g.
 *    RAM2RAM session), xKeyIdSize is set to 0.
 *
 *  @param[in]  pxKeyId
 *    Identifier of the key provided:
 *    - If no key ID is required (e.g. RAM2RAM session), pxKeyId is set to NULL.
 *    - The key ID is typically used for MPEG transport stream decryption session
 *      (i.e. stream session associated to EMI 0x00XX) and indicates the parity
 *      of the keys. In such a case, the function must consider the parity of
 *      the key ID as the key polarity: for instance key ID = 0x00 => even
 *      key and key ID = 0x01 => odd key.
 *    - The key ID is also required to support key rotations with HLS and MPEG-DASH
 *      standards. MPEG/DASH uses a key ID of 16 bytes.
 *    - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
 *      of the key. For PVR raw encryption EMI, the key ID is always NULL.
 *
 *  @param[in]  xEmi
 *    Identifier of the cryptographic algorithm (TDES, AES, DVB-CSA) and block
 *    cipher modes (ECB, CBC, CTR) to be used by the crypto-engine.
 *
 *  @param[in]  xCipheredContentKeySize
 *    Size in bytes of the ciphered content key:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *
 *  @param[in]  pxCipheredContentKey
 *    Buffer, allocated by the caller, containing the ciphered content key. It
 *    is equal to CipheredContentKey=TDES(ContentKey, L1ProtKey).
 *
 *  @param[in]  xCipheredProtectingKeySize
 *    Size in bytes of the intermediate level protecting keys (pxL1CipheredProtectingKey
 *    and pxL2CipheredProtectingKey) used within the key ladder.
 *
 *  @param[in]  pxL1CipheredProtectingKey
 *    Buffer, allocated by the caller, containing the first ciphered protecting
 *    key fed into the key ladder. It is equal to
 *    L1CipheredProtKey=TDES(L1ProtKey, L2ProtKey).
 *
 *  @param[in]  pxL2CipheredProtectingKey
 *    Buffer, allocated by the caller, containing the second ciphered protecting
 *    key fed into the key ladder. It is equal to L2CipheredProtKey=TDES(L2ProtKey,
 *    RootKey)
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL key pointers
 *    - Size of content key does not match the crypto algorithm defined by the EMI
 *    - Unsupported protecting key size
 *    - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
 *      and vice-versa)
 *
 *  @retval  SEC_ERROR_BAD_EMI
 *    The EMI given as input parameter is not supported.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @remarks
 *    This function may be called at any time during a given session to change
 *    the EMI and/or the keys.
 *
 *  @see
 *    \ref s_sec_bc_2kl_prot
*/
typedef TSecStatus (*TSecSet2LevelProtectedKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi,
        size_t              xKeyIdSize,
        TUnsignedInt8*     pxKeyId,
        size_t              xCipheredContentKeySize,
  const TUnsignedInt8*     pxCipheredContentKey,
        size_t              xCipheredProtectingKeySize,
  const TUnsignedInt8*     pxL1CipheredProtectingKey,
  const TUnsignedInt8*     pxL2CipheredProtectingKey
);


/**
  @brief
    This function unwraps a content key protected by an ETSI key ladder and set it in
    a key table for stream en/decryption operations. Refer  to @ref a_ref "[ETSI]"
    and @ref a_ref "[OMS]" for the specification of the ETSI key ladder.

    The ETSI key ladder shall share the same key table than NOCS and CERT key
    ladders in order to support a change of key protection on the same session.

  @param[in]  pxKladConfig
    Key ladder configuration

  @param[in]  xSession
    Handle of the session

  @param[in]  xEmi
    Identifier of the cryptographic algorithm and block cipher modes to be used by
    the crypto-engine.

  @param[in]  xKeyIdSize
    Size in bytes of the key ID (pxKeyId).

  @param[in]  pxKeyId
    - Identifier of the key provided. It is typically used for MPEG transport stream decryption session
    (i.e. stream session associated to EMI 0x00XX) and indicates the parity
    of the keys. In such a case, the function must consider the parity of
    the key ID as the key polarity: for instance key ID = 0x00 => even
    key and key ID = 0x01 => odd key.
    - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
     of the key. For PVR raw encryption EMI, the key ID is always NULL.

  @param[in]  xCipheredContentKeySize
    Size in bytes of the ciphered content key. It depends on the key ladder
    cipher (#TSecKladCipher) and the scrambling algorithm:
    - It is always equal to 16 bytes when the key ladder cipher is AES-128
    - It is equal to 16 bytes when the key ladder cipher is TDES and the
      scrambling algorithm is AES or DVB-CSA3
    - It is equal to 8 bytes when the key ladder cipher is TDES and the
      scrambling algorithm is DES or DVB-CSA2.

  @param[in]  pxCipheredContentKey
    Buffer, allocated by the caller, containing the content key (CW) ciphered with
    K1.

  @param[in]  xCipheredProtectingKeySize
    Size in bytes of the intermediate level protecting keys (pxL1CipheredProtectingKey
    and pxL2CipheredProtectingKey) used as key ladder inputs.

  @param[in]  pxL1CipheredProtectingKey
    Buffer, allocated by the caller, containing the protecting key (K1) ciphered
    with K2.

  @param[in]  pxL2CipheredProtectingKey
    Buffer, allocated by the caller, containing the protecting key (K2) ciphered
    with K3.

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR_BAD_PARAMETER
    An invalid parameter is provided:
    - NULL or unknown session
    - NULL key pointers
    - NULL key ladder config pointer
    - Size of key does not match the crypto algorithm defined by the EMI
    - Unsupported key ladder configuration (TSecEtsiKladConfig::profile or
      TSecEtsiKladConfig::cipher)

  @retval  SEC_ERROR_BAD_EMI
    The EMI given as input parameter is not supported.

  @retval  SEC_ERROR
    Any other errors

  @remarks
    This function may be called at any time during a given session to change
    the key.

  @see
    @ref s_sec_bc_etsi_kl
*/
typedef TSecStatus (*TSecSetEtsi2LevelProtectedKey)
(
        TSecCipherSession    xSession,
        TUnsignedInt16       xEmi,
  const TSecEtsiKladConfig* pxKladConfig,
        size_t               xKeyIdSize,
  const TUnsignedInt8*      pxKeyId,
        size_t               xCipheredContentKeySize,
  const TUnsignedInt8*      pxCipheredContentKey,
        size_t               xCipheredProtectingKeySize,
  const TUnsignedInt8*      pxL1CipheredProtectingKey,
  const TUnsignedInt8*      pxL2CipheredProtectingKey
);


/**
  @ingroup g_sec_r2r

  @brief
    This function unwraps a content key protected by a MKL 2-level key ladder
    and sets it in a key table for RAM2RAM operations.

    The caller has to configure the key ladder owner, key ladder root key and
    algorithm of all key ladder internal elements.

  @param[in]  pxKladConfig
    Key ladder configuration

  @param[in]  xSession
    Handle of the session

  @param[in]  xEmi
    Identifier of the cryptographic algorithm and block cipher modes to be used by
    the crypto-engine.

  @param[in]  xCipheredContentKeySize
    Size in bytes of the ciphered content key. It depends on the key ladder
    cipher (#TSecKladCipher) and the scrambling algorithm:
    - It is always equal to 16 bytes when the key ladder cipher is AES-128
    - It is equal to 16 bytes when the key ladder cipher is TDES and the
      scrambling algorithm is AES or DVB-CSA3
    - It is equal to 8 bytes when the key ladder cipher is TDES and the
      scrambling algorithm is DES or DVB-CSA2.

  @param[in]  pxCipheredContentKey
    Buffer, allocated by the caller, containing the content key (CW) ciphered with
    the L1 protecting key.

  @param[in]  xCipheredProtectingKeySize
    Size in bytes of all intermediate level protecting keys (pxL1CipheredProtectingKey
    and pxL2CipheredProtectingKey) used as key ladder inputs.

  @param[in]  pxL1CipheredProtectingKey
    Buffer, allocated by the caller, containing the L1 protecting key ciphered
    with L2 protecting key.

  @param[in]  pxL2CipheredProtectingKey
    Buffer, allocated by the caller, containing the L2 protecting key ciphered
    with the key ladder root key.

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR_BAD_PARAMETER
    An invalid parameter is provided:
    - NULL or unknown session
    - NULL key pointers
    - NULL key ladder config pointer
    - Size of key does not match the crypto algorithm defined by the EMI

  @retval  SEC_ERROR_BAD_EMI
    The EMI given as input parameter is not supported.

  @retval  SEC_ERROR
    Any other errors

  @remarks
    This function may be called at any time during a given RAM2RAM session to
    change
    the key.

  @see
    @ref s_sec_bc_generic_2kl_prot
*/
typedef TSecStatus (*TSecSetMkl2LevelProtectedKey)
(
        TSecCipherSession    xSession,
        TUnsignedInt16       xEmi,
  const TSecKladConfig*     pxKladConfig,
        size_t               xCipheredContentKeySize,
  const TUnsignedInt8*      pxCipheredContentKey,
        size_t               xCipheredProtectingKeySize,
  const TUnsignedInt8*      pxL1CipheredProtectingKey,
  const TUnsignedInt8*      pxL2CipheredProtectingKey
);


/**
   @ingroup g_sec_r2r

   @brief
     This function copies the key available on the CERT key output interface in
     a crypto-engine for RAM2RAM or stream encryption/decryption operations.

     The type of operation (encryption or decryption) is defined at session
     opening (TSecOpenXxxEncryptSession or TSecOpenXxxDecryptSession). The EMI
     provided as input parameter clearly identifies the cryptographic algorithm
     and chaining mode. At this stage the driver have all the information
     required to allocate hardware resources.

     This function also acknowledges the CERT block to release the CERT key output
     interface and allow the output of another key.

   @param[in]  xSession
     Handle of the session

   @param[in]  xEmi
     Identifier of the cryptographic algorithm (TDES, AES, DVB-CSA, ASA) and
     block cipher modes (ECB, CBC, CTR) to be used by the crypto-engine.

   @param[in]  xKeyIdSize
     Size in bytes of the key ID (pxKeyId). If no key ID is required (e.g.
     RAM2RAM session), xKeyIdSize is set to 0.

   @param[in]  pxKeyId
     Identifier of the key provided:
     - If no key ID is required (e.g. RAM2RAM session), pxKeyId is set to NULL.
     - The key ID is typically used for MPEG transport stream decryption session
       (i.e. stream session associated to EMI 0x00XX) and indicates the parity
       of the keys. In such a case, the function must consider the parity of
       the key ID as the key polarity: for instance key ID = 0x00 => even
       key and key ID = 0x01 => odd key.
     - The key ID is also required to support key rotations with HLS and MPEG-DASH
       standards. MPEG/DASH uses a key ID of 16 bytes.
     - For PVR MPEG encryption EMI, the key ID may be NULL or indicate the polarity
       of the key. For PVR raw encryption EMI, the key ID is always NULL.

   @retval  SEC_NO_ERROR
     The operation was completed successfully.

   @retval  SEC_ERROR_BAD_PARAMETER
     An invalid parameter is provided:
     - NULL or unknown session
     - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
       and vice-versa)

   @retval  SEC_ERROR_BAD_EMI
     The EMI given as input parameter is not supported.

   @retval  SEC_ERROR
     Any other errors

   @remarks
     - This function may be called at any time during a given session to change
       the keys.
     - Although CERT key usage still need to be enforced by the SoC, this function
       no longer needs to return the #SEC_ERROR_BAD_USAGE status. Indeed, key usage
       are usually checked at en/decryption time that happens after this call.

   @warning
     - The CERT block must be acknowledged even in case the EMI provided is not
       supported (#SEC_ERROR_BAD_EMI). Otherwise the CERT block will no longer
       be able output keys on the key output interface.

   @see
     \ref s_sec_bc_certkey
*/
typedef TSecStatus (*TSecUseCertKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi,
        size_t              xKeyIdSize,
        TUnsignedInt8*     pxKeyId
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function copies the flash protection key in a crypto-engine key table
 *    for RAM2RAM operations. It is not intended to be used for stream processing
 *    operations.
 *
 *    The type of operation (encryption or decryption) is defined at session
 *    opening (#TSecOpenRam2RamEncryptSession or #TSecOpenRam2RamDecryptSession).
 *
 *    The EMI provided as input parameter clearly identifies the cryptographic
 *    algorithm and chaining mode. At this stage the driver have all the
 *    information required to allocate hardware resources.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  xEmi
 *    Identifier of the cryptographic algorithm (TDES, AES) and block
 *    cipher modes (ECB, CBC, CTR) to be used by the RAM2RAM crypto-engine.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if xSession is NULL or does not correspond to a
 *    valid session.
 *
 *  @retval  SEC_ERROR_BAD_EMI
 *    The EMI given as input parameter is not supported.
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This status is returned if xSession is not related to a RAM2RAM session,
 *    i.e. session opened with TSecOpenRam2RamEncryptSession() or
 *    TSecOpenRam2RamDecryptSession().
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @remarks
 *    - This function may be called at any time during a given session to change
 *      the EMI and/or key associated to this session. A session is related to
 *      one single key at a time, the latest one set.
 *
 *  @note
 *    - This function must rely on the authenticated cleartext instance of the
 *      flash protection key made available by the secure chipset start-up
 *      process. It must never access the original flash protection key located
 *      in the flash SCS_Params_Area.
 *
 *  @see TSecEncryptFlashProtKey()
*/
typedef TSecStatus (*TSecUseFlashProtKey)
(
        TSecCipherSession   xSession,
        TUnsignedInt16      xEmi
);


/**
 *  @brief
 *    This function uses the legacy secret key for stream processing operations.
 *    It is not intended to be used for RAM2RAM operations.
 *
 *    The EMI associated to these operations is not explicitly given by the
 *    caller, but it is implicitly linked to this legacy key.
 *
 *    The main use case behind this function is the ability of playing back a
 *    media content stored on the HDD protected with a custom key.
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    This status is returned if xSession is NULL or does not correspond to a
 *    valid session.
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This status is returned if xSession is not related to a stream session, i.e.
 *    session opened with TSecOpenStreamEncryptSession() or
 *    TSecOpenStreamDecryptSession().
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    This function replaces the scrSetLegacyDecryptionMode() function of the
 *    deprecated SCR API.
*/
typedef TSecStatus (*TSecUseLegacyKey)
(
        TSecCipherSession   xSession
);


/**
 *  @brief
 *    This function associates metadata to a stream decryption session. It
 *    typically concerns content usage rules such as copy control information.
 *
 *    Metadata consists in a collection of TLV descriptors (1-byte length, 1-byte
 *    tag) complying with the format defined in [META].
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in] xMetadataLength
 *    Size in bytes of the metadata buffer.
 *
 *  @param[in] pxMetadata
 *    Buffer allocated by the caller containing metadata descriptors
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - Metadata parameters inconsistency (e.g. xMetadataSize!=0 and pxMetadata=NULL,
 *      and vice-versa)
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This status is returned if xSession is related to a RAM2RAM session, i.e.
 *    session opened with TSecOpenRam2RamEncryptSession() or
 *    TSecOpenRam2RamDecryptSession().
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @note
 *    -# This function may be called any time during a stream session in case of
 *       change of metadata (descriptors added, removed or updated). In such a
 *       case, it is called before setting the key (secSetXxxKey or
 *       secUseXxxKey) and the metadata provided shall be considered as
 *       applicable until the next TSecSetMetadata() call.
 *    -# The removal of a given metadata descriptor implies the cancellation of
 *       the underlying feature. For instance, if a copy protection descriptor
 *       is no longer available in the metadata buffer, the driver shall no
 *       longer manage the copy control on this stream session.
 *    -# If all metadata descriptors are removed, TSecSetMetadata is called with
 *       pxMetadata set to NULL and xMetadataSize set to 0.
*/
typedef TSecStatus (*TSecSetMetadata)
(
        TSecCipherSession   xSession,
        size_t              xMetadataSize,
  const TUnsignedInt8*     pxMetadata
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function performs a RAM2RAM data encryption.
 *
 *    It encrypts the data contained in pxInput buffer and stores the resulted
 *    encrypted data in pxOutput buffer.
 *
 *    This function may be called several times during a given session in order
 *    to encrypt a message split in several chunks. If the block cipher modes of
 *    operation requests an initialization vector (IV), the caller is in charge
 *    of the consistency of the IV passed at each chunk.
 *
 *    The size of message to be encrypted does not need to be a multiple of the
 *    cipher block size. In such a case, the n residual bytes (with
 *    n = xMessageSize mod cipher block size) are left in the clear and simply
 *    copied from input to output buffer provided the mode of operation is ECB
 *    or CBC. For CTR mode, encryption actually consists in a simple XOR, thus
 *    residual bytes are encrypted as any other bytes of the message.
 *
 *    This function does not manage any padding. If a padding schema is required,
 *    the caller is in charge of applying it to the input message before calling
 *    this function.
 *
 *    If the caller provides the same input and output buffer (pxInput=pxOutput),
 *    the function must perform an in-place encryption.
 *
 *  @pre
 *    The content key has been set through TSecSetClearTextKey(),
 *    TSecSet2LevelProtectedKey(), TSecUseCertKey() or TSecUseFlashProtKey().
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to encrypt. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the encrypted message. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It does not
 *    need to be a multiple of the cipher block size.
 *
 *  @param[in] pxInitVector
 *    Buffer allocated by the caller containing the initialization vector (IV)
 *    required by CBC and CTR modes. These cipher modes of operation are part of
 *    the EMI definition. If no IV is required (ECB mode), pxInitVector is set
 *    to NULL.\n
 *    Normally the CTR mode requires a nonce and a counter that are then
 *    combined to produce the actual counter block encryption. In the current
 *    API the caller is in charge of providing the initial counter block instead
 *    of a nonce and a counter. This initial counter block is passed within
 *    pxInitVector buffer. The function TSecSessionEncrypt() is then in charge of
 *    incrementing the counter block of one unit for each block encryption.
 *
 *    The caller is in charge of providing a correct and consistent IV, even
 *    in case the data to be encrypted requires several calls to this function.
 *
 *  @param[in] xInitVectorSize
 *    Size in bytes of the initialization vector. It is equal to the cipher
 *    block size (8 bytes for TDES and 16 bytes for AES). If no IV is required
 *    (ECB mode), xInitVectorSize is set to zero.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL input/output buffers
 *    - Unsupported IV size (not a multiple of cipher block size)
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This error status is returned if xSession refers to a decryption session.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
*/
typedef TSecStatus (*TSecSessionEncrypt)
(
        TSecCipherSession   xSession,
  const TUnsignedInt8*     pxInput,
        TUnsignedInt8*     pxOutput,
        size_t              xMessageSize,
  const TUnsignedInt8*     pxInitVector,
        size_t              xInitVectorSize
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function performs a RAM2RAM data decryption.
 *
 *    It decrypts the data contained in pxInput buffer and stores the resulted
 *    decrypted data in pxOutput buffer.
 *
 *    This function may be called several times during a given session in order
 *    to decrypt a message split in several chunks. If the block cipher modes of
 *    operation requests an initialization vector (IV), the caller is in charge
 *    of the consistency of the IV passed at each chunk.
 *
 *    The size of message to be decrypted does not need to be a multiple of the
 *    cipher block size. In such a case, the n residual bytes (with
 *    n = xMessageSize mod cipher block size) are left as is and simply
 *    copied from input to output buffer provided the mode of operation is ECB
 *    or CBC. For CTR mode, decryption actually consists in a simple XOR, thus
 *    residual bytes are decrypted as any other bytes of the message.
 *
 *    This function does not manage any padding. If the plaintext message was
 *    padded by the client, it is also in charge of removing it after the return
 *    of this function call.
 *
 *    If the caller provides the same input and output buffer (pxInput=pxOutput),
 *    the function must perform an in-place decryption.
 *
 *  @pre
 *    The content key has been set through TSecSetClearTextKey(),
 *    TSecSet2LevelProtectedKey(), TSecUseCertKey() or TSecUseFlashProtKey().
 *
 *  @param[in]  xSession
 *    Handle of the session
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It does not
 *    need to be a multiple of the cipher block size.
 *
 *  @param[in] pxInitVector
 *    Buffer allocated by the caller containing the initialization vector (IV)
 *    required by CBC and CTR modes. These cipher modes of operation are part of
 *    the EMI definition. If no IV is required (ECB mode), pxInitVector is set
 *    to NULL.\n
 *    Normally the CTR mode requires a nonce and a counter that are then
 *    combined to produce the actual counter block encryption. In the current
 *    API the caller is in charge of providing the initial counter block instead
 *    of a nonce and a counter. This initial counter block is passed within
 *    pxInitVector buffer. The function TSecSessionDecrypt() is then in charge of
 *    incrementing the counter block of one unit for each block decryption.
 *
 *    The caller is in charge of providing a correct and consistent IV, even
 *    in case the data to be encrypted requires several calls to this function.
 *
 *  @param[in] xInitVectorSize
 *    Size in bytes of the initialization vector. It is equal to the cipher
 *    block size (8 bytes for TDES and 16 bytes for AES). If no IV is
 *    required (ECB mode), xInitVectorSize is set to zero.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown session
 *    - NULL input/output buffers
 *    - Unsupported IV size (not a multiple of cipher block size)
 *
 *  @retval  SEC_ERROR_BAD_USAGE
 *    This error status is returned if xSession refers to an encryption session.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
*/
typedef TSecStatus (*TSecSessionDecrypt)
(
        TSecCipherSession   xSession,
  const TUnsignedInt8*     pxInput,
        TUnsignedInt8*     pxOutput,
        size_t              xMessageSize,
  const TUnsignedInt8*     pxInitVector,
        size_t              xInitVectorSize
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function allocates a buffer that is compatible with hardware
 *    crypto-processor constraints.
 *
 *    The rational for this function is that hardware crypto-processors may
 *    requires special buffers (e.g. contiguous pages in physical memory, limited
 *    address space, pages alignment, etc) to work properly.
 *
 *    Buffers allocated by this function are intended to be used as RAM2RAM
 *    input/output messages and digest input messages.
 *
 *  @param[in] xBufferSize
 *    Size in bytes of the buffer to allocate
 *
 *  @returns
 *    - Pointer to the buffer allocated in case of success
 *    - NULL in case of failure
 *
 *  @warning
 *    The implementation of this function on the TEE side must always allocate
 *    a protected buffer, while the implementation on the REE side must always
 *    allocate an unprotected buffer.
 *
 *  @see
 *    - TSecFreeBuffer
 *    - \ref p_sec_buffer
*/
typedef TUnsignedInt8* (*TSecAllocateBuffer)
(
  size_t  xBufferSize
);


/**
 *  @ingroup g_sec_r2r
 *
 *  @brief
 *    This function frees a buffer allocated by TSecAllocateBuffer().
 *
 *  @param[in] pxBuffer
 *    Pointer to the buffer to free
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully. This status is also returned if
 *    \c pxBuffer=NULL.
 *
 *  @retval  SEC_ERROR
 *    Any other errors
 *
 *  @see
 *    TSecAllocateBuffer
*/
typedef TSecStatus (*TSecFreeBuffer)
(
  TUnsignedInt8*     pxBuffer
);


/**
  @ingroup g_sec_r2r

  @brief
    This function enables the protected buffer policy on a given RAM2RAM session.

    It can only be called from the TEE. Protected buffer policy is
    always disabled on REE side and is disabled by default on TEE side.

    Refer to \ref p_sec_prot_buf for further information about this topic.

  @param[in]  xSession
    Handle of the session

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR_BAD_PARAMETER
    An invalid parameter is provided:
    - NULL or unknown session

  @retval  SEC_ERROR
    Any other errors

  @remarks
    This function is only required from the trusted execution environment.
*/
typedef TSecStatus (*TSecEnableProtectedBuffer)
(
        TSecCipherSession   xSession
);



/* -------------------------------------------------------------------------- */
/*                           STREAM PROCESSING SESSION                        */
/* -------------------------------------------------------------------------- */

/**
  @brief Define the even/odd polarity of a key

  @details
  This polarity is used to handle the synchronization of key change occurring
  every crypto-period on DVB/MPEG2-TS stream.
*/
typedef enum
{
  SEC_KEY_POLARITY_EVEN,
  /**< Even polarity
  */
  SEC_KEY_POLARITY_ODD,
  /**< Odd polarity
  */
  SEC_KEY_POLARITY_NONE,
  /**< Polarity is not applicable
  */
  SEC_KEY_POLARITY_LAST
  /**< Last enum entry
  */
} TSecKeyPolarity;



/**
  @ingroup g_sec_stream_processing
  @brief
    Pointer to an opaque structure to be defined by the entity in charge of
    developing the SEC API and used as a handle for stream processing sessions.
*/
typedef struct SSecStreamSession* TSecStreamSession;

/**
  @ingroup g_sec_stream_processing
  @brief
    Structure defining key parameters.
*/
typedef struct
{
  TSecKeyPolarity   polarity;
  /**<
    Even/odd key polarity. In OTT use cases, this polarity is unused and set to
    ::SEC_KEY_POLARITY_NONE. It may also be set to ::SEC_KEY_POLARITY_NONE for
    some local PVR use cases.
  */
  TBoolean          protectedBuffer;
  /**<
    This parameter enables/disables the usage of secure media path (SMP) on a
    stream en/decryption session.

    Refer to \ref p_sec_prot_buf for further information about this topic.
  */

  TUnsignedInt16    emi;
  /**<
    Identifier of the cryptographic algorithm and block cipher modes to be used
    by the crypto-engine.
  */
} TSecKeyParams;


/**
  @ingroup g_sec_stream_processing
  @brief
    Type defining user intents
*/
typedef enum
{
  SEC_USER_INTENT_WATCH,
  /**< User is watching the content */
  SEC_USER_INTENT_RECORD,
  /**< User is recording the content */
  SEC_USER_INTENT_EXPORT,
  /**< User is exporting the content */
  SEC_USER_INTENT_EXPORT_XCODE,
  /**< User is exporting the content with transcoding ON */
  SEC_LAST_USER_INTENT
  /**< Last enum entry */
} TSecUserIntent;


/**
  @ingroup g_sec_stream_processing
  @brief
    This type defines the format of a chunk processed by the @link
    ISecStreamSession::processOpaqueData processOpaqueData @endlink function.
*/
typedef struct
{
  TUnsignedInt32      clearSize;
  /**< Number of bytes of clear data at the beginning of this chunk.
       May be 0 if no clear bytes exist for this chunk */
  TUnsignedInt32      protSize;
  /**< Number of bytes of protected data following the clear data.
       May be 0 if no protected bytes exist for this chunk. clearSize and
       protSize must not be both 0 and clearSize + protSize must equal the
       chunk size. */
  TUnsignedInt8*      initVector;
  /**< Initialization vector needed for the decryption */
  TUnsignedInt32      initVectorSize;
  /**< Size in bytes of the initialization vector. The size must be the same
       for all chunks of a chain. */
} TSecChunkInfo;


/**
  @ingroup g_sec_stream_processing
  @brief
    This type defines the format of the default opaque input buffer used by the @link
    ISecStreamSession::processOpaqueData processOpaqueData @endlink
    function. This format is compliant with all OTT common encryption scheme (e.g.
    'cenc', 'cbc1', 'cens', 'cbcs').
  @see \ref p_sec_stream_opaque_buffers
*/
typedef struct
{
  TUnsignedInt32       version;
  /**< Version of the structure. **IT MUST BE SET TO 1**. It is used by the driver
       to identify the opaque structure passed in as parameter of @link
       ISecStreamSession::processOpaqueData processOpaqueData @endlink. */
  TUnsignedInt8*       data;
  /**< Buffer containing the data to be processed  */
  TUnsignedInt32       size;
  /**< Size in bytes of data to be processed  */
  TSecChunkInfo*       chunks;
  /**< Array of chunk info entries  */
  TUnsignedInt32       chunkCount;
  /**< Number of chunk info entries  */
  TUnsignedInt32       skipBlocks;
  /**< Number of unencrypted 16-byte cipher blocks in a protection pattern.
       Must be zero when pattern encryption is unused. */
  TUnsignedInt32       cryptBlocks;
  /**< Number of encrypted 16-byte cipher blocks in a protection pattern.
       Must be zero when pattern encryption is unused. */
}TSecOpaqueInputBufferV1;


/**
  @ingroup g_sec_stream_processing
  @brief
    This type defines the format of the default opaque output buffer used by the @link
    ISecStreamSession::processOpaqueData processOpaqueData @endlink
    function.
*/
typedef struct
{
  TUnsignedInt8*    data;
  /**< Address of the buffer containing the data processed */
  TUnsignedInt32    size;
  /**< Size in bytes of the buffer containing the data processed */
} TSecOpaqueOutputBuffer;


/**
  @ingroup g_sec_stream_processing
  @brief
    This structure defines the stream processing interface

  @see
  - @ref p_sec_stream_processing "Stream processing"

*/
typedef struct
{
  TSecStatus (*open)
  (
    TSecStreamSession*     pxSession,
    TTransportSessionId     xTransportSessionId,
    TUnsignedInt16          xEmi
  );
  /**<
    @brief
      This function opens a stream session for en/decryption operations.

      It is possible to change the en/decryption key during the same session. The
      protection of the key (clear-text, key ladder, CERT) may also change during
      a session.

    @param[out]  pxSession
      Session handle assigned by this function

    @param[in]  xTransportSessionId
      Transport session ID associated to the streams to be en/decrypted. It is
      usually defined by the player. This identifier must be globally unique
      among all sessions of the same type (i.e. encryption type or decryption type).
      Refer to \ref p_sec_tsid for further information.

    @param[in]  xEmi
      Identifier of the cryptographic algorithm and block cipher modes to be used
      by the crypto-engine.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      This status is returned if:
      - pxSession is NULL
      - xTransportSessionId is invalid (#TRANSPORT_SESSION_ID_INVALID)
      - The value of xTransportSessionId is already used in another session of the
        same type (encryption or decryption).

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter is not supported.

    @retval  SEC_ERROR
      Any other errors

    @warning
      - Opening several decryption sessions with the same transport
        session ID, or several encryption sessions with the same transport
        session ID is not allowed.
      - Opening a decryption and an encryption session with the same transport
        session ID must be supported.
  */

  TSecStatus (*close)
  (
    TSecStreamSession     xSession
  );
  /**<
    @brief
      This function closes a stream session and frees allocated resources.

    @param[out]  pxSession
      Handle of the session to be closed

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      This status is returned if xSession is NULL or does not correspond to a
      valid session.

    @retval  SEC_ERROR
      Any other errors

    @warning
      - This function must <b>wipe</b> and free key slots associated to this session.
  */

  TSecStatus (*getKeySlot)
  (
          TSecStreamSession   xSession,
          size_t              xKeyIdSize,
    const TUnsignedInt8*     pxKeyId,
          TUnsignedInt16*    pxKeySlotId
  );
  /**<
    @brief
      This function gets a key table slot identifier.

      In an MPEG-DVB use case, pxKeyId is NULL and this function returns the single
      key slot allocated to this session.

      In OTT, two slots are available per session in order to support key rotation.
      When this function is called with a different key ID on the same session,
      it toggles between the two slots allocated and returns the resulting key
      slot identifier. When it is called several times in a raw with the same key ID
      on the same session, it returns the same key slot identifier. This behavior
      is illustrated as follows:
      @verbatim
      getKeySlot(Session_1, KeyId_1) -> KeySlotId_0
      getKeySlot(Session_1, KeyId_2) -> KeySlotId_1
      getKeySlot(Session_1, KeyId_3) -> KeySlotId_0
      getKeySlot(Session_1, KeyId_3) -> KeySlotId_0
      getKeySlot(Session_1, KeyId_4) -> KeySlotId_1
      @endverbatim

    @param[in]  pxSession
      Handle of the session

    @param[in]  xKeyIdSize
      Size in bytes of the key ID (pxKeyId). If no key ID is required (MPEG-DVB
      use case), xKeyIdSize is set to 0.

    @param[in]  pxKeyId
      Key ID. This parameter is required in OTT use cases (e.g. DASH, HLS).
      If no key ID is required (MPEG-DVB use case), pxKeyId is set to NULL.

    @param[out]  pxKeySlotId
      Identifier of the key slot returned

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - NULL or unknown session
      - NULL key slot ID pointer
      - Key ID parameters inconsistency (e.g. xKeyIdSize!=0 and pxKeyId=NULL,
        and vice-versa)

    @retval  SEC_ERROR
      Any other errors

    @warning
      - Key slots must be wiped when the session is closed (see ISecStreamSession::close)

    @see
      \ref p_sec_stream_processing_overview "Stream processing overview"
*/

  TSecStatus (*setClearTextKey)
  (
          TUnsignedInt16      xKeySlotId,
    const TSecKeyParams*     pxKeyParams,
          size_t              xClearTextKeySize,
    const TUnsignedInt8*     pxClearTextKey
  );
  /**<
    @brief
      This function sets a clear-text key in a key table slot for stream
      en/decryption operations.

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxKeyParams
      Key parameters

    @param[in]  xClearTextKeySize
      Size in bytes of the clear-text key:
      - AES: 16 bytes
      - DVB-CSA2: 8 bytes
      - DVB-CSA3: 16 bytes
      - DES: 8 bytes

    @param[in]  pxClearTextKey
      Buffer containing the clear-text key. The buffer is allocated by the
      caller.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - Unknown key slot ID
      - NULL key parameter pointer
      - NULL clear-text key pointer
      - Size of key does not match the crypto algorithm defined by the EMI
        (TSecKeyParams::emi)

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter (TSecKeyParams::emi) is not supported.

    @retval  SEC_ERROR
      Any other errors

    @remarks
      This function may be called at any time during a given session to change
      the key.

    @see
      @ref s_sec_bc_clearkey
  */

  TSecStatus (*setCertKey)
  (
          TUnsignedInt16      xKeySlotId,
    const TSecKeyParams*     pxKeyParams
  );
  /**<
    @brief
      This function sets a CERT key in a key table slot for stream
      en/decryption operations.

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxKeyParams
      Key parameters

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - Unknown key slot ID
      - NULL key parameter pointer

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter (TSecKeyParams::emi) is not supported.

    @retval  SEC_ERROR
      Any other errors

    @remarks
      This function may be called at any time during a given session to change
      the key.

    @see
      @ref s_sec_bc_certkey
  */

  TSecStatus (*set2LevelProtectedKey)
  (
          TUnsignedInt16      xKeySlotId,
    const TSecKeyParams*     pxKeyParams,
          size_t              xCipheredContentKeySize,
    const TUnsignedInt8*     pxCipheredContentKey,
          size_t              xCipheredProtectingKeySize,
    const TUnsignedInt8*     pxL1CipheredProtectingKey,
    const TUnsignedInt8*     pxL2CipheredProtectingKey
  );
  /**<
    @brief
      This function unwraps a 2-level protected key and set it in a key table slot
      for stream en/decryption operations.

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxKeyParams
      Key parameters

    @param[in]  xCipheredContentKeySize
      Size in bytes of the ciphered content key:
      - AES: 16 bytes
      - DVB-CSA2: 8 bytes
      - DVB-CSA3: 16 bytes

    @param[in]  pxCipheredContentKey
      Buffer, allocated by the caller, containing the ciphered content key ciphered
      with L1PK.

    @param[in]  xCipheredProtectingKeySize
      Size in bytes of the intermediate level protecting keys (pxL1CipheredProtectingKey
      and pxL2CipheredProtectingKey) used within the key ladder.

    @param[in]  pxL1CipheredProtectingKey
      Buffer, allocated by the caller, containing the ciphered protecting key
      (L1PK) ciphered with L2PK.

    @param[in]  pxL2CipheredProtectingKey
      Buffer, allocated by the caller, containing the ciphered protecting key
      (L2PK) ciphered with a chipset root key.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - Unknown key slot ID
      - NULL key parameter pointer
      - NULL key pointers
      - Size of key does not match the crypto algorithm defined by the EMI
        (TSecKeyParams::emi)

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter (TSecKeyParams::emi) is not supported.

    @retval  SEC_ERROR
      Any other errors

    @remarks
      This function may be called at any time during a given session to change
      the key.

    @see
     @ref s_sec_bc_2kl_prot
  */

  TSecStatus (*processOpaqueData)
  (
          TSecStreamSession   xSession,
          TUnsignedInt16      xKeySlotId,
    const void*              pxOpaqueInput,
          void*              pxOpaqueOutput,
    const TUnsignedInt8*     pxInitVector,
          size_t              xInitVectorSize,
          TBoolean            xLastChunk
  );
  /**<
    @brief
      This function decrypts an @link p_sec_stream_opaque_buffers opaque buffer @endlink
      with the key referenced by @a xKeySlotId. It is used in OTT use cases (e.g. DASH, HLS) only.

      If the data to be decrypted is split in a chain of chunks, this function may be
      called once per chunk. In such a case, the xLastChunk parameter is used to
      indicate the end of the chain.

    @param[in]  xSession
      Handle of the session

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxOpaqueInput
      Reference to the opaque buffer to be decrypted

    @param[out] pxOpaqueOutput
      Reference to the decrypted opaque buffer

    @param[in] pxInitVector
      This parameter is deprecated and should be set to NULL. If an IV is required,
      it is now signaled within the @link TSecOpaqueInputBufferV1 opaque input buffer
      structure @endlink.

    @param[in] xInitVectorSize
      This parameter is deprecated and should be set to 0. If an IV is required,
      it is now signaled within the @link TSecOpaqueInputBufferV1 opaque input buffer
      structure @endlink.

    @param[in]  xLastChunk
      Flag used to indicate the decryption of the last chunk of a chain.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - NULL or unknown session
      - Unknown key slot ID
      - NULL input buffer

    @retval  SEC_ERROR
      Any other errors

    @see @ref p_sec_stream_opaque_buffers "Opaque buffer processing"
  */

  TSecStatus (*processContentUsageRules)
  (
          TUnsignedInt16      xKeySlotId,
    const TUnsignedInt8*     pxUsageRules,
          size_t              xUsageRulesSize
  );
  /**<
    @brief
      This function is in charge of processing and enforcing usage rules for the
      content associated to @a xKeySlotId. This function may be called at any time
      during a stream session (::ISecStreamSession).

      If it is not called for a given key slot, or called with \c pxUsageRules=NULL,
      no usage rules at all are enforced on the related stream.

      Refer to [PRM_UR] specification for the definition of the usage rules data
      format.

    @param[in]  xKeySlotId
      Identifier of the key slot linked to the content for which usage rules must
      be enforced.

    @param[in]  pxUsageRules
      Buffer containing the usage rules to be enforced.

    @param[in]  xUsageRulesSize
      Size in bytes of the usage rules buffer

    @retval  SEC_NO_ERROR
      The operation was completed successfully and rules are compliant.

    @retval  SEC_ERROR_RULES_VIOLATION
      Operation completed with a violation of usage rules

    @retval  SEC_ERROR
      Any other errors

    @see @ref p_sec_stream_usage_rules "Usage rules processing operations"
  */

  TSecStatus (*setEtsi2LevelProtectedKey)
  (
    const TSecEtsiKladConfig* pxKladConfig,
          TUnsignedInt16      xKeySlotId,
    const TSecKeyParams*      pxKeyParams,
          size_t               xCipheredContentKeySize,
    const TUnsignedInt8*      pxCipheredContentKey,
          size_t               xCipheredProtectingKeySize,
    const TUnsignedInt8*      pxL1CipheredProtectingKey,
    const TUnsignedInt8*      pxL2CipheredProtectingKey
  );
  /**<
    @brief
      This function unwraps a content key protected by an ETSI key ladder and set it in
      a key table slot for stream en/decryption operations. Refer  to @ref a_ref "[ETSI]"
      and @ref a_ref "[OMS]" for the specification of the ETSI key ladder.

      The ETSI key ladder shall share the same key table than NOCS and CERT key
      ladders in order to support a change of key protection on the same session.

    @param[in]  pxKladConfig
      Key ladder configuration

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxKeyParams
      Key parameters

    @param[in]  xCipheredContentKeySize
      Size in bytes of the ciphered content key. It depends on the key ladder
      cipher (#TSecKladCipher) and the scrambling algorithm:
      - It is always equal to 16 bytes when the key ladder cipher is AES-128
      - It is equal to 16 bytes when the key ladder cipher is TDES and the
        scrambling algorithm is AES or DVB-CSA3
      - It is equal to 8 bytes when the key ladder cipher is TDES and the
        scrambling algorithm is DES or DVB-CSA2.

    @param[in]  pxCipheredContentKey
      Buffer, allocated by the caller, containing the content key (CW) ciphered with
      K1.

    @param[in]  xCipheredProtectingKeySize
      Size in bytes of the intermediate level protecting keys (pxL1CipheredProtectingKey
      and pxL2CipheredProtectingKey) used as key ladder inputs.

    @param[in]  pxL1CipheredProtectingKey
      Buffer, allocated by the caller, containing the protecting key (K1) ciphered
      with K2.

    @param[in]  pxL2CipheredProtectingKey
      Buffer, allocated by the caller, containing the protecting key (K2) ciphered
      with K3.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - Unknown key slot ID
      - NULL key parameter pointer
      - NULL key ladder config pointer
      - NULL key pointers
      - Size of key does not match the crypto algorithm defined by the EMI
        (TSecKeyParams::emi)
      - Unsupported key ladder configuration (TSecEtsiKladConfig::profile or
        TSecEtsiKladConfig::cipher)

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter (TSecKeyParams::emi) is not supported.

    @retval  SEC_ERROR
      Any other errors

    @remarks
      This function may be called at any time during a given session to change
      the key.

    @see
      @ref s_sec_bc_etsi_kl
  */

  TSecStatus (*setUserIntent)
  (
    TUnsignedInt16      xKeySlotId,
    TSecUserIntent      xUserIntent
  );
  /**<
    @brief
      This function defines the intent of the user associated to a stream decryption
      or encryption session bound to \c xKeySlotId. It tells whether the session is
      used in a watch, record or export context. The intent is not supposed to
      change over a stream session.

      The default user intent is #SEC_USER_INTENT_WATCH. Thus, if this function is
      not called, the SEC implementation must assume this default intent. Otherwise,
      it is assumed that this function is always called before processContentUsageRules()
      is called, although the intent is not changing over a stream session.

    @param[in]  xKeySlotId
      Identifier of the key slot linked to the user intent.

    @param[in]  xUserIntent
      User intent.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR
      Any other errors
  */

  TSecStatus (*setMkl2LevelProtectedKey)
  (
    const TSecKladConfig*    pxKladConfig,
          TUnsignedInt16      xKeySlotId,
    const TSecKeyParams*      pxKeyParams,
          size_t               xCipheredContentKeySize,
    const TUnsignedInt8*      pxCipheredContentKey,
          size_t               xCipheredProtectingKeySize,
    const TUnsignedInt8*      pxL1CipheredProtectingKey,
    const TUnsignedInt8*      pxL2CipheredProtectingKey
  );
  /**<
    @brief
      This function unwraps a MKL 2-level protected key and set it in a key table slot
      for stream en/decryption operations.

      The caller has to configure the key ladder owner, key ladder root key and
      algorithm of all key ladder internal elements.

    @param[in]  pxKladConfig
      Key ladder configuration

    @param[in]  xKeySlotId
      Identifier of the key table slot

    @param[in]  pxKeyParams
      Key parameters

    @param[in]  xCipheredContentKeySize
      Size in bytes of the ciphered content key. It depends on the key ladder
      cipher (#TSecKladCipher) and the scrambling algorithm:
      - It is always equal to 16 bytes when the key ladder cipher is AES-128
      - It is equal to 16 bytes when the key ladder cipher is TDES and the
        scrambling algorithm is AES or DVB-CSA3
      - It is equal to 8 bytes when the key ladder cipher is TDES and the
        scrambling algorithm is DES or DVB-CSA2.

    @param[in]  pxCipheredContentKey
      Buffer, allocated by the caller, containing the content key (CW) ciphered with
      the L1 protecting key.

    @param[in]  xCipheredProtectingKeySize
      Size in bytes of all intermediate level protecting keys (pxL1CipheredProtectingKey
      and pxL2CipheredProtectingKey) used as key ladder inputs.

    @param[in]  pxL1CipheredProtectingKey
      Buffer, allocated by the caller, containing the L1 protecting key ciphered
      with L2 protecting key.

    @param[in]  pxL2CipheredProtectingKey
      Buffer, allocated by the caller, containing the L2 protecting key ciphered
      with the key ladder root key.

    @retval  SEC_NO_ERROR
      The operation was completed successfully.

    @retval  SEC_ERROR_BAD_PARAMETER
      An invalid parameter is provided:
      - Unknown key slot ID
      - NULL key parameter pointer
      - NULL key ladder config pointer
      - NULL key pointers
      - Size of key does not match the crypto algorithm defined by the EMI
        (TSecKeyParams::emi)

    @retval  SEC_ERROR_BAD_EMI
      The EMI given as input parameter (TSecKeyParams::emi) is not supported.

    @retval  SEC_ERROR
      Any other errors

    @remarks
      This function may be called at any time during a given session to change
      the key.

    @see
      @ref s_sec_bc_generic_2kl_prot
  */

  TSecStatus (*enableSecureProcessor)
  (
    TUnsignedInt16      xKeySlotId
  );
  /**<
    @brief
      This function enables the usage of the secure processor for a stream
      en/decryption session. The session is referenced through a key slot
      identifier.

    @param[in]  xKeySlotId
      Key slot identifier

    @retval  SEC_NO_ERROR
      The operation was completed successfully

    @retval  SEC_ERROR
      The operator failed
  */

} ISecStreamSession;



/******************************************************************************/
/*                                                                            */
/*                                    RSA                                     */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    Padding schemes used together with RSA.
*/
typedef enum
{
  SEC_RSA_PKCS1_V1_5_PADDING,
   /**<  @deprecated
    */
  SEC_RSA_OAEP_SHA_1_PADDING,
   /**<  OAEP padding scheme based on SHA-1. No label is provided. It generates
    *    an overhead of 42 bytes.
    */
  SEC_RSA_OAEP_SHA_256_PADDING,
   /**<  OAEP padding scheme based on SHA-256. No label is provided. It generates
    *    an overhead of 66 bytes.
    */
  SEC_RSA_NO_PADDING,
   /**<  No padding.
    */
  SEC_RSA_OAEP_SHA_384_PADDING,
   /**<  @deprecated
    */
  LAST_SEC_RSA_PADDING
   /**<  Internal use only
    */
}TSecRsaPadding;


/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecRsaGenerateKey)
(
  size_t                xKeySize,
  TUnsignedInt32        xE,
  TUnsignedInt8*       pxN,
  TUnsignedInt8*       pxP,
  TUnsignedInt8*       pxQ,
  TUnsignedInt8*       pxDP,
  TUnsignedInt8*       pxDQ,
  TUnsignedInt8*       pxQInv
);



/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    This function computes CRT parameters dP, dQ and qInv from the two primes
 *    p and q and the public exponent e.
 *
 *  @param[in]  xKeySize
 *    RSA key size in bytes
 *
 *  @param[in]  xE
 *    Public RSA exponent e used as input parameter for the computation of CRT
 *    parameters. May be any of the following values: 3, 17, 65537.
 *
 *  @param[in]  pxP
 *    Buffer containing the prime number p used as input parameter for the
 *    computation of CRT parameters. The buffer is allocated by the caller and
 *    is xKeySize/2 bytes long.
 *
 *  @param[in]  pxQ
 *    Buffer containing the prime number q used as input parameter for the
 *    computation of CRT parameters. The buffer is allocated by the caller and
 *    is xKeySize/2 bytes long.
 *
 *  @param[out]  pxDP
 *    Buffer where to store the CRT exponent dP computed. The buffer is
 *    allocated by the caller and is xKeySize/2 bytes long.
 *
 *  @param[out]  pxDQ
 *    Buffer where to store the CRT exponent dQ computed. The buffer is
 *    allocated by the caller and is xKeySize/2 bytes long.
 *
 *  @param[out]  pxQInv
 *    Buffer where to store the CRT coefficient qInv computed. The buffer is
 *    allocated by the caller and is xKeySize/2 bytes long.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Unsupported values of public exponent e
 *    - Unsupported key size
 *    - Missing parameter (NULL pointer)
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @remarks
 *    -# All buffers used for passing input and output parameters are allocated
 *       by the caller. Refer to parameters description above for the size of
 *       each buffer.
 *       \n
 *    -# A big number input parameter (p, q) is provided by the caller through a
 *        byte array buffer in big endian format. If the effective size of the big
 *       number is smaller than the buffer size, it must be padded by the caller
 *       with leading zeros.
 *       \n
 *    -# A big number output parameter (dP, dQ, qInv) is returned by this function
 *       through a byte array buffer in big endian format. If the effective size
 *       of the big number is smaller than the buffer size, this function must
 *       pad the output with leading zeros to match the buffer size.
*/
typedef TSecStatus (*TSecRsaComputeCrtParams)
(
        size_t                xKeySize,
        TUnsignedInt32        xE,
  const TUnsignedInt8*       pxP,
  const TUnsignedInt8*       pxQ,
        TUnsignedInt8*       pxDP,
        TUnsignedInt8*       pxDQ,
        TUnsignedInt8*       pxQInv
);



/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    This function pads a cleartext input message and RSA encrypts the padded
 *    message with the public key.
 *
 *    Padding schemes supported are OAEP and PKCS #1 v1.5. It is also possible
 *    to use this function with the special padding scheme #SEC_RSA_NO_PADDING.
 *    In such a case, the caller must ensure the input message size is equal to
 *    key size.
 *
 *  @param[in]  pxInput
 *    Input message to be encrypted.
 *
 *  @param[in]  xInputSize
 *    Size in bytes of input message to operate on. The maximum size depends on
 *    the key size and the padding scheme:
 *    - #SEC_RSA_OAEP_SHA_1_PADDING: maxSize = xKeySize - 2*20 - 2
 *    - #SEC_RSA_OAEP_SHA_256_PADDING: maxSize = xKeySize - 2*32 - 2
 *    .
 *    If #SEC_RSA_NO_PADDING is used, xInputSize must be equal to xKeySize
 *
 *  @param[out]  pxOutput
 *    Buffer where to write resulting encrypted message. This buffer is always
 *    xKeySize bytes long.
 *
 *  @param[in]  xE
 *    Public RSA exponent e used for the operation. May be any of the following
 *    values: 3, 17, 65537.
 *
 *  @param[in]  pxN
 *    Buffer containing the RSA modulus n used for the operation. Buffer size is
 *    equal to key size.
 *
 *  @param[in]  xKeySize
 *    RSA key size in bytes
 *
 *  @param[in]  xPadding
 *    One of the valid padding types defined in TSecRsaPadding. If xPadding is
 *    equal to #SEC_RSA_NO_PADDING, the caller must ensure the message size is
 *    equal to key size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Size of input message not consistent: either too long if a padding
 *      scheme is specified, or different from key size in case of no padding.
 *    - Unsupported exponent
 *    - Unsupported padding scheme
 *    - Unsupported key size
 *    - Missing parameter (NULL pointer)
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @remarks
 *    -# All buffers are allocated by the caller.
*/
typedef TSecStatus (*TSecRsaPublicEncrypt)
(
  const TUnsignedInt8*       pxInput,
        size_t                xInputSize,
        TUnsignedInt8*       pxOutput,
        TUnsignedInt32        xE,
  const TUnsignedInt8*       pxN,
        size_t                xKeySize,
        TSecRsaPadding        xPadding
);



/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    This function pads a cleartext input message and RSA encrypts the padded
 *    message with the private key.
 *
 *    Padding schemes supported are OAEP and PKCS #1 v1.5. It is also possible
 *    to use this function with the special padding scheme #SEC_RSA_NO_PADDING.
 *    In such a case, the caller must ensure the input message size is equal to
 *    key size.
 *
 *  @param[in]  pxInput
 *    Input message to be encrypted.
 *
 *  @param[in]  xInputSize
 *    Size in bytes of input message to operate on. The maximum size depends on
 *    the key size and the padding scheme:
 *    - #SEC_RSA_OAEP_SHA_1_PADDING: maxSize = xKeySize - 2*20 - 2
 *    - #SEC_RSA_OAEP_SHA_256_PADDING: maxSize = xKeySize - 2*32 - 2
 *    .
 *    If #SEC_RSA_NO_PADDING is used, xInputSize must be equal to xKeySize
 *
 *  @param[out]  pxOutput
 *    Buffer where to write resulting encrypted message. This buffer is always
 *    xKeySize bytes long.
 *
 *  @param[in]  xE
 *    Public RSA exponent e. May be any of the following values: 3, 17, 65537.
 *
 *  @param[in]  pxN
 *    Buffer containing the RSA modulus n used for the operation. Buffer size is
 *    equal to key size.
 *
 *  @param[in]  pxP
 *    Buffer containing the private key prime number p. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxQ
 *    Buffer containing the private key prime number q. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxDP
 *    Buffer containing the private key CRT exponent dP. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxDQ
 *    Buffer containing the private key CRT exponent dQ. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxQInv
 *    Buffer containing the private key CRT coefficient qInv. The buffer is
 *    allocated by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  xKeySize
 *    RSA key size in bytes
 *
 *  @param[in]  xPadding
 *    One of the valid padding types defined in TSecRsaPadding. If xPadding is
 *    equal to #SEC_RSA_NO_PADDING, the caller must ensure the message size is
 *    equal to key size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Size of input message not consistent: either too long if a padding
 *      scheme is specified, or different from key size in case of no padding.
 *    - Unsupported exponent
 *    - Unsupported padding scheme
 *    - Unsupported key size
 *    - Missing parameter (NULL pointer)
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @remarks
 *    -# Although the main purpose of this function is to make private key
 *       operations, the public key (e, n) may be required if the underneath RSA
 *       implementation makes use of the cryptographic blinding technique.
 *       \n
 *    -# All buffers are allocated by the caller.
 *       \n
 *    -# A big number input parameter (n, p, q, dP, dQ, qInv) is provided by the
 *       caller through a byte array buffer in big endian format. If the
 *       effective size of the big number is smaller than the buffer size, it
 *       must be padded by the caller with leading zeros.
*/
typedef TSecStatus (*TSecRsaPrivateEncrypt)
(
  const TUnsignedInt8*       pxInput,
        size_t                xInputSize,
        TUnsignedInt8*       pxOutput,
        TUnsignedInt32        xE,
  const TUnsignedInt8*       pxN,
  const TUnsignedInt8*       pxP,
  const TUnsignedInt8*       pxQ,
  const TUnsignedInt8*       pxDP,
  const TUnsignedInt8*       pxDQ,
  const TUnsignedInt8*       pxQInv,
        size_t                xKeySize,
        TSecRsaPadding        xPadding
);



/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    This function RSA decrypts a ciphered input message with the public key
 *    and removes the padding.
 *
 *    Padding schemes supported are OAEP and PKCS #1 v1.5. It is also possible
 *    to use this function with the special padding scheme #SEC_RSA_NO_PADDING.
 *    In such a case, the output message size is equal to key size.
 *
 *  @param[in]  pxInput
 *    Input message to be decrypted. This buffer is always xKeySize bytes long.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write resulting decrypted message. This buffer is always
 *    xKeySize bytes long although the size of the decrypted message
 *    (pxOutputSize) may be smaller. The output message must start in
 *    pxOutput[0] and end up in pxOutput[*pxOutputSize-1].
 *
 *  @param[out]  pxOutputSize
 *    Size in bytes of output message after decryption and padding removal.
 *
 *  @param[in]  xE
 *    Public RSA exponent e used for the operation. May be any of the following
 *    values: 3, 17, 65537.
 *
 *  @param[in]  pxN
 *    Buffer containing the RSA modulus n used for the operation. Buffer size is
 *    equal to key size.
 *
 *  @param[in]  xKeySize
 *    RSA key size in bytes
 *
 *  @param[in]  xPadding
 *    One of the valid padding types defined in TSecRsaPadding. If xPadding is
 *    equal to #SEC_RSA_NO_PADDING, the caller must ensure the message size is
 *    equal to key size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Unsupported exponent
 *    - Unsupported padding scheme
 *    - Unsupported key size
 *    - Missing parameter (NULL pointer)
 *
 *  @retval  SEC_ERROR_BAD_PADDING
 *    The verification of the padding occurring after the RSA decryption
 *    has failed.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @remarks
 *    -# All buffers are allocated by the caller.
*/
typedef TSecStatus (*TSecRsaPublicDecrypt)
(
  const TUnsignedInt8*       pxInput,
        TUnsignedInt8*       pxOutput,
        size_t*              pxOutputSize,
        TUnsignedInt32        xE,
  const TUnsignedInt8*       pxN,
        size_t                xKeySize,
        TSecRsaPadding        xPadding
);



/**
 *  @ingroup g_sec_rsa
 *
 *  @brief
 *    This function RSA decrypts a ciphered input message with the private key
 *    and removes the padding.
 *
 *    Padding schemes supported are OAEP and PKCS #1 v1.5. It is also possible
 *    to use this function with the special padding scheme #SEC_RSA_NO_PADDING.
 *    In such a case, the output message size is equal to key size.
 *
 *  @param[in]  pxInput
 *    Input message to be decrypted. This buffer is always xKeySize bytes long.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write resulting decrypted message. This buffer is always
 *    xKeySize bytes long although the size of the decrypted message
 *    (pxOutputSize) may be smaller. The output message must start in
 *    pxOutput[0] and end in pxOutput[*pxOutputSize-1].
 *
 *  @param[out]  pxOutputSize
 *    Size in bytes of output message after decryption and padding removal.
 *
 *  @param[in]  xE
 *    Public RSA exponent e. May be any of the following values: 3, 17, 65537.
 *
 *  @param[in]  pxN
 *    Buffer containing the RSA modulus n used for the operation. Buffer size is
 *    equal to key size.
 *
 *  @param[in]  pxP
 *    Buffer containing the private key prime number p. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxQ
 *    Buffer containing the private key prime number q. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxDP
 *    Buffer containing the private key CRT exponent dP. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxDQ
 *    Buffer containing the private key CRT exponent dQ. The buffer is allocated
 *    by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  pxQInv
 *    Buffer containing the private key CRT coefficient qInv. The buffer is
 *    allocated by the caller and is xKeySize/2 bytes long.
 *
 *  @param[in]  xKeySize
 *    RSA key size in bytes
 *
 *  @param[in]  xPadding
 *    One of the valid padding types defined in TSecRsaPadding. If xPadding is
 *    equal to #SEC_RSA_NO_PADDING, the caller must ensure the message size is
 *    equal to key size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Unsupported exponent
 *    - Unsupported padding scheme
 *    - Unsupported key size
 *    - Missing parameter (NULL pointer)
 *
 *  @retval  SEC_ERROR_BAD_PADDING
 *    The verification of the padding occurring after the RSA decryption
 *    has failed.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @remarks
 *    -# Although the main purpose of this function is to make private key
 *       operations, the public key (e, n) may be required if the underneath RSA
 *       implementation makes use of the cryptographic blinding technique.
 *    \n
 *    -# All buffers are allocated by the caller.
 *    \n
 *    -# A big number input parameter (n, p, q, dP, dQ, qInv) is provided by the
 *       caller through a byte array buffer in big endian format. If the
 *       effective size of the big number is smaller than the buffer size, it
 *       must be padded by the caller with leading zeros.
*/
typedef TSecStatus (*TSecRsaPrivateDecrypt)
(
  const TUnsignedInt8*       pxInput,
        TUnsignedInt8*       pxOutput,
        size_t*              pxOutputSize,
        TUnsignedInt32        xE,
  const TUnsignedInt8*       pxN,
  const TUnsignedInt8*       pxP,
  const TUnsignedInt8*       pxQ,
  const TUnsignedInt8*       pxDP,
  const TUnsignedInt8*       pxDQ,
  const TUnsignedInt8*       pxQInv,
        size_t                xKeySize,
        TSecRsaPadding        xPadding
);



/******************************************************************************/
/*                                                                            */
/*                               DIFFIE-HELLMAN                               */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_dh
 *
 *  @brief
 *    Generate Diffie-Hellman public/private key pair from g and p parameters.
 *    The public key is equal to g^x mod p, where x is random number considered
 *    as the private key. If the private key is provided as input, only the
 *    public key is generated.
 *
 *  @param[in]  pxG
 *    Buffer containing the DH generator g used for the operation. The buffer is
 *    allocated by the caller and is xKeySize bytes long. The caller ensures it
 *    is padded with leading zeros if the effective size of g is smaller than
 *    the buffer size.
 *
 *  @param[in]  pxP
 *    Buffer containing the DH prime modulus p used for the operation. The
 *    buffer is allocated by the caller and is xKeySize bytes long.
 *
 *  @param[in]  pxInputPrivKey
 *    Buffer containing an optional input private key from which the public has
 *    to be generated. The buffer is allocated by the caller and is xKeySize
 *    bytes long. The caller ensures it is padded with leading zeros if the
 *    effective size of this private key is smaller than the buffer size.
 *    If no private key is provided as input (\c pxInputPrivKey=NULL), this
 *    function generates a random private key and stores it in pxOutputPrivKey
 *    buffer.
 *
 *  @param[out]  pxOutputPrivKey
 *    Buffer where to write the generated private key, in case no private key is
 *    provided as input (\c pxInputPrivKey==NULL). The buffer is allocated by the
 *    caller and is xKeySize bytes long. It must be padded with leading zeros
 *    if the effective size of the private key is smaller than the buffer size.
 *    This parameter could be set to NULL if a private key is provided as input
 *    (\c pxInputPrivKey<>NULL).
 *
 *  @param[out]  pxPubKey
 *    Buffer where to write the generated public key. The buffer is allocated by
 *    the caller and is xKeySize bytes long. It must be padded with leading
 *    zeros if the effective size of the public key is smaller than the buffer
 *    size.
 *
 *  @param[in]  xKeySize
 *    DH key size in bytes
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - A mandatory parameter (pxG, pxP or pxPubKey) is missing (NULL pointer)
 *    - Both pxInputPrivKey and pxOutputPrivKey are NULL
 *    - Unsupported key size
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecDhGenerateKey)
(
  const TUnsignedInt8*   pxG,
  const TUnsignedInt8*   pxP,
  const TUnsignedInt8*   pxInputPrivKey,
        TUnsignedInt8*   pxOutputPrivKey,
        TUnsignedInt8*   pxPubKey,
        size_t            xKeySize
);



/**
 *  @ingroup g_sec_dh
 *
 *  @brief
 *    Compute Diffie-Hellman shared secret as otherPubKey^privKey mod p.
 *
 *  @param[in]  pxP
 *    Buffer containing the DH prime modulus p used for the operation. The
 *    buffer is allocated by the caller and is xKeySize bytes long.
 *
 *  @param[in]  pxPrivKey
 *    Buffer containing the DH private key. The buffer is allocated by the
 *    caller and is xKeySize bytes long. The caller ensures it is padded with
 *    leading zeros if the effective size of this key is smaller than the buffer
 *    size.
 *
 *  @param[in]  pxOtherPubKey
 *    Buffer containing the DH public key of the other peer. The buffer is
 *    allocated by the caller and is xKeySize bytes long. The caller ensures it
 *    is padded with leading zeros if the effective size of this key is smaller
 *    than the buffer size.
 *
 *  @param[out]  pxSharedSecret
 *    Buffer where to write the computed shared secret. The buffer is allocated
 *    by the caller and is xKeySize bytes long. It must padded with leading
 *    zeros if the effective size of this key is smaller than the buffer size.
 *
 *  @param[in]  xKeySize
 *    DH key size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    - Missing parameter (NULL pointer)
 *    - Unsupported key size
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecDhComputeKey)
(
  const TUnsignedInt8*   pxP,
  const TUnsignedInt8*   pxPrivKey,
  const TUnsignedInt8*   pxOtherPubKey,
        TUnsignedInt8*   pxSharedSecret,
        size_t            xKeySize
);



/******************************************************************************/
/*                                                                            */
/*                                    SHA-1                                   */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_sha
 *
 *  @brief
 *    Type used to link a sequence of operations together in a given context
 *    (e.g. SHA init, update and final). It is defined as a pointer to an opaque
 *    structure to be defined by the entity in charge of implementing the SEC
 *    API.
*/
typedef struct SSecHashContext* TSecHashContext;


/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha1Init)
(
  TSecHashContext*    pxContext
);



/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha1Update)
(
        TSecHashContext  xContext,
  const TUnsignedInt8*  pxMessageChunk,
        size_t           xChunkSize
);



/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha1Final)
(
  TSecHashContext  xContext,
  TUnsignedInt8*  pxMessageDigest
);



/******************************************************************************/
/*                                                                            */
/*                                    SHA-256                                 */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_sha256
 *
 *  @brief
 *    Initializes a staged SHA-256 message digesting.
 *
 *  @param[out]  pxContext
 *    Context assigned to this SHA-256 message digesting. It is passed back to
 *    TSecSha256Update() and TSecSha256Final().
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL context address (\c pxContext=NULL)
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @note
 *    - The driver must be able to process several digest contexts in parallel.
*/
typedef TSecStatus (*TSecSha256Init)
(
  TSecHashContext*    pxContext
);



/**
 *  @ingroup g_sec_sha256
 *
 *  @brief
 *    This function processes the next chunk of the message to be hashed.
 *
 *  @param[in]  xContext
 *    Message hashing context
 *
 *  @param[in]  pxMessageChunk
 *    Buffer containing the message chunk to be processed. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[in]  xChunkSize
 *    Size in bytes of the message chunk
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown context
 *    - NULL message chunk pointer (\c pxMessageChunk=NULL) with an non-null
 *      chunk size (\c xChunkSize>0). Note that an null chunk size is not
 *      considered as an error, whatever the value of \c pxMessageChunk.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @note
 *    - This function must be able to process an empty message chunk
 *      (\c xChunkSize=0).
*/
typedef TSecStatus (*TSecSha256Update)
(
        TSecHashContext  xContext,
  const TUnsignedInt8*  pxMessageChunk,
        size_t           xChunkSize
);



/**
 *  @ingroup g_sec_sha256
 *
 *  @brief
 *    Finalizes SHA stages and returns the message digest.
 *
 *  @param[in]  xContext
 *    Message hashing context
 *
 *  @param[out]  pxMessageDigest
 *    Buffer where to write the resulting message digest. The buffer is
 *    allocated by the caller and is 32 bytes (256 bits) long.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown context
 *    - NULL message digest pointer (\c pxMessageDigest=NULL).
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @warning
 *    - This function must free the context whenever a valid \c xContext
 *      parameter is provided. This must be done independently on whether  \c
 *      pxMessageDigest parameter is valid or not, or whether this function is
 *      able to finalize the digest or not. Otherwise, there is a risk that
 *      resources are never deallocated. For instance, if the caller provides a
 *      valid context with \c pxMessageDigest=NULL, the function must free the
 *      context and return #SEC_ERROR_BAD_PARAMETER.
*/
typedef TSecStatus (*TSecSha256Final)
(
  TSecHashContext  xContext,
  TUnsignedInt8*  pxMessageDigest
);


/******************************************************************************/
/*                                                                            */
/*                                    SHA-384                                 */
/*                                                                            */
/******************************************************************************/


/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha384Init)
(
  TSecHashContext*    pxContext
);



/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha384Update)
(
        TSecHashContext  xContext,
  const TUnsignedInt8*  pxMessageChunk,
        size_t           xChunkSize
);



/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecSha384Final)
(
  TSecHashContext  xContext,
  TUnsignedInt8*  pxMessageDigest
);


/******************************************************************************/
/*                                                                            */
/*                                    HMAC                                    */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_hmacsha256
 *
 *  @brief
 *    Initializes a staged HMAC-SHA-256 message authentication digesting.
 *
 *  @param[in]  pxKey
 *    HMAC key used for the operation
 *
 *  @param[in]  xKeySize
 *    Size in bytes of the HMAC key. The maximum key size is equal to the
 *    SHA-256 block size, i.e. 64 bytes. If a longer key has to be used, the
 *    caller is in charge of hashing it beforehand.
 *
 *  @param[out]  pxContext
 *    Context assigned to this HMAC-SHA-256 message digesting. It is passed back
 *    to TSecHmacSha256Update() and TSecHmacSha256Final().
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL key (\c pxKeySize=NULL) with a non-null key size (xKeySize>0). Note
 *      that an null key size is not considered as an error, whatever the value
 *      of \c pxKey.
 *    - Invalid key size (xKeySize>64)
 *    - NULL context (\c pxContext=NULL)
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @note
 *    - The driver must be able to process several digest contexts in parallel.
 *    - This function must be able to process a digest with an empty key
 *      (\c xKeySize=0).
*/
typedef TSecStatus (*TSecHmacSha256Init)
(
  const TUnsignedInt8*      pxKey,
        size_t               xKeySize,
        TSecHashContext*    pxContext
);


/**
 *  @ingroup g_sec_hmacsha256
 *
 *  @brief
 *    Updates the staged HMAC-SHA-256 authentication digesting with the next
 *    message chunk.
 *
 *  @param[in]  xContext
 *    Message hashing context
 *
 *  @param[in]  pxMessageChunk
 *    Buffer containing the message chunk to be processed. If it is not allocated by
 *    TSecAllocateBuffer, it is the driver responsibility to reallocate and copy
 *    the data in a temporary buffer compatible with RAM2RAM crypto-processor
 *    constraints.
 *
 *  @param[in]  xChunkSize
 *    Size in bytes of the message chunk
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown context
 *    - NULL message chunk pointer (\c pxMessageChunk=NULL) with an non-null
 *      chunk size (\c xChunkSize>0). Note that an null chunk size is not
 *      considered as an error, whatever the value of \c pxMessageChunk.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @note
 *    - This function must be able to process an empty message chunk
 *      (\c xChunkSize=0).
*/
typedef TSecStatus (*TSecHmacSha256Update)
(
        TSecHashContext  xContext,
  const TUnsignedInt8*  pxMessageChunk,
        size_t           xChunkSize
);


/**
 *  @ingroup g_sec_hmacsha256
 *
 *  @brief
 *    Finalizes HMAC-SHA-256 stages and returns the message digest.
 *
 *  @param[in]  xContext
 *    Message hashing context
 *
 *  @param[out]  pxMessageDigest
 *    Buffer where to write the resulting message digest. The buffer is
 *    allocated by the caller and is 32 bytes (256 bits) long.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    An invalid parameter is provided:
 *    - NULL or unknown context
 *    - NULL message digest pointer (\c pxMessageDigest=NULL).
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 *
 *  @warning
 *    - This function must free the context whenever a valid \c xContext
 *      parameter is provided. This must be done independently on whether  \c
 *      pxMessageDigest parameter is valid or not, or whether this function is
 *      able to finalize the digest or not. Otherwise, there is a risk that
 *      resources are never deallocated. For instance, if the caller provides a
 *      valid context with \c pxMessageDigest=NULL, the function must free the
 *      context and return #SEC_ERROR_BAD_PARAMETER.
*/
typedef TSecStatus (*TSecHmacSha256Final)
(
  TSecHashContext  xContext,
  TUnsignedInt8*  pxMessageDigest
);


/******************************************************************************/
/*                                                                            */
/*                                  RANDOM                                    */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_rand
 *
 *  @brief
 *    Generate a sequence of true random bytes and store it in specified buffer.
 *
 *    The random number shall be non-deterministic and, at a minimum, comply with
 *    random number generator tests as specified in FIPS 140-2, Security
 *    Requirements for Cryptographic Modules, section 4.9.1.
 *
 *  @param[in]  xNumOfBytes
 *    Number of random bytes to be generated. The maximum number is limited to
 *    1024 bytes.
 *
 *  @param[out]  pxRandomBytes
 *    Buffer where to write the xNumOfBytes generated random bytes.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Missing or malformed parameters provided resulting in a premature
 *    termination of the operation.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and no random data could be generated.
*/
typedef TSecStatus (*TSecGenerateRandomBytes)
(
  size_t           xNumOfBytes,
  TUnsignedInt8*  pxRandomBytes
);


/******************************************************************************/
/*                                                                            */
/*                                    ECDSA                                   */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_ecdsa g_sec_ecdh
 *
 *  @brief
 *    Elliptic curve domain parameters. Refer to \ref p_sec_ec for further
 *    information about the curve to be supported.
 *
 *    All these domain parameters are considered as big numbers and are provided
 *    through SEC client's buffers being keySize bytes long. The SEC client is
 *    in charge padding each buffer with leading zeros if the effective size of
 *    the domain parameter conveyed is smaller than keySize.
*/
typedef struct
{
  TUnsignedInt8* q;
  /**<  Finite field: equal to p in case of prime field curves or equal
   *    to 2^n in case of binary field curves.
  */
  TUnsignedInt8* a;
  /**<  Curve parameter a (q-3 in Suite B)
  */
  TUnsignedInt8* b;
  /**<  Curve parameter b
  */
  TUnsignedInt8* GX;
  /**<  X coordinates of G which is a base point on the curve
  */
  TUnsignedInt8* GY;
  /**<  Y coordinates of G which is a base point on the curve
  */
  TUnsignedInt8* n;
  /**<  Prime which is the order of G point
  */
  TUnsignedInt8* h;
  /**<  Cofactor, which is the order of the elliptic curve divided by the order
   *    of the point G. For the Suite B curves, h = 1.
  */
  size_t         keySize;
  /**<  Key size in bytes. It corresponds to the size in bytes of the prime n
   *    and is equal to:
   *    - P-256: 32 bytes
  */
} TSecEcParams;


/**
  @deprecated This function no longer needs to be implemented
*/
typedef TSecStatus (*TSecEcdsaGenerateKey)
(
        TSecEcParams     xParams,
  const TUnsignedInt8*  pxInputPrivKey,
        TUnsignedInt8*  pxOutputPrivKey,
        TUnsignedInt8*  pxPubKeyX,
        TUnsignedInt8*  pxPubKeyY
);



/**
 *  @ingroup g_sec_ecdsa
 *
 *  @brief
 *    Sign a message with ECDSA algorithm and returns an uncompressed signature
 *    made of the pair (r, s).
 *
 *  @param[in]  xParams
 *    Elliptic curve domain parameters. They are considered as big numbers and
 *    are provided through buffers allocated by the caller and being
 *    xParams.keySize bytes long. The caller is in charge padding each buffer
 *    with leading zeros if the effective size of the domain parameter conveyed
 *    is smaller than xParams.keySize.
 *
 *  @param[in]  xHashType
 *    One of the hash algorithm defined in TSecHashType
 *
 *  @param[in]  pxPrivKey
 *    Buffer containing the private key used for signing the message. The buffer
 *    is allocated by the caller and is xParams.keySize bytes long. The caller
 *    ensures it is padded with leading zeros if the effective size of this
 *    private key is smaller than the buffer size.
 *
 *  @param[in]  pxMessage
 *    Message to be signed.
 *
 *  @param[in]  pxMessageSize
 *    Size in bytes of message to be signed.
 *
 *  @param[out]  pxSigR
 *    Buffer where to write the uncompressed r component of the resulting
 *    signature. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It must be padded with leading zeros if the effective size of
 *    r is smaller than the buffer size.
 *
 *  @param[out]  pxSigS
 *    Buffer where to write the uncompressed s component of the resulting
 *    signature. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It must be padded with leading zeros if the effective size of
 *    s is smaller than the buffer size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Missing or malformed parameters provided resulting in a premature
 *    termination of the operation.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecEcdsaSign)
(
        TSecEcParams     xParams,
        TSecHashType     xHashType,
  const TUnsignedInt8*  pxPrivKey,
  const TUnsignedInt8*  pxMessage,
        size_t           xMessageSize,
        TUnsignedInt8*  pxSigR,
        TUnsignedInt8*  pxSigS
);



/**
 *  @ingroup g_sec_ecdsa
 *
 *  @brief
 *    Verify an ECDSA signature made of an uncompressed pair (r, s).
 *
 *  @param[in]  xParams
 *    Elliptic curve domain parameters. They are considered as big numbers and
 *    are provided through buffers allocated by the caller and being
 *    xParams.keySize bytes long. The caller is in charge padding each buffer
 *    with leading zeros if the effective size of the domain parameter conveyed
 *    is smaller than xParams.keySize.
 *
 *  @param[in]  xHashType
 *    One of the hash algorithm defined in TSecHashType
 *
 *  @param[in]  pxPubKeyX
 *    Buffer containing the public key X coordinates used for verifying the
 *    signature. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It is padded by the caller with leading zeros if the effective
 *    size of this public key coordinates is smaller than the buffer size.
 *
 *  @param[in]  pxPubKeyY
 *    Buffer containing the public key Y coordinates used for verifying the
 *    signature. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It is padded by the caller with leading zeros if the effective
 *    size of this public key coordinates is smaller than the buffer size.
 *
 *  @param[in]  pxMessage
 *    Signed message
 *
 *  @param[in]  pxMessageSize
 *    Size in bytes of signed message.
 *
 *  @param[in]  pxSigR
 *    Buffer containing the uncompressed r component of the signature to be
 *    verified. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It is padded by the caller with leading zeros if the effective
 *    size of r is smaller than the buffer size.
 *
 *  @param[in]  pxSigS
 *    Buffer containing the uncompressed s component of the signature to be
 *    verified. The buffer is allocated by the caller and is xParams.keySize
 *    bytes long. It is padded by the caller with leading zeros if the effective
 *    size of s is smaller than the buffer size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed and the verification of the signature is OK.
 *
 *  @retval  SEC_ERROR_BAD_SIGNATURE
 *    The operation was completed but the verification of the signature failed.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Missing or malformed parameters provided resulting in a premature
 *    termination of the operation.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecEcdsaVerify)
(
        TSecEcParams     xParams,
        TSecHashType     xHashType,
  const TUnsignedInt8*  pxPubKeyX,
  const TUnsignedInt8*  pxPubKeyY,
  const TUnsignedInt8*  pxMessage,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxSigR,
  const TUnsignedInt8*  pxSigS
);



/******************************************************************************/
/*                                                                            */
/*                                    ECDH                                   */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_sec_ecdh
 *
 *  @brief
 *    Generate ECDH public/private key pair.
 *
 *    A private key may be provided as input, in which case only the public is
 *    generated by this function.
 *
 *  @param[in]  xParams
 *    Elliptic curve domain parameters. They are considered as big numbers and
 *    are provided through buffers allocated by the caller and being
 *    xParams.keySize bytes long. The caller is in charge padding each buffer
 *    with leading zeros if the effective size of the domain parameter conveyed
 *    is smaller than xParams.keySize.
 *
 *  @param[in]  pxInputPrivKey
 *    Buffer containing an optional input private key from which the public has
 *    to be generated. The buffer is allocated by the caller and is
 *    xParams.keySize bytes long. It is padded by the caller with leading zeros
 *    if the effective size of this private key is smaller than the buffer size.
 *    If no private key is provided as input (pxInputPrivKey=NULL), this
 *    function generates a random private key and stores it in pxOutputPrivKey
 *    buffer.
 *
 *  @param[out]  pxOutputPrivKey
 *    Buffer where to write the generated private key, in case no private key is
 *    provided as input (pxInputPrivKey==NULL). The buffer is allocated by the
 *    caller and is xParams.keySize bytes long. It must be padded with leading
 *    zeros if the effective size of the private key is smaller than the buffer
 *    size.
 *
 *  @param[out]  pxPubKeyX
 *    Buffer where to write the X coordinates of the generated public key. The
 *    buffer is allocated by the caller and is xParams.keySize bytes long. It
 *    must be padded with leading zeros if the effective size of the public key
 *    X coordinates is smaller than the buffer size.
 *
 *  @param[out]  pxPubKeyY
 *    Buffer where to write the Y coordinates of the generated public key. The
 *    buffer is allocated by the caller and is xParams.keySize bytes long. It
 *    must be padded with leading zeros if the effective size of the public key
 *    Y coordinates is smaller than the buffer size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Missing or malformed parameters provided resulting in a premature
 *    termination of the operation.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecEcdhGenerateKey)
(
        TSecEcParams     xParams,
  const TUnsignedInt8*  pxInputPrivKey,
        TUnsignedInt8*  pxOutputPrivKey,
        TUnsignedInt8*  pxPubKeyX,
        TUnsignedInt8*  pxPubKeyY
);



/**
 *  @ingroup g_sec_ecdh
 *
 *  @brief
 *    Compute ECDH shared secret key. This key corresponds to the X coordinates
 *    of the computed P point.
 *
 *  @param[in]  xParams
 *    Elliptic curve domain parameters. They are considered as big numbers and
 *    are provided through buffers allocated by the caller and being
 *    xParams.keySize bytes long. The caller is in charge padding each buffer
 *    with leading zeros if the effective size of the domain parameter conveyed
 *    is smaller than xParams.keySize.
 *
 *  @param[in]  pxPrivKey
 *    Buffer containing the ECDH private key. The buffer is allocated by the
 *    caller and is xParams.keySize bytes long. It is padded by the caller with
 *    leading zeros if the effective size of this private key is smaller than
 *    the buffer size.
 *
 *  @param[in]  pxOtherPubKeyX
 *    Buffer containing the X coordinates of other peer's ECDH public key. The
 *    buffer is allocated by the caller and is xParams.keySize bytes long. It is
 *    padded by the caller with leading zeros if the effective size of the
 *    public key X coordinates is smaller than the buffer size.
 *
 *  @param[in]  pxOtherPubKeyY
 *    Buffer containing the Y coordinates of other peer's ECDH public key. The
 *    buffer is allocated by the caller and is xParams.keySize bytes long. It is
 *    padded by the caller with leading zeros if the effective size of the
 *    public key Y coordinates is smaller than the buffer size.
 *
 *  @param[out]  pxSharedSecret
 *    Buffer where to write the shared secret corresponding to the X coordinates
 *    of the computed P point. The buffer is allocated by the caller and is
 *    xParams.keySize bytes long. It must padded with leading zeros if the
 *    effective size of the shared secret is smaller than the buffer size.
 *
 *  @retval  SEC_NO_ERROR
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Missing or malformed parameters provided resulting in a premature
 *    termination of the operation.
 *
 *  @retval  SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
*/
typedef TSecStatus (*TSecEcdhComputeKey)
(
        TSecEcParams     xParams,
  const TUnsignedInt8*  pxPrivKey,
  const TUnsignedInt8*  pxOtherPubKeyX,
  const TUnsignedInt8*  pxOtherPubKeyY,
        TUnsignedInt8*  pxSharedSecret
);



/******************************************************************************/
/*                                                                            */
/*                                 KEYSTORE                                   */
/*                                                                            */
/******************************************************************************/


/**
  @ingroup g_sec_keystore

  @brief
    Unprotect the given keystore through operations based on the TEE SoC hardware
    secrets.

  @details
    This function has two different operation modes (1-step and 2-step modes) that
    depend on the keystore protection function implementation.

    <b>1-step mode</b>: ::TSecUnprotectKeystore expects a protected keystore as input parameter
    After the call completes successfully, pxKeystore contains the unprotected keystore.
    So it is assumed that the operation is done in-place and that the unprotected
    keystore is smaller or equal to the protected keystore.

    <b>2-step mode</b>: ::TSecUnprotectKeystore does not expect any protected keystore
    as input parameter. In order for the caller to provide a pxKeystore
    buffer of the right size, TSecUnprotectKeystore is called a first time with
    a @c NULL pxKeystore parameter and returns successfully with pxOutputSize set
    to the right value. Then the caller allocates a buffer of a suitable size and
    call TSecUnprotectKeystore a second time to actually get the unprotected keystore.

  @param[inout]  pxKeystore
    1-step mode: Buffer containing the keystore to be unprotected. If the
    operation completes successfully, the buffer contain the unprotected keystore.@n
    2-step mode: Buffer where to copy the unprotected keystore.

  @param[in]  xInputSize
    1-step mode: Size in bytes of the keystore to be unprotected.@n
    2-step mode: This parameter is set to 0 on the first call. It is set to the
    output buffer size on the second call.

  @param[out]  pxOutputSize
    Size in bytes of the unprotected keystore.

  @retval  SEC_NO_ERROR
    The operation was completed successfully.

  @retval  SEC_ERROR
    Errors occurred and the operation could not be completed successfully.

  @note
    The protection function is not part of this API. Its definition is left to
    the SoC vendor responsibility.
*/
typedef TSecStatus (*TSecUnprotectKeystore)
(
  TUnsignedInt8*     pxKeystore,
  size_t              xInputSize,
  size_t*            pxOutputSize
);


/******************************************************************************/
/*                                                                            */
/*                               FUNCTION TABLE                               */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_sec_functionTable
 *
 *  @brief
 *    Structure defining the SEC function table.
 */
typedef struct
{
  TUnsignedInt32                secApiVersion;
  /**< Version of the SEC API. Use the macro SECAPI_VERSION_INT to assign the
   *   right value.
   */
  TSecGetNuid                   secGetNuid;
  /**< Get chipset NUID (32 bits)
    */
  TSecGetChipsetRevision        secGetChipsetRevision;
  /**< Get chipset revision
    */
  TSecEncryptData               secEncryptData;
  /**< RAM2RAM encryption
    */
  TSecDecryptData               secDecryptData;
  /**< RAM2RAM decryption
    */
  TSecGenerateRandomBytes       secGenerateRandomBytes;
  /**< Generate a sequence of random bytes
    */
  TSecRsaGenerateKey            secRsaGenerateKey;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecRsaPublicEncrypt          secRsaPublicEncrypt;
  /**< RSA encryption using public key
    */
  TSecRsaPrivateEncrypt         secRsaPrivateEncrypt;
  /**< RSA encryption using private key
    */
  TSecRsaPublicDecrypt          secRsaPublicDecrypt;
  /**< RSA decryption using public key
    */
  TSecRsaPrivateDecrypt         secRsaPrivateDecrypt;
  /**< RSA decryption using private key
    */
  TSecDhGenerateKey             secDhGenerateKey;
  /**< Generate Diffie-Hellman keys
    */
  TSecDhComputeKey              secDhComputeKey;
  /**< Compute Diffie-Hellman shared secret
    */
  TSecSha1Init                  secSha1Init;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecSha1Update                secSha1Update;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecSha1Final                 secSha1Final;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecSha256Init                secSha256Init;
  /**< Initialize SHA-256 message digest
    */
  TSecSha256Update              secSha256Update;
  /**< Update SHA-256 message digest
    */
  TSecSha256Final               secSha256Final;
  /**< Finalize SHA-256 message digest
    */
  TSecHmacSha256Init            secHmacSha256Init;
  /**< Initialize HMAC-SHA-256 message digest
    */
  TSecHmacSha256Update          secHmacSha256Update;
  /**< Update HMAC-SHA-256 message digest
    */
  TSecHmacSha256Final           secHmacSha256Final;
  /**< Finalize HMAC-SHA-256 message digest
    */
  TSecEcdsaGenerateKey          secEcdsaGenerateKey;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecEcdsaSign                 secEcdsaSign;
  /**< Sign a message with ECDSA
    */
  TSecEcdsaVerify               secEcdsaVerify;
  /**< Verify an ECDSA signature
    */
  TSecEcdhGenerateKey           secEcdhGenerateKey;
  /**< Generate ECDH public/private key pair
    */
  TSecEcdhComputeKey           secEcdhComputeKey;
  /**< Compute ECDH shared secret key
    */
  TSecOpenRam2RamEncryptSession secOpenRam2RamEncryptSession;
  /**< Open a RAM2RAM encryption session
    */
  TSecOpenRam2RamDecryptSession secOpenRam2RamDecryptSession;
  /**< Open a RAM2RAM decryption session
    */
  TSecCloseSession              secCloseSession;
  /**< Close a block cipher encryption or decryption session
    */
  TSecSetClearTextKey           secSetClearTextKey;
  /**< Set a clear-text key in a key table
    */
  TSecSet2LevelProtectedKey     secSet2LevelProtectedKey;
  /**< Set protected keys in a 2-level key ladder crypto-engine
    */
  TSecUseCertKey                secUseCertKey;
  /**< Set a CERT key in a key table
    */
  TSecSessionEncrypt            secSessionEncrypt;
  /**< Perform RAM2RAM encryption operations
    */
  TSecSessionDecrypt            secSessionDecrypt;
  /**< Perform RAM2RAM decryption operations
    */
  TSecGetNuid64                 secGetNuid64;
  /**< Get chipset NUID (64 bits)
    */
  TSecGetChipsetExtension       secGetChipsetExtension;
  /**< Get chipset extension
    */
  TSecSha384Init                secSha384Init;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecSha384Update              secSha384Update;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecSha384Final               secSha384Final;
  /**< @deprecated This function no longer needs to be implemented
    */
  TSecRsaComputeCrtParams       secRsaComputeCrtParams;
  /**< Computes RSA CRT parameters dP, dQ and qInv.
    */
  TSecEncryptFlashProtKey       secEncryptFlashProtKey;
  /**< Encrypt the flash protection key with the flash protection root key
    */
  TSecUseFlashProtKey           secUseFlashProtKey;
  /**< Use the flash protection key for RAM2RAM operations
    */
  TSecOpenStreamEncryptSession secOpenStreamEncryptSession;
  /**< Open a stream encryption session
    */
  TSecOpenStreamDecryptSession secOpenStreamDecryptSession;
  /**< Open a stream decryption session
    */
  TSecSet0LevelProtectedKey    secSet0LevelProtectedKey;
  /**< Set protected keys in a 0-level key ladder crypto-engine
    */
  TSecSet1LevelProtectedKey    secSet1LevelProtectedKey;
  /**< Set protected keys in a 1-level key ladder crypto-engine
    */
  TSecSetMetadata              secSetMetadata;
  /**< Associate metadata to a stream encryption/decryption session
    */
  TSecAllocateBuffer           secAllocateBuffer;
  /**< Allocate a cryptoprocessor compliant buffer
    */
  TSecFreeBuffer               secFreeBuffer;
  /**< Free a cryptoprocessor compliant buffer
    */
  TSecUseLegacyKey             secUseLegacyKey;
  /**< Use the legacy secret key
    */
  TSecEnableProtectedBuffer     secEnableProtectedBuffer;
  /**< Enables protected buffer enforcement
    */
  const ISecStreamSession*      secStreamDecryptSession;
  /**< Stream decrypt session interface.
  */
  const ISecStreamSession*      secStreamEncryptSession;
  /**< Stream encrypt session interface.
  */
  TSecEnableTeePrivilegedMode   secEnableTeePrivilegedMode;
  /**< Enables TEE privileged mode
  */
  TSecGetTeePrivilegedMode      secGetTeePrivilegedMode;
  /**< Reads the state of the TEE privileged mode
  */
  TSecSetEtsi2LevelProtectedKey secSetEtsi2LevelProtectedKey;
  /**< Set protected keys in a 2-level ETSI key ladder
    */
  TSecUnprotectKeystore         secUnprotectKeystore;
  /**< Unprotects the keystore
    */
  TSecSetMkl2LevelProtectedKey  secSetMkl2LevelProtectedKey;
  /**< Set protected keys through a generic MKL 2-level key ladder
    */
  TSecGetChipId                 secGetChipId;
  /**< Get chip ID (64 bits)
    */

} TSecFunctionTable;


/**
 *  @ingroup g_sec_functionTable
 *
 *  @brief
 *    This function returns a pointer to the SEC driver function table if
 *    successful, NULL in case of error.
 *
 *  @warning
 *    This function must not allocate memory for the function table because it
 *    will be never freed. The function table is purely static for a given
 *    implementation, thus it is much better and simpler to declare the table as
 *    a static global variable and simply returns its address as depicted in the
 *    sample code below.
 *    \code
  static TSecFunctionTable    gSecFunctionTable =
  {
      SECAPI_VERSION_INT,
      secGetNuid,
      ...
  };

  TSecFunctionTable* secGetFunctionTable(void)
  {
    return &gSecFunctionTable;
  }
 *    \endcode
 *
*/
TSecFunctionTable* secGetFunctionTable
(
  void
);


#ifdef __cplusplus
}
#endif

#endif /* CA_SEC_H */

/* END OF FILE */
