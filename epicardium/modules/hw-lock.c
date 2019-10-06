#include "modules/log.h"
#include "modules/modules.h"
#include "modules/mutex.h"

#include "FreeRTOS.h"
#include "task.h"

#include <errno.h>

static struct mutex hwlock_mutex[_HWLOCK_MAX] = { { 0 } };

void hwlock_init(void)
{
	for (int i = 0; i < _HWLOCK_MAX; i++) {
		/*
		 * TODO: mutex_create() names these all these mutexes
		 *       "&hwlock_mutex[i]" which is not helpful at all.  We
		 *       should somehow rename them to the actual hwlock names.
		 */
		mutex_create(&hwlock_mutex[i]);
	}
}

void hwlock_acquire(enum hwlock_periph p)
{
	assert(p < _HWLOCK_MAX);
	mutex_lock(&hwlock_mutex[p]);
}

int hwlock_acquire_nonblock(enum hwlock_periph p)
{
	assert(p < _HWLOCK_MAX);
	if (mutex_trylock(&hwlock_mutex[p])) {
		return 0;
	} else {
		return -EBUSY;
	}
}

int hwlock_acquire_timeout(enum hwlock_periph p, TickType_t wait)
{
	assert(p < _HWLOCK_MAX);

	/*
	 * Check for code still defining a timeout.  It will be ignored in the
	 * new implementation but a warning is emitted so someone hopefully
	 * eventually fixes the offending code ...
	 *
	 * At some point, the signature of this function should be changed.
	 * Alternatively it could be split into two, similar to the mutex API.
	 */
	if (wait != 0 && wait != portMAX_DELAY) {
		LOG_WARN(
			"hwlock",
			"Attempting to lock %d with a timeout (from %p).",
			p,
			__builtin_return_address(0)
		);
	}

	/* Non-blocking lock attempt */
	if (wait == 0) {
		if (mutex_trylock(&hwlock_mutex[p]) == true) {
			return 0;
		} else {
			return -EBUSY;
		}
	}

	mutex_lock(&hwlock_mutex[p]);
	return 0;
}

void hwlock_release(enum hwlock_periph p)
{
	assert(p < _HWLOCK_MAX);
	mutex_unlock(&hwlock_mutex[p]);
}
