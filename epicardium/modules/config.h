#ifndef EPICARDIUM_MODULES_CONFIG_H_INCLUDED
#define EPICARDIUM_MODULES_CONFIG_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

//initialize configuration values and load card10.cfg
void load_config(void);

// returns default_value if not found or invalid
bool config_get_boolean_with_default(const char *key, bool default_value);
int config_get_integer_with_default(const char *key, int default_value);

// returns dflt if not found, otherwise same pointer as buf
char *config_get_string_with_default(const char *key, char *buf, size_t buf_len, char *dflt);

#endif//EPICARDIUM_MODULES_CONFIG_H_INCLUDED
