#include <string.h>

#include "py/obj.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "extmod/machine_i2c.h"
#include "i2c.h"

#define MICROPY_HW_I2C1_NAME "X"
#define MICROPY_HW_I2C2_NAME "Y"

STATIC const mp_obj_type_t machine_hard_i2c_type;

typedef struct _machine_hard_i2c_obj_t {
    mp_obj_base_t base;
    mxc_i2c_regs_t *i2c;
} machine_hard_i2c_obj_t;

STATIC const machine_hard_i2c_obj_t machine_hard_i2c_obj[2] = {
    [0] = {{&machine_hard_i2c_type}, MXC_I2C1_BUS0},
    [1] = {{&machine_hard_i2c_type}, MXC_I2C1_BUS1},
};


int machine_hard_i2c_transfer(mp_obj_base_t *self_in, uint16_t addr, size_t n, mp_machine_i2c_buf_t *bufs, unsigned int flags) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (flags & MP_MACHINE_I2C_FLAG_READ) {
	return I2C_MasterRead(self->i2c, addr, bufs->buf, bufs->len, !(flags & MP_MACHINE_I2C_FLAG_STOP));
    } else {
	return I2C_MasterRead(self->i2c, addr, bufs->buf, bufs->len, !(flags & MP_MACHINE_I2C_FLAG_STOP));
    }
}


/******************************************************************************/
/* MicroPython bindings for machine API                                       */

mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // parse args
    enum { ARG_id };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // work out i2c bus
    int i2c_id = 0;
    if (!mp_obj_is_str(args[ARG_id].u_obj)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "Provide I2C bus as string"));
    }

    const char *port = mp_obj_str_get_str(args[ARG_id].u_obj);
    if (strcmp(port, MICROPY_HW_I2C1_NAME) == 0) {
        i2c_id = 1;
    } else if (strcmp(port, MICROPY_HW_I2C2_NAME) == 0) {
        i2c_id = 2;
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
            "I2C(%s) doesn't exist", port));
    }

    // get static peripheral object
    machine_hard_i2c_obj_t *self = (machine_hard_i2c_obj_t*)&machine_hard_i2c_obj[i2c_id - 1];
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .transfer = machine_hard_i2c_transfer,
};

STATIC const mp_obj_type_t machine_hard_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .make_new = machine_hard_i2c_make_new,
    .protocol = &machine_hard_i2c_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_soft_i2c_locals_dict,
};

