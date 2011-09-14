#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#include <libconfig.h>

#include "configfile.h"
#include "hack.h"

config_t *_get_config() {
	static config_t _config;
	return &_config;
}

__attribute__((destructor)) static void configfile_destroy() {
	config_destroy(config);
}

void configfile_init() {
	config_init(config);
	if( config_read_file(config, CONFIGFILE_LOCATION) == CONFIG_FALSE ) {
		config_destroy(config);
		impossible("%s:%d %s", config_error_file(config), config_error_line(config), config_error_text(config));
	}
}
