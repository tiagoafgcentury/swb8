/**
** @file nv_prmSyntax.h
**
** @brief
**   DVR/VOD Library PRM Syntax functions.
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


#ifndef NV_PRMSYNTAX_H
#define NV_PRMSYNTAX_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                     TYPES                                  */
/*                                                                            */
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
 *  @ingroup prmSyntax
 *  @brief
 *    This function generate a prmSyntax from a Credential.
 *
 *
 *  @pre
 *    The DVR/VOD Library must be running.
 *  @post
 *
 *
 *  @param[in] xCredentialsSize
 *            Size in bytes of the credentials supplied by the application.
 *  @param[in] pxCredentials
 *            Address of the buffer containing the Credentials.\n
 *            Only HLCM and LCM are currently supported.\n
 *            Shall not be \c NULL.
 *  @param[out] pxPrmSyntaxSize
 *            Pointer where the function will store the size in bytes of prmSyntax
 *  @param[out] ppxPrmSyntax
 *            address of the buffer containing prmSyntax\n
 *            Shall not be \c NULL.\n
 *  @retval DVL_NO_ERROR
 *            The function execution was successful.
 *  @retval DVL_ERROR_BAD_PARAMETER
 *            One or more arguments of the function are invalid or missing.
 *  @retval DVL_ERROR_DVL_NOT_RUNNING
 *            The function could not be executed as the DVR/VOD Library is not
 *            running.
 *  @retval DVL_ERROR
 *            The function execution failed.
*/
NAGRA_CA_API TDvlStatus dvlGeneratePrmSyntax
(
        TSize                                xCredentialsSize,
        TUnsignedInt8*                      pxCredentials,
        TSize*                              pxPrmSyntaxSize,
        TChar**                            ppxPrmSyntax
);

#ifdef __cplusplus
}
#endif

#endif /* NV_PRMSYNTAX_H */

/* END OF FILE */
