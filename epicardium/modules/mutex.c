#include "modules/mutex.h"

#include <assert.h>

void _mutex_create(struct mutex *m, const char *name)
{
	/* Assert that the mutex has not been initialized already */
	assert(m->name == NULL);

	/*
	 * The name is just the parameter stringified which is almost always a
	 * pointer.  If it is, skip over the '&' because it adds no value as
	 * part of the name.
	 */
	if (name[0] == '&') {
		m->name = &name[1];
	} else {
		m->name = name;
	}

	m->_rtos_mutex = xSemaphoreCreateMutexStatic(&m->_rtos_mutex_data);
}

void mutex_lock(struct mutex *m)
{
	int ret = xSemaphoreTake(m->_rtos_mutex, portMAX_DELAY);

	/* Ensure locking was actually successful */
	assert(ret == pdTRUE);
}

bool mutex_trylock(struct mutex *m)
{
	int ret = xSemaphoreTake(m->_rtos_mutex, 0);
	return ret == pdTRUE;
}

void mutex_unlock(struct mutex *m)
{
	/* Ensure that only the owner can unlock a mutex */
	assert(mutex_get_owner(m) == xTaskGetCurrentTaskHandle());

	int ret = xSemaphoreGive(m->_rtos_mutex);

	/*
	 * Ensure that unlocking was successful; that is, the mutex must have
	 * been acquired previously (no multiple unlocks).
	 */
	assert(ret == pdTRUE);
}

TaskHandle_t mutex_get_owner(struct mutex *m)
{
	return xSemaphoreGetMutexHolder(m->_rtos_mutex);
}
