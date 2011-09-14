#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <libconfig.h>

#include "configfile.h"
#include "hack.h"

static config_t _config;
static config_t *_configptr = NULL;

config_t *_get_config() {
	return _configptr;
}

__attribute__((destructor)) static void configfile_destroy() {
	if( _configptr != NULL ) config_destroy(_configptr);
}

static bool configfile_read_file( const char *filename ) {
	if( filename[0] == '$' ) {
		const char *varname = filename + 1;
		filename = getenv(varname);
		if( filename == NULL ) return false;
	}

	FILE *stream;
	if( (stream = fopen(filename, "r")) == NULL ) {
		if( errno != ENOENT ) pline("Unable to read nethack configuration file %s: %s", filename, strerror(errno));
		return false;
	}
	if( config_read(&_config, stream) == CONFIG_FALSE ) {
		fclose(stream);
		pline("Error reading nethack configuration file %s at line %d: %s", filename, config_error_line(&_config), config_error_text(&_config));
		return false;
	}
	if( fclose(stream) == EOF ) {
		pline("Unable to close nethack configuration file %s: %s", filename, strerror(errno));
		return false;
	}
	return true;
}

void configfile_init() {
	config_init(&_config);
	static const char * const locations[] = CONFIGFILE_LOCATIONS;

	for( const char * const *location = locations; *location != NULL; location++ ) {
		configfile_read_file(*location);
	}
	_configptr = &_config;
}

// vim:set noexpandtab textwidth=120
