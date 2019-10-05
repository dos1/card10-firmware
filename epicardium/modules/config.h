#ifndef EPICARDIUM_MODULES_CONFIG_H_INCLUDED
#define EPICARDIUM_MODULES_CONFIG_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

//initialize configuration values and load card10.cfg
void load_config(void);

// returns default_value if not found or invalid
bool config_get_boolean(const char *key, bool default_value);
int config_get_integer(const char *key, int default_value);

// returns NULL if not found, otherwise same pointer as buf
char *config_get_string(const char *key, char *buf, size_t buf_len);

#endif//EPICARDIUM_MODULES_CONFIG_H_INCLUDED
