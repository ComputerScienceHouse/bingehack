#ifndef MYSQL_H
#define MYSQL_H

typedef struct {
	void *handle;

	// MySQL library functions
	MYSQL *(*init)( MYSQL * );
	MYSQL *(*real_connect)( MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag );
	int (*real_query)( MYSQL *mysql, const char *stmt_str, unsigned long length );
	MYSQL_RES *(*use_result)( MYSQL *mysql );
	void (*free_result)( MYSQL_RES *result );
	MYSQL_ROW (*fetch_row)( MYSQL_RES *result );
	const char *(*error)( MYSQL *mysql );
	int (*ping)( MYSQL *mysql );
	int (*options)( MYSQL *mysql, enum mysql_option option, const void *arg );
	my_ulonglong (*num_rows)( MYSQL_RES *result );
	void (*close)( MYSQL *mysql );
	unsigned long (*real_escape_string)( MYSQL *mysql, char *to, const char *from, unsigned long length );
} mysql_t;

bool mysql_library_startup();
bool mysql_library_shutdown();
bool mysql_library_available();
char *mysql_library_escape_string( MYSQL *db, const char *str );

mysql_t *_get_mysql();
#define mysql (*_get_mysql())

#endif
