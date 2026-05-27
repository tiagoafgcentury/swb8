/**
** @file ca_dmx.h
**
** @brief
**   This file defines the interface between the Nagravision Software
**   and the decoder's demux hardware.
**
**  @ingroup DMX
**
**  $Id: //CAK/components/dalapi/TAGS/DALAPI_7_17_3/src/com/dmx/ca_dmx.h#1 $
**  $Change: 297498 $
**
** COPYRIGHT:
**   1998-2009 NagraVision S.A.
**
*/
/**
 *  @defgroup DMX Demultiplexer
 *  @brief
 *    Demultiplexer component
 *
 *    This component is the interface to the demultiplexer driver.
*/

#ifndef CA_DMX_H
#define CA_DMX_H

#ifdef DMX_WRAP
#include "dmxwrap.h"
#endif /* DMX_WRAP */

#ifdef NAGRA_CA_V5

#ifdef DMX_6_CAK_5
#include "dmxhd6c5.h"
#endif /* DMX_6_CAK_5 */
#include "ca_dmx_o.h"

#else /* NAGRA_CA_V5 */
/* CAK V6 */
#ifdef DMX_5_CAK_6
#include "dmxhd5c6.h"
#endif /* DMX_5_CAK_6 */

/* DMX driver V6 implemented, use this API */

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"
#include "ca_defsx.h"
#include "ca_os.h"

#include "ca_mpeg.h"

/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    The demultiplexer interface only defines \c TDmxFilterId as an opaque
 *    pointer. Each implementation may define the structure it likes to suit its
 *    needs. The status \c DMX_FILTER_ID_INVALID shall be used for invalid
 *    filter identifiers
 *
 *  @see DMX_FILTER_ID_INVALID
*/
typedef struct SDmxFilterId* TDmxFilterId;

/**
 *  @brief
 *    Invalid value of \c TDmxFilterId
 *
 *  @see TDmxFilterId
*/
#define DMX_FILTER_ID_INVALID ((TDmxFilterId) NULL)


/**
 *  @brief
 *    Pattern for packet filtering
 *
*/
typedef TUnsignedInt8 TDmxFilterPattern;


/**
 *  @brief
 *    Buffer status
 *
 * @see
 *    TDmxFilterQueryBufferCallback
*/
typedef enum
{
  DMX_BUFFER_AVAILABLE,
    /**<  The buffer has been provided
    */
  DMX_BUFFER_ERROR
    /**<  No buffer has been provided
    */
} TDmxBufferStatus;


/**
 *  @brief
 *    State of the received section
 *
 * @see
 *    TDmxFilterReceivedSectionCallback
*/
typedef enum
{
  DMX_SECTION_AVAILABLE,
    /**<  Section received successfully
    */
  DMX_SECTION_CRC_ERROR,
    /**<  The section's CRC is corrupted
    */
  DMX_SECTION_BUFFER_ERROR,
    /**<  No buffer was available
    */
  DMX_SECTION_TIMEOUT_ERROR,
    /**<  The filtering timed out
    */
  DMX_SECTION_ERROR
    /**<  Other error
    */
} TDmxSectionStatus;


/**
 *  @brief
 *    Demultiplexer loop modes
 *
 *  @see
 *    dmxFilterFilterStart
*/
typedef enum
{
  DMX_LOOP_CONTINUOUS,
    /**<  The demultiplexer shall filter all incoming sections matching the
     *    pattern
    */
  DMX_LOOP_ONE_SHOT,
    /**<  The demultiplexer shall filter one section only. All subsequent
     *    sections shall be ignored until the next call to \c dmxFilterStart()
     *    call. The parameters can be changed.
    */
  DMX_LOOP_TOGGLE
    /**<  This mode is used to acquire sections broadcast alternately on two
     *    different tables whose table ID differs by the bit 0 (i.e 0x80 and
     *    0x81).\n
     *    To do so, right after a section is acquired, the DMX driver shall
     *    change the bit 0 of the first byte of the mask pattern so that it
     *    differs from the bit 0 of the first byte of the section just acquired.
     *    \n
     *    The initial value of the first byte of the mask pattern given by \c
     *    dmxFilterSetPatterns is equal to \c 0xFE in order to acquire the first
     *    section independently of table ID parity.
    */
} TDmxLoopMode;


/**
 *  @brief
 *    Packet CRC check mode
 *
 * @see
 *    dmxFilterFilterStart
*/
typedef enum
{
  DMX_CRC_CHECK,
    /**<  The demultiplexer driver is in charge of checking the CRC
    */
  DMX_CRC_IGNORE
    /**<  The demultiplexer driver shall ignore the CRC
    */
} TDmxCrcMode;


/**
 *  @brief
 *    Status returned by DMX functions
 *
*/
typedef enum
{
  DMX_NO_ERROR,
    /**<  Success
    */
  DMX_WARNING_SOFTWARE_FILTERING,
    /**<  Deprecated
    */
  DMX_ERROR_NO_MORE_RESOURCES,
    /**<  No more filter resources are available for the demultiplexer
    */
  DMX_ERROR_UNKNOWN_ID,
    /**<  The filter resource was not opened
    */
  DMX_ERROR_FILTER_RUNNING,
    /**<  The filter to be modified is still running
    */
  DMX_ERROR_BAD_PID,
    /**<  The given PID is out of range
    */
  DMX_ERROR_BAD_PARAMETER,
    /**<  The given parameters are inconsistent
    */
  DMX_ERROR
    /**<  Other error
    */
} TDmxStatus;


/******************************************************************************/
/*                                                                            */
/*                      CALLBACK FUNCTION TYPEDEFS                            */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    This callback function is used to provide the demux driver with a buffer
 *    where to store a section matching the corresponding filter set previously.
 *
 *  @param[in]  pxFilterId
 *                Identifier to the filter
 *
 *  @param[out] ppxBuffer
 *                Reference to the buffer allocated
 *
 *  @param[out] pxBufferSize
 *                Size of the buffer allocated
 *
 *  @param[in]  pxPrivateData
 *                Private data coming from \c dmxFilterOpen
 *
 *  @retval  DMX_BUFFER_AVAILABLE
 *             The buffer has been allocated.
 *
 *  @retval  DMX_BUFFER_ERROR
 *             The buffer has not been allocated.
 *
 *  @remarks
 *    -# If \c DMX_BUFFER_AVAILABLE is returned, the demultiplexer driver will
 *       have to call sooner or later the section callback
 *       \c TDmxFilterReceivedSectionCallback in order to indicate to the CA
 *       software that the buffer returned by \c TDmxFilterQueryBufferCallback
 *       can be freed. If \c DMX_BUFFER_ERROR is returned, it is not necessary
 *       to call the section callback.
*/
typedef TDmxBufferStatus (*TDmxFilterQueryBufferCallback)
(
        TDmxFilterId      xFilterId,
        TUnsignedInt8** ppxBuffer,
        TSize*           pxBufferSize,
        void*            pxPrivateData
);


/**
 *  @brief
 *    This callback function is used to notify the CA software of the
 *    acquisition of a section matching the corresponding filter set previously.
 *
 *    The filter ID is given back as parameter for a quick reference to the
 *    original filter settings.
 *
 *  @param[in]  pxFilterId
 *                Identifier of the filter

 *  @param[in]  xSectionStatus
 *                State of the received section. Can be one of:
 *                - \c DMX_SECTION_AVAILABLE: section received successfully
 *                - \c DMX_SECTION_ERROR: generic section receive error
 *                - \c DMX_SECTION_CRC_ERROR: the section's CRC is corrupted
 *                - \c DMX_SECTION_BUFFER_ERROR: no buffer was available
 *                - \c DMX_SECTION_TIMEOUT_ERROR: the filtering did timeout
 *                .
 *  @param[in]  pxSection
 *                Reference to the buffer provided by \c
 *                TDmxFilterQueryBufferCallback containing the section acquired.
 *
 *  @param[in]  xSectionSize
 *                Size of the section acquired
 *
 *  @param[in]  pxPrivateData
 *                Private data coming from \c dmxFilterOpen
 *
 *  @return  void
 *
 *  @remarks
 *    -# If buffer callback \c TDmxFilterQueryBufferCallback successfully
 *       returned a buffer (\c DMX_BUFFER_AVAILABLE ), it is mandatory for the
 *       demultiplexer driver to call the section callback
 *       \c TDmxFilterReceivedSectionCallback, even if an error occurred or if
 *       the filter was stopped or reset and no section is available.
 *    .
 *    -# In case the CRC mode is enabled (c.f. \c dmxFilterStart), if the
 *       demultiplexer  driver is able to detect a section corrupted before
 *       having called the buffer callback, the driver can skip the section
 *       without calling the section callback. On the contrary, if the check of
 *       the CRC occurs after having called the buffer callback, the
 *       demultiplexer has to call the section callback and  set the section
 *       status to \c DMX_SECTION_CRC_ERROR. In this latter case, xSectionSize
 *       shall be set to 0 and pxSection shall contain the address of the buffer
 *       provided so that it can be freed.
 *    .
 *    -# In case of timeout (\c DMX_SECTION_TIMEOUT_ERROR ) or any other error
 *       (\c DMX_SECTION_ERROR ), \c xSectionSize shall be set to 0 and \c
 *       pxSection shall contain the address of the buffer provided so that it
 *       can be freed.
 *    .
 *    -# If the buffer callback returned \c DMX_BUFFER_ERROR, there is no need
 *       for the  driver to call the section callback. However, if the driver
 *       calls it, it has to set the section status to \c
 *       DMX_SECTION_BUFFER_ERROR and \c pxSection and \c xSectionSize shall be
 *       set to NULL and 0 respectively.
*/
typedef void (*TDmxFilterReceivedSectionCallback)
(
        TDmxFilterId        xFilterId,
        TDmxSectionStatus   xSectionStatus,
  const TUnsignedInt8*     pxSection,
        TSize               xSectionSize,
        void*              pxPrivateData
);

/******************************************************************************/
/*                                                                            */
/*                            FUNCTION PROTOTYPES                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    This function opens a demux filter.
 *
 *  @param[out] pxFilterId
 *                Identifier assigned to the demux filter opened.
 *
 *  @param[in]  xTransportSessionId
 *                Transport session identifier associated to the stream to be
 *                filtered.
 *
 *  @param[in]  xFilterDepth
 *                Maximum size in bytes of the matching section filter pattern,
 *                starting from the table ID. If the filter requires more than
 *                the table ID, \c xFilterDepth will include the two bytes of
 *                \c section_length.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the CA software for internal processing.
 *                This pointer shall be passed back as parameter of the two
 *                callback functions \c TDmxFilterQueryBufferCallback and \c
 *                TDmxFilterReceivedSectionCallback
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_NO_MORE_RESOURCES
 *             No more demux filter resources are available
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterOpen
(
        TDmxFilterId*	         pxFilterId,
        TTransportSessionId	    xTransportSessionId,
        TSize	                  xFilterDepth,
  const void*	                 pxPrivateData
);


/**
 *  @brief
 *    This function closes the given demux filter
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter to be closed
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             This filter resource was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterClose
(
  TDmxFilterId   xFilterId
);


/**
 *  @brief
 *    This function defines the filtering patterns of the demux filter.
 *
 *    Let's illustrate these parameters by an example. If we would like to catch
 *    a section with the first byte set to 0x42 and the third one different than
 *    0xC4, then the filtering patterns shall be defined as follows:
 *
 *      \c pxValue=0x4200C4          \n
 *      \c pxEqualMask=0xFF0000      \n
 *      \c pxNotEqualMask=0x0000FF   \n
 *
 *  @param[in]  xFilterId
 *                Identifier of the demux filter
 *
 *  @param[in]  pxValue
 *                Section filter value pattern. This pattern is the array of
 *                bytes that are compared to the incoming section. This
 *                pattern is used in combination with the "equal" and
 *                "not-equal" masks. These two masks indicates precisely which
 *                bit of the value pattern are taken into account ( i.e. all
 *                bits not covered by a mask are ignored).
 *
 *  @param[in]  pxEqualMask
 *                Bitmap indicating which bits of the value pattern must match
 *                the incoming section.
 *
 *  @param[in]  pxNotEqualMask
 *                Bitmap indicating which bits of the value pattern must not match
 *                the incoming section.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR_BAD_PARAMETER
 *             The given parameters are inconsistent
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# The size of the patterns is equal to the filter depth given in
 *       \c dmxFilterOpen().
 *    .
 *    -# This function shall only be called when the filter is stopped.
 *    .
 *    -# The \c pxEqualMask and \c pxNotEqualMask masks must not overlap
 *       (the bit-and operation of these two masks shall be equal to 0).
 *       \c DMX_ERROR_BAD_PARAMETER shall be returned when masks overlap.
*/
TDmxStatus dmxFilterSetPatterns
(
        TDmxFilterId   xFilterId,
  const TUnsignedInt8* pxValue,
  const TUnsignedInt8* pxEqualMask,
  const TUnsignedInt8* pxNotEqualMask
);


/**
 *  @brief
 *    This function set the PID of packet to be filtered
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xPid
 *                PID of the packet to be filtered
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_BAD_PID
 *             The PID is out of range
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is still running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function can called only if the filter is stopped.
*/
TDmxStatus dmxFilterSetPid
(
  TDmxFilterId    xFilterId,
  TPid            xPid
);



/**
 *  @brief
 *    This function starts filtering on a specified filter.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @param[in]  xTimeout
 *                Timeout on the filter in [ms]. If 0, the timeout is considered
 *                to be infinite. If the timeout expires and no section has been
 *                catched, the notification callbacks must be called anyway with
 *                \c pxSection=NULL and \c xSectionSize=0.
 *
 *  @param[in]  xLoopMode
 *                This mode defines the behavior of the filter right after
 *                catching a matching section:
 *                - \c DMX_LOOP_CONTINUOUS: the filter remains active as long as
 *                  not explicitely stopped by \c calling \c dmxFilterStop().
 *                - \c DMX_LOOP_ONE_SHOT: the filter automatically stops after
 *                  catching a section.
 *                - \c DMX_LOOP_TOGGLE: this mode is used to acquire sections
 *                  broadcast alternately on two different tables. The filter
 *                  remains active as long as not explicitely stopped by
 *                  calling \c dmxFilterStop().
 *
 *  @param[in]  xCrcMode
 *                Indicate whether the CRC has to be checked or not:
 *                - \c DMX_CRC_CHECK
 *                - \c DMX_CRC_IGNORE
 *
 *  @param[in]  xQueryBufferCallback
 *                Callback used by the DMX driver to get a buffer in order to
 *                store a matching section.
 *
 *  @param[in]  xReceivedSectionCallback
 *                Callback used by the DMX driver to notify the CA software of
 *                that a section matching the filter pattern has been acquired.
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR_FILTER_RUNNING
 *             The filter is already running
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# Both callbacks are mandatory
 *    .
 *    -# After a one shot filtering, the filter is simply stopped and can be
 *       restarted at any time with \c dmxFilterStart().
 *    .
 *    -# This function can only be called when the filter is stopped.
*/
TDmxStatus dmxFilterStart
(
  TDmxFilterId                        xFilterId,
  TOsTime                             xTimeout,
  TDmxLoopMode                        xLoopMode,
  TDmxCrcMode                         xCrcMode,
  TDmxFilterQueryBufferCallback       xQueryBufferCallback,
  TDmxFilterReceivedSectionCallback   xReceivedSectionCallback
);


/**
 *  @brief
 *    This function stops the filtering on a specified filter and freezes the
 *    buffer in its current state.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
 *  @remarks
 *    -# This function does flush the reception buffer.
*/
TDmxStatus dmxFilterStop
(
  TDmxFilterId   xFilterId
);


/**
 *  @brief
 *    This function resets the filtering on a specified filter by flushing the
 *    buffer and resetting the timeout.
 *
 *  @param[in]  xFilterId
 *                Identifier of the filter
 *
 *  @retval  DMX_NO_ERROR
 *             Success
 *
 *  @retval  DMX_ERROR_UNKNOWN_ID
 *             The filter was not opened
 *
 *  @retval  DMX_ERROR
 *             Other error
 *
*/
TDmxStatus dmxFilterReset
(
  TDmxFilterId   xFilterId
);

#endif /* NAGRA_CA_V5 */

#endif /* CA_DMX_H */

/* ca_dmx.h */
