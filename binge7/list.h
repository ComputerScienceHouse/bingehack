#ifndef LIST_H
#define LIST_H

typedef struct list list;

typedef int (*compare)(void *v, const void *w);
typedef void (*free_function)( void *v, void *arg );
typedef void *(*copy_function)( void *v, void *arg );
typedef void *(*collect_function)( void *v, void *arg );
typedef int (*select_function)( void *v, void *arg );

list *list_new( void );
void list_destroy( list *l );
list *list_copy( list *l );
list *list_collect( list *l, collect_function, void *arg );
list *list_delete_if( list *l, select_function, void *arg );
list *list_select( list *l, select_function, void *arg );

void list_lock(list *l);
void list_unlock(list *l);

int list_count(list *l);
free_function list_set_free(list *l, free_function f, void *arg);
copy_function list_set_copy(list *l, copy_function f, void *arg);
void *list_shift(list *l);
void *list_unshift(list *l, void *v);
void *list_pop(list *l);
void *list_push(list *l, void *v);
void *list_remove(list *l);
void list_clear(list *l);
void list_sort(list *l, compare cmp);
void list_reverse(list *l);

void *list_reset(list *);
void *list_next(list *);
void *list_goto(list *l, const void *v, compare cmp);
void *list_goto_index(list *l, int i);

struct st_table;

list *list_from_st( struct st_table *t );
list *list_from_st_keys( struct st_table *t );

#endif
