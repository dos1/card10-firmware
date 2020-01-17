#include "leds.h"
#include "pmic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "epicardium.h"
#include "modules.h"

#include <stdbool.h>

/*
 * TODO: create smth like vTaskDelay(pdMS_TO_TICKS(//put ms here)) for us,
 *       remove blocking delay from /lib/leds.c to avoid process blocking
 */

#define NUM_LEDS 15 /* Take from lib/card10/leds.c */

static void do_update(void)
{
	hwlock_acquire(HWLOCK_LED);

	leds_update_power();
	leds_update();

	hwlock_release(HWLOCK_LED);
}

void epic_leds_set(int led, uint8_t r, uint8_t g, uint8_t b)
{
	if (led == PERSONAL_STATE_LED && personal_state_enabled())
		return;

	leds_prep(led, r, g, b);
	do_update();
}

void epic_leds_set_hsv(int led, float h, float s, float v)
{
	if (led == PERSONAL_STATE_LED && personal_state_enabled())
		return;

	leds_prep_hsv(led, h, s, v);
	do_update();
}

void epic_leds_prep(int led, uint8_t r, uint8_t g, uint8_t b)
{
	if (led == PERSONAL_STATE_LED && personal_state_enabled())
		return;

	leds_prep(led, r, g, b);
}

int epic_leds_get_rgb(int led, uint8_t *rgb)
{
	if (led == PERSONAL_STATE_LED && personal_state_enabled())
		return -EPERM;
	if (led < 0 || led >= NUM_LEDS)
		return -EINVAL;

	leds_get_rgb(led, rgb);
	return 0;
}

void epic_leds_prep_hsv(int led, float h, float s, float v)
{
	if (led == PERSONAL_STATE_LED && personal_state_enabled())
		return;

	leds_prep_hsv(led, h, s, v);
}

void epic_leds_set_all(uint8_t *pattern_ptr, uint8_t len)
{
	uint8_t(*pattern)[3] = (uint8_t(*)[3])pattern_ptr;
	for (int i = 0; i < len; i++) {
		if (i == PERSONAL_STATE_LED && personal_state_enabled())
			continue;

		leds_prep(i, pattern[i][0], pattern[i][1], pattern[i][2]);
	}
	do_update();
}

void epic_leds_set_all_hsv(float *pattern_ptr, uint8_t len)
{
	float(*pattern)[3] = (float(*)[3])pattern_ptr;
	for (int i = 0; i < len; i++) {
		if (i == PERSONAL_STATE_LED && personal_state_enabled())
			continue;

		leds_prep_hsv(i, pattern[i][0], pattern[i][1], pattern[i][2]);
	}
	do_update();
}

void epic_leds_dim_top(uint8_t value)
{
	leds_set_dim_top(value);
	if (personal_state_enabled() == 0) {
		hwlock_acquire(HWLOCK_I2C);
		leds_update();
		hwlock_release(HWLOCK_I2C);
	}
}

void epic_leds_dim_bottom(uint8_t value)
{
	leds_set_dim_bottom(value);
	if (personal_state_enabled() == 0) {
		hwlock_acquire(HWLOCK_I2C);
		leds_update();
		hwlock_release(HWLOCK_I2C);
	}
}

void epic_leds_set_rocket(int led, uint8_t value)
{
	hwlock_acquire(HWLOCK_I2C);
	pmic_set_led(led, value > 31 ? 31 : value);
	hwlock_release(HWLOCK_I2C);
}

int epic_leds_get_rocket(int led)
{
	int ret = 0;

	hwlock_acquire(HWLOCK_I2C);
	ret = pmic_get_led(led);
	hwlock_release(HWLOCK_I2C);
	return ret;
}

void epic_set_flashlight(bool power)
{
	hwlock_acquire(HWLOCK_I2C);
	leds_flashlight(power);
	hwlock_release(HWLOCK_I2C);
}

void epic_leds_update(void)
{
	do_update();
}

void epic_leds_set_powersave(bool eco)
{
	hwlock_acquire(HWLOCK_I2C);
	leds_powersave(eco);
	hwlock_release(HWLOCK_I2C);
}

void epic_leds_set_gamma_table(uint8_t rgb_channel, uint8_t gamma_table[256])
{
	leds_set_gamma_table(rgb_channel, gamma_table);
}

void epic_leds_clear_all(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < NUM_LEDS; i++) {
		if (i == PERSONAL_STATE_LED && personal_state_enabled())
			continue;

		leds_prep(i, r, g, b);
	}

	do_update();
}
