#include "epicardium.h"
#include "api/common.h"

#include "max32665.h"
#include "mxc_delay.h"
#include "tmr.h"

/* stdarg.h must be included before mpprint.h */
#include <stdarg.h>

#include "py/lexer.h"
#include "py/mpconfig.h"
#include "py/mperrno.h"
#include "py/mpstate.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mpprint.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Initialize everything for MicroPython */
void pycardium_hal_init(void)
{
	/* TMR5 is used for interrupts from Epicardium */
	NVIC_EnableIRQ(TMR5_IRQn);

	/*
	 * Enable UART RX Interrupt so Pycardium can sleep until
	 * a character becomes available.
	 */
	epic_interrupt_enable(EPIC_INT_UART_RX);

	/*
	 * Configure SysTick timer for 1ms period.
	 */
	SysTick_Config(SystemCoreClock / 1000);
}

/******************************************************************************
 * Serial Communication
 */

/* Receive single character */
int mp_hal_stdin_rx_chr(void)
{
	int chr;
	while ((chr = epic_uart_read_char()) < 0) {
		__WFI();
	}
	return chr;
}

/* Send a string */
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len)
{
	epic_uart_write_str(str, len);
}

/* Send a string, but replace \n with \n\r */
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len)
{
	/*
	 * Only print one line at a time.  Insert `\r` between lines so
	 * they are properly displayed on the serial console.
	 */
	size_t i, last = 0;
	for (i = 0; i < len; i++) {
		if (str[i] == '\n') {
			epic_uart_write_str(&str[last], i - last);
			epic_uart_write_str("\r", 1);
			last = i;
		}
	}
	epic_uart_write_str(&str[last], len - last);
}

/* Send a zero-terminated string */
void mp_hal_stdout_tx_str(const char *str)
{
	mp_hal_stdout_tx_strn(str, strlen(str));
}

/* Used by MicroPython for debug output */
int DEBUG_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = mp_vprintf(MP_PYTHON_PRINTER, fmt, args);
	va_end(args);
	return ret;
}

void __attribute__((noreturn)) sbrk_is_not_implemented___see_issue_44(void);
intptr_t _sbrk(int incr)
{
	sbrk_is_not_implemented___see_issue_44();
}

void epic_isr_ctrl_c(void)
{
	/* Taken from lib/micropython/micropython/lib/utils/interrupt_char.c */
	MP_STATE_VM(mp_pending_exception) =
		MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception));
#if MICROPY_ENABLE_SCHEDULER
	if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
		MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
	}
#endif
}

void mp_hal_set_interrupt_char(char c)
{
	if (c != '\xFF') {
		mp_obj_exception_clear_traceback(
			MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception))
		);
	}

	if (c == 0x03) {
		epic_interrupt_enable(EPIC_INT_CTRL_C);
	} else {
		epic_interrupt_disable(EPIC_INT_CTRL_C);
	}
}

/******************************************************************************
 * SysTick timer at 1000 Hz
 */

static volatile uint64_t systick_count = 0;

void SysTick_Handler(void)
{
	systick_count += 1;
}

/*
 * Get an absolute "timestamp" in microseconds.
 */
static uint64_t systick_get_us()
{
	uint32_t irqsaved = __get_PRIMASK();
	__set_PRIMASK(0);

	uint64_t counts_per_us = SystemCoreClock / 1000000;
	uint64_t us            = systick_count * 1000 +
		      (SysTick->LOAD - SysTick->VAL) / counts_per_us;

	__set_PRIMASK(irqsaved);

	return us;
}

static void systick_delay_precise(uint32_t us)
{
	/*
	 * Calculate how long the busy-spin needs to be.  As the very first
	 * instruction, read the current timer value to ensure as little skew as
	 * possible.
	 *
	 * Subtract 0.3us (constant_offset) to account for the duration of the
	 * calculations.
	 */
	uint32_t count_to_overflow = SysTick->VAL;
	uint32_t count_reload      = SysTick->LOAD;
	uint32_t clocks_per_us     = SystemCoreClock / 1000000;
	uint32_t constant_offset   = clocks_per_us * 3 / 10;
	uint32_t delay_count       = us * clocks_per_us - constant_offset;

	/*
	 * Calculate the final count for both paths.  Marked as volatile so the
	 * compiler can't move this into the branches and screw up the timing.
	 */
	volatile uint32_t count_final_direct = count_to_overflow - delay_count;
	volatile uint32_t count_final_underflow =
		count_reload - (delay_count - count_to_overflow);

	if (delay_count > count_to_overflow) {
		/*
		 * Wait for the SysTick to underflow and then count down
		 * to the final value.
		 */
		while (SysTick->VAL <= count_to_overflow ||
		       SysTick->VAL > count_final_underflow) {
			__NOP();
		}
	} else {
		/*
		 * Wait for the SysTick to count down to the final value.
		 */
		while (SysTick->VAL > count_final_direct) {
			__NOP();
		}
	}
}

static void systick_delay_sleep(uint32_t us)
{
	uint64_t final_time = systick_get_us() + (uint64_t)us - 2;

	while (1) {
		uint64_t now = systick_get_us();

		if (now >= final_time) {
			break;
		}

		/*
		 * Sleep with WFI if more than 1ms of delay is remaining.  The
		 * SysTick interrupt is guaranteed to happen within any timespan
		 * of 1ms.
		 *
		 * Use a critical section encompassing both the check and the
		 * WFI to prevent a race-condition where the interrupt happens
		 * just in between the check and WFI.
		 */
		uint32_t irqsaved = __get_PRIMASK();
		__set_PRIMASK(0);
		if ((now + 1000) < final_time) {
			__WFI();
		}
		__set_PRIMASK(irqsaved);

		/*
		 * Handle pending MicroPython 'interrupts'.  This call could
		 * potentially not return here when a handler raises an
		 * exception.  Those will propagate outwards and thus make the
		 * delay return early.
		 *
		 * One example of this happeing is the KeyboardInterrupt
		 * (CTRL+C) which will abort the running code and exit to REPL.
		 */
		mp_handle_pending();
	}
}

static void systick_delay(uint32_t us)
{
	if (us == 0)
		return;

	/*
	 * For very short delays, use the systick_delay_precise() function which
	 * delays with a microsecond accuracy.  For anything >1ms, use
	 * systick_delay_sleep() which puts the CPU to sleep when nothing is
	 * happening and also checks for MicroPython interrupts every now and
	 * then.
	 */
	if (us < 1000) {
		systick_delay_precise(us);
	} else {
		systick_delay_sleep(us);
	}
}

/******************************************************************************
 * Time & Delay
 */

void mp_hal_delay_ms(mp_uint_t ms)
{
	systick_delay(ms * 1000);
}

void mp_hal_delay_us(mp_uint_t us)
{
	systick_delay(us);
}

mp_uint_t mp_hal_ticks_ms(void)
{
	return (mp_uint_t)(systick_get_us() / 1000);
}

mp_uint_t mp_hal_ticks_us(void)
{
	return (mp_uint_t)systick_get_us();
}

/******************************************************************************
 * Fatal Errors
 */

extern NORETURN void *Reset_Handler(void);

void NORETURN nlr_jump_fail(void *val)
{
	char msg[] = " >>> nlr_jump_fail <<<\r\n";
	epic_uart_write_str(msg, sizeof(msg));

	Reset_Handler();
}

/******************************************************************************
 * TRNG
 */

int mp_hal_trng_read_int(void)
{
	int result;
	epic_trng_read((uint8_t *)&result, sizeof(result));
	return result;
}
