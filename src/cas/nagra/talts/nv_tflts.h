/**
  @file nv_tflts.h

  @brief
  Defines the Nagra trusted framework layers session interface related to the
  rich execution environment.

  @details
  This interface is used by the Nagra rich client running in the rich
  execution environment. It allows it communicating with the Nagra trusted
  client running on the trusted execution environment.

  @note
  Trusted environment technology is dependent on the platform definition
  making it available. The communication technology and layer between rich
  and trusted environments is dependent of this platform technology.
  The preliminary steps and processes required to establish this
  communication path are in charge of the platform implementation.

  COPYRIGHT:
    2013 - 2016 Nagravision S.A.
*/

/*
   ==========================================================================
   IMPORTANT REMARK :
   ==========================================================================

   Comments in this file use special tags to allow automatic API
   documentation generation in HTML format, using the GNU-General Public
   Licensed Doxygen tool.
   For more information about Doxygen, please check www.doxygen.org

   Depending on the platform, the CHM file may not open properly if it is
   stored on a network drive. So either the file should be moved on a local
   drive or add the following registry entry on Windows platform (regedit):
   [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003

   ==========================================================================
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_tfl_ts
  @brief
  Describe the Nagra trusted session interface of the Nagra clients.

  @details

  The Nagra <b>Trusted Session</b> interface introduces definitions for
  managing a communication session with a trusted client isolated within a
  trusted execution environment. Please make sure to have read the
  documentation pages for a complete description of the interface constraints
  and requirements.

  @see @ref p_tfl_invoke "trusted client invocation" description.
*/

#ifndef NV_TFLTS_H
#define NV_TFLTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_tfl.h"

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_tfl_ts
  @brief
  This structure defines the Nagra trusted session interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    Nagra trusted session interface version number.
    @details
    Assign it to the ::TFLAPI_VERSION_INT result.
  */

  uint32_t (*open)
  (
    void**   ppSession,
    uint32_t  select
  );
  /**<
    @brief
    This function opens a session with a trusted client.

    It allows the Nagra rich client executing in the rich environment to open
    a session on the Nagra trusted client executing in the trusted environment.

    When the Nagra rich client calls this function, it must be blocked while
    the trusted execution environment is requested to open a session on the
    Nagra trusted client. The latter is called using
    nvTrustedSessionOpenEntry().
    Once the trusted operation has been processed, the Nagra rich client
    caller must be resumed with the results of the operation.

    When successful, this operation must provide a valid session handle; it
    must be used to identify the trusted session in all subsequent related
    operations. It is defined by the implementation of the function described
    here. The value returned by the trusted operation must be provided back
    as the returned value of this rich function call.

    @pre
    The trusted execution environment must be up and running.
    The rich execution environment must have set up the communication with the
    trusted execution environment.

    @post
    The Nagra trusted client has been called using nvTrustedSessionOpenEntry().
    Its result is provided back as the return value of this operation.
    A valid handle is set in *ppSession after successful processing or a
    @c NULL value is set if the session opening failed.

    @param[out] ppSession
    Reference on the trusted session handle.

    @param[in]    select
    Internal selection parameter.

    @return
    The operation mainly returns the value returned by the distant trusted
    operation; ::NV_TRUSTED_SUCCESS means the session with the trusted client
    is successfully opened. An error means the session has not been opened.
    Check ::TNvTrustedResult for a list of the possible returned values.
    Check also nvTrustedSessionOpenEntry() description for the meaning of the
    trusted session open operation return values.
    However error codes can also be raised directly by the rich execution
    execution environment implementation or by the trusted framework
    independently of the trusted client operation result.
    These independent errors are detailed in the following return values.

    @retval ::NV_TRUSTED_ERROR_CLIENT_MEMORY
    The client environment runs out of memory.

    @retval ::NV_TRUSTED_ERROR_TRUSTED_MEMORY
    The trusted environment runs out of memory.

    @retval ::NV_TRUSTED_ERROR_COMMUNICATION
    The client environment failed to communicate with the trusted client.
    It can result from the following conditions but not limited to this list:
    + The rich execution environment failed to reach the trusted execution
      environment.
    + The trusted client does not exist.
    + The trusted client is busy.
    + The trusted client denied the access to the rich client.

    @retval ::NV_TRUSTED_ERROR_BAD_PARAMETER
    The ppSession parameter is @c NULL.

    @retval ::NV_TRUSTED_ERROR
    An unexpected error occurs.

    @note
    The session handle returned by this function is not the session	context or
    handle defined by the trusted operation within the trusted execution
    environment. It must allow the implementation to identify the trusted
    session to address.

    @note
    When the trusted session resource is no more needed, it must be closed
    using ::INvTrustedSession::close().

    @see ::TNvTrustedResult, ::INvTrustedSession::close(),
         ::INvTrustedSession::command(), nvTrustedSessionOpenEntry(),
         @ref p_tfl_invoke "Trusted client invocation".
  */

  void (*close)
  (
    void* pSession
  );
  /**<
    @brief
    This function closes a session with a trusted client.

    It allows the Nagra rich client executing in the rich execution
    environment to close a session previously opened on the Nagra trusted
    client executing in the trusted execution environment. A trusted session
    must always be closed when no more needed.

    When the Nagra rich client calls this function, it must be blocked while
    the trusted execution environment is requested to close a trusted session
    associated to the Nagra trusted client. The latter must be called using
    ::nvTrustedSessionCloseEntry. Once the trusted operation has been
    processed, the Nagra rich client caller must be resumed.

    When the operation finally returns, the trusted session handle is no more
    valid; it must no more be used with any trusted operation call.

    @pre
    A valid trusted session must have been previously opened successfully
    using ::INvTrustedSession::open().

    @post
    The trusted client has been called using nvTrustedSessionCloseEntry().
    The trusted session handle is no more valid.

    @param[in] pSession
    Trusted session handle.

    @see ::TNvTrustedResult, ::INvTrustedSession::open(),
         ::INvTrustedSession::command(), nvTrustedSessionCloseEntry(),
         @ref p_tfl_invoke "Trusted client invocation".
  */


  uint32_t (*command)
  (
    void*            pSession,
    uint32_t         nBlocks,
    TNvTrustedBlock* pBlocks
  );
  /**<
    @brief
    This function process an operation with a trusted client.

    It allows the Nagra rich client executing in the rich execution
    environment to process an operation with the Nagra trusted client
    associated to the a trusted session. The latter must have been previously
    opened successfully using ::INvTrustedSession::open().

    When the Nagra rich client calls this function, it must be blocked. The
    memory blocks parameters must be provided to the trusted execution
    environment; If memory blocks cannot be remapped in the trusted execution
    environment and must be exchanged by copy, only the input and
    input/output memory blocks should be exchanged and temporary trusted
    output memory blocks should be allocated locally with the right size. The
    Nagra trusted client associated to the trusted session must be called
    using nvTrustedSessionCommandEntry(). Once the trusted operation has been
    processed, output data must be provided back to the rich execution
    environment as well as the returned value. If rich memory blocks have not
    been directly remapped, output data must be copied back to rich memory
    blocks and their size adjusted down consequently; local trusted output
    memory blocks must be freed. The rich caller can therefore be resumed with
    updated results.

    Refer to ::TNvTrustedBlock and related definitions for a detailed description
    of the memory blocks.

    @pre
    A valid trusted session must have been previously opened successfully using
    ::INvTrustedSession::open().

    @post
    The trusted client has been called with nvTrustedSessionCommandEntry().
    The memory block parameters are updated according to their direction.

    @param[in]    pSession
    Trusted session handle.

    @param[in]    nBlocks
    Number of memory blocks provided in the @a pBlocks parameter.
    This number cannot be @c 0.

    @param[inout] pBlocks
    An array of ::TNvTrustedBlock; the number of blocks is provided with the
    @a nBlocks parameter. It cannot be @c NULL.

    @return
    The operation mainly returns the value returned by the distant trusted
    operation; ::NV_TRUSTED_SUCCESS means the command has been successfully
    processed by the trusted client. An error means the operation has
    failed. Memory blocks provided must still be updated according to their
    direction and content. Refer to @ref s_tfl_invoke_outputs section for
    further details. Check ::TNvTrustedResult for a list of the possible
    returned values. Check also nvTrustedSessionCommandEntry() description
    for the meaning of the trusted session command operation return values.
    However error codes can also be raised directly by the rich execution
    environment implementation or by the trusted framework independently of
    the trusted client operation result. These independent errors are detailed
    in following the return values.

    @retval ::NV_TRUSTED_ERROR_CLIENT_MEMORY
    The rich execution environment runs out of memory.

    @retval ::NV_TRUSTED_ERROR_TRUSTED_MEMORY
    The trusted execution environment runs out of memory.

    @retval ::NV_TRUSTED_ERROR_COMMUNICATION
    The rich execution environment failed to communicate with the trusted
    execution environment.

    @retval ::NV_TRUSTED_ERROR_BAD_PARAMETER
    A parameter is invalid or inconsistent:
    + The trusted session handle is invalid.
    + The number of memory blocks is @c 0.
    + The address of memory block array is @c NULL.
    + One of the memory block provided is invalid.
      i.e. its @a size field is @c NULL or its @a pAddr field is NULL.

    @retval ::NV_TRUSTED_ERROR_BLOCK_TOO_SHORT
    An output block is too short to hold the expected result.
    Refer to @ref s_tfl_invoke_outputs section for further details.

    @retval ::NV_TRUSTED_ERROR
    An unexpected error occurs.

    @see ::TNvTrustedResult, ::INvTrustedSession::open(),
         ::INvTrustedSession::close(), nvTrustedSessionCommandEntry(),
         @ref p_tfl_invoke "Trusted client invocation".

    @note
    All memory block allocations are managed by the Nagra rich client.
    When a memory block is provided, it must refer a valid memory space whether
    it is provided as input and/or output. Refer to @ref s_tfl_invoke_outputs section for
    further details. Memory blocks may be remapped directly in the trusted
    execution environment memory space if this feature is available or
    transferred back and forth by copy according to their direction; in that
    case local trusted temporary output memory blocks should be used.
  */

} INvTrustedSession;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_tfl_ts
  @brief
  Provide the Nagra trusted session interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Nagra trusted session interface structure must
  remain accessible as long as the Nagra rich client is running.

  @details
  This function is used by the Nagra rich client to retrieve the trusted
  session interface structure.

  This function should be called once during Nagra rich client initialization
  but it cannot definitively be assumed. Therefore the address of the structure
  and the memory allocated to it must be valid as long as the Nagra rich
  client is loaded and running.

  @return
  A constant pointer to the Nagra trusted session interface structure.

  @see ::INvTrustedSession, @ref p_tfl_invoke "Trusted client invocation".
*/

const INvTrustedSession* nvGetTrustedSessionInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_TFLTS_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
