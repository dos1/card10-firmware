#include "modules/log.h"
#include "modules/mutex.h"

#include "api/dispatcher.h"

#include "FreeRTOS.h"
#include "task.h"

#define TIMEOUT pdMS_TO_TICKS(2000)

TaskHandle_t dispatcher_task_id;

struct mutex api_mutex = { 0 };

void dispatcher_mutex_init(void)
{
	mutex_create(&api_mutex);
}

/*
 * API dispatcher task.  This task will sleep until an API call is issued and
 * then wake up to dispatch it.
 */
void vApiDispatcher(void *pvParameters)
{
	LOG_DEBUG("dispatcher", "Ready.");
	while (1) {
		if (api_dispatcher_poll()) {
			mutex_lock(&api_mutex);
			api_dispatcher_exec();
			mutex_unlock(&api_mutex);
		}
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}
