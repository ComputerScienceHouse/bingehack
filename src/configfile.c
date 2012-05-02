#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <libconfig.h>
#include <glib.h>

#include "configfile.h"
#include "hack.h"

#define NETHACK_CONFIGFILE_ERROR nethack_configfile_error_quark()

GQuark nethack_configfile_error_quark() {
	return g_quark_from_static_string("nethack-configfile-error-quark");
}

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

static bool configfile_read_file( const char *filename, GError **error ) {
	if( filename[0] == '$' ) {
		const char *varname = filename + 1;
		filename = getenv(varname);
		if( filename == NULL ) {
			g_set_error(error,
				NETHACK_CONFIGFILE_ERROR,
				NETHACK_CONFIGFILE_ERROR_VARIABLE_UNSET,
				"Variable \"%s\" is not set",
				varname);
			return false;
		}
	}

	FILE *stream;
	if( (stream = fopen(filename, "r")) == NULL ) {
		g_set_error(error,
			NETHACK_CONFIGFILE_ERROR,
			(errno == ENOENT ?
			 NETHACK_CONFIGFILE_ERROR_LIBC_ENOENT :
			 NETHACK_CONFIGFILE_ERROR_LIBC),
			"Unable to read nethack configuration file %s: %s",
			filename,
			strerror(errno));
		return false;
	}
	if( config_read(&_config, stream) == CONFIG_FALSE ) {
		fclose(stream);
		g_set_error(error,
			NETHACK_CONFIGFILE_ERROR,
			NETHACK_CONFIGFILE_ERROR_LIBCONFIG,
			"Error reading nethack configuration file %s at line %d: %s",
			filename,
			config_error_line(&_config),
			config_error_text(&_config));
		return false;
	}
	if( fclose(stream) == EOF ) {
		g_set_error(error,
			NETHACK_CONFIGFILE_ERROR,
			NETHACK_CONFIGFILE_ERROR_LIBC,
			"Unable to close nethack configuration file %s: %s",
			filename,
			strerror(errno));
		return false;
	}
	return true;
}

bool configfile_init() {
	config_init(&_config);
	static const char * const locations[] = CONFIGFILE_LOCATIONS;
	bool ret = false;
	GSList *errors = NULL;

	for( const char * const *location = locations; *location != NULL; location++ ) {
		GError *err = NULL;
		if( configfile_read_file(*location, &err) ) {
			g_assert(err == NULL);
			ret = true;
		} else {
			g_assert(err != NULL);
			errors =g_slist_append(errors, err);
			err = NULL;

		}
	}
	while( errors != NULL ) {
		GError *err = (GError *) errors->data;
		// Skip file not found or variable unset errors if we found any config file.
		if( !( ret == true &&
		   err->domain == NETHACK_CONFIGFILE_ERROR &&
		   (
			   err->code == NETHACK_CONFIGFILE_ERROR_LIBC_ENOENT ||
			   err->code == NETHACK_CONFIGFILE_ERROR_VARIABLE_UNSET
		   )
		))
			pline("%s", err->message);
		g_error_free(err);
		GSList *tmp = g_slist_remove_link(errors, errors);
		g_slist_free_1(errors);
		errors = tmp;
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
