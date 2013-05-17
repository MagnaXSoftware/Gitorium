#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "common.h"

#include <libconfig.h>

#define RC_FILE "/etc/gitorium/config.cfg"

config_t aCfg;

void gitorium__config_close(void);
void gitorium__config_init(void);
int gitorium__repo_config_load(config_t *cfg);

#endif // CONFIG_H_INCLUDED
