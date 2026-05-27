/**
**  @file ca_rph.h
**
**  @brief
**   Return path communication streams management.
**
**   This file defines the RPH API that can be used by the CA software to
**   communicate with the head-end through a return path connection.
**
**  @ingroup RPH
**
**  $Revision: 1.37 $
**
** COPYRIGHT:
**   2001-2009 NagraVision S.A.
**
*/
/**
 *  @defgroup RPH Return Path
 *  @brief
 *    Modem Communication component
 *
 *    The return path feature is intended to communicate in
 *    both ways with the head-end.
*/

#ifndef CA_RPH_H
#define CA_RPH_H

#if defined RPH_DEBUG_SHELL || defined RPH_WRAP
#include "rphwrap.h"
#endif /* RPH_DEBUG_SHELL or RPH_WRAP */

#ifdef NAGRA_CA_V5

#ifdef RPH_6_CAK_5
#include "rphhd6c5.h"
#endif /* RPH_6_CAK_5 */
#include "ca_rph_o.h"

#else /* NAGRA_CA_V5 */
/* CAK V6 */
#ifdef RPH_5_CAK_6
#include "rphhd5c6.h"
#endif /* RPH_5_CAK_6 */

/* RPH driver V6 implemented, use this API */

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"
#include "ca_os.h"


/******************************************************************************/
/*                                                                            */
/*                                 CONSTANTS                                  */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    The telephone number is made of maximum 20 numerical ASCII digits.
 *
 *  @see TRphPhoneNumber
 *
 */
#define RPH_PHONE_NUMBER_STR_MAX_LEN    20


/**
 *  @brief
 *    The PPP authentication username is made of maximum 16 characters.
 *
 *  @see TRphPppUsername
 *
 */
#define RPH_PPP_USERNAME_STR_MAX_LEN    16


/**
 *  @brief
 *    The PPP authentication password is made maximum 16 characters
 *
 *  @see TRphPppPassword
 *
 */
#define RPH_PPP_PASSWORD_STR_MAX_LEN    16

/**
 *  @brief
 *    Invalid value for TRphSessionHandle.
 *
 *  @see TRphSessionHandle
 */
#define RPH_SESSION_HANDLE_INVALID ((TRphSessionHandle) NULL)


/**
 *  @brief
 *    When calling \c rphReceive with this flag set, the calling task must
 *    wait for incoming data until time-out expiration or until \c xBufferSize
 *    bytes are read. Three cases may occur:
 *    -# The function returns \c RPH_NO_ERROR and \c *pxReceivedSize is equal to
 *       \c xBufferSize
 *    -# The function returns \c RPH_ERROR_TIMEOUT and \c *pxReceivedSize is
 *       strictly less than \c xBufferSize
 *    -# The function returns another error and \c *pxReceivedSize
 *       is not relevant.
 *    .
 *
 *  @see TRphReceiveFlags
 */
#define RPH_RECEIVE_WAIT        BIT0

/**
 *  @brief
 *    When calling \c rphReceive with this flag set, the function must
 *    immediately return with the bytes received in the internal buffer.
 *    In this case, if the function returns \c RPH_NO_ERROR, \c *pxReceivedSize
 *    may be equal to or less than \c xBufferSize, depending on the number
 *    of bytes currently in the internal buffer.
 *
 *  @see TRphReceiveFlags
 */
#define RPH_RECEIVE_NO_WAIT     BIT1

/**
 *  @brief
 *    When calling \c rphReceive with this flag set, if some bytes are in the
 *    internal buffer, the function must immediately return with these bytes;
 *    otherwise, it must wait until a packet is received or the timeout occurs.
 *    Three cases may occur:
 *    -# The function returns \c RPH_NO_ERROR and \c *pxReceivedSize is equal to
 *       or less than xBufferSize, depending on the size of the data that were
 *       in the internal buffer or that have been received in the first packet.
 *    -# The function returns \c RPH_ERROR_TIMEOUT and \c *pxReceivedSize is 0.
 *    -# The function returns another error and \c *pxReceivedSize is not
 *       relevant.
 *    .
 *
 *  @see TRphReceiveFlags
 */
#define RPH_RECEIVE_PACKET      BIT2


/******************************************************************************/
/*                                                                            */
/*                                  TYPES                                     */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Flags used to define the type of waiting in the \c rphReceive() function.
 *
 *    The flags \c RPH_RECEIVE_WAIT, \c RPH_RECEIVE_NO_WAIT and \c
 *    RPH_RECEIVE_PACKET are mutually exclusive and thus cannot be combined. If
 *    two or more of them are specified, the function \c rphReceive()
 *    shall return \c RPH_ERROR.
 */
typedef TUnsignedInt8      TRphReceiveFlags;


/**
 *  @brief
 *    This enumeration is the status returned by RPH functions.
*/
typedef enum
{
  RPH_NO_ERROR,
    /**< Success */

  RPH_ERROR_LOW_MEMORY,
    /**< The system has no enough free memory to perform the requested
     *    operation
     */

  RPH_ERROR_BAD_HANDLE,
    /**< The handle passed to the function is not valid */

  RPH_ERROR_BAD_PARAMETER,
    /**< Some parameter has an unsupported value */

  RPH_ERROR_TIMEOUT,
    /**< A timeout occurred */

  RPH_ERROR
    /**< Other error. This error status is returned if an unexpected error
     *   occurs in DALRPH implementation. On such an error CAK will try to
     *   cancel pending operations.
     *   (ex. if an operation fails because of an unexpected condition
     *    CAK will cancel that operation and any previous ones linked.)
     */
} TRphStatus;

/**
 *  @brief
 *    This enumeration is the status returned in the RPH reception callback.
*/
typedef enum
{
  RPH_RECEPTION_NO_ERROR,
    /**< Successful data reception */

  RPH_RECEPTION_TIMEOUT,
    /**< The timeout occurred */

  RPH_RECEPTION_DISCONNECTED,
    /**< The connection has been disposed */

  RPH_RECEPTION_ERROR
    /**< Other error. This error status is returned if an unexpected error
     *   occurs in DALRPH implementation. On such an error CAK will try to
     *   cancel pending operations.
     *   (ex. if an operation fails because of an unexpected condition
     *    CAK will cancel that operation and any previous ones linked.)
     */
} TRphReceptionStatus;

/**
 *  @brief
 *    This enumeration is the status returned in the RPH send callback.
*/
typedef enum
{
  RPH_SEND_NO_ERROR,
    /**< Successful data send */

  RPH_SEND_TIMEOUT,
    /**< The timeout occurred */

  RPH_SEND_DISCONNECTED,
    /**< The connection has been disposed */

  RPH_SEND_ERROR
    /**< Other error. This error status is returned if an unexpected error
     *   occurs in DALRPH implementation. On such an error CAK will try to
     *   cancel pending operations.
     *   (ex. if an operation fails because of an unexpected condition
     *    CAK will cancel that operation and any previous ones linked.)
     */
} TRphSendStatus;

/**
 *  @brief
 *    This enumeration is the status of a connection request given in
 *    the connection callback.
*/
typedef enum
{
  RPH_CONNECTED,
    /**< The function returned successfully */

  RPH_CONNECTION_LOCAL_FAILURE,
    /**< The connection failed due to a local error (resource busy,
     *   no dial tone, ...)
     */

  RPH_CONNECTION_REMOTE_FAILURE,
    /**< The connection failed due to a remote error (no answer,
     *   no carrier, ...)
     */

  RPH_ERROR_AUTHENTICATION_FAILED,
    /**< Authentication failed due to a bad username or password (for PPP
     *   connection type)
     */

  RPH_CONNECTION_ERROR
    /**< Other connection error. This error status is returned if an unexpected
     *   error occurs in DALRPH implementation. On such an error CAK will try
     *   to cancel pending operations.
     *   (ex. if an operation fails because of an unexpected condition
     *    CAK will cancel that operation and any previous ones linked.)
     */
} TRphConnectionStatus;


/**
 *  @brief
 *    Connection descriptor enumerated type. The connection type specifies the
 *    kind of connection used for a session.
*/
typedef enum
{
  RPH_CT_MODEM,
    /**< Telephone line modem for terminal-server connection
    */
  RPH_CT_TCPIP,
    /**< Internet TCP/IP connection
    */
  RPH_CT_PPP,
    /**< PPP (Point-to-Point Protocol) connection
    */
  RPH_CT_TCP_IP_V6,
    /**< Internet TCP/IP V6 connection
    */
  RPH_CT_UDP_IP,
    /**< UDP IP connection
    */
  RPH_CT_UDP_IP_V6,
    /**< UDP IP V6 connection
    */
  RPH_CT_LIST,
    /**< List of connection types
    */
  RPH_CT_CUSTOM = 100
    /**< Generic connection type different from the previous ones and used to
     *   represent exotic protocols used by custom projects to connect custom
     *   devices
    */
} TRphConnectionType;


/**
 *  @brief
 *    The telephone number is defined as a NULL-terminated string of
 *    numerical ASCII digits.
*/
typedef TChar TRphPhoneNumber[RPH_PHONE_NUMBER_STR_MAX_LEN + 1];


/**
 *  @brief
 *    PPP authentication username is defined as a NULL-terminated string.
*/
typedef TChar TRphPppUsername[RPH_PPP_USERNAME_STR_MAX_LEN + 1];


/**
 *  @brief
 *    PPP authentication password is defined as a NULL-terminated string.
*/
typedef TChar TRphPppPassword[RPH_PPP_PASSWORD_STR_MAX_LEN + 1];


/**
 *  @brief
 *    Connection descriptor used to define a \c RPH_CT_TCPIP or a \c
 *    RPH_CT_UDP_IP connection type.
*/
typedef struct
{
  TUnsignedInt8  address[4];
    /**< IP address
    */
  TUnsignedInt16 port;
    /**< IP port
    */
} TRphIp;

/**
 *  @brief
 *    Connection descriptor used to define a \c RPH_CT_TCPIP_V6 or a \c
 *    RPH_CT_UDP_IP_V6 connection type.
*/
typedef struct
{
  TUnsignedInt8  address[16];
    /**< IP address
    */
  TUnsignedInt16 port;
    /**< IP port
    */
} TRphIp6;


/**
 *  @brief
 *    Connection descriptor used to define a \c RPH_CT_PPP connection type.
*/
typedef struct
{
  TRphPhoneNumber phoneNumber;
    /**< Modem phone number
    */
  TRphIp  ip;
    /**< IP address and port
    */
  TRphPppUsername       username;
    /**< "Peer-ID" to be used for PAP authentication or "name" to be used for
     *   CHAP authentication
    */
  TRphPppPassword       password;
    /**< "Password" to be used for PAP authentication or "secret" to compute
     *   the hash value for CHAP authentication mode
    */
} TRphPpp;


/**
 *  @brief
 *    The Custom descriptor is used to define a \c RPH_CT_CUSTOM type
*/
typedef struct
{
  TUnsignedInt32 address;
    /**< Simple number defined per project to identify a unique resource to be
     *   addressed
    */
} TRphCustom;


/**
 *  @brief
 *    Connection list definition.
*/
typedef struct
{
  TUnsignedInt8          number;
    /**< Number of connection
    */
  struct SRphConnection* pConnectionArray;
    /**< Connection array
    */
} TRphConnectionList;


/**
 *  @brief
 *    Union defining the different connection descriptors
*/
union URphConnection
{
  TRphPhoneNumber  phoneNumber;
    /**< Modem phone number for connection type \c RPH_CT_MOD
    */
  TRphIp           ip;
    /**< IP descriptor for connection type \c RPH_CT_TCPIP and \c RPH_CT_UDP_IP
    */
  TRphIp6          ip6;
    /**< IP V6 descriptor for connection type \c RPH_CT_TCP_IP_V6 and \c
     *   RPH_CT_UDP_IP_V6
    */
  TRphPpp          ppp;
    /**< PPP descriptor for connection type \c RPH_CT_PPP
    */
  TRphCustom       custom;
    /**< Custom descriptor for connection type \c RPH_CT_CUSTOM
    */
  TRphConnectionList  list;
    /**<  Descriptor used to provide a list of connection descriptors. It is
     *    bound to the connection type RPH_CT_LIST.
    */
};


/**
 *  @brief
 *    Structure defining a connection
 */
struct SRphConnection
{
  TRphConnectionType        type;
    /**< Connection type
    */
  union URphConnection      descriptor;
    /**< Connection descriptor
    */
};

/**
 *  @brief
 *    Type defining a connection
 */
typedef struct SRphConnection TRphConnection;

/**
 *  @brief
 *    The session handle descriptor is totally defined by the implementation.
 *    The interface only sees an opaque pointer. Use \c
 *    RPH_SESSION_HANDLE_INVALID for invalid session identifiers.
 *
 *  @see RPH_SESSION_HANDLE_INVALID
 */
typedef struct SRphSessionHandle* TRphSessionHandle;


/**
 *  @brief
 *    This callback is provided by the CA software to allow the RPH notifying
 *    about the connection status. Before this callback is called, any failure
 *    xas considered as local failure. Once this callback entered any failure
 *    will be considered remote failure.
 *
 *  @pre
 *    The function \c rphConnect() has been called for this handle.
 *
 *  @param[in] xSessionHandle
 *               Session handle
 *
 *  @param[in] xStatus
 *               Status of the connection
 *
 *  @param[in] pxPrivateData
 *               Private data passed to the on connection request and given back
 *               with no processing done by the driver. It is usually used by
 *               the CA software for internal processing.
 */
typedef void (*TRphConnectionCallback)
(
        TRphSessionHandle    xSessionHandle,
        TRphConnectionStatus xStatus,
  const void*                pxPrivateData
);


/**
 *  @brief
 *    This callback is provided by the CA software to receive data from the RPH
 *    asynchronously.
 *
 *  @pre
 *    The function \c rphReceive() has been called for this handle.
 *
 *  @param[in] xSessionHandle
 *               Session handle
 *
 *  @param[in] xReceiveStatus
 *               Status of the data reception
 *
 *  @param[in] xDataSize
 *               Size of the data received by the RPH, in bytes, and transmitted
 *               through this call; shall be less than or equal to the size of
 *               the buffer \c pxData
 *
 *  @param[in] pxData
 *               Data buffer provided by the CA software and filled with \c
 *               xDataSize bytes of received data
*/
typedef void (*TRphReceptionCallback)
(
  TRphSessionHandle      xSessionHandle,
  TRphReceptionStatus    xStatus,
  TSize                  xDataSize,
  TUnsignedInt8*        pxData
);


/**
 *  @brief
 *    This callback is provided by the CA software to asynchronously send data
 *    to the RPH.
 *
 *  @pre
 *    The function \c rphSendA() has been called for this handle.
 *
 *  @param[in] xSessionHandle
 *               Session handle
 *
 *  @param[in] xStatus
 *               Status of the data sending
 *
 *  @param[in] xSentSize
 *               Number of bytes actually sent. Must be equal to \c xBufferSize
 *               used in \c rphSendA call if the function returns \c
 *               RPH_SEND_NO_ERROR. Must be less than \c xBufferSize if \c
 *               xStatus=RPH_SEND_TIMEOUT. Undefined in the other cases.
 *
 *  @remarks
 *    -# If the timeout occurs, the number of bytes correctly sent must be
 *       returned in \c xSentSize and \c xStatus=RPH_SEND_TIMEOUT.
 *    .
 *    -# If another error occurs (on \c RPH_SEND_DISCONNECTED or on \c RPH_SEND_ERROR
 *       ), the value returned by \c xSentSize is not used.
*/
typedef void (*TRphSendCallback)
(
  TRphSessionHandle   xSessionHandle,
  TRphSendStatus      xStatus,
  TSize                     xSentSize
);


/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    This function attempts to make a connection with a remote computer. It is
 *    asynchronous and must not block the calling task. This caters for systems
 *    where the connection is only available at discrete times, for instance
 *    when the set-top box connects through a PC-controlled modem that can only
 *    be used when the user of the PC has established a connection to the
 *    Internet.
 *
 *    To simplify the transition from the older synchronous \c rphConnect(), it
 *    is admitted that \c rphConnect() attempts to establish the connection
 *    synchronously and calls the callback from within the function's body,
 *    provided that the caller is not delayed for more than 30 seconds. New
 *    implementations must perform the connection asynchronously, i.e. it must
 *    return immediately and attempt to connect in background. As soon as the
 *    connection is established, data might be received, even if \c rphReceive()
 *    has not been called yet. These must not be lost and at least \c
 *    xInternalBufferSize bytes must be stored in an internal FIFO buffer. These
 *    will be returned to the forthcoming call to \c rphReceive(). Once this
 *    buffer is filled, it is acceptable that some bytes get lost.
 *
 *    Successful connection requests must be released by calling \c
 *    rphDisconnect(), even if the connection is not actually established.
 *
 *  @param[out] pxSessionHandle
 *                Pointer to the session handle; returned value is significant
 *                only when the function returns \c RPH_NO_ERROR
 *
 *  @param[in]  pxConnection
 *                Pointer to the descriptor of the connection
 *
 *  @param[in]  xInternalBufferSize
 *                Minimum internal receive buffer size in bytes that must be
 *                guarantied by the RPH driver
 *
 *  @param[in]  xConnectionCallback
 *                Callback used by the RPH to notify the CA software of the result
 *                of the connection attempt. It is called only if the function
 *                returns \c RPH_NO_ERROR.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the application for internal processing.
 *                This pointer is passed as a parameter to the callback function.
 *
 *  @retval RPH_NO_ERROR
 *            Connection succeded. In this case, the callback \c
 *            xConnectionCallback must be called.
 *
 *  @retval RPH_ERROR_LOW_MEMORY
 *            Memory allocation failed
 *
 *  @retval RPH_ERROR_BAD_PARAMETER
 *            Some parameter has an unsupported value
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *    -# If the function is called in PPP mode and another PPP connection type
 *       is already established to another PPP phone number, the function must
 *       call the \c xConnectionCallback callback with \c
 *       RPH_CONNECTION_LOCAL_FAILURE status.
 *    .
 *    -# For an IP connection, the IP stack already provides a reception buffer
 *       so that it is not necessary for the driver to manage its own internal
 *       buffer.
*/
TRphStatus rphConnect
(
        TRphSessionHandle*     pxSessionHandle,
        TRphConnection*        pxConnection,
        TSize                  xInternalBufferSize,
        TRphConnectionCallback xConnectionCallback,
  const void*                  pxPrivateData
);

/**
 *  @brief
 *    This function attempts to make a connection with a remote computer. It is
 *    similar to the function \c rphConnect(), with an additional parameter: a
 *    callback that may be used to asynchronously receive data from the RPH.
 *
 *    Refer to the description of \c rphConnect() for further explanations about
 *    this function.
 *
 *  @param[out] pxSessionHandle
 *                Pointer to the session handle; returned value is significant
                  only when the function returns \c RPH_NO_ERROR
 *
 *  @param[in]  pxConnection
 *                Descriptor of the connection
 *
 *  @param[in]  xInternalBufferSize
 *                Minimum internal receive buffer size in bytes that must be
 *                guarantied by the RPH driver
 *
 *  @param[in]  xConnectionCallback
 *                Callback used by the RPH to notify the CA software of the result
 *                of the connection attempt. It is called only if the function
 *                returns \c RPH_NO_ERROR.
 *
 *  @param[in]  xReceptionCallback
 *                Callback used by the CA software to asynchronously receive
 *                data from the RPH through the function \c rphReceiveA(). If
 *                set to NULL, the function \c rphReceiveA() shall return an
 *                error and only the synchronous reception shall be used.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the application for internal processing.
 *                This pointer is passed as a parameter to the callback
 *                function.
 *
 *  @retval RPH_NO_ERROR
 *            Success. In this case, the callback \c xConnectionCallback must be
 *            called.
 *
 *  @retval RPH_ERROR_LOW_MEMORY
 *            Memory allocation failed
 *
 *  @retval RPH_ERROR_BAD_PARAMETER
 *            Some parameter has an unsupported value
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *    -# If the function is called in PPP mode and another PPP connection type
 *       is already established to another PPP phone number, the function must
 *       call the \c xConnectionCallback callback with \c
 *       RPH_CONNECTION_LOCAL_FAILURE status.
 *    .
 *    -# For an IP connection, the IP stack already provides a reception buffer
 *       so that it is not necessary for the driver to manage its own internal
 *       buffer.
 */
TRphStatus rphConnectA
(
        TRphSessionHandle*     pxSessionHandle,
        TRphConnection*        pxConnection,
        TSize                   xInternalBufferSize,
        TRphConnectionCallback  xConnectionCallback,
        TRphReceptionCallback   xReceptionCallback,
  const void*                  pxPrivateData
);


/**
 *  @brief
 *    This function attempts to make a connection with a remote computer. It is
 *    similar to the function \c rphConnect(), with an additional parameter: a
 *    callback that may be used to asynchronously send data from the RPH.
 *
 *    Refer to the description of \c rphConnect() for further explanations about
 *    this function.
 *
 *  @param[out] pxSessionHandle
 *                Pointer to the session handle; returned value is significant
 *                only when the function returns \c RPH_NO_ERROR.
 *
 *  @param[in]  pxConnection
 *                Descriptor of the connection
 *
 *  @param[in]  xInternalBufferSize
 *                Minimum internal receive buffer size in bytes that must be
 *                guarantied by the RPH driver
 *
 *  @param[in]  xConnectionCallback
 *                Callback used by the RPH to notify the CA software of the result
 *                of the connection attempt. It is called only if the function
 *                returns \c RPH_NO_ERROR.
 *
 *  @param[in]  xSendCallback
 *                Callback used by the CA software to asynchronously send data
 *                from the RPH through the function \c rphSendA(). If set
 *                to NULL, the function \c rphSendA() shall return an error and
 *                only the synchronous send shall be used.
 *
 *  @param[in]  pxPrivateData
 *                Private data used by the application for internal processing.
 *                This pointer is passed as a parameter to the callback function.
 *
 *  @retval RPH_NO_ERROR
 *            Connection succeded
 *
 *  @retval RPH_ERROR_LOW_MEMORY
 *            Memory allocation failed
 *
 *  @retval RPH_ERROR_BAD_PARAMETER
 *            Some parameter has an unsupported value
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *    -# If the function is called in PPP mode and another PPP connection type
 *       is already established to another PPP phone number, the function must
 *       call the \c xConnectionCallback callback with \c
 *       RPH_CONNECTION_LOCAL_FAILURE status.
 *    .
 *    -# For an IP connection, the IP stack already provides a reception buffer
 *       so that it is not necessary for the driver to manage its own internal
 *       buffer.
 */
TRphStatus rphConnectAA
(
        TRphSessionHandle*       pxSessionHandle,
        TRphConnection*          pxConnection,
        TSize                     xInternalBufferSize,
        TRphConnectionCallback    xConnectionCallback,
        TRphSendCallback          xSendCallback,
  const void*                    pxPrivateData
);


/**
 *  @brief
 *    This function closes the connection with the remote computer (if any) and
 *    cancels the connection request. It must be called for all successful calls
 *    to \c rphConnect() or \c rphConnectA(), whether the connection was
 *    actually established or not.
 *
 *    The callback given in \c rphConnect() or \c rphConnectA() shall no longer
 *    be called after \c rphDisconnect() is completed.
 *
 *    If this function is called during an asynchronous data reception, the RPH
 *    shall call the reception callback with the reception status set to \c
 *    RPH_RECEPTION_DISCONNECTED.
 *
 *  @param[in] xSessionHandle
 *               Handle to the session to disconnect
 *
 *  @retval RPH_NO_ERROR
 *            Disonnection succeded
 *
 *  @retval RPH_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation.
 *            Cancelling pending operations have no meaning for a disconnect.
 *            CAK will NOT handle this through retry.
 *
 *  @remarks
 *    -# If this function is called during an asynchronous data reception,
 *       the RPH shall call the reception callback with the reception status
 *       set to \c RPH_RECEPTION_DISCONNECTED.
 */
TRphStatus rphDisconnect
(
  TRphSessionHandle  xSessionHandle
);


/**
 *  @brief
 *    This function gets a certain amount of bytes from a return path session.
 *    In \c RPH_RECEIVE_WAIT mode, if the internal buffer already contains the
 *    requested number of bytes, the function immediately returns with these
 *    bytes. Otherwise, it waits until the requested number of bytes is
 *    received or the timeout occurs.
 *
 *    In \c RPH_RECEIVE_PACKET mode, if the internal buffer already contains data,
 *    the function immediately returns with these bytes. Otherwise, it waits
 *    until the next packet is received or the timeout occurs.
 *
 *  @param[in]  xSessionHandle
 *                Handle to the current session
 *
 *  @param[out] pxData
 *                Pointer to the receive data buffer of \c xBufferSize bytes.
 *                This buffer is allocated by the application.
 *
 *  @param[in]  xBufferSize
 *                Size in bytes of the provided receive data buffer.
 *                If the flag \c RPH_RECEIVE_WAIT is set, the caller
 *                expects that this buffer will be completely filled.
 *
 *  @param[out] pxReceivedSize
 *                Number of bytes actually read
 *
 *  @param[in]  xTimeout
 *                Maximum time to wait for completion (in milliseconds). Only
 *                used if \c RPH_RECEIVE_WAIT respectively \c RPH_RECEIVE_PACKET
 *                is set. A zero value specifies that no time-out is required.
 *                In this case, the calling task must wait until it receives \c
 *                xBufferSize bytes respectively the next packet.
 *
 *  @param[in]  xFlags
 *                RPH receive flags:
 *                - \c RPH_RECEIVE_WAIT
 *                - \c RPH_RECEIVE_NO_WAIT
 *                - \c RPH_RECEIVE_PACKET
 *
 *  @retval RPH_NO_ERROR
 *            Data correctly received
 *
 *  @retval RPH_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid
 *
 *  @retval RPH_ERROR_TIMEOUT
 *            A timeout occurred during connection
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *    -# With the \c RPH_RECEIVE_WAIT flag, if the timeout occurs, the n bytes
 *       (with n < \c xBufferSize) received until the timeout must be copied
 *       into the reception buffer \c pxData and n must be returned in \c
 *       pxReceivedSize.
 *    .
 *    -# If another error occurs (\c RPH_ERROR_BAD_HANDLE or \c RPH_ERROR ), the
 *       other returned values are not used.
 */
TRphStatus rphReceive
(
  TRphSessionHandle xSessionHandle,
  TUnsignedInt8*    pxData,
  TSize             xBufferSize,
  TSize*            pxReceivedSize,
  TOsTime           xTimeout,
  TRphReceiveFlags  xFlags
);

/**
 *  @brief
 *    This function gets a certain amount of bytes from a return path session.
 *
 *    This function shall immediately return. The data shall be asynchronously
 *    sent through the reception callback provided in the call to \c
 *    rphConnectA(). To each successful call to the function \c rphReceiveA()
 *    shall correspond exactly 1 call to the reception callback. If the function
 *    \c rphDisconnect() is called during the reception, the callback shall
 *    still be called with the status \c RPH_RECEIVE_DISCONNECTED.
 *
 *    In \c RPH_RECEIVE_WAIT mode, if the internal buffer already contains the
 *    requested number of bytes, the callback is immediately called with these
 *    bytes; otherwise, it shall be called when the requested number of bytes is
 *    received or the timeout occurs.
 *
 *    In \c RPH_RECEIVE_PACKET mode, if the internal buffer already contains
 *    data, the callback is immediately called with these bytes; otherwise, it
 *    shall be called when the next packet is received or the timeout occurs.
 *
 *    If the connection has been established through the function \c
 *    rphConnect() or if no reception callback has been specified in \c
 *    rphConnectA(), this function shall return \c RPH_ERROR.
 *
 *  @pre
 *    The handle has been obtained from a successful call to the function
 *    \c rphConnectA().
 *
 *  @post
 *    The reception callback is called exactly once.
 *
 *  @param[in] xSessionHandle
 *               Handle to the current session
 *
 *  @param[in] pxData
 *               Receive data buffer of \c xBufferSize bytes allocated by the CA
 *               software.
 *
 *  @param[in] xBufferSize
 *               Size in bytes of the provided receive data buffer.
 *               If the flag \c RPH_RECEIVE_WAIT is set, the caller
 *               expects that this buffer will be completely filled.
 *
 *  @param[in] xTimeout
 *               Maximum time to wait for completion (in milliseconds). Only
 *               used if \c RPH_RECEIVE_WAIT respectively \c RPH_RECEIVE_PACKET
 *               is set. A zero value specifies that no time-out is required. In
 *               this case, the callback must be called when it receives \c
 *               xBufferSize bytes respectively the next packet.
 *
 *  @param[in] xFlags
 *                RPH receive flags:
 *                - \c RPH_RECEIVE_WAIT
 *                - \c RPH_RECEIVE_NO_WAIT
 *                - \c RPH_RECEIVE_PACKET
 *
 *  @retval RPH_NO_ERROR
 *            Successful call. The callback will be called.
 *
 *  @retval RPH_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *    -# With the \c RPH_RECEIVE_WAIT flag, if the timeout occurs, the n bytes
 *       (with n < \c xBufferSize) received until the timeout must be copied
 *       into the reception buffer \c pxData and n must be returned in the
 *       parameter \c xDataSize of the reception callback.
 */
TRphStatus rphReceiveA
(
  TRphSessionHandle   xSessionHandle,
  TUnsignedInt8*     pxData,
  TSize               xBufferSize,
  TOsTime             xTimeout,
  TRphReceiveFlags    xFlags
);


/**
 *  @brief
 *    This function sends a certain amount of bytes to a return path session.
 *    This call will always wait for the time specified in the time-out value
 *    for completion.
 *
 *    This function shall not be used for a UDP connection type.
 *
 *  @param[in]  xSessionHandle
 *                Handle to the current session
 *
 *  @param[in]   pxData
 *                 Send data buffer allocated by the CA software
 *
 *  @param[in]  xBufferSize
 *                Size in bytes of the send data buffer
 *
 *  @param[out] pxSentSize
 *                Number of bytes actually sent. Must be equal to \c xSize if
 *                the function returns \c RPH_NO_ERROR. Must be less than xSize
 *                if the function returns \c RPH_ERROR_TIMEOUT. Undefined in the
 *                other cases.
 *
 *  @param[in]  xTimeout
 *                Maximum time to wait for completion (in milliseconds). A zero
 *                value specifies that no time-out is required. In this case,
 *                the calling task must wait until all bytes are sent.
 *
 *  @retval RPH_NO_ERROR
 *            Data correctly sent
 *
 *  @retval RPH_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid
 *
 *  @retval RPH_ERROR_TIMEOUT
 *            A timeout occurred during connection
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 *
 *  @remarks
 *  -# If the timeout occurs, the number of bytes correctly sent must be
 *     returned in \c pxSentSize.
 *  .
 *  -# If another error occurs (\c RPH_ERROR_BAD_HANDLE or \c RPH_ERROR ), the
 *     value returned by \c pxSentSize is not used.
 *  .
 *  -# During a modem connection, once the data are sent, the driver must
 *     immediately toggle to reception mode and store the incoming data in its
 *     internal buffer.
 *
 */
TRphStatus rphSend
(
        TRphSessionHandle  xSessionHandle,
  const TUnsignedInt8*    pxData,
        TSize              xBufferSize,
        TSize*            pxSentSize,
        TOsTime            xTimeout
);

/**
 *  @brief
 *    This function sends a certain amount of bytes to a return path session.
 *
 *    This function is asynchronous and must return immediately. The send
 *    process result shall be asynchronously sent through the reception callback
 *    provided in the call to \c rphConnectAA().  To each successful call to the
 *    function \c rphSendA() shall correspond exactly 1 call to the send callback.
 *    If the function \c rphDisconnect() is called during the sending, the callback
 *    shall still be called with the status \c RPH_SEND_DISCONNECTED.
 *
 *    This function shall not be used for a UDP connection type.
 *
 *  @param[in]  xSessionHandle
 *                Handle to the current session
 *
 *  @param[in]  pxData
 *                Send data buffer
 *
 *  @param[in]  xBufferSize
 *                Size in bytes of the send data buffer
 *
 *  @param[in]  xTimeout
 *                Maximum time to wait for completion (in milliseconds). A zero
 *                value specifies that no time-out is required. In this case, the
 *                calling task must wait until all bytes are sent.
 *
 *  @retval RPH_NO_ERROR
 *            No problem occurs
 *
 *  @retval RPH_ERROR
 *            Other error. This error status is returned if an unexpected error
 *            occurs in DALRPH implementation. On such an error CAK will try to
 *            cancel pending operations.
 *            (ex. if an operation fails because of an unexpected condition
 *             CAK will cancel that operation and any previous ones linked.)
 *            CAK will handle this by trigerring 'LOCAL FAILURE' retry.
 */
TRphStatus rphSendA
(
        TRphSessionHandle   xSessionHandle,
  const TUnsignedInt8*     pxData,
        TSize               xBufferSize,
        TOsTime             xTimeout
);

/**
 *  @brief
 *    This function is customer-specific and shall be implemented upon request
 *    only.
 *
 *    It provides the RPH with the connection metadata given by the STB
 *    application in order to apply specific connection rules.
 *
 *    This function is systematically called right after rphConnect(), even in
 *    absence of metadata. In such a case \c xSizeOfMetadata is set to 0 and
 *    \c pxMetadata is set to \c NULL.
 *
 *  @pre
 *    The function rphConnect() has been called for this handle.
 *
 *  @param[in]  xSessionHandle
 *                Handle to the current session
 *
 *  @param[in]  xSizeOfMetadata
 *                Size in bytes of the metadata buffer
 *
 *  @param[in]  pxMetadata
 *                Buffer containing the connnection metadata
 *
 *  @retval RPH_NO_ERROR
 *            No problem occurs
 *
 *  @retval RPH_ERROR
 *            Other error
 */
TRphStatus rphSetMetadata
(
        TRphSessionHandle   xSessionHandle,
        TSize               xSizeOfMetadata,
  const TUnsignedInt8*     pxMetadata
);


#endif /* NAGRA_CA_V5 */

#endif /* CA_RPH_H */


/* END OF FILE */
