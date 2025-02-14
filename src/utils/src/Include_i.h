/**
 * Internal functionality
 */
#ifndef __UTILS_I_H__
#define __UTILS_I_H__

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "com/amazonaws/kinesis/video/utils/Include.h"

/**
 * Thread wrapper for Windows
 */
typedef struct {
    // Stored routine
    startRoutine storedStartRoutine;

    // Original arguments
    PVOID storedArgs;
} WindowsThreadRoutineWrapper, *PWindowsThreadRoutineWrapper;

/**
 * Internal String operations
 */
STATUS strtoint(PCHAR, PCHAR, UINT32, PUINT64, PBOOL);

/**
 * Internal safe multiplication
 */
STATUS unsignedSafeMultiplyAdd(UINT64, UINT64, UINT64, PUINT64);

/**
 * Internal Double Linked List operations
 */
STATUS doubleListAllocNode(UINT64, PDoubleListNode*);
STATUS doubleListInsertNodeHeadInternal(PDoubleList, PDoubleListNode);
STATUS doubleListInsertNodeTailInternal(PDoubleList, PDoubleListNode);
STATUS doubleListInsertNodeBeforeInternal(PDoubleList, PDoubleListNode, PDoubleListNode);
STATUS doubleListInsertNodeAfterInternal(PDoubleList, PDoubleListNode, PDoubleListNode);
STATUS doubleListRemoveNodeInternal(PDoubleList, PDoubleListNode);
STATUS doubleListGetNodeAtInternal(PDoubleList, UINT32, PDoubleListNode*);

/**
 * Internal Single Linked List operations
 */
STATUS singleListAllocNode(UINT64, PSingleListNode*);
STATUS singleListInsertNodeHeadInternal(PSingleList, PSingleListNode);
STATUS singleListInsertNodeTailInternal(PSingleList, PSingleListNode);
STATUS singleListInsertNodeAfterInternal(PSingleList, PSingleListNode, PSingleListNode);
STATUS singleListGetNodeAtInternal(PSingleList, UINT32, PSingleListNode*);

/**
 * Internal Hash Table operations
 */
#define DEFAULT_HASH_TABLE_BUCKET_LENGTH    2
#define DEFAULT_HASH_TABLE_BUCKET_COUNT     10000
#define MIN_HASH_TABLE_ENTRIES_ALLOC_LENGTH 8

/**
 * Bucket declaration
 * NOTE: Variable size structure - the buckets can follow directly after the main structure
 * or in case of allocated array it's a separate allocation
 */
typedef struct {
    UINT32 count;
    UINT32 length;
    PHashEntry entries;
} HashBucket, *PHashBucket;

UINT64 getKeyHash(UINT64);
PHashBucket getHashBucket(PHashTable, UINT64);

/**
 * Internal Directory functionality
 */
STATUS removeFileDir(UINT64, DIR_ENTRY_TYPES, PCHAR, PCHAR);
STATUS getFileDirSize(UINT64, DIR_ENTRY_TYPES, PCHAR, PCHAR);

/**
 * Endianness functionality
 */
INLINE INT16 getInt16Swap(INT16);
INLINE INT16 getInt16NoSwap(INT16);
INLINE INT32 getInt32Swap(INT32);
INLINE INT32 getInt32NoSwap(INT32);
INLINE INT64 getInt64Swap(INT64);
INLINE INT64 getInt64NoSwap(INT64);

INLINE VOID putInt16Swap(PINT16, INT16);
INLINE VOID putInt16NoSwap(PINT16, INT16);
INLINE VOID putInt32Swap(PINT32, INT32);
INLINE VOID putInt32NoSwap(PINT32, INT32);
INLINE VOID putInt64Swap(PINT64, INT64);
INLINE VOID putInt64NoSwap(PINT64, INT64);

/**
 * Unaligned access functionality
 */
INLINE INT16 getUnalignedInt16Be(PVOID);
INLINE INT16 getUnalignedInt16Le(PVOID);
INLINE INT32 getUnalignedInt32Be(PVOID);
INLINE INT32 getUnalignedInt32Le(PVOID);
INLINE INT64 getUnalignedInt64Be(PVOID);
INLINE INT64 getUnalignedInt64Le(PVOID);

INLINE VOID putUnalignedInt16Be(PVOID, INT16);
INLINE VOID putUnalignedInt16Le(PVOID, INT16);
INLINE VOID putUnalignedInt32Be(PVOID, INT32);
INLINE VOID putUnalignedInt32Le(PVOID, INT32);
INLINE VOID putUnalignedInt64Be(PVOID, INT64);
INLINE VOID putUnalignedInt64Le(PVOID, INT64);

//////////////////////////////////////////////////////////////////////////////////////////////
// TimerQueue functionality
//////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Startup and shutdown timeout value
 */
#define TIMER_QUEUE_SHUTDOWN_TIMEOUT (200 * HUNDREDS_OF_NANOS_IN_A_MILLISECOND)

/**
 * Timer entry structure definition
 */
typedef struct {
    UINT64 period;
    UINT64 invokeTime;
    UINT64 customData;
    TimerCallbackFunc timerCallbackFn;
} TimerEntry, *PTimerEntry;

/**
 * Internal timer queue definition
 */
typedef struct {
    volatile TID executorTid;
    volatile ATOMIC_BOOL shutdown;
    volatile ATOMIC_BOOL terminated;
    volatile ATOMIC_BOOL started;
    UINT32 maxTimerCount;
    UINT32 activeTimerCount;
    UINT64 invokeTime;
    MUTEX executorLock;
    CVAR executorCvar;
    MUTEX startLock;
    CVAR startCvar;
    MUTEX exitLock;
    CVAR exitCvar;
    PTimerEntry pTimers;
} TimerQueue, *PTimerQueue;

// Public handle to and from object converters
#define TO_TIMER_QUEUE_HANDLE(p)   ((TIMER_QUEUE_HANDLE)(p))
#define FROM_TIMER_QUEUE_HANDLE(h) (IS_VALID_TIMER_QUEUE_HANDLE(h) ? (PTimerQueue)(h) : NULL)

// Internal Functions
STATUS timerQueueCreateInternal(UINT32, PTimerQueue*);
STATUS timerQueueFreeInternal(PTimerQueue*);
STATUS timerQueueEvaluateNextInvocation(PTimerQueue);

// Executor routine
PVOID timerQueueExecutor(PVOID);

//////////////////////////////////////////////////////////////////////////////////////////////
// Semaphore functionality
//////////////////////////////////////////////////////////////////////////////////////////////

// Shutdown spinlock will sleep this period and check
#define SEMAPHORE_SHUTDOWN_SPINLOCK_SLEEP_DURATION (5 * HUNDREDS_OF_NANOS_IN_A_MILLISECOND)

/**
 * Internal semaphore definition
 */
typedef struct {
    // Current granted permit count
    volatile SIZE_T permitCount;

    // Whether the semaphore is locked for granting any more permits
    volatile ATOMIC_BOOL locked;

    // Whether we are shutting down
    volatile ATOMIC_BOOL shutdown;

    // Max allowed permits
    SIZE_T maxPermitCount;

    // Semaphore notification await cvar
    CVAR semaphoreNotify;

    // Notification cvar lock
    MUTEX semaphoreLock;

    // Notification cvar for awaiting till drained
    CVAR drainedNotify;

    // Draining notification lock
    MUTEX drainedLock;
} Semaphore, *PSemaphore;

// Public handle to and from object converters
#define TO_SEMAPHORE_HANDLE(p)   ((SEMAPHORE_HANDLE)(p))
#define FROM_SEMAPHORE_HANDLE(h) (IS_VALID_SEMAPHORE_HANDLE(h) ? (PSemaphore)(h) : NULL)

// Internal Functions
STATUS semaphoreCreateInternal(UINT32, PSemaphore*);
STATUS semaphoreFreeInternal(PSemaphore*);
STATUS semaphoreAcquireInternal(PSemaphore, UINT64);
STATUS semaphoreReleaseInternal(PSemaphore);
STATUS semaphoreSetLockInternal(PSemaphore, BOOL);
STATUS semaphoreWaitUntilClearInternal(PSemaphore, UINT64);

//////////////////////////////////////////////////////////////////////////////////////////////
// Instrumented allocators functionality
//////////////////////////////////////////////////////////////////////////////////////////////
PVOID instrumentedAllocatorsMemAlloc(SIZE_T);
PVOID instrumentedAllocatorsMemAlignAlloc(SIZE_T, SIZE_T);
PVOID instrumentedAllocatorsMemCalloc(SIZE_T, SIZE_T);
PVOID instrumentedAllocatorsMemRealloc(PVOID, SIZE_T);
VOID instrumentedAllocatorsMemFree(PVOID);

//////////////////////////////////////////////////////////////////////////////////////////////
// File logging functionality
//////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Default values for the limits
 */
#define KVS_COMMON_FILE_INDEX_BUFFER_SIZE 256

/**
 * file logger declaration
 */
typedef struct {
    // string buffer. once the buffer is full, its content will be flushed to file
    PCHAR stringBuffer;

    // Size of the buffer in bytes
    // This will point to the end of the FileLogger to allow for single allocation and preserve the processor cache locality
    UINT64 stringBufferLen;

    // lock protecting the print operation
    MUTEX lock;

    // bytes starting from beginning of stringBuffer that contains valid data
    UINT64 currentOffset;

    // directory to put the log file
    CHAR logFileDir[MAX_PATH_LEN + 1];

    // file to store last log file index
    CHAR indexFilePath[MAX_PATH_LEN + 1];

    // max number of log file allowed
    UINT64 maxFileCount;

    // index for next log file
    UINT64 currentFileIndex;

    // print log to stdout too
    BOOL printLog;

    // file logger logPrint callback
    logPrintFunc fileLoggerLogPrintFn;

    // Original stored logger function
    logPrintFunc storedLoggerLogPrintFn;
} FileLogger, *PFileLogger;

/**
 * syslog logger declaration
 */
#if !defined(_WIN32) && !defined(_WIN64)
typedef struct {
    // print log to stdout too
    BOOL printLog;

    // syslog logger logPrint callback
    logPrintFunc syslogLoggerLogPrintFn;

    // Original stored logger function
    logPrintFunc storedLoggerLogPrintFn;
} SyslogLogger, *PSyslogLogger;
#endif

/////////////////////////////////////////////////////////////////////
// Internal functionality
/////////////////////////////////////////////////////////////////////
/**
 * Flushes currentOffset number of chars from stringBuffer into logfile.
 * If maxFileCount is exceeded, the earliest file is deleted before writing to the new file.
 * After flushLogToFile finishes, currentOffset is set to 0, whether the status of execution was success or not.
 *
 * @return - STATUS of execution
 */
STATUS flushLogToFile();

#ifdef __cplusplus
}
#endif

#endif // __UTILS_I_H__
