#include "epicardium.h"

#include "py/builtin.h"
#include "py/obj.h"
#include "py/runtime.h"

static mp_obj_t mp_vibra_set(mp_obj_t state_obj)
{
	if (state_obj == mp_const_true) {
		epic_vibra_set(1);
	} else if (state_obj == mp_const_false) {
		epic_vibra_set(0);
	} else {
		mp_raise_TypeError("expected bool");
	}
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(vibra_set_obj, mp_vibra_set);

static mp_obj_t mp_vibra_vibrate(mp_obj_t a_obj)
{
	int millis = mp_obj_get_int(a_obj);
	epic_vibra_vibrate(millis);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(vibra_vibrate_obj, mp_vibra_vibrate);

static const mp_rom_map_elem_t vibra_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_vibra) },
	{ MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&vibra_set_obj) },
	{ MP_ROM_QSTR(MP_QSTR_vibrate), MP_ROM_PTR(&vibra_vibrate_obj) }
};
static MP_DEFINE_CONST_DICT(vibra_module_globals, vibra_module_globals_table);

// Define module object.
const mp_obj_module_t vibra_module = {
	.base    = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&vibra_module_globals,
};

/* Register the module to make it available in Python */
/* clang-format off */
MP_REGISTER_MODULE(MP_QSTR_vibra, vibra_module, MODULE_VIBRA_ENABLED);
