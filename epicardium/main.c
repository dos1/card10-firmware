#include "modules/modules.h"
#include "modules/log.h"
#include "modules/filesystem.h"
#include "card10-version.h"

#include "FreeRTOS.h"
#include "task.h"
#include "mxc_delay.h"

#include <stdlib.h>
#include <string.h>

int main(void)
{
	LOG_INFO("startup", "Epicardium startup ...");
	LOG_INFO("startup", "Version " CARD10_VERSION);

	LOG_DEBUG("startup", "Initializing hardware ...");
	hardware_early_init();

	char *version_buf = CARD10_VERSION;
	epic_disp_print(0, 5, "epicardium:", 0xffff, 0x0000);
	epic_disp_print(0, 24, version_buf, 0xffff, 0x0000);
	epic_disp_update();
	mxc_delay(2000000);

	LOG_DEBUG("startup", "Initializing tasks ...");

	/* Serial */
	if (xTaskCreate(
		    vSerialTask,
		    (const char *)"Serial",
		    configMINIMAL_STACK_SIZE * 2,
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
		    tskIDLE_PRIORITY + 4,
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
	if (ble_shall_start()) {
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
	}

	/* LEDs */
	if (xTaskCreate(
		    vLedTask,
		    (const char *)"LED",
		    configMINIMAL_STACK_SIZE,
		    NULL,
		    tskIDLE_PRIORITY + 1,
		    NULL) != pdPASS) {
		LOG_CRIT("startup", "Failed to create %s task!", "LED");
		abort();
	}

	/* Lifecycle */
	if (xTaskCreate(
		    vLifecycleTask,
		    (const char *)"Lifecycle",
		    configMINIMAL_STACK_SIZE * 4,
		    NULL,
		    tskIDLE_PRIORITY + 3,
		    NULL) != pdPASS) {
		LOG_CRIT("startup", "Failed to create %s task!", "Lifecycle");
		abort();
	}

	/* Watchdog petting */
	watchdog_clearer_init();

	LOG_DEBUG("startup", "Starting FreeRTOS ...");
	vTaskStartScheduler();

	LOG_CRIT("startup", "FreeRTOS did not start due to unknown error!");
	abort();
}
