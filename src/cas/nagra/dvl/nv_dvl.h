/**
** @file nv_dvl.h
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
**   2006-2017 Nagra India
**
**
*/

#ifndef NV_DVL_H
#define NV_DVL_H

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
 *  - <b> 3.5.4 - 23-Nov-2017 </b>
 *    - Updated comments related to dvlDebugControlRole.
 *  - <b> 3.5.2 - 11-Sep-2017 </b>
 *    - Added Secure Media Path enforcement through compilation flag.
 *  - <b> 3.5.1 - 24-Apr-2017 </b>
 *    - Base version of DVL Dual Library.
 *
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
 *  @defgroup groupRecord Content Recording or Migrating
*/

/**
 *  @defgroup groupPlayback Content Playback
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
 *  @defgroup prmSyntax Prm Syntax
*/

/**
 *  @defgroup groupODM Opaque Data Management
*/

/**
 *  @defgroup groupMemory Memory Management
*/

/**
 *  @defgroup groupDebug Debugging
*/

/**
 * @defgroup groupPlaybackRecordInfo Playback and Recording Info
*/

/**
 *  @defgroup groupHomeNetwork Home Network Management
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
/* Content recording, migration, playback & LCM metadata */
#include "nv_pvr.h"
/* PRM Management Message */
#include "nv_pmm.h"
/* PRM Syntax */
#include "nv_prmSyntax.h"
/* Opaque data management */
#include "nv_opaque.h"
/* Home network management */
#include "nv_homeNetwork.h"

#ifdef __cplusplus
}
#endif

#endif /* NV_DVL_H */

/* END OF FILE */
