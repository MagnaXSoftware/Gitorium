#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "common.h"

#include <libconfig.h>

#define RC_FILE "/etc/gitorium/config.cfg"

config_t aCfg;

int gitorium_config_close(void);
int gitorium_config_init(void);

#endif // CONFIG_H_INCLUDED
