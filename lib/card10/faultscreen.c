#include "faultscreen.h"

#include <stdio.h>
#include "Fonts/fonts.h"
#include "faultsplash.h"
#include "display.h"
#include "gfx.h"

#include "max32665.h"
#include "nvic_table.h"
#include "core_cmFunc.h"
#include "core1.h"

struct regs_t {
	unsigned int pc;
	unsigned int sp;
	unsigned int lr;
} static regs;

#define LOAD_REGS()                                                            \
	__asm__ __volatile__(                                                  \
		"ldr %0, [sp, #24] \n"                                         \
		"ldr %1, [sp, #20] \n"                                         \
		"mov %2, sp \n"                                                \
		: "=r"(regs.pc), "=r"(regs.lr), "=r"(regs.sp))

static void generic_handler(const char *reason)
{
	char lines[3][64];

	Core1_Stop();
	__disable_irq();

	snprintf(lines[0], sizeof(lines[0]), "pc: 0x%08x", regs.pc);
	snprintf(lines[1], sizeof(lines[1]), "sp: 0x%08x", regs.sp);
	snprintf(lines[2], sizeof(lines[2]), "lr: 0x%08x", regs.lr);

	gfx_copy_region(
		&display_screen,
		0,
		0,
		160,
		80,
		GFX_RLE_MONO,
		faultsplash_rle_len,
		faultsplash_rle
	);

	Color black = gfx_color(&display_screen, BLACK);
	Color white = gfx_color(&display_screen, WHITE);

	gfx_puts(&Font12, &display_screen, 52, 32, reason, white, black);
	gfx_puts(&Font12, &display_screen, 52, 44, lines[0], white, black);
	gfx_puts(&Font12, &display_screen, 52, 56, lines[1], white, black);
	gfx_puts(&Font12, &display_screen, 52, 68, lines[2], white, black);
	gfx_update(&display_screen);
}

static void NMI_Handler_(void)
{
	LOAD_REGS();
	generic_handler("NMI Fault");
}

static void HardFault_Handler_(void)
{
	LOAD_REGS();
	generic_handler("HardFault");
}

static void MemManage_Handler_(void)
{
	LOAD_REGS();
	generic_handler("MemMgmtFault");
}

static void BusFault_Handler_(void)
{
	LOAD_REGS();
	generic_handler("BusFault");
}

static void UsageFault_Handler_(void)
{
	LOAD_REGS();
	generic_handler("UsageFault");
}

void card10_set_fault_handlers(void)
{
	NVIC_SetVector(NonMaskableInt_IRQn, NMI_Handler_);
	NVIC_SetVector(HardFault_IRQn, HardFault_Handler_);
	NVIC_SetVector(MemoryManagement_IRQn, MemManage_Handler_);
	NVIC_SetVector(BusFault_IRQn, BusFault_Handler_);
	NVIC_SetVector(UsageFault_IRQn, UsageFault_Handler_);
}
