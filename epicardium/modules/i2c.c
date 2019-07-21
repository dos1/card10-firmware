#include "modules/log.h"
#include "modules/modules.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <errno.h>

static StaticSemaphore_t i2c_mutex_data;
static SemaphoreHandle_t i2c_mutex;

int i2c_init(void)
{
	i2c_mutex = xSemaphoreCreateMutexStatic(&i2c_mutex_data);
	return 0;
}

int i2c_lock(void)
{
	if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
		LOG_WARN("i2c", "Mutex is busy");
		return -EBUSY;
	}
	return 0;
}

void i2c_unlock(void)
{
	if (xSemaphoreGive(i2c_mutex) != pdTRUE) {
		LOG_ERR("i2c", "Mutex was not acquired correctly");
	}
}
