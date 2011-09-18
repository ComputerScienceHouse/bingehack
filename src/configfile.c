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

bool configfile_available() {
	return _configptr != NULL;
}

void configfile_destroy() {
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

bool configfile_init() {
	config_init(&_config);
	static const char * const locations[] = CONFIGFILE_LOCATIONS;
	bool ret = false;

	for( const char * const *location = locations; *location != NULL; location++ ) {
		if( configfile_read_file(*location) ) ret = true;
	}
	_configptr = &_config;
	return ret;
}

bool configfile_get_string( const char *path, const char **str ) {
	if( config_lookup_string(config, path, str) == CONFIG_FALSE ) {
		pline("Config setting %s not found or wrong type", path);
		return false;
	}
	return true;
}

// vim:set noexpandtab textwidth=120
