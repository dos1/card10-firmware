#include "py/runtime.h"
#include "py/gc.h"
#include "lib/utils/pyexec.h"

#include "max32665.h"

static char *stack_top;
static char heap[4096];

int main(void)
{
	/* TODO: Replace this with a proper heap implementation */
	int stack_dummy;
	stack_top = (char *)&stack_dummy;

	/* TMR5 is used to notify on keyboard interrupt */
	NVIC_EnableIRQ(TMR5_IRQn);

	while (1) {
		gc_init(heap, heap + sizeof(heap));

		mp_init();
		pyexec_friendly_repl();
		mp_deinit();
	}
}

void gc_collect(void)
{
	/* TODO: Replace this with a proper heap implementation */
	void *dummy;

	gc_collect_start();
	gc_collect_root(
		&dummy,
		((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t)
	);
	gc_collect_end();
}
