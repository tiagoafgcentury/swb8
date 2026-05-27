/**
** @file ca_cak.h
**
** @brief
**    Conditional Access Kernel external definitions.
**
**    The syntax of the comments of this file is compliant to the format used by
**    Doxygen to generate html documentation from source and header files.
**    Doxygen can be downloaded from www.doxygen.org and is provided under GNU
**    General Public License.
**
** COPYRIGHT:
**   2001-2023 Nagravision S.A.
**
** CONFIGURATION MANAGEMENT:
**   $Id: //CAK/components/cakapi/TAGS/CAKAPI_1_40_0/src/com/ca_cak.h#1 $
**   $Change: 328639 $
**
** CLASSIFICATION:
**   CONFIDENTIAL
**
*/

#ifndef CA_CAK_H
#define CA_CAK_H

#ifdef __cplusplus
extern "C" {
#endif

/** @{ */
#define CAKAPI_VERSION_MAJOR  6
#define CAKAPI_VERSION_MEDIUM 14
#define CAKAPI_VERSION_MINOR  0
/** @} */

/** @page p_history Changes history
 *  - <b> 6.14.0 - 15-Dec-2023 </b>
 *    - Added CA_ACCESS_DENIED_USAGE_RULES_TRANSITION
 *    - Added caSystemGetNagraTaVersion
 *    - Added caRequestSetCaSystemId function
 *    - Added caSetDbDumpInterface function
 *    - Updated EMI document version (1.0.26)
 *
 *  - <b> 6.13.0 - 13-Dec-2022 </b>
 *    - Added CA_REQUEST_TYPE_DATABASE_DUMP request
 *
 *  - <b> 6.12.0 - 13-Sep-2021 </b>
 *    - Added caMpegFilterCfgGetPpid
 *    - Updated descriptions of existing functions (typos, missing information).
 *
 *  - <b> 6.11.0 - 23-Mar-2021 </b>
 *    - Added CA_ACCESS_DENIED_SOFTWARE_UPGRADE_REQUIRED
 *
 *  - <b> 6.10.1 - 30-Sep-2020 </b>
 *    - Added caPullEmmContextGetPpid
 *    - Updated the format returned by caPullEmmContextGetSerialNumber
 *    - caResume can return CA_ERROR_CA_NOT_RUNNING when an issue is detected in operator data
 *
 *  - <b> 6.10.0 - 24-Apr-2020 </b>
 *    - Added CA_REQUEST_TYPE_PULL_EMM_GET_CONTEXT
 *    - Added object caPullEmmContex
 *    - Added caPullEmmContextGetToken
 *    - Added caPullEmmContextGetSignature
 *    - Added caPullEmmContextGetSerialNumber
 *    - Added caPullEmmContextGetPrivateData
 *    - Added CA_REQUEST_TYPE_PULL_EMM_PROCESSING
 *    - Added caRequestSetPullEmmContext
 *    - Added CA_LISTENER_TYPE_PULL_EMM
 *
 *  - <b> 6.9.2 - 30-Mar-2020 </b>
 *    - caStartUp can return CA_ERROR_CA_NOT_RUNNING when an issue is detected in operator data
 *    - Updated description of caStartUp regarding functions to be called before.
 *
 *  - <b> 6.9.1 - 11-Jul-2019 </b>
 *    - caSetPersoDataPath does not require a read only path.
 *    - Doxygen document: added nagravision watermark software interface specification.
 *
 *  - <b> 6.9.0 - 11-Jul-2019 </b>
 *    - Added caSetWmkSwInterface
 *    - caSystemGetUpgradeWarningDelay is deprecated
 *    - CA_SYSTEM_SOFTWARE_UPGRADE_RECOMMENDED is deprecated
 *
 *  - <b> 6.8.0 - 15-Nov-2018 </b>
 *    - Added caSetTrustedStoragePath
 *    - Added CA_ACCESS_DENIED_DEVICE_ACTIVATION_REQUIRED
 *    - Added CA_ACCESS_CORRUPTED_ECM
 *    - Added CA_REQUEST_TYPE_FILTERED_EMM_PROCESSING
 *    - caRequestSetBuffer can be used with a program descrambling request
 *    - Added TCaMpegFilterCfg object
 *    - Added caSystemGetEcmMpegFilterCfgs
 *    - Added caSystemGetEmmMpegFilterCfgs
 *    - Added caMpegFilterCfgGetCaSystemId
 *    - Added caMpegFilterCfgGetPattern
 *    - Added caRequestSetMpegFilterCfg
 *    - Typo corrections
 *
 *  - <b> 6.7.0 - 09-May-2018 </b>
 *    - Added caSetPersoDataPath
 *    - Added caSetStoragePath
 *    - Added caRequestSetUserIntent
 *    - Added caRequestSetUsageRulesStatus
 *    - Added caRequestSetSecureMediaPath
 *    - Added caProgramGetKeySlotId
 *    - Added CA_ACCESS_USAGE_RULES_VALIDATION_REQUIRED and CA_ACCESS_DENIED_USAGE_RULES_VIOLATION
 *
 *  - <b> 6.6.0 - 24-May-2017 </b>
 *    - Add flag CA_SMARTCARD_TSIO_CAPABLE
 *
 *  - <b> 6.5.0 - 09-Jan-2017 </b>
 *    - Updated description of caRequestSetEmi() which is compatible with request
 *      #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *
 *  - <b> 6.4.1 - 20-Jul-2016 </b>
 *    - Ensure compatibility with 64-bit version of Windows.
 *    - Added CA_PRODUCT_FREE_PERIOD and CA_PRODUCT_VOUCHER
 *    - Add precision to comment of caSmartcardGetDate()
 *
 *  - <b> 6.4.0 - 26-Jan-2016 </b>
 *    - Added caSmartcardGetDate()
 *
 *  - <b> 6.3.0 - 22-Jan-2014 </b>
 *    - Added caPurchaseGetUid()
 *    - Added caRequestSetPurchaseUid()
 *    - Updated description of caRequestSetTransportSessionId() which is compatible with request
 *      #CA_REQUEST_TYPE_EMM_FILTERING
 *    - Updated description of caRequestSetRefurbishmentData since returned values were not of
 *      the valid kind.
 *
 *  - <b> 6.2.0 - 19-Mar-2014 </b>
 *    - Added #CA_LISTENER_TYPE_CHIPSET_HARDWARE_RESET.
 *
 *  - <b> 6.1.3 - 04-Dec-2013 </b>
 *    - Updated description of caRequestSetPriority() to handle up to 256
 *      priorities.
 *
 *  - <b> 6.1.2 - 24-Oct-2013 </b>
 *    - Added caEventGetWinnerPurchase()
 *    - Added caEventGetGrantingPurchases()
 *    - Added caRequestSetStreamProcMode()
 *    - Added caRequestSetEcmStreamId()
 *    - Added caSmartcardGetMaxEcmStream()
 *    - Added caRequestSetEmi()
 *    - Added #CA_ACCESS_DENIED_TSIO
 *    - Added caRequestSetPriority()
 *
 *  - <b> 6.0.2 - 10-Jul-2013 </b>
 *    - Added #CA_PRODUCT_A_LA_CARTE
 *
 *  - <b> 6.0.1 - 08-Avr-2013 </b>
 *    - caExportationGetObjects() can now return #CA_OBJECT_NOT_AVAILABLE
 *    - Added #CA_SMARTCARD_REMOVABLE flag
 *
 *  - <b> 6.0.0 - 11-Jul-2012 </b>
 *    - Added caPause() and caResume()
 *    - Added #CA_LISTENER_TYPE_POPUP
 *    - Added caProgramGetAccessErrorCode()
 *    - Added #CA_PRODUCT_RECURRENT_PERIOD_SUB
 *    - Updated layout to generate .chm file with doxygen.
*/

/** @mainpage Overview
 *  - @subpage p_history
 *  - @subpage p_preface
 *
 *  <hr>Copyright &copy; 2011-2023 Nagravision. All rights reserved.\n
 *  CH-1033 Cheseaux, Switzerland\n
 *  Tel: +41 21 732 03 11  Fax: +41 21 732 01 00\n
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

/** @page p_preface Preface
 *  <h2>Objectives</h2>
 *  This document defines the NagraVision Conditional Access Kernel Application
 *  Programming Interface. It
 *  This document is the CAK API Reference Guide.- It provides a detail
 *  specification of all types and functions.
 *
 *  <hr><h2>Audience</h2>
 *  This document is mainly intended for set-top box manufacturers or middleware
 *  providers in charge of integrating the Conditional Access.
 *
 * <hr><h2>References</h2>
 *  - \anchor REF_EMI [EMI] Nagravision, System Architecture, Encryption Method Indicator
 *    Specification, Version 1.0.26\n
*/

/******************************************************************************/
/*                                                                            */
/*                              GROUPS DEFINITION                             */
/*                                                                            */
/******************************************************************************/

/** @defgroup g_cak_state CAK State Life Cycle */
/** @defgroup g_request Generic Request Framework */
/** @{
 *  @ingroup g_request
*/
/** @defgroup g_request_types Types & Definitions */
/** @defgroup g_request_main Main Functions */
/** @defgroup g_request_setter Setters */
/** @defgroup g_listener Listeners & Exportations*/
/** @defgroup g_object CA Classes */
/** @} */
/** @{
 *  @ingroup g_object
*/
/** @defgroup g_connection caConnection */
/** @defgroup g_content caContent */
/** @defgroup g_credit caCredit */
/** @defgroup g_event caEvent */
/** @defgroup g_mpeg_filter_configuration caMpegFilterCfg */
/** @defgroup g_irdcmd caIrdCommand */
/** @defgroup g_key caKey */
/** @defgroup g_unique_key caUniqueKey */
/** @defgroup g_random_key caRandomKey */
/** @defgroup g_operator caOperator */
/** @defgroup g_pin caPinCode */
/** @defgroup g_popup caPopup */
/** @defgroup g_price caPrice */
/** @defgroup g_product caProduct */
/** @defgroup g_program caProgram */
/** @defgroup g_purchase caPurchase */
/** @defgroup g_recharge caRecharge */
/** @defgroup g_server caServer */
/** @defgroup g_service caService */
/** @defgroup g_signature caSignature */
/** @defgroup g_card caSmarcard */
/** @defgroup g_stblock caStbLock */
/** @defgroup g_system caSystem */
/** @} */
/** @defgroup g_log Log */
/** @defgroup g_func_request Legacy Functional Request */


/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"
#include "ca_defsx.h"
#include "ca_mpeg.h"
#include "ca_dvb.h"
#include "nv_acd.h"
#include "nv_wmk_sw.h"

/******************************************************************************/
/*                                                                            */
/*                              VERSION TOOL                                  */
/*                                                                            */
/******************************************************************************/

#ifndef CAK_TOOL_VERSION
#define CAK_TOOL_STRINGIFY(s) CAK_TOOL_TOSTRING(s)
#define CAK_TOOL_TOSTRING(s)  #s

#define CAK_TOOL_VERSION_INT(a, b, c) (a<<16 | b<<8 | c)
#define CAK_TOOL_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define CAK_TOOL_VERSION(a, b, c) CAK_TOOL_VERSION_DOT(a, b, c)
#endif

#define CAKAPI_VERSION_INTER CAK_TOOL_VERSION(CAKAPI_VERSION_MAJOR, \
              CAKAPI_VERSION_MEDIUM, \
              CAKAPI_VERSION_MINOR)

#define CAKAPI_VERSION_INT   CAK_TOOL_VERSION_INT(CAKAPI_VERSION_MAJOR, \
            CAKAPI_VERSION_MEDIUM, \
            CAKAPI_VERSION_MINOR)

#define CAKAPI_VERSION_STRING  "CAKAPI_" CAK_TOOL_STRINGIFY(CAKAPI_VERSION_INTER)


/******************************************************************************/
/*                                                                            */
/*                                     TYPES                                  */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    Program provider identifier.
*/
typedef TUnsignedInt16 TPpid;


/**
 *  @ingroup g_credit
 *
 *  @brief
 *    Credit amount, used for credit records and purchase prices.
*/
typedef TUnsignedInt32 TCreditAmount;

/**
 *  @brief
 *    Type used to represent the mask of the flags used by several object to
 *    detail flags validity.
*/
typedef TUnsignedInt32 TFlagsMask;

/**
 * @brief
 *   Type used to represent the parental control either stored in the smartcard
 *   or required to see a program.
 *
*/
typedef TUnsignedInt8   TCaParentalControlThreshold;

/**
 * @brief
 *   Type used to represent the parental control bitmap either stored in the
 *   smartcard or required to see a program.
 *
*/
typedef TUnsignedInt16  TCaParentalControlBitmap;

#ifndef CA_ICC_H
/**
 *  @ingroup g_card
 *
 *  @brief
 *    Smartcard session identifier.
*/
typedef TUnsignedInt32  TIccSessionId;

/**
 *  @ingroup g_card
 *
 *  @brief
 *    Special ID to specify no smartcard.
*/
#define ICC_SESSION_ID_NONE ((TIccSessionId)-1)
#endif /* CA_ICC_H */


/**
 *  @ingroup g_init
 *
 * @brief
 *   UTC time importation callback.
 *
 *   This decoder manufacturer's function returns the current UTC time and date
 *   to the CA. It will be called each time the CA needs this information.
 *
 *  @param   pxUtcTime
 *             UTC Time extracted from the stream. For a DVB stream, it should
 *             be extracted from the TDT.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TUtcTimeImportation)
(
  TCalendarTime*  pxUtcTime
);


/**
 *  @ingroup g_init
 *
 * @brief
 *   Pairing data importation callback.
 *
 *   This decoder manufacturer's function gets the pairing data
 *   (reconstruction), stored in the protected memory area of the decoder. The
 *   buffer is provided by this function. It should remain valid until the
 *   CA shuts down.
 *
 *  @param  ppxNaspPairingData
 *            Pointer to the first byte of Nagravision-provided pairing
 *            data.
 *
 *  @retval CA_NO_ERROR
 *            Success.
 *  @retval CA_ERROR
 *            Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *
 *  @see caConvertPairingDataFrom50To61, caConvertPairingDataFrom60To61.
*/
typedef TCaStatus (*TPairingDataImportation)
(
  const TUnsignedInt8** ppxNaspPairingData
);

/**
 *  @ingroup g_event
 *
 *  @brief
 *    Type used for a event. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaEvent TCaEvent;

/**
 *  @ingroup g_event
 *
 *  @brief
 *    The event ID is the unique identifier of an event within
 *    the entire system.
*/
typedef TUnsignedInt16 TCaEventId;

/**
 *  @ingroup g_event
 *
 *  @brief
 *    Preview time format of a pay per view event, expressed in Seconds. It is
 *    a free access period at the beginning of an IPPV event.
 *
*/
typedef TUnsignedInt32  TEventPreviewTime;

/**
 *  @ingroup g_program
 *
 *  @brief
 *    Define access statuses
 *
*/
typedef enum
{
  CA_ACCESS_CLEAR = 0,
    /**<  The given content (event, asset, ...) is not scrambled.
     *    The CA is not required.
    */
  CA_ACCESS_GRANTED,
    /**<  Access is granted by the smartcard.
    */
  CA_ACCESS_FREE,
    /**<  Access is granted by the smartcard, the service is free.
    */

  CA_ACCESS_DENIED = 100,
    /**<  Access is denied by the smartcard.
    */
  CA_ACCESS_NO_VALID_SMARTCARD,
    /**<  Deprecated and renamed CA_ACCESS_NO_VALID_SECURE_DEVICE to be
     *    compatible with card-less CAS.
    */
  CA_ACCESS_NO_VALID_SECURE_DEVICE = CA_ACCESS_NO_VALID_SMARTCARD,
    /**<  The secure device (e.g. smartcard) is not inserted, or the CAK is
     *    temporarily unable to communicate with it. it may also be incompatible
     *    with the program to be descrambled (e.g. program operated by an
     *    operator not managed by the secure device). Therefore, access is not
     *    granted. In a card-based solution, it is advised to make a smart card
     *    information request to get further information about the state of the
     *    smart card.
    */
  CA_ACCESS_SMARTCARD_BLACKLISTED,
    /**<  Deprecated. The smartcard is blacklisted. Access is not granted.
    */
  CA_ACCESS_SMARTCARD_SUSPENDED,
    /**<  Deprecated. The smartcard is suspended. Access is not granted.
    */
  CA_ACCESS_BLACKED_OUT,
    /**<  The related content (event, asset, ...) is blacked out in the user's
     *    area. Access is not granted.
    */
  CA_ACCESS_DENIED_NO_VALID_CREDIT,
    /**<  The access to the program is not authorized, because there
     *    is not enough credit remaining.
    */
  CA_ACCESS_DENIED_COPY_PROTECTED,
    /**<  The access to the program is not authorized, because it is
     *    copy-protected.
    */
  CA_ACCESS_DENIED_PARENTAL_CONTROL,
    /**<  The access to the program is not authorized, because of parental
     *    control settings
    */
  CA_ACCESS_DENIED_DIALOG_REQUIRED,
    /**<  The access to the program is not authorized and requires a dialog
     *    popup
    */
  CA_ACCESS_DENIED_PAIRING_REQUIRED,
    /**<  The access to the program is not authorized, because the smartcard is
     *    not paired.
    */
  CA_ACCESS_DENIED_CHIPSET_PAIRING_REQUIRED,
    /**<  The access to the program is not authorized, because the chipset is
     *    not paired.
    */
  CA_ACCESS_EMI_UNSUPPORTED,
    /**<  The program is scrambled with an algorithm (EMI) that is not supported
     *    by the STB
    */
  CA_ACCESS_DENIED_TSIO,
    /**<  The access to the program is not authorized, because TSIO mode is enforced
     *    on this program and the card does not support it.
    */
  CA_ACCESS_USAGE_RULES_VALIDATION_REQUIRED,
    /**<  The access to the program is granted but usages rules must be validated by
     *    the middleware before potentially setting keys in the descrambler.
    */
  CA_ACCESS_DENIED_USAGE_RULES_VIOLATION,
    /**<  The access to the program is not authorized due to usage rules
     *    violation
    */
  CA_ACCESS_DENIED_DEVICE_ACTIVATION_REQUIRED,
    /**<  The access to the program is not authorized since current device
     *    has to be activated.
    */
  CA_ACCESS_CORRUPTED_ECM,
    /**<  The access to the program is unknown since last ECM received
     *    by CAK is corrupted.
     *    No keys have been set into the descrambler.
    */
  CA_ACCESS_DENIED_SOFTWARE_UPGRADE_REQUIRED,
    /**<  The access to the program is not authorized since current device
     *    software has to be updated.
    */
  CA_ACCESS_DENIED_USAGE_RULES_TRANSITION,
    /**<  The access to the program is not authorized since a transition
     *    to new usage rules does not allow current descrambling.
    */
  CA_ACCESS_GRANTED_PPT = 1000,
    /**<  Deprecated.
     *    The access is granted by the smartcard for a PPT service.
    */
  CA_ACCESS_DENIED_FOR_PARENTAL_RATING,
    /**<  Deprecated.
     *    The access is not authorized, because parental rating settings
     *    prevent it.
    */
  CA_ACCESS_RIGHT_SUSPENDED,
    /**<  Deprecated.
     *    The related entitlement is suspended. Access is not granted.
    */
  CA_ACCESS_DENIED_BUT_PPT
    /**<  Deprecated.
     *    The access to the program is not authorized, because it is a
     *    pay per time for which no time slice is currently activated.
    */
} TCaAccess;


/** @addtogroup g_card
 *  @{
*/

/**
 *  @brief
 *    Flags giving information about the smart card.
 *
 *  @see #CA_SMARTCARD_CLEAN, #CA_SMARTCARD_VIRGIN, #CA_SMARTCARD_DLST_OBSERVED,
 *    #CA_SMARTCARD_PROCESSED_EMM_UN, #CA_SMARTCARD_REMOVABLE
*/
typedef TUnsignedInt32 TSmartcardFlags;

#define CA_SMARTCARD_CLEAN               BIT0
  /**<  When set to 1, this flag indicates that the smart card did not received
   *    any EMM since the last refurbishment.
   *    @see TSmartcardFlags
  */

#define CA_SMARTCARD_VIRGIN              BIT1
  /**<  When set to 1, this flag indicates that the smart card did not received
   *    any EMM since its personalization.
   *    @see TSmartcardFlags
  */

#define CA_SMARTCARD_DLST_OBSERVED       BIT2
  /**<  When set to 1, this flag indicates that the daylight saving time shall
   *    be observed.
   *    @see TSmartcardFlags
  */
#define CA_SMARTCARD_PROCESSED_EMM_UN    BIT3
  /**<  When set to 1, this flag indicates that the smart card processed an
   *    EMM-UN (unique address). It is reset to 0 at smart card insertion
   *    @see TSmartcardFlags
  */
#define CA_SMARTCARD_REMOVABLE           BIT4
  /**<  When set to 1, this flag indicates that the caSmartcard object
   *    corresponds to a physical card that can be removed from the card
   *    reader.
   *    @see TSmartcardFlags
  */
#define CA_SMARTCARD_TSIO_CAPABLE        BIT5
  /**<  When set to 1, this flag indicates that the smart card is able to
   *    descramble the stream.
   *    @see TSmartcardFlags, CA_STREAM_PROCESSING_MODE_TSIO
  */

/**
 *  @brief
 *    Delta time offset in seconds.
*/
typedef TSignedInt32  TDeltaTime;

/**
 *  @brief
 *    smartcard set ID.
*/
typedef TUnsignedInt16  TCaScSetId;

/**
 *  @brief
 *    Enumeration used the represent the state of the smartcard.
*/
typedef enum
{
  CA_SMARTCARD_STATE_OK,
  /**<  The smartcard is communicating well with the CA and no problem has
   *    been encountered (smartcard well inserted, well identified and no
   *    communication error).
  */
  CA_SMARTCARD_STATE_ERROR,
  /**<  A smartcard communication error occurred. It is set when no more
   *    acceptable communication is possible with the smartcard because of
   *    perturbations (i.e. glitches, electrical or magnetic interference,
   *    heat, etc).
  */
  CA_SMARTCARD_STATE_MUTE,
  /**<  A smartcard is inserted in the reader but the driver cannot
   *    communicate with it at all. The card may be inserted upside down.
  */
  CA_SMARTCARD_STATE_INVALID,
  /**<  The smartcard that is currently in the decoder is not identified
   *    as a NagraVision smartcard.
  */
  CA_SMARTCARD_STATE_BLACKLISTED,
  /**<  A blacklisted smartcard is like an erased smartcard, it is
   *    irremediably lost. Such a case could possibly happen for serious
   *    non-respect of agreements or for irregular use of the smartcard.
  */
  CA_SMARTCARD_STATE_SUSPENDED,
  /**<  It is possible to suspend the smartcard until certain conditions are
   *    reached, and then to cancel the suspension. The difference between
   *    blacklisted and suspended is the reversibility that does not exist
   *    for the first case and exists for the second one. A good example for
   *    the suspension is a bad payer.
  */
  CA_SMARTCARD_STATE_NEVER_PAIRED,
  /**<  Indicates that the smartcard has never been paired with any decoder.
   *    For security reasons the smartcard must be paired.
  */
  CA_SMARTCARD_STATE_NOT_PAIRED,
  /**<  Indicates that the smartcard is not paired with the current decoder
   *    (smartcard dedicated for another decoder). For security reasons the
   *    smartcard must be paired. The CA won't accept a non-paired card.
  */
  CA_SMARTCARD_STATE_EXPIRED,
  /**<  Indicates that the smartcard has expired. Access is denied to any
   *    events or services upon such a condition. The smartcard expiration
   *    date may be updated at any time by CAS head-end.
  */
  CA_SMARTCARD_STATE_NOT_CERTIFIED,
  /**<  Indicates that the smartcard is not certified with the current decoder.
  */
  CA_SMARTCARD_STATE_INCOMPATIBLE,
  /**<  Indicates that the smartcard is not compatible with another currently
   *    inserted smartcard.
  */
  CA_SMARTCARD_NUM_STATES
  /**<  Number of elements in the enumeration; for internal use only.
  */
} TSmartcardState;

/**
 *  @brief
 *    Type used the represent the CaSmartcard class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaSmartcard TCaSmartcard;

/** @} g_card */



/** @addtogroup g_mpeg_filter_configuration
*  @{
*/

/**
 *  @brief
 *    MPEG filter configuration object.
 *
 * The structure itself is private to the CAK to prevent external access to its
 * fields.
*/
typedef struct SCaMpegFilterCfg TCaMpegFilterCfg;

/** @} g_mpeg_filter_configuration */


/** @addtogroup g_content
 *  @{
*/

/**
 *  @brief
 *    Type used for a content. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaContent TCaContent;

/** @} g_content */

/** @addtogroup g_credit
 *  @{
*/

/**
 *  @brief
 *    Flags stored with a credit record in the smartcard.
 *
 *  @see CA_CREDIT_SUSPENDED
*/
typedef TUnsignedInt8  TCreditFlags;

#define CA_CREDIT_SUSPENDED  BIT0
  /**<  The credit record is suspended
   *    @see TCreditFlags
  */
#define CA_CREDIT_EXPIRED    BIT1
  /**<  The credit record is expired
   *    @see TCreditFlags
  */

/**
 *  @brief
 *    Type used the represent the CaCredit class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaCredit TCaCredit;

/**
 *  @brief
 *    The credit ID is the unique identifier of a product within
 *    the entire system.
*/
typedef TUnsignedInt32 TCaCreditId;

/** @} g_credit */

/** @addtogroup g_pullEmmContext
 *  @{
*/

/**
 *  @brief
 *    Type used the represent the CaPullEmmContext class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaPullEmmContext TCaPullEmmContext;

/** @} g_pullEmmContext */

/**
 *  @ingroup g_listener
 *
 *  @brief
 *    Type used for an exportation. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaExportation TCaExportation;



/** @addtogroup g_listener
 *  @{
*/

/**
 *  @brief
 *    This type represents a Listener used to perform a notification
 *    or exportation. The structure SCaListener is not exported.
*/
typedef struct SCaListener TCaListener;


/**
 *  @brief
 *    All the functions related to the listeners return a
 *    request status using the type TCaListenerStatus.
*/
typedef enum
{
  CA_LISTENER_NO_ERROR,
    /**<  Successful.
    */
  CA_LISTENER_ERROR,
    /**<  An unspecified error occurred.
    */
  CA_LISTENER_INVALID_TYPE,
    /**<  The specified type is invalid.
    */
  CA_LISTENER_INVALID,
    /**<  The pointer to the listener is invalid.
    */
  CA_LISTENER_NO_MORE_RESOURCES,
    /**<  Not enough resource to perform the required treatment
     *    (memory, semaphore, etc).
    */
  CA_LISTENER_PARAMETER_INVALID,
    /**<  One of the specified parameters is invalid.
    */
  CA_LISTENER_NOT_AVAILABLE,
    /**<  This Listener type is not available in the CAK.
    */
  CA_LISTENER_PARAMETER_MISSING,
    /**<  One of the mandatory parameters is missing
    */
  CA_LISTENER_ALREADY_REGISTERED,
    /**<  This Listener is already commited
    */
  CA_LISTENER_NUM_STATUS
    /**<  Number of elements in the enumeration; not used.
    */
} TCaListenerStatus;

/**
 *  @brief
 *    This enumeration specifies the types of Listener that may be
 *    processed by the CAK. The availability of a Listener type
 *    however depends on the features available in your CAK library.
*/
typedef enum
{
  CA_LISTENER_TYPE_SMARTCARDS,
    /**<  Smartcards notification.
    */
  CA_LISTENER_TYPE_PRODUCTS_LOADED,
    /**<  Notifies the application when a new product list is fully loaded.
    */
  CA_LISTENER_TYPE_PRODUCTS_LOADING,
    /**<  Notifies the application when a new product list is available but
     *    not complete yet.
    */
  CA_LISTENER_TYPE_PURCHASE_HISTORY,
    /**<  Purchase history notification.
    */
  CA_LISTENER_TYPE_RECHARGE_HISTORY,
    /**<  Notification occurring whenever the recharge history changes,
     *    that is to say when a recharge is added, removed or updated.
     *    This notification is also triggered by the smart card insertion
     *    in order to inform the application that the recharge history is
     *    available.
    */
  CA_LISTENER_TYPE_NEW_RECHARGE,
    /**<  Notification occurring whenever the smart card receives a new
     *    recharge. Unlike the recharge history notification, this notification
     *    only occurs when the credit balance of the smart card is increased.
    */
  CA_LISTENER_TYPE_TERMINATION,
    /**<  Termination notification.
    */
  CA_LISTENER_TYPE_CREDITS,
    /**<  Smartcard credits notification.
    */
  CA_LISTENER_TYPE_ACCESS_RIGHTS,
    /**<  Notifies the application that rights managed by the smart card have
     *    changed. It is advised to re-compute the access status of any events
     *    or services upon such a notification.
    */
  CA_LISTENER_TYPE_PROGRAMS,
    /**<  Programs notification.
    */
  CA_LISTENER_TYPE_IRD_COMMAND,
    /**<  IRD command exportation.
    */
  CA_LISTENER_TYPE_NEW_PURCHASE,
    /**<  New purchase record.
    */
  CA_LISTENER_TYPE_SYSTEM,
    /**<  System notification.
    */
  CA_LISTENER_TYPE_DEPRECATED_13,
    /**<  Deprecated.
    */
  CA_LISTENER_TYPE_OPERATORS,
    /**<  Operators notification.
    */
  CA_LISTENER_TYPE_PIN_CODES,
    /**<  Pincodes notification.
    */
  CA_LISTENER_TYPE_SUBSCRIPTIONS,
    /**<  Subscription rights notification.
    */
  CA_LISTENER_TYPE_DATABASE,
    /**<  Notifies the application whenever CAK database changes.
    */
  CA_LISTENER_TYPE_STB_LOCK,
    /**<  StbLock notification.
    */
  CA_LISTENER_TYPE_POPUP,
    /**<  Popup notification
    */
  CA_LISTENER_TYPE_CHIPSET_HARDWARE_RESET,
    /**<  Notifies the middleware that a full hardware reset of the chipset is
     *    required. The middleware must call caPause() before resetting the
     *    chipset. It is recommended to make these operations (caPause + reset)
     *    as soon as possible, although the middleware is free to choose the
     *    most suitable time.
    */
  CA_LISTENER_TYPE_PULL_EMM,
    /**<  Notifies the middleware that a connection to the backend is required
     *    to verify if out-of-band EMM needs to be processed by CAK.
    */
  CA_LISTENER_NUM_TYPES
    /**<  Number of elements in the enumeration; not used.
    */
} TCaListenerType;


/**
 * @brief
 *    Listener callback prototype.
 *
 *  The callback registered is used by the CAK in order to export data to the
 *  application or notify it of an event.
 *  This callback must be implemented by the application and has to match the
 *  function prototype defined below.<p>
 *
 *  Objects returned by this callback only apply to exportations. In such a
 *  case, exported CaExportation objects must be disposed of when no longer
 *  needed by calling caExportationDispose(). In case of notifications, the
 *  callback does not provide any exported object (pxExportation is NULL) but
 *  just informs the application that an event occurred (smart card information
 *  changed, products loaded, ...). If the application is interested in getting
 *  updated data, it has to request it explicitly.
 *
 *  @param   pxListener
 *             Listener object associated to this callback.
 *  @param   pxExportation
 *             Exportation object or NULL in case of notification
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# The disposal function (caListenerDispose) shall not be
 *       called within this callback.
 *
 *  @see caListenerSetExportationMode
*/
typedef void (*TCaListenerCallback)
(
  const TCaListener*      pxListener,
        TCaExportation*   pxExportation

);

/** @} g_listener */

/** @addtogroup g_request_types
 *  @{
*/

/**
**  @brief
**    This is the current version of the CAK generic requests
*/
#define CA_GENERIC_REQUEST_VERSION                                          1664

/**
 *  @brief
 *    Possible status associated to a request
*/
typedef enum
{
/* global status */
  CA_REQUEST_NO_ERROR,
    /**< successful */
  CA_REQUEST_ERROR,
    /**< unspecified error */
  CA_REQUEST_NO_MORE_RESOURCES,
    /**< the request could not be allocated or routed */
/* processing status */
  CA_REQUEST_NOT_PROCESSED,
    /**< the request is not yet processed */
  CA_REQUEST_PROCESSING,
    /**< the request is being processed */
  CA_REQUEST_PROCESSED,
    /**< the request has been processed */
/* invalid requests */
  CA_REQUEST_INVALID,
    /**< the pointer to the request is invalid */
  CA_REQUEST_INVALID_TYPE,
    /**< the function does not apply to this request type */
  CA_REQUEST_INVALID_SYNCHRONISM,
    /**< this specified synchronism is unsupported */
  CA_REQUEST_PROCESSING_FAILED,
    /**< the request processing failed; the getter cannot be used */
/* invalid parameters */
  CA_REQUEST_PARAMETER_INVALID,
    /**< a parameter of the request is invalid */
  CA_REQUEST_PARAMETER_OUT_OF_RANGE,
    /**< the parameter is out of range */
  CA_REQUEST_PARAMETER_MISSING,
    /**< a parameter is missing; the request cannot be processed */
  CA_REQUEST_PARAMETER_INCOMPATIBLE,
    /**< the specified parameters are incompatible */
  CA_REQUEST_NOT_AVAILABLE,
    /**< the requested information is not available in the CAK */
  CA_REQUEST_EXPORTATION_UNSUPPORTED,
    /**< the exportation is not supported by this request type */
  CA_REQUEST_NUM_STATUS
    /**< number of status */
} TCaRequestStatus;

/**
 *  @brief
 *    Types of request that may be done to the CAK
*/
typedef enum
{
  CA_REQUEST_TYPE_SYSTEM = CA_GENERIC_REQUEST_VERSION,
    /**< Information about the system (decoder) */
  CA_REQUEST_TYPE_SMARTCARDS,
    /**< Information about all smartcard currently inserted in the decoder */
  CA_REQUEST_TYPE_PRODUCTS,
    /**< List of available products */
  CA_REQUEST_TYPE_PRODUCT_PURCHASE,
    /**< Impulsive purchase of a product */
  CA_REQUEST_TYPE_PURCHASE_HISTORY,
    /**< History list of purchased products */
  CA_REQUEST_TYPE_RECHARGE_HISTORY,
    /**< Histor list of credit recharges */
  CA_REQUEST_TYPE_PDT_START,
    /**< Request to start the Product Definition Task */
  CA_REQUEST_TYPE_PDT_STOP,
    /**< Request to stop the Product Definition Task */
  CA_REQUEST_TYPE_HASH_SIGN,
    /**< Request a signature on a hash block */
  CA_REQUEST_TYPE_EVENT,
    /**< Information about an event */
  CA_REQUEST_TYPE_CREDITS,
    /**< List of available credits */
  CA_REQUEST_TYPE_PROGRAMS,
    /**< List of available programs */
  CA_REQUEST_TYPE_UNIQUE_KEY,
    /**< Unique key retrieval */
  CA_REQUEST_TYPE_RANDOM_KEY,
    /**< Random key retrieval */
  CA_REQUEST_TYPE_WATCH_PPT,
    /**< Watch pay-per-time product */
  CA_REQUEST_TYPE_VOD_PLAYOUT,
    /**< Open a VOD playout session */
  CA_REQUEST_TYPE_RESYNC_CALLBACK,
    /**< Resynchronization of cardless module */
  CA_REQUEST_TYPE_PRODUCTS_INFO,
    /**< Information about products */
  CA_REQUEST_TYPE_ENABLE_CONSUMPTION,
    /**< Enable consumption */
  CA_REQUEST_TYPE_DISABLE_CONSUMPTION,
    /**< Disable consumption */
  CA_REQUEST_TYPE_DEPRECATED_20,
    /**< Deprecated */
  CA_REQUEST_TYPE_OPERATORS,
    /**< Information about Operators*/
  CA_REQUEST_TYPE_EMM_PROCESSING,
    /**< Process an EMM (Out of band format) */
  CA_REQUEST_TYPE_PIN_CODES_INFO,
    /**< Information about pin codes*/
  CA_REQUEST_TYPE_PIN_CODE_CHANGE,
    /**< Change pin code */
  CA_REQUEST_TYPE_PIN_CODE_CHECK,
    /**< Check pin code */
  CA_REQUEST_TYPE_SET_PARENTAL_CONTROL,
    /**< Set parental control threshold and bitmap */
  CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING,
    /**< Descramble a program */
  CA_REQUEST_TYPE_EMM_FILTERING,
    /**< Filters EMM sections */
  CA_REQUEST_TYPE_OPERATOR_RESTRICTION,
    /**< define an operator restriction */
  CA_REQUEST_TYPE_SERVER_CONNECT,
    /**< Connect to a CAS server */
  CA_REQUEST_TYPE_CONTENT_PLAYBACK,
    /**< Descramble a content being played back */
  CA_REQUEST_TYPE_PVP_KEY,
    /**< Compute a key for protecting A/V content stored on PVP */
  CA_REQUEST_TYPE_FILTERED_EMM_PROCESSING,
  /**< Process a filtered EMM (MPEG Section format) */
  CA_REQUEST_TYPE_PULL_EMM_GET_CONTEXT,
  /**< Get context information in order to retrieve pull EMM from the backend */
  CA_REQUEST_TYPE_PULL_EMM_PROCESSING,
  /**< Process pull EMM response provided by the backend */
  CA_REQUEST_TYPE_DATABASE_DUMP,
  /**< Request the database dump of Enable/Protect secure device*/
  CA_REQUEST_NUM_TYPES
    /**< number of supported request types */
} TCaRequestType;

/**
 * @brief
 *   Processing status
*/
typedef enum
{
  CA_PROCESSING_NO_ERROR,
    /**< the request has been processed successfully */
  CA_PROCESSING_ERROR,
    /**< an unspecified error occurred during the request processing */
  CA_PROCESSING_NO_VALID_SMARTCARD,
    /**< Deprecated and renamed CA_PROCESSING_NO_VALID_SECURE_DEVICE to be
      *  compatible with card-less CAS */
  CA_PROCESSING_NO_VALID_SECURE_DEVICE = CA_PROCESSING_NO_VALID_SMARTCARD,
    /**< no valid secure device has been found to process the request */
  CA_PROCESSING_PARAMETER_INVALID,
    /**< a parameter of the request is invalid */
  CA_PROCESSING_PARAMETER_OUT_OF_RANGE,
    /**< a parameter of the request is out of range */
  CA_PROCESSING_PARAMETER_MISSING,
    /**< a required parameter of the request is missing */
  CA_PROCESSING_PARAMETER_INCOMPATIBLE,
    /**< one or more parameters are incompatible with this request */
  CA_PROCESSING_TERMINATION,
    /**< the process stopped due to the CAK termination */
  CA_PROCESSING_NO_MORE_RESOURCES,
    /**< the request processing stopped due to unavailability
     *   of a system resource (allocated memory, semaphore) */
  CA_PROCESSING_LOW_CREDIT,
    /**< The smartcard does not contain enough credit to process the request */
  CA_PROCESSING_NO_CREDIT,
    /**< The smartcard does not contain any credit at all and this prevents
     *   the CAK from processing the request */
  CA_PROCESSING_CREDIT_SUSPENDED,
    /**< The smartcard credit is suspended and this prevents the CAK from
     *   processing the request. */
  CA_PROCESSING_CREDIT_EXPIRED,
    /**< The smartcard credit is expired and this prevents the CAK from
     *   processing the request. */
  CA_PROCESSING_MEMORY_FULL,
    /**< The smartcard memory is full and this prevents the CAK from processing
     *   the request. */
  CA_PROCESSING_CONNECTION_ERROR,
    /**< The CAK faced a connection error and this prevents the CAK from
     *   processing the request. */
  CA_PROCESSING_OUT_OF_PURCHASE_WINDOW,
    /**< The request did not complete successfully because of a purchase
         window problem. */
  CA_PROCESSING_SUBSCRIBER_UNAUTHORIZED,
    /**< The request did not complete successfully because the subscriber is
         not authorized. */
  CA_PROCESSING_PRODUCT_UNPURCHASABLE,
    /**< The request did not complete successfully because the product is not
         purchasable. */
  CA_PROCESSING_BLACKOUT,
    /**< The request did not complete successfully because the subscriber is in
         blackout area. */
  CA_PROCESSING_WRONG_PIN_CODE,
    /**< The request did not complete successfully because the given pincode was
         wrong. */
  CA_PROCESSING_CONNECTION_INFO_MISSING,
    /**< The request did not complete successfully because some information
         (e.g. server address) is missing to establish a connection */
  CA_PROCESSING_CONNECTION_IN_PROGRESS,
    /**< The request is rejected because a connection is already in progress.
         This takes into account connections triggered by the application and
         the CAK itself. */
  CA_PROCESSING_NUM_STATUS
  /**< number of processing status */
} TCaProcessingStatus;

/**
 *  @brief
 *    This enumeration defines the error status returned by all the
 *    functions of CAK's exported classes.
*/
typedef enum
{
  CA_OBJECT_NO_ERROR,
  /**<  The function terminated successfully.
  */
  CA_OBJECT_INVALID,
  /**<  The object is invalid.
  */
  CA_OBJECT_PARAMETER_INVALID,
  /**<  A parameter is invalid (probably set to NULL).
  */
  CA_OBJECT_NOT_AVAILABLE,
  /**<  The requested information is not available in the CAK.
  */
  CA_OBJECT_ERROR
  /**<  An unspecified error occured during the function execution.
  */
} TCaObjectStatus;

/**
 *  @brief
 *    This enumeration defines the stream processing modes. It specifies whether
 *    streams are descrambled by the set-top box or by the smart card.
*/
typedef enum
{
  CA_STREAM_PROCESSING_MODE_DVB,
  /**<  This mode indicates that streams are descrambled by the set-top box
  */
  CA_STREAM_PROCESSING_MODE_TSIO
  /**<  This mode indicates that streams are descrambled by the smart card
  */
} TCaStreamProcMode;

/**
 *  @brief
 *    Type used to uniquely identify the delivery system an MPEG program is
 *    coming from. In a DVB system the source ID corresponds to the original
 *    network ID. In an ATSC system, it is simply equal to the ATSC source ID.
*/
typedef TUnsignedInt32    TCaSourceId;


/**
 *  @brief
 *    Type used for a request. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaRequest TCaRequest;


/**
 *  @brief
 *    Function type used for the request callback. For asynchronous requests,
 *    the application must provide the address of a function meeting this
 *    type.
 *  @param pxRequest
 *    Request that the CAK has processed
*/
typedef void (*TCaRequestCallback)
(
  TCaRequest*  pxRequest
);


/**
 *  @brief
 *    Function type used for the request exportation callback. For requests
 *    allowing or requiring data exportation, the application must provide the
 *    address of a function meeting this type.
 *  @param pxRequest
 *    Request that the CAK has processed
*/
typedef void (*TCaRequestExportationCallback)
(
  const TCaRequest*     pxRequest,
        TCaExportation* pxExportation
);


/** @} g_request_types */

/**
 *  @ingroup g_irdcmd
 *
 *  @brief
 *    Type used the represent the CaIrdCommand class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaIrdCommand TCaIrdCommand;

/** @addtogroup g_pin
 * @{
*/

/**
 *  @brief
 *    Type used for a pinCode. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaPinCode TCaPinCode;

/**
 *  @brief
 *    Flags stored with a pincode record in the smartcard.
*/
typedef TUnsignedInt32  TPinCodeFlags;

#define CA_PIN_CODE_SUSPENDED   BIT0
  /**<  The pincode is suspended
   *    @see TCreditFlags
  */
#define CA_PIN_CODE_BLOCKED     BIT1
  /**<  The pincode is blocked.
  */
#define CA_PIN_CODE_UNLIMITED   BIT2
  /**<  The pincode is unlimited.
  */

/**
 *  @brief
 *    The pincode ID is the unique identifier of a pincode.
*/
typedef TUnsignedInt32 TCaPinCodeId;

/** @} g_pin */

/** @addtogroup g_purchase
 * @{
*/

/**
 *  @brief
 *    Flags giving information on a purchase.
 *
 *  @see CA_PURCHASE_WATCHED, CA_PURCHASE_REFUNDED, CA_PURCHASE_SUSPENDED,
 *       CA_PURCHASE_PAY_PER_TIME, CA_PURCHASE_IMPULSIVE,
 *       CA_PURCHASE_THROUGH_CAK_API
*/
typedef TUnsignedInt32 TPurchaseFlags;

#define CA_PURCHASE_WATCHED               BIT0
  /**<  The related product has been watched.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_REFUNDED              BIT1
  /**<  The purchase has been refunded.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_SUSPENDED             BIT2
  /**<  The related product is suspended.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_PAY_PER_TIME          BIT3
  /**<  Deprecated. The related product is a pay-per-time.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_IMPULSIVE             BIT4
  /**<  Deprecated. The purchase was performed impulsively.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_USED_CARD_CREDIT      BIT4
  /**<  The purchase was performed impulsively, i.e. used the smart card credit.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_THROUGH_CAK_API       BIT5
  /**<  Deprecated. The purchase was performed through the CAK purchase API.
   *    Available only with Tiger smart cards.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_CONDITIONAL           BIT6
  /**<  The related product is conditional.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_COLLECTED             BIT7
  /**<  The related product has been collected by the head-end through a
   *    return path connection.
   *    @see TPurchaseFlags
   */

#define CA_PURCHASE_WATCHED_TO_REPORT     BIT8
  /**<  The related product has been watched in the ELK meaning of the watched flag.
   *    (application still have to report flag as no secure reporting was done by
   *     the CAS).
   *    @see TPurchaseFlags
   */


/**
 *  @brief
 *    Mode in which the purchase action was performed.
*/
typedef enum
{
  CA_PURCHASE_MODE_OFFLINE,
    /**< Offline Purchase mode.   */
  CA_PURCHASE_MODE_ONLINE,
    /**< Online Purchase mode.    */
  CA_PURCHASE_MODE_SMS,
    /**< SMS Purchase mode.       */
  CA_PURCHASE_MODE_UNKNOWN,
    /**< Unknown Purchase mode.   */
  CA_PURCHASE_MODE_FREE_PRELOADED,
    /**< Purchase mode applying to free entitlements preloaded at smart card
     *   production.
     */
  CA_PURCHASE_NUM_MODES
    /**< Number of purchase modes. Should never be passed to any function.   */
} TCaPurchaseMode;



/**
 *  @brief
 *    Type used for a product. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaPurchase TCaPurchase;

/** @} g_purchase */

/** @addtogroup g_product
 * @{
*/

/**
 *  @brief
 *    The product ID is the unique identifier of a product within
 *    the entire system.
*/
typedef TUnsignedInt32 TCaProductId;

/**
 *  @brief
 *    The CA product type
*/
typedef enum
{
  CA_PRODUCT_UNDEFINED = 0,
    /**<  Undefined type
    */
  CA_PRODUCT_EVENT = 1,
    /**<  Single-event product
    */
  CA_PRODUCT_SERVICE = 2,
    /**<  Single-service subscription
    */
  CA_PRODUCT_SERVICE_PACKAGE = 3,
    /**<  Single or multiple-service subscription
    */
  CA_PRODUCT_EVENT_PACKAGE = 4,
    /**<  Package of events
    */
  CA_PRODUCT_N_OF_M_SHOWINGS = 5,
    /**<  N out of M showings (impulsive selection)
    */
  CA_PRODUCT_N_OF_M_EVENTS = 6,
    /**<  N out of M events (impulsive selection)
    */
  CA_PRODUCT_N_OF_M_CHANNELS = 7,
    /**<  N out of M channels (impulsive selection)
    */
  CA_PRODUCT_PAY_PER_TIME = 8,
    /**<  Pay-per-time by coins
    */
  CA_PRODUCT_PPT_BY_POINTS = 11,
    /**<  Pay-per-time by points
    */
  CA_PRODUCT_FREE_PREVIEW = 16,
    /**<  Free preview profile
    */
  CA_PRODUCT_VOD_RENTAL = 17,
    /**<  Single-content VOD rental
    */
  CA_PRODUCT_VOD_PACKAGE = 18,
    /**<  Multiple-content VOD rental
    */
  CA_PRODUCT_VOD_SUBSCRIPTION = 19,
    /**<  VOD subscription
    */
  CA_PRODUCT_RENTAL_SUBSCRIPTION = 20,
    /**<  Subscription with short validity dates not automatically renewed
    */
  CA_PRODUCT_RECURRENT_PERIOD_SUB = 21,
    /**<  Recurrent period subscription. Typically used for week-end offers.
    */
  CA_PRODUCT_A_LA_CARTE = 22,
    /**<  'A la carte' products.
    */
  CA_PRODUCT_FREE_PERIOD = 23,
    /**<  Free period product granting access to all or subset of operator channels in anonymous mode
    */
  CA_PRODUCT_VOUCHER = 24,
    /**<  Product used to extend access for a given period in an anonymous mode
    */
  CA_PRODUCT_MAX_NUM
    /**<  Number of enum values.
    */
} TCaProductType;

/**
 *  @brief
 *    Flags giving information on a product.
*/
typedef TUnsignedInt32 TProductFlags;

#define CA_PRODUCT_PURCHASABLE            BIT0
  /**<  Flag to specify: product is purchasable.
  */

#define CA_PRODUCT_PURCHASED              BIT1
  /**<  Flag to specify: product has been purchased at least once.
  */

#define CA_PRODUCT_NOT_LOADED             BIT2
  /**<  Flag to specify: details on this product are not completely loaded.
  */

#define CA_PRODUCT_IMPULSIVE              BIT3
  /**<  Flag to specify: product can be purchased impulsively.
  */

#define CA_PRODUCT_OFFLINE_PURCHASE       BIT3
  /**<  Flag to specify: product can be purchased offline.
  */

#define CA_PRODUCT_ONLINE_PURCHASE        BIT4
  /**<  Flag to specify: product can be purchased online.
  */

#define CA_PRODUCT_SMS_PURCHASE           BIT5
  /**<  Flag to specify: product can be purchased by sms.
  */

#define CA_PRODUCT_MULTIPLE_PURCHASE      BIT6
  /**<  Flag to specify: product can be purchased several times.
  */

#define CA_PRODUCT_OFFLINE_CONSUMPTION  BIT7
  /**<  Flag to specify: product can be purchased with impulsively consumption.
  */


/**
 *  @brief
 *    Type used for a product. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaProduct TCaProduct;

/** @} g_product */

/** @addtogroup g_program
 * @{
*/

/**
 *  @brief
 *    Type used the represent the CaProgram class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaProgram TCaProgram;


/**
 * @brief
 *   Type used to represent mode of consumption of an event or a program.
 *
*/
typedef enum
{
  CA_CONSUMPTION_MODE_CONTINUOUS,
  CA_CONSUMPTION_MODE_ONE_SHOT,
  CA_CONSUMPTION_NUM_MODES
} TCaConsumptionMode;

/** @} g_program */

/**
 *  @ingroup g_price
 *
 *  @brief
 *    Type used for a price. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaPrice TCaPrice;

/** @addtogroup g_recharge
 * @{
*/

/**
 *  @brief
 *    This enumeration details the possible reasons of a recharge.
*/
typedef enum {
  CA_RECHARGE_REASON_USER_REQUEST,
  /**<  The recharge has been initiated by the user.
  */
  CA_RECHARGE_REASON_PROMOTION,
  /**<  The recharge is a promotion of the operator.
  */
  CA_RECHARGE_REASON_REFUND
  /**<  The recharge is a refund.
  */
} TCaRechargeReason;

/**
 *  @brief
 *    Type used the represent the CaRecharge class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaRecharge TCaRecharge;

/** @} g_recharge */

/** @addtogroup g_key
 * @{
*/

/**
 *  @brief
 *    This enumeration represents the key types.
*/
typedef enum {
  CA_KEY_TYPE_GENERIC8,
  /**< 8-byte key.
  */
  CA_KEY_TYPE_GENERIC16,
  /**< 16-byte key.
  */
  CA_KEY_TYPE_DES,
  /**< 8-byte key with 8 parity bits.
  */
  CA_KEY_TYPE_3DES,
  /**< 16-byte key with 16 parity bits.
  */
  CA_KEY_TYPE_DVB_CSA,
  /**< 8-byte key with entropy reduction.
  */
  CA_KEY_TYPE_NUMBER_OF_KEY_TYPES
  /**< Number of key types, should never be passed to any function.
  */
} TCaKeyType;


/**
 *  @brief
 *    Type used the represent the CaKey class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaKey TCaKey;

/** @} g_key */


/**
 *  @ingroup g_unique_key
 *
 *  @brief
 *    Type used the represent the CaUniqueKey class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaUniqueKey TCaUniqueKey;


/**
 *  @ingroup g_random_key
 *
 *  @brief
 *    Type used the represent the CaRandomKey class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaRandomKey TCaRandomKey;


/**
 *  @ingroup g_signature
 *
 *  @brief
 *    Type used the represent the CaSignature class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaSignature TCaSignature;

/** @addtogroup g_service
 * @{
*/

/**
 *  @brief
 *    Type used for a service. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaService TCaService;

/** @} g_service */

/** @addtogroup g_system
 * @{
*/

/**
 *  @brief
 *    Type used the represent the CaSystem class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaSystem TCaSystem;

/**
 *  @brief
 *    Flags of the CAK system.
 *
*/
typedef TUnsignedInt32  TSystemFlags;


#define CA_SYSTEM_SOFTWARE_UPGRADE_RECOMMENDED  BIT0
  /**<  software upgrade is recommended
   *    @deprecated
  */
#define CA_SYSTEM_SOFTWARE_UPGRADE_REQUIRED     BIT1
  /**<  software upgrade is required.
  */

/** @} g_system */

/** @addtogroup g_operator
 * @{
*/

/**
 *  @brief
 *    Flags giving information about an operator.
 */
typedef TUnsignedInt32 TOperatorFlags;

#define CA_OPERATOR_ADMINISTRATOR   BIT0
  /**<  The operator has administrator privileges.
   *    @see TOperatorFlags
   */

#define CA_OPERATOR_SUSPENDED       BIT1
  /**<  The operator is suspended
   *    @see TOperatorFlags
   */

#define CA_OPERATOR_INCONSISTENT    BIT2
  /**<  The operator gets some corrupted data.
   *    @see TOperatorFlags
   */

#define CA_OPERATOR_INITIALIZED     BIT3
  /**<  The operator is initialized.
   *    @see TOperatorFlags
   */

#define CA_OPERATOR_IN_CONNECTION   BIT4
  /**<  A connection to the operator’s server is in progress.
   *    @see TOperatorFlags
   */

/**
 *  @brief
 *    Type used for an operator. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaOperator TCaOperator;

/** @} g_operator */

/** @addtogroup g_connection
 * @{
*/

/**
 *  @brief
 *    Type used to report the result of a connection. Depending on the version
 *    of the CAK, a connection failure is reported either with the single status
 *    CA_CONNECTION_RESULT_FAILED or with more detailed statuses (local error,
 *    remote error, aborted).
*/
typedef enum
{
  CA_CONNECTION_RESULT_SUCCESSFULL,
  /**< The connection ended properly
  */
  CA_CONNECTION_RESULT_FAILED = 100,
  /**< The connection did not end properly for any kind of reasons (local,
   *   remote errors).
  */
  CA_CONNECTION_RESULT_LOCAL_ERROR,
  /**< The connection failed due to a local error (e.g. no dial tone, resource
   *   busy, etc).
  */
  CA_CONNECTION_RESULT_REMOTE_ERROR,
  /**< The connection failed due to a remote error (e.g. server busy, no answer,
   *   connection dropped, etc).
  */
  CA_CONNECTION_RESULT_ABORTED
  /**< The connection has been aborted (e.g. smart card has been removed,
   *   application canceled the connection request).
  */
} TCaConnectionResult;



/**
 *  @brief
 *    Type used for a connection. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaConnection TCaConnection;


/**
 *  @brief
 *    This enumeration define the reasons of a connection request.
*/
typedef enum
{
  CA_CONNECTION_REASON_CARD_REPORTING,
  /**< The application requests a connection to report smart card entitlements
   *   to the head-end
  */
  CA_CONNECTION_REASON_REFURBISHMENT
  /**< The application requests a connection to start a refurbishment session
  */
} TCaConnectionReason;


/**
 *  @brief
 *    This mode defines how connection errors are to be handled by the CAK.
*/
typedef enum
{
  CA_CONNECTION_MODE_ONE_SHOT,
  /**< The CAK attempts to establish the connection with the server once only.
   *   No further attempts are made in case of failure.
  */
  CA_CONNECTION_MODE_BACKGROUND
  /**< The CAK repeatedly attempts to get connected until it succeeds. In order
   *   to avoid overloading the server, a delay is enforced between two
   *   connection attempts.
  */
} TCaConnectionMode;


/**
 *  @brief
 *    Type used to define the state of a connection.
*/
typedef enum
{
  CA_CONNECTION_STATE_DISCONNECTED,
  /**< The CAK is not connected to a server.
  */
  CA_CONNECTION_STATE_CONNECTING,
  /**< The CAK is establishing a connection with a server
  */
  CA_CONNECTION_STATE_CONNECTED,
  /**< The CAK is connected to a server
  */
  CA_CONNECTION_STATE_LOGGED_IN
  /**< The CAK is logged in to a server. This state is optional. If the
    *  connection does not require any login or authentication, the connection
    *  remains in the connected state.
   */
} TCaConnectionState;

/** @} g_connection */

/**
 *  @ingroup g_server
 *
 *  @brief
 *    Type used for a server. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaServer TCaServer;


/**
 *  @ingroup g_popup
 *
 *  @brief
 *    Type used for a popup object. The structure itself is private to the CAK to
 *    prevent external access to its fields.
*/
typedef struct SCaPopup TCaPopup;


/** @addtogroup g_stblock
 * @{
*/

/**
 *  @brief
 *    Flags associeted to a stb zone lock object.
 *
*/
typedef TUnsignedInt8  TStbLockFlags;

#define CA_STB_LOCKED   BIT0
  /**<  The stb zone is locked
   *    @see TStbLockFlags
  */


/**
 *  @brief
 *    Type used the represent the CaStbLock class. The structure itself is
 *    private to the CAK to prevent external access to its fields.
*/
typedef struct SCaStbLock TCaStbLock;

/** @} g_stblock */

/**
*  @ingroup g_init
*
*  @brief
*    Database dump exportation callback.
*
*    CAK provides database chunks
*
*  @param  pxRequest
*            Request associated to the current database dump
*
*  @param  xChunkSize
*            Size of the chunk provided by CAK
*
*  @param  pxChunk
*            Pointer to database dump chunk.
*
*  @param  xlastChunk
*            if True, this chunk is the last one completing the
*            database dump payload.
*
*  @remark
*    -# If an issue occured on application side while processing
*       the current chunk, the associated request shall be closed.
*
*/
typedef struct {
  void(*sendChunk)
    (
      const TCaRequest*       pxRequest,
      const TUnsignedInt32     xChunkSize,
      const TUnsignedInt8*    pxChunk,
      const TBoolean           xlastChunk
      );
} INvDbDump;

/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/


/** @addtogroup g_cak_state
 * @{
*/

/**
 *  @brief
 *    Initialize all the CA tasks and related modules.
 *
 *    This function creates all the tasks composing the CA as well as their
 *    shared resources such as queues, semaphores, and so on. After their
 *    initializations, the tasks are made pending, waiting for the start-up
 *    synchronization signal.
 *    This call may be done only once. If the CA is shutdown by a call to
 *    caTermination, it can be initialized again by calling caInitialization.
 *
 *  @pre
 *    The CAK must not be running.
 *
 *  @post
 *    The CAK may be started.
 *    Registrations may be done at this time.
 *
 *  @param   xPairingImportation
 *             Callback function used by the CA to get its pairing data block
 *             from the decoder memory.
 *  @param   xUtcTimeImportation
 *             Callback function used by the CA to get the UTC time.
 *  @param   xMaxNumberOfIcc
 *             Number of smartcards the CA should be able to deal with.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# There is no upper limit to the number of smartcard the CA can manage,
 *       but each additional smartcard will require some resources.
*/
NAGRA_CA_API TCaStatus caInitialization
(
  TPairingDataImportation  xPairingImportation,
  TUtcTimeImportation      xUtcTimeImportation,
  TUnsignedInt8            xMaxNumberOfIcc
);


/**
 *  @brief
 *    Set the file system path where the CAK can find the personalization data.
 *
 *  @pre
 *    caInitialization must have been called.
 *
 *  @post
 *    The CAK ready to start.
 *
 *  @param   pxPersoDataPath
 *             Null-terminated string describing a path to a directory. It must
 *             end with a directory separator suitable for the underlying OS.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSetPersoDataPath
(
  const TChar* pxPersoDataPath
);

/**
 *  @brief
 *    Set the file system path where the CAK can store its private data.
 *
 *  @pre
 *    caInitialization must have been called.
 *
 *  @post
 *    The CAK ready to start.
 *
 *  @param   pxStoragePath
 *             Null-terminated string describing a path to a directory. It must
 *             end with a directory separator suitable for the underlying OS.
 *             The specified path must be read/write.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSetStoragePath
(
  const TChar* pxStoragePath
);

/**
 *  @brief
 *    Set the trusted file system path where the CAK can store its sensitive private data.
 *
 *  @pre
 *    caInitialization must have been called.
 *
 *  @post
 *    The CAK ready to start.
 *
 *  @param   pxTrustedStoragePath
 *             Null-terminated string describing a path to a directory. It must
 *             end with a directory separator suitable for the underlying OS.
 *             The specified path must be read/write.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSetTrustedStoragePath
(
  const TChar* pxTrustedStoragePath
);

/**
 *  @brief
 *    Set the implementation of the watermark software interface
 *
 *  @pre
 *    caInitialization must have been called.
 *
 *  @post
 *    The CAK ready to start.
 *
 *  @param   pxNvWmkSwInterface
 *             Implementation of the Watermark software interface.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSetWmkSwInterface
(
  const INvWmkSw* pxNvWmkSwInterface
);

/**
 *  @brief
 *    Set the implementation of the database dump interface
 *
 *  @pre
 *    caInitialization must have been called.
 *
 *  @post
 *    The CAK is ready to start.
 *
 *  @param   pxDbDumpInterface
 *             Implementation of the database dump interface.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSetDbDumpInterface
(
  const INvDbDump* pxDbDumpInterface
);



/**
 *  @brief
 *    Start up all CA threads
 *
 *    After the call to caInitialization, all CAK threads are waiting for a
 *    start-up synchronization signal. The purpose of this function is to send
 *    this signal in order to have all CAK threads in the running state.
 *
 *  @pre
 *    caInitialization must have been called.
 *    caSetPersoDataPath must have been called when POSIX is used.
 *    caSetStoragePath must have been called when POSIX is used and Enable/Protect is supported.
 *    caSetTrustedStoragePath must have been called when CAK Single posix flavor is used.
 *
 *  @post
 *    The CAK is up and running
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             An internal issue occured while starting up.
 *             Only a system request can be created.
 *
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caStartUp
(
  void
);


/**
 *  @brief
 *    Pauses the execution of all CAK threads.
 *
 *    This function is synchronous, which means the caller is blocked until all
 *    CAK threads complete their current processing. After this call, the CAK is
 *    in pause state. Call caResume() to resume the execution of CAK threads.
 *
 *  @retval  CA_NO_ERROR
 *             Success
 *
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running so it cannot be paused
 *
 *  @retval  CA_ERROR
 *             Failure
 *
 */
NAGRA_CA_API TCaStatus caPause
(
  void
);


/**
 *  @brief
 *    This function resumes the execution of all CAK threads. It is the
 *    counterpart of caPause().
 *
 *  @retval  CA_NO_ERROR
 *             Success
 *
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             An internal issue occured while resuming.
 *             Only a system request can be created.
 *
 *  @retval  CA_ERROR
 *             Failure
*/
NAGRA_CA_API TCaStatus caResume
(
  void
);


/**
 * @brief
 *   Terminate the CAK.
 *
 *   This function is responsible of terminating the CA. The termination
 *   consists in the deletion of all the CA tasks as well as the cleanup of
 *   their related resources (semaphores, memory, etc.).
 *
 *  @retval  CA_NO_ERROR
 *             Success
 *
 *  @retval  CA_ERROR
 *             Failure
 *
 *  @remark
 *    -# This function is synchronous. It means that the caller will be
 *       suspended as long as all the task have not performed their cleanup and
 *       have not terminated.
 *    -# During the caTermination() call execution, no other functions of the CA
 *       API may be called.
 *    -# All callbacks related to any pending CA request will be called before
 *       caTermination() returns with the corresponding CA_XXX_SHUTDOWN status.
 *    -# WARNING: The CA termination feature is not recommended since it is not
 *       fully functional
*/
NAGRA_CA_API TCaStatus caTermination
(
  void
);

/** @} g_cak_state */

/** @addtogroup g_event
 * @{
*/

/**
 *  @brief
 *    This function gets the unique ID of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxEventId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: event ID.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetId
(
  const TCaEvent*   pxEvent,
        TCaEventId* pxEventId
);

/**
 *  @brief
 *    This function gets the start time of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxStartTime
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: start time of the event (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetStartTime
(
  const TCaEvent*      pxEvent,
        TCalendarTime* pxStartTime
);

/**
 *  @brief
 *    This function gets the duration of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxDuration
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: duration in seconds of the event.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetDuration
(
  const TCaEvent*       pxEvent,
        TCaDuration*    pxDuration
);

/**
 *  @brief
 *    This function gets the service of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   ppxService
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: service of the event.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetService
(
  const TCaEvent*     pxEvent,
        TCaService** ppxService
);

/**
 *  @brief
 *    This function gets the name of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxSize
 *             Pointer to fill with the size of the event name.
 *             Cannot be NULL.
 *             OUT: number of bytes in the event name.
 *  @param   ppxName
 *             Pointer to fill with the event name. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the name of the event.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetName
(
  const TCaEvent*        pxEvent,
        TSize*           pxSize,
  const TUnsignedInt8** ppxName
);

/**
 *  @brief
 *    This function gets the list of products allowing access to this event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxNumberOfProducts
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: number of products in the array.
 *  @param   pppxProductArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Array of products.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetRelatedProducts
(
  const TCaEvent*       pxEvent,
        TUnsignedInt32* pxNumberOfProducts,
  const TCaProduct*** pppxProductArray
);

/**
 *  @brief
 *    This function gets the event access indicating if the
 *    smartcard grants access to the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxAccess
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: event access.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetAccess
(
  const TCaEvent*  pxEvent,
        TCaAccess* pxAccess
);

/**
 *  @brief
 *    This function gets the free preview time of the event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxPreviewTime
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: event free preview time.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetPreviewTime
(
  const TCaEvent*          pxEvent,
        TEventPreviewTime* pxPreviewTime
);

/**
 *  @brief
 *    This function gets the list of operators of this event.
 *
 *  @param   pxEvent
 *             Pointer to an event object.
 *  @param   pxNumberOfOperators
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: number of operators in the array.
 *  @param   pppxOperatorArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Array of operators.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caEventGetOperators
(
  const TCaEvent*        pxEvent,
        TUnsignedInt32*  pxNumberOfOperators,
  const TCaOperator*** pppxOperatorArray
);

/**
 *  @brief
 *    This function returns the winner entitlement (caPurchase object) selected
 *    by the secure device to grant access to the event.
 *
 *    This entitlement is also returned by caEventGetGrantingPurchases() that
 *    return all entitlements granting access to the event.
 *
 *  @param[in]  pxEvent
 *                Pointer to caEvent object
 *  @param[out] ppxPurchase
 *                caPurchase object corresponding to the winner entitlement
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
 *
 *  @note
 *    - If the event is in the clear, scrambled in free access or subject to blackout,
 *      this function returns #CA_OBJECT_NOT_AVAILABLE, independently on whether
 *      the secure device contains entitlements granting access.
 *    - The application may have to confirm the consumption of the winner
 *      entitlement. In such case, streams are not descrambled before consumption
 *      is confirmed.
 *
 *  @see caEventGetGrantingPurchases
*/
NAGRA_CA_API TCaObjectStatus caEventGetWinnerPurchase
(
  const TCaEvent*        pxEvent,
        TCaPurchase** ppxPurchase
);

/**
 *  @brief
 *    This function returns all the entitlements (array of caPurchase objects)
 *    that could be used by the secure device to grant access to the event.
 *
 *  @param[in]  pxEvent
 *                Pointer to caEvent object
 *  @param[out] pxNumberOfPurchases
 *                Number of caPurchase objects contained in the array.
 *  @param[out] pppxPurchaseArray
 *                Array of caPurchase objects. Buffer is allocated by the CAK.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
 *  @note
 *    - Entitlements granting access are returned even if the event is in the clear
 *      or scrambled in free access.
 *
 *  @see caEventGetWinnerPurchase
*/
NAGRA_CA_API TCaObjectStatus caEventGetGrantingPurchases
(
  const TCaEvent*        pxEvent,
        TUnsignedInt32*  pxNumberOfPurchases,
        TCaPurchase*** pppxPurchaseArray
);

/** @} g_event */

/** @addtogroup g_card
 * @{
*/

/**
 * @brief
 *   This function gets the CA system ID to use for ECM related to a smartcard.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxEcmCaSystemId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: ECM CA system ID; defines which ECM stream is being
 *             currently filtered by the CAK.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetEcmCaSystemId
(
  const TCaSmartcard*   pxSmartcard,
        TCaSystemId*    pxEcmCaSystemId
);


/**
 * @brief
 *   This function gets the CA system ID to use for EMM related to a smartcard.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxEmmCaSystemId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: EMM CA system ID; defines which EMM stream is being
 *             currently filtered by the CAK
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetEmmCaSystemId
(
  const TCaSmartcard*   pxSmartcard,
        TCaSystemId*    pxEmmCaSystemId
);


/**
 * @brief
 *   This function gets the flags of the smart card.
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags of the smart card
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetFlags
(
  const TCaSmartcard*     pxSmartcard,
        TSmartcardFlags*  pxFlags
);

/**
 * @brief
 *   This function gets the flags mask of the smart card.
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags mask of the smart card
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetFlagsMask
(
  const TCaSmartcard*     pxSmartcard,
        TFlagsMask*       pxFlagsMask
);

/**
 * @brief
 *   This function gets the concatenation of the ROM code version and the
 *   patch revision of the smartcard, for example "NCMED002 Rev100".
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   ppxVersion
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the smartcard version.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetVersion
(
  const TCaSmartcard*     pxSmartcard,
  const TChar**          ppxVersion
);

/**
 * @brief
 *   This function gets the smartcard serial number as a NULL-terminated string
 *   complying the following format: "xx xxxx xxxx cc" where xxxxxxxxxx is the
 *   10-digit smart card unique address and cc a 2-digit checksum.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   ppxSerialNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: serial number of the smartcard defined as a
 *             NULL-terminated string with the following format:
 *             "xx xxxx xxxx xx".
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSerialNumber
(
  const TCaSmartcard*     pxSmartcard,
  const TChar**          ppxSerialNumber
);


/**
 * @brief
 *   This function gets the smartcard serial number as a NULL-terminated string
 *   complying the following Mediaguard format: "p pp.xxx.xxx.xxx cc" where ppp
 *   is the 3-digit product code, xxxxxxxxx the 9-digit smart card unique
 *   address and cc a 2-digit checksum.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   ppxSerialNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the smart card serial
 *             number in Mediaguard format.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSerialNumberMg
(
  const TCaSmartcard*     pxSmartcard,
  const TChar**          ppxSerialNumber
);

/**
 * @brief
 *   This function gets the smartcard serial number as a 32 bits integer.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxSerialNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: serial number of the smartcard defined as a
 *             32 interger.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSerialNumberInt
(
  const TCaSmartcard*     pxSmartcard,
        TUnsignedInt32*   pxSerialNumber
);


/**
 * @brief
 *   This function gets the smartcard begin time.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxBeginDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Begin date of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetBeginDate
(
  const TCaSmartcard*     pxSmartcard,
        TCalendarTime*    pxBeginDate
);

/**
 * @brief
 *   This function gets the smartcard expiration time.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxExpirationDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Expiration date of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetExpirationDate
(
  const TCaSmartcard*     pxSmartcard,
        TCalendarTime*    pxExpirationDate
);


/**
 * @brief
 *   This function gets the smartcard renewal date and time.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxRenewalDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Renewal date of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetRenewalDate
(
  const TCaSmartcard*     pxSmartcard,
        TCalendarTime*    pxRenewalDate
);


/**
 * @brief
 *   This function gets the current date of the smartcard.
 *
 *  The current date is potentially updated when the smart card receives an
 *  encrypted message from the head-end.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Current date of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
 * @remarks
 *    -# This getter is a dynamic function, which recalculates the smartcard
 *       date each time it is called, until a smartcards change listener
 *       CA_LISTENER_TYPE_SMARTCARDS is triggered; in this case, a new
 *       smartcard request must be issued.
 *    -# The smartcards change listener CA_LISTENER_TYPE_SMARTCARDS is not
 *       triggered when the date changes.
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetDate
(
  const TCaSmartcard*   pxSmartcard,
        TCalendarTime*  pxDate
);


/**
 * @brief
 *   This function gets the smartcard provider ID.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxProviderId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: provider ID of the smartcard associated to this item.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetProviderId
(
  const TCaSmartcard*   pxSmartcard,
        TPpid*          pxProviderId
);

/**
 * @brief
 *   This function gets the smartcard session ID.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxSessionId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: session ID of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSessionId
(
  const TCaSmartcard*     pxSmartcard,
        TIccSessionId*    pxSessionId
);

/**
 * @brief
 *   This function gets the smartcard ZIP code.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxZipCodeSize
 *             Pointer to fill with the size of the ZIP code.
 *             Cannot be NULL.
 *             OUT: number of bytes in the ZIP code.
 *  @param   ppxZipCode
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: ZIP code of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetZipCode
(
  const TCaSmartcard*    pxSmartcard,
        TSize*           pxZipCodeSize,
  const TUnsignedInt8** ppxZipCode
);

/**
 * @brief
 *   This function gets the smartcard's ZIP code extension.
 *
 *  @param[in]  pxSmartcard
 *                Smartcard object handle
 *  @param[out] pxZipCodeExtensionSize
 *                Size in bytes of the ZIP code extension.
 *  @param[out] ppxZipCodeExtension
 *                Buffer containing the ZIP code extension. Memory is allocated
 *                by CAK.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetZipCodeExtension
(
  const TCaSmartcard*    pxSmartcard,
        TSize*           pxZipCodeExtensionSize,
  const TUnsignedInt8** ppxZipCodeExtension
);

/**
 * @brief
 *   This function gets the smartcard segments.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxSegmentsSize
 *             Pointer to fill with the size of the segments.
 *             Cannot be NULL.
 *             OUT: number of bytes in the segments.
 *  @param   ppxSegments
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Segments of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSegments
(
  const TCaSmartcard*    pxSmartcard,
        TSize*           pxSegmentsSize,
  const TUnsignedInt8** ppxSegments
);

/**
 * @brief
 *   This function gets the smartcard delta time.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxDeltaTime
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Delta time of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetDeltaTime
(
  const TCaSmartcard*  pxSmartcard,
        TDeltaTime*    pxDeltaTime
);

/**
 * @brief
 *   This function gets the current smartcard state.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxState
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: current state of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetState
(
  const TCaSmartcard*      pxSmartcard,
        TSmartcardState*        pxState
);

/**
 * @brief
 *   This function gets the smartcard parental control info.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxThreshold
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: parental control threshold of the smartcard.
 *  @param   pxBitmap
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: parental control bitmap of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetParentalControl
(
  const TCaSmartcard*           pxSmartcard,
  TCaParentalControlThreshold*  pxThreshold,
  TCaParentalControlBitmap*     pxBitmap
);

/**
 *  @brief
 *    This function gets the list of paired systems of this smartcard.
 *
 *  @param   pxSmartcard
 *             Pointer to a smartcard object.
 *  @param   pxNumberOfSystems
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Number of paired systems in the system array.
 *  @param   pppxSystemArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: array of pointers on caSystem objects.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetPairedSystems
(
  const TCaSmartcard*        pxSmartcard,
        TUnsignedInt32*      pxNumberOfSystems,
  const TCaSystem***       pppxSystemArray
);

/**
 * @brief
 *   This function gets the smartcard set ID.
 *
 *  @param   pxSmartcard
 *             Smartcard object to use.
 *  @param   pxScSetId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: set ID of the smartcard.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetSetId
(
  const TCaSmartcard*      pxSmartcard,
        TCaScSetId*        pxScSetId
);

/**
 * @brief
 *   This function gets the metadata of a smartcard object.
 *
 *  @param   pxSmartcard
 *             Pointer on a smartcard object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: metadata size.
 *  @param   ppxMetadataBuffer
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the metadata buffer.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetMetadata
(
  const TCaSmartcard*       pxSmartcard,
        TSize*              pxSize,
  const TUnsignedInt8**    ppxMetadataBuffer
);

/**
 * @brief
 *   This function gets a dump of the smart card database.
 *
 *  @param[in]  pxSmartcard
 *                Pointer on smartcard object.
 *  @param[out] pxSize
 *                Size in bytes of database dump.
 *  @param[out] ppxDatabaseDump
 *                Buffer containing the database dump. Memory is allocated by
 *                CAK.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetDatabase
(
  const TCaSmartcard*       pxSmartcard,
        TSize*              pxSize,
  const TUnsignedInt8**    ppxDatabaseDump
);

/**
 * @brief
 *   This function returns the maximum number of ECM streams the smart card is
 *   able to handle in parallel.
 *
 *  @param[in]  pxSmartcard
 *                Pointer on smartcard object.
 *  @param[out] pxMaxEcmStream
 *                Maximum number of ECM streams supported by the card.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSmartcardGetMaxEcmStream
(
  const TCaSmartcard*    pxSmartcard,
        TUnsignedInt8*   pxMaxEcmStream
);

/** @} g_card */

/** @addtogroup g_content
 * @{
*/

/**
 * @brief
 *   This function gets the metadata of a content object.
 *
 *  @param   pxContent
 *             Pointer on a content object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: metadata size.
 *  @param   ppxMetadataBuffer
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the metadata buffer.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caContentGetMetadata
(
  const TCaContent*         pxContent,
        TSize*              pxSize,
  const TUnsignedInt8**    ppxMetadataBuffer
);

/**
 * @brief
 *   This function gets the parental control info of a content object.
 *
 *  @param   pxContent
 *             Pointer on a content object.
 *  @param   pxThreshold
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Parental control threshold of the content.
 *  @param   pxBitmap
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Parental control Bitmap of the content.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caContentGetParentalControl
(
  const TCaContent*                   pxContent,
        TCaParentalControlThreshold*  pxThreshold,
        TCaParentalControlBitmap*     pxBitmap
);

/**
 * @brief
 *   This function returns an access status indicating whether the subscriber is
 *   allowed to play back an asset.
 *
 *  @param[in]  pxContent
 *                Pointer on a content object.
 *  @param[out] pxAccess
 *                Pointer to fill with the information. Cannot be NULL
 *                OUT: content playback access status
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caContentGetPlaybackAccess
(
  const TCaContent*     pxContent,
        TCaAccess*      pxAccess
);


/**
 * @brief
 *   This function gets the fingerprint associated to the caContent object.
 *
 *  @param[in]  pxContent
 *                Pointer on a content object.
 *  @param[out] pxSize
 *                Size in bytes of the fingerprint buffer. The pointer pxSize
 *                provided by the caller cannot be NULL.
 *  @param[out] ppxFingerprintBuffer
 *                Buffer allocated by CAK containing the fingerprint. The pointer
 *                ppxFingerprintBuffer provided by the caller cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This status should not be considered as an error. It is returned
 *             if no fingerprint is associated to the caContent object.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caContentGetFingerprint
(
  const TCaContent*         pxContent,
        TSize*              pxSize,
  const TUnsignedInt8**    ppxFingerprintBuffer
);

/** @} g_content */

/** @addtogroup g_credit
 * @{
*/

/**
 * @brief
 *   This function gets the provider id.
 *
 *  @param   pxCredit
 *             Pointer on a credit object.
 *  @param   pxProviderId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Provider Id.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caCreditGetProviderId
(
  const TCaCredit* pxCredit,
        TPpid*     pxProviderId
);

/**
 * @brief
 *   This function gets the amount of the credit.
 *
 *  @param   pxCredit
 *             Pointer on a credit object.
 *  @param   pxAmount
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Amount of credit.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caCreditGetAmount
(
  const TCaCredit*     pxCredit,
        TCreditAmount* pxAmount
);

/**
 * @brief
 *   This function gets the flags of the credit.
 *
 *  @param   pxCredit
 *             Pointer on a credit object.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags of the credit object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caCreditGetFlags
(
  const TCaCredit*     pxCredit,
        TCreditFlags*  pxFlags
);

/**
 * @brief
 *   This function gets the flags mask of the credit.
 *
 *  @param   pxCredit
 *             Pointer on a credit object.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags mask of the credit object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caCreditGetFlagsMask
(
  const TCaCredit*     pxCredit,
        TFlagsMask*    pxFlagsMask
);

/**
 *  @brief
 *    This function provides the purchase metadata.
 *
 *  @param   pxCredit
 *             Pointer to a credit object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: size in bytes of the credit metadata.
 *  @param   ppxMetadata
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: buffer containing the metadata of the credit.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caCreditGetMetadata
(
  const TCaCredit*            pxCredit,
        TSize*                pxSize,
        TUnsignedInt8**      ppxMetadata
);


/**
 * @brief
 *   This function gets the credit id.
 *
 *  @param   pxCredit
 *             Pointer on a credit object.
 *  @param   pxCreditId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Credit Id.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caCreditGetId
(
  const TCaCredit*    pxCredit,
        TCaCreditId*  pxCreditId
);

/** @} g_credit */

/** @addtogroup g_listener
 * @{
*/

/**
 *  @brief
 *    This function disposes of a CaExportation object. It must be called when
 *    the exportation object is no longer needed.
 *
 *  @pre
 *    pxExportation shall not be NULL.
 *
 *  @param   pxExportation
 *             Exportation object to be disposed of
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             The exportation has been successfully disposed
 *  @retval  CA_OBJECT_INVALID
 *             The object is not a CaExportation
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             The parameter is invalid (probably pxExportation set to NULL)
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
 *  @remark
 *    -# Once disposed, the exportation must not be accessed any longer.
*/
NAGRA_CA_API TCaObjectStatus caExportationDispose
(
  TCaExportation*       pxExportation
);


/**
 * @brief
 *    This function gets an array of objects pointer. The returned pointers
 *    are only valid while the exportation object is valid (i.e. not disposed).
 *
 *  The type of the return pointer depends on the type of the associated
 *  listener or request:<br>
 *  - IRD command listeners will receive CaIrdCommand objects.
 *  - New purchase listeners will receive CaPurchase objects.
 *  - STB lock listener will send CaStbLock object.
 *  - Popup listener will send CaPopup object.
 *  - Program descrambling request will send CaProgram objects.
 *
 *  @pre
 *    pxExportation, pxNumberOfObjects, pppxObjectArray shall not be NULL.
 *
 *  @param   pxExportation
 *             The exportation object
 *  @param   pxNumberOfObjects
 *             OUT: Number of objects pointers in the ppxObjectArray.
 *  @param   pppxObjectArray
 *             Pointer to fill with the information; cannot be NULL
 *             OUT: Array of pointer on objects.
 *             if the object is invalid or unsupported by the CAK,
 *             the address is set to NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is not a CaExportation
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             One of the specified parameters is invalid.
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             The information is not available in the CAK, i.e. caExporation
 *             is empty and does not contain any object.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
 *
*/
NAGRA_CA_API TCaObjectStatus caExportationGetObjects
(
  const TCaExportation*   pxExportation,
        TUnsignedInt32*   pxNumberOfObjects,
        void***         pppxObjectArray
);

/** @} g_listener */

/** @addtogroup g_listener
 * @{
*/

/**
 * @brief
 *    This function creates a Listener object that the application will use
 *    to receive information from the CAK.
 *
 *  Once a Listener has been successfully created, it can be registered through
 *  calling caListenerRegister(). No notifications are sent until the listener
 *  has been registered.
 *
 *  @pre
 *    ppxListener and xCallback shall not be NULL.
 *
 *  @param   xType
 *             Type of listener to create.
 *  @param   xCallback
 *             Callback that will be called by the CAK when related
 *             information are ready to be signaled. It must not be NULL
 *  @param   ppxListener
 *             Pointer where to store the listener. This pointer cannot be NULL.
 *             OUT: listener object or NULL in case of error.
 *
 *  @retval  CA_LISTENER_NO_ERROR
 *             Successful.
 *  @retval  CA_LISTENER_NO_MORE_RESOURCES
 *             Not enough resource to perform the required treatment
 *             (memory, semaphore, etc).
 *  @retval  CA_LISTENER_INVALID_TYPE
 *             The specified type is invalid.
 *  @retval  CA_LISTENER_PARAMETER_INVALID
 *             One of the specified parameters is invalid.
 *  @retval  CA_LISTENER_NOT_AVAILABLE
 *             This Listener type is not available in the CAK.
 *  @retval  CA_LISTENER_ERROR
 *             An unspecified error occurred.
 *
 *  @see caListenerDispose
*/
NAGRA_CA_API TCaListenerStatus caListenerCreate
(
  TCaListenerType         xType,
  TCaListenerCallback     xCallback,
  TCaListener**         ppxListener
);

/**
 * @brief
 *   This function performs the actual registration to the CAK of the created
 *   CaListener object.
 *
 *  No Notification is performed until caListenerRegister is called.
 *
 *  @pre
 *    The listener must be created by a call to caListenerCreate().
 *    pxListener shall not be NULL.
 *
 *  @post
 *    Even if this function returns an error value, the application must still
 *    dispose of the listener by calling caListenerDispose().
 *
 *  @param   pxListener
 *             Listener to register.
 *
 *  @retval  CA_LISTENER_NO_ERROR
 *             Registration successful.
 *  @retval  CA_LISTENER_INVALID
 *             A parameter is invalid (probably pxListener set to NULL)
 *  @retval  CA_LISTENER_PARAMETER_INVALID
 *             One of the specified parameters is invalid.
 *  @retval  CA_LISTENER_PARAMETER_MISSING
 *             One of the mandatory parameters is missing
 *  @retval  CA_LISTENER_ALREADY_REGISTERED
 *             the given listener was already registered
 *  @retval  CA_LISTENER_ERROR
 *             An unspecified error occurred.
 *
 *  @see caListenerCreate
*/
NAGRA_CA_API TCaListenerStatus caListenerRegister
(
  TCaListener*       pxListener
);


/**
 * @brief
 *   This function disposes of a Listener object created by the
 *   function caListenerCreate().
 *
 *  @pre
 *    The listener must be created by a call to caListenerCreate().
 *    pxListener shall not be NULL.
 *
 *  @param   pxListener
 *             Listener to dispose of.
 *
 *  @retval  CA_LISTENER_NO_ERROR
 *             The listener has been disposed of successfully.
 *  @retval  CA_LISTENER_INVALID
 *             The specified pointer does not correspond to a valid listener
 *  @retval  CA_LISTENER_PARAMETER_INVALID
 *             A parameter is invalid (probably pxListener set to NULL)
 *  @retval  CA_LISTENER_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaListenerStatus caListenerDispose
(
  TCaListener*       pxListener
);


/**
 * @brief
 *   This function activates the exportation mode for some types of listeners
 *
 *  By default, listeners only receive notifications, i.e. the application is
 *  told that some data have changed without further information; it is up to
 *  the client application to issue a new request to get the updated objects.
 *
 *  It is allowed for registrations of type #CA_LISTENER_TYPE_IRD_COMMAND,
 *  #CA_LISTENER_TYPE_NEW_PURCHASE, #CA_LISTENER_TYPE_STB_LOCK and
 *  #CA_LISTENER_TYPE_POPUP.
 *
 *  Exportation mode must be set before the listener is registered.
 *
 *  @pre
 *    The listener must be created by a call to caListenerCreate() and must not
 *    have already been registered.
 *    pxListener shall not be NULL.
 *
 *  @param   pxListener
 *             Listener for which the exportation mode must be activated
 *
 *  @retval  CA_LISTENER_NO_ERROR
 *             Exportation mode is active for the given listener
 *  @retval  CA_LISTENER_INVALID
 *             The specified pointer does not correspond to a valid listener
 *  @retval  CA_LISTENER_PARAMETER_INVALID
 *             A parameter is invalid (probably pxListener set to NULL)
 *  @retval  CA_LISTENER_ALREADY_REGISTERED
 *             Command failed because the given listener was already registered
 *  @retval CA_LISTENER_INVALID_TYPE
 *            This setter does not apply to the type of the given listener.
 *  @retval  CA_LISTENER_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaListenerStatus caListenerSetExportationMode
(
  TCaListener*       pxListener
);

/** @} g_listener */

/** @addtogroup g_request_main
 * @{
*/

/**
 *  @brief
 *    Creates a request that the application wants to send to the CAK.
 *
 *  @post
 *    The request must be disposed by a call to caRequestDispose().
 *
 *  @param   xType
 *             Type of request to create
 *  @param   ppxRequest
 *             Pointer where to store the address of the created request.
 *             This pointer cannot be NULL.
 *             OUT: address of the created request. If the request type is
 *             invalid or unsupported by the CAK, the address is set to NULL.
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The request has been successfully created
 *  @retval  CA_REQUEST_INVALID_TYPE
 *             The specified type is invalid
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             A parameter is invalid (probably ppxRequest set to NULL)
 *  @retval  CA_REQUEST_NO_MORE_RESOURCES
 *             The request could not be allocated (no more memory available).
 *             You should retry after having disposed some current requests
 *  @retval  CA_REQUEST_NOT_AVAILABLE
 *             This request type is not available in the CAK
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *
 *  @remark
 *    -# If successfully created, the request must be ultimately disposed by
 *       the caller, through a call to the function caRequestDispose().
 *       Beware that the request must not be disposed when it is sent
 *       (synchronously or asynchronously) to the CAK and the response has
 *       not been received yet.
*/
NAGRA_CA_API TCaRequestStatus caRequestCreate
(
  TCaRequestType    xType,
  TCaRequest**    ppxRequest
);

/**
 *  @brief
 *    Disposes (deletes) a request previously created by caRequestCreate().
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *    It must no be currently processed by the CAK.
 *
 *  @param   pxRequest
 *             Request to dispose
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The request has been successfully disposed
 *  @retval  CA_REQUEST_PROCESSING
 *             The request is being processed and cannot be disposed right now.
 *             Retry once the CAK has sent the response.
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid (probably pxRequest set to NULL)
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *
 *  @remark
 *    -# Once successfully disposed, the request must not be accessed any more.
*/
NAGRA_CA_API TCaRequestStatus caRequestDispose
(
  TCaRequest*    pxRequest
);

/**
 *  @brief
 *    Sends a request to the CAK for its processing.
 *    If the application has specified a response callback through the setter
 *    caRequestSetAsynchronousResponse(), the function returns immediately and
 *    the response is available only when the CAK calls the callback.
 *    Otherwise, the response is available as soon as the function returns
 *    (synchronous response); in this case, the calling task is blocked during
 *    the request processing.
 *    In both cases, it is forbidden to access to the request or dispose it
 *    during its processing (i.e. during the call to this function AND as long
 *    as the asynchronous response has not been received, if applicable).
 *
 *    A request can be sent at most once to the CAK. Whatever the processing
 *    succeeds or fails, it cannot be sent again.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *    It must never have been processed by the CAK.
 *
 *  @param   pxRequest
 *             Request to send
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The request has been successfully sent. If synchronous, the
 *             application may access to the response immediately, through
 *             the authorized getters. If asynchronous, the application must
 *             wait until the CAK calls the specified callback.
 *  @retval  CA_REQUEST_PROCESSING
 *             The request is being processed and cannot be sent again
 *  @retval  CA_REQUEST_PROCESSED
 *             The request has already been processed and cannot be sent again
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_INVALID_SYNCHRONISM
 *             This request cannot be performed synchronously. You should
 *             specify a callback through the function
 *             caRequestSetAsynchronousResponse().
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid (probably pxRequest set to NULL)
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *
 *  @remark
 *    -# Even if this function returns an error value, the application must
 *       still dispose the request by calling caRequestDispose().
 *    -# Also note that, even in the error cases, the getter
 *       caRequestGetProcessingStatus() is available and may help the application
 *       understanding why the request could not be sent.
*/

NAGRA_CA_API TCaRequestStatus caRequestSend
(
  TCaRequest*    pxRequest
);



/**
 *  @brief
 *    Gets the processing status. It may be called only once the request has
 *    been processed, even if the processing failed.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *             Request to use
 *  @param   pxProcessingStatus
 *             Pointer to fill with the processing status. Cannot be NULL.
 *             The value is meaningful only if the function returns
 *             CA_REQUEST_NO_ERROR.
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The processing status could be successfully retrieved
 *  @retval  CA_REQUEST_NOT_PROCESSED
 *             The request has not been processed yet. The getter cannot
 *             be used yet
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The getter cannot be used yet
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid (probably a parameter set to NULL)
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestGetProcessingStatus
(
  const TCaRequest*           pxRequest,
        TCaProcessingStatus*  pxProcessingStatus
);

/**
 *  @brief
 *    Gets the private data associated to the request.
 *    This getter may be called at any time, for any type of request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *             Request to use
 *  @param   ppxPrivateData
 *             Pointer to fill with the address of the private data.
 *             Cannot be NULL
 *             OUT: address of the private data, as set through the last
 *             call to caRequestSetPrivateData(). May be NULL
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The information could be successfully retrieved
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The getter cannot be used yet
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *
 *  @see caRequestSetPrivateData
*/
NAGRA_CA_API TCaRequestStatus caRequestGetPrivateData
(
  const TCaRequest*        pxRequest,
  const void**            ppxPrivateData
);

/**
 *  @brief
 *    This function gets an array of objects pointer. The returned pointers
 *    are only valid while the request object is valid i.e. not disposed.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate() and sent
 *    by a call to caRequestSend().
 *
 *  @param   pxRequest
 *             Request to use
 *  @param   pxNumberOfObjects
 *             Pointer to fill with the information.
 *             Cannot be NULL
 *             OUT: Number of object pointers in the array ppxObjectArray.
 *  @param   pppxObjectArray
 *             Pointer to fill with the information.
 *             Cannot be NULL
 *             OUT: Array of pointers to object pointers.
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The information could be successfully retrieved
 *  @retval  CA_REQUEST_NOT_PROCESSED
 *             The request has not been processed yet. The getter cannot
 *             be used
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The associated getter cannot be used yet
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_INVALID_TYPE
 *             The getter does not apply to this request type
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_REQUEST_PROCESSING_FAILED
 *             The request processing failed; the getter cannot be used
 *  @retval  CA_REQUEST_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestGetObjects
(
  const TCaRequest*        pxRequest,
        TUnsignedInt32*    pxNumberOfObjects,
        void***          pppxObjectArray
);

/** @} g_request_main */

/** @addtogroup g_request_setter
 * @{
*/

/**
 *  @brief
 *    Sets the private data associated to a request.
 *    This setter applies to any type of request. It must be called
 *    before the request processing.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *             Request to use
 *  @param   pxPrivateData
 *             Private data to set
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The parameter could be set successfully
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_PROCESSED
 *             The request has been processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *
 *  @see caRequestGetPrivateData
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPrivateData
(
        TCaRequest*  pxRequest,
  const void*        pxPrivateData
);

/**
 *  @brief
 *    Specifies that the CAK must send the response asynchronously,
 *    In this case, the function caRequestSend() immediately returns;
 *    the specified callback is called later, once the request has
 *    been processed. The callback is called anyway if the function
 *    caRequestSend() returns CA_REQUEST_NO_ERROR.
 *    This setter applies to any type of request. It must be called
 *    before the request processing.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *             Request to use
 *  @param    xCallback
 *             Callback that the CAK must use to send the response.
 *             Must not be NULL
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The parameter could be set successfully
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_PROCESSED
 *             The request has been processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid
 *  @retval  CA_REQUEST_INVALID_SYNCHRONISM
 *             This request type does not support the asynchronous mode.
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
 *  @remark
 *    -# In the callback, the application shall only notify its task about
 *       the reception of the response. The extraction of the response data
 *       (getters) and the request disposal shall not be performed in the
 *       callback.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetAsynchronousResponse
(
  TCaRequest*         pxRequest,
  TCaRequestCallback   xCallback
);

/**
 *  @brief
 *    This function associates a product to a request
 *    of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE
 *  @param   pxProduct
 *            Product to associate with the request
 *            Must not be NULL
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the product has not been
 *        set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetProduct
(
        TCaRequest*  pxRequest,
  const TCaProduct*  pxProduct
);

/**
 *  @brief
 *    This function associates a Packet Identifier to a request of type
 *    #CA_REQUEST_TYPE_PDT_START
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PDT_START
 *  @param    xPid
 *            Packet Identifier to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the product has not been
 *        set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPid
(
  TCaRequest*  pxRequest,
  TPid         xPid
);


/**
 *  @brief
 *    This function associates a buffer to a request. Depending of the request
 *    type, data provided may differ.
 *     -# #CA_REQUEST_TYPE_HASH_SIGN            Hash block to be signed by the CAK.
 *     -# #CA_REQUEST_TYPE_EMM_PROCESSING       EMM to process.
 *     -# #CA_REQUEST_TYPE_FILTERED_EMM_PROCESSING   EMM to process.
 *     -# #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING ECM to process.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use. It must be of type #CA_REQUEST_TYPE_HASH_SIGN,
 *            #CA_REQUEST_TYPE_EMM_PROCESSING, #CA_REQUEST_TYPE_FILTERED_EMM_PROCESSING,
 *            #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or #CA_REQUEST_TYPE_PULL_EMM_PROCESSING
 *  @param    xSizeOfBuffer
 *            Size of the provided buffer.
 *  @param    pxBuffer
 *            Buffer to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_PARAMETER_OUT_OF_RANGE
 *            The size of the buffer is out of range
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Multiple call to this function may be done on a given request.
 *        Limitation on whether the multiple buffers are taken into account or
 *        not is specific to the request type.
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetBuffer
(
        TCaRequest*    pxRequest,
        TSize           xSizeOfBuffer,
  const TUnsignedInt8* pxBuffer
);


/**
 *  @brief
 *    This function associates a transport session identifier to a request
 *    of type #CA_REQUEST_TYPE_PDT_START or #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *    or #CA_REQUEST_TYPE_EMM_FILTERING
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PDT_START or
 *            #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or
 *            #CA_REQUEST_TYPE_EMM_FILTERING
 *  @param    xTransportSessionId
 *            Transport session id to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the product has not been
 *        set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetTransportSessionId
(
  TCaRequest*                pxRequest,
  TTransportSessionId         xTransportSessionId
);


/**
 *  @brief
 *    This function associates a descriptor block to a request
 *    of type #CA_REQUEST_TYPE_EVENT
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_EVENT
 *  @param    xSizeOfDescriptors
 *            Size of the descriptor block.
 *  @param    pxDescriptorBlock
 *            Buffer of descriptors.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  This function can be called several times on the same request
 *        with different type of descriptors.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetDescriptors
(
        TCaRequest*     pxRequest,
        TSize            xSizeOfDescriptors,
  const TUnsignedInt8*  pxDescriptorBlock
);

/**
 *  @brief
 *    This function associates a start time to a request
 *    of type #CA_REQUEST_TYPE_EVENT
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_EVENT
 *  @param   pxStartTime
 *            The UTC start date and time of the concerned event.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Current date and time is used if this function is not called before
 *        processing.the request.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetStartTime
(
        TCaRequest*    pxRequest,
  const TCalendarTime* pxStartTime
);

/**
 *  @brief
 *    This function associates a CMM to a request
 *    of type #CA_REQUEST_TYPE_RANDOM_KEY
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_RANDOM_KEY
 *  @param   xSizeOfCmm
 *            Size of the CMM.
 *  @param   pxCmm
 *            Pointer to the CMM.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Current date and time is used if this function is not called before
 *        processing.the request.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetCmm
(
        TCaRequest*    pxRequest,
        TSize           xSizeOfCmm,
  const TUnsignedInt8* pxCmm
);

/**
 *  @brief
 *    This function associates a type of key to a request
 *    of type #CA_REQUEST_TYPE_RANDOM_KEY
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_RANDOM_KEY
 *  @param   xKeyType
 *            Type of key.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Current date and time is used if this function is not called before
 *        processing.the request.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetKeyType
(
  TCaRequest* pxRequest,
  TCaKeyType   xKeyType
);


/**
 *  @brief
 *    This function associates a program to a request. Depending of the request
 *    type, data provided may differ.
 *      -# #CA_REQUEST_TYPE_WATCH_PPT  Watch Pay-Per-Time product.
 *      -# #CA_REQUEST_TYPE_ENABLE_CONSUMPTION Pay-Per-Time product.
 *      -# #CA_REQUEST_TYPE_DISABLE_CONSUMPTION Pay-Per-Time product.
 *      -# #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING Any product.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_WATCH_PPT,
 *            #CA_REQUEST_TYPE_ENABLE_CONSUMPTION or
 *            #CA_REQUEST_TYPE_DISABLE_CONSUMPTION or
 *            #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *  @param   pxProgram
 *            Program to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Multiple call to this function may be done on a given request.
 *        Limitation on whether the multiple calls are taken into account or
 *        not is specific to the request type.
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetProgram
(
        TCaRequest*    pxRequest,
  const TCaProgram*    pxProgram
);


/**
 *  @brief
 *    This function associates a consumption mode to a request
 *    of type #CA_REQUEST_TYPE_ENABLE_CONSUMPTION.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_ENABLE_CONSUMPTION.
 *  @param   xConsumptionMode
 *            Consumption mode to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetConsumptionMode
(
        TCaRequest* pxRequest,
        TCaConsumptionMode xConsumptionMode
);


/**
 *  @brief
 *    This function associates a purchase mode to a request
 *    of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *  @param   xPurchaseMode
 *            Purchase mode to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPurchaseMode
(
        TCaRequest* pxRequest,
        TCaPurchaseMode xPurchaseMode
);


/**
 *  @brief
 *    This function associates a credit to a request
 *    of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *  @param   pxCredit
 *            Credit to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetCredit
(
        TCaRequest* pxRequest,
        TCaCredit*  pxCredit
);


/**
 *  @brief
 *    This function associates an exportation callback to a request
 *    of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *  @param   xCallback
 *            Callback that shall be used by the CAK to export information to
 *            the application.
 *            Cannot be NULL.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_EXPORTATION_UNSUPPORTED
 *            This request type does not support the exportation.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetExportationCallback
(
        TCaRequest*                     pxRequest,
        TCaRequestExportationCallback    xCallback
);


/**
 *  @brief
 *    This function associates a threshold and a bitmap to a request
 *    of type #CA_REQUEST_TYPE_SET_PARENTAL_CONTROL or #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_SET_PARENTAL_CONTROL
 *            or #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *  @param    xThreshold
 *            threshold to associate with the request.
 *  @param    xBitmap
 *            bitmap to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the parental control has
 *        not been set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetParentalControl
(
  TCaRequest*                   pxRequest,
  TCaParentalControlThreshold    xThreshold,
  TCaParentalControlBitmap       xBitmap
);

/**
 *  @brief
 *    This function associates a pincode object to a request
 *    of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE or #CA_REQUEST_TYPE_PIN_CODE_CHECK
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE
 *            or #CA_REQUEST_TYPE_PIN_CODE_CHECK
 *  @param   pxPinCode
 *            pointer on a caPinCode object.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the pincode has not been
 *        set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPinCode
(
  TCaRequest*                   pxRequest,
  const TCaPinCode*             pxPinCode
);

/**
 *  @brief
 *    This function associates the current pincode value to a request
 *    of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE or #CA_REQUEST_TYPE_PIN_CODE_CHECK
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE
 *            or #CA_REQUEST_TYPE_PIN_CODE_CHECK
*  @param   xPinCodeDataSize
 *            Data size of the pincode.
 *  @param   pxPinCodeData
 *            Data buffer of the pincode.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the pincode value has not
 *        been set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetCurrentPinCode
(
  TCaRequest*                   pxRequest,
  TSize                          xPinCodeDataSize,
  TUnsignedInt8*                pxPinCodeData
);

/**
 *  @brief
 *    This function associates the new pincode value to a request
 *    of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PIN_CODE_CHANGE
 *  @param   xPinCodeDataSize
 *            Data size of the pincode.
 *  @param   pxPinCodeData
 *            Data buffer of the pincode.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the pincode value has not
 *        been set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetNewPinCode
(
  TCaRequest*                   pxRequest,
  TSize                          xPinCodeDataSize,
  TUnsignedInt8*                pxPinCodeData
);


/**
 *  @brief
 *    This function associates a source identifier to a request
 *    of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *    It may have been sent to the CAK already.
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *  @param   xSourceId
 *            Source id to associate with the request.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the source Id has not been
 *        set through this function.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetSourceId
(
  TCaRequest*                pxRequest,
  TCaSourceId                 xSourceId
);


/**
 *  @brief
 *    This function associates a PMT section to a request
 *    of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *    It may have been sent to the CAK already.
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *  @param   xSizeOfSection
 *            Size in bytes of the PMT section to associate with the request.
 *  @param   pxSection
 *            Buffer containing the PMT section.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the PMT has not been
 *        set through this function.
 *    -#  This setter can be called optionnally on requests following the first
 *        one.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPmtSection
(
        TCaRequest*          pxRequest,
        TSize                 xSizeOfSection,
  const TUnsignedInt8*       pxSection
);


/**
 *  @brief
 *    This function associates a list of elementary stream PIDs to a request
 *    of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *  @param   xNumberOfPids
 *            Number of elementary stream PIDs included in pxPidArray.
 *  @param   pxPidArray
 *            Array containing the elementary stream PIDs.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_PARAMETER_OUT_OF_RANGE
 *            The value of the number of PIDs is out of range
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request cannot be successfully sent if the elementary stream PIDs
 *        have not been set through this function.
 *    -#  This setter can be called optionnally on requests following the first
 *        one.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetElemStreamPids
(
        TCaRequest*          pxRequest,
        TUnsignedInt32        xNumberOfPids,
  const TPid*                pxPidArray
);


/**
 *  @brief
 *    This function associates a list of operator IDs (PPID)to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle. It must be of one of the following types:
 *                - #CA_REQUEST_TYPE_OPERATOR_RESTRICTION
 *                - #CA_REQUEST_TYPE_EVENT
 *                - #CA_REQUEST_TYPE_PRODUCTS_INFO
 *                - #CA_REQUEST_TYPE_PURCHASE_HISTORY
 *                - #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *                - #CA_REQUEST_TYPE_RESYNC_CALLBACK
 *
 *  @param[in]  xNumberOfPpids
 *                Number of operator IDs. If set to 0, all operators are taken
 *                into account.
 *  @param[in]  pxPpidArray
 *                Array containing the operator IDs. If set to NULL, all
 *                operators are taken into account.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetOperatorIds
(
        TCaRequest*          pxRequest,
        TUnsignedInt32        xNumberOfPpids,
  const TPpid*               pxPpidArray
);

/**
 *  @brief
 *    This function associates a name to a request of type
 *    #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PRODUCT_PURCHASE.
 *  @param   xSize
 *            Number of bytes in the name buffer.
 *  @param   pxName
 *            Name buffer.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  The request #CA_REQUEST_TYPE_PRODUCT_PURCHASE only use
 *        caRequestSetName as an option to set the event and service name of
 *        the product purchased.
*/
NAGRA_CA_API TCaRequestStatus caRequestSetName
(
        TCaRequest*          pxRequest,
        TSize                 xSize,
  const TUnsignedInt8*       pxName
);

/**
 *  @brief
 *    This function associates a right ID to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use. It must be of type #CA_REQUEST_TYPE_EVENT.
 *  @param   xRightId
 *            Right ID
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetRightId
(
        TCaRequest*    pxRequest,
        TUnsignedInt32  xRightId
);

/**
 * @brief
 *   This function defines the connection reason of a request of type
 *   #CA_REQUEST_TYPE_SERVER_CONNECT.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in] pxRequest
 *               Reference to the request concerned by this function call
 *  @param[in] xReason
 *               Connection reason
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetConnectionReason
(
        TCaRequest*         pxRequest,
        TCaConnectionReason  xReason
);

/**
 * @brief
 *   This function defines the connection mode of a request of type
 *   #CA_REQUEST_TYPE_SERVER_CONNECT.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in] pxRequest
 *               Reference to the request concerned by this function call
 *  @param[in] xMode
 *               Connection mode
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetConnectionMode
(
        TCaRequest*       pxRequest,
        TCaConnectionMode  xMode
);

/**
 * @brief
 *   This function provides metadata to be passed directly to the RPH API upon a
 *   request of type #CA_REQUEST_TYPE_SERVER_CONNECT.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in] pxRequest
 *               Reference to the request concerned by this function call
 *               It must be of type #CA_REQUEST_TYPE_SERVER_CONNECT
 *  @param[in] xSizeOfMetadata
 *               Size in bytes of the metadata buffer
 *  @param[in] pxMetadata
 *               Metadata to be passed to the RPH API function rphSetMetadata().
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetConnectionMetadata
(
        TCaRequest*    pxRequest,
        TSize           xSizeOfMetadata,
  const TUnsignedInt8* pxMetadata
);

/**
 * @brief
 *   This function provides refurbishment data to be sent to the server during a
 *   refurbishment session triggered by a request of type
 *   #CA_REQUEST_TYPE_SERVER_CONNECT.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in] pxRequest
 *               Reference to the request concerned by this function call
 *  @param[in] xSizeOfData
 *               Size in bytes of the refurbishment data
 *  @param[in] pxRefurbishmentData
 *               Refurbishment data to be sent to the server
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetRefurbishmentData
(
        TCaRequest*    pxRequest,
        TSize           xSizeOfData,
  const TUnsignedInt8* pxRefurbishmentData
);


/**
 *  @brief
 *    This function provides a buffer of credentials to a request
 *    of type #CA_REQUEST_TYPE_CONTENT_PLAYBACK
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in] pxRequest
 *               Reference to the request concerned by this function call
 *  @param[in] xSizeOfCredentials
 *               Size in bytes of the credentials buffer
 *  @param[in] pxCredentials
 *               Credentials buffer
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetCredentials
(
        TCaRequest*          pxRequest,
        TSize                 xSizeOfCredentials,
  const TUnsignedInt8*       pxCredentials
);


/**
 *  @brief
 *    This function enables the request’s quick mode. It is used to increase the
 *    time performance of some type of requests (e.g. event information request).
 *    This implies that the request will return partial data.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *             Request to use
 *
 *  @retval  CA_REQUEST_NO_ERROR
 *             The information could be set successfully
 *  @retval  CA_REQUEST_PROCESSING
 *             The request has been sent to the CAK and is being processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_PROCESSED
 *             The request has been processed.
 *             The setter cannot be used any more.
 *  @retval  CA_REQUEST_INVALID
 *             The pointer does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid (probably pxRequest set to NULL)
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type or the setter
 *            is not implemented in this CAK.
 *  @retval  CA_REQUEST_ERROR
 *             An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetQuickMode
(
  TCaRequest*  pxRequest
);

/**
 *  @brief
 *    This function associates an ECM stream ID to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or #CA_REQUEST_TYPE_CONTENT_PLAYBACK.
 *
 *    ECM stream ID must be allocated in the range [0..N-1], where N is the value
 *    returned by caSmartcardGetMaxEcmStream(). Refer to the developer guide for
 *    further information about allocation strategies and constraints.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *                It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *                or #CA_REQUEST_TYPE_CONTENT_PLAYBACK
 *  @param[in] xEcmStreamId
 *                ECM stream ID
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type or the setter
 *            is not implemented in this CAK.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetEcmStreamId
(
  TCaRequest*    pxRequest,
  TUnsignedInt8  xEcmStreamId
);

/**
 *  @brief
 *    This function associates a stream processing mode to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or #CA_REQUEST_TYPE_CONTENT_PLAYBACK.
 *
 *    CA_STREAM_PROCESSING_MODE_DVB configures the request so that streams are
 *    descrambled by the set-top box. This is the default mode. It automatically
 *    applies if this function is not called. CA_STREAM_PROCESSING_MODE_TSIO
 *    configures the request so that streams are descrambled by the TSIO capable
 *    smart card.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *                It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *                or #CA_REQUEST_TYPE_CONTENT_PLAYBACK
 *  @param[in] xStreamProcMode
 *                Stream processing mode
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetStreamProcMode
(
  TCaRequest*        pxRequest,
  TCaStreamProcMode   xStreamProcMode
);

/**
 *  @brief
 *    This function associates an EMI (Encryption Method Identifier) to a request of
 *    type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or #CA_REQUEST_TYPE_CONTENT_PLAYBACK.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *                It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *                or #CA_REQUEST_TYPE_CONTENT_PLAYBACK
 *  @param[in]  xEmi
 *                EMI as defined in \ref REF_EMI "[EMI]"
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetEmi
(
  TCaRequest*    pxRequest,
  TUnsignedInt16  xEmi
);

/**
 *  @brief
 *    This function associates a priority to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or #CA_REQUEST_TYPE_CONTENT_PLAYBACK.
 *
 *    Priorities are defined in ascending order from 0 to 255, where 0 is the
 *    highest priority and 255 the lowest priority.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *                It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING
 *                or #CA_REQUEST_TYPE_CONTENT_PLAYBACK
 *  @param[in]  xPriority
 *                Priority to assign
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPriority
(
  TCaRequest*    pxRequest,
  TUnsignedInt8   xPriority
);

/**
 *  @brief
 *    This function associates a purchase unique ID to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *
 *    Purchase unique ID can be retrieved using a request of type
 *    #CA_REQUEST_TYPE_EVENT
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *  @param[in]  xPurchaseUid
 *                Purchase unique ID to assign.
 *                Cannot be higher than 0xFFFFFF.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_PARAMETER_OUT_OF_RANGE
 *            The value of the UID is out of range
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPurchaseUid
(
  TCaRequest*    pxRequest,
  TUnsignedInt32  xPurchaseUid
);

/**
 *  @brief
 *    This function associates a user intent to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *  @param[in]  xUserIntent
 *                User intent to assign.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_PARAMETER_OUT_OF_RANGE
 *            The value of the user intent is out of range
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetUserIntent
(
  TCaRequest*    pxRequest,
  TNvUserIntent   xUserIntent
);

/**
 *  @brief
 *    This function sets the status of current usage rules to a request of type
 *    #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *  @param[in]  xUsageRulesCompliant
 *                Informs if usage rules are considered as compliant or
 *                or not by the middleware
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            A parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_PARAMETER_OUT_OF_RANGE
 *            The value of the user intent is out of range
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetUsageRulesStatus
(
  TCaRequest*    pxRequest,
  TBoolean        xUsageRulesCompliant
);

/**
 *  @brief
 *    This function sets the secure media path configuration associated
 *    to a request of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle
 *
 *  @param[in]  xSecureMediaPathEnabled
 *                Informs if the secure media path has to be enabled or not.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            Successful operation
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            pxRequest does not correspond to a valid request
 *  @retval  CA_REQUEST_PARAMETER_INVALID
 *             The parameter is invalid (probably pxRequest set to NULL)
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetSecureMediaPath
(
  TCaRequest*    pxRequest,
  TBoolean        xSecureMediaPathEnabled
);

/**
 *  @brief
 *    This function associates an Mpeg filter configuration to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PROGRAM_DESCRAMBLING or
 *            #CA_REQUEST_TYPE_FILTERED_EMM_PROCESSING.
 *  @param   pxFilterCfg
 *            Mpeg filter configuration to associate with the request.<br>
 *            Must correspond to the Mpeg filter configuration used to filter the
 *            section provided using the caRequestSetBuffer method.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Multiple call to this function may be done on a given request.
 *        Limitation on whether the multiple calls are taken into account or
 *        not is specific to the request type.
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetMpegFilterCfg
(
        TCaRequest*       pxRequest,
  const TCaMpegFilterCfg* pxFilterCfg
);

/**
 *  @brief
 *    This function activate the force mode to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PULL_EMM_GET_CONTEXT
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Multiple call to this function may be done on a given request.
 *        Limitation on whether the multiple calls are taken into account or
 *        not is specific to the request type.
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetForceMode
(
  TCaRequest*    pxRequest
);


/**
 *  @brief
 *    This function associates an pull EMM context to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param   pxRequest
 *            Request to use.
 *            It must be of type #CA_REQUEST_TYPE_PULL_EMM_PROCESSING
 *  @param   pxPullEmmContext
 *             Pointer on the context object associated to the server response.
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
 *  @remark
 *    -#  Multiple call to this function may be done on a given request.
 *        Limitation on whether the multiple calls are taken into account or
 *        not is specific to the request type.
 *
*/
NAGRA_CA_API TCaRequestStatus caRequestSetPullEmmContext
(
        TCaRequest*          pxRequest,
  const TCaPullEmmContext*   pxPullEmmContext
);


/**
 *  @brief
 *    This function associates a CA System ID to a request.
 *
 *  @pre
 *    The request must be created by a call to caRequestCreate().
 *
 *  @param[in]  pxRequest
 *                Request handle. It must be of one of the following types:
 *                - #CA_REQUEST_TYPE_DATABASE_DUMP
 *
 *  @param[in]  xCaSystemId
 *                CA System ID to associate to the request
 *
 *  @retval CA_REQUEST_NO_ERROR
 *            The parameter could be set successfully
 *  @retval CA_REQUEST_PROCESSING
 *            The request has been sent to the CAK and is being processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_PROCESSED
 *            The request has been processed.
 *            The setter cannot be used any more.
 *  @retval CA_REQUEST_INVALID
 *            The pointer does not correspond to a valid request
 *  @retval CA_REQUEST_PARAMETER_INVALID
 *            The parameter is invalid
 *  @retval CA_REQUEST_INVALID_TYPE
 *            This setter does not apply to the passed request type.
 *  @retval CA_REQUEST_ERROR
 *            An unspecified error occurred
*/
NAGRA_CA_API TCaRequestStatus caRequestSetCaSystemId
(
        TCaRequest*          pxRequest,
  const TCaSystemId           xCaSystemId
);

/** @} g_request_setter */

/** @addtogroup g_irdcmd
 * @{
*/

/**
 * @brief
 *   This function gets the IRD command data buffer.
 *
 *  @param   pxIrdCommand
 *             IRD command object to use.
 *  @param   pxDataLength
 *             Pointer filled by the function with the IRD command data length
 *             in bytes. Cannot be NULL.
 *  @param   ppxData
 *             Pointer to a buffer in which this function will copy
 *             the IRD command data
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caIrdCommandGetData
(
  const TCaIrdCommand*   pxIrdCommand,
        TSize*           pxDataLength,
        TUnsignedInt8** ppxData
);

/** @} g_irdcmd */

/** @addtogroup g_pin
 * @{
*/

/**
 *  @brief
 *    This function gets the ID of the pincode.
 *  @param   pxPincode
 *             Pointer to a pincode object.
 *  @param   pxId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: value of the pincode ID.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPinCodeGetId
(
  const TCaPinCode*      pxPincode,
        TCaPinCodeId*    pxId
);

/**
 * @brief
 *   This function gets the flags of the pinode.
 *  @param   pxPinCode
 *             Pointer on a pincode object.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags of the pincode object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPinCodeGetFlags
(
  const TCaPinCode*    pxPinCode,
        TPinCodeFlags* pxFlags
);

/**
 * @brief
 *   This function gets the flags mask of the pincode.
 *  @param   pxPinCode
 *             Pointer on a pincode object.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags mask of the pincode object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPinCodeGetFlagsMask
(
  const TCaPinCode*    pxPinCode,
        TFlagsMask*    pxFlagsMask
);

/**
 * @brief
 *   This function gets the number of attempts left on a pincode.
 *  @param   pxPinCode
 *             Pointer on a pincode object.
 *  @param   pxNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: number of attempts left on the pincode.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPinCodeGetNumAttemptsLeft
(
  const TCaPinCode*     pxPinCode,
        TUnsignedInt32* pxNumber
);

/** @} g_pin */

/** @addtogroup g_purchase
 * @{
*/

/**
 * @brief
 *   This function returns the product purchased.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   ppxProduct
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer on the caProduct purchased..
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetProduct
(
  const TCaPurchase*     pxPurchase,
  const TCaProduct**    ppxProduct
);

/**
 * @brief
 *   This function returns the credit used to purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   ppxCredit
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer on the caCredit used.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetCredit
(
  const TCaPurchase*     pxPurchase,
  const TCaCredit**     ppxCredit
);

/**
 * @brief
 *   This function gets the date of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Purchase Date (UTC)
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetDate
(
  const TCaPurchase*     pxPurchase,
        TCalendarTime*   pxDate
);

/**
 * @brief
 *   This function gets the begin date of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxBeginDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Purchase Begin Date (UTC)
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetBeginDate
(
  const TCaPurchase*     pxPurchase,
        TCalendarTime*   pxBeginDate
);

/**
 * @brief
 *   This function gets the renewal date of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxRenewalDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Purchase renewal Date (UTC)
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetRenewalDate
(
  const TCaPurchase*     pxPurchase,
        TCalendarTime*   pxRenewalDate
);

/**
 * @brief
 *   This function gets the cost of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxCost
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Cost of the purchase
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetCost
(
  const TCaPurchase*     pxPurchase,
        TCreditAmount*   pxCost
);

/**
 * @brief
 *   This function gets the flags of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags of the purchase
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetFlags
(
  const TCaPurchase*     pxPurchase,
        TPurchaseFlags*  pxFlags
);

/**
 * @brief
 *   This function gets the flags mask of the purchase.
 *  @param   pxPurchase
 *             Purchase object to use.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Flags mask of the purchase
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetFlagsMask
(
  const TCaPurchase*     pxPurchase,
        TFlagsMask*      pxFlagsMask
);

/**
 *  @brief
 *    This function provides the purchase expiration date.
 *
 *  @param   pxPurchase
 *             Pointer to a purchase object.
 *  @param   pxExpirationDate
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Expiration date of the purchase (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetExpirationDate
(
  const TCaPurchase*   pxPurchase,
        TCalendarTime* pxExpirationDate
);

/**
 *  @brief
 *    This function provides the purchase mode.
 *
 *  @param   pxPurchase
 *             Pointer to a purchase object.
 *  @param   pxMode
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Mode of the purchase.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetMode
(
  const TCaPurchase*      pxPurchase,
        TCaPurchaseMode*  pxMode
);

/**
 *  @brief
 *    This function provides the points consumed during the purchase.
 *
 *  @param   pxPurchase
 *             Pointer to a purchase object.
 *  @param   pxPoints
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: number of points consummed.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetPointsConsumed
(
  const TCaPurchase*      pxPurchase,
        TUnsignedInt32*   pxPoints
);

/**
 *  @brief
 *    This function provides the purchase metadata.
 *
 *  @param   pxPurchase
 *             Pointer to a purchase object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: size in bytes of the purchase metadata.
 *  @param   ppxMetadata
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: buffer containing the metadata of the purchase.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetMetadata
(
  const TCaPurchase*          pxPurchase,
        TSize*                pxSize,
        TUnsignedInt8**      ppxMetadata
);



/**
 *  @brief
 *    This function provides the purchase unique ID.
 *
 *  @param[in]   pxPurchase
 *             Pointer to a purchase object.
 *  @param[out]  pxUid
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Uid of the purchase object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPurchaseGetUid
(
  const TCaPurchase*          pxPurchase,
        TUnsignedInt32*       pxUid
);

/** @} g_purchase */

/** @addtogroup g_product
 * @{
*/

/**
 *  @brief
 *    This function gets the unique ID of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxProductId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: product ID.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetId
(
  const TCaProduct*   pxProduct,
        TCaProductId* pxProductId
);

/**
 *  @brief
 *    This function gets the name of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxSize
 *             Pointer to fill with the size of the product name.
 *             Cannot be NULL.
 *             OUT: number of bytes in the product name.
 *  @param   ppxName
 *             Pointer to fill with the product name. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the name of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetName
(
  const TCaProduct*       pxProduct,
        TSize*            pxSize,
  const TUnsignedInt8**  ppxName
);

/**
 *  @brief
 *    This function gets the CA type of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxCaType
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: CA type of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetCaType
(
  const TCaProduct*     pxProduct,
        TCaProductType* pxCaType
);

/**
 *  @brief
 *    This function gets the business type of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxBusinessType
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: business type of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetBusinessType
(
  const TCaProduct*     pxProduct,
        TUnsignedInt8*  pxBusinessType
);

/**
 *  @brief
 *    This function gets the description of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxSize
 *             Pointer to fill with the size of the product description.
 *             Cannot be NULL.
 *             OUT: number of bytes in the product description.
 *  @param   ppxDescription
 *             Pointer to fill with the product description. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes getting
 *                  the description of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetDescription
(
  const TCaProduct*      pxProduct,
        TSize*           pxSize,
  const TUnsignedInt8** ppxDescription
);

/**
 *  @brief
 *    This function gets the time at which the product starts to be purchasable.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxPurchaseStartTime
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: purchase start time of the product (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetPurchaseStartTime
(
  const TCaProduct*    pxProduct,
        TCalendarTime* pxPurchaseStartTime
);

/**
 *  @brief
 *    This function gets the time after which the product is not
 *    purchasable anymore.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxPurchaseEndTime
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: purchase end time of the product (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetPurchaseEndTime
(
  const TCaProduct*    pxProduct,
        TCalendarTime* pxPurchaseEndTime
);

/**
 *  @brief
 *    This function gets the cost of a product.
 *    In the case of a pay-per-time product, this function returns
 *    the cost of a time slice.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxCost
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: cost of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetCost
(
  const TCaProduct*    pxProduct,
        TCreditAmount* pxCost
);

/**
 *  @brief
 *    This function gets the duration of a slice in second if
 *    the product is a pay-per-time.
 *    It returns CA_OBJECT_NOT_AVAILABLE if the product is not a pay-per-time.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxDuration
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: duration of a slice in second.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetSliceDuration
(
  const TCaProduct*  pxProduct,
        TCaDuration* pxDuration
);

/**
 *  @brief
 *    This function gets the number of points/credit of a product.
 *    Depending on the product type, it can refer to money, number of slices,
 *    number of visualizations.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxPoints
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: number of points/credit of a product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetPoints
(
  const TCaProduct*     pxProduct,
        TUnsignedInt32* pxPoints
);

/**
 *  @brief
 *    This function gets the begin date of product as the validity
 *    start time of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxBeginDate
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: begin date of the product (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetBeginDate
(
  const TCaProduct*    pxProduct,
        TCalendarTime* pxBeginDate
);
/**
 *  @brief
 *    This function gets the expiration date of product as the validity
 *    end time of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxExpirationDate
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: expiration date of the product's entitlement (UTC).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetExpirationDate
(
  const TCaProduct*    pxProduct,
        TCalendarTime* pxExpirationDate
);

/**
 *  @brief
 *    This function gets the flags of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: flags associated to the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/

NAGRA_CA_API TCaObjectStatus caProductGetFlags
(
  const TCaProduct*      pxProduct,
        TProductFlags*   pxFlags
);

/**
 *  @brief
 *    This function gets the flags mask of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: flags associated to the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetFlagsMask
(
  const TCaProduct*      pxProduct,
        TFlagsMask*      pxFlagsMask
);

/**
 *  @brief
 *    This function gets the list of event purchasable through a product.
 *    This list can be optionally filtered by a service.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxServiceFilter
 *             Filter used to return only the events relative to a service.
 *             If this parameter is NULL, all the events purchasable by
 *             the product are returned.
 *  @param   pxNumberOfEvents
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Number of events in the event array.
 *  @param   pppxEventArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: an array of event pointers.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetEvents
(
  const TCaProduct*        pxProduct,
  const TCaService*        pxServiceFilter,
        TUnsignedInt32*    pxNumberOfEvents,
        TCaEvent***      pppxEventArray
);

/**
 *  @brief
 *    This function gets the list of services a product gives right on.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxNumberOfServices
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Number of elements in the service array.
 *  @param   pppxServiceArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: array of pointers on caService objects.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetServices
(
  const TCaProduct*        pxProduct,
        TUnsignedInt32*    pxNumberOfServices,
        TCaService***    pppxServiceArray
);

/**
 *  @brief
 *    This function gets the list of purchases of this product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxNumberOfPurchases
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Number of elements in the purchase array.
 *  @param   pppxPurchaseArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: array of pointers on caPurchase objects.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetPurchases
(
  const TCaProduct*        pxProduct,
        TUnsignedInt32*    pxNumberOfPurchases,
        TCaPurchase***    pppxPurchaseArray
);

/**
 *  @brief
 *    This function gets the list of prices of this product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxNumberOfPrices
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Number of prices in the price array.
 *  @param   pppxPriceArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: array of pointers on caPrice objects.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetPrices
(
  const TCaProduct*        pxProduct,
        TUnsignedInt32*    pxNumberOfPrices,
        TCaPrice***      pppxPriceArray
);

/**
 *  @brief
 *    This function gets the disclaimer of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxSize
 *             Pointer to fill with the size of the disclaimer.
 *             Cannot be NULL.
 *             OUT: number of bytes in the disclaimer.
 *  @param   ppxDisclaimer
 *             Pointer to fill with the disclaimer. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the disclaimer of the product.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetDisclaimer
(
  const TCaProduct*      pxProduct,
        TSize*           pxSize,
  const TUnsignedInt8** ppxDisclaimer
);

/**
 *  @brief
 *    This function gets the remaining rental time of the product.
 *    It returns CA_REQUEST_NOT_AVAILABLE if the product is not purchased.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxRemainingRentalTime
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: remaining rental time in seconds.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
 *
 *  @remarks
 *    -# This getter is a dynamic function, which recalculates the product
 *       remaining rental time each time it is called.
*/
NAGRA_CA_API TCaObjectStatus caProductGetRemainingRentalTime
(
  const TCaProduct*  pxProduct,
        TCaDuration* pxRemainingRentalTime
);

/**
 *  @brief
 *    This function gets the provider id of the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxProviderId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: provider id.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetProviderId
(
  const TCaProduct* pxProduct,
        TPpid*      pxProviderId
);


/**
 *  @brief
 *    This function gets the descriptors linked to the product.
 *  @param   pxProduct
 *             Pointer to a product object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *            OUT: size of the descriptor buffer in bytes.
 *  @param   ppxDescriptors
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Buffer containing the product descriptors.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caProductGetDescriptors
(
  const TCaProduct*     pxProduct,
        TSize*          pxSize,
  const TUnsignedInt8** ppxDescriptors
);

/** @} g_product */

/** @addtogroup g_program
 * @{
*/

/**
 * @brief
 *   This function gets the transport session ID related to an MPEG
 *   program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   pxTransportSessionId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: transport session ID of the MPEG program.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetTransportSessionId
(
  const TCaProgram*          pxProgram,
        TTransportSessionId* pxTransportSessionId
);

/**
 * @brief
 *   This function gets the number of an MPEG program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   pxNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Number of the MPEG program.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetNumber
(
  const TCaProgram*     pxProgram,
        TProgramNumber* pxNumber
);

/**
 * @brief
 *   This function gets the current access status of an MPEG program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   pxAccess
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Access status of the MPEG program.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetAccess
(
  const TCaProgram* pxProgram,
        TCaAccess*  pxAccess
);

/**
 * @brief
 *   This function returnred the error code associated to a program access. It
 *   is intended to give more detail than the access status returned by
 *   caProgamGetAccess(). Unlike the access status, error codes are not defined
 *   in this API through a C enumeration type.
 *
 *  @param[in]   pxProgram
 *    Pointer to program object
 *
 *  @param[out]  pxAccessErrorCode
 *    Error code related to the program access.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetAccessErrorCode
(
  const TCaProgram*      pxProgram,
        TUnsignedInt32*  pxAccessErrorCode
);

/**
 * @brief
 *   This function gets the metadata of an MPEG program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   ppxMetadataBuffer
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the metadata buffer.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: metadata size.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetMetadata
(
  const TCaProgram*         pxProgram,
        TUnsignedInt8**    ppxMetadataBuffer,
        TSize*              pxSize
);

/**
 * @brief
 *   This function gets the key slot ID associated to the caProgram object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   pxKeySlotId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Identifier of the key slot returned
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetKeySlotId
(
  const TCaProgram*        pxProgram,
        TUnsignedInt16*    pxKeySlotId
);

/**
 * @brief
 *   This function gets the purchase object of an MPEG program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   ppxPurchase
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the purchase object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetPurchase
(
  const TCaProgram*         pxProgram,
        TCaPurchase**      ppxPurchase
);

/**
 * @brief
 *   This function gets the content object of an MPEG program object.
 *
 *  @param   pxProgram
 *             Pointer on a program object.
 *  @param   ppxContent
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the content object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caProgramGetContent
(
  const TCaProgram*         pxProgram,
        TCaContent**       ppxContent
);

/** @} g_program */

/** @addtogroup g_price
 * @{
*/

/**
 *  @brief
 *    This function gets the value of the price.
 *  @param   pxPrice
 *             Pointer to a price object.
 *  @param   pxAmount
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: value of the price.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPriceGetValue
(
  const TCaPrice*      pxPrice,
        TCreditAmount* pxAmount
);

/**
 *  @brief
 *    This function gets the credit associated to a price.
 *  @param   pxPrice
 *             Pointer to a price object.
 *  @param   ppxCredit
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: pointer on the credit object associated to this price.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPriceGetCredit
(
  const TCaPrice*      pxPrice,
  const TCaCredit**   ppxCredit
);

/** @} g_price */

/** @addtogroup g_recharge
 * @{
*/

/**
 * @brief
 *   This function returns the date of the recharge.
 *  @param   pxThis
 *             Recharge object to use.
 *  @param   pxDate
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: date of the recharge.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caRechargeGetDate
(
  const TCaRecharge*        pxThis,
        TCalendarTime*          pxDate
);

/**
 * @brief
 *   This function returns the amount of the recharge.
 *  @param   pxThis
 *             Recharge object to use.
 *  @param   pxAmount
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: amount of the recharge.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caRechargeGetAmount
(
  const TCaRecharge*        pxThis,
        TCreditAmount*          pxAmount
);


/**
 * @brief
 *   This function returns the reason of the recharge.
 *  @param   pxThis
 *             Recharge object to use.
 *  @param   pxReason
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: reason of the recharge.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caRechargeGetReason
(
  const TCaRecharge*          pxThis,
        TCaRechargeReason*  pxReason
);

/** @} g_recharge */

/** @addtogroup g_key
 * @{
*/

/**
 * @brief
 *   This function gets the random key itself.
 *  @param   pxRandomKey
 *             Random key object to use.
 *  @param   pxDataLength
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Random key length in byte.
 *  @param   ppxData
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Random key itself.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caRandomKeyGetData
(
  const TCaRandomKey*    pxRandomKey,
        TSize*           pxDataLength,
        TUnsignedInt8** ppxData
);

/**
 * @brief
 *   This function gets the CMM related to the random key.
 *  @param   pxRandomKey
 *             Random key object to use.
 *  @param   pxCmmLength
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: CMM length in byte.
 *  @param  ppxCmm
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: CMM.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caRandomKeyGetCmm
(
  const TCaRandomKey*    pxRandomKey,
        TSize*           pxCmmLength,
        TUnsignedInt8** ppxCmm
);


/**
 *  @brief
 *    This function gets the key itself.
 *
 *  @param   pxKey
 *             Key object to use.
 *  @param   pxDataLength
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Key length in byte.
 *  @param   ppxData
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Key itself.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caKeyGetData
(
  const TCaKey*          pxKey,
        TSize*           pxDataLength,
        TUnsignedInt8** ppxData
);

/**
 * @brief
 *   This function gets the unique key data buffer.
 *
 *  @param   pxUniqueKey
 *             Unique key object to use.
 *  @param   pxDataLength
 *             Pointer filled by the function with the unique key data length
 *             in bytes. Cannot be NULL.
 *  @param   ppxData
 *             Pointer to a buffer in which this function will copy
 *             the unique key data
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caUniqueKeyGetData
(
  const TCaUniqueKey*    pxUniqueKey,
        TSize*           pxDataLength,
        TUnsignedInt8** ppxData
);

/** @} g_key */

/** @addtogroup g_signature
 * @{
*/

/**
 * @brief
 *   This function gets the signature data buffer.
 *  @param   pxThis
 *             Signature object to use.
 *  @param   pxDataLength
 *             Pointer filled by the function with the signature data length in
 *             bytes. Cannot be NULL.
 *  @param   ppxData
 *             Pointer to a buffer in which this function will copy containing
 *             the signature data
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSignatureGetData
(
  const TCaSignature*    pxThis,
        TSize*           pxDataLength,
        TUnsignedInt8** ppxData

);

/** @} g_signature */

/** @addtogroup g_service
 * @{
*/

/**
 *  @brief
 *    This function gets the DVB triplet of the service.
 *  @param   pxThis
 *             Pointer to a service object.
 *  @param   pxDvbTriplet
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: DVB triplet.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caServiceGetLocator
(
  const TCaService*  pxThis,
        TDvbTriplet* pxDvbTriplet
);

/**
 *  @brief
 *    This function gets the name of the service.
 *  @param   pxThis
 *             Pointer to a service object.
 *  @param   pxSize
 *             Pointer to fill with the size of the service name.
 *             Cannot be NULL.
 *             OUT: number of bytes in the service name.
 *  @param   ppxName
 *             Pointer to fill with the service name. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the name of the service.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caServiceGetName
(
  const TCaService*      pxThis,
        TSize*           pxSize,
  const TUnsignedInt8** ppxName
);

/**
 *  @brief
 *    This function gets the provider name of the service.
 *  @param   pxThis
 *             Pointer to a service object.
 *  @param   pxSize
 *             Pointer to fill with the size of the provider name.
 *             Cannot be NULL.
 *             OUT: number of bytes in the provider name.
 *  @param   ppxProviderName
 *             Pointer to fill with the provider name. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the name of the provider.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caServiceGetProviderName
(
  const TCaService*      pxThis,
        TSize*           pxSize,
  const TUnsignedInt8** ppxProviderName
);

/**
 *  @brief
 *    This function gets the type of the service as described
 *    in the DVB Service Descriptor.
 *  @param   pxThis
 *             Pointer to a service object.
 *  @param   pxType
 *             Pointer to fill with the service type. Cannot be NULL.
 *             OUT: type of the service.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caServiceGetType
(
  const TCaService*    pxThis,
        TUnsignedInt8* pxType
);

/** @} g_service */

/** @addtogroup g_system
 * @{
*/

/**
 * @brief
 *   This function gets the system flags.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: flags of the system.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetFlags
(
  const TCaSystem*     pxSystem,
        TSystemFlags*  pxFlags
);

/**
 * @brief
 *   This function indicates which flag returned by caSystemGetFlags is
 *   applicable.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxFlagsMask
 *             Pointer to fill with the size of the flags mask. Cannot be NULL
 *             OUT: bitmap flags mask. A bit set to 0 means that the
 *             corresponding flag is not applicable.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetFlagsMask
(
  const TCaSystem*    pxSystem,
        TFlagsMask*   pxFlagsMask
);

/**
 * @brief
 *   This function gets the CAK information.
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxVersion
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the version of the
 *             CAK. The maximum length of this string is 64 characters,
 *             terminator included.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetCakVersion
(
  const TCaSystem*     pxSystem,
  const TChar**       ppxVersion
);


/**
 * @brief
 *   This function gets the Nagra TA information.
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxVersion
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the version of the
 *             CAK. The maximum length of this string is 64 characters,
 *             terminator included.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetNagraTaVersion
(
  const TCaSystem*     pxSystem,
  const TChar**       ppxVersion
);


/**
 * @brief
 *   This function gets the IRD (or CA) serial number.
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxSerialNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: serial number of the decoder defined as a NULL-terminated
 *             string with the following format: "xx xxxx xxxx xx"

 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetIrdSerialNumber
(
  const TCaSystem*    pxSystem,
  const TChar**      ppxSerialNumber
);

/**
 * @brief
 *   This function gets the IRD (or CA) serial number, integer format.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxSerialNumber
 *             Pointer to fill with the size of the flags mask. Cannot be NULL
 *             OUT: serial number, integer format.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetIrdSerialNumberInt
(
  const TCaSystem*        pxSystem,
        TUnsignedInt32*   pxSerialNumber
);


/**
 * @brief
 *   This function gets the Nagravision project information.
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxProjectInformation
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: address of a NULL-terminated string getting the name of
 *             the Nagravision project of this set-top box
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetProjectInformation
(
  const TCaSystem*    pxSystem,
  const TChar**      ppxProjectInformation
);

/**
 * @brief
 *   This function gets the chipset unique identifier (NUID).
 *
 *  @param   pxSystem
 *             System object to use.
 *  @param  ppxNuid
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: address of a chipset identifier
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetNuid
(
  const TCaSystem*    pxSystem,
  const TChar**      ppxNuid
);

/**
 * @brief
 *   This function gets the display period of the software download popup.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxDisplayPeriod
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: the display period of the software download popup.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
 *  @deprecated
*/
NAGRA_CA_API TCaObjectStatus caSystemGetUpgradeWarningDelay
(
  const TCaSystem*    pxSystem,
        TCaDuration*  pxDisplayPeriod
);

/**
 * @brief
 *   This function gets the tags of all private descriptors supported by the CAK.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxTagsLength
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: number of tags in the tags byte-array.
 *  @param   ppxTags
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: a byte-array containing the tags of all private descriptors
 *                  supported by the CAK.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetDescriptorsTags
(
  const TCaSystem*       pxSystem,
        TSize*           pxTagsLength,
  const TUnsignedInt8** ppxTags
);

/**
 * @brief
 *   This function gets the system parental control info.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxThreshold
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: parental control threshold of the system.
 *  @param   pxBitmap
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: parental control bitmap of the system.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetParentalControl
(
  const TCaSystem*              pxSystem,
  TCaParentalControlThreshold*  pxThreshold,
  TCaParentalControlBitmap*     pxBitmap
);

/**
 * @brief
 *   This function gets the revision of the chipset (aka chipset cut).
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxChipsetRevision
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the chipset revision.
 *             The maximum length of this string is 20 characters, terminator
 *             included.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetChipsetRevision
(
  const TCaSystem*     pxSystem,
  const TChar**       ppxChipsetRevision
);

/**
 *  @brief
 *    This function gets the chipset type string (aka CTS).
 *  @param   pxSystem
 *             System object to use.
 *  @param   ppxChipsetType
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: NULL-terminated string containing the chipset type.
 *             The maximum length of this string is 20 characters, terminator
 *             included.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetChipsetType
(
  const TCaSystem*     pxSystem,
  const TChar**       ppxChipsetType
);

/**
 *  @brief
 *   This function gets the security architecture ID related to the chipset
 *   pairing feature.
 *  @param   pxSystem
 *             System object to use.
 *  @param   pxChipsetPairingSaId
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: chipset pairing security architecture ID
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetChipsetPairingSaId
(
  const TCaSystem*      pxSystem,
        TUnsignedInt8*  pxChipsetPairingSaId
);


/**
 *  @brief
 *    This function gets the maximum index of CSC data currently stored in flash.
 *    The index returned takes into account CSC data programmed in OTP during
 *    manufacturing and CSC data received over the air.
 *
 *  @param[in]  pxSystem
 *                System object to use.
 *  @param[out] pxCscMaxIndex
 *                CSC maximum index currently stored in flash
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caSystemGetCscMaxIndex
(
  const TCaSystem*      pxSystem,
        TUnsignedInt8*  pxCscMaxIndex
);

/**
 *  @brief
 *    This function gets the list of ECM Mpeg filter configurations of this
 *    system.
 *
 *  @param[in]  pxSystem
 *               System object to use. Should not be NULL.
 *  @param[out] pxNumberOfFilterCfgs
 *               Pointer to fill with the information. Should not be NULL.
 *               OUT: Number of Mpeg filter configurations in the array.
 *  @param[out] pppxFilterCfgsArray
 *               Pointer to fill with the information. Should not be NULL.
 *               OUT: array of pointers to Mpeg filter configuration objects.
 *                    Objects remain valid as long as the pxSystem is valid.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caSystemGetEcmMpegFilterCfgs
(
  const TCaSystem*           pxSystem,
        TUnsignedInt32*      pxNumberOfFilterCfgs,
  const TCaMpegFilterCfg***  pppxFilterCfgsArray
);

/**
 *  @brief
 *    This function gets the list of EMM Mpeg filter configurations of this
 *    system.
 *
 *  @param[in]  pxSystem
 *               System object to use. Should not be NULL.
 *  @param[out] pxNumberOfFilterCfgs
 *               Pointer to fill with the information. Should not be NULL.
 *               OUT: Number of Mpeg filter configurations in the array.
 *  @param[out] pppxFilterCfgsArray
 *               Pointer to fill with the information. Should not be NULL.
 *               OUT: array of pointers to Mpeg filter configuration objects.
 *                    Objects remain valid as long as the pxSystem is valid.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caSystemGetEmmMpegFilterCfgs
(
  const TCaSystem*           pxSystem,
        TUnsignedInt32*      pxNumberOfFilterCfgs,
  const TCaMpegFilterCfg***  pppxFilterCfgsArray
);

/** @} g_system */

/** @addtogroup g_operator
 * @{
*/

/**
 * @brief
 *   This function returns the operator's identifier.
 *
 *  @param   pxOperator
 *             Pointer on a operator object.
 *  @param   pxPpid
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Operator's ID.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetId
(
  const TCaOperator*    pxOperator,
        TPpid*          pxPpid
);

/**
 * @brief
 *   This function returns information about an operator.
 *
 *  @param   pxOperator
 *             Pointer on a operator object.
 *  @param   pxFlags
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: bitmap flags value.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetFlags
(
  const TCaOperator*        pxOperator,
        TOperatorFlags*     pxFlags
);

/**
 * @brief
 *   This function indicates which flag returned from caOperatorGetFlags
 *   is applicable.
 *
 *  @param   pxOperator
 *             Pointer on a operator object.
 *  @param   pxFlagsMask
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: bitmap flags mask: a bit set to 0 means that the
 *             corrisponding flag is not applicable.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetFlagsMask
(
  const TCaOperator*      pxOperator,
        TFlagsMask*       pxFlagsMask

);

/**
 * @brief
 *   This function gets the operator expiration date.
 *
 *  @param   pxThis
 *             Operator object to use.
 *  @param   pxExpirationDate
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: expiration date for the operator.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetExpirationDate
(
  const TCaOperator*    pxThis,
        TCalendarTime*  pxExpirationDate
);

/**
 * @brief
 *   This function gets the operator Ecm Ca System Id.
 *
 *  @param   pxThis
 *             Operator object to use.
 *  @param   pxEcmCaSystemId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: ECM CA system ID for the operator.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetEcmCaSystemId
(
  const TCaOperator* pxThis,
        TCaSystemId* pxEcmCaSystemId
);

/**
 * @brief
 *   This function gets the operator Emm Ca System Id.
 *
 *  @param   pxThis
 *             Operator object to use.
 *  @param   pxEmmCaSystemId
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: EMM CA system ID for the operator.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetEmmCaSystemId
(
  const TCaOperator* pxThis,
        TCaSystemId* pxEmmCaSystemId
);

/**
 *  @brief
 *    This function gets the list of servers.
 *
 *  @param   pxOperator
 *             Pointer to an operator object.
 *  @param   pxNumberOfServers
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: number of servers in the array.
 *  @param   pppxServerArray
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: Array of servers.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetServers
(
  const TCaOperator*    pxOperator,
        TUnsignedInt32* pxNumberOfServers,
  const TCaServer*** pppxServerArray
);

/**
 * @brief
 *   This function gets the operator renewal date.
 *
 *  @param   pxThis
 *             Operator object to use.
 *  @param   pxRenewalDate
 *             Pointer to fill with the information. Cannot be NULL.
 *             OUT: renewal date of the operator.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetRenewalDate
(
  const TCaOperator*    pxThis,
        TCalendarTime*  pxRenewalDate
);

/**
 *  @brief
 *    This function gets the name of the operator.
 *  @param   pxOperator
 *             Pointer to an operator object.
 *  @param   pxSize
 *             Pointer to fill with the size of the operator name.
 *             Cannot be NULL.
 *             OUT: number of bytes in the operator name.
 *  @param   ppxName
 *             Pointer to fill with the operator name. Cannot be NULL.
 *             OUT: NULL-terminated string of pxSize bytes
 *                  getting the name of the operator.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caOperatorGetName
(
  const TCaOperator*      pxOperator,
        TSize*            pxSize,
  const TUnsignedInt8**  ppxName
);

/** @} g_operator */

/** @addtogroup g_connection
 * @{
*/

/**
 * @brief
 *   This function gets the date of the connection.
 *
 *
 *  @param[in]  pxThis
 *                Connection object to use.
 *  @param[out] pxDate
 *                Pointer to be filled in with the date of the connection.
 *                Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caConnectionGetDate
(
  const TCaConnection*  pxThis,
        TCalendarTime*  pxDate
);

/**
 * @brief
 *   This function gets the result of the connection.
 *
 *  @param[in]  pxThis
 *                Connection object to use.
 *  @param[out] pxResult
 *                Pointer to be filled in with the result of the connection.
 *                Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caConnectionGetResult
(
  const TCaConnection*       pxThis,
        TCaConnectionResult* pxResult
);

/**
 * @brief
 *   This function gets the state of the connection.
 *
 *  @param[in]  pxConnection
 *                Connection object to use.
 *  @param[out] pxState
 *                Pointer to be filled in with the state of the connection.
 *                Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caConnectionGetState
(
  const TCaConnection*         pxConnection,
        TCaConnectionState*    pxState
);

/**
 * @brief
 *   This function returns statistics computed from the data exchange over a
 *   connection. These statistical data and their format are customer-specific.
 *
 *  @param[in]  pxConnection
 *                Connection object to use.
 *  @param[out] pxSize
 *                Pointer to be filled with the size (in bytes) of the buffer
 *                containing the statistical data. Cannot be NULL.
 *  @param[out] ppxData
 *                Pointer to be filled with the connection statistical data.
 *                Refer to customer specific documentation for the definition
 *                and format of these data. Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caConnectionGetStatistics
(
  const TCaConnection*    pxConnection,
        TSize*            pxSize,
        TUnsignedInt8**  ppxData
);

/** @} g_connection */

/** @addtogroup g_server
 * @{
*/

/**
 * @brief
 *   This function returns the details of the last connection that occurred
 *   between the CAK and this server.
 *
 *  @param[in]   pxServer
 *                 Server object to use.
 *  @param[out] ppxLastConnection
 *                Pointer to be filled in with a caConnection object. Cannot be
 *                NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caServerGetLastConnection
(
  const TCaServer*       pxServer,
  const TCaConnection** ppxLastConnection
);

/**
 * @brief
 *   This function returns the details of the next connection to be established
 *   between the CAK and this server.
 *
 *  @param[in]  pxServer
 *                Server object concerned by this call
 *  @param[out] ppxNextConnection
 *                Pointer to be filled in with a caConnection object. Cannot be
 *                NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caServerGetNextConnection
(
  const TCaServer*       pxServer,
  const TCaConnection** ppxNextConnection
);

/**
 * @brief
 *   This function returns the delay that could be enforced to further
 *   connections following a remote error with this server.
 *
 *  @param[in]  pxServer
 *                Server object concerned by this call
 *  @param[out] pxDelay
 *                Pointer to be filled in with the remote error delay, expressed
 *                in seconds. Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caServerGetRemoteErrorDelay
(
  const TCaServer*    pxServer,
        TCaDuration*  pxDelay
);

/**
 * @brief
 *   This function returns the last error remote failure date.
 *
 *  @param[in]  pxServer
 *                Server object concerned by this call
 *  @param[out] pxLastRemoteFailureDate
 *                Pointer to be filled in with the last remote failure date.
 *                Cannot be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caServerGetLastRemoteFailureDate
(
  const TCaServer*      pxServer,
        TCalendarTime*  pxLastRemoteFailureDate
);

/** @} g_server */

/** @addtogroup g_popup
 * @{
*/

/**
 *  @brief
 *    This function gets the text associated to a popup.
 *
 *  @param[in]   pxPopup
 *    Pointer to a popup object.
 *
 *  @param[out]  pxSize
 *    Size in bytes of the text
 *
 *  @param[out]   ppxText
 *    Buffer, allocated by the CAK, containing the popup text (pxSize bytes).
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caPopupGetText
(
  const TCaPopup*         pxPopup,
        TSize*            pxSize,
  const TUnsignedInt8**  ppxText
);

/** @} g_popup */


/** @addtogroup g_stblock
 * @{
*/

/**
 * @brief
 *   This function gets the flags of the stb lock.
 *
 *  @param[in]   pxStbLock
 *    Pointer on a stb lock object.
 *
 *  @param[out]   pxFlags
 *    Flags of the stb lock object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caStbLockGetFlags
(
  const TCaStbLock*     pxStbLock,
        TStbLockFlags*  pxFlags
);

/**
 * @brief
 *   This function gets the flags mask of the stb lock.
 *
 *  @param[in]   pxStbLock
 *    Pointer on a stb lock object.
 *
 *  @param[out]   pxFlagsMask
 *    Flags mask of the stb lock object.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caStbLockGetFlagsMask
(
  const TCaStbLock*     pxStbLock,
        TFlagsMask*     pxFlagsMask
);

/** @} g_stblock */


/** @addtogroup g_mpeg_filter_configuration
 * @{
*/

/**
 *  @brief
 *    This function gets the Ca System Identifier to be used to parse the CAT
 *    or PMT tables during the lookup of the EMM or ECM PID.
 *
 *  @param[in]   pxThis
 *    Pointer to a Mpeg filter configuration object. Should not be NULL.
 *
 *  @param[out]  pxCaSystemId
 *    Pointer to fill with the information. Should not be NULL.
 *    OUT: Ca System Identifier of the filter.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caMpegFilterCfgGetCaSystemId
(
  const TCaMpegFilterCfg*  pxThis,
        TCaSystemId*       pxCaSystemId
);

/**
 *  @brief
 *    This function gets the PPID which can be used to parse
 *    the CAT or PMT tables during the lookup of the EMM or ECM PID.
 *
 *  @param[in]   pxThis
 *    Pointer to a Mpeg filter configuration object. Should not be NULL.
 *
 *  @param[out]  pxPpid
 *    Pointer to fill with the information. Cannot be NULL.
 *    OUT: Operator's ID associated to the filter.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caMpegFilterCfgGetPpid
(
  const TCaMpegFilterCfg*  pxThis,
        TPpid*             pxPpid
);

/**
 *  @brief
 *    This function gets the pattern to be used to filter MPEG sections.
 *
 *  Pattern applies to the MPEG section starting from the table identifier byte.
 *
 *  @param[in]   pxThis
 *    Pointer to a Mpeg filter configuration object. Should not be NULL.
 *
 *  @param[out]  pxSize
 *    Size in bytes of the pattern. Should not be NULL.
 *
 *  @param[out]   ppxPattern
 *    Buffer, allocated by the CAK, containing the filter pattern
 *    (pxSize bytes). Returned pointer will be valid as long as pxThis
 *    is valid.<br>
 *    Should not be NULL.
 *
 *  @param[out]   ppxPatternMask
 *    Buffer, allocated by the CAK, containing the filter pattern mask
 *    (pxSize bytes). Returned pointer will be valid as long as pxThis
 *    is valid.<br>
 *    Should not be NULL.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Successful.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL).
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK.
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred.
*/
NAGRA_CA_API TCaObjectStatus caMpegFilterCfgGetPattern
(
  const TCaMpegFilterCfg*  pxThis,
        TSize*             pxSize,
  const TUnsignedInt8**    ppxPattern,
  const TUnsignedInt8**    ppxPatternMask
);

/** @} g_mpeg_filter_configuration */


/** @addtogroup g_pullEmmContext
 * @{
*/

/**
 * @brief
 *   This function gets the token of a context object.
 *
 *  @param   pxPullEmmContext
 *             Pointer on a context object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: token size.
 *  @param   ppxToken
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the token buffer.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPullEmmContextGetToken
(
  const TCaPullEmmContext*    pxPullEmmContext,
        TSize*                pxSize,
        TUnsignedInt8**      ppxToken
);

/**
 * @brief
 *   This function gets the signature of a context object.
 *
 *  @param   pxPullEmmContext
 *             Pointer on a context object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: signature size.
 *  @param   ppxSignature
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the signature buffer.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPullEmmContextGetSignature
(
  const TCaPullEmmContext*    pxPullEmmContext,
        TSize*                pxSize,
        TUnsignedInt8**      ppxSignature
);

/**
 * @brief
 *   This function gets the serial number of a context object.
 *
 *  @param   pxPullEmmContext
 *             Pointer on a context object.
 *  @param   ppxSerialNumber
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the serial number buffer defined as a
 *             NULL-terminated string with the following format:
 *             "xxxxxxxxxx"
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPullEmmContextGetSerialNumber
(
  const TCaPullEmmContext*    pxPullEmmContext,
  const TChar**              ppxSerialNumber
);

/**
 * @brief
 *   This function gets the private data of a context object.
 *
 *  @param   pxPullEmmContext
 *             Pointer on a context object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: private data size.
 *  @param   ppxPrivateData
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: pointer to the private buffer.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPullEmmContextGetPrivateData
(
  const TCaPullEmmContext*    pxPullEmmContext,
        TSize*                pxSize,
        TUnsignedInt8**      ppxPrivateData
);

/**
 * @brief
 *   This function gets the PPID of a context object.
 *
 *  @param   pxPullEmmContext
 *             Pointer on a context object.
 *  @param   pxSize
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: Length of the PPID string.
 *  @param   ppxPpid
 *             Pointer to fill with the information. Cannot be NULL
 *             OUT: PPID string value.
 *
 *  @retval  CA_OBJECT_NO_ERROR
 *             Success.
 *  @retval  CA_OBJECT_INVALID
 *             The object is invalid for this getter.
 *  @retval  CA_OBJECT_PARAMETER_INVALID
 *             A parameter is invalid (probably set to NULL)
 *  @retval  CA_OBJECT_NOT_AVAILABLE
 *             This information is not available in the CAK
 *  @retval  CA_OBJECT_ERROR
 *             An unspecified error occurred
 *
*/
NAGRA_CA_API TCaObjectStatus caPullEmmContextGetPpid
(
  const TCaPullEmmContext*    pxPullEmmContext,
        TSize*                pxSize,
        TUnsignedInt8**      ppxPpid
);

/** @} g_pullEmmContext */

/** @addtogroup g_log
 * @{
*/

/**
 *  @brief
 *    Re-enables all log messages.
 *
 *    Restores the log levels from the internal table.
 *
 *  @pre
 *    logDisable must have been called.
*/
NAGRA_CA_API void caLogEnable
(
  void
);


/**
 *  @brief
 *    Disables all log messages.
 *
 *    Saves the current log levels in an internal table.
 *
 *  @post
 *    logEnable may be called.
*/
NAGRA_CA_API void caLogDisable
(
  void
);


/**
 * @brief
 *   Returns all applicable log categories
 * @param[out]   pxNumOfCategories
 *   Number of categories returned in the pxCategories array
 * @param[out]   pppxCategories
 *   Pointer to array of pxNumOfCategories null-terminated strings identifying
 *   applicable categories.
 * @see caLogSetRole(), caLogSetFeature()
*/
NAGRA_CA_API void caLogGetCategories
(
  TSize*     pxNumOfCategories,
  TChar*** pppxCategories
);


/**
 * @brief
 *   Returns all applicable log roles
 * @param[out]   pxNumOfRoles
 *   Number of roles returned in the pxRoles array
 * @param[out]   pppxRoles
 *   Pointer to array of pxNumOfRoles null-terminated strings identifying
 *   applicable roles
 * @see caLogSetRole()
*/
NAGRA_CA_API void caLogGetRoles
(
  TSize*      pxNumOfRoles,
  TChar***  pppxRoles
);


/**
 * @brief
 *   Defines the log traces output by the role pxRole.
 * @param[in]   pxRole
 *   String identifying the role concerned
 * @param[in]   pxCategory
 *   String identifying the category associated to the role
*/
NAGRA_CA_API void caLogSetRole
(
  const TChar*  pxRole,
  const TChar*  pxCategory
);


/**
 * @brief
 *   Returns all applicable log features
 * @param[out]   pxNumOfFeatures
 *   Number of features returned in the pxFeatures array
 * @param[out]   pppxFeatures
 *   Pointer to array of pxNumOfFeatures null-terminated strings identifying
 *  applicable  features.
 * @see caLogSetFeature()
*/
NAGRA_CA_API void caLogGetFeatures
(
  TSize*     pxNumOfFeatures,
  TChar*** pppxFeatures
);


/**
 * @brief
 *   Defines the log traces output by the feature pxFeature.
 * @param[in]   pxFeature
 *   String identifying the feature concerned
 * @param[in]   pxCategory
 *   String identifying the category associated to the feature
*/
NAGRA_CA_API void caLogSetFeature
(
  const TChar*  pxFeature,
  const TChar*  pxCategory
);

/** @} g_log */

/******************************************************************************/
/*                                                                            */
/*                          LEGACY FUNCTIONAL API                             */
/*                                                                            */
/******************************************************************************/


/** @cond DOX_NO_EXPORT */

/**
 *  @brief
 *    Debug module identifiers. Used in CAK V6 and in CAN.
 *  @warning
 *    Do addition at the end, just before DEBUG_NUM_MODULES.
*/
typedef enum
{
  MOD_APPL,      /**<  0: Application                     */
  MOD_INIT,      /**<  1: Initialization                  */
  MOD_OSY,       /**<  2: Operation System                */
  MOD_MEM,       /**<  3: Memory Allocator                */
  MOD_GLB,       /**<  4: Global                          */
  MOD_TLS,       /**<  5: Tools (list, queue, ...)        */
  MOD_TLS_VTT,   /**<  6: Tools (vtt only)                */
  MOD_TLS_MEM,   /**<  7: Tools (vtt only)                */
  MOD_DMX,       /**<  8: Demultiplexer                   */
  MOD_DSC,       /**<  9: Descrambler                     */
  MOD_PSI,       /**< 10: Programm Specific Information   */
  MOD_ICC,       /**< 11: Integrated Circuit Card         */
  MOD_RPH,       /**< 12: Return Path                     */
  MOD_TST,       /**< 13: Test                            */
  MOD_CAK,       /**< 14: Conditionnal Access Kernel      */
  MOD_DRT,       /**< 15: Data Router Task                */
  MOD_DRT_EMM,   /**< 16: Data Router Task EMM            */
  MOD_DRT_ECM,   /**< 17: Data Router Task ECM            */
  MOD_DRT_ALR,   /**< 18: Data Router Task Alarm          */
  MOD_ICT,       /**< 19: ICC Task                        */
  MOD_ICT_EMM,   /**< 20: ICC Task EMM                    */
  MOD_ICT_ECM,   /**< 21: ICC Task ECM                    */
  MOD_ICT_PRK,   /**< 22: ICC Task Pairing                */
  MOD_ICT_SCK,   /**< 23: Secure Clock                    */
  MOD_ICT_SSE,   /**< 24: ICC Task for SSE                */
  MOD_DBT,       /**< 25: Database Task                   */
  MOD_MGT,       /**< 26: Management Task                 */
  MOD_ITT,       /**< 27: ITM Task                        */
  MOD_EXP,       /**< 28: Execptions                      */
  MOD_IT1,       /**< 29: T=1 ICC Interface               */
  MOD_MON,       /**< 30: Monitor Task                    */
  MOD_UIT,       /**< 31: User Interface Task             */
  MOD_ITM,       /**< 32: ITM Server                      */
  MOD_CRPT,      /**< 33: Cryptographic algorithms        */
  MOD_CAO,       /**< 34: Conditionnal Access OpenTV      */
  MOD_VFS,       /**< 35: Versatile File System           */
  MOD_OMM,       /**< 36: Out-Of-Band EMM                 */
  MOD_CDE,       /**< 37: Digital Video Recorder extension*/
  MOD_CAR,       /**< 38: Conditional Access Root         */
  MOD_MAI,       /**< 39: Multiprocessor Application interface  */
  MOD_IPC,       /**< 40: Inter-Process Communication     */
  MOD_CAN,       /**< 41: Conditional Access NagraVision  */
  MOD_BDC,       /**< 42: Basic Display Control           */
  MOD_COP,       /**< 43: Copy Protection                 */
  MOD_SSE,       /**< 44: Secure Set-Top Box Engine       */
  MOD_DIL,       /**< 45: Data Item Loader (DIL main)     */
  MOD_PPP,       /**< 46: PES Post-Processor (DIL module) */
  MOD_TSM,       /**< 47: Tuner Session Manager (DIL)     */
  MOD_PTA,       /**< 48: PMT Table Analyzer    (DIL)     */
  MOD_SPL,       /**< 49: Synchro Pid Listener  (DIL)     */
  MOD_DLM,       /**< 50: Downloads Manager     (DIL)     */
  MOD_GCO,       /**< 51: Garbage Collector     (DIL)     */
  MOD_LDR,       /**< 52: Loader Main Module    (DIL)     */
  MOD_SCB,       /**< 53: Scrib (DIL module)              */
  MOD_IBT,       /**< 54: In-Band Tuner driver            */
  MOD_DIS,       /**< 55: Data Item Storage driver        */
  MOD_PTL,       /**< 56: PSI Table Loader driver         */
  MOD_UTC,       /**< 57: UTCASH                          */
  MOD_PDT,       /**< 58: Program Definition Task         */
  MOD_GEN_REQ,   /**< 59: Generic Requests                */
  MOD_GEN_REG,   /**< 60: Generic Registrations           */
  MOD_AIN,       /**< 61: Application Interface NagraVision  */
  MOD_OSN,       /**< 62: Operation System NagraVision    */
  MOD_CAJ,       /**< 63: CAJ fake client                 */
  MOD_KDA,       /**< 64: Kernel Device Agent             */
  MOD_PSL,       /**< 65: Program Signalling Loader       */
  MOD_ESL,       /**< 66: EMM Signalling Loader           */
  MOD_DSX,       /**< 67: Descrambler Extended            */
  MOD_DBD,       /**< 68: Data Block Decryptor            */
  MOD_FFP,       /**< 69: FVOD File Parser                */
  MOD_SDAG,      /**< 70: Secure Device Aladin Glue       */
  MOD_SDM,       /**< 71: Secure Device Module            */
  MOD_SHPM,      /**< 72: Shpm Module                     */
  MOD_PEV,       /**< 73: Pushed-ECM Vod module           */
  MOD_SDCL,      /**< 74: Secure Device Cardless          */
  MOD_SDME,      /**< 75: Secure Device Merlin            */
  MOD_NDSP,      /**< 76: Object Nagra DeScriPtor         */
  MOD_FOOG,      /**< 77: Functional Over Object Glue     */
  MOD_DB,        /**< 78: Database behaviour              */
  MOD_DBG,       /**< 79: Debug debugging                 */
  MOD_SEC,       /**< 80: Secure chipset                  */
  MOD_SCR,       /**< 81: Stream crypto engine            */
  MOD_CASH,      /**< 82: CA shell                        */
  MOD_SDNG,      /**< 83: Secure Device Can Glue          */
  MOD_SCS,       /**< 84: Smartcard simulator             */
  MOD_DVL,       /**< 85: DVR/VOD Library                 */
  MOD_SCE,       /**< 86: Steady Clock Engine             */
  MOD_COA,       /**< 87: Cohabitation table              */
  MOD_SHCA,      /**< 88: Shca                            */
  MOD_SDAL,      /**< 89: Secure Device Aladin            */
  MOD_RPX,       /**< 90: Remote Positioning eXtension Module */
  MOD_SDTI,      /**< 91: secure device Tiger             */
  DEBUG_NUM_MODULES /**< 92: Module Count                 */
} TDebugModule;

/**
 * @brief
 *   Flag used to perform a control for every module
*/
#define MOD_ALL    0xFFFFFFFF

/**
 * @brief
 *   Debug classes. Used in CAK V6 and in CAN
*/
#define CLASS_NONE        0
  /**< no role */
#define CLASS_ALL         ((TDebugClass32)(-1))
  /**< all classes */
#define CLASS_FUNCTION    ((TDebugClass32)(CLASS_ENTRY | CLASS_EXIT))
  /**< entry + exit point of a function; replaces CLASS_FUNC */

#define CLASS_GENERIC     BIT0   /**< default class */
#define CLASS_FUNC        BIT1   /**< entry/exit points; deprecated (use FUNCTION) */
#define CLASS_LOOP        BIT2   /**< cycling loops */
#define CLASS_SWITCH      BIT3   /**< switch and branch */
#define CLASS_API         BIT3   /**< API; deprecated (use API role) */
#define CLASS_WAIT        BIT4   /**< waiting intervals */
#define CLASS_PERF        BIT5   /**< performance tests */
#define CLASS_PAYLOAD     BIT6   /**< data exchanges */
#define CLASS_TEST        BIT7   /**< tests */
#define CLASS_PARAMETER   BIT8   /**< parameters of a function */
#define CLASS_ENTRY       BIT9   /**< entry point of a function */
#define CLASS_EXIT        BIT10  /**< exit point of a function */
#define CLASS_FIELD       BIT11  /**< field of a structure */
#define CLASS_ERROR       BIT12  /**< error */
#define CLASS_WARNING     BIT13  /**< warning */

#define DEBUG_CLASS_NB 32

typedef TUnsignedInt16 TDebugClass;
typedef TUnsignedInt32 TDebugClass32;

/**
 * @brief
 *   Verbosity level to control the amount of messages to display.
*/
typedef enum
{
  VERBOSITY_SILENT,
  /**< no output at all, except DBG_ALWAYS; default value */
  VERBOSITY_LOW,
  /**< global debug messages */
  VERBOSITY_MIDDLE,
  /**< global detailed debug messages */
  VERBOSITY_HIGH,
  /**< internal detailed debug messages */
  VERBOSITY_MAX,
  /**< internal very detailed debug messages */
  DEBUG_VERBOSITY_NB
  /**< number of verbosities (5) */
} TDebugVerbosity;

/**
 * @brief
 *   Prevent a conflict with Windows switch
*/
#ifdef _WIN32
#define CAI NAGRA_CAI
#endif /* _WIN32 */

/**
 *  @brief
 *    Debug module identifiers. Used in CAK V5 only
 *  @warning
 *    Do addition at the end
 *  @deprecated
 *    To be used only with debugPrintMessage() and debugPrintArray()
*/
typedef enum
{
  INIT,               /**< 00: Initialization         */
  CAOS,               /**< 01: CAT scheduler          */
  CADB,               /**< 02: CAT data base          */
  ALARM,              /**< 03: Alarms                 */
  ECM,                /**< 04: CAT Ecm                */
  EMM,                /**< 05: CAT Emm                */
  ICC,                /**< 06: Smart card command     */
  ICINF,              /**< 07: Smart card information */
  ICADR,              /**< 08: Smart card addresses   */
  CACC,               /**< 09: CAT call collector     */
  REFNB,              /**< 10: Reference number       */
  EVINF,              /**< 11: Event information      */
  IPPV,               /**< 12: Impulse purchase       */
  PLIST,              /**< 13: Pay-per-view list      */
  CREDT,              /**< 14: Credit                 */
  SYINF,              /**< 15: System information     */
  IRDCM,              /**< 16: Ird command            */
  IRD,                /**< 17: Ird                    */
  TIME,               /**< 18: Time                   */
  TIMER,              /**< 19: Timer                  */
  ERFLT,              /**< 20: Icc error handler      */
  STDMSG,             /**< 21: Standard message       */
  DBUG,               /**< 22: Debug                  */
  CDRT,               /**< 23: Ecm DRT                */
  MDRT,               /**< 24: Emm DRT                */
  IDRT,               /**< 25: Iemm DRT               */
  PSQ,                /**< 26: Protected static queue */
  CCT,                /**< 27: CCT                    */
  OSY,                /**< 28: OSY                    */
  CCD,                /**< 29: CCD                    */
  ICD,                /**< 30: ICD                    */
  STB,                /**< 31: Stb test environment   */
  CAI,                /**< 32: CA/SDK interface       */
  DRT,                /**< 33: DRT                    */
  PSI,                /**< 34: PSI                    */
  DMX,                /**< 35: DMX                    */
  DSC,                /**< 36: DSC                    */
  MEM,                /**< 37: MEM                    */
  IT1,                /**< 38: IT1                    */
  T1S,                /**< 39: T1S                    */
  MON,                /**< 40: Monitor command.       */
  EXCPT,              /**< 41: EXCPT                  */
  APPL,               /**< 42: Application            */
  MSGQ,               /**< 43: Message queue          */
  PHE,                /**< 44: POD Html engine        */
  ASSERT_FAILURE,     /**< 45: Assertion failure      */
  MTT,                /**< 46: Message Task Template  */
  LST,                /**< 47: Generic list module    */
  DEBUG_NUM_MODULE_IDS /**< 48: Number of module       */
} TDebugModuleId;

#ifdef _WIN32
#undef CAI /* prevent a conflict with Windows switch */
#endif /* _WIN32 */

/**
 * @brief
 *   Debug level
 * @deprecated
 *   To be used only with debugPrintMessage() and debugPrintArray().
*/
typedef enum
{
  LEVEL_DISABLED,
  LEVEL_LOW,
  LEVEL_MEDIUM,
  LEVEL_HIGH,
  LEVEL_MAX
} TDebugLevel;

/**
 * @brief
 *   Debug role (functionality)
*/
typedef TUnsignedInt32 TDebugRole;
#define DEBUG_ROLE_NONE       0
  /**< no role */
#define DEBUG_ROLE_ALL        ((TDebugRole)(-1))
  /**< all roles */

#define DEBUG_ROLE_GENERIC                BIT0  /**< default role for generic functions */
#define DEBUG_ROLE_API                    BIT1  /**< API of the DAL or the CAK */
#define DEBUG_ROLE_PURCHASE               BIT2  /**< product or event purchase */
#define DEBUG_ROLE_PRODUCT_LIST           BIT3  /**< product list */
#define DEBUG_ROLE_SMARTCARD              BIT4  /**< smartcard issues (T=1, com, alarms) */
#define DEBUG_ROLE_EVENT                  BIT5  /**< event information requests */
#define DEBUG_ROLE_PROGRAM                BIT6  /**< program requests and notifications */
#define DEBUG_ROLE_ECM                    BIT7  /**< ECM processing and CW setting */
#define DEBUG_ROLE_EMM                    BIT8  /**< EMM processing and DB reload */
#define DEBUG_ROLE_COLLECT                BIT9  /**< call collection trigger and process */
#define DEBUG_ROLE_PURCHASE_HISTORY       BIT10 /**< purchase history request */
#define DEBUG_ROLE_RECHARGE               BIT11 /**< recharge history/new request */
#define DEBUG_ROLE_IRD_COMMAND            BIT12 /**< IRD command reception/exportation */
#define DEBUG_ROLE_PDT_FILTERING          BIT13 /**< PDT filtering and loading */
#define DEBUG_ROLE_CONSUMPTION            BIT14 /**< Consumption request */
#define DEBUG_ROLE_VOD                    BIT15 /**< video on demand */
#define DEBUG_ROLE_PVP                    BIT15 /**< Personal Video Player */
#define DEBUG_ROLE_PUSH_VOD               BIT16 /**< push video on demand */
#define DEBUG_ROLE_PAIRING                BIT17 /**< pairing data */
#define DEBUG_ROLE_TEST                   BIT18 /**< test about a module */
#define DEBUG_ROLE_PERFORMANCE            BIT19 /**< performance tests */
#define DEBUG_ROLE_SYSTEM                 BIT20 /**< system information request */
#define DEBUG_ROLE_CREDIT                 BIT21 /**< credit list request */
#define DEBUG_ROLE_HASH_SIGN              BIT22 /**< hash sign request */
#define DEBUG_ROLE_UNIQUE_KEY             BIT23 /**< unique key request */
#define DEBUG_ROLE_OPERATOR_RESTRICTION   BIT23 /**< unique key request */
#define DEBUG_ROLE_PARENTAL_CONTROL       BIT24 /**< parental control request */
#define DEBUG_ROLE_RANDOM_KEY             BIT25 /**< random key request */
#define DEBUG_ROLE_CLESE                  BIT25 /**< clese processing */
#define DEBUG_ROLE_PIN_CODE               BIT26 /**< PIN code check/change */
#define DEBUG_ROLE_NETWORK                BIT27 /**< network ID list request */
#define DEBUG_ROLE_OPERATOR               BIT28 /**< MOP/SOP */
#define DEBUG_ROLE_RPX                    BIT29 /**< Remote Positioning eXtension */
#define DEBUG_ROLE_RESYNC_CALLBACK        BIT30 /**< resync callback request */


#define DEBUG_ROLE_SPECIFIC         BIT31
  /**<
   * used for specific independent roles; use this value for any
   * functionality that should not interact with another one
  */

/**
 * @brief
 *   Debug role (remapping) (temporary)
*/
#define DEBUG_ROLE_PRM             DEBUG_ROLE_VOD


/**
 *  @brief
 *    Disables all debug messages.
 *
 *    Saves the current debug levels in an internal table.
 *
 *  @post
 *    debugEnable may be called.
 *
 *  @deprecated
*/
NAGRA_CA_API void caDebugDisable
(
  void
);


/**
 *  @brief
 *    Re-enables all debug messages.
 *
 *    Restores the debug levels from the internal table.
 *
 *  @pre
 *    debugDisable must have been called.
 *
 *  @deprecated
*/
NAGRA_CA_API void caDebugEnable
(
  void
);


/**
 * @brief
 *   Defines the debug traces outputted by the module xModule.
 *   Debug of any role is outputted. Use the function
 *   debugControlRole() to control also the debug role.
 * @param   xModule
 *   Concerned module, or MOD_ALL to control all modules
 * @param   xVerbosity
 *   Verbosity that this module must use for debug
 *   (from VERBOSITY_LOW to VERBOSITY_MAX)
 * @param  xClass
 *   Class(es) that this module must output, or CLASS_ALL to
 *   output any class
 * @deprecated
 *   Use debugControlRole() instead.
 * @remarks
 * - Replaces the previous control performed for the concerned module(s).
 *   The previous control may have been hard-coded through dbginit file,
 *   or done through a previous call to a debugControlXxx() function.
 * @deprecated
*/
NAGRA_CA_API void caDebugControlX
(
  TDebugModule     xModule,
  TDebugVerbosity  xVerbosity,
  TDebugClass32    xClass
);

/**
 * @brief
 *   Defines the debug traces outputted by the module xModule.
 * @param   xModule
 *   Concerned module, or MOD_ALL to control all modules
 * @param   xVerbosity
 *   Verbosity that this module must use for debug
 *   (from VERBOSITY_LOW to VERBOSITY_MAX)
 * @param  xClass
 *   Class(es) that this module must output, or CLASS_ALL to
 *   output any class
 * @param   xRole
 *   Role(s) of output that this module must output, or
 *   DEBUG_ROLE_ALL to output any role
 * @remarks
 * - Replaces the previous control performed for the concerned module(s).
 *   The previous control may have been hard-coded through dbginit file,
 *   or done through a previous call to a debugControlXxx() function.
 * @deprecated
*/
NAGRA_CA_API void caDebugControlRole
(
  TDebugModule     xModule,
  TDebugVerbosity  xVerbosity,
  TDebugClass32    xClass,
  TDebugRole       xRole
);

/**
 * @brief
 *   Sets the debug level for the given module. Used for old modules (CAK 5).
 * @param  xModuleId
 *   Module identifier concerned by the debug.
 * @param  xLevel
 *   Level the module has to be set to.
 * @deprecated
 *   Use debugControlX for the new modules (CAK 6).
*/
NAGRA_CA_API void caDebugSetLevel
(
  TDebugModuleId  xModuleId,
  TDebugLevel     xLevel
);


/**
 *  @brief
 *    Size of the old zip code in bytes.
 *
 *  @deprecated
 *  @see TZipCode
*/
#define CA_ZIP_CODE_LEN  6


/**
 *  @brief
 *    Icc capabilities revision string length
*/
#define CAK_ICC_CAPABILITIES_STRING_LENGTH       4

/**
 *  @brief
 *    Icc capabilities revision NULL char position in the pRevision string
*/
#define CAK_ICC_CAPABILITIES_NULL_INDEX          0

/**
 *  @brief
 *    Icc capabilities  features position in the pRevision string
*/
#define CAK_ICC_CAPABILITIES_FEATURES_INDEX      1

/**
 *  @brief
 *    Icc capabilities content position in the pRevision string
*/
#define CAK_ICC_CAPABILITIES_CONTENT_INDEX       2

/**
 *  @brief
 *    Icc capabilities localization position in the pRevision string
*/
#define CAK_ICC_CAPABILITIES_LOCALIZATION_INDEX  3



/**
 *  @brief
 *    Smartcard alarms managed by the CA application.
 *  @deprecated
*/
typedef enum
{
  CA_ALARM_SMARTCARD_OK,
    /**<  The smart card is communicating well with the CA and no
          problem has been encountered (smart card well inserted,
          well identified and no communication error).
    */
  CA_ALARM_SMARTCARD_COM_ERROR,
    /**<  A smart card communication error occurred. It is set when
          no more acceptable communication is possible with the smart
          card because of perturbations (i.e. glitches, electrical or
          magnetic interference, heat, etc. ).
    */
  CA_ALARM_SMARTCARD_MUTE,
    /**<  A smart card is inserted in the reader but the driver cannot
          communicate with it at all. The card may be inserted upside down.
    */
  CA_ALARM_SMARTCARD_INVALID,
    /**<  The smart card that is currently in the decoder is not
          identified as a NagraVision smart card.
    */
  CA_ALARM_SMARTCARD_BLACKLISTED,
    /**<  A blacklisted smart card is like an erased smart card,
          it is irremediably lost. Such a case could possibly happen
          for serious non-respect of agreements or for irregular use
          of the smart card.
    */
  CA_ALARM_SMARTCARD_SUSPENDED,
    /**<  It is possible to suspend the smart card until certain
          conditions are reached, and then to cancel the suspension.
          The difference between blacklisted and suspended is the
          reversibility that doesn't exist for the first case and
          exists for the second one. A good example for the suspension
          is a bad payer.
    */
  CA_ALARM_SMARTCARD_NEVER_PAIRED,
    /**<  Indicates that the smart card has never been paired with any
          decoder. For security reasons the smart card must be paired.
    */
  CA_ALARM_SMARTCARD_NOT_PAIRED,
    /**<  Indicates that the smart card is not paired with the current
          decoder (smart card dedicated for another decoder).
          For security reasons the smart card must be paired. The CA
          won't accept a non-paired card.
    */
  CA_ALARM_SMARTCARD_EXPIRED,
    /**<  Indicates that the smart card is expired. Access to any event
          or services is denied upon such a condition. The smartcard
          expiration date may be updated at any time by the CAS head-end.
    */
  CA_ALARM_SMARTCARD_NOT_CERTIFIED
    /**<  Indicates that the smart card is not certified with the current
          decoder.
    */
} TSmartcardAlarm;

/**
 *  @brief
 *    This record holds the current data of a smartcard alarm.
 *  @deprecated
 *
*/
typedef struct
{
  TIccSessionId     iccSessionId;
    /**<  Smartcard session identifier concerned by this information.
    */
  TBoolean          changed;
    /**<  TRUE if the status changed from the last exportation.
          Always TRUE at the first exportation.
    */
  TSmartcardAlarm   alarm;
    /**<  Smartcard alarm for this smartcard session identifier.
    */
} TSmartcardAlarmItem;

/**
 *  @brief
 *    Format used by each item of the smartcard information table.
 *  @deprecated
*/
typedef struct
{
        TIccSessionId  iccSessionId;
  /**<  Smartcard session identifier of this smartcard.
  */
  const TChar*        pSerialNumber;
  /**<  Serial number of the smartcard defined as a null terminated string with
   *    the following format: "xx xxxx xxxx xx".
  */
  const TChar*        pVersion;
  /**<  Null terminated string containing the DNASP version of the smartcard
   *    with the following format: "DNASPxxx".
  */
  const TChar*        pRevision;
  /**<  Null terminated string containing the DNASP version of the smartcard
   *    with the following format: "Revxxx".
  */
        TPpid          provider;
  /**<  Smartcard program provider.
  */
        TCaSystemId    ecmCaSystemId;
  /**<  ECM CA System Id.
  */
        TCaSystemId    emmCaSystemId;
  /**<  EMM CA System Id.
  */
} TSmartcardInfoItem;


/**
 *  @brief
 *    Credit list request status.
 *  @deprecated
*/
typedef enum
{
  CA_CREDIT_SUCCESS,
  /**<  The requested credit is returned successfully.
  */
  CA_CREDIT_NO_VALID_SMARTCARD,
  /**<  No NagraVision smartcard is plugged in, or the CA is temporarily unable
   *    to communicate with any of them. Returned data is not valid.
  */
  CA_CREDIT_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_CREDIT_ERROR
  /**<  The processing of the credit request has failed.
  */
} TCreditStatus;



/**
 *  @brief
 *    Format used by each item of the credit list.
 *  @deprecated
*/
typedef struct
{
  TIccSessionId  iccSessionId;
  /**<  Smartcard session identifier of the smartcard containing this credit.
  */
  TPpid          provider;
  /**<  Operator or provider identifier.
  */
  TCreditAmount  credit;
  /**<  Value of the credit for the concerned operator.
  */
  TCreditFlags   flags;
  /**<  Flags returning some information about each credit record:
    *   - CA_CREDIT_SUSPENDED: Credit record is suspended
    *   - CA_CREDIT_EXPIRED: Credit record is expired
  */
} TCreditItem;


/**
 *  @brief
 *    Event informtaion request status.
 *
 *    The status defines the validity of the returned result.
 *  @deprecated
 *
*/
typedef enum
{
  CA_EVENT_REQUEST_OK,
  /**<  The request was processed correctly and the answer is valid.
  */
  CA_EVENT_REQUEST_INCOMPATIBLE_DESCRIPTORS,
  /**<  The provided descriptors are not compatible with the smartcard.
  */
  CA_EVENT_REQUEST_BAD_DESCRIPTOR,
  /**<  The provided descriptors are corrupted.
  */
  CA_EVENT_REQUEST_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_EVENT_REQUEST_ERROR
  /**<  Other error.
  */
} TEventInfoStatus;


/**
 *  @brief
 *    Event access type, indicating if the smartcard grants access to a defined
 *    event.
 *  @deprecated
 *
*/
typedef enum
{
  CA_EVENT_ACCESS_GRANTED,
  /**<  Smartcard grants access to the given event.
  */
  CA_EVENT_ACCESS_DENIED,
  /**<  Smartcard denies access to the given event.
  */
  CA_EVENT_ACCESS_DENIED_FOR_PARENTAL_RATING,
  /**<  NOT AVAILABLE YET.
   *    The access to the current event or service is not authorized, because
   *    parental rating settings prevent it.
  */
  CA_EVENT_ACCESS_NO_VALID_SMARTCARD,
  /**<  The smartcard is not plugged in, or the CA is temporarily unable to
   *    communicate with it. Therefore, access is not granted.
  */
  CA_EVENT_ACCESS_SMARTCARD_BLACKLISTED,
  /**<  The smartcard is blacklisted. Access is not granted.
  */
  CA_EVENT_ACCESS_SMARTCARD_SUSPENDED,
  /**<  The smartcard is suspended. Access is not granted.
  */
  CA_EVENT_ACCESS_BLACKED_OUT,
  /**<  The related event is blacked out in the user's area. Access is not
   *    granted.
  */
  CA_EVENT_ACCESS_RIGHT_SUSPENDED,
  /**<  The subscription to the related event is suspended. Access is not
   *    granted.
  */
  CA_EVENT_ACCESS_CLEAR,
  /**<  The related event is broadcast in clear. The CA is not required.
  */
  CA_EVENT_ACCESS_DENIED_BUT_PPT,
  /**<  The access to the event is not authorized, because it is a
   *    pay per time for which no time slice is currently activated.
  */
  CA_EVENT_ACCESS_DENIED_NO_VALID_CREDIT
  /**<  The access to the event is not authorized, because there
   *    is not enough credit remaining.
  */
} TEventAccess;


/**
 *  @brief
 *    Event type.
 *  @deprecated
 *
*/
typedef enum
{
  CA_EVENT_TYPE_SCRAMBLED_PPV,
    /**<  Pay-per-view event.
    */
  CA_EVENT_TYPE_SCRAMBLED_IPPV,
    /**<  Impulse pay-per-view event.
    */
  CA_EVENT_TYPE_SCRAMBLED_IPPV_WITH_FLOATING_PREVIEW,
    /**<  NOT AVAILABLE YET.
     *    Impulse pay-per-view with preview event.
    */
  CA_EVENT_TYPE_SCRAMBLED_SUB,
    /**<  Subscription event.
    */
  CA_EVENT_TYPE_SCRAMBLED_VOD,
    /**<  VOD event.
    */
  CA_EVENT_TYPE_SCRAMBLED_FREE,
    /**<  Free event.
    */
  CA_EVENT_TYPE_CLEAR
    /**<  Not scrambled event.
    */
} TEventType;



/**
 *  @brief
 *    Event purchase status.
 *  @deprecated
*/
typedef enum
{
  CA_EVENT_PURCHASE_SUCCESS,
  /**<  The purchase has been performed successfully.
  */
  CA_EVENT_PURCHASE_NO_VALID_SMARTCARD,
  /**<  The smartcard is not inserted, or the CA is temporarily unable to
   *    communicate with it. Therefore, the purchase has failed.
  */
  CA_EVENT_PURCHASE_MEMORY_FULL,
  /**<  The smartcard is out of memory. Therefore, the right related to the
   *    impulse pay-per-view could not be created.
  */
  CA_EVENT_PURCHASE_LOW_CREDIT,
  /**<  The smartcard does not contain enough credit to buy the concerned
   *    impulse pay-per-view.
  */
  CA_EVENT_PURCHASE_NO_CREDIT,
  /**<  The smartcard does not contain any credit at all to buy the concerned
   *    impulse pay-per-view.
  */
  CA_EVENT_PURCHASE_CREDIT_SUSPENDED,
  /**<  The smartcard credit is suspended, which means that the concerned
   *    impulse pay-per-view cannot be bought.
  */
  CA_EVENT_PURCHASE_CREDIT_EXPIRED,
  /**<  The smartcard credit is expired, which means that the concerned
   *    impulse pay-per-view cannot be bought.
  */
  CA_EVENT_PURCHASE_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_EVENT_PURCHASE_ERROR
  /**<  An error has occurred during the processing of the request.
  */
} TEventPurchaseStatus;



/**
 *  @brief
 *    Random key status.
 *  @deprecated
*/
typedef enum
{
  CA_RANDOM_KEY_SUCCESS,
  /**<  The request has been processed successfully. The returned data is
   *    valid.
  */
  CA_RANDOM_KEY_NO_VALID_SMARTCARD,
  /**<  The smartcard is not inserted, or the CA is temporarily unable to
   *    communicate with it. Returned data is not valid.
  */
  CA_RANDOM_KEY_ERROR
  /**<  An error has occurred during the processing of the request.
  */
} TRandomKeyStatus;

/**
 *  @brief
 *    This enumeration represents the key types.
 *  @deprecated
*/
typedef enum {
  CA_RANDOM_KEY_GENERIC8,
  /**< 8-byte key.
  */
  CA_RANDOM_KEY_GENERIC16,
  /**< 16-byte key.
  */
  CA_RANDOM_KEY_DES,
  /**< 8-byte key with 8 parity bits.
  */
  CA_RANDOM_KEY_3DES,
  /**< 16-byte key with 16 parity bits.
  */
  CA_RANDOM_KEY_DVB_CSA,
  /**< 8-byte key with entropy reduction.
  */
  CA_RANDOM_KEY_NUMBER_OF_KEY_TYPES
  /**< Number of key types, should never be passed to any function.
  */
} TRandomKeyType;


/**
 *  @brief
 *    Purchase list request status.
 *  @deprecated
*/
typedef enum
{
  CA_EVENT_PURCHASE_LIST_SUCCESS,
  /**<  The request has been processed successfully. The returned data is
   *    valid.
  */
  CA_EVENT_PURCHASE_LIST_NO_VALID_SMARTCARD,
  /**<  No NagraVision smartcard is plugged in, or the CA is temporarily unable
   *    to communicate with any of them. Returned data is not valid.
  */
  CA_EVENT_PURCHASE_LIST_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_EVENT_PURCHASE_LIST_ERROR
  /**<  The processing of the request has failed. Data is not valid.
  */
} TEventPurchaseListStatus;


/**
 *  @brief
 *    Flags stored in the smartcard with each pay per view record.
 *  @deprecated
 *
 *  @see CA_EVENT_PURCHASE_LIST_SUSPENDED
 *  @see CA_EVENT_PURCHASE_LIST_IMPULSIVE
 *  @see CA_EVENT_PURCHASE_LIST_WATCHED
 *  @see CA_EVENT_PURCHASE_LIST_SINGLE_SLICE
 *  @see CA_EVENT_PURCHASE_LIST_SLICE
*/
typedef TUnsignedInt8  TEventPurchaseListFlags;

#define CA_EVENT_PURCHASE_LIST_SUSPENDED     BIT0
  /**<  Event is suspended.
   *    @see TEventPurchaseListFlags
  */
#define CA_EVENT_PURCHASE_LIST_IMPULSIVE     BIT1
  /**<  Event has been bought Impulsively.
   *    @see TEventPurchaseListFlags
  */
#define CA_EVENT_PURCHASE_LIST_WATCHED       BIT2
  /**<  Event has been watched.
   *    @see TEventPurchaseListFlags
  */
#define CA_EVENT_PURCHASE_LIST_SINGLE_SLICE  BIT3
  /**<  No auto slice renewal.
   *    @see TEventPurchaseListFlags
  */
#define CA_EVENT_PURCHASE_LIST_SLICE         BIT4
  /**<  Event charged by time slice.
   *    @see TEventPurchaseListFlags
  */


/**
 *  @brief
 *    Format used by each item of the pay-per-view event list.
 *  @deprecated
*/
typedef struct
{
  TIccSessionId           iccSessionId;
  /**<  Smartcard session identifier of the smartcard containing this event.
  */
  TPpid                   provider;
  /**<  Operator or provider identifier.
  */
  TSize                   eventNameLength;
  /**<  Event name length in bytes, not including the null terminator.
  */
  const TUnsignedInt8*    pEventName;
  /**<  Pointer to a null terminated string containing the event name.
  */
  TSize                   serviceNameLength;
  /**<  Service name length in bytes, not including the null terminator.
  */
  const TUnsignedInt8*    pServiceName;
  /**<  Pointer to a null terminated string containing the service name.
  */
  const TCalendarTime*    pStartTime;
  /**<  The UTC start time and date of the concerned event.
  */
  TEventPurchaseListFlags flags;
  /**<  Flags returning some information about the current event.
  */
  TCreditAmount           cost;
  /**<  Cost of the current event.
  */
} TEventPurchaseListItem;


/**
 *  @brief
 *    Location information request status.
 *  @deprecated
*/
typedef enum
{
  CA_LOCATION_INFO_SUCCESS,
  /**<  The requested location information is provided successfully.
  */
  CA_LOCATION_INFO_NO_VALID_SMARTCARD,
  /**<  The CA does not communicate with any smartcard.
  */
  CA_LOCATION_INFO_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_LOCATION_INFO_ERROR
  /**<  The processing of the request has failed. Data is not valid.
  */
} TLocationInfoStatus;


/**
 *  @brief
 *    Former Zip code format.
 *  @deprecated
 *
 *  @see CA_ZIP_CODE_LEN
*/
typedef TUnsignedInt8  TZipCode[CA_ZIP_CODE_LEN];


/**
 *  @brief
 *    Format used by each item of the location information table.
 *  @deprecated
*/
typedef struct {
  TIccSessionId  iccSessionId;
  /**<  Smartcard session identifier of this smartcard.
  */
  TZipCode       zipCode;
  /**<  Zip code.
  */
  TDeltaTime     deltaTime;
  /**<  Delta time in seconds.
  */
} TLocationInfoItem;



/**
 *  @brief
 *    Format used the decoder information table.
 *  @deprecated
*/
typedef struct
{
  const TChar*  pProjectName;
  /**<  Name of the NagraVision project of this decoder. NULL terminated
   *    string.
  */
  const TChar*  pSerialNumber;
  /**<  Serial number of the decoder defined as a NULL terminated string with
   *    the following format: "xx xxxx xxxx xx".
  */
  const TChar*  pVersion;
  /**<  NULL terminated string containing the version of the CA software with
   *    the following format: "x.y.z".
  */
  const TChar*  pRevision;
  /**<  NULL terminated string containing the revision of the CA software with
   *    the following format: "x.y.z".
  */
} TIrdInfo;



/**
 *  @brief
 *    System information request status.
 *  @deprecated
*/
typedef enum
{
  CA_SYSTEM_INFO_SUCCESS,
  /**<  The request has been processed successfully. Returned data is valid.
  */
  CA_SYSTEM_INFO_CA_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_SYSTEM_INFO_ERROR
  /**<  The processing of the request has failed. Data is not valid.
  */
} TSystemInfoStatus;


/**
 *  @brief
 *    This enumeration type defines the different acces alarms managed
 *    by the CA application.
 *  @deprecated
*/
typedef enum
{
  CA_ALARM_ACCESS_FREE,
    /**<  The access to the current event or service is authorized.
          No special rights are required for this event or service.
    */
  CA_ALARM_ACCESS_GRANTED,
    /**<  The access to the current event or service is authorized.
          The rights are granted for this event or service.
    */
  CA_ALARM_ACCESS_GRANTED_PPT,
    /**<  The access to the current event or service is authorized.
          The rights are PPT granted for this event or service.
    */
  CA_ALARM_ACCESS_GRANTED_FOR_PREVIEW,
    /**<  NOT AVAILABLE YET.
          The access to the current event is authorized for the preview time.
          The right is valid only for this preview.
    */
  CA_ALARM_ACCESS_GRANTED_BUT_WARNING_PREVIEW,
    /**<  NOT AVAILABLE YET.
          The access to the current event is authorized for the preview time.
          The preview has entered its warning state and will end up shortly.
    */
  CA_ALARM_ACCESS_DENIED,
    /**<  The access to the current event or service is not authorized.
          The rights are not granted for this program or channel.
    */
  CA_ALARM_ACCESS_DENIED_BUT_PREVIEW,
    /**<  NOT AVAILABLE YET.
          The access to the current event is not authorized but there is a
          preview possible.
    */
  CA_ALARM_ACCESS_DENIED_FOR_PARENTAL_RATING,
    /**<  NOT AVAILABLE YET.
          The access to the current event or service is not authorized,
          because parental rating settings prevent it.
    */
  CA_ALARM_ACCESS_BLACKOUT,
    /**<  The access to the current event or service is not authorized,
          because the owner of the smart card (decoder) is living in a
          place that is blacked out.
    */
  CA_ALARM_ACCESS_DENIED_BUT_PPT,
    /**<  The access to the current event or service is not authorized,
          because it is a pay per time for which no time slice is
          currently activated.
    */
  CA_ALARM_ACCESS_DENIED_NO_VALID_CREDIT,
    /**<  The access to the current event or service is not authorized,
          because there is not enough credit remaining.
    */
  CA_ALARM_ACCESS_DENIED_COPY_PROTECTED
    /**<  The access to the current event or service is not authorized,
     *     because it is copy-protected.
    */
} TAccessAlarm;

/**
 *  @brief
 *    This record holds the current status of an elementary stream.
 *  @deprecated
 *
*/
typedef struct
{
  TPid                  elementaryStreamPid;
    /**<  Elementary stream PID.
    */
  TDvbServiceId         serviceId;
    /**<  DVB Service identifier.
    */
  TTransportSessionId     transportSessionId;
    /**<  Transport stream session identifier.
    */
  TBoolean              changed;
    /**<  TRUE if the alarm status changed from the last exportation.
    */
  TAccessAlarm          alarm;
    /**<  Access alarm identifier for this service and elementary stream.
    */
  TIccSessionId         iccSessionId;
    /**<  Smartcard session identifier of the smartcard in charge of this
          stream.
    */
} TAccessAlarmItem;


/**
 *  @brief
 *    This type identifies a specific registration item. It is returned by the
 *    listener functions and is used to cancel that registration.
 *  @deprecated
 *
 *  @see CA_REGISTRATION_ID_INVALID
*/
typedef TUnsignedInt32 TCaRegistrationId;

/**
 *  @brief
 *    Invalid registration id for TCaRegistrationId.
 *  @deprecated
 *
 *  @see TCaRegistrationId
*/
#define CA_REGISTRATION_ID_INVALID  ((TCaRegistrationId) -1)



/**
 *  @brief
 *    The PIN code format.
 *    The PIN code has a simple format carrying a 32-bit value that is not
 *    dependent of the CAK.
*/
typedef TUnsignedInt32 TPinCode;

/**
 *  @brief
 *    PIN code status.
*/
typedef enum
{
  CA_PIN_CODE_PIN_ACCEPTED,
  /**<  The PIN code to check was correct or the PIN code
   *    to change has been changed successfully.
  */
  CA_PIN_CODE_PIN_REJECTED,
  /**<  The PIN code to check was invalid or the PIN code
   *    to change has not been changed due an invalid old PIN.
  */
  CA_PIN_CODE_PIN_SUSPENDED,
  /**<  The PIN code is suspended.
  */
  CA_PIN_CODE_PIN_INDEX_INVALID,
  /**<  The PIN index is not valid.
  */
  CA_PIN_CODE_NO_VALID_SMARTCARD,
  /**<  No NagraVision smartcard is inserted, or the CA
   *    is temporarily unable to communicate with any of them.
  */
  CA_PIN_CODE_ERROR
  /**<  Another error occurred.
  */
} TPinCodeStatus;

/**
 *  @brief
 *    PIN number that must be checked or changed.
*/
typedef enum
{
  CA_PIN_INDEX_1 = 1,
  /**<  The PIN index 1.
  */
  CA_PIN_INDEX_2,
  /**<  The PIN index 2.
  */
  CA_PIN_INDEX_3,
  /**<  The PIN index 3.
  */
  CA_PIN_INDEX_4,
  /**<  The PIN index 4.
  */
  CA_PIN_INDEX_LAST
  /**< Last type. To be used only for consistency checks.
  */
} TPinIndex;

/**
 *  @brief
 *    Credit list callback.
 *
 *    This function is used to return the result of a credit request. It is
 *    called in the CA context as soon as the request is processed. The credit
 *    data is returned as a credit records table. The CA allocates the
 *    requisite memory. Deallocation is performed by calling the
 *    acknowledgement function caCreditAcknowledgement().
 *
 *  @deprecated
 *
 *  @param   xStatus
 *             Status returned in response of the credit request.
 *  @param   xNumberOfItems
 *             Number of items contained in the table.
 *  @param   pxCreditArray
 *             Pointer to the first item of the credit array. This pointer is
 *             set to NULL in case the smartcard does not contain any credit
 *             record.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# Once the application has finished using the provided list, it must
 *       free it by calling caCreditAcknowledgement().
*/
typedef TCaStatus (*TCreditCallback)
(
  TCreditStatus  xStatus,
  TUnsignedInt8  xNumberOfItems,
  TCreditItem*   pxCreditArray
);


/**
 *  @brief
 *    Event information callback.
 *
 *    This function is used to return the event information asked by means of
 *    the caEventInformationRequest() function. It is called in the CA context
 *    as soon as the request is processed.
 *
 *  @deprecated
 *
 *  @param   pxPrivateData
 *             Reference to optional private data, provided during the request.
 *  @param   xStatus
 *             Status of the request.
 *  @param   xProvider
 *             Provider identifier of the event. All information provided below
 *             concern this provider.
 *  @param   xAccess
 *             Information of access about the given event.
 *  @param   xType
 *             Type of the given event
 *  @param   xCost
 *             Cost of the given event. This argument is not relevant for
 *             subscription events.
 *  @param   xPreviewTime
 *             Number of seconds of preview time. This argument is not relevant
 *             for subscription events.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TEventInfoCallback)
(
  void*              pxPrivateData,
  TEventInfoStatus   xStatus,
  TPpid              xProvider,
  TEventAccess       xAccess,
  TEventType         xType,
  TCreditAmount      xCost,
  TEventPreviewTime  xPreviewTime
);

/**
 *  @brief
 *    Event purchase callback.
 *
 *    This function is used to return the result of an impulse purchase
 *    request. It is called in the CA context as soon as the request is
 *    processed.
 *
 *  @deprecated
 *
 *  @param   pxPrivateData
 *             Reference to optional private data.
 *  @param   xStatus
 *             Status returned in response to the impulse purchase request.
 *  @param   xProvider
 *             Provider identifier related to the event to be purchased.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TEventPurchaseCallback)
(
  void*                 pxPrivateData,
  TEventPurchaseStatus  xStatus,
  TPpid                 xProvider
);

/**
 *  @brief
 *    Random key generating request callback.
 *
 *    This function is used to return the result of a generate key request.
 *    It is called in the CA context as soon as the request is processed.
 *
 *  @deprecated
 *
 *  @param  pxPrivateData
 *            Pointer to private data returned back by the CA.
 *  @param  xStatus
 *            Status returned in response to the generate key request.
 *  @param  xKeySize
 *            Size of generated key.
 *  @param  pxKeyData
 *            Pointer to key buffer.
 *  @param  xSecretSize
 *            Size of secret.
 *  @param  pxSecretData
 *            Pointer to secret buffer.
 *
 *  @retval CA_NO_ERROR
 *            Success.
 *  @retval CA_ERROR
 *            Failure.
 *
 *  @warning  Once the application has finished using the provided buffers, it
 *            must call caGenerateRandomKeyAcknowledgement().
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TGenerateRandomKeyCallback)
(
          void*            pxPrivateData,
          TRandomKeyStatus  xStatus,
          TSize             xKeySize,
    const TUnsignedInt8*   pxKeyData,
          TSize             xSecretSize,
    const TUnsignedInt8*   pxSecretData
);

/**
 *  @brief
 *    Extract key request callback.
 *
 *    This function is used to return the result of an extract key request.
 *    It is called in the CA context as soon as the request is processed.
 *
 *  @deprecated
 *
 *  @param  pxPrivateData
 *            Pointer to private data returned back by the CA.
 *  @param  xStatus
 *            Status returned in response to the extract key request.
 *  @param  xKeySize
 *            Size of extracted key.
 *  @param  pxKeyData
 *            Pointer to key buffer.
 *
 *  @retval CA_NO_ERROR
 *            Success.
 *  @retval CA_ERROR
 *            Failure.
 *
 *  @warning  Once the application has finished using the provided buffers, it
 *            must call caExtractRandomKeyAcknowledgement().
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TExtractRandomKeyCallback)
(
          void*            pxPrivateData,
          TRandomKeyStatus  xStatus,
          TSize             xKeySize,
    const TUnsignedInt8*   pxKeyData
);

/**
 *  @brief
 *    Event purchase list callback.
 *
 *    This function is used to return the result of an event purchase list
 *    request. It is called in the CA context as soon as the request is
 *    processed. The CA allocates the needed memory. Deallocation is performed
 *    by calling the acknowledgement function
 *    caEventPurchaseListAcknowledgement().
 *
 *  @deprecated
 *
 *  @param   xStatus
 *             Status returned in response of the pay-per-view list request.
 *  @param   xNumberOfItems
 *             Number of items (purchased pay-per-view events) contained in the
 *             list.
 *  @param   pxEventPurchaseListArray
 *             Pointer to the first item of the pay-per-view event array. This
 *             pointer is set to NULL in case of an empty array.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TEventPurchaseListCallback)
(
  TEventPurchaseListStatus xStatus,
  TUnsignedInt8            xNumberOfItems,
  TEventPurchaseListItem*  pxEventPurchaseListArray
);

/**
 *  @brief
 *    Localtion information request callback.
 *
 *    This function is used to return the result of a location information
 *    request. It is called in the CA context as soon as the request is
 *    processed.
 *
 *  @deprecated
 *
 *  @param   xStatus
 *             Status returned in response of the location information request.
 *  @param   xNumberOfSmartcards
 *             Number of currently active smartcards.
 *  @param   pxLocationInfoArray
 *             Pointer to the first item of the location information array.
 *             This pointer is set to NULL if there is no smartcard.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# Once the application has finished using the provided list, it must
 *       free it by calling caLocationInformationAcknowledgement().
*/
typedef TCaStatus (*TLocationInfoCallback)
(
  TLocationInfoStatus  xStatus,
  TUnsignedInt8        xNumberOfSmartcards,
  TLocationInfoItem*   pxLocationInfoArray
);

/**
 *  @brief
 *    System information request callback.
 *
 *  @deprecated
 *
 *  @param   xStatus
 *             Status returned in response to a system information request.
 *  @param   pxIrdInfo
 *             Reference to the record containing the decoder information.
 *  @param   xNumberOfSmartcards
 *             Number of currently active smartcards.
 *  @param   pxSmartcardInfoArray
 *             Reference to an array of records containing the smartcards
 *             information.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# Once the application has finished using the provided data, it must
 *       free it by calling caSystemInformationAcknowledgement().
*/
typedef TCaStatus (*TSystemInfoCallback)
(
  TSystemInfoStatus    xStatus,
  TIrdInfo*            pxIrdInfo,
  TUnsignedInt8        xNumberOfSmartcards,
  TSmartcardInfoItem*  pxSmartcardInfoArray
);



/**
 *  @brief
 *    PIN Code Callback
 *
 *    This function is used by the CAK to return the PIN code status
 *    after checking or changing request.
 *
 *  @param   pxPrivateData
 *             Private data. It is passed back by the CA. Content of private
 *             data is independent of the CA.
 *  @param   xPinIndex
 *             The PIN index.
 *  @param   xStatus
 *             PIN Code status.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
*/
typedef TCaStatus (*TPinCodeStatusCallback)
(
  void*                  pxPrivateData,
  TPinIndex               xPinIndex,
  TPinCodeStatus          xStatus
);



/**
 *  @brief
 *    Access notification callback.
 *
 *    This function is called whenever event access rights are likely to be
 *    out-of-date due to smartcard rights modifications. This function does not
 *    return an identifier to a specific event. Any event may be concerned by
 *    this notification.
 *
 *  @deprecated
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# As a consequence of note 1, the event information request must never
 *       be done directly inside this notification callback function.
*/
typedef TCaStatus (*TAccessNotificationCallback)
(
  void
);

/**
 *  @brief
 *    Event purchase list notification callback.
 *
 *    This function is called by the CA whenever an event purchase list
 *    notification occurs. This function does not return any data. Retrieval of
 *    the related data has to go through the caEventPurchaseListRequest()
 *    function.
 *
 *  @deprecated
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# As a consequence of note 1, the event purchase list request must never
 *       be done directly inside this notification callback function.
*/
typedef TCaStatus (*TEventPurchaseListNotification)
(
  void
);

/**
 *  @brief
 *    Credit list notification callback.
 *
 *    This function is called by the CA whenever a credit notification occurs.
 *    This function does not return any data. Retrieval of the related data
 *    must go through the caCreditRequest() function.
 *
 *  @deprecated
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# As a consequence of note 1, the credit request must never be done
 *       directly inside this notification callback function.
*/
typedef TCaStatus (*TCreditNotification)
(
  void
);


/**
 *  @brief
 *    Location information notification callback.
 *
 *    This function is called whenever a location information notification
 *    occurs. This function does not return any data. Retrieval of the related
 *    data must go through the caLocationInformationRequest() function.
 *
 *  @deprecated
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# Because of note 1, the location information request must never be done
 *       directly inside this notification callback function.
*/
typedef TCaStatus (*TLocationInfoNotification)
(
  void
);

/**
 *  @brief
 *    System information notification callback.
 *
 *    This function is called whenever a system information notification
 *    occurs. It does not return any data.
 *
 *    Retrieval of the related data is made through the
 *    caSystemInformationRequest() function.
 *
 *  @deprecated
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# Because of note 1, the system information request must never be done
 *       directly inside this notification callback function.
*/
typedef TCaStatus (*TSystemInfoNotification)
(
  void
);

/**
 *  @brief
 *    This function is used to export the CA alarm to the manufacturer.
 *
 *    It exports the status of every managed smartcards and every descrambling
 *    stream.
 *
 *  @deprecated
 *
 *  @param  xNumberOfSmartcardAlarms
 *            Number of smartcard status records in the following table.
 *  @param  pxSmartcardAlarmArray
 *            Table of smartcard alarm records. Pointer may be NULL.
 *  @param  xNumberOfAccesAlarms
 *            Number of stream access alarm records in the following table.
 *  @param  pxAccessAlarmArray
 *            Table of stream access alarm records. Pointer may be NULL.
 *
 *  @retval CA_NO_ERROR
 *            Success
 *  @retval CA_ERROR
 *            Failure
 *
 *  @remarks
 *    This function is called in the CA context. Therefore, its execution
 *    should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TAlarmExportation)
(
        TUnsignedInt8           xNumberOfSmartcardAlarms,
  const TSmartcardAlarmItem*    pxSmartcardAlarmArray,
        TUnsignedInt8           xNumberOfAccesAlarms,
  const TAccessAlarmItem*       pxAccessAlarmArray
);

/**
 *  @brief
 *    IRD command exportation callback.
 *
 *    This decoder manufacturer's callback function is used to export each IRD
 *    command (or part of command) to the decoder. The CA allocates the
 *    required memory. Deallocation is performed by calling the acknowledgement
 *    function caIrdCmdAcknowledgement().
 *
 *  @deprecated
 *
 *  @param   xIrdCmdLength
 *             Length of the command.
 *  @param   pxIrdCmd
 *             Pointer to the command.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TIrdCmdExportation)
(
  TUnsignedInt8   xIrdCmdLength,
  TUnsignedInt8*  pxIrdCmd
);
/**
 * @brief
 *   Termination notification callback.
 *
 *   This function is called when the CA begins its termination process. No
 *   more requests will be processed once this notification is sent.
 *
 *  @deprecated
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TTerminationNotification)
(
  void
);


/**
 *  @brief
 *    Product type
 *
*/
typedef enum
{
  CA_PRODUCT_TYPE_PPV,
    /**<  Pay-per-view product. This product cannot be impulsively purchased.
    */
  CA_PRODUCT_TYPE_IPPV
    /**<  Impulse pay-per-view product.
    */
} TProductType;

/**
 *  @brief
 *    Push-VOD informtaion request status.
 *
 *    The status defines the validity of the returned result.
 *
*/
typedef enum
{
  CA_PVD_REQUEST_OK,
    /**<  The request was processed correctly and the answer is valid.
    */
  CA_PVD_REQUEST_INCOMPATIBLE_DESCRIPTORS,
    /**<  The provided descriptors are not compatible with the smartcard.
    */
  CA_PVD_REQUEST_BAD_DESCRIPTOR,
    /**<  The provided descriptors are corrupted.
    */
  CA_PVD_REQUEST_CA_SHUTDOWN,
    /**<  The CA was shutdown while processing the request.
    */
  CA_PVD_REQUEST_ERROR
    /**<  Other error.
    */
} TPvdInfoStatus;

/**
 *  @brief
 *    Rental time format of a pay per view event, expressed in Seconds. It may
 *    be the total rental time of a pay-per-view event or the remaining rental
 *    time of a pay-per-view event in case it has been purchased.
 *
*/
typedef TUnsignedInt32  TRentalTime;

/**
 *  @brief
 *    Product information callback.
 *
 *    This function is used to return the product information asked by means of
 *    the caPvdProductInformationRequest() function. It is called in the CA
 *    context as soon as the request is processed.
 *
 *  @param   pxPrivateData
 *             Reference to optional private data, provided during the request.
 *  @param   xStatus
 *             Status of the request.
 *  @param   xProvider
 *             Provider identifier of the asset. All information provided below
 *             concern this provider.
 *  @param   xAccess
 *             Information of access about the given product.
 *  @param   xType
 *             Type of the given event.
 *  @param   xRemainingRentalTime
 *             Number of seconds of the remaining rental period of the product.
 *             In case the product has not been purchased or the rental time is
 *             over, the remaining rental time is in both cases zero.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TPvdProductInfoCallback)
(
  void*              pxPrivateData,
  TPvdInfoStatus      xStatus,
  TPpid               xProvider,
  TCaAccess           xAccess,
  TProductType        xType,
  TCreditAmount       xCost,
  TRentalTime         xRemainingRentalTime
);


/**
 *  @brief
 *    Asset information callback.
 *
 *    This function is used to return the asset information asked by means of
 *    the caPvdAssetInformationRequest() function. It is called in the CA
 *    context as soon as the request is processed.
 *
 *  @param   pxPrivateData
 *             Reference to optional private data, provided during the request.
 *  @param   xStatus
 *             Status of the request.
 *  @param   xProvider
 *             Provider identifier of the asset. All information provided below
 *             concern this provider.
 *  @param   xAccess
 *             Information of access about the given asset.
 *  @param   xRemainingRentalTime
 *             Number of seconds of the remaining rental period of the asset.
 *             In case no product allowing watching the given asset has been
 *             purchased or the rental time is over, the remaining rental time
 *             is zero.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TPvdAssetInfoCallback)
(
  void*            pxPrivateData,
  TPvdInfoStatus    xStatus,
  TPpid             xProvider,
  TCaAccess         xAccess,
  TRentalTime       xRemainingRentalTime
);

/**
 *  @brief
 *    The network ID format.
*/
typedef TUnsignedInt16 TNetworkIdListItem;

/**
 *  @brief
 *    Network ID status.
*/
typedef enum
{
  CA_NETWORK_ID_LIST_SUCCESS,
  /**<  The request has been processed successfully. The returned data is
   *    valid.
  */
  CA_NETWORK_ID_LIST_NO_VALID_SMARTCARD,
  /**<  The smartcard is not inserted, or the CA is temporarily unable to
   *    communicate with it. Returned data is not valid.
  */
  CA_NETWORK_ID_LIST_SHUTDOWN,
  /**<  The CA was shutdown while processing the request.
  */
  CA_NETWORK_ID_LIST_ERROR
  /**<  An error has occurred during the processing of the request.
  */
} TNetworkIdListStatus;


/**
 *  @brief
 *    Network ID list callback.
 *
 *    This function is used to return the result of a network ID request.
 *    It is called in the CA context as soon as the request is processed.
 *
 *  @param   xStatus
 *             Status returned in response to the network ID request.
 *  @param   xNumberOfItems
 *             Number of items (network ID) contained in the list
 *  @param   pxNetworkIdListArray
 *             Pointer to the first item of the network ID array. This
 *             pointer is set to NULL in case of an empty array.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
*/
typedef TCaStatus (*TNetworkIdListCallback)
(
  TNetworkIdListStatus   xStatus,
  TUnsignedInt8          xNumberOfItems,
  TNetworkIdListItem*   pxNetworkIdListArray
);



/**
 *  @brief
 *    Call Collection status.
*/
typedef enum
{
  CA_CALL_COLLECTION_ESTABLISHING_CONNECTION,
  /**<  The CA starts establishing the connection.
  */
  CA_CALL_COLLECTION_CONNECTED,
  /**<  The connection was established successfully. The CAK can start
  *     exchanging data.
  */
  CA_CALL_COLLECTION_COMPLETED,
  /**<  The call collection completed successfully. The process is considered
   *    as over and no other status will be sent to the application.
  */
  CA_CALL_COLLECTION_LOCAL_FAILURE,
  /**<  The connection failed due to a local error. The phone line may not
   *    be connected to the set-top box or may be currently used by the
   *    subscriber.
   *    Upon such an error, the retry mechanism is enabled preventing other
   *    calls to be done for a short period (typically 10min). Current call
   *    collection will be resumed automatically at the end of the retry
   *    period.
  */
  CA_CALL_COLLECTION_REMOTE_FAILURE,
  /**<  The connection failed due to a remote error. The server may be down
   *    or is not answering (busy).
   *    Upon such an error, the retry mechanism is enabled preventing other
   *    calls to be done for a rather long period (typically 1h30). Current
   *    call collection will be resumed automatically at the end of the retry
   *    period.
  */
  CA_CALL_COLLECTION_AUTHENTICATION_FAILED,
  /**<  The CA could not be successfully authenticated by the remote server,
   *    probably because the smart card does not have the valid data; it is
   *    advised that the end-user calls the operator to report the problem.
  */
  CA_CALL_COLLECTION_NO_VALID_SMARTCARD,
  /**<  No NagraVision smartcard is inserted, or the CA is temporarily unable
   *    to communicate with any of them.
  */
  CA_CALL_COLLECTION_NO_CALL_INFORMATION,
  /**<  The smart card does not have valid call information data; it is advised
   *    that the end-user calls the operator to report the problem.
  */
  CA_CALL_COLLECTION_ERROR
  /**<  Another error occurred. Another attempt is performed at once.
  */
} TCallCollectionStatus;


/**
 *  @brief
 *    Call Collection Status Callback
 *    This function is used by the CAK to notify the application of the call
 *    collection progress. It is called each time the status of the call
 *    initiated through caCallCollectionRequest changes.
 *
 *  @param   xStatus
 *             Call collection status.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# This function is called in the CA context. Therefore, its execution
 *       should be as short as possible. It must not block the calling task.
 *    -# CA_CALL_COLLECTION_LOCAL_FAILURE and CA_CALL_COLLECTION_REMOTE_FAILURE
 *       notifications are generated in case the connection could not be
 *       established properly due to a local error (phone line not connected or
 *       already used by the user) or a remote error (server down or busy).
 *    -# CA_CALL_COLLECTION_AUTHENTICATION_FAILED notification is generated if
 *       the CA could not be successfully authenticated by the remote server,
 *       probably because the smart card does not have the valid authentication
 *       data; it is advised that the end-user calls the operator to report the
 *       problem.
 *    -# CA_CALL_COLLECTION_NO_VALID_SMARTCARD notification is generated if there
 *       is no NagraVision smartcard is inserted, or the CA is temporarily unable
 *       to communicate with any of them.
 *    -# CA_CALL_COLLECTION_NO_CALL_INFORMATION notification is generated if the
 *       smart card does not have valid call information data; it is advised that
 *       the end-user calls the operator to report the problem.
 *    -# CA_CALL_COLLECTION_ERROR notification is generated in case of other
 *       communication error.
*/
typedef TCaStatus (*TCallCollectionStatusCallback)
(
  TCallCollectionStatus  xStatus
);


/**
 *  @brief
 *    Credit list request (Obsolete).
 *
 *    This function sends a request to the CA in order to get the credit
 *    remaining in the smartcard. The processing of the request runs
 *    asynchronously with regard to the calling task. The CA returns the
 *    resulting data by means of the provided callback function.
 *
 *  @deprecated
 *
 *  @param   xCreditCallback
 *             Callback function used to return the credit information to the
 *             decoder.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully. It does not mean the
 *             processing of the request is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/

NAGRA_CA_API TCaStatus caCreditRequest
(
  TCreditCallback  xCreditCallback
);

/**
 *  @brief
 *    Credit list acknowledgement (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced credit list may be flushed.
 *
 *  @deprecated
 *
 *  @param   pxCreditArray
 *             Pointer to the first item of the credit list to be flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caCreditAcknowledgement
(
  TCreditItem*  pxCreditArray
);


/**
 *  @brief
 *    Event information request (Obsolete).
 *
 *    This function sends a request to the CA in order to get the information
 *    of an event. To do so, the CA needs the DVB descriptors of the concerned
 *    event. They are formatted as found in an MPEG table [1]; see 3.2.1 for a
 *    brief description. In a DVB system, all the descriptors for this request
 *    are found in the SI tables (NIT, SDT and EIT). The start time is
 *    extracted from the EIT (p/f or schedule) of the corresponding event.
 *<br>
 *    It is also possible to obtain the default access status of the events on
 *    a given service by passing only the service descriptors
 *    (pxServiceDescriptorBlock) and the current time (pxStartTime). Notice
 *    that the default access status is overridden by event access status, if
 *    any.
 *<br>
 *    The processing of the request is performed asynchronously with regard to
 *    the calling task. The CA returns the resulting data by means of the
 *    provided callback function. The CA software can manage a burst of
 *    requests. Therefore, it is not necessary to wait for a response before
 *    sending another request.
 *
 *  @deprecated
 *
 *  @param   xEventInfoCB
 *             Callback function called by the CA to return the resulting event
 *             information. It is called only once per request.
 *  @param   pxPrivateData
 *             Reference to optional private data. It is passed to the callback
 *             function. Content of private data is independent of the CA.
 *             There is no safe copy of the content.
 *  @param   pxStartTime
 *             The UTC start date and time of the concerned event.
 *  @param   xSizeOfEventSpecificDescriptors
 *             Size in bytes of the event specific descriptor block passed
 *             below. In a DVB system, it corresponds to the EIT descriptors.
 *  @param   pxEventSpecificDescriptorBlock
 *             Pointer to the event specific descriptors. All the event
 *             specific descriptors must be given. In a DVB system, it
 *             corresponds to the EIT descriptors. Depending of the event, the
 *             descriptors may be found in the EIT present, following or
 *             schedule. Use NULL when requesting default event access for a
 *             service.
 *  @param   xSizeOfServiceDescriptors
 *             Size in bytes of the service descriptor block passed below. In a
 *             DVB system, it corresponds to the SDT descriptors.
 *  @param   pxServiceDescriptorBlock
 *             Pointer to the service descriptors. All the service descriptors
 *             must be given. In a DVB system, it corresponds to the SDT
 *             descriptors.
 *  @param   xSizeOfSystemWideDescriptors
 *             Size in bytes of the system wide descriptor block passed below.
 *             In a DVB system, it corresponds to the NIT descriptors.
 *  @param   pxSystemWideDescriptorBlock
 *             Pointer to the system wide descriptors. All the system wide
 *             descriptors must be given. In a DVB system, it corresponds to
 *             the NIT descriptors.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# The calling task shall not overwrite or de-allocate all the referenced
 *       data before the callback function is called (or until the function
 *       returns if an error occurred.
 *    -# All the descriptors found in the given table must be provided. The
 *       application may not filter any of them.
 *    -# If there is no descriptor at any level, its corresponding size and
 *       pointer may be set to NULL. At least one descriptor loop should not be
 *       NULL and provide some data. The descriptor loop length and the
 *       descriptor loop pointer should be consistent.
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caEventInformationRequest
(
  TEventInfoCallback  xEventInfoCB,
  void*               pxPrivateData,
  TCalendarTime*      pxStartTime,
  TSize               xSizeOfEventSpecificDescriptors,
  TUnsignedInt8*      pxEventSpecificDescriptorBlock,
  TSize               xSizeOfServiceDescriptors,
  TUnsignedInt8*      pxServiceDescriptorBlock,
  TSize               xSizeOfSystemWideDescriptors,
  TUnsignedInt8*      pxSystemWideDescriptorBlock
);


/**
 *  @brief
 *    Event purchase request (Obsolete).
 *
 *    This function allows the decoder manufacturer to send a request to the CA
 *    in order to purchase a pay-per-view event impulsively. To do so, the CA
 *    needs the DVB descriptors of the concerned event. They are formatted as
 *    found in an MPEG table [1]; see 3.2.1 for a brief description. In a DVB
 *    system, all the descriptors for this request are found in the SDT and
 *    EIT. The start time is extracted from the EIT (p/f or schedule) of the
 *    corresponding event.
 *
 *    It is advised to make a call to caEventInformationRequest() to ensure
 *    that the selected event is purchasable.
 *
 *    The processing of the request is performed asynchronously with regard to
 *    the calling task. The CA returns the resulting status by means of the
 *    provided callback function. The CA software can manage a burst of
 *    requests. Therefore, it is not necessary to wait for a response before
 *    sending another request.
 *
 *  @deprecated
 *
 *  @param   xEventPurchaseCallback
 *             Callback function called by the CA to return the resulting event
 *             impulse purchase status to the decoder.
 *  @param   pxPrivateData
 *             Reference to optional private data. It is passed to the callback
 *             function. Content of private data is independent of the CA.
 *             There is no safe copy of the content.
 *  @param   pxEventName
 *             Pointer to a null terminated string containing the event name
 *             coming from the DVB Short event descriptor. Only used if the
 *             smartcard is Dnasp2; shall be set to NULL when not used.
 *  @param   pxServiceName
 *             Pointer to a null terminated string containing the service name
 *             coming from the DVB Service descriptor. Only used if the
 *             smartcard is Dnasp2; shall be set to NULL when not used.
 *  @param   pxStartTime
 *             The UTC start time and date of the concerned event extracted
 *             from the EIT. Only used if the smartcard is Dnasp2; shall be
 *             set to NULL when not used.
 *  @param   xSizeOfEventSpecificDescriptors
 *             Size in bytes of the event specific descriptor block passed
 *             below. In a DVB system, it corresponds to the EIT descriptors.
 *  @param   pxEventSpecificDescriptorBlock
 *             Pointer to the event specific descriptors. All the event
 *             specific descriptors must be given. In a DVB system, it
 *             corresponds to the EIT descriptors. Depending of the event, the
 *             descriptors may be found in the EIT present, following or
 *             schedule.
 *  @param   xSizeOfServiceDescriptors
 *             Size in bytes of the service descriptor block passed below. In a
 *             DVB system, it corresponds to the SDT descriptors.
 *  @param   pxServiceDescriptorBlock
 *             Pointer to the service descriptors. All the service descriptors
 *             must be given. In a DVB system, it corresponds to the SDT
 *             descriptors.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully. It does not mean that
 *             the purchase is successful. The result of the processing of the
 *             request is returned asynchronously by means of the provided
 *             callback.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# The CA performs a safe copy of all the data passed by reference
 *       (except private data).  It means that the calling task may overwrite
 *       or deallocate all the referenced data as soon as it returns from the
 *       function caEventPurchaseRequest().
 *    -# All the descriptors found is the given table must be provided. The
 *       application may not filter any of them.
 *    -# If there is no descriptor at any level, its corresponding size and
 *       pointer may be set to NULL. At least one descriptor loop should not be
 *       NULL and provide some data. The descriptor loop length and the
 *       descriptor loop pointer should be consistent.
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caEventPurchaseRequest
(
  TEventPurchaseCallback  xEventPurchaseCallback,
  void*                   pxPrivateData,
  const TChar*            pxEventName,
  const TChar*            pxServiceName,
  const TCalendarTime*    pxStartTime,
  TSize                   xSizeOfEventSpecificDescriptors,
  TUnsignedInt8*          pxEventSpecificDescriptorBlock,
  TSize                   xSizeOfServiceDescriptors,
  TUnsignedInt8*          pxServiceDescriptorBlock
);



/**
 *  @brief
 *    Random key generating request (Obsolete).
 *
 *    This function sends a request to the CA in order to generate a random key.
 *    Data resulting from this request is returned asynchronously to the
 *    caller by means of the provided callback.
 *
 *  @deprecated
 *
 *  @param   xGenerateRandomKeyCallback
 *             Callback function called by the CAK to notify the application
 *             of the random key generating validity.
 *  @param   xKeyType
 *             Type of the key to generate.
 *  @param   pxPrivateData
 *             Optional private data. It is passed to the callback function.
 *             Content of private data is independent of the CA.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caGenerateRandomKeyRequest
(
  TGenerateRandomKeyCallback  xGenerateRandomKeyCallback,
  TRandomKeyType              xKeyType,
  void*                      pxPrivateData
);


/**
 *  @brief
 *    Random key generating acknowledgement (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced key and secret may be flushed.
 *
 *  @deprecated
 *
 *  @param   pxKeyData
 *             Pointer to the key to be flushed.
 *  @param   pxSecretData
 *             Pointer to the secret to be flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caGenerateRandomKeyAcknowledgement
(
  TUnsignedInt8* pxKeyData,
  TUnsignedInt8* pxSecretData
);


/**
 *  @brief
 *    Extract key request (Obsolete).
 *
 *    This function sends a request to the CA in order to extract a key from a
 *    given secret.
 *    Data resulting from this request is returned asynchronously to the
 *    caller by means of the provided callback.
 *
 *  @deprecated
 *
 *  @param   xExtractRandomKeyCallback
 *             Callback function called by the CAK to notify the application
 *             of the extract key validity.
 *  @param   xSecretSize
 *             Size of the secret.
 *  @param   pxSecretData
 *             Pointer to secret data.
 *  @param   pxPrivateData
 *             Optional private data. It is passed to the callback function.
 *             Content of private data is independent of the CA.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caExtractRandomKeyRequest
(
  TExtractRandomKeyCallback  xExtractRandomKeyCallback,
  TSize                      xSecretSize,
  TUnsignedInt8*            pxSecretData,
  void*                     pxPrivateData
);


/**
 *  @brief
 *    Extract key acknowledgement (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced key may be flushed.
 *
 *  @deprecated
 *
 *  @param   pxKeyData
 *             Pointer to the key to be flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caExtractRandomKeyAcknowledgement
(
  TUnsignedInt8* pxKeyData
);


/**
 *  @brief
 *    Event purchase list request (Obsolete).
 *
 *    This function sends a request to the CA in order to get the event purchase
 *    list. Data resulting from this request is returned asynchronously to the
 *    caller by means of the provided callback.
 *
 *  @deprecated
 *
 *  @param   xEventPurchaseListCallback
 *             Callback function called by the CA to return the event purchase
 *            list to the decoder.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caEventPurchaseListRequest
(
  TEventPurchaseListCallback  xEventPurchaseListCallback
);


/**
 *  @brief
 *    Event purchase list acknowledgment (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced event purchase list may be flushed.
 *
 *  @deprecated
 *
 *  @param   pxEventPurchaseListArray
 *             Pointer to the first item of the pay-per-view event array to be
 *             flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caEventPurchaseListAcknowledgement
(
  TEventPurchaseListItem*  pxEventPurchaseListArray
);


/**
 *  @brief
 *    Location Information Request (Obsolete).
 *
 *    This function sends a request to the CA in order to get the smartcard
 *    location information. The processing of the request runs asynchronously
 *    with regard to the calling task. The CA returns the resulting data by
 *    means of the provided callback function.
 *
 *  @deprecated
 *
 *  @param   xLocationInfoCallback
 *             Callback function used to return the location information to the
 *             decoder.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully. It does not mean the
 *             processing of the request is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caLocationInformationRequest
(
  TLocationInfoCallback  xLocationInfoCallback
);

/**
 *  @brief
 *    Localtion information acknowledgment (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced location information array may be
 *    flushed.
 *
 *  @deprecated
 *
 *  @param   pxLocationInfoArray
 *             Pointer to the first item of the location information array to
 *             be flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caLocationInformationAcknowledgement
(
  TLocationInfoItem*  pxLocationInfoArray
);


/**
 *  @brief
 *    IRD command acknowledgment (Obsolete).
 *
 *    This function is called by the manufacturer in order to inform the CA
 *    that the command has been processed and that the memory used by this
 *    command may be deallocated.
 *
 *  @deprecated
 *
 *  @param   pxIrdCmd
 *             Pointer to the command to be flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caIrdCmdAcknowledgement
(
  TUnsignedInt8*  pxIrdCmd
);



/**
 *  @brief
 *    System information request (Obsolete).
 *
 *    This function allows the decoder manufacturer to send a request to the CA
 *    in order to get the system information. The processing of this request
 *    runs asynchronously with regard to the calling task. The CA returns the
 *    information by means of the provided callback function.
 *
 *  @deprecated
 *
 *  @param   xSystemInfoCallback
 *             Callback function called by the CA to return the CA system
 *             information.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully. It does not mean the
 *             processing of the request is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caSystemInformationRequest
(
  TSystemInfoCallback  xSystemInfoCallback
);



/**
 *  @brief
 *    System information acknowledgment (Obsolete).
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced system information may be flushed.
 *
 *  @deprecated
 *
 *  @param   pxIrdInfo
 *             Reference to the record containing the decoder information to
 *             free.
 *  @param   pxSmartcardInfoArray
 *             Reference to an array of records containing the smartcards
 *             information to free.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caSystemInformationAcknowledgement
(
  TIrdInfo*            pxIrdInfo,
  TSmartcardInfoItem*  pxSmartcardInfoArray
);


/**
 *  @brief
 *    Check a smartcard PIN code.
 *
 *    This CA function is called by the application in order to
 *    check a smartcard PIN code.
 *
 *  @param   xPinCodeStatusCallback
 *             Callback function called by the CAK to notify the application
 *             of the PIN code validity.
 *  @param   xPinIndex
 *             The PIN index.
 *  @param   xPinCode
 *             The secret PIN code.
 *  @param   pxPrivateData
 *             Optional private data. It is passed to the callback function.
 *             Content of private data is independent of the CA.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
*/
NAGRA_CA_API TCaStatus caCheckPinCode
(
  TPinCodeStatusCallback   xPinCodeStatusCallback,
  TPinIndex                xPinIndex,
  TPinCode                 xPinCode,
  void*                   pxPrivateData
);


/**
 *  @brief
 *    Change a smartcard PIN code.
 *
 *    This CA function is called by the application in order to
 *    change a smartcard PIN code.
 *
 *  @param   xPinCodeStatusCallback
 *             Callback function called by the CAK to notify the application
 *             of the PIN code changing validity.
 *  @param   xPinIndex
 *             The PIN index.
 *  @param   xOldPinCode
 *             The old secret PIN code.
 *  @param   xNewPinCode
 *             The new secret PIN code.
 *  @param   pxPrivateData
 *             Optional private data. It is passed to the callback function.
 *             Content of private data is independent of the CA.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
*/
NAGRA_CA_API TCaStatus caChangePinCode
(
  TPinCodeStatusCallback   xPinCodeStatusCallback,
  TPinIndex                xPinIndex,
  TPinCode                 xOldPinCode,
  TPinCode                 xNewPinCode,
  void*                   pxPrivateData
);



/******************************************************************************/
/*                       NOTIFICATION REGISTRATION                            */
/******************************************************************************/


/**
 *  @brief
 *    Access notification registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to be notified of changes about access information of any events.
 *
 *    The registration may be cancelled with the caCancelRegistration()
 *    function.
 *
 *  @deprecated
 *
 *  @param   xAccessNotificationCB
 *             Decoder manufacturer's function called whenever an access
 *             information notification occurs.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
 *
 *  @remark
 *    -# Only changes implied by a smartcard change are reported. In
 *       particular, if the SI/PSI changes, no notification is generated.
*/
NAGRA_CA_API TCaStatus caRegisterAccessNotification
(
  TAccessNotificationCallback   xAccessNotificationCB,
  TCaRegistrationId*           pxRegistrationId
);



/**
 *  @brief
 *    Event purchase list notification registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to be notified when the content of the event purchase list changes.
 *
 *    The registration may be cancelled with the caCancelRegistration()
 *    function
 *
 *  @deprecated
 *
 *  @param   xEventPurchaseListNotification
 *             Decoder manufacturer's function called whenever a pay-per-view
 *             event list notification occurs.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterEventPurchaseListNotification
(
  TEventPurchaseListNotification  xEventPurchaseListNotification,
  TCaRegistrationId*              pxRegistrationId
);


/**
 *  @brief
 *    Credit list notification registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to be notified of any smartcard credit changes.
 *    The registration may be cancelled with the caCancelRegistration()
 *     function.
 *
 *  @deprecated
 *
 *  @param   xCreditNotification
 *             Decoder manufacturer's function called whenever a credit
 *             notification occurs.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterCreditNotification
(
  TCreditNotification  xCreditNotification,
  TCaRegistrationId*   pxRegistrationId
);

/**
 *  @brief
 *    Location information notification registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to be notified of any change about the smartcard location
 *    information.
 *
 *    The registration may be cancelled with the caCancelRegistration()
 *    function.
 *
 *  @deprecated
 *
 *  @param   xLocationInfoNotification
 *             Decoder manufacturer's function called whenever a location
 *             information notification occurs.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterLocationInformationNotification
(
  TLocationInfoNotification  xLocationInfoNotification,
  TCaRegistrationId*         pxRegistrationId
);



/**
 *  @brief
 *    System information notification registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to be notified of any change about the system information.
 *
 *    The registration may be cancelled with the caCancelRegistration()
 *    function.
 *
 *  @deprecated
 *
 *  @param   xSystemInfoNotification
 *             Decoder manufacturer's function called whenever a system
 *             information notification occurs.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterSystemInfoNotification
(
  TSystemInfoNotification  xSystemInfoNotification,
  TCaRegistrationId*       pxRegistrationId
);


/**
 *  @brief
 *    This function allows the decoder manufacturer to register
 *    with the CA in order to receive all the CA alarms for
 *    all the smartcards and all the descrambling streams (Obsolete).
 *
 *    The registration can be cancelled with caCancelRegistration.
 *
 *  @deprecated
 *
 *  @param  xAlarmExportation
 *            Decoder manufacturer's function called whenever an alarm
 *            exportation occurs.
 *  @param  pxRegistrationId
 *            Identifier to use to cancel this registration.
 *
 *  @retval CA_NO_ERROR
 *            Registration OK
 *  @retval CA_ERROR_CA_NOT_RUNNING
 *            The CA is currently not running
 *  @retval CA_ERROR
 *            Registration failed
 *
 *  @see caCancelRegistration
 *
*/
NAGRA_CA_API TCaStatus caRegisterAlarmExportation
(
  TAlarmExportation  xAlarmExportation,
  TCaRegistrationId* pxRegistrationId
);


/**
 *  @brief
 *    IRD command exportation registration (Obsolete).
 *
 *    This function allows the decoder manufacturer to register with the CA in
 *    order to receive each IRD command.
 *
 *    The registration may be cancelled with the caCancelRegistration()
 *    function.
 *
 *  @deprecated
 *
 *  @param   xIrdCmdExportation
 *             Decoder manufacturer's function called whenever a command (or
 *             part of a command) is exported to the decoder.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterIrdCmdExportation
(
  TIrdCmdExportation  xIrdCmdExportation,
  TCaRegistrationId*  pxRegistrationId
);


/**
 * @brief
 *   Termination callback registration (Obsolete).
 *
 *   This function allows the decoder manufacturer and the application
 *   programmer to register with the CA in order to be notified of the
 *   termination of the CA.
 *   The registration may be cancelled with the caCancelRegistration()
 *   function (section 6.2).
 *   The notification is done as soon as the function caTermination() is
 *   called, before the real termination process. Pending request will be
 *   replied with CA_XXX_SHUTDOWN after this notification.
 *   Once terminated, the CA is not able to process requests any more.
 *
 *  @deprecated
 *
 *  @param   xTerminationNotification
 *             Decoder manufacturer's function called when the CA terminates.
 *  @param   pxRegistrationId
 *             Identifier for this specific registration. It is required to
 *             cancel the registration.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             Registration has failed.
*/
NAGRA_CA_API TCaStatus caRegisterTerminationNotification
(
  TTerminationNotification  xTerminationNotification,
  TCaRegistrationId*        pxRegistrationId
);


/**
 * @brief
 *   Cancel a CA registration (Obsolete).
 *
 *   This function is responsible of the removing of a specific registration
 *   data from the CA notification or exportation lists.
 *
 *  @deprecated
 *
 *  @param   xRegistrationId
 *             Identifier of the registration to cancel. It is provided by the
 *             registration function.
 *
 *  @retval  CA_NO_ERROR
 *             Registration is successful.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             Registration has failed.
 *
 *  @remark
 *    -# At CA termination, all remaining registrations are implicitly canceled.
*/
NAGRA_CA_API TCaStatus caCancelRegistration
(
  TCaRegistrationId  xRegistrationId
);

/**
 *  @brief
 *    Get unique key.
 *
 *    This function copies the unique key into the provided buffer.
 *
 *  @pre
 *    CA must be running.
 *
 *  @post
 *    None.
 *
 *  @param   pxUniqueKey
 *             Pointer where to copy CA unique key.
 *  @param   pxKeySize
 *             IN  : Size of provided buffer (must be at least 16 bytes).
 *             OUT : Size of copied data.
 *
 *  @retval  CA_NO_ERROR
 *             The key has been copied successfully.
 *  @retval  CA_ERROR
 *             Buffer size error.
*/
NAGRA_CA_API TCaStatus caGetUniqueKey
(
  TUnsignedInt8* pxUniqueKey,
  TSize*         pxKeySize
);


/**
 *  @brief
 *    Request a call collection
 *
 *    This CA function is called by the application in order to
 *    request an immediate call collection.
 *
 *  @param   xCallCollectionStatusCallback
 *             Callback used by the CA to notify the application of the call
 *             collection progress. Notice that the callback is called for
 *             application-initiated call collections and never for smart
 *             card-initiated call collections.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# If the application makes multiple requests, the CAK takes into account
 *       only the last request and previous parameter is overwritten.
 *    -# In case the application requests a call collection while a standard
 *       automatic call collection is already in progress, the application
 *       benefits from the current call and is notified of the current status.
 *       Afterwards, status message are provided as if the call was initiated
 *       by the application.
 *    -# A call collection remains active until it is completed successfully or
 *       cancelled by a call to the caCallCollectionCancelRequest function
 *       In case of error, the call collection is restarted automatically
 *       after a delay (retry period).
 *    -# A call collection requested by the application may be canceled by
 *       means of the caCallCollectionCancelRequest function.
*/
NAGRA_CA_API TCaStatus caCallCollectionRequest
(
  TCallCollectionStatusCallback xCallCollectionStatusCallback
);


/**
 *  @brief
 *    Cancel a call collection request.
 *
 *    This function allows the application to cancel a previous call collection
 *    request.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
 *
 *  @remark
 *    -# If the connection is currently under way, the application stops being
 *       notified of the call progress (xCallCollectionStatusCallback no longer
 *       called), but the call is not aborted until the end of the collection.
 *       A canceled call collection is not restarted automatically in case of
 *       errors.
 *    -# The caCallCollectionCancelRequet function cannot be used to cancel a
 *       standard call collection that was not explicitly requested by the
 *       application.
*/
NAGRA_CA_API TCaStatus caCallCollectionCancelRequest
(
  void
);

/**
 *  @brief
 *    Network ID list request.
 *
 *    This function sends a request to the CA in order to get the network ID
 *    list. Data resulting from this request is returned asynchronously to the
 *    caller by means of the provided callback.
 *
 *  @param   xNetworkIdListCallback
 *             Callback function called by the CA to return the network ID list
 *             to the application.
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caNetworkIdListRequest
(
  TNetworkIdListCallback   xNetworkIdListCallback
);


/**
 *  @brief
 *    Network ID list acknowledgment.
 *
 *    This function must be called by the decoder manufacturer in order to
 *    acknowledge the CA that the referenced network ID list may be flushed.
 *
 *  @param   pxNetworkIdListArray
 *             Pointer to the first item of the network ID array to be
 *             flushed.
 *
 *  @retval  CA_NO_ERROR
 *             Success.
 *  @retval  CA_ERROR
 *             Failure.
*/
NAGRA_CA_API TCaStatus caNetworkIdListAcknowledgement
(
  TUnsignedInt16*         pxNetworkIdListArray
);


/**
 *  @brief
 *    Asset information request.
 *
 *    This function sends a request to the CA in order to get the information
 *    of an asset. To do so, the CA needs the descriptors contained in the CA
 *    Descriptor File of the asset. The decoder application shall pass the
 *    complete data contained in the CA Descriptor File as a raw buffer,
 *    pointed by pxAssetDescriptorBlock.
 *<br>
 *    The processing of the request is performed asynchronously with regard to
 *    the calling task. The CA returns the resulting data by means of the
 *    provided callback function. The CA software can manage a burst of
 *    requests. Therefore, it is not necessary to wait for a response before
 *    sending another request.
 *
 *  @param  xAssetInfoCallback
 *            Callback function called by the CA to return the resulting asset
 *            information. It is called only once per request.
 *  @param pxPrivateData
 *            Reference to optional private data. It is passed to the callback
 *            function. Content of private data is independent of the CA. There
 *            is no safe copy of the content.
 *  @param  xSizeOfAssetSpecificDescriptors
 *            Size in bytes of the asset specific descriptor block passed below.
 *  @param pxAssetSpecificDescriptorBlock
 *            Pointer to buffer containing the asset specific descriptors.
 *            The buffer must contain all the data contained in the CA
 *            Descriptor File.
 *
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# The calling task shall not overwrite or de-allocate all the referenced
 *       data before the callback function is called (or until the function
 *       returns if an error occurred).
 *    -# All the descriptors found in the CA Descriptor File of the asset
 *       must be provided. The application may not filter any of them.
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caPvdAssetInformationRequest
(
  TPvdAssetInfoCallback  xAssetInfoCallback,
  void*                 pxPrivateData,
  TSize                  xSizeOfAssetSpecificDescriptors,
  TUnsignedInt8*        pxAssetSpecificDescriptorBlock
);

/**
 *  @brief
 *    Product information request.
 *
 *    This function sends a request to the CA in order to get the information
 *    of an Push-VOD product. To do so, the CA needs the descriptors contained
 *    in the CA Descriptor File of the asset. The decoder application shall pass
 *    the complete data contained in the CA Descriptor File as a raw buffer,
 *    pointed by pxProductDescriptorBlock.
 *<br>
 *    The processing of the request is performed asynchronously with regard to
 *    the calling task. The CA returns the resulting data by means of the
 *    provided callback function. The CA software can manage a burst of
 *    requests. Therefore, it is not necessary to wait for a response before
 *    sending another request.
 *
 *  @param  xProductInfoCallback
 *            Callback function called by the CA to return the resulting product
 *            information. It is called only once per request.
 *  @param pxPrivateData
 *            Reference to optional private data. It is passed to the callback
 *            function. Content of private data is independent of the CA. There
 *            is no safe copy of the content.
 *  @param  xSizeOfProductSpecificDescriptors
 *            Size in bytes of the product specific descriptor block passed
 *            below.
 *  @param pxProductSpecificDescriptorBlock
 *            Pointer to buffer containing the product specific descriptors.
 *            The buffer must contain all the data contained in the CA
 *            Descriptor File.
 *
 *
 *  @retval  CA_NO_ERROR
 *             The request has been sent successfully.
 *  @retval  CA_ERROR_CA_NOT_RUNNING
 *             The CA is currently not running and the request cannot be
 *             processed.
 *  @retval  CA_ERROR
 *             The request has not been sent successfully.
 *
 *  @remark
 *    -# The calling task shall not overwrite or de-allocate all the referenced
 *       data before the callback function is called (or until the function
 *       returns if an error occurred).
 *    -# All the descriptors found in the CA Descriptor File of the product
 *       must be provided. The application may not filter any of them.
 *    -# There is no calling back if the function returns an error.
*/
NAGRA_CA_API TCaStatus caPvdProductInformationRequest
(
  TPvdProductInfoCallback  xProductInfoCallback,
  void*                   pxPrivateData,
  TSize                    xSizeOfProductSpecificDescriptors,
  TUnsignedInt8*          pxProductSpecificDescriptorBlock
);


/** @endcond DOX_NO_EXPORT */

#ifdef __cplusplus
}
#endif

#endif /* CA_CAK_H */

/* EOF */
