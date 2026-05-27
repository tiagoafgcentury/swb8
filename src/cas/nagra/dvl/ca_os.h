/**
**  @file ca_os.h
**
**  @brief
**    This file describes the NagraVision operating system API.
**
**  @ingroup  OSY
**
**  $Id: //CAK/components/dalapi/TAGS/DALAPI_7_17_3/src/com/osy/ca_os.h#1 $
**  $Change: 297498 $
**
**  COPYRIGHT:
**   2001-2008 Nagravision S.A.
*/

/**
 *  @defgroup OSY Operating System
 *  @brief
 *    This file describes the OS functions to be implemented in order to
 *    integrate the NagraVision CA applications. All functions of this API may
 *    be concurrently called from different task contexts (multi-tasking OS).
 *    They must therefore be re-entrant. On the other hand, osTaskIdentify() and
 *    osTimerCreate() are the only functions of this API that cannot be called
 *    from any task context, but only in contexts created by osTaskCreate().
 *
*/

#ifndef CA_OS_H
#define CA_OS_H

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#ifdef OSY_ANY_WRAP
#include "osywrap.h"
#endif /* OSY_ANY_WRAP */

#include "ca_defs.h"

/******************************************************************************/
/*                                                                            */
/*                               OS API TYPES                                 */
/*                                                                            */
/******************************************************************************/

#ifndef OSY_NO_TYPES

               /* -------------------------------------------- */
               /*                 General types                */
               /* -------------------------------------------- */

/**
 *  @brief
 *    This type defines the status returned by OSY functions.
 *
*/
typedef enum
{
  OS_NO_ERROR,
    /**< The function call succeeded
    */
  OS_ERROR
    /**< The function call failed
    */
} TOsStatus;


/**
 *  @brief
 *    Type used to express a number of items. For instance, number of semaphore
 *    tokens.
*/
typedef TUnsignedInt16 TOsCount;


/**
 *  @brief
 *    Type used to specify the OS time (waiting time, timer
 *    trigger, etc ...), it is expressed in milliseconds.
*/
typedef TUnsignedInt32 TOsTime;




               /* -------------------------------------------- */
               /*                   Task types                 */
               /* -------------------------------------------- */


/**
 *  @brief
 *    Type used to specify the handle of a task created by \c osTaskCreate().
 *    This handle consists in a pointer to a private structure to be defined in
 *    the implementation of the OSY layer.
 *
 *    The CA software never dereferences this handle. In other words, it never
 *    looks at what is pointed to by the handle. It is passed as is to other
 *    task management functions.
 *
 *  @see osTaskCreate, osTaskIdentify, osTaskWait, osTaskDelete, osEventSend
*/
typedef struct SOsTaskId *TOsTaskId;

/**
 *  @brief
 *    Type used to specify the entry point of a task created by \c
 *    osTaskCreate().
 *
 *  @see osTaskCreate
*/
typedef void (*TOsEntryPoint)(void);

/**
 *  @brief
 *    Type used to specify the size of a task created by \c osTaskCreate(). The
 *    size is expressed in bytes.
 *
 *  @see osTaskCreate
*/
typedef TUnsignedInt32 TOsStackSize;


/**
 *  @brief
 *    Type defining the different priority levels that can be assigned to a
 *    task created by \c osTaskCreate(). The OSY has to map these levels onto
 *    real task priorities of the underlying OS.
 *
 *  @see osTaskCreate
*/
typedef enum
{
  OS_PRIORITY_VERY_LOW,
    /**< Very low priority level
    */
  OS_PRIORITY_LOW,
    /**< Low priority level
    */
  OS_PRIORITY_NORMAL,
    /**< Normal priority level
    */
  OS_PRIORITY_HIGH,
    /**< High priority level
    */
  OS_PRIORITY_VERY_HIGH
    /**< Very high priority level
    */
} TOsPriority;


               /* -------------------------------------------- */
               /*                   Event types                */
               /* -------------------------------------------- */


/**
 *  @brief
 *    Type used to define a bitmap OS events. Each task is assigned one set of
 *    16 events.
 *
 *  @see osEventAllocate, osEventFree, osEventReceive, osEventSend,
 *       osTimerCreate
*/
typedef TUnsignedInt16 TOsEvent;


/**
 *  @brief
 *    Type defining the flags of an OS event.
 *
 *  @see osEventReceive
*/
typedef enum
{
  OS_EVENT_NO_WAIT,
    /**< If this flag is specified, the calling task shall enter the sleeping
    *    mode to wait for the synchronization condition until time-out
    *    expiration.
    */
  OS_EVENT_WAIT
    /**< If this flag is specified, this function shall returns immediately with
    *    the current active events of the expected event set.
    */
} TOsEventFlags;




               /* -------------------------------------------- */
               /*                 Semaphore types              */
               /* -------------------------------------------- */


/**
 *  @brief
 *    Type used to specify the handle of a semaphore created by
 *    \c osSemaphoreCreate(). This handle consists in a pointer to a private
 *    structure to be defined in the implementation of the OSY layer.
 *
 *    The CA software never dereferences this handle. In other words, it never
 *    looks at what is pointed to by the handle. It is passed as is to other
 *    semaphore management functions.
 *
 *  @see osSemaphoreCreate, osSemaphoreGet, osSemaphoreRelease,
 *    osSemaphoreDelete
*/
typedef struct SOsSemaphoreId *TOsSemaphoreId;




               /* -------------------------------------------- */
               /*                   Timer types                */
               /* -------------------------------------------- */


/**
 *  @brief
 *    Type used to specify the handle of a timer created by \c osTimerCreate().
 *    This handle consists in a pointer to a private structure to be defined in
 *    the implementation of the OSY layer.
 *
 *    The CA software never dereferences this handle. In other words, it never
 *    looks at what is pointed to by the handle. It is passed as is to other
 *    timer management functions.
 *
 *  @see osTimerCreate, osTimerStart, osTimerStop, osTimerDelete
*/
typedef struct SOsTimerId *TOsTimerId;

/**
 *  @brief
 *    Type defining the modes of an OS timer.
 *
 *  @see osTimerCreate
*/
typedef enum
{
  OS_TIMER_ONE_SHOT,
    /**< A one-shot timer fires once only. Upon expiration related OS events are
    *    sent to the parent task and the timer is no longer running.
    */
  OS_TIMER_CONTINUOUS
    /**< A continous timer fires periodically. It is automatically re-started
    *    for a new occurrence.
    */
} TOsTimerMode;




               /* -------------------------------------------- */
               /*            Persistent memory types           */
               /* -------------------------------------------- */
/**
 *  @brief
 *    Deprecated. These attributes are no longer used. Definition is kept
 *    for backward compatibility.
 *
 *  @deprecated
*/
typedef TUnsignedInt8  TOsPersistentMemoryFlags;

#define OS_PERSISTENT_MEMORY_WRITE_ONCE  BIT0
  /**< Deprecated
   *   @see TOsPersistentMemoryFlags
   *   @deprecated
  **/
#define OS_PERSISTENT_MEMORY_LOCKABLE    BIT1
  /**< Deprecated
   *   @see TOsPersistentMemoryFlags
   *   @deprecated
  */
#define OS_PERSISTENT_MEMORY_LOCKED      BIT2
  /**< Deprecated
   *   @see TOsPersistentMemoryFlags
   *   @deprecated
  */

/**
 *  @brief
 *    Type defining the different types of persistent memories.
 *
 *  @see TOsPersistentMemoryStorage
*/
typedef enum
{
  OS_PERSISTENT_MEMORY_NVRAM,
  /**< Non-volatile random access memory
  */
  OS_PERSISTENT_MEMORY_EEPROM,
  /**< Electrically erasable read-only memory or any other EPROM type
  */
  OS_PERSISTENT_MEMORY_FLASH,
  /**< Flash
  */
  OS_PERSISTENT_MEMORY_OTP,
  /**< One-time programmable memory
  */
  OS_PERSISTENT_MEMORY_OTHER,
  /**< Other type of persistent memory
  */
  OS_PERSISTENT_NUM_MEMORIES
  /**< Number of types. Internal use only.
  */
} TOsPersistentMemoryType;

/**
 *  @brief
 *    Type used to specify the handle of a persistent memory storage. This
 *    handle consists in a pointer to a private structure to be defined in
 *    the implementation of the OSY layer.
 *
 *    The CA software never dereferences this handle. In other words, it never
 *    looks at what is pointed to by the handle. It is passed as is to other
 *    persistent memory management functions.
 *
 *  @see TOsPersistentMemoryStorage, osPersistentMemoryRead,
 *    osPersistentMemoryWrite, osPersistentMemoryGetStorageUsage
*/
typedef struct SOsPersistentStorageId *TOsPersistentStorageId;

/**
 *  @brief
 *    Type defining the attributes of a persistent memory storage.
 *
 *  @see osPersistentMemoryGetStorageTable
*/
typedef struct
{
  TOsPersistentStorageId        storageId;
  /**< ID of this storage
  */
  TOsPersistentMemoryType       type;
  /**< Type of persistent memory
  */
  TOsPersistentMemoryFlags      flags;
  /**< Deprecated
  *    @deprecated
  */
  TSize                         size;
  /**< Available size, in bytes
  */
} TOsPersistentMemoryStorage;

/**
 *  @brief
 *    Gets the usage of a persistent memory storage.
 *
 *    Refer to the system requirements of the CA software module concerned for
 *    further information about specific requirements that shall be fulfilled by
 *    a storage of a given usage.
*/
typedef enum
{
  OS_PERSISTENT_MEMORY_USAGE_SECURITY,
  /**< Security usage
  */
  OS_PERSISTENT_MEMORY_USAGE_STANDARD,
  /**< Standard usage
  */
  OS_PERSISTENT_MEMORY_USAGE_DVL,
  /**< DVL usage
  */
  OS_PERSISTENT_MEMORY_USAGE_CSC,
  /**< Storage containing the Chipset Secure Channel (CSC) data record
   *   programmed in the STB OTP memory. Obviously this storage is accessible in
   *   reading mode only.
  */
  OS_PERSISTENT_MEMORY_USAGE_IRD_PARAMS,
  /**< Read-only storage containing IRD parameters (HW version, boot version,
   *   etc).
  */
  OS_PERSISTENT_MEMORY_USAGE_PAIRING_DATA,
  /**< Read-only storage containing pairing data. SSA signature and optional
    *  CRC must not be included in this storage.
  */
  OS_PERSISTENT_MEMORY_NUM_USAGES
  /**< Number of usages. Internal use only.
  */
} TOsPersistentMemoryUsage;

#endif /* OSY_NO_TYPES */


/******************************************************************************/
/*                                                                            */
/*                              OS API PROTOTYPES                             */
/*                                                                            */
/******************************************************************************/


               /* -------------------------------------------- */
               /*      Task management function prototypes     */
               /* -------------------------------------------- */

/**
 *  @brief
 *    This function creates a new task and starts it at the entry point given
 *    by the caller.
 *
 *    Time-slice of pre-emption of the underlying OS is not formally required as
 *    long as the task created is regularly given CPU time.
 *
 *  @param[in] xEntryPoint
 *               Task main function entry point called by the OSY right after
 *               the task is created.
 *
 *  @param[in] xStackSize
 *               Stack size in bytes.
 *
 *  @param[in] xPriority
 *               Task's priority. This one is not the real OS scheduling
 *               priority, but a NagraVision priority level for the CA
 *               tasks among themselves.\n
 *               The OSY layer is responsible of mapping the priority according
 *               to all other applications.
 *
 *  @param[out] pxTaskId
 *                Task identifier assigned to the task and returned by this
 *                function. It must be unique for each task created.
 *
 *  @retval  OS_NO_ERROR
 *             The task could be created with success
 *
 *  @retval  OS_ERROR
 *              The creation of the task failed
 *
 *  @remarks
 *    -# If task creation fails, the returned task identifier
 *       shall be set to \c NULL
 *    .
 *    -# Trying to create a task with a stack size set to 0 shall return
 *       \c OS_ERROR
*/

NAGRA_CA_API TOsStatus osTaskCreate
(
  TOsEntryPoint     xEntryPoint,
  TOsStackSize      xStackSize,
  TOsPriority       xPriority,
  TOsTaskId*        pxTaskId
);




/**
 *  @brief
 *    This function returns the task identifier of the calling task. This is a
 *    non-blocking function.
 *
 *  @param[in] pxTaskId
 *               Task identifier of the calling task.
 *
 *  @retval  OS_NO_ERROR
 *              The calling task could be identified
 *
 *  @retval  OS_ERROR
 *              The task identification failed
 *
 *  @remarks
 *    -# This function has to return \c OS_ERROR if it is called from a task
 *       context that has not been created by \c osTaskCreate().
*/
NAGRA_CA_API TOsStatus osTaskIdentify
(
  TOsTaskId*        pxTaskId
);




/**
 *  @brief
 *    Upon this function call, the calling task shall enter the sleeping mode
 *    for the duration specified by \c xTime. This is a blocking function.
 *
 *  @param[in] xTime
 *               Waiting time. This value is expressed in milliseconds.
 *
 *  @retval  OS_NO_ERROR
 *              The calling task entered the sleeping mode successfully
 *
 *  @retval  OS_ERROR
 *              An error occurred
 *
 *  @remarks
 *    -# The waiting time equal to 0 should not be used. However, if \c xTime is
 *       set to 0, the function shall return \c OS_NO_ERROR at once without
 *       entering the sleeping state.
 *    .
 *    -# If a granularity of one millisecond cannot be achieved, \c xTime shall
 *       be rounded to the closer upper value.
*/
NAGRA_CA_API TOsStatus osTaskWait
(
  TOsTime           xTime
);




/**
 *  @brief
 *    This function deletes the task specified.
 *
 *  @param[in] xTaskId
 *               Task identifier of the task to be deleted.
 *
 *  @retval  OS_NO_ERROR
 *              The task could be deleted.
 *
 *  @retval  OS_ERROR
 *              The task deletion failed.
 *
 *  @remarks
 *    -# NagraVision tries as far as possible to ensure that this function will
 *       be called after the task specified has performed its cleanup and is
 *       ready to be deleted. This means that, most of the times, the main
 *       function of the task to be deleted should have returned when \c
 *       osTaskDelete() is called. However, depending on task priorities or OS
 *       rescheduling, the task may still be running. For this reason,
 *       NagraVision advises \c osTaskDelete() to wait for a little time in
 *       order for the task to terminate. Waiting more than 10 milliseconds
 *       should cause this function to return \c OS_ERROR (task still running
 *       after 10 ms).
*/
NAGRA_CA_API TOsStatus osTaskDelete
(
  TOsTaskId         xTaskId
);




               /* -------------------------------------------- */
               /*     Event management function prototypes     */
               /* -------------------------------------------- */


/**
 *  @brief
 *    Event management is the basic mechanism that allows the different tasks
 *    to synchronize themselves. Each task has its own independent set of
 *    events. Each event is 1 bit encoded. An event is active when its bit
 *    is set to 1. Events are neither queued nor counted. Synchronization
 *    occurs as soon as a waiting task receives one event of its expected event
 *    set.
 *
 *    The function \c osEventAllocate() allocates one event from the set of
 *    events that are still not allocated to the calling task.
 *
 *  @param[out] pxEvent
 *                Event allocated by the OSY and returned to the caller. All
 *                 bits of \c pxEvent are set to 0, except the bit of the event
 *                 allocated that is set to 1.
 *
 *  @retval  OS_NO_ERROR
 *                The event could be allocated successfully
 *
 *  @retval  OS_ERROR
 *                The event allocation failed
 *
 *  @remarks
 *    -# This function shall return \c OS_ERROR if all events are already
 *       allocated.
 *    .
 *    -# The event returned remains allocated until it is explicitly freed by a
 *       call to \c osEventFree.
 *
 *  @see osEventFree()
*/

NAGRA_CA_API TOsStatus osEventAllocate
(
  TOsEvent*         pxEvent
);




/**
 *  @brief
 *    This function frees one or more specified allocated events.
 *
 *  @param[in] xEvents
 *               Bitmap of events to be freed.
 *
 *  @retval  OS_NO_ERROR
 *              Events could be freed successfully
 *
 *  @retval  OS_ERROR
 *              An error occurred
 *
 *
 *  @remarks
 *    -# Trying to free non-allocated events is allowed and shall not be
 *       considered as an error.
 *    .
 *    -# More than one event may be freed with a single call to \c
 *       osEventFree().
 *
 *  @see osEventAllocate()
*/

NAGRA_CA_API TOsStatus osEventFree
(
  TOsEvent          xEvents
);




/**
 *  @brief
 *    This function allows a task to wait for a specified set of events to be
 *    sent by other tasks. This mechanism is used to synchronize tasks among
 *    themselves.
 *
 *    An event is considered active for the task as soon as it has been
 *    internally received by this task. The synchronization is achieved when at
 *    least one event of the expected set of events is active.
 *
 *    If one or more expected events are not allocated, the function must return
 *    with an error.
 *
 *    This function only returns events that are expected (i.e. part of
 *    \c xExpectedEvents) and active (internally received by the task).
 *
 *    Events returned by this function must be implicitly deactivated (set to
 *    0). However, active events that are not returned by this function shall
 *    remain active.
 *
 *  @param[in] xExpectedEvents
 *               Bitmap of events the calling task is waiting for.
 *
 *  @param[in] xFlags
 *               Flags used to specify whether \c osEventReceive is blocking or
 *               not:
 *               - If the flag \c OS_EVENT_WAIT is set, the calling task shall
 *                 enter the sleeping state until the synchronization is fired
 *                 or until the timeout given by \c xTimeout expires.
 *               - If the flag \c OS_EVENT_NO_WAIT is set, this function shall
 *                 returns at once and \c pxReceivedEvents shall contain the
 *                 events that are expected and currently active. The
 *                 function shall not wait for another task to send one of the
 *                 expected events.
 *
 *  @param[in] xTimeout
 *               Maximum time the calling task is allowed to wait for the
 *               synchronization condition. This value is expressed in
 *               milliseconds. A zero value specifies that no time-out is
 *               required. In this case, the calling task must wait until event
 *               synchronization is achieved.\n
 *               Time-out value has no effect if the event processing flag is
 *               set to \c OS_EVENT_NO_WAIT.
 *
 *  @param[out] pxReceivedEvents
 *                Set of events that has been internally received by the task
 *                and that were expected by the caller (part of \c
 *                xExpectedEvents). When this function returns, all events
 *                specified in \c pxReceivedEvents shall be implicitely cleared
 *                internally (no longer active).
 *
 *  @retval  OS_NO_ERROR
 *              The function returned successfully.
 *
 *  @retval  OS_ERROR
 *              An error occurred
 *
 *
 *  @remarks
 *    -# If one of more events specified in \c xExpectedEvents has not been
 *       allocated, this function shall return \c OS_ERROR.
 *    .
 *    -# If a granularity of one millisecond cannot be achieved, real time-out
 *       value must be rounded to the closer upper value.
 *    .
 *    -# If the timeout expires and no event has been received, the function
 *       shall returns with the status \c OS_NO_ERROR and all bits of \c
 *       pxReceivedEvents shall be set to 0.
 *
 *  @see osEventSend
*/
NAGRA_CA_API TOsStatus osEventReceive
(
  TOsEvent          xExpectedEvents,
  TOsEventFlags     xFlags,
  TOsTime           xTimeout,
  TOsEvent*         pxReceivedEvents
);




/**
 *  @brief
 *    This function sends one or more events to another task (target task). It
 *    is not a blocking function. If sent successfully, these events are
 *    internally activated in the receiving task.
 *
 *    If the receiving task was waiting for one of these events, the
 *    synchronization is achieved and the receiving task is woken up.
 *
 *  @param[in] xTaskId
 *               Identifier of the target task
 *
 *  @param[in] xSentEvents
 *               Bitmap of events to be sent
 *
 *  @retval  OS_NO_ERROR
 *              The event task could be sent.
 *  @retval  OS_ERROR
 *              A global error occurred.
 *
 *  @remarks
 *    -# The events to send must be allocated. If not the function
 *       returns \c OS_ERROR.
 *    .
 *    -# Events are bit encoded. As a result, they are not queued nor counted
 *       when received by the target task.
 *
 *  @see osEventReceive
*/

NAGRA_CA_API TOsStatus osEventSend
(
  TOsTaskId         xTaskId,
  TOsEvent          xSentEvents
);




               /* -------------------------------------------- */
               /*     Timer management function prototypes     */
               /* -------------------------------------------- */

/**
 *  @brief
 *    The timer mechanism allows a task to be notified after a real time delay.
 *    The notification is done by sending events to the task that created the
 *    timer.
 *
 *    A timer can run in two modes a) one-shot b) continuous. In the first
 *    case, events are sent only once when the timer expires. In the second
 *    case, the timer is automatically re-started for a new occurrence so that
 *    events are sent periodically.
 *
 *    This function creates a timer, but do not start it. The caller provides
 *    the set of events to be sent at expiration as well as the running mode
 *    (one-shot or continuous). The timeout is not provided at the timer
 *    creation. It is provided when the timer is started (see \c osTimerStart).
 *
 *    Events provided to this function shall be already allocated. Otherwise,
 *    \c osTimerCreate shall return an error.
 *
 *  @param[in] xEventToSend
 *               Events to be sent to the task that created the timer when the
 *               timeout given to \c osTimerStart is reached.
 *
 *  @param[in] xTimerMode
 *               Mode of the timer: one-shot or continuous
 *
 *  @param[out] pxTimerId
 *                Timer identifier assigned by the OSY layer to the timer
 *                created. It must be unique for each timer created.
 *
 *  @retval  OS_NO_ERROR
 *              The timer could be created successfully
 *
 *  @retval  OS_ERROR
 *              The timer creation failed
 *
 *  @remarks
 *    -# This function is always called from a task context created
 *       by \c osTaskCreate().
*/
NAGRA_CA_API TOsStatus osTimerCreate
(
  TOsEvent          xEventToSend,
  TOsTimerMode      xTimerMode,
  TOsTimerId*       pxTimerId
);




/**
 *  @brief
 *    This function starts a timer already created. This is a non-blocking
 *    function. The behaviour of the timer depends on the mode specified at the
 *    timer creation.
 *
 *    When the time-out specified is reached, the task that created the timer is
 *    notified by the events specified at the timer creation.
 *
 *  @param[in] xTimerId
 *               Identifier of the timer to be started
 *
 *  @param[in] xTimeout
 *               Time interval expressed in milliseconds after which the timer
 *               events shall be sent
 *
 *  @retval  OS_NO_ERROR
 *              The timer could be started successfully
 *
 *  @retval  OS_ERROR
 *              The start-up of the timer failed
 *
 *  @remarks
 *    -# If a granularity of one millisecond cannot be granted, real timeout
 *       value must be rounded to the closer upper value.
 *    .
 *    -# If the timeout is set to 0, the function shall return \c OS_NO_ERROR
 *       and the timer shall expire immediately.
 *    .
 *    -# Starting a non-created timer shall cause the function to return \c
 *       OS_ERROR.
 *    .
 *    -# Starting a timer already started shall not generate an error. Instead,
 *       the timer shall be restarted with the new timeout.
 *    .
 *    -# Events to be sent at the timer expiration shall be allocated. Otherwise
 *       this function shall return \c OS_ERROR. This may occur if some events
 *       that were allocated at the timer creation have been freed before calling
 *       \c osTimerStart.
*/
NAGRA_CA_API TOsStatus osTimerStart
(
  TOsTimerId        xTimerId,
  TOsTime           xTimeout
);




/**
 *  @brief
 *    This function stops a timer previously started. This is not a blocking
 *    function. If the timer is not running, no error shall be returned.
 *
 *  @param[in] xTimerId
 *               Identifier of the timer to be stopped
 *
 *  @retval  OS_NO_ERROR
 *              The timer could be stopped successfully
 *
 *  @retval  OS_ERROR
 *              An error occurred and the timer could not be stopped
 *
 *  @remarks
 *    -# Trying to stop a timer that is not created (unknown \c xTimerId) shall
 *       cause the function to return \c OS_ERROR.
*/

NAGRA_CA_API TOsStatus osTimerStop
(
  TOsTimerId        xTimerId
);




/**
 *  @brief
 *    This function deletes a timer.
 *
 *  @param[in] xTimerId
 *               Identifier of the timer to be deleted.
 *
 *  @retval  OS_NO_ERROR
 *              The timer could be deleted successfully
 *
 *  @retval  OS_ERROR
 *              The deletion of the timer failed
 *
 *  @remarks
 *    -# Deleting a timer that is not created (unknown \c xTimerId) shall cause
 *       the function to return \c OS_ERROR.
 *    .
 *    -# Deleting a timer that is currently running shall cause the function to
 *       return \c OS_ERROR.
*/
NAGRA_CA_API TOsStatus osTimerDelete
(
  TOsTimerId        xTimerId
);




               /* -------------------------------------------- */
               /*   Semaphore management function prototypes   */
               /* -------------------------------------------- */


/**
 *  @brief
 *    This function creates a semaphore.
 *
 *  @param[in] xCount
 *               Initial semaphore token count.
 *
 *  @param[out] pxSemaphoreId
 *                 Semaphore identifier assigned by the OSY layer to the
 *                 semaphore created. It must be unique for each semaphore
 *                 created.
 *
 *  @retval  OS_NO_ERROR
 *              The semaphore could be created successfully
 *
 *  @retval  OS_ERROR
 *              The semaphore creation failed
 *
 *  @see osSemaphoreDelete
*/
NAGRA_CA_API TOsStatus osSemaphoreCreate
(
  TOsCount          xCount,
  TOsSemaphoreId*   pxSemaphoreId
);




/**
 *  @brief
 *    This function requests a semaphore token. If available (count > 0), a
 *    token is given to the calling task and this function returns immediately.
 *
 *    If the semaphore has no more token available (count = 0), the calling task
 *    shall be blocked waiting for tokens to be released.
 *
 *  @param[in] xSemaphoreId
 *               Identifier of the semaphore concerned by the token request.
 *
 *  @retval  OS_NO_ERROR
 *              The semaphore token could be attributed successfully
 *
 *  @retval  OS_ERROR
 *              The attribution of the semaphore token failed. It could be due
 *              to the fact that the semaphore has been deleted.
 *
 *  @remarks
 *    -# If a task is waiting for a token from a semaphore that is deleted, the
 *       task shall be woken up and \c osSemaphoreGet shall return \c OS_ERROR.
 *
 *  @see osSemaphoreRelease
*/
NAGRA_CA_API TOsStatus osSemaphoreGet
(
  TOsSemaphoreId    xSemaphoreId
);




/**
 *  @brief
 *    This function releases a semaphore token (is incremented of one unit).
 *    It is not a blocking function.
 *
 *  @param[in] xSemaphoreId
 *               Identifier of the semaphore concerned by the token release
 *
 *  @retval  OS_NO_ERROR
 *              The semaphore token could be released successfully
 *
 *  @retval  OS_ERROR
 *              The release of the semaphore token failed
 *
 *  @see osSemaphoreGet
*/
NAGRA_CA_API TOsStatus osSemaphoreRelease
(
  TOsSemaphoreId    xSemaphoreId
);




/**
 *  @brief
 *    This function deletes a semaphore. If tasks were waiting for a token from
 *    this sempahore, they shall be unblocked and notified of an error status
 *    (see \c osSemaphoreGet).
 *
 *  @param[in] xSemaphoreId
 *               Identifier of the semaphore to be deleted.
 *
 *  @retval  OS_NO_ERROR
 *              The semaphore could be deleted successfully
 *
 *  @retval  OS_ERROR
 *              The deletion of the semaphore failed
*/
NAGRA_CA_API TOsStatus osSemaphoreDelete
(
  TOsSemaphoreId    xSemaphoreId
);




               /* -------------------------------------------- */
               /*         Memory management prototypes         */
               /* -------------------------------------------- */


/**
 *  @brief
 *    This function allows a task to allocate a block of memory. The allocated
 *    memory can be taken either from the main memory heap or from a memory pool
 *    dedicated to Nagravision.
 *
 *  @param[in] xSize
 *               Size in bytes of the memory block to be allocated
 *
 *  @remark
 *    -# If \c xSize is set to 0, this function shall return \c NULL without
 *       allocating any memory.
 *
 *  @return
 *    This function returns the pointer to the allocated memory
 *    block if successful, returns \c NULL otherwise.
 *
 *  @see
 *    osMemoryFree()
*/
NAGRA_CA_API void* osMemoryAllocate
(
  TSize            xSize
);




/**
 *  @brief
 *    This function allows a task to free a memory block previously allocated.
 *
 *  @param[in] pxMemBlock
 *               Pointer to the memory block to be freed.
 *
 *  @return
 *    None
 *
 *  @remarks
 *    -# This function must not fail if trying to free a NULL pointer
 *
 *  @see
 *    osMemoryAllocate()
*/
NAGRA_CA_API void osMemoryFree
(
  void*             pxMemBlock
);




               /* -------------------------------------------- */
               /*    Persistent memory management prototypes   */
               /* -------------------------------------------- */


/**
 *  @brief
 *    This function returns the list of available persistent memory storages.
 *
 *    A storage is a logical block of persistent memory that shall be mapped
 *    onto a physical block. A logical block of a given size will most probably
 *    require a larger physical block if a wear-leveling technique is
 *    implemented to increase the lifetime of the physical memory.
 *
 *    A persistent memory storage shall fulfill a certain number of requirements
 *    that depend on the usage made by the CA software (e.g. size of the
 *    storage, average number of writing cycles, etc). Refer to the system
 *    requirements of the CA software module concerned for further information.
 *
 *  @param[out] pxNumStorages
 *                Number of items returned in \c ppxStorageTable
 *
 *  @param[out] ppxStorageTable
 *                Table describing the characteristics of each persistent memory
 *                storage. The OSY layer is in charge of the allocation of the
 *                memory required for this table and shall guarantee that it
 *                will remain available at any time.
 *
 *  @retval  OS_NO_ERROR
 *              The storage table has been returned successfully
 *
 *  @retval  OS_ERROR
 *              The exportation of the storage table failed
 *
 *  @remarks
 *    -# The returned pointer \c ppxStorageTable must be the same as long as the
 *       decoder is not rebooted.
 *    .
 *    -# An update of the decoder's software shall not modify the persistent
 *       memory storages.
 *    .
 *    -# A persistent memory storage shall not be part of a sector containing
 *       code checked by the Nagravision secure boot procedure (SSA-SBP).
 *    .
 *    -# A persistent memory storage shall not be addressable by a middleware
 *       application interpreted by a virtual machine (OpenTV, Liberate, MHP,
 *       etc).
*/
NAGRA_CA_API TOsStatus osPersistentMemoryGetStorageTable
(
  TUnsignedInt8*                 pxNumStorages,
  TOsPersistentMemoryStorage**  ppxStorageTable
);




/**
 *  @brief
 *    This function reads a given number of bytes from a persistent memory
 *    storage and copy them in a buffer allocated by the caller.
 *
 *  @param[in] xStorageId
 *               ID of the storage data are read from
 *
 *  @param[in] xFirstByte
 *               Number of bytes, from the beginning of the storage, the reading
 *               shall start from. A value of 0 corresponds to the very first
 *               byte of the storage.
 *
 *  @param[in] xNumBytes
 *               Number of bytes to be copied in \c pxBuffer, starting from and
 *               including \c xFirstByte.
 *
 *  @param[out] pxBuffer
 *                Destination buffer where to copy the data from the persistent
 *                memory storage. This buffer is \c xNumBytes long and is
 *                allocated by the caller.
 *
 *  @retval  OS_NO_ERROR
 *              The reading succeeded
 *
 *  @retval  OS_ERROR
 *              The reading failed
 *
 *  @remarks
 *    -# The calling task guarantees not to access outside the available
 *       storage.
 *    .
 *    -# It is the driver responsibility to manage concurrent access to the
 *       physical persistent memory. In particular, if the application is
 *       running from the flash memory, the driver is responsible for the
 *       possible concurrent access to that memory. Same remark if two logical
 *       storages are mapped onto the same physical memory.
 *    .
 *    -# This function shall not return an error due to the fact that \c
 *       xNumBytes is equal to 0. In such a case, \c OS_NO_ERROR is returned and
 *       the destination buffer \c pxBuffer is not altered.
*/
NAGRA_CA_API TOsStatus osPersistentMemoryRead
(
  TOsPersistentStorageId   xStorageId,
  TSize                    xFirstByte,
  TSize                    xNumBytes,
  TUnsignedInt8*           pxBuffer
);




/**
 *  @brief
 *    This function writes a given number of bytes into a persistent memory
 *    storage.
 *
 *  @param[in] xStorageId
 *               ID of the storage data are written to
 *
 *  @param[in]   xFlags
 *                 Deprecated
 *
 *  @param[in] xFirstByte
 *               Number of bytes, from the beginning of the storage, the
 *               writting shall start from. A value of 0 corresponds to the very
 *               first byte of the storage.
 *
 *  @param[in] xNumBytes
 *               Number of bytes to be written to the persistent memory storage,
 *               starting from and including \c xFirstByte.
 *
 *  @param[in] pxBuffer
 *               Source buffer containing the data to be written to the
 *               persistent memory storage. This buffer is \c xNumBytes long and
 *               is allocated by the caller.
 *
 *  @retval  OS_NO_ERROR
 *             The persistent memory storage has been written successfully
 *
 *  @retval  OS_ERROR
 *             An error occurred (bad ID, bad offset, write failed)
 *
 *  @remarks
 *    -# The calling task guarantees not to access outside the available
 *       storage.
 *    .
 *    -# It is the driver responsibility to manage concurrent access to the
 *       physical persistent memory. In particular, if the application is
 *       running from the flash memory, the driver is responsible for the
 *       possible concurrent access to that memory. Same remark if two logical
 *       storages are mapped onto the same physical memory.
 *    .
 *    -# The driver has to guarantee the integrity of the data at any time. In
 *       particular, this function shall be robust against untimely power
 *       cycles. For instance, if the decoder is switch off during a writing
 *       operation, the driver shall guarantee that the persistent memory
 *       storage gets back to the state that precedes the erroneous writing
 *       operation when the decoder is switch on again.
 *    .
 *    -# This function shall not return an error due to the fact that \c
 *       xNumBytes is equal to 0. In such a case, \c OS_NO_ERROR is returned
 *       without writing any bytes from the source buffer \c pxBuffer.
*/
NAGRA_CA_API TOsStatus osPersistentMemoryWrite
(
        TOsPersistentStorageId    xStorageId,
        TOsPersistentMemoryFlags  xFlags,
        TSize                     xFirstByte,
        TSize                     xNumBytes,
  const TUnsignedInt8*            pxBuffer
);




/**
 *  @brief
 *    This function returns the usage associated to a storage. Storages of a
 *    given usage shall fulfill specific requirements that depend on
 *    the CA software. Refer to the system requirements of the CA software
 *    concerned for further information.
 *
 *  @param[in] xStorageId
 *               ID of the storage
 *
 *  @param[out] pxUsage
 *                Usage associated to this storage.
 *
 *  @retval  OS_NO_ERROR
 *             The usage has been returned successfully
 *
 *  @retval  OS_ERROR
 *             An error occurred (bad ID, invalid parameter)
*/
NAGRA_CA_API TOsStatus osPersistentMemoryGetStorageUsage
(
  TOsPersistentStorageId      xStorageId,
  TOsPersistentMemoryUsage*   pxUsage
);




               /* -------------------------------------------- */
               /*      Miscellaneous function prototypes       */
               /* -------------------------------------------- */


/**
 *  @brief
 *    This function sends a NULL-terminated char string to the STB console.
 *    This console is used for logging traces (when built in DEBUG) and for
 *    occasionnaly dumping some management data.
 *
 *  @param[in] pxString
 *               Null-terminated char string to be sent to the console.
 *
 *  @remarks
 *    -# Requirement RQ-OSY_OSPRT_0001: (Behavior)
 *       This function shall not interleave strings from several successive
 *       calls. It shall be re-entrant and support multi-threading but one
 *       string is always printed as a whole before starting printing another.
 *    .
 *    -# Requirement RQ-OSY_OSPRT_0002: (Behavior)
 *       This function shall be synchronous. Caller shall block until all
 *       characters are processed.
 *    .
 *    -# Requirement RQ-OSY_OSPRT_0003: (Perf & Sizing)
 *       This function shall be able to handle strings being 1024 characters
 *       long at least.
 *    .
 *    -# Requirement RQ-OSY_OSPRT_0004: (Perf & Sizing)
 *       CAK will not dump at a rate higher than 1024 bytes every 150 ms (this
 *       allows to send at a speed of 115200 baud without buffer overrun).
 *    .
 */
NAGRA_CA_API void osPrint
(
  const TChar*  pxString
);

/**
 *  @brief
 *    This function gets a character from the STB console. Characters shall
 *    be buffered in a FIFO in order to ensure that no character is lost. As
 *    soon as the FIFO is full, characters are silently discarded (to ensure
 *    slight handle of overrun conditions).
 *
 *  @return
 *    First character stored in the FIFO buffer
 *
 *  @remarks
 *    -# Requirement RQ-OSY_OSCHG_0001: (Behavior)
 *       This is a blocking call. If there is no character pending, the calling
 *       task must be blocked waiting for a character.
 *    .
 *    -# Requirement RQ-OSY_OSCHG_0002: (Behavior)
 *       No character should be lost. If no task asks for an incoming character,
 *       it should be stored in a FIFO buffer for the next calls.
 *    .
 *    -# Requirement RQ-OSY_OSCHG_0003: (Behavior)
 *       When buffer is full, overrun characters must be discarded (buffer has a
 *       predetermined size).
 *    .
 *    -# Requirement RQ-OSY_OSCHG_0004:
 *       Any character sent to the STB through its console shall be available for
 *       CAK through osCharacterGet (but, of course, the ones discarded because
 *       buffer is full), whatever could be the number of console readers.
 */
NAGRA_CA_API TChar osCharacterGet
(
  void
);




/**
 *  @brief
 *    This function terminates the execution of the decoder program.
 *
 *  @param[in] pxString
 *               Null-terminated string to be displayed on the debug link before
 *               stopping the execution.
 *
 *  @param[in] pxFile
 *               Null-terminated string containing the caller's function source
 *               file name.
 *
 *  @param[in] xLine
 *               Caller's function source file line.
 *
 *  @return
 *    This function never returns.
 *
 *  @remarks
 *    -# On development platforms, this function should ideally call a debugger
 *       or at least stop the program execution. On production models, it is
 *       advised to reset the decoder.
*/
NAGRA_CA_API void osAbort
(
  const TChar* pxString,
  const TChar* pxFile,
  const int     xLine
);




/**
 *  @brief
 *    This function returns the operating system time in ms. It shall not depend
 *    on the presence of a TDT table but should rely on the operating system or
 *    CPU clock instead.
 *
 *  @return
 *    Operating system time in milliseconds.
 *
 *  @remarks
 *    -# The returned time values shall be monotonic over the full \c TOsTime
 *       range. The counter shall wrap around every 2^(8*sizeof(TOsTime)) ms.
 *    .
 *    -# If the system time is not expressed in ms, a simple linear conversion
 *       is not sufficient to guarantee the monotonicity of the returned values.
 *       One way to comply with this requirement is to increment a base counter
 *       every second and, when the \c osTimerGet() function is called, return
 *       the base counter value adjusted with the number of ms elapsed from the
 *       last second.
*/
NAGRA_CA_API TOsTime osTimeGet
(
  void
);


#endif /* CA_OS_H */


/* END OF FILE */
