#include "epicardium.h"
#include "modules/modules.h"
#include "modules/log.h"

#include "portexpander.h"
#include "MAX77650-Arduino-Library.h"

#include <stdint.h>

static const uint8_t pin_mask[] = {
	[BUTTON_LEFT_BOTTOM]  = 1 << 5,
	[BUTTON_RIGHT_BOTTOM] = 1 << 3,
	[BUTTON_RIGHT_TOP]    = 1 << 6,
};

uint8_t epic_buttons_read(uint8_t mask)
{
	uint8_t ret = 0;

	hwlock_acquire(HWLOCK_I2C);
	if (portexpander_detected() && (mask & 0x7)) {
		/*
		 * Not using PB_Get() here as that performs one I2C transaction
		 * per button.
		 */
		uint8_t pin_status = ~portexpander_in_get(0xFF);

		for (uint8_t m = 1; m < 0x8; m <<= 1) {
			if (mask & m && pin_status & pin_mask[m]) {
				ret |= m;
			}
		}
	}

	if (mask & BUTTON_RESET && MAX77650_getDebounceStatusnEN0()) {
		ret |= BUTTON_RESET;
	}

	hwlock_release(HWLOCK_I2C);
	return ret;
}
