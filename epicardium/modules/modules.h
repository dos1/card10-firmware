#ifndef MODULES_H
#define MODULES_H

#include "FreeRTOS.h"
#include "gpio.h"
#include "modules/mutex.h"

#include <stdint.h>
#include <stdbool.h>

/* ---------- Panic -------------------------------------------------------- */
void panic(const char *format, ...)
	__attribute__((noreturn, format(printf, 1, 2)));

/* ---------- Dispatcher --------------------------------------------------- */
void vApiDispatcher(void *pvParameters);
void dispatcher_mutex_init(void);
extern struct mutex api_mutex;
extern TaskHandle_t dispatcher_task_id;

/* ---------- Hardware Init & Reset ---------------------------------------- */
int hardware_early_init(void);
int hardware_init(void);
int hardware_reset(void);

/* ---------- Lifecycle ---------------------------------------------------- */
void vLifecycleTask(void *pvParameters);
void return_to_menu(void);

/* ---------- Serial ------------------------------------------------------- */
#define SERIAL_READ_BUFFER_SIZE 128
#define SERIAL_WRITE_STREAM_BUFFER_SIZE 512
void serial_init();
void vSerialTask(void *pvParameters);
void serial_enqueue_char(char chr);
void serial_flush(void);
extern TaskHandle_t serial_task_id;
/* Turn off the print queue and do prints synchroneous from now on. */
void serial_return_to_synchronous();

// For the eSetBit xTaskNotify task semaphore trigger
enum serial_notify{
      SERIAL_WRITE_NOTIFY = 0x01,
      SERIAL_READ_NOTIFY  = 0x02,
};

/* ---------- LED Animation / Personal States ------------------------------ */
#define PERSONAL_STATE_LED 14
void vLedTask(void *pvParameters);
int personal_state_enabled();

/* ---------- PMIC --------------------------------------------------------- */
void vPmicTask(void *pvParameters);

/* ---------- Watchdog ----------------------------------------------------- */
void watchdog_init();
void watchdog_clearer_init();

/* Critical battery voltage */
#define BATTERY_CRITICAL   3.40f

enum pmic_amux_signal {
	PMIC_AMUX_DISABLED    = 0x0,
	PMIC_AMUX_CHGIN_U     = 0x1,
	PMIC_AMUX_CHGIN_I     = 0x2,
	PMIC_AMUX_BATT_U      = 0x3,
	PMIC_AMUX_BATT_CHG_I  = 0x4,
	PMIC_AMUX_BATT_DIS_I  = 0x5,
	PMIC_AMUX_BATT_NULL_I = 0x6,
	PMIC_AMUX_THM_U       = 0x7,
	PMIC_AMUX_TBIAS_U     = 0x8,
	PMIC_AMUX_AGND_U      = 0x9,
	PMIC_AMUX_SYS_U       = 0xA,
	_PMIC_AMUX_MAX,
};

/*
 * Read a value from the PMIC's AMUX.  The result is already converted into its
 * proper unit.  See the MAX77650 datasheet for details.
 */
int pmic_read_amux(enum pmic_amux_signal sig, float *result);


/* ---------- BLE ---------------------------------------------------------- */
void vBleTask(void *pvParameters);
bool ble_shall_start(void);
void ble_uart_write(uint8_t *pValue, uint8_t len);

/* ---------- Hardware (Peripheral) Locks ---------------------------------- */
void hwlock_init(void);

enum hwlock_periph {
	HWLOCK_I2C = 0,
	HWLOCK_ADC,
	HWLOCK_LED,
	HWLOCK_SPI_ECG,
	_HWLOCK_MAX,
};

int hwlock_acquire_timeout(enum hwlock_periph p, TickType_t wait);
void hwlock_acquire(enum hwlock_periph p);
int hwlock_acquire_nonblock(enum hwlock_periph p);
void hwlock_release(enum hwlock_periph p);

/* ---------- Display ------------------------------------------------------ */
/* Forces an unlock of the display. Only to be used in Epicardium */
void disp_forcelock();

/* ---------- BHI160 ------------------------------------------------------- */
#define BHI160_FIFO_SIZE             128
#define BHI160_MUTEX_WAIT_MS          50
void vBhi160Task(void *pvParameters);

/* ---------- MAX30001 ----------------------------------------------------- */
void vMAX30001Task(void *pvParameters);
void max30001_mutex_init(void);

/* ---------- GPIO --------------------------------------------------------- */
extern gpio_cfg_t gpio_configs[];


/* ---------- Sleep -------------------------------------------------------- */
void sleep_deepsleep(void);
#endif /* MODULES_H */
