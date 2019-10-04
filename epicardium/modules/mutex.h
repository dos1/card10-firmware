#ifndef _MUTEX_H
#define _MUTEX_H

#ifndef __SPHINX_DOC
/* Some headers are not recognized by hawkmoth for some odd reason */
#include <stddef.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#else
typedef unsigned int size_t;
typedef _Bool bool;

typedef void *TaskHandle_t;
typedef void *SemaphoreHandle_t;
typedef int StaticSemaphore_t;
#endif /* __SPHINX_DOC */


/**
 * Mutex data type.
 */
struct mutex {
	/* Name of this mutex, kept for debugging purposes.  */
	const char *name;

	/* FreeRTOS mutex data structures. */
	SemaphoreHandle_t _rtos_mutex;
	StaticSemaphore_t _rtos_mutex_data;
};

/**
 * Create a new mutex.
 *
 * Call this function as early as possible, in an obvious location so it is easy
 * to find.  Mutexes should be defined statically so they stay alive for the
 * entire run-time of the firmware.
 */
#define mutex_create(mutex) _mutex_create(mutex, #mutex)
void _mutex_create(struct mutex *m, const char *name);

/**
 * Lock a mutex.
 *
 * If the mutex is held by another task, :c:func:`mutex_lock` will block the
 * current task until the mutex is unlocked.
 *
 * .. warning::
 *
 *    This function is **not** safe to use in a timer!
 */
void mutex_lock(struct mutex *m);

/**
 * Try locking a mutex.
 *
 * If the mutex is currently locked by another task, :c:func:`mutex_trylock`
 * will return ``false`` immediately.  If the attmept to lock was successful, it
 * will return ``true``.
 *
 * This funciton is safe for use in timers.
 */
bool mutex_trylock(struct mutex *m);

/**
 * Unlock a mutex.
 *
 * You **must** call this function from the same task which originally locked
 * the mutex.
 */
void mutex_unlock(struct mutex *m);

/**
 * Get the current owner of the mutex.
 *
 * Returns the task-handle of the task currently holding the mutex.  If the
 * mutex is unlocked, ``NULL`` is returned.
 */
TaskHandle_t mutex_get_owner(struct mutex *m);

#endif /* _MUTEX_H */
