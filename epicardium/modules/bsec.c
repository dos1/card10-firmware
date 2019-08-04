/* Adapted from  bsec_iot_example.c and bsec_iot_ulp_plus_example.c */

#include "card10.h"
#include "bosch.h"

#include "epicardium.h"
#include "modules.h"
#include "modules/log.h"

#include "FreeRTOS.h"
#include "task.h"

#include "max32665.h"
#include "gcr_regs.h"
#include <stdio.h>

/**********************************************************************************************************************/
/* header files */
/**********************************************************************************************************************/
/* card10: ULP example specific: */
/* BSEC configuration files are available in the config/ folder of the release package. Please chose a configuration file with 3s maximum time between `bsec_sensor_control()` calls */
#include "bsec_integration.h"

/**********************************************************************************************************************/
/* functions */
/**********************************************************************************************************************/

/*!
 * @brief           Capture the system time in microseconds
 *
 * @return          system_current_time    current system timestamp in microseconds
 */
int64_t get_timestamp_us()
{
	int tick = xTaskGetTickCount();
	return tick * 1000;
}

/*!
 * @brief           Handling of the ready outputs
 *
 * @param[in]       timestamp       time in nanoseconds
 * @param[in]       iaq             IAQ signal
 * @param[in]       iaq_accuracy    accuracy of IAQ signal
 * @param[in]       temperature     temperature signal
 * @param[in]       humidity        humidity signal
 * @param[in]       pressure        pressure signal
 * @param[in]       raw_temperature raw temperature signal
 * @param[in]       raw_humidity    raw humidity signal
 * @param[in]       gas             raw gas sensor signal
 * @param[in]       bsec_status     value returned by the bsec_do_steps() call
 *
 * @return          none
 */
void output_ready(
	int64_t timestamp,
	float iaq,
	uint8_t iaq_accuracy,
	float temperature,
	float humidity,
	float pressure,
	float raw_temperature,
	float raw_humidity,
	float gas,
	bsec_library_return_t bsec_status,
	float static_iaq,
	float co2_equivalent,
	float breath_voc_equivalent
) {
	return;
	printf("bosch data time: %u, iaq: %u, iaq_a: %u, temp10: %u, hum10: %u, pres: %u, raw_temp10: %u, raw_hum10: %u, gas: %u, static_iaq: %u, co21e6: %u, breath_voc1e6: %u\n",
	       (unsigned int)(timestamp / 9e6),
	       (unsigned int)(iaq),
	       (unsigned int)(iaq_accuracy),
	       (unsigned int)(temperature * 10),
	       (unsigned int)(humidity * 10),
	       (unsigned int)(pressure),
	       (unsigned int)(raw_temperature * 10),
	       (unsigned int)(raw_humidity * 10),
	       (unsigned int)(gas),
	       (unsigned int)(static_iaq),
	       (unsigned int)(co2_equivalent * 1e6),
	       (unsigned int)(breath_voc_equivalent * 1e6));
}

static int bsec_load(char *path, uint8_t *buffer, uint32_t n_buffer)
{
	uint32_t len = 0;
	int fd, res;

	printf("BSEC load %s %d\n", path, (int)n_buffer);

	if ((fd = epic_file_open(path, "r")) < 0) {
		printf("Open failed\n");
		return 0;
	}

	uint32_t header;
	if ((res = epic_file_read(fd, &header, sizeof(header))) !=
	    sizeof(header)) {
		printf("Header failed\n");
		goto done;
	}

	if (header > n_buffer) {
		printf("Too large\n");
		goto done;
	}

	if (epic_file_read(fd, buffer, header) != header) {
		printf("Read failed\n");
		goto done;
	}

	len = header;

	printf("Success\n");
done:
	epic_file_close(fd);
	return len;
}
/*!
 * @brief           Load previous library state from non-volatile memory
 *
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to state_buffer
 */
uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
	return bsec_load("bsec_iaq.state", state_buffer, n_buffer);
}

/*!
 * @brief           Save library state to non-volatile memory
 *
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 *
 * @return          none
 */
void state_save(const uint8_t *state_buffer, uint32_t length)
{
	int fd, res;

	printf("BSEC state_save %d\n", (int)length);

	if ((fd = epic_file_open("bsec_iaq.state", "w")) < 0) {
		printf("Open failed\n");
		return;
	}

	uint32_t header = length;
	if ((res = epic_file_write(fd, &header, sizeof(header))) !=
	    sizeof(header)) {
		printf("Header failed\n");
		goto done;
	}

	if (epic_file_write(fd, state_buffer, header) != header) {
		printf("Write failed\n");
		goto done;
	}

	printf("Success\n");
done:
	epic_file_close(fd);
}

/*!
 * @brief           Load library config from non-volatile memory
 *
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to config_buffer
 */
uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
	return bsec_load("bsec_iaq.config", config_buffer, n_buffer);
}

#if 0
/* card10: ULP example specific: */
// Attach a button (or other) interrupt here to the ulp_plus_button_press() handler function to
// enable this interrupt to trigger a ULP plus

/*!
 * @brief           Interrupt handler for press of a ULP plus button
 *
 * @return          none
 */
void ulp_plus_button_press()
{
    /* We call bsec_update_subscription() in order to instruct BSEC to perform an extra measurement at the next
     * possible time slot
     */

    bsec_sensor_configuration_t requested_virtual_sensors[1];
    uint8_t n_requested_virtual_sensors = 1;
    bsec_sensor_configuration_t required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
    uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;
    bsec_library_return_t status = BSEC_OK;

    /* To trigger a ULP plus, we request the IAQ virtual sensor with a specific sample rate code */
    requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
    requested_virtual_sensors[0].sample_rate = BSEC_SAMPLE_RATE_ULP_MEASUREMENT_ON_DEMAND;

    /* Call bsec_update_subscription() to enable/disable the requested virtual sensors */
    status = bsec_update_subscription(requested_virtual_sensors, n_requested_virtual_sensors, required_sensor_settings,
         &n_required_sensor_settings);

    /* The status code would tell is if the request was accepted. It will be rejected if the sensor is not already in
     * ULP mode, or if the time difference between requests is too short, for example. */
}
#endif

/*!
 * @brief       Main function which configures BSEC library and then reads and processes the data from sensor based
 *              on timer ticks
 *
 * @return      result of the processing
 */
void vBSECTask(void *pvParameters)
{
	return_values_init ret;
	/* Call to the function which initializes the BSEC library */
#if 0
    /* Switch on ultra_low-power mode and provide no temperature offset */
    ret = bsec_iot_init(BSEC_SAMPLE_RATE_ULP, 0.0f, card10_bosch_i2c_write, card10_bosch_i2c_read, card10_bosch_delay, state_load, config_load);
#else
	ret = bsec_iot_init(
		BSEC_SAMPLE_RATE_LP,
		0.0f,
		card10_bosch_i2c_write,
		card10_bosch_i2c_read,
		card10_bosch_delay,
		state_load,
		config_load
	);
#endif
	if (ret.bme680_status) {
		/* Could not intialize BME680 or BSEC library */
		while (1)
			;
	} else if (ret.bsec_status) {
		/* Could not intialize BSEC library */
		while (1)
			;
	}
	/* Call to endless loop function which reads and processes data based on sensor settings */
#if 0
    /* State is saved every 10.000 samples, which means every 100 * 300 secs = 500 minutes  */
    bsec_iot_loop(sleep, get_timestamp_us, output_ready, state_save, 100);
#else
	/* State is saved every 10.000 samples, which means every 10.000 * 3 secs = 500 minutes  */
	//bsec_iot_loop(card10_bosch_delay, get_timestamp_us, output_ready, state_save, 10000);

	/* State is saved every 100 samples, which means every 1200 * 3 secs = 60 minutes  */
	bsec_iot_loop(
		card10_bosch_delay,
		get_timestamp_us,
		output_ready,
		state_save,
		1200
	);
#endif
	while (1)
		;
}
