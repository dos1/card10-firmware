#include <stdio.h>
#include <stdlib.h>

#include <ff.h>

#include "max32665.h"
#include "uart.h"
#include "cdcacm.h"

#include "card10.h"
#include "pmic.h"
#include "leds.h"
#include "api/dispatcher.h"
#include "l0der/l0der.h"
#include "modules/modules.h"
#include "modules/log.h"
#include "modules/stream.h"
#include "modules/filesystem.h"
#include "api/interrupt-sender.h"

#include "Heart.h"
#include "gfx.h"
#include "display.h"
#include "card10-version.h"

#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t dispatcher_task_id;

void vBleTask(void *pvParameters);

/*
 * API dispatcher task.  This task will sleep until an API call is issued and
 * then wake up to dispatch it.
 */
void vApiDispatcher(void *pvParameters)
{
	LOG_DEBUG("dispatcher", "Ready.");
	while (1) {
		if (api_dispatcher_poll()) {
			api_dispatcher_exec();
		}
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

int main(void)
{
	LOG_INFO("startup", "Epicardium startup ...");
	LOG_INFO("startup", "Version " CARD10_VERSION);

	card10_init();
	card10_diag();

	gfx_copy_region(
		&display_screen,
		0,
		0,
		160,
		80,
		GFX_RAW,
		sizeof(Heart),
		(const void *)(Heart)
	);
	gfx_update(&display_screen);

	/* TODO: Move this to its own function */
	SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

	if (cdcacm_init() < 0) {
		LOG_ERR("startup", "USB-Serial unavailable");
	}

	fatfs_init();
	api_interrupt_init();
	stream_init();

	LOG_INFO("startup", "Initializing tasks ...");

	/* Serial */
	if (xTaskCreate(
		    vSerialTask,
		    (const char *)"Serial",
		    configMINIMAL_STACK_SIZE,
		    NULL,
		    tskIDLE_PRIORITY + 1,
		    NULL) != pdPASS) {
		LOG_CRIT("startup", "Failed to create %s task!", "Serial");
		abort();
	}

	/* PMIC */
	if (xTaskCreate(
		    vPmicTask,
		    (const char *)"PMIC",
		    configMINIMAL_STACK_SIZE,
		    NULL,
		    tskIDLE_PRIORITY + 1,
		    NULL) != pdPASS) {
		LOG_CRIT("startup", "Failed to create %s task!", "PMIC");
		abort();
	}

	/* API */
	if (xTaskCreate(
		    vApiDispatcher,
		    (const char *)"API Dispatcher",
		    configMINIMAL_STACK_SIZE * 16,
		    NULL,
		    tskIDLE_PRIORITY + 2,
		    &dispatcher_task_id) != pdPASS) {
		LOG_CRIT(
			"startup",
			"Failed to create %s task!",
			"API Dispatcher"
		);
		abort();
	}

	/* BLE */
	if (xTaskCreate(
		    vBleTask,
		    (const char *)"BLE",
		    configMINIMAL_STACK_SIZE * 10,
		    NULL,
		    tskIDLE_PRIORITY + 1,
		    NULL) != pdPASS) {
		LOG_CRIT("startup", "Failed to create %s task!", "BLE");
		abort();
	}

	LOG_INFO("startup", "Initializing dispatcher ...");
	api_dispatcher_init();

	/* light sensor */
	LOG_INFO("startup", "starting light sensor ...");
	epic_light_sensor_run();

	/*
	 * See if there's a l0dable.elf to run. If not, run pycardium.
	 * This is temporary until epicardium gets a l0dable API from pycardium.
	 */
	const char *l0dable = "l0dable.elf";
	if (f_stat(l0dable, NULL) == FR_OK) {
		LOG_INFO("startup", "Running %s ...", l0dable);
		struct l0dable_info info;
		int res = l0der_load_path(l0dable, &info);
		if (res != 0) {
			LOG_ERR("startup", "l0der failed: %d\n", res);
		} else {
			LOG_INFO(
				"startup", "Starting %s on core1 ...", l0dable
			);
			core1_start(info.isr_vector);
		}
	} else {
		LOG_INFO("startup", "Starting pycardium on core1 ...");
		core1_start((void *)0x10080000);
	}

	LOG_INFO("startup", "Starting FreeRTOS ...");
	vTaskStartScheduler();

	LOG_CRIT("startup", "FreeRTOS did not start due to unknown error!");
	abort();
}
