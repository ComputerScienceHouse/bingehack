#ifndef CONFIGFILE_H
#define CONFIGFILE_H

// The following libaraies must be included for this header to work
#if 0
#include <stdbool.h>
#include <libconfig.h>
#endif

#define config _get_config()
config_t *_get_config();

void configfile_init();

#endif
