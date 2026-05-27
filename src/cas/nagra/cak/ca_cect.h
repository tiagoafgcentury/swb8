/**
**  @file ca_cect.h
**
**  @brief
**   CERT Commands Translator (CECT)
**
**   This file specifies the CERT Commands Translator API that allows Nagra CA
**   software to send CERT transactions to the CERT block.
**
** Copyright:
**   2011-2019 Nagravision S.A.
**
*/
/*
** REMARK:
**    Comments in this file use special tags to allow automatic API
**    documentation generation in HTML format, using the
**    GNU-General Public Licensed Doxygen tool.
**    For more information about Doxygen, please check www.doxygen.org
*/



#ifndef CA_CECT_H
#define CA_CECT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Below Macro NV_INTERNAL_API added to have
 * manually obfuscated cect module.
 * Since cect exposes a single API cectGetFunctionTable() only,
 * only cectGetFunctionTable() will be manually obfuscated.
*/
#ifndef NV_INTERNAL_API
#define NV_INTERNAL_API
#endif

/**
 * Defines the version number of the CECT API to be implemented. This version
 * has to be included in the function table returned by cectGetFunctionTable().
 * To do so, use the macro CECTAPI_VERSION_INT to put it in the right format.
*/
#define CECTAPI_VERSION_MAJOR  4
#define CECTAPI_VERSION_MEDIUM 3
#define CECTAPI_VERSION_MINOR  1

/******************************************************************************/
/*                                                                            */
/*                           GENERAL INCLUDE FILES                            */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"
#include <stddef.h>

#ifdef CA_INCLUDE_OBFUSCATE_SYMBOLS
  #ifndef cectGetFunctionTable
    #define cectGetFunctionTable    CkTSr_FpiMs
  #endif /* ndef cectGetFunctionTable */
#endif

/******************************************************************************/
/*                                                                            */
/*                              VERSION TOOL                                  */
/*                                                                            */
/******************************************************************************/


#ifndef CECT_TOOL_VERSION
#define CECT_TOOL_STRINGIFY(s) CECT_TOOL_TOSTRING(s)
#define CECT_TOOL_TOSTRING(s)  #s

#define CECT_TOOL_VERSION_INT(a, b, c) (a<<16 | b<<8 | c)
#define CECT_TOOL_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define CECT_TOOL_VERSION(a, b, c) CECT_TOOL_VERSION_DOT(a, b, c)
#endif

#define CECTAPI_VERSION_INTER CECT_TOOL_VERSION(CECTAPI_VERSION_MAJOR, \
              CECTAPI_VERSION_MEDIUM, \
              CECTAPI_VERSION_MINOR)

#define CECTAPI_VERSION_INT   CECT_TOOL_VERSION_INT(CECTAPI_VERSION_MAJOR, \
            CECTAPI_VERSION_MEDIUM, \
            CECTAPI_VERSION_MINOR)

#define CECTAPI_VERSION_STRING  "CECTAPI_" CECT_TOOL_STRINGIFY(CECTAPI_VERSION_INTER)


/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    Bitmap indicating which system information field is present
 *
 *  @see CECT_MASK_PCUID, CECT_MASK_OCUID, CECT_MASK_SEGUID, CECT_MASK_SOCUID,
 *    CECT_MASK_BOOT_CODE_VERSION, CECT_MASK_MSID, CECT_MASK_IRD_SN,
 *    CECT_MASK_SOC_CONFIG
 */
typedef TUnsignedInt8 TCectSystemFieldBitmap;
/**  @ingroup g_c1_config
     @{
*/
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.pcuid contains a significant value.
 */
#define CECT_MASK_PCUID                  BIT0
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.ocuid contains a significant value.
 */
#define CECT_MASK_OCUID                  BIT1
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.segUid contains a significant value.
 */
#define CECT_MASK_SEGUID                 BIT2
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.socUid contains a significant value.
 */
#define CECT_MASK_SOCUID                 BIT3
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.bootCodeVersion contains a significant value.
 */
#define CECT_MASK_BOOT_CODE_VERSION      BIT4
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.msid contains a significant value.
 */
#define CECT_MASK_MSID                   BIT5
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.irdSerialNumber contains a significant value.
 */
#define CECT_MASK_IRD_SN                 BIT6
/** Mask to be applied on TCectSystem.fieldBitmap to know whether
 *   TCectSystem.socConfig contains a significant value.
 */
#define CECT_MASK_SOC_CONFIG             BIT7
/** @} */

/**
  @brief
    Constants to be used to interpret TC2ctSystem.dataValidity
 */
/** Mask to be applied on TC2ctSystem.dataValidity to know whether
 *   the PERSOKeys record contains a valid symmetric key.
 */
#define C2CT_MASK_SYM_KEYS               BIT0
/** Mask to be applied on TC2ctSystem.dataValidity to know whether
 *   the PERSOKeys record contains a valid asymmetric key.
 */
#define C2CT_MASK_ASYM_KEYS              BIT1
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.pcuid contains a significant value.
 */
#define C2CT_MASK_PCUID                  BIT8
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.bindingNb contains a significant value.
 */
#define C2CT_MASK_BINDING_NB             BIT9
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.ocuid contains a significant value.
 */
#define C2CT_MASK_OCUID                  BIT10
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.segUid contains a significant value.
 */
#define C2CT_MASK_SEGUID                 BIT11
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.keyType contains a significant value.
 */
#define C2CT_MASK_KEY_TYPE               BIT12
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.ppid contains a significant value.
 */
#define C2CT_MASK_PPID                   BIT13
/** Mask to be applied on TCectSystem.dataValidity to know whether
 *   TC2ctSystem.idx contains a significant value.
 */
#define C2CT_MASK_IDX                    BIT14


/**
 *  @brief
 *    Status code returned by functions of the CECT API
*/
typedef enum
{
  CECT_NO_ERROR,
   /**< Requested operation was completed successfully
    */
  CECT_ERROR_BAD_PARAMETER,
   /**< Invalid or unknown parameter
    */
  CECT_ERROR,
   /**< Requested operation failed
    */
  CECT_ERROR_OTP_LOCKED,
   /**< CERT OTP record is locked
    */
  CECT_ERROR_KEY_PENDING,
   /**< A key is already pending on the CERT key interface. The CERT is no longer
    *   able to output another as long as the previous has not been ackknowledged
    *   with a secUseCertKey call or bsdUseCertKey call.
    */
  CECT_ERROR_NOT_AVAILABLE,
   /**< Requested operation not available
    */
  LAST_CECT_STATUS
   /**< Internal use only.
    */
} TCectStatus;


/**
  @ingroup g_c1_config
  @brief
    Constants defining different items size
 */
#define CECT_CUID_SIZE               6
/**< CERT unique ID size
 */
#define CECT_SEGUID_SIZE             2
/**< CERT segment unique ID size
 */
#define CECT_SEGNBR_SIZE             4
/**< CERT segment number size
 */
#define CECT_SOCUID_SIZE             8
/**< SoC unique ID size
 */
#define CECT_BOOT_CODE_VERSION_SIZE  4
/**< Boot code version size
 */
#define CECT_MSID_SIZE               4
/**< CERT market segment ID size
 */
#define CECT_IRDSN_SIZE              4
/**< IRD serial number size
 */
#define CECT_ACT_BITMAP_SIZE         9
/**< \b CERT1 activation bitmap size
 */
#define CECT_NETLIST_VERSION_SIZE    4
/**< CERT netlist version size
 */
#define CECT_CI_STATUS_SIZE          4
/**< CERT Common Interface status register size
 */
#define CECT_KEY_COMP_SIZE           2
/**< Key component size
 */
#define CECT_INTEGRA_TYPE_SIZE       CECT_KEY_COMP_SIZE
/**< Integra type size
 */
#define CECT_INTEGRA_VALUE_SIZE      8
/**< Integra value size
 */
#define CECT_OTA_KEY_SIZE            16
/**< CERT OTA key size
 */
#define CECT_CIPHERED_HH_KEY_SIZE    32
/**< Ciphered hammer-head key size, including IV and TAG
 */
#define CECT_IDEANXT_SW_KEY_SIZE     16
/**< IDEANxT software key size
 */
#define CECT_PPID_SIZE               2
/**< PPID size
 */
#define CECT_DATE_TIME_SIZE          4
/**< Date and time size
 */
#define CECT_ADF_SIZE                64
/**< ADF size
 */
#define CECT_GA_SIZE                 4
/**< GA size
 */
#define CECT_CAID_SIZE               4
/**< CAID size
 */
#define CECT_GUA_SIZE                4
/**< GUA size
 */
#define CECT_BAE_REVOCATION_SIZE     8
/**< BAE revocation size
 */
#define CECT_BAE_COUNTERMEASURE_SIZE 12
/**< BAE countermeasure size
 */
#define CECT_AUTHENT_TAG_SIZE        8
/**< Authentication tag size
 */
#define CECT_IV_SIZE                 8
/**< Initialization vectore size
 */
#define CECT_HASH_SIZE               16
/**< CERT custom hash size
 */
#define CECT_OTP_SLOT_STATUS_SIZE    4
/**< OTP slot status size
 */
#define CECT_DATA_VALIDITY_SIZE      2
/**< CERT2 data validity size
 */
#define CECT_BINDING_NB_SIZE         8
/**< BindingNb size
 */
#define CECT_KEY_TYPE_SIZE           2
/**< KeyType size
 */
#define CECT_IDX_SIZE                8
/**< Idx size
 */


/**
  @brief
    Constants to be used to interpret the OTP slot status
 */
#define C2CT_MASK_PERSO_KEYS_REC_LOCKED          BIT0
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the PERSOKeys record is locked.
 */
#define C2CT_MASK_PERSO_KEYS_REC_VIRGIN          BIT4
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the PERSOKeys record is virgin.
 */
#define C2CT_MASK_PERSO_KEYS_REC_AUTHENTICATED   BIT8
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the PERSOKeys record is authenticated.
 */
#define C2CT_MASK_BINDING_NB_REC_LOCKED          BIT16
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the BindingNb record is locked.
 */
#define C2CT_MASK_BINDING_NB_REC_VIRGIN          BIT20
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the BindingNb record is virgin.
 */
#define C2CT_MASK_BINDING_NB_REC_AUTHENTICATED   BIT24
/**< Mask to be applied on TCectSystem.otpSlotStatus to know whether
 *   the BindingNb record is authenticated.
 */


/**
 *  @ingroup g_c1_config
 *
 *  @brief
 *    Structure used to accommodate all CERT system information parameters
 */
typedef struct
{
  TUnsignedInt8 pcuid[CECT_CUID_SIZE];
   /**< CUID programmed in OTP personalization slot
    */
  TUnsignedInt8 ocuid[CECT_CUID_SIZE];
   /**< CUID programmed in OTP over-the-air slot
    */
  TUnsignedInt8 segUid[CECT_SEGUID_SIZE];
   /**< Segment unique identifier
    */
  TUnsignedInt8 socUid[CECT_SOCUID_SIZE];
   /**< Unique identifier of the SoC. This ID is defined and owned by the SoC
    *   manufacturer.
    */
  TUnsignedInt8 bootCodeVersion[CECT_BOOT_CODE_VERSION_SIZE];
   /**< This 4-byte parameter is used for the versioning of the STB boot code.
    */
  TUnsignedInt8 msid[CECT_MSID_SIZE];
   /**< Market segment identifier as defined in NOCS specification
    */
  TUnsignedInt8 irdSerialNumber[CECT_IRDSN_SIZE];
   /**< Serial number of the set-top box (aka STB CA S/N) stored in the SoC.
    */
  TUnsignedInt8 socConfig;
   /**< Bitmap providing the "security state" of the SoC
    */
  TCectSystemFieldBitmap fieldBitmap;
   /**< Bitmap signaling presence/absence of system information fields above
    *   (pcuid to socConfig). A bit set to '1' means the corresponding field
    *   is present.
    */
  TUnsignedInt8 actBitmap[CECT_ACT_BITMAP_SIZE];
   /**< Activation bitmap defining which CERT commands are currently active.
    */
  TUnsignedInt8 actVersion;
   /**< Version of the activation bitmap. Mainly used to revoke activation of
    *   some mode of operations or commands.
    */
  TUnsignedInt8 netlistVersion[CECT_NETLIST_VERSION_SIZE];
   /**< Version of the netlist
    */
  TUnsignedInt8 ciStatus[CECT_CI_STATUS_SIZE];
   /**< This 4-byte parameter corresponds to the Common Interface status
    *   register.
    */
  TUnsignedInt8 cert3_segUid[CECT_SEGUID_SIZE];
   /**< Unique identifier of active CERT3 Segment if any. Filled with zero if no segment programmed.@n
    *   Relevant only if \b CERT3 hardware block.
    */
  TUnsignedInt8 cert3_segState;
   /**< State of active CERT3 Segment. Filled with zero if no segment programmed.@n
    *   Relevant only if \b CERT3 hardware block.
    */
  TUnsignedInt8 cert3_segNr;
   /**< Number of active CERT3 Segment.@n
    *   Possible values:@n
    *   - 0, 1, 2 for hardware segment@n
    *   - 3 for software segment@n
    *   - 255 when CERT3 is not segmented@n
    *   Relevant only if \b CERT3 hardware block.
    */
} TCectSystem;

/**
 *  @ingroup g_c2_config
 *
 *  @brief
 *    Structure used to accommodate all CERT2 system information parameters
 */
typedef struct
{
  TUnsignedInt8 dataValidity[CECT_DATA_VALIDITY_SIZE];
   /**< Bitmap signaling validity of CERT info (PCUID, OCUID, SEGUID, BindingNb,
        PPID, KeyType, Idx, symmetric and asymmetric keys). A bit set to '1'
        means the corresponding info is valid.
    */
  TUnsignedInt8 pcuid[CECT_CUID_SIZE];
   /**< CUID identifying personalization keys programmed in chipset OTP (aka NUID).
    */
  TUnsignedInt8 ocuid[CECT_CUID_SIZE];
   /**< CUID identifying OTA keys (aka VUA)
    */
  TUnsignedInt8 segUid[CECT_SEGUID_SIZE];
   /**< Segment unique identifier
    */
  TUnsignedInt8 bindingNb[CECT_BINDING_NB_SIZE];
   /**< Binding number
    */
  TUnsignedInt8 ppid[CECT_PPID_SIZE];
   /**< PPID
    */
  TUnsignedInt8 keyType[CECT_KEY_TYPE_SIZE];
   /**< KeyType
    */
  TUnsignedInt8 idx[CECT_IDX_SIZE];
   /**< Idx
    */
  TUnsignedInt8 netlistVersion[CECT_NETLIST_VERSION_SIZE];
   /**< Version of the netlist
    */
  TUnsignedInt8 ciStatus[CECT_CI_STATUS_SIZE];
   /**< This 4-byte parameter corresponds to the Common Interface status
    *   register.
    */
  TUnsignedInt8 otpSlotStatus[CECT_OTP_SLOT_STATUS_SIZE];
   /**< This 4-byte parameter provides the status of the OTP slot
    */
} TC2ctSystem;


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    Pointer to an opaque structure to be defined by the entity in charge of
 *    developing the CECT component and used as a handle on a CERT key ladder.
 */
typedef struct SCectKlHandle* TCectKlHandle;

/**
 *  @brief
 *    Type used to specify the parity of the content key (control word).
*/
typedef enum
{
  CECT_PARITY_EVEN,
  CECT_PARITY_ODD
} TCectParity;


/** @addtogroup g_c1_bae
 * @{
*/

/**
 *  @brief
 *    Type used to specify a CAID range
*/
typedef struct
{
  TUnsignedInt8 min[CECT_CAID_SIZE];
  /**< CAID range lower bound
  */
  TUnsignedInt8 max[CECT_CAID_SIZE];
  /**< CAID range upper bound
  */
} TCectCaidRange;


/**
 *  @brief
 *    BAE ENT record
*/
typedef struct
{
  TUnsignedInt8   integraType[CECT_INTEGRA_TYPE_SIZE];
  /**< Entitlement integra type. Used as a key component by the CERT key
   *   generator.
  */
  TUnsignedInt8   ppid[CECT_PPID_SIZE];
  /**< Entitlement's PPID
   */
  TUnsignedInt8   ruCaid[CECT_CAID_SIZE];
  /**< Entitlement's ruCAID
   */
  TUnsignedInt8   level;
  /**< Entitlement's level
   */
  size_t          caidRangesNum;
  /**< Number of CAID ranges
   */
  TCectCaidRange* pCaidRanges;
  /**< Buffer allocated by the caller containing caidRangesNum CAID ranges
   */
  size_t          caidRangeIndex;
  /**< Index, starting from 0, of the CAID range that matches the ECM CAID
   *   range.
   */
  TUnsignedInt8   integraValue[CECT_INTEGRA_VALUE_SIZE];
  /**< Value of the entitlement's integra
   */
} TCectBaeEnt;


/**
 *  @brief
 *    BAE OPE record
*/
typedef struct
{
  TUnsignedInt8   integraType[CECT_INTEGRA_TYPE_SIZE];
  /**< OPE integra type. Used as a key component by the CERT key
   *   generator.
   */
  TUnsignedInt8   gua[CECT_GUA_SIZE];
   /**< MOP GUA
    */
  size_t          caidRangesNum;
  /**< Number of CAID ranges
   */
  TCectCaidRange* pCaidRanges;
  /**< Buffer allocated by the caller containing caidRangesNum CAID ranges
   */
  size_t          caidRangeIndex;
  /**< Index, starting from 0, of the CAID range that matches the entitlement
   *   ruCAID.
   */
  TUnsignedInt8   integraValue[CECT_INTEGRA_VALUE_SIZE];
  /**< Value of the OPE integra
   */
} TCectBaeOpe;


/**
 *  @brief
 *    BAE PAO record
*/
typedef struct
{
  TUnsignedInt8   integraType[CECT_INTEGRA_TYPE_SIZE];
  /**< PAO integra type. Used as a key component by the CERT key
   *   generator.
   */
  TUnsignedInt8   rDateTime[CECT_DATE_TIME_SIZE];
  /**< MOP RDateTime
   */
  TUnsignedInt8   ga[CECT_GA_SIZE];
  /**< MOP GA
   */
  TUnsignedInt8   adf[CECT_ADF_SIZE];
  /**< MOP ADF
   */
  TUnsignedInt8   integraValue[CECT_INTEGRA_VALUE_SIZE];
  /**< Value of the PAO integra
   */
} TCectBaePao;


/**
 *  @brief
 *    BAE ECM record
*/
typedef struct
{
  TUnsignedInt8   revocation[CECT_BAE_REVOCATION_SIZE];
  /**< Revocation. Default value is 0x00000000'00000000.
   */
  TUnsignedInt8   integraType[CECT_INTEGRA_TYPE_SIZE];
  /**< ECM integra type. Used as a key component by the CERT key
   *   generator.
   */
  TUnsignedInt8   timestamp[CECT_DATE_TIME_SIZE];
  /**< ECM timestamp
   */
  TUnsignedInt8   level;
  /**< ECM level
   */
  TUnsignedInt8   version;
  /**< ECM version
   */
  TUnsignedInt8   counterMeasure[CECT_BAE_COUNTERMEASURE_SIZE];
  /**< Countermeasure. Default value is 0x00000000'00000000'00000000
   */
  size_t          cipheredContentKeySize;
  /**< Size, in bytes, of the ciphered content keys (aka control words). It
   *   depends on the scrambling algorithm:
   *    - AES: 16 bytes
   *    - DVB-CSA2: 8 bytes
   *    - DVB-CSA3: 16 bytes
   *    - ASA64: 8 bytes
   *    - ASA128: 16 bytes
   */
  TUnsignedInt8*  pCipheredEvenContentKey;
  /**< Buffer, allocated by the caller, containing the ciphered even content key.
   */
  TUnsignedInt8*  pCipheredOddContentKey;
  /**< Buffer, allocated by the caller, containing the ciphered odd content key.
   */
  size_t          caidRangesNum;
  /**< Number of CAID ranges
   */
  TCectCaidRange* pCaidRanges;
  /**< Buffer allocated by the caller containing caidRangesNum CAID ranges
   */
  size_t          caidRangeIndex;
  /**< Index, starting from 0, of the CAID range that matches the entitlement
   *   CAID range.
   */
  TUnsignedInt8   integraValue[CECT_INTEGRA_VALUE_SIZE];
  /**< Value of the ECM integra
   */
} TCectBaeEcm;


/**
 *  @brief
 *    BAE keys
*/
typedef struct
{
  TUnsignedInt8   gHhKeyComp[CECT_KEY_COMP_SIZE];
  /**< Key component of GHHCWKey
   */
  TUnsignedInt8   cipheredGHhCwKey[CECT_CIPHERED_HH_KEY_SIZE];
  /**< 32-byte buffer, allocated by the caller, containing the ciphered GHHCWKey.
   *   This buffer also contain the IV and the authentication tag:
   *   cipheredGHhCwKey = IV(8) || [GHHCWKey](16) || TAG(8)
   */
  TUnsignedInt8   uHhKeyComp[CECT_KEY_COMP_SIZE];
  /**< Key component of UHHCWKey
   */
  TUnsignedInt8   cipheredUHhCwKey[CECT_CIPHERED_HH_KEY_SIZE];
  /**< 32-byte buffer, allocated by the caller, containing the ciphered UHHCWKey.
   *   This buffer also contain the IV and the authentication tag:
   *   cipheredUHhCwKey = IV(8) || [UHHCWKey](16) || TAG(8)
   */
} TCectBaeKeys;

/** @} bae */

/** @addtogroup g_c1_perso
 * @{
*/

/**
 *  @brief
 *    OTA keys record
*/
typedef struct
{
  TUnsignedInt8   integraType[CECT_INTEGRA_TYPE_SIZE];
  /**< OTA keys integra type used as key component by the CERT key generator.
   */
  TUnsignedInt8   ocuid[CECT_CUID_SIZE];
  /**< OTA CUID (aka NUID OTA)
   */
  TUnsignedInt8   ukOta[CECT_OTA_KEY_SIZE];
  /**< UKota key programmed (or to be program) in the CERT OTA key slot. It is
   *   unique and identified by the OCUID.
   */
  TUnsignedInt8   gkOta[CECT_OTA_KEY_SIZE];
  /**< GKota key programmed (or to be program) in the CERT OTA key slot. It is
   *   global and associated to the OCUID.
   */
  TUnsignedInt8   integraValue[CECT_INTEGRA_VALUE_SIZE];
  /**< Value of the OTA keys integra
   */
} TCectOtaKeys;


/** @} perso */


/******************************************************************************/
/*                                                                            */
/*                           FUNCTION TABLE DEFINITION                        */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_c1_config
 *
 *  @brief
 *    This function is used to read system information (public data) of the CERT
 *    block.
 *
 *  @param[out] pxSystem
 *    Pointer to system object allocated by the caller and to be filled in with
 *    CERT system information.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR
 *    Operation failed
*/
typedef TCectStatus (*TCectSystemGetInfo)
(
  TCectSystem*  pxSystem
);


/**
 *  @ingroup g_c1_act
 *
 *  @brief
 *    This function activates the CERT block. It requires an unique and/or
 *    a global activation descriptor complying with the following format
 *    (see CAS_IrdPersoData):
 *
 *    <pre>
 *      tag            (1) = A0h
 *      length         (1) = 14h
 *      key_comp       (2)
 *      version        (1)
 *      act_bitmap     (9)
 *      integra        (8)
 *    </pre>
 *
 *    Unique and global descriptors are identified thanks to the key_comp field.
 *
 *  @param[in] xNuidSize
 *    This parameter is no longer required and can be set to 0.
 *    However, it is kept for backward compatiblity. In such a case it indicates
 *    the size in bytes of the NUID: 4 bytes (standard NUID) or to 6 bytes
 *    (extended NUID).
 *
 *  @param[in] pxNuid
 *    This parameter is no longer required and can be set to NULL.
 *    However, it is kept for backward compatibility. In such a case it corresponds
 *    to the buffer containing the xNuidSize bytes of the NUID in MSBF format.
 *
 *  @param[in] xDescriptorsSize
 *    Size in bytes of pxDescriptors buffer.
 *
 *  @param[in] pxDescriptors
 *    Buffer, allocated by the caller, containing a collection of TLV descriptors
 *    including the activation descriptor.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR
 *    Operation failed
*/
typedef TCectStatus (*TCectActivate)
(
        size_t              xNuidSize,
  const TUnsignedInt8*     pxNuid,
        size_t              xDescriptorsSize,
  const TUnsignedInt8*     pxDescriptors
);


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    This function creates a key ladder instance
 *
 *  @pre
 *    This functions requires the CERT block to be activated (TCectActivate).
 *
 *  @param[out] pxHandle
 *    Handle assigned to the key ladder instance
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR
 *    - Operation failed
 *
 *  @warning
 *    In order to avoid a deadlock, it is not authorized to have several key
 *    ladder instances created from the \b same thread. A key ladder instance
 *    has to be diposed of as soon as possible.
*/
typedef TCectStatus (*TCectCreateKeyLadder)
(
  TCectKlHandle* pxHandle
);


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    This function disposes of a key ladder instance
 *
 *  @param[out] xHandle
 *    Handle on key ladder instance
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    Handle provided is invalid or unknown.
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @warning
 *    It is important to dispose of a key ladder instance as soon as possible in
 *    order to unlock the underlying CERT resource.
*/
typedef TCectStatus (*TCectDisposeKeyLadder)
(
  TCectKlHandle xHandle
);


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    This function requests the CERT block to output a key from a root level
 *    key ladder.
 *
 *    The output key available on the CERT key interface is then mainly intended
 *    to be used for RAM2RAM operations. This is achieved by calling
 *    secUseCertKey() or bsdUseCertKey functions. Refer to the SEC and BSD API
 *    for further details about these functions.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - A key ladder instance has been created (see TCectCreateKeyLadder)
 *
 *  @param[in] xKlHandle
 *    Handle on key ladder instance
 *
 *  @param[in] xKeyProtPath
 *    Key protection path (KPP) to be used for the operation. It is used by the
 *    CECT to select the right CERT key ladder. Refer to [NOCSAD] for KPP values
 *    definitions.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    A parameter is invalid
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @retval CECT_ERROR_KEY_PENDING
 *    Operation failed because a key is aleardy pending on the CERT key
 *    interface. Call secUseCertKey() or bsdUseCertKey() to solve this issue.
 *
 *  @note
 *    The content cipher algorithm signaled in the key protection path (KPP)
 *    has to match the EMI of the targeted RAM2RAM crypto-engine, i.e. the EMI
 *    parameter of secUseCertKey() or bsdUseCertKey().
 *
*/
typedef TCectStatus (*TCectProcessRootLevelProtKey)
(
        TCectKlHandle       xHandle,
        TUnsignedInt8       xKeyProtPath
);


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    This function requests the CERT block to output a key from a 1-level
 *    key ladder.
 *
 *    The output key available on the CERT key interface may be used then by a
 *    crypto-engine for encryption/decryption operations.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - A key ladder instance has been created (see TCectCreateKeyLadder)
 *
 *  @param[in] xKlHandle
 *    Handle on key ladder instance
 *
 *  @param[in] xKeyProtPath
 *    Key protection path (KPP) to be used for the operation. It is used by the
 *    CECT to select the right CERT key ladder. Refer to [NOCSAD] for KPP values
 *    definitions.
 *
 *  @param[in]  xCipheredContentKeySize
 *    Size in bytes of the ciphered content key. It depends on the content cipher
 *    algorithm signaled in the KPP:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *    - ASA64: 8 bytes
 *    - ASA128: 16 bytes
 *
 *  @param[in]  pxCipheredContentKey
 *    Buffer, allocated by the caller, containing the ciphered content key. It
 *    is equal to CipheredContentKey=TDES(ContentKey, L1ProtKey).
 *
 *  @param[in]  xCipheredProtKeySize
 *    Size in bytes of the intermediate level protection keys used within the
 *    key ladder. It is always equal to 16 bytes for NOCS chipset.
 *
 *  @param[in]  pxL1CipheredProtKey
 *    Buffer, allocated by the caller, containing the first ciphered protection
 *    key fed into the key ladder. It is equal to
 *    L1CipheredProtKey=TDES(L1ProtKey, RootKey).
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    A parameter is invalid
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @retval CECT_ERROR_KEY_PENDING
 *    Operation failed because a key is aleardy pending on the CERT key
 *    interface. Call secUseCertKey() or bsdUseCertKey() to solve this issue.
 *
 *  @note
 *    The content cipher algorithm signaled in the key protection path (KPP)
 *    has to match the EMI of the targeted crypto-engine, i.e. the EMI
 *    parameter of secUseCertKey() or bsdUseCertKey().
*/
typedef TCectStatus (*TCectProcess1LevelProtKey)
(
        TCectKlHandle       xHandle,
        TUnsignedInt8       xKeyProtPath,
        size_t              xCipheredContentKeySize,
  const TUnsignedInt8*     pxCipheredContentKey,
        size_t              xCipheredProtKeySize,
  const TUnsignedInt8*     pxL1CipheredProtKey
);


/**
 *  @ingroup g_c1_kl
 *
 *  @brief
 *    This function requests the CERT block to output a key from a 2-level
 *    key ladder.
 *
 *    The output key available on the CERT key interface may be used then by a
 *    crypto-engine for encryption/decryption operations.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - A key ladder instance has been created (see TCectCreateKeyLadder)
 *
 *  @param[in] xKlHandle
 *    Handle on key ladder instance
 *
 *  @param[in] xKeyProtPath
 *    Key protection path (KPP) to be used for the operation. It is used by the
 *    CECT to select the right CERT key ladder. Refer to [NOCSAD] for KPP values
 *    definitions.
 *
 *  @param[in]  xCipheredContentKeySize
 *    Size in bytes of the ciphered content key. It depends on the cipher content
 *    algorithm signaled in the KPP:
 *    - TDES keying option 2: 2*8=16 bytes
 *    - AES: 16 bytes
 *    - DVB-CSA2: 8 bytes
 *    - DVB-CSA3: 16 bytes
 *    - ASA64: 8 bytes
 *    - ASA128: 16 bytes
 *
 *  @param[in]  pxCipheredContentKey
 *    Buffer, allocated by the caller, containing the ciphered content key. It
 *    is equal to CipheredContentKey=TDES(ContentKey, L1ProtKey).
 *
 *  @param[in]  xCipheredProtKeySize
 *    Size in bytes of the intermediate level protection keys used within the
 *    key ladder. It is always equal to 16 bytes for NOCS chipset.
 *
 *  @param[in]  pxL1CipheredProtKey
 *    Buffer, allocated by the caller, containing the first ciphered protection
 *    key fed into the key ladder. It is equal to
 *    L1CipheredProtKey=TDES(L1ProtKey, L2ProtKey).
 *
 *  @param[in]  pxL2CipheredProtKey
 *    Buffer, allocated by the caller, containing the second ciphered protection
 *    key fed into the key ladder. It is equal to
 *    L2CipheredProtKey=TDES(L2ProtKey, RootKey).
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    A parameter is invalid
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @retval CECT_ERROR_KEY_PENDING
 *    Operation failed because a key is aleardy pending on the CERT key
 *    interface. Call secUseCertKey() or bsdUseCertKey() to solve this issue.
 *
 *  @note
 *    The content cipher algorithm signaled in the key protection path (KPP)
 *    has to match the EMI of the targeted crypto-engine, i.e. the EMI
 *    parameter of secUseCertKey() or bsdUseCertKey().
*/
typedef TCectStatus (*TCectProcess2LevelProtKey)
(
        TCectKlHandle       xHandle,
        TUnsignedInt8       xKeyProtPath,
        size_t              xCipheredContentKeySize,
  const TUnsignedInt8*     pxCipheredContentKey,
        size_t              xCipheredProtKeySize,
  const TUnsignedInt8*     pxL1CipheredProtKey,
  const TUnsignedInt8*     pxL2CipheredProtKey
);


/**
 *  @ingroup g_c1_bae
 *
 *  @brief
 *    This function requests the CERT block to output a key from a 2-level
 *    key ladder.
 *
 *    The output key available on the CERT key interface may be used then by a
 *    crypto-engine for encryption/decryption operations.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - A key ladder instance has been created (see TCectCreateKeyLadder)
 *
 *  @param[in] xKlHandle
 *    Handle on key ladder instance
 *
 *  @param[in] xKeyProtPath
 *    Key protection path (KPP) to be used for the operation. Refer to [NOCSAD]
 *    for KPP values definitions.
 *
 *  @param[in]  xParity
 *    Parity of the first key to output on the CERT key interface
 *
 *  @param[in]  pxEnt
 *    Pointer to the BAE ENT record
 *
 *  @param[in]  pxOpe
 *    Pointer to the BAE OPE record
 *
 *  @param[in]  pxPao
 *    Pointer to the BAE PAO record
 *
 *  @param[in]  pxEcm
 *    Pointer to the BAE ECM record
 *
 *  @param[in]  pxKeys
 *    Pointer to the BAE keys structure
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    A parameter is invalid
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @retval CECT_ERROR_KEY_PENDING
 *    Operation failed because a key is aleardy pending on the CERT key
 *    interface. Call secUseCertKey() or bsdUseCertKey() to solve this issue.
*/
typedef TCectStatus (*TCectProcessBaeKeys)
(
        TCectKlHandle  xHandle,
        TUnsignedInt8  xKeyProtPath,
        TCectParity    xParity,
  const TCectBaeEnt*  pxEnt,
  const TCectBaeOpe*  pxOpe,
  const TCectBaePao*  pxPao,
  const TCectBaeEcm*  pxEcm,
  const TCectBaeKeys* pxKeys
);

/**
 *  @ingroup g_c1_bae
 *
 *  @brief
 *    This function requests the CERT block to output the second key of a BAE
 *    transaction.
 *
 *    The output key available on the CERT key interface may be used then by a
 *    crypto-engine for encryption/decryption operations.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - A key ladder instance has been created (see TCectCreateKeyLadder)
 *    - BAE keys have been processed (see TCectProcessBaeKeys)
 *
 *  @param[in] xKlHandle
 *    Handle on key ladder instance
 *
 *  @param[in]  xParity
 *    Parity of the second key to output on the CERT key interface
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    A parameter is invalid
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @retval CECT_ERROR_KEY_PENDING
 *    Operation failed because a key is aleardy pending on the CERT key
 *    interface. Call secUseCertKey() or bsdUseCertKey() to solve this issue.
*/
typedef TCectStatus (*TCectFinalizeBaeProcessing)
(
  TCectKlHandle  xHandle,
  TCectParity    xParity
);


/**
 *  @ingroup g_c1_la
 *
 *  @brief
 *    This function encrypt a message with the CERT LocalAlgo algorithm in hardware
 *    mode.
 *
 *    This function generates an initialization vector and authentication tag
 *    that must be given back to the decryption function TCectDecryptLocalAlgoHardware.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to encrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the encrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[out] pxInitVector
 *    8-byte buffer containing the initialization vector (IV) computed by the
 *    CERT block. This IV has to be given back to the decryption function
 *    (TCectDecryptLocalAlgoHardware).
 *
 *  @param[out] pxTag
 *    8-byte authentication tag computed by the CERT block. This tag has to be
 *    given back to the decryption function (TCectDecryptLocalAlgoHardware).
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput, pxKeyComp, pxInitVector or pxTag is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectEncryptLocalAlgoHardware)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxKeyComp,
        TUnsignedInt8*  pxInitVector,
        TUnsignedInt8*  pxTag
);


/**
 *  @ingroup g_c1_la
 *
 *  @brief
 *    This function decrypt a message with the CERT LocalAlgo algorithm in hardware
 *    mode.
 *
 *    This function requires an IV and the authentication tag generated by the
 *    encryption TCectEncryptLocalAlgoHardware.
 *
 *  @pre
 *    - The CERT is activated (see TCectActivate / TCectActivateCert3)
 *    - Input message is encrypted with TCectEncryptLocalAlgoHardware
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the initialization vector (IV) computed by the
 *    encryption function.
 *
 *  @param[in] pxTag
 *    8-byte authentication tag computed by the encryption function
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput, pxKeyComp, pxInitVector or pxTag is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed, including message authentication failure.
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectDecryptLocalAlgoHardware)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxKeyComp,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxTag
);


/**
 *  @ingroup g_c1_idea
 *
 *  @brief
 *    This function encrypt a message with the CERT IDEANxT algorithm in
 *    hammer-head (HH) mode.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to encrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the encrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the IDEANxT initialization vector (IV). This
 *    parameter is optional. If it is NULL, message is encrypted in ECB mode.
 *    Otherwise it is encrypted in CBC mode.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[in] pxCipheredHhIdeaNxtKey
 *    32-byte buffer containing the ciphered HHIdeaNxtKey. This buffer also
 *    contain the hammer-head IV and authentication tag:
 *    pxCipheredHhIdeaNxtKey = IV(8) || [HHIdeaNxtKey](16) || TAG(8)
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput, pxKeyComp or pxCipheredHhIdeaNxtKey is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectEncryptIdeaNxtHammerHead)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxKeyComp,
  const TUnsignedInt8*  pxCipheredHhIdeaNxtKey
);


/**
 *  @ingroup g_c1_idea
 *
 *  @brief
 *    This function decrypt a message with the CERT IDEANxT algorithm in
 *    hammer-head (HH) mode.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the IDEANxT initialization vector (IV). This
 *    parameter is optional. If it is NULL, message is decrypted in ECB mode.
 *    Otherwise it is decrypted in CBC mode.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[in] pxCipheredHhIdeaNxtKey
 *    32-byte buffer containing the ciphered HHIdeaNxtKey. This buffer also
 *    contain the hammer-head IV and authentication tag:
 *    pxCipheredHhIdeaNxtKey = IV(8) || [HHIdeaNxtKey](16) || TAG(8)
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput, pxKeyComp or pxCipheredHhIdeaNxtKey is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectDecryptIdeaNxtHammerHead)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxKeyComp,
  const TUnsignedInt8*  pxCipheredHhIdeaNxtKey
);


/**
 *  @ingroup g_c1_idea
 *
 *  @brief
 *    This function decrypt a message with the CERT IDEANxT algorithm in
 *    software mode.
 *
 *    This function requires an IV and the authentication tag generated by the
 *    encryption TCectEncryptLocalAlgoHardware.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the IDEANxT initialization vector (IV). This
 *    parameter is optional. If it is NULL, message is decrypted in ECB mode.
 *    Otherwise it is decrypted in CBC mode.
 *
 *  @param[in] pxSwIdeaNxtKey
 *    16-byte buffer containing the IDEANxT software key.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput or pxSwIdeaNxtKey is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectDecryptIdeaNxtSoftware)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxSwIdeaNxtKey
);


/**
 *  @ingroup g_c1_scwal
 *
 *  @brief
 *    This function decrypt a message with the CERT SCWAL-DATA algorithm in
 *    hardware (HW) mode.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the SCWAL-DATA initialization vector (IV). This
 *    parameter is optional. If it is NULL, message is decrypted in ECB mode.
 *    Otherwise it is decrypted in CBC mode.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[in] xVariant
 *    SCWAL-DATA algorithm variant to be used. The value of the variant is in
 *    range [0..7]
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput or pxKeyComp is NULL
 *    - xVariant is greater than 7
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectDecryptScwalDataHardware)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxKeyComp,
        TUnsignedInt8    xVariant
);


/**
 *  @ingroup g_c1_scwal
 *
 *  @brief
 *    This function decrypt a message with the CERT SCWAL-DATA algorithm in
 *    hammer-head (HH) mode.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxInput
 *    Buffer containing the input message to decrypt.
 *
 *  @param[out]  pxOutput
 *    Buffer where to write the decrypted message.
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of messages (pxInput, pxOutput) to operate on. It must be a
 *    multiple of 8.
 *
 *  @param[in] pxInitVector
 *    8-byte buffer containing the SCWAL-DATA initialization vector (IV). This
 *    parameter is optional. If it is NULL, message is decrypted in ECB mode.
 *    Otherwise it is decrypted in CBC mode.
 *
 *  @param[in] pxKeyComp
 *    2-byte key component
 *
 *  @param[in] pxCipheredHhScwalKey
 *    32-byte buffer containing the ciphered HHSCWALKey. This buffer also
 *    contain the hammer-head IV and authentication tag:
 *    pxCipheredHhScwalKey = IV(8) || [HHSCWALKey](16) || TAG(8)
 *
 *  @param[in] xVariant
 *    SCWAL-DATA algorithm variant to be used. The value of the variant is in
 *    range [0..7]
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 8
 *    - pxInput, pxOutput, pxKeyComp or pxCipheredHhScwalKey is NULL
 *    - xVariant is greater than 7
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
 *    - In-place operation (pxOutput=pxIntput) is supported
*/
typedef TCectStatus (*TCectDecryptScwalDataHammerHead)
(
        TUnsignedInt8*  pxInput,
        TUnsignedInt8*  pxOutput,
        size_t           xMessageSize,
  const TUnsignedInt8*  pxInitVector,
  const TUnsignedInt8*  pxKeyComp,
  const TUnsignedInt8*  pxCipheredHhScwalKey,
        TUnsignedInt8    xVariant
);


/**
 *  @ingroup g_c1_cushash
 *
 *  @brief
 *    This function computes a hash on a message with the CERT CusHash algorithm.
 *    This function is available only in \b CERT1.
 *
 *  @pre
 *    The CERT is activated (see TCectActivate)
 *
 *  @param[in]  pxMessage
 *    Buffer containing the message to hash
 *
 *  @param[in]  xMessageSize
 *    Size in bytes of the message to operate on. It must be a multiple of 16.
 *
 *  @param[out]  pxHash
 *    16-byte buffer where to write the hash computed
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    - xMessageSize is not a multiple of 16
 *    - pxMessage or pxHash is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed
 *
 *  @note
 *    - All buffers must be allocated by the caller
*/
typedef TCectStatus (*TCectProcessCusHash)
(
  const TUnsignedInt8*  pxMessage,
        size_t           xMessageSize,
        TUnsignedInt8*  pxHash
);


/**
 *  @ingroup g_c1_perso
 *
 *  @brief
 *    This function programs the CERT SEG OTA record. This operation writes
 *    OTP memory and is non-reversible. If the operation is successful, it
 *    requires a reboot of the hardware platform.
 *
 *    This function requires an unique and/or a global segmentation descriptor.
 *    Descriptor format depends on the targeted hardware.
 *    - For a \b CERT1 hardware block, here is the descriptor format
 *      (see CAS_IrdPersoData):
 *
 * <pre>
 *         tag            (1) = B0h
 *         length         (1) = 10h
 *         key_comp       (2)
 *         seguid         (2)
 *         segnr          (4)
 *         integra        (8)
 * </pre>
 *    Unique and global descriptors are identified thanks to the \e key_comp
 *    field.
 *
 *
 *    - If target hardware is a \b CERT3 block, then it will program one of the
 *      following records according to the tag and content of the descriptor:
 *      + SEG CERT1 (legacy crypto),
 *      + SEG0 CERT3, SEG1 CERT3 or SEG2 CERT3 (new crypto).
 *      .
 *
 *      The descriptor format is the following one (see CAS_IrdPersoData):
 * <pre>
 *         tag              (1) = \b B1h for Legacy crypto, \b B3h for new crypto
 *         length           (1) = 1Ch
 *         prev_seguid      (2) = 0000h if Legacy crypto
 *         seguid           (2)
 *         seg_record{
 *           iv_type{
 *             iv           (5)
 *             struct_id    (1)
 *             key_comp     (2)
 *           }
 *           ciphered_data{
 *             filler       (2) = 0000h
 *             seguid       (2)
 *             he_seg       (4)
 *           }
 *           auth_tag       (8)
 *         }
 * </pre>
 *
 *  @param[in] xDescriptorsSize
 *    Size in bytes of pxDescriptors buffer.
 *
 *  @param[in] pxDescriptors
 *    Buffer, allocated by the caller, containing a collection of TLV descriptors
 *    including the segmentation descriptor to program in OTP memory.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    pxDescriptors is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed, including record authentication failure.
 *
 *  @retval CECT_ERROR_OTP_LOCKED
 *    The SEG record is locked and no longer modifiable
 *
 *  @note
 *    It is not necessary to activate the CERT block before calling this
 *    function.
*/
typedef TCectStatus (*TCectProgramSeg)
(
        size_t              xDescriptorsSize,
  const TUnsignedInt8*     pxDescriptors
);

/**
 *  @ingroup g_c1_perso
 *
 *  @brief
 *    This function programs the CERT OTAKeys record. This operation writes
 *    OTP memory and is non-reversible. If the operation is successful, it
 *    requires a reboot of the hardware platform.
 *
 *  @param[in]  pxOtaKeys
 *    Buffer containing the record to program in CERT OTP memory.
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR_BAD_PARAMETER
 *    pxOtaKeys is NULL
 *
 *  @retval CECT_ERROR
 *    Operation failed, including record authentication failure.
 *
 *  @retval CECT_ERROR_OTP_LOCKED
 *    The SEG record is locked and no longer modifiable
 *
 *  @note
 *    It is not necessary to activate the CERT block before calling this
 *    function.
*/
typedef TCectStatus (*TCectProgramOtaKeys)
(
  TCectOtaKeys*  pxOtaKeys
);

/**
 *  @ingroup g_c3_act
 *
 *  @brief
 *    This function activates the \b CERT3 block.
 *
 *    Several types of descriptors can are managed and needed by this function to
 *    activate the CERT3 block:
 *
 *    - A \b CERT3 activation descriptor is mandatory. It must comply with the following
 *    format (see CAS_IrdPersoData):
 *
 *      | Field            | Size (B)         |Value|
 *      |:-----------------|-----------------:|----:|
 *      | tag              |       1          | A3h |
 *      | length           |       4          |     |
 *      | key_comp         |       2          |     |
 *      | struc_id         |       1          | D1h |
 *      | filler           |       1          | 00h |
 *      | act_bitmap       |       4          |     |
 *      | config_version   |       2          |     |
 *      | act_version      |       2          |     |
 *      | assertion_list   |      16          |     |
 *      | actset_list_size |       4          |     |
 *      | actset_list_byte | actset_list_size |     |
 *      | integra          |      16          |     |
 *
 *    - It also requires an AKL key component descriptor that will be used for all
 *    key ladder operations. This descriptor must be given as a CSC data descriptor
 *    to the function and comply with the following format (see CAS_IrdPersoData):
 *
 *      | Field            | Size (B)|Value|
 *      |:-----------------|--------:|----:|
 *      | tag              |       1 | 31h |
 *      | length           |       1 | 02h |
 *      | key_comp         |       2 |     |
 *
 *    - A \b CERT3 device certificate descriptor can also be conveyed as a
 *    CSC data descriptor. It is optionnal, and must comply with the following format
 *    (see CAS_IrdPersoData):
 *
 *      | Field                 | Size (B)| Value |
 *      |:----------------------|--------:|------:|
 *      | tag                   |       1 |   30h |
 *      | length                |       1 |   34h |
 *      | iv                    |      12 |       |
 *      | filler1               |       2 | 0000h |
 *      | certificate_sn        |       8 |       |
 *      | device_manufacturer_id|       4 |       |
 *      | device_model_id       |       8 |       |
 *      | device_sn             |       8 |       |
 *      | filler2               |       2 | 0000h |
 *      | capabilities          |       2 |       |
 *      | mac                   |      16 |       |
 *
 *  @param[in] xCscDataDescriptorsSize
 *    Size in bytes of pxCscDataDescriptors buffer.
 *
 *  @param[in] pxCscDataDescriptors
 *    Buffer, allocated by the caller, containing a collection of TLV
 *    descriptors coming from CSCData (OTP). These descriptors include
 *    the cert device certificate descriptor.
 *
 *  @param[in] xActivationRecordDescriptorSize
 *    Size in bytes of pxActivationRecordDescriptor buffer.
 *
 *  @param[in] pxActivationRecordDescriptor
 *    Buffer, allocated by the caller, containing the cert activation descriptor
 *
 *  @retval CECT_NO_ERROR
 *    Operation completed successfully
 *
 *  @retval CECT_ERROR
 *    Operation failed
*/
typedef TCectStatus (*TCectActivateCert3)
(
        size_t              xCscDataDescriptorsSize,
  const TUnsignedInt8*     pxCscDataDescriptors,
        size_t              xActivationRecordDescriptorSize,
  const TUnsignedInt8*     pxActivationRecordDescriptor
);

/**
 * @brief
 *   This function resets the AKL CERT IP block.
 *
 * @retval CECT_NO_ERROR
 *   The AKL CERT IP block has been reset successfully
 *
 * @retval CECT_ERROR_NOT_AVAILABLE
 *   The AKL CERT IP block reset operation is not available
 *
 * @retval CECT_ERROR
 *   Reset operation failed
 *
 * @note
 *   This function can be called from REE and TEE as long as the TEE privileged mode
 *   is not enabled. It can be called from TEE only, as soon as the TEE privileged
 *   mode is enabled.
*/
typedef TCectStatus(*TCectResetAkl)
(
  void
);

/**
 *  @ingroup g_c1_fct_table
 *
 *  @brief
 *    Structure defining the CERT driver function table.
 */
typedef struct
{
  TUnsignedInt32 cectApiVersion;
   /**< Version of the CECT API. Use the macro CECTAPI_VERSION_INT to assign the
    *   the right value.
    */
  TCectSystemGetInfo cectSystemGetInfo;
   /**< Get hardware block system information (public data)
    */
  TCectActivate cectActivate;
   /**< Activate the \b CERT1 block
    */
  TCectCreateKeyLadder cectCreateKeyLadder;
   /**< Create a key ladder instance
    */
  TCectDisposeKeyLadder cectDisposeKeyLadder;
   /**< Dipose of a key ladder instance
    */
  TCectProcessRootLevelProtKey cectProcessRootLevelProtKey;
   /**< Process a root level protection key
    */
  TCectProcess1LevelProtKey cectProcess1LevelProtKey;
   /**< Process a 1-level protection key
    */
  TCectProcess2LevelProtKey cectProcess2LevelProtKey;
   /**< Process a 2-level protection key
    */
  TCectProcessBaeKeys cectProcessBaeKeys;
   /**<  Process BAE keys
    */
  TCectFinalizeBaeProcessing cectFinalizeBaeProcessing;
   /**< Finalize BAE processing
    */
  TCectEncryptLocalAlgoHardware cectEncryptLocalAlgoHardware;
   /**< Encrypt data with LocalAlgo in hardware mode
    */
  TCectDecryptLocalAlgoHardware cectDecryptLocalAlgoHardware;
   /**< Decrypt data with LocalAlgo in hardware mode
    */
  TCectEncryptIdeaNxtHammerHead cectEncryptIdeaNxtHammerHead;
   /**< Encrypt data with IDEANxT in hammer-head mode
    */
  TCectDecryptIdeaNxtHammerHead cectDecryptIdeaNxtHammerHead;
   /**< Decrypt data with IDEANxT in hammer-head mode
    */
  TCectDecryptIdeaNxtSoftware cectDecryptIdeaNxtSoftware;
   /**< Decrypt data with IDEANxT in software mode
    */
  TCectDecryptScwalDataHardware cectDecryptScwalDataHardware;
   /**< Decrypt data with SCWAL-DATA in hardware mode
    */
  TCectDecryptScwalDataHammerHead cectDecryptScwalDataHammerHead;
   /**< Decrypt data with SCWAL-DATA in hammer-head mode
    */
  TCectProcessCusHash cectProcessCusHash;
   /**< Process a custom hash
    */
  TCectProgramSeg cectProgramSeg;
   /**< Program SEG record
    */
  TCectProgramOtaKeys cectProgramOtaKeys;
   /**< Program OTAKeys record
    */
  TCectActivateCert3 cectActivateCert3;
   /**< Activate the \b CERT3 block
    */
  TCectResetAkl cectResetAkl;
   /**< Reset the \b CERT block
    */
} TCectFunctionTable;


/**
 *  @ingroup g_c2_fct_table
 *
 *  @brief
 *    Structure defining the CERT2 driver function table.
 */
typedef struct
{
  TUnsignedInt32 cectApiVersion;
   /**< Version of the CECT API. Use the macro CECTAPI_VERSION_INT to assign the
    *   the right value.
    */

  TCectStatus (*getSystemInfo)
  (
    TC2ctSystem*  pxSystem
  );
  /**<
    @ingroup g_c2_config

    @brief
      This function is used to read system information (public data) of the CERT
      block.

    @param[out] pxSystem
      Pointer to system object allocated by the caller and to be filled in with
      CERT system information.

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR
      Operation failed
  */

  TCectStatus (*getLeafCertificate)
  (
    TUnsignedInt8* pxCertificate
  );
  /**<
    @ingroup g_c2_pki

    @brief
      This function returns the 224-byte CERT leaf certificate.

    @pre
      None

    @param[out] pxCertificate
      224-byte buffer, allocated by the caller, where to write the CERT leaf
      certificate.

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      A parameter is invalid

    @retval CECT_ERROR
      Operation failed

    @retval CECT_ERROR_KEY_PENDING
      Operation failed because a key is already pending on the CERT key
      interface.
  */

  TCectStatus (*processCertificates)
  (
    const TUnsignedInt8* pxCertificates,
          size_t          xCertificatesSize,
    const TUnsignedInt8* pxKi2,
          TUnsignedInt8* pxRootKey
  );
  /**<
    @ingroup g_c2_pki

    @brief
      This function requests the CERT block to validate the certificate chain,
      authenticate and decrypt Ki2 and return the NKL root key in protected form.

      Output keys available on the CERT key interface may be used then by a
      crypto-engine for RAM2RAM operations.

    @pre
      None

    @param[in] pxCertificates
      Buffer, allocated by the caller, containing a chain of certificates
      (Intermediate||Leaf).

    @param[in]  xCertificatesSize
      Size in bytes of the certificate chain. It must be a multiple of 32 bytes.

    @param[in] pxKi2
      160-byte buffer, allocated by the caller, containing the signed and ciphered
      Ki2 key.

    @param[out] pxRootKey
      48-byte buffer, allocated by the caller, where to write the NKL root key.

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      A parameter is invalid

    @retval CECT_ERROR
      Operation failed

    @retval CECT_ERROR_KEY_PENDING
      Operation failed because a key is already pending on the CERT key
      interface.
  */

  TCectStatus (*createKeyLadder)
  (
    TCectKlHandle*          pxHandle
  );
   /**<
    @ingroup g_c2_nkl

    @brief
      This function creates a key ladder instance

    @pre
     None

    @param[out] pxHandle
      Handle assigned to the key ladder instance

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR
      - Operation failed

    @warning
      In order to avoid a deadlock, it is not authorized to have several key
      ladder instances created from the \b same thread. A key ladder instance
      has to be disposed of as soon as possible.
  */

  TCectStatus (*disposeKeyLadder)
  (
    TCectKlHandle xHandle
  );
  /**<
    @ingroup g_c2_nkl

    @brief
      This function disposes of a key ladder instance

    @param[out] xHandle
      Handle on key ladder instance

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      Handle provided is invalid or unknown.

    @retval CECT_ERROR
      Operation failed

    @warning
      It is important to dispose of a key ladder instance as soon as possible in
      order to unlock the underlying CERT resource.
  */

  TCectStatus (*processRootLevelProtKeys)
  (
          TCectKlHandle       xHandle,
          TUnsignedInt16      xEmi,
          TBoolean            xSwapPolarity,
    const TUnsignedInt8*     pxConfig,
    const TUnsignedInt8*     pxRootKey,
    const TUnsignedInt8*     pxKeyComp
  );
  /**<
    @ingroup g_c2_nkl

    @brief
      This function requests the CERT block to output keys (odd & even) from
      a root level key ladder.

      Output keys available on the CERT key interface may be used then by a
      crypto-engine for RAM2RAM operations.

    @pre
      - A key ladder instance has been created (see TC2ctFunctionTable::createKeyLadder)

    @param[in] xKlHandle
      Handle on key ladder instance

    @param[in] xEmi
      Encryption Method Indicator identifying the scrambling algorithm

    @param[in] xSwapPolarity
      When set to TRUE, even and odd keys are swapped before being output on the
      CERT key interface.

    @param[in] pxConfig
      26-byte buffer, allocated by the caller, containing the NKL configuration
      message complying with the following format:
      - IV(8)||Data(8)||Tag(8)||KeyCompConf(2)

    @param[in] pxRootKey
      48-byte buffer, allocated by the caller, containing the NKL root key
      returned by @link TC2ctFunctionTable::processCertificates processCertificates
      @endlink in protected form. This root key is also known as PRMKEY1. This field
      shall be set to NULL if not used. Note that pxKeyComp and pxRootKey may not
      be both set to NULL.

    @param[in] pxKeyComp
      2-byte buffer, allocated by the caller, containing the NKL key component.
      This field shall be set to NULL if not used. Note that pxKeyComp and pxRootKey
      may not be both set to NULL.

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      A parameter is invalid

    @retval CECT_ERROR
      Operation failed

    @retval CECT_ERROR_KEY_PENDING
      Operation failed because a key is already pending on the CERT key
      interface.
  */

  TCectStatus (*process1LevelProtKeys)
  (
          TCectKlHandle       xHandle,
          TUnsignedInt16      xEmi,
          TBoolean            xSwapPolarity,
    const TUnsignedInt8*     pxConfig,
    const TUnsignedInt8*     pxRootKey,
    const TUnsignedInt8*     pxKeyComp,
    const TUnsignedInt8*     pxCipheredEvenContentKey,
    const TUnsignedInt8*     pxCipheredOddContentKey,
          size_t              xCipheredContentKeySize,
    const TUnsignedInt8*     pxL1CipheredProtKey,
          size_t              xL1CipheredProtKeySize
  );
  /**<
    @ingroup g_c2_nkl

    @brief
      This function requests the CERT block to output keys (odd & even) from
      a 1-level key ladder.

      Output keys available on the CERT key interface may be used then by a
      crypto-engine for encryption/decryption operations.

    @pre
      - A key ladder instance has been created (see TC2ctFunctionTable::createKeyLadder)

    @param[in] xKlHandle
      Handle on key ladder instance

    @param[in] xEmi
      Encryption Method Indicator identifying the scrambling algorithm

    @param[in] xSwapPolarity
      When set to TRUE, even and odd keys are swapped before being output on the
      CERT key interface.

    @param[in] pxConfig
      26-byte buffer, allocated by the caller, containing the NKL configuration
      message complying with the following format:
      - IV(8)||Data(8)||Tag(8)||KeyCompConf(2)

    @param[in] pxRootKey
      48-byte buffer, allocated by the caller, containing the NKL root key
      returned by @link TC2ctFunctionTable::processCertificates processCertificates
      @endlink in protected form. This root key is also known as PRMKEY1. This field
      shall be set to NULL if not used. Note that pxKeyComp and pxRootKey may not
      be both set to NULL.

    @param[in] pxKeyComp
      2-byte buffer, allocated by the caller, containing the NKL key component.
      This field shall be set to NULL if not used. Note that pxKeyComp and pxRootKey
      may not be both set to NULL.

    @param[in]  pxCipheredEvenContentKey
      Buffer, allocated by the caller, containing the ciphered even content key.

    @param[in]  pxCipheredOddContentKey
      Buffer, allocated by the caller, containing the ciphered odd content key.

    @param[in]  xCipheredContentKeySize
      Size in bytes of the ciphered content keys. It depends on the scrambling
      algorithm signaled by xEmi:
      - TDES keying option 2: 2*8=16 bytes
      - AES: 16 bytes
      - DVB-CSA2: 8 bytes
      - DVB-CSA3: 16 bytes
      - ASA64: 8 bytes
      - ASA128: 16 bytes

    @param[in]  pxL1CipheredProtKey
      Buffer, allocated by the caller, containing the ciphered protection
      key fed into the key ladder.

    @param[in]  xL1CipheredProtKeySize
      Size in bytes of the intermediate level protection keys used within the
      key ladder.It depends on the algorithm of the update key ladder stage
      block:
      - AES: 16 bytes
      - TDES: 16 bytes
      - HH: 32 bytes
      - AES-GCM: 48 bytes

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      A parameter is invalid

    @retval CECT_ERROR
      Operation failed

    @retval CECT_ERROR_KEY_PENDING
      Operation failed because a key is already pending on the CERT key
      interface.
  */

  TCectStatus (*process2LevelProtKeys)
  (
          TCectKlHandle       xHandle,
          TUnsignedInt16      xEmi,
          TBoolean            xSwapPolarity,
    const TUnsignedInt8*     pxConfig,
    const TUnsignedInt8*     pxRootKey,
    const TUnsignedInt8*     pxKeyComp,
    const TUnsignedInt8*     pxCipheredEvenContentKey,
    const TUnsignedInt8*     pxCipheredOddContentKey,
          size_t              xCipheredContentKeySize,
    const TUnsignedInt8*     pxL1CipheredProtKey,
          size_t              xL1CipheredProtKeySize,
    const TUnsignedInt8*     pxL2CipheredProtKey,
          size_t              xL2CipheredProtKeySize
  );
  /**<
    @ingroup g_c2_nkl

    @brief
      This function requests the CERT block to output keys (odd & even) from
      a 2-level key ladder.

      Output keys available on the CERT key interface may be used then by a
      crypto-engine for encryption/decryption operations.

    @pre
      - A key ladder instance has been created (see TC2ctFunctionTable::createKeyLadder)

    @param[in] xKlHandle
      Handle on key ladder instance

    @param[in] xEmi
      Encryption Method Indicator identifying the scrambling algorithm

    @param[in] xSwapPolarity
      When set to TRUE, even and odd keys are swapped before being output on the
      CERT key interface.

    @param[in] pxConfig
      26-byte buffer, allocated by the caller, containing the NKL configuration
      message complying with the following format:
      - IV(8)||Data(8)||Tag(8)||KeyCompConf(2)

    @param[in] pxRootKey
      48-byte buffer, allocated by the caller, containing the NKL root key
      returned by @link TC2ctFunctionTable::processCertificates processCertificates
      @endlink in protected form. This root key is also known as PRMKEY1. This field
      shall be set to NULL if not used. Note that pxKeyComp and pxRootKey may not
      be both set to NULL.

    @param[in] pxKeyComp
      2-byte buffer, allocated by the caller, containing the NKL key component.
      This field shall be set to NULL if not used. Note that pxKeyComp and pxRootKey
      may not be both set to NULL.

    @param[in]  pxCipheredEvenContentKey
      Buffer, allocated by the caller, containing the ciphered even content key.

    @param[in]  pxCipheredOddContentKey
      Buffer, allocated by the caller, containing the ciphered odd content key.

    @param[in]  xCipheredContentKeySize
      Size in bytes of the ciphered content keys. It depends on the scrambling
      algorithm signaled by xEmi:
      - TDES keying option 2: 2*8=16 bytes
      - AES: 16 bytes
      - DVB-CSA2: 8 bytes
      - DVB-CSA3: 16 bytes
      - ASA64: 8 bytes
      - ASA128: 16 bytes

    @param[in]  pxL1CipheredProtKey
      Buffer, allocated by the caller, containing the first ciphered protection
      key fed into the key ladder.

    @param[in]  xL1CipheredProtKeySize
      Size in bytes of the first intermediate level protection keys used within the
      key ladder. It depends on the algorithm of the update key ladder stage
      block:
      - AES: 16 bytes
      - TDES: 16 bytes
      - HH: 32 bytes
      - AES-GCM: 48 bytes

    @param[in]  pxL2CipheredProtKey
      Buffer, allocated by the caller, containing the second ciphered protection
      key fed into the key ladder.

    @param[in]  xL2CipheredProtKeySize
      Size in bytes of the second intermediate level protection keys used within the
      key ladder. It depends on the algorithm of the update key ladder stage
      block:
      - AES: 16 bytes
      - TDES: 16 bytes
      - HH: 32 bytes
      - AES-GCM: 48 bytes

    @retval CECT_NO_ERROR
      Operation completed successfully

    @retval CECT_ERROR_BAD_PARAMETER
      A parameter is invalid

    @retval CECT_ERROR
      Operation failed

    @retval CECT_ERROR_KEY_PENDING
      Operation failed because a key is already pending on the CERT key
      interface.
  */



} TC2ctFunctionTable;


/******************************************************************************/
/*                                                                            */
/*                             FUNCTION PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @ingroup g_c1_fct_table
 *
 *  @brief
 *    This function returns a pointer to the CERT Command Translator function
  *   table if successful, NULL in case of error.
 *
*/
NV_INTERNAL_API const TCectFunctionTable* cectGetFunctionTable
(
  void
);

/**
 *  @ingroup g_c2_fct_table
 *
 *  @brief
 *    This function returns a pointer to the CERT2 Command Translator function
  *   table if successful, NULL in case of error.
 *
*/
const TC2ctFunctionTable* c2ctGetFunctionTable
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* CA_CECT_H */

/* END OF FILE */
