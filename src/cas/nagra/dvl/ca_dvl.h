/**
** @file ca_dvl.h
**
** @brief
**   DVR/VOD Library external definitions.
**
**   The syntax of the comments of this file is compliant to the format used by
**   Doxygen to generate html documentation from source and header files.
**   Doxygen can be downloaded from www.doxygen.org and is provided under GNU
**   General Public License.
**
** $Revision:  $
**
** Copyright:
**   2006-2017 Nagra France S.A.
**
**
*/


#ifndef CA_DVL_H
#define CA_DVL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the version number of the PRM API.
 */
#define PRMAPI_VERSION_MAJOR  3
#define PRMAPI_VERSION_MEDIUM 5
#define PRMAPI_VERSION_MINOR  7

/** @page p_history Changes history
 *  - <b> 3.5.7 - 02-Jul-2019 </b>
 *    - dvlCredentialsInformationRequest() is extended to support lcm 400
 *    - Added the following members to TDvlLcmRotType in nv_info.h.
 *      - lcmRotType
 *      - deviceIdSize
 *      - deviceId
 *  - <b> 3.5.6 - 26-Nov-2018 </b>
 *    - General improvements.
 *  - <b> 3.5.5 - 21-Feb-2018 </b>
 *    - Added user intent support for Transcode usecase.
 *      @remarks
 *        User Intent feature is only available with DVL dual Library.
 *  - <b> 3.5.4 - 23-Nov-2017 </b>
 *    - Updated comments related to dvlDebugControlRole.
 *  - <b> 3.5.2 - 11-Sep-2017 </b>
 *    - Added Secure Media Path enforcement through compilation flag.
 *  - <b> 3.5.1 - 24-Apr-2017 </b>
 *    - Renamed the following files
 *      - nv_info.h to nv_relatedCredInfo.h
 *      - nv_info_lite.h to nv_info.h
 *      - nv_pvrvod_lite.h to nv_pvr.h
 *    - Moved TDvlPlaybackErrorCB to nv_dvlDeprecated.h
 *    - DVL Lite will be now addressed as DVL Dual.
 *    - Updated comments related to DVL Dual Library.
 *  - <b> 3.5.0 - 28-Mar-2017 </b>
 *    - PRM API version update for updates in documentation to reflect CERT full feature support. No change in PRM API from 3.4.9.
 *  - <b> 3.4.9 - 07-Feb-2017 </b>
 *    - Added Secure Media Path support in Playback and Record usecase.
 *      - Added member variable secureMediaPathEnabled in \ref TDvlRecordSessionParameters and \ref TDvlEntitlementsParameters.
 *    - Added interface \c dvlConvertTimeshiftToRecording().
 *    - Moved system and chipset information APIs from nv_info.h to nv_info_lite.h file to use in DVL dual.
 *    - Moved Playback, Record and LCM metadata update APIs from nv_pvrvod.h to nv_pvrvod_lite.h file to use in DVL dual.
 *    - Removed all deprecated APIs from DVL dual and moved to nv_dvlDeprecated.h file.
 *    - Moved the common interfaces to nv_common.h file.
 *    - home network functionality moved to nv_homeNetwork.h from nv_homeDomain.h
 *    - Added new secret type to \c TDvlSecretsType for NOC3 chipset secrets
 *    - Removed interfaces \c dvlOnPause() and \c dvlOnResume().
 *      @remarks
 *        To manage standby on CERT flavor dvlInitialize() and dvlTerminate() shall be used.
 *      @remarks
 *        Secure Media Path support and interface \c dvlConvertTimeshiftToRecording() are only available with DVL dual Library.
 *  - <b> 3.4.8 - 05-Jan-2017 </b>
 *    - Used acdapi for user intent support in Playback and Record usecase.
 *      - Removed the structure \ref TDvlUserIntent and used the structure \ref TNvUserIntent from acdapi
 *      @remarks
 *        User Intent feature is only available with DVL dual Library.
 *  - <b> 3.4.7 - 28-Nov-2016 </b>
 *    - Add user intent support for Playback and Record.
 *      - Added new structure \ref TDvlUserIntent to be used by client for user intent setting
 *      - Added member variable userIntent of \ref TDvlUserIntent in TDvlEntitlementsParameters and TDvlRecordSessionParameters
 *      @remarks
 *        User Intent feature is only available with DVL dual Library.
 *  - <b> 3.4.6 - 17-Oct-2016 </b>
 *    - Add interface to get execution status of Home Domain API:
 *      - \c dvlGetLastHDStatus()
 *  - <b> 3.4.5 - 05-Aug-2016 </b>
 *    - General Improvements
 *  - <b> 3.4.4 - 17-Jun-2016 </b>
 *    - DVL interface split across multiple files
 *  - <b> 3.4.3 - 15-Jun-2016 </b>
 *    - update the input credentials that can be provided to \c dvlGeneratePrmSyntax()
 *    - update doxygen comments to indicate \c dvlGeneratePrmSyntax() now supports LCM
 *    - Add information related to pairing data usage in NOCS3 chipsets
 *    - update doxygen comments about pairing data usage \c TPairingImportationCB() in NOCS3 chipsets
 *  - <b> 3.4.0 - 15-April-2016 </b>
 *    - Add parameter expirationDate to TDvlNeighbourhoodInfo
 *    - update doxygen comments about output credentials storage
 *    - Note: As a new parameter has been added to TDvlNeighbourhoodInfo, the size
 *      of TDvlNeighbourhoodInfo has changed
 *  - <b> 3.3.0R - 04-April-2015 </b>
 *    - fix type of input parameter in \c dvlGeneratePrmSyntax()
 *    - update doxygen comments about output credentials storage
 *  -  <b> 3.3.0P - 04-Jan-2015 </b>
 *    - Add interfaces for Home Domain joining:
 *      - \c dvlJSONResponseGetHDE(), \c dvlPostHDE()
 *    - Add interfaces for creating Home Domain credentials:
 *      - \c dvlCreateHomeDomainContentMessage(),
 *        \c dvlCreateHomeDomainContentMessageParameters()
 *    - Add interface to generate a prmSyntax from Credentials \c dvlGeneratePrmSyntax()
 *    - Update \c dvlPlaybackSetEntitlements() and
 *      \c TDvlAccessStatus in order to support HLCM playback
 *    - Add interface to retrieve home domain information (Draft in Progress)
 *      - \c dvlHomeDomainsInformationRequest()
 *    - Add possibility top get information about a HLCM
 *      - new type of credentials information \c THlcmData
 *      - \c TCredentialsUnion updated
 *  -  <b> 3.2.0 - 26-Sep-2014 </b>
 *    - Add interfaces \c dvlOnPause() and \c dvlOnResume() to manage standby on CERT flavor
 *  -  <b> 3.1.0 - 06-Aug-2014 </b>
 *    - Add interfaces to manage direct communication with DVS \c dvlCreateJSONPayloadForServer(),
 *      \c dvlJSONResponseGetStatus(), \c dvlJSONResponseGetLCIs(), \c dvlJSONResponseGetEntitlements(),
 *      \c dvlJSONResponseGetPrivateData()
 *    - remove deprecated DVL direct mode interface \c dvlFetchEntitlements() and related structures
 *  - <b> 3.0.3 - 10-Mar-2014 </b>
 *    - Entitlements Information
 *      - Modification of input parameters of \c dvlRelatedCredentialsInformationRequest()
 *    - Entitlements Playback
 *      - Modification of input parameters of \c dvlPlaybackSetRelatedEntitlements()
 *    - Entitlements importation
 *      - Update type \c TDvlEntitlementImport
 *      - Update fields in \c TDvlEntitlementsImportStatus
 *    - Opaque Data
 *      - Application Data in \c dvlOpaqueDataMessageSetApplicationData() shall be base64 or at least JSON compatible (and no more required to be URL safe)
 *  - <b> 3.0.2 - 05-Feb-2014 </b>
 *    - New function to release importation handle \c dvlEntitlementsImportationRelease()
 *    - Function to set WDE table \c dvlSetWdeFunctionTable() is moved in a specific header file ca_dvl_wde.h
 *  - <b> 3.0.1 - 14-Nov-2013 </b>
 *    - Correction fields of \c TDvlEntitlementsImportStatus
 *    - Replace Application data in \c dvlFetchEntitlements() and related callback with handle
 *    - Add handle in \c dvlImportEntitlements()
 *    - Add communication timeout in \c dvlFetchEntitlements()
 *  - <b> 3.0.0 - 24-Oct-2013 </b>
 *    - Add support for Direct mode and Pre-delivery:
 *      - New function to set WDE table \c dvlSetWdeFunctionTable() in order to perform direct delivery
 *      - New function to import Entitlements in indirect delivery mode \c dvlImportEntitlements()
 *      - New function to handle direct delivery mode \c dvlFetchEntitlements()
 *      - New function to start a playback with supplying prmSyntax instead of entitlements \c dvlPlaybackSetRelatedEntitlements()
 *      - New function \c dvlRelatedCredentialsInformationRequest() to retrieve information about an Entitlement managed in DVL cache
 *    - Introduce definitions for using incoming DVL descrambling API.
 *      - New function to select descrambler \c dvlPlaybackSetDescramblingMode()
 *      - Additionnal parameter in \c TDvlRecordSessionParameters to select descrambler
 *        @remark
 *         only \c DVL_CONTENT_PROCESSING_MODE_SEC mode is supported in this version of DVL
 *  - <b> 2.1.0 - 04-Sep-2013 </b>
 *    - removed never implemented function dvlRegisterHomeNetworkChangesNotification
 *  - <b> 2.0.1 - 24-Jul-2013 </b>
 *    - Updated doxygen layout to improve generated .chm file.
 *    - Add History
 *    - Add Overview
 *    - display deprecated functions
 */


/** @mainpage Overview
 *  - @subpage p_history
 *
 *  <hr>Copyright &copy; 2013 Nagravision. All rights reserved.\n
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


/******************************************************************************/
/*                                                                            */
/*                              GROUPS DEFINITION                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @defgroup groupInitTerm Initialization and termination
*/

/**
 *  @defgroup groupSystemInfo System and Chipset Information
*/

/**
 *  @defgroup groupHomeNetwork Home Network Management
*/

/**
 *  @defgroup groupHomeDomain Home Domain Management
*/

/**
 *  @defgroup groupRecord Content Recording or Migrating
*/

/**
 *  @defgroup groupPlayback Content Playback
*/

/**
 *  @defgroup groupPlaybackRecordInfo Playback and Recording Info
*/

/**
 *  @defgroup groupCredInfo Credentials Information
*/

/**
 *  @defgroup groupPMM PRM Management Messages
*/

/**
 *  @defgroup groupMetadata LCM Metadata Update
*/

/**
 *  @defgroup groupMemory Memory Management
*/

/**
 *  @defgroup groupODM Opaque Data Management
*/

/**
 *  @defgroup groupImport Entitlements Importation Management
*/

/**
 *  @defgroup prmSyntax Prm Syntax
*/

/**
 *  @defgroup groupDebug Debugging
*/


/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"
#include "ca_defsx.h"

/* Init, Terminate, memory & debug */
#include "nv_common.h"
/* System, chipset & credential information */
#include "nv_info.h"
/* Related Credentials Information request */
#include "nv_relatedCredInfo.h"
/* Content recording, migration, plaback & LCM metadata */
#include "nv_pvr.h"
/* Legacy LCM & Playback set related entitlements */
#include "nv_pvrvod.h"
/* PRM Management Message */
#include "nv_pmm.h"
/* PRM Sytax */
#include "nv_prmSyntax.h"
/* Opaque data management */
#include "nv_opaque.h"
/* Entitlement importation management */
#include "nv_import.h"
/* Home domain management */
#include "nv_homeDomain.h"
/* Home network management */
#include "nv_homeNetwork.h"
/* DVL Deprecated APIs */
#include "nv_dvlDeprecated.h"

#ifdef __cplusplus
}
#endif

#endif /* CA_DVL_H */

/* END OF FILE */
