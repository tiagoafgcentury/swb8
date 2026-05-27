/**
**  @file ca_types.h
**
**  @brief
**    This file defines the general types and constants used by the
**    NagraVision embedded software.
**
**  $Id: //CAK/components/glbapi/TAGS/GLBAPI_1_2_1/src/ca_types.h#1 $
**
**  COPYRIGHT:
**    2014-2015 Nagravision S.A.
**
*/


#ifndef CA_TYPES_H
#define CA_TYPES_H

/******************************************************************************/
/*                                                                            */
/*                           GENERAL INCLUDE FILES                            */
/*                                                                            */
/******************************************************************************/

/* Grab size_t */
#include <stddef.h>

/* Grab C90 limits */
#include <limits.h>

/******************************************************************************/
/*                                                                            */
/*                            CAT TYPES DEFINITIONS                           */
/*                                                                            */
/******************************************************************************/


/* Look for a suitable 8-bit type */
#if UCHAR_MAX == 0xFF
/**
 *  @brief
 *    Signed 8-bit type.
*/
typedef signed char           TSignedInt8;


/**
 *  @brief
 *    Unsigned 8-bit type.
*/
typedef unsigned char         TUnsignedInt8;
#else
#error "No suitable type found for 8 bit integers"
#endif


/* Look for a suitable 16-bit type */
#if USHRT_MAX == 0xFFFF
/**
 *  @brief
 *    Signed 16-bit type.
*/
typedef signed short int      TSignedInt16;

/**
 *  @brief
 *    Unsigned 16-bit type.
*/
typedef unsigned short int    TUnsignedInt16;
#else
#error "No suitable type found for 16 bit integers"
#endif


/* Look for a suitable 32-bit type */
#if UINT_MAX == 0xFFFFFFFFL
/**
 *  @brief
 *    Signed 32-bit type.
*/
typedef signed int            TSignedInt32;

/**
 *  @brief
 *    Unsigned 32-bit type.
*/
typedef unsigned int          TUnsignedInt32;
#elif ULONG_MAX == 0xFFFFFFFFL
/**
 *  @brief
 *    Signed 32-bit type.
*/
typedef signed long int       TSignedInt32;

/**
 *  @brief
 *    Unsigned 32-bit type.
*/
typedef unsigned long int     TUnsignedInt32;
#else
#error "No suitable type found for 32 bit integers"
#endif


/**
 *  @brief
 *    Character type
*/
typedef char                  TChar;

/**
 *  @brief
 *    Size type
*/
typedef size_t                TSize;

/**
   *  @brief
   *    Boolean Type.
  */
typedef TUnsignedInt8         TBoolean;

/**
 * @brief
 *   The evaluation of a condition is false
 * @see TBoolean
*/
#ifndef FALSE
#define FALSE      ((TBoolean)(0))
#endif

/**
 * @brief
 *   The evaluation of a condition is true
 * @see TBoolean
*/
#ifndef TRUE
#define TRUE       ((TBoolean)(!FALSE))
#endif


#endif /* CA_TYPES_H */

/* END OF FILE */
