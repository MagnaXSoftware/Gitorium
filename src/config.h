#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <stdlib.h>
#include <libconfig.h>
#include <string.h>

#include "common.h"

#define RC_FILE "/etc/gitorium/config.cfg"

config_t aCfg;

int gitorium_config_close(void);
int gitorium_config_save(void);
int gitorium_config_init(void);

#endif // CONFIG_H_INCLUDED
