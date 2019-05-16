#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include <elektra.h>

typedef struct {
    kdb_boolean_t active;
    kdb_unsigned_long_t on_time;
    kdb_unsigned_long_t off_time;
    kdb_boolean_t show_invisible;
} ScreenBaseConfig;

#endif