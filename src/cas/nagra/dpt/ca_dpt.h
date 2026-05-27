/**
**  @file ca_dpt.h
**
**  @brief
**   Device Personalization Toolbox (DPT)
**
**  $Id: //CAK/components/dpt/TAGS/DPT_3_8_3/src/com/ca_dpt.h#1 $
**
**  CLASSIFICATION: CONFIDENTIAL
**
*/

#ifndef CA_DPT_H
#define CA_DPT_H

#define DPT_VERSION_MAJOR  3
#define DPT_VERSION_MEDIUM 8
#define DPT_VERSION_MINOR  3

/** @page p_history Changes history
  - **3.8.3 - 23/09/2020**
    - Added macro NASC32_TIER0_NO_SCS to support NOCS 3.2 Profile A
    - Added macro NASC31_TIER0_NO_SCS and redefined NASC31_TIER0 as an alias NASC31_TIER0_NO_SCS
  - **3.8.2 - 14/07/2020**
    - Split dptVerifyStbCaSnPairing() which now only verifies CASN read from CSD/BSD while
      new function dptVerifyStbCaSnPairingCert() verifies CASN read from CERT GPD transaction.
  - **3.8.1 - 23/01/2020**
    - Improved robustness of dptProgramCertSegment() function
  - **3.8.0 - 29/08/2019**
    - Added support of CERT segmentation with dptProgramCertSegment() function
    - Updated dptProcessCertRootLevelKey() to check that SdbgDisable and PersoDone
      SoC bits are always set
    - Fixed the way to generate the NOCS3.2 report checknumber
  - **3.7.5 - 13/08/2019**
    - Updated again CERT3 check number computation to rely on NUIDPerso only
  - **3.7.4 - 02/07/2019**
    - Fixed length bug on dptGetCscDataDescriptor and dptGetCscDataDescriptorByKpp
    - Fixed length bug dptCertVerifyEncryptedPairingData
  - **3.7.3 - 27/02/2019**
    - Added dptGetDeviceManufacturerId() and dptGetDeviceModelId() functions
    - Fixed a bug on CERT3 check number computation
  - **3.7.2 - 18/01/2019**
    - Updated compile options
  - **3.7.1 - 17/01/2019**
    - Updated dptGetCscDataDescriptorByKpp() to remove the useless check on the key offset.
      This is required to be compliant with CSCData V7.x
  - **3.7.0 - 27/11/2018**
    - Updated for NASC 3.2 and NOCS 3.2
  - **3.6.1 - 29/06/2018**
    - Added new macros NASC31_TIER0 for NASC 3.1 Tier 0 and NOCS31A for NOCS 3.1 Profile A
    - dptComputeCscDataCheckNumber() and dptVerifyCscDataCn() are complied and used only if
      NOCS31A is undefined.
    - Use of secGetNuid instead of csdGetNuid in dptNuidCompare(to support NOCS3.1 Profile A case)
    - Implemented new function dptVerifyStbCaSnPairing() to compare STB_CA_SN in the PK
      with the reference STB_CA_SN set in the chipset
    - Remove obsolete functions dptGetStbNuidString() and dptStbNuidToString()
    - In the documentation, adapted the applicability of functions:
      dptGetCertCheckNumber()
      dptCertEncryptPairingData()
      dptCertVerifyEncryptedPairingData()
      dptVerifyStbCaSnPairing()
    - In the documentation, removed SCCT definition and added new NASC3.1 and NASC3.1 Tier 0 description
    - Fix the bug of wrong nuid comparison length in dptNuidCompare (48 bits -> 32 bits)
  - **3.6.0 - 22/06/2018**
    - Added dptGetPreDescramblingL1ProtectingKeyByEmi to test pre-descrambling with
      DVB-CSA2, DVB-CSA3 and AES-128
  - **3.5.2 - 27/10/2017**
    - Implemented new function dptGetCscDataConfiguration to get csc configuration data
    - Remove function dptGetCscdVersionString
    - Remove function dptGetCsadSetIdString
  - **3.5.1 - 8/Sep/2017**
    - Implemented new function dptGetCscdVersionString to get csc data version
    - Implemented new function dptGetCsadSetIdString to get csad set id
    - Fix issue that some functions using cert do not properly unlock cert in case
      of errors
  - **3.5.0 - 20/July/2016**
    - Added new NASC31 marco configuration
    - Added NASC31 function dptNuidCompare to read and compare nuid from sec & cert.
    - Added function dptGetStbNuidString and dptStbNuidToString to retrieve chipset
      nuid in string format.
    - Removed function dptGetNocs3ReportingCrc as it can't be used to fill the
      Device Production Report Version 2.0.6.
    - Removed function dptGetNuidInt

  - **3.4.0 - 25/Mar/2016**
    - Added function dptGetNuidInt to retrieve chipset nuid in uint32 value and crc32.

  - **3.3.1 - 16/Dec/2015**
    - Added SCCT macro switch for CONNECT product.

  - **3.3.0 - 12/May/2015**
    - Added dptCertVerifyEncryptedPairingData to verify the encrypted pairing data.

  - **3.2.0 - 05/Feb/2015**
    - Added dptStbCaSnToString to convert CA_SN passed in parameter
    - Added pre-compilation switch for NASC Version
  .
  - **3.1.3 - 03/Feb/2014**
    - dptGetCscDataDescriptor may return NULL pointer, so check it.
  .
  - **3.1.2 - 10/Jan/2014**
    - Added documentation sections
  .
  - **3.1.1 - 10/Dec/2013**
    - Fixed small bug in dptGetStbCaSnString
  .
  - **3.1.0 - 07/Oct/2013**
    - Added dptGetStbCaSnString
    - Added dptGetPreDescramblingL1ProtectingKey
    - Updated EMI used by dptComputeCscDataCheckNumber
    - Added debug traces
  .
  - **3.0.0 - 23/Sep/2013**
    - Renamed module DPT
    - Added CRC16, CRC32
    - Added dptVerifyCscData
    - Added dptGetNocs3ReportingCrc
    - Added dptArrayToUnsignedIntN, dptUnsignedInt32ToArray, dptUnsignedInt16ToArray
  .
  - **2.0.3 - 17/Jan/2013**
    - Updated iptProcessCertRootLevelKey() and iptGetCuid() to avoid nesting
      calls to certLock
  .
  - **2.0.2 - 11/Dec/2012**
    - Another fix of pairing data encryption procedure.
  .
  - **2.0.1 - 10/Dec/2012**
    - Updated CERT transactions to encrypt pairing data with a CERT key.
  .
  - **2.0.0 - 07/Nov/2012**
    - Added iptCertEncryptPairingData() to encrypt pairing data with a CERT key.
  .
  - **1.0.0 - 11/May/2012**
    - First issue
*/

/******************************************************************************/
/*                                                                            */
/*                             OVERALL DOCUMENTATION                          */
/*                                                                            */
/******************************************************************************/

/** @mainpage Overview
 *  - @subpage p_history
 *  - @subpage p_preface
 *  - @subpage p_depend
 *
 *  <hr>Copyright &copy; 2014 Nagravision. All rights reserved.\n
 *  CH-1033 Cheseaux, Switzerland\n
 *  Tel: +41 21 7320311  Fax: +41 21 7320100\n
 *  http://www.nagra.com
 *
 *  All trademarks and registered trademarks are the property of their respective
 *  owners.
 *
 *  This document is supplied with an understanding that the notice(s) herein or
 *  any other contractual agreement(s) made that instigated the delivery of a
 *  hard copy, electronic copy, facsimile or file transfer of this document are
 *  strictly observed and maintained.
 *
 *  The information contained in this document is subject to change without notice.
 *
 *  <b>Security Policy of Nagravision Kudelski Group</b>\n
 *  Any recipient of this document, without exception, is subject to a
 *  Non-Disclosure Agreement (NDA) and access authorization.
*/

/* -------------------------------------------------------------------------- */

/** @page p_preface Preface
 *  <h2>Objectives</h2>
 *  This document specifies the API of the Device Personalization Toolbox (DPT).
 *
 *  The main purpose of this toolbox is to ease some tasks of the device personalization,
 *  such as pairing key encryption, CSC data integrity check, etc. It also contains
 *  basic utilities. It is delivered in source code format and has to be compiled
 *  by the user.
 *
 *  <hr><h2>Audience</h2>
 *  This document is intended for engineers in charge of the personalization of
 *  Nagra secrets into the device.
 *
*/

/* -------------------------------------------------------------------------- */

/** @page p_depend Dependencies and Compile Options
 *  <h2>Select the NASC Version</h2>
 *  The DPT can be used on devices targeting different NASC levels.
 *  Depending on the NASC level, some function are removed as they rely on new components (e.g. CERT).
 *
 *  The following pre-compilation define shall be set and are mutually exclusive :
 *  - `NASC14`          		(for NASC1.4 devices)
 *  - `NASC15`          		(for NASC1.5 devices)
 *  - `NASC30`          		(for NASC3.0 devices)
 *  - `NASC31`          		(for NASC3.1 devices)
 *  - `NASC31_TIER0_NO_SCS`    	(for NASC3.1 Tier 0 (no SCS) devices)
 *  - `NASC32`          		(for NASC3.2 devices)
 *  - `NASC32_TIER0_NO_SCS`    	(for NASC3.2 Tier 0 (no SCS) devices)
 *
 *  Such definition can be set at compilation (e.g. `gcc -DNASC14`) or statically within
 *  the code by adding a `#define` statement
 *
 *  <b>Note:</B> NOCS1.0 may use a different CSD API. Please refer to the chipset specific
 *  documentation to compute the CSC Data Checknumber.
 *
 *  <h3>In NASC1.4</h3>
 *  If the device is NASC1.4, please define the pre-compile option `NASC14`.

 *  <h3>In NASC1.5</h3>
 *  If the device is NASC1.5, please define the pre-compile option `NASC15`.
 *
 *  <h3>In NASC3.0</h3>
 *  If the device is NASC3.0, please define the pre-compile option `NASC30`.
 *
 *  <h3>In NASC3.1</h3>
 *  If the device is NASC3.1, please define the pre-compile option `NASC31`.
 *
 *  <h3>In NASC3.1 Tier 0</h3>
 *  If the device is NASC3.1 Tier 0 (no SCS), please define the pre-compile option `NASC31_TIER0_NO_SCS`.
 *
 *  <h3>In NASC3.2</h3>
 *  If the device is NASC3.2, please define the pre-compile option `NASC32`.
 *
 *  <h3>In NASC3.2 Tier 0</h3>
 *  If the device is NASC3.2 Tier 0 (no SCS), please define the pre-compile option `NASC32_TIER0_NO_SCS`.
 *
 *  DPT depends on the following API:
 *  - CERT API 1.4.0 or higher for NOCS3.2, CERT API 1.0.2 or higher otherwise
 *  - SEC API 4.1.0 or higher (OS environment)
 *  - BSD API 3.2.1 or higher (bare environment)
 *  - CSD API 3.0.0 or higher
 *  - stdio
 *
 *  <h2>API Dependency</h2>
 *  SEC API and BSD API are mutually exclusive. By default, DPT depends on BSD
 *  API. Define the `USE_SEC` compile switch at compile time to make DPT dependent
 *  on SEC API and CSD API instead of BSD API.
*/


/******************************************************************************/
/*                                                                            */
/*                              GROUPS DEFINITION                             */
/*                                                                            */
/******************************************************************************/

/** @defgroup g_cscd CSC Data */
/** @defgroup g_pk Pairing Data */
/** @defgroup g_report Reporting */
/** @defgroup g_predesc Pre-Descrambling Test */
/** @defgroup g_seg CERT Segmentation */
/** @defgroup g_crc CRC */
/** @defgroup g_misc Miscellaneous */


/******************************************************************************/
/*                                                                            */
/*                           GENERAL INCLUDE FILES                            */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"


/******************************************************************************/
/*                                                                            */
/*                              TYPES & CONSTANTS                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Constant used to select the check number to be computed by the CERT block
*/
#define DPT_CERT_TEST_CHECK_NUMBER 0
#define DPT_CERT_REPORT_CHECK_NUMBER 1


/******************************************************************************/
/*                                                                            */
/*                              PUBLIC FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/


/**
 *  @ingroup g_misc
 *
 *  @brief
 *    Tool to convert a byte array to a value
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxArray
 *    Array to convert
 *
 *  @param[in] xSize
 *    Size in bytes of array
 *
 *  @return
 *    Converted value
 *
*/
TUnsignedInt32 dptArrayToUnsignedIntN
(
  const TUnsignedInt8*  pxArray,
        TUnsignedInt8    xSize
);

/**
 *  @ingroup g_misc
 *
 *  @brief
 *    Tool to convert a 32-bit integer  to a byte array
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] xInteger
 *    32-bit integer to convert
 *
 *  @param[out] pxArray
 *    Buffer containing the converted array
 *
 *  @return
 *    0 if success; -1 otherwise
 *
*/
int dptUnsignedInt32ToArray
(
  TUnsignedInt32   xInteger,
  TUnsignedInt8*  pxArray
);


/**
 *  @ingroup g_misc
 *
 *  @brief
 *    Tool to convert a 16-bit integer  to a byte array
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] xInteger
 *    16-bit integer to convert
 *
 *  @param[out] pxArray
 *    Buffer containing the converted array
 *
 *  @return
 *    0 if success; -1 otherwise
 *
*/
int dptUnsignedInt16ToArray
(
  TUnsignedInt16   xInteger,
  TUnsignedInt8*  pxArray
);


/**
 *  @ingroup g_crc
 *
 *  @brief
 *   This function computes the CRC16-CCITT (0x1021) for a given buffer
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxData
 *    Buffer containing the data to operate on
 *  @param[in] xSize
 *    Size in bytes on the data to operate on
 *  @param[out] pxCrc
 *    2-byte array allocated by the caller, where to store the CRC computed
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
int dptCrc16Ccitt
(
  const TUnsignedInt8* pxData,
        size_t          xSize,
        TUnsignedInt8* pxCrc
);


/**
 *  @ingroup g_crc
 *
 *  @brief
 *   This function computes the CRC32 for a given buffer
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxData
 *    Buffer containing the data to operate on
 *  @param[in] xSize
 *    Size in bytes on the data to operate on
 *  @param[out] pxCrc
 *    4-byte array allocated by the caller, where to store the CRC computed
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
int dptCrc32
(
  const TUnsignedInt8* pxData,
        size_t          xSize,
        TUnsignedInt8* pxCrc
);


/**
 *  @ingroup g_report
 *
 *  @brief
 *    This function sends a transaction to the CERT block to read the 8-byte
 *    Check Number (Test or Report). It uses the global variable gCertCmd to
 *    store transaction input and output data.
 *
 *  @applies
 *    All NASC versions, except NASC 1.4
 *
 *  @param[in] xCheckNumType
 *    Indicate if the function has to return the test check number
 *    (DPT_CERT_TEST_CHECK_NUMBER) or the report check number (DPT_CERT_REPORT_CHECK_NUMBER).
 *
 *  @param[out] pxCertCheckNumber
 *    Buffer allocated by the caller, where to store to the 8-byte CERT Check Number computed
 *    by the CERT block.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
 *  @codesample
 *    This example gets the CERT check number to be returned to Nagra through the
 *    reporting process.
 *  \code
 *  TUnsignedInt8 checkNumber[8];
 *
 *  if(!dptGetCertCheckNumber(DPT_CERT_REPORT_CHECK_NUMBER, checkNumber))
 *  {
 *    // Operation is successful
 *  }
 *  \endcode
*/
int dptGetCertCheckNumber
(
  TUnsignedInt8   xCheckNumType,
  TUnsignedInt8* pxCertCheckNumber
);

/**
 *  @ingroup g_pk
 *
 *  @brief
 *    This function encrypts (AES-128-CBC) the pairing data with a CERT key.
 *
 *    The CSC data buffer (pxCscData) provided must comply with the following
 *    format:
 *
 *    | Field                  | Size |
 *    |------------------------|------|
 *    | Record_Length          |  4   |
 *    | NUID                   |  4   |
 *    | Version                |  2   |
 *    | Provider_ID            |  2   |
 *    | Data                   |  n   |
 *    | CheckNumber            |  4   |
 *
 *    The pairing data buffer (pxPairingData) provided must comply with the
 *    following format:
 *
 *    | Field                  |   Size   |
 *    |------------------------|----------|
 *    | SSV_Length             |    4     |
 *    | Record_Length          |    4     |
 *    | STB_CA_SN              |    4     |
 *    | Version                |    2     |
 *    | PairingKeyInfo         | 102..502 |
 *    | SSV_CertificateData    |   256    |
 *    | SSV_Signature          |   256    |
 *
 *    Encrypted data are written in the same buffer (in-place encryption). The
 *    SSV_length field is not encrypted.
 *
 *  @applies
 *    All NASC versions, except NASC 1.4 and NASC 1.5
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in, out] pxPairingData
 *    RAM buffer containing the pairing data. The first 4 bytes corresponds to
 *    the length of data to be encrypted. Encrypted data are written in the same
 *    buffer (in-place encryption). The SSV_length field is not encrypted.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
 *  @note
 *    By default this function requires an implementation of the BSDAPI.
 *    Define the constant `USE_SEC` when compiling this file to rely on
 *    a SECAPI implementation.
*/
int dptCertEncryptPairingData
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxPairingData
);

/**
 *  @ingroup g_pk
 *
 *  @brief
 *    This function decrypts (AES-128-CBC) the encrypted pairing data with a
 *    CERT key and verifies with the pairing data provided.
 *
 *    The CSC data buffer (pxCscData) provided must comply with the following
 *    format:
 *
 *    | Field                  | Size |
 *    |------------------------|------|
 *    | Record_Length          |  4   |
 *    | NUID                   |  4   |
 *    | Version                |  2   |
 *    | Provider_ID            |  2   |
 *    | Data                   |  n   |
 *    | CheckNumber            |  4   |
 *
 *    The pairing data buffer (pxPairingData) provided must comply with the
 *    following format:
 *
 *    | Field                  |   Size   |
 *    |------------------------|----------|
 *    | SSV_Length             |    4     |
 *    | Record_Length          |    4     |
 *    | STB_CA_SN              |    4     |
 *    | Version                |    2     |
 *    | PairingKeyInfo         | 102..502 |
 *    | SSV_CertificateData    |   256    |
 *    | SSV_Signature          |   256    |
 *
 *    The encrypted pairing data buffer (pxEncryptedPairingData) provided must
 *    comply with the following format:
 *
 *    | Field                  |   Size   |
 *    |------------------------|----------|
 *    | SSV_Length             |    4     |
 *    | encrypted_PK           |    n     |
 *
 *    Decrypted data are written in the same buffer (in-place decryption). The
 *    SSV_length field is not impacted.
 *
 *  @applies
 *    All NASC versions, except NASC 1.4 and NASC 1.5
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] pxPairingData
 *    RAM buffer containing the pairing data for the verification.
 *
 *  @param[in, out] pxEncryptedPairingData
 *    RAM buffer containing the encrypted pairing data. The first 4 bytes corresponds to
 *    the length of data to be decrypted. Decrypted data are written in the same
 *    buffer (in-place decryption). The SSV_length field is not impacted.
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
 *  @note
 *    By default this function requires an implementation of the BSDAPI.
 *    Define the constant `USE_SEC` when compiling this file to rely on
 *    a SECAPI implementation.
*/
int dptCertVerifyEncryptedPairingData
(
  const TUnsignedInt8*  pxCscData,
  const TUnsignedInt8*  pxPairingData,
        TUnsignedInt8*  pxEncryptedPairingData
);

/**
 *  @ingroup g_pk
 *
 *  @brief
 *    This function compares STB_CA_SN in the PK with the reference STB_CA_SN set
 *    in the chipset by reading from CSD/BSD
 *
 *    The pairing data buffer (pxPairingData) provided must comply with the
 *    following format:
 *
 *    | Field                  |   Size   |
 *    |------------------------|----------|
 *    | SSV_Length             |    4     |
 *    | Record_Length          |    4     |
 *    | STB_CA_SN              |    4     |
 *    | Version                |    2     |
 *    | PairingKeyInfo         | 102..502 |
 *    | SSV_CertificateData    |   256    |
 *    | SSV_Signature          |   256    |
 *
 *  @applies
 *    All NASC versions, except NASC 1.4 and NASC 1.5
 *
 *  @param[in] pxPairingData
 *    Buffer containing the pairing data to be compared.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
int dptVerifyStbCaSnPairing(const TUnsignedInt8*  pxPairingData);

/**
 *  @ingroup g_pk
 *
 *  @brief
 *    This function compares STB_CA_SN in the PK with the reference STB_CA_SN set
 *    in the chipset by read from CERT
 *
 *    The pairing data buffer (pxPairingData) provided must comply with the
 *    following format:
 *
 *    | Field                  |   Size   |
 *    |------------------------|----------|
 *    | SSV_Length             |    4     |
 *    | Record_Length          |    4     |
 *    | STB_CA_SN              |    4     |
 *    | Version                |    2     |
 *    | PairingKeyInfo         | 102..502 |
 *    | SSV_CertificateData    |   256    |
 *    | SSV_Signature          |   256    |
 *
 *  @applies
 *    All NASC versions, except NASC 1.4 and NASC 1.5
 *
 *  @param[in] pxPairingData
 *    Buffer containing the pairing data to be compared.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
int dptVerifyStbCaSnPairingCert(const TUnsignedInt8*  pxPairingData);

/**
 *  @ingroup g_cscd
 *
 *  @brief
 *    This function verifies the CSC data. It verifies the CRC and compares the
 *    CERT test check number stored in the CSC data with the test check number
 *    computed by the CERT block.
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @return
 *    0 in case of success, -1 otherwise.
 *
*/
int dptVerifyCscData
(
  const TUnsignedInt8*  pxCscData
);

/**
*  @ingroup g_misc
*
*  @brief
*    This function reads Nuid value from both sec and cert then compare them
*
*    It is useful to verify that when TEE privileged mode is enabled the cert
*    is not accessible anymore from REE.
*
*  @applies
*    All NASC versions, except NASC 1.4, NASC 1.5 and NASC 3.0
*
*  @param[out] pxResult
*    TBoolean type variable address for returning Nuid comparison result.
*    It has to be allocated by the caller and has to be TBoolean type address.
*
*  @return
*    0 in case of success, -1 otherwise.
*
*  @codesample
*  \code
*  TBoolean nuidResult;
*
*  if(!dptNuidCompare(nuidResult))
*  {
*    // Operation is successful, print the result
*    printf(nuidResult);
*  }
*  \endcode
*/
int dptNuidCompare
(
    TBoolean *pxResult
);

/**
 *  @ingroup g_misc
 *
 *  @brief
 *    This function reads the 4-byte STB CA S/N from the driver and convert it
 *    in a null-terminated string complying the format "xx xxxx xxxx xx".
 *
 *    <b>Note:</b> the STB CA S/N must be written in the chipset persistent memory
 *    before calling this function.
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[out] pxStbCaSnString
 *    Buffer containing the null-terminated string formatted STB CA S/N.
 *    It has to be allocated by the caller and has to be 16 bytes long.
 *
 *  @return
 *    0 in case of success, -1 otherwise.
 *
 *  @codesample
 *  \code
 *  TChar StbCanSnString[16];
 *
 *  if(!dptGetStbCaSnString(StbCanSnString))
 *  {
 *    // Operation is successful, print the result
 *    printf(StbCanSnString);
 *  }
 *  \endcode
*/
int dptGetStbCaSnString
(
  TChar* pxStbCaSnString
);


/**
*  @ingroup g_misc
*
*  @brief
*    This function takes the 4-byte STB CA S/N in input and convert it
*    in a null-terminated string complying the format "xx xxxx xxxx xx".
*
*    It is useful when the STB CA S/N is not written in the chipset persistent
*    memory, or when there is no reset after the write.
*
*  @applies
*    All NASC versions
*
*  @param[in] pxStbCaSnArray
*    Buffer containing 4-byte STB CA S/N to be converted.
*
*  @param[out] pxStbCaSnString
*    Buffer containing the null-terminated string formatted STB CA S/N.
*    It has to be allocated by the caller and has to be 16 bytes long.
*
*  @return
*    0 in case of success, -1 otherwise.
*
*  @codesample
*  \code
*  TChar StbCanSnString[16];
*  TUnsignedInt8 caSnBuffer[4] = { 0x00, 0x01, 0x00, 0x00 };
*
*  if(!dptStbCaSnToString(caSnBuffer, StbCanSnString))
*  {
*    // Operation is successful, print the result
*    printf(StbCanSnString);
*  }
*  \endcode
*/
int dptStbCaSnToString
(
TUnsignedInt8 * pxStbCaSnArray,
TChar* pxStbCaSnString
);

/**
 *  @ingroup g_predesc
 *
 *  @brief
 *    This function gets, from the CSC data, the DVB-CSA2 1-level protecting key to be
 *    injected in the key ladder to make the pre-descrambling test.
 *
 *    This key corresponds to the pxL1CipheredProtectingKey parameter of the
 *    SEC API secSet1LevelProtectedKey function / DSC API dscSet1LevelProtectedKey function
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[out] pxL1CipheredProtectingKeySize
 *    Size of the protecting key. Equal to 16 bytes for NOCS chipset.
 *
 *  @param[out] pxL1CipheredProtectingKey
 *    Buffer containing the 1-level protecting key. This buffer is allocated by
 *    the caller. If this parameter is set to NULL, this function returns
 *    the size of the key.
 *
 *  @return
 *    0 in case of success, -1 otherwise.
 *
 *  @codesample
 *    This example gets the L1 protected key required for the pre-descrambling
 *    test from the CSC data. It is assumed that CSC data are available from
 *    `pxCscData` pointer and that even and odd content key (control words) are
 *    available from `pxEvenContentKey` and `pxOddContentKey` pointer respectively.
 *  \code
 *  TSecFunctionTable*   pSecFt;
 *  TSecCipherSession    pSecSession = NULL;
 *  TUnsignedInt8        l1ProtKey[16];      // Use to store the L1 protecting key
 *  size_t               l1ProtKeySize;      // Fake use
 *  TUnsignedInt8        keyId[1];           // Use to specify the key parity (odd or even)
 *  TUnsignedInt16       emi=0x0000;         // DVB-CSA algorithm
 *  size_t               contentKeySize=8;   // DVB-CSA content key size
 *
 *  do
 *  {
 *    // Get the SEC function table
 *    if(NULL == (pSecFt = secGetFunctionTable())){break;}
 *
 *    // Open a SEC stream decryption session with the Transport Session ID 0
 *    if(SEC_NO_ERROR != pSecFt->secOpenStreamDecryptSession(&pSecSession, 0)){break;}
 *
 *    // Get the L1 protecting key from the CSC data. It is assumed that pxCscData
 *    // points to the CSC data.
 *    if(dptGetPreDescramblingL1ProtectingKey(pxCscData, &l1ProtKeySize, l1ProtKey)){break;}
 *
 *    // Set the even content key
 *    keyId[0]=0;
 *    if(SEC_NO_ERROR != pSecFt->secSet1LevelProtectedKey(
 *      pSecSession, emi, 1, keyId,
 *      contentKeySize, pxEvenContentKey,
 *      l1ProtKeySize, l1ProtKey
 *      )){break;}
 *
 *    // Set odd content key
 *    keyId[0]=1;
 *    if(SEC_NO_ERROR != pSecFt->secSet1LevelProtectedKey(
 *      pSecSession, emi, 1, keyId,
 *      contentKeySize, pxOddContentKey,
 *      l1ProtKeySize, l1ProtKey
 *      )){break;}
 *
 *    // Stream is being descrambled. Close the SEC session to when you need to
 *    // stop the descrambling.
 *  }
 *  while(0);
 *  \endcode
*/
int dptGetPreDescramblingL1ProtectingKey
(
  const TUnsignedInt8*  pxCscData,
        size_t*         pxL1CipheredProtectingKeySize,
        TUnsignedInt8*  pxL1CipheredProtectingKey
);

/**
 *  @ingroup g_predesc
 *
 *  @brief
 *    This function gets, from the CSC data, the 1-level protecting key to be
 *    injected in the key ladder to make the pre-descrambling test. The key is
 *    selected through the EMI parameter.
 *
 *    This key corresponds to the pxL1CipheredProtectingKey parameter of the
 *    SEC API secSet1LevelProtectedKey function / DSC API dscSet1LevelProtectedKey function
 *
 *    EMI supported by this functions are:
 *    - DVB-CSA2 : 0x0000
 *    - DVB-CSA3 : 0x0001
 *    - AES-128  : 0x0020, 0x0021 and 0x0022
 *
 *  @applies
 *    All NASC versions
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] xEmi
 *    Selected EMI
 *
 *  @param[out] pxL1CipheredProtectingKeySize
 *    Size of the protecting key. Equal to 16 bytes for NOCS chipset.
 *
 *  @param[out] pxL1CipheredProtectingKey
 *    Buffer containing the 1-level protecting key. This buffer is allocated by
 *    the caller. If this parameter is set to NULL, this function returns
 *    the size of the key.
 *
 *  @return
 *    0 in case of success, -1 otherwise.
 *
*/
int dptGetPreDescramblingL1ProtectingKeyByEmi
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt16   xEmi,
        size_t*         pxL1CipheredProtectingKeySize,
        TUnsignedInt8*  pxL1CipheredProtectingKey
);


/**
*  @ingroup g_cscd
*
*  @brief
*    This function gets and outputs, from the CSC data record, the 4-bytes CSC configuration data
*    so that STBM can include it in Production Report as single field.
*
*  @applies
*    All NASC versions, except NASC 1.4
*
*  @param[in] pxCscData
*    Buffer containing the CSC data to be parsed.
*
*  @param[out] pxCscDataConfiguration
*    Buffer containing the output 4-bytes CSC configuration data.
*    It has to be allocated by the caller and has to be at least 4 bytes long.
*
*  @return
*    0 in case of success, -1 otherwise.
*
*  @codesample
*    This example parses CSC data record and output the CSC configuration data.
*    It is assumed that CSC data are available from `pxCscData` pointer (cscDataBuffer).
*  \code
*  TUnsignedInt8 CscDataConfiguration[4];
*
*  if(!dptGetCscDataConfiguration(cscDataBuffer, CscDataConfiguration))
*  {
*    // Operation is successful, print the result
*  }
*  \endcode
*/
int dptGetCscDataConfiguration
(
    const TUnsignedInt8*  pxCscData,
    TUnsignedInt8*        pxCscDataConfiguration
);



/**
  @ingroup g_cscd

  @brief
    This function returns a string of 3 characters corresponding to the device
    manufacturer ID coming from the CSCD device certificate.

  @applies
    NASC 3.2

  @param[in] pxCscData
    Buffer containing the CSC data to be parsed.

  @param[out] pxDeviceManufacturerId
    4-byte buffer allocated by the caller where this function stores the
    3-character device manufacturer ID plus the null terminator.

  @return
    0 in case of a successful operation, -1 otherwise.

  @codesample
  \code
  TUnsignedInt8 DeviceManufacturerId[4];

  if(!dptGetDeviceManufacturerId(cscDataBuffer, DeviceManufacturerId))
  {
    printf("DeviceManufacturerId: %s\n", DeviceManufacturerId);
  }
  \endcode
 */
int dptGetDeviceManufacturerId
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxDeviceManufacturerId
);



/**
  @ingroup g_cscd

  @brief
    This function returns a string of 3 characters corresponding to the device
    model ID coming from the CSCD device certificate.

  @applies
    NASC 3.2

  @param[in] pxCscData
    Buffer containing the CSC data to be parsed.

  @param[out] pxDeviceModelId
    4-byte buffer allocated by the caller where this function stores the
    3-character device model ID plus the null terminator.

  @return
    0 in case of a successful operation, -1 otherwise.

  @codesample
  \code
  TUnsignedInt8 DeviceModelId[4];

  if(!dptGetDeviceModelId(cscDataBuffer, DeviceModelId))
  {
    printf("DeviceModelId: %s\n", DeviceModelId);
  }
  \endcode
 */
int dptGetDeviceModelId
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxDeviceModelId
);


/**
  @ingroup g_seg

  @brief
    This function programs CERT segmentation records in OTP.
    Segmentation records are retrieved from CSCD.

    This function requires the implementation of certAklReset() introduced in
    CERT API 1.4.0. This latter function allows resetting the CERT IP block
    without the need to reboot the device.

  @pre
    CERT API v1.4.0 or higher must be available

  @applies
    NASC 3.2

  @param[in] pxCscData
    Buffer containing the CSC data to be parsed.

  @return
    0 in case of a successful operation, -1 otherwise.
 */
int dptProgramCertSegment
(
  const TUnsignedInt8*  pxCscData
);


#endif /* CA_DPT_H */
