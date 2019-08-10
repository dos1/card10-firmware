#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"
#include "leds.h"

static char *stack_top;

#if MICROPY_ENABLE_GC
static char heap[4096];
#endif

int mp_hal_stdin_rx_chr(void);

int main(int argc, char **argv)
{
	int stack_dummy;
	stack_top = (char *)&stack_dummy;
	leds_init();

#if MICROPY_ENABLE_GC
	gc_init(heap, heap + sizeof(heap));
#endif

	mp_init();
	pyexec_friendly_repl();
	mp_deinit();
	return 0;
}

void gc_collect(void)
{
	// WARNING: This gc_collect implementation doesn't try to get root
	// pointers from CPU registers, and thus may function incorrectly.
	void *dummy;
	gc_collect_start();
	gc_collect_root(
		&dummy,
		((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t)
	);
	gc_collect_end();
	gc_dump_info();
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename)
{
	mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path)
{
	return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs)
{
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val)
{
	while (1)
		;
}

void NORETURN __fatal_error(const char *msg)
{
	while (1)
		;
}

#ifndef NDEBUG
void MP_WEAK
__assert_func(const char *file, int line, const char *func, const char *expr)
{
	printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
	__fatal_error("Assertion failed");
}
#endif
