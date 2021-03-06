#include "bootloader.h"
/* Autogenerated */
#include "splash-screen.h"
#include "card10-version.h"

#include "gfx.h"
#include "display.h"

static void bootloader_display_splash(void)
{
	gfx_copy_region(
		&display_screen,
		0,
		0,
		160,
		80,
		GFX_RLE_MONO,
		sizeof(splash),
		(const void *)(splash)
	);
	gfx_update(&display_screen);
}

/*
 * Initialize the display.
 */
void bootloader_display_init(void)
{
	bootloader_display_splash();
}

/*
 * Show the bootloader version on the display.
 */
void bootloader_display_header(void)
{
	gfx_clear(&display_screen);

	Color white = gfx_color(&display_screen, WHITE);
	bootloader_display_line(0, "Bootloader", white);
	bootloader_display_line(1, __DATE__, white);
	bootloader_display_line(2, CARD10_VERSION, white);
}

void bootloader_display_error(char *errtype, char *line1, char *line2)
{
	gfx_clear(&display_screen);

	Color red    = gfx_color(&display_screen, RED);
	Color yellow = gfx_color(&display_screen, YELLOW);
	Color white  = gfx_color(&display_screen, WHITE);

	bootloader_display_line(0, "[FATAL ERROR]", red);
	bootloader_display_line(1, errtype, yellow);
	bootloader_display_line(2, CARD10_VERSION, white);
	bootloader_display_line(3, line1, white);
	bootloader_display_line(4, line2, white);
}

/*
 * Display a line of text on the display.
 */
void bootloader_display_line(int line, char *string, uint16_t color)
{
	Color black = gfx_color(&display_screen, BLACK);
	gfx_puts(&Font16, &display_screen, 0, 16 * line, string, color, black);
	gfx_update(&display_screen);
}
