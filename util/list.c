#include "list.h"

#include <stdlib.h>

typedef struct node node;

struct node {
  void *value;
  node *next;
};

static node *node_new(void *value, node *next);
static void node_destroy(node *n);

struct list {
  int count;
  node *first;
  node *current;
  free_function free;
  void *free_arg;
  copy_function copy;
  void *copy_arg;
};

list *
list_new( void )
{
	list *l = (list *) calloc(sizeof(list), 1);
	if( !l )
		return NULL;

  return l;
}

void
list_destroy( list *l )
{
  list_clear(l);

  /* possible race condition, maybe we want to pull the mutex out
   * and unlock/destroy it *after* freeing */

  free(l);
}

void
list_clear( list *l )
{
  list_reset(l);

  while( l->count )
    list_shift(l);

  list_reset(l);
}

list *
list_copy( list *l )
{
  int i;
  list *n = list_new();

  list_reset(l);

  list_set_free(n, l->free, l->free_arg);
  list_set_copy(n, l->copy, l->copy_arg);
  for( i = 0; i < list_count(l); i++ ) {
    void *v = list_next(l);

    if( l->copy )
      v = l->copy(v, l->copy_arg);

    list_push(n, v);
  }

  list_reset(n);

  return n;
}

list *
list_collect( list *l, collect_function f, void *arg )
{
  list *n;
  copy_function scopy = l->copy;
  void *scopy_arg = l->copy_arg;

  /* set up copy with collect function */
  list_set_copy(l, (copy_function) f, arg);
  n = list_copy(l);

  /* restore the copy function */
  list_set_copy(l, scopy, scopy_arg);

  /* clean up initialization of the other list */
  list_set_free(n, NULL, NULL);
  list_set_copy(n, NULL, NULL);

  return n;
}

list *
list_delete_if( list *l, select_function f, void *arg )
{
  list_reset(l);

  while( l->current ) {
    if( f(l->current->value, arg) )
      list_remove(l);
    else
      list_next(l);
  }

  list_reset(l);

  return l;
}

list *
list_select( list *l, select_function f, void *arg )
{
  list *n;

  n = list_new();
  list_set_free(n, l->free, l->free_arg);
  list_set_copy(n, l->copy, l->copy_arg);

  list_reset(l);

  while( l->current ) {
    if( f(l->current->value, arg) )
      list_unshift(n, l->current->value);

    list_next(l);
  }

  list_reset(l);

  list_reset(n);

  return n;
}

int
list_count( list *l )
{
  return l->count;
}

free_function
list_set_free( list *l, free_function f, void *arg )
{
  free_function old;

  old = l->free;

  l->free = f;
  l->free_arg = arg;

  return old;
}

copy_function
list_set_copy( list *l, copy_function f, void *arg )
{
  copy_function old;

  old = l->copy;

  l->copy = f;
  l->copy_arg = arg;

  return old;
}

void *
list_shift( list *l )
{
  node *n;
  void *v;

  if( l->count == 0 )
    return NULL;

  n = l->first ? l->first->next : NULL;
  v = l->first ? l->first->value : NULL;

  node_destroy(l->first);
  l->first = n;

  l->count--;

  if( l->free )
    l->free(v, l->free_arg);

  return v;
}

void *
list_unshift( list *l, void *v )
{
  node *n;

  n = node_new(v, l->first);
  if( !n )
    return NULL;

  l->first = n;
  l->count++;

  return v;
}

void *
list_push( list *l, void *v )
{
  node *last, *n;

  n = node_new(v, NULL);
  if( !n )
    return NULL;

  if( !l->first ) {
    l->first = n;
  } else {
    last = l->first;
    while( last->next )
      last = last->next;
    last->next = n;
  }

  l->count++;

  return v;
}

void *
list_pop( list *l )
{
  node *n;
  void *v;

  if( l->count == 0 )
    return NULL;

  if( l->count == 1 ) {
    v = list_shift(l);
    return v;
  }

  n = l->first ? l->first : NULL;

  while( n->next->next ) {
    n = n->next;
  }

  v = n->next->value;

  node_destroy(n->next);
  n->next = NULL;

  l->count--;

  if( l->free )
    l->free(v, l->free_arg);

  return v;
}

void *
list_remove( list *l )
{
  node **link, *n;
  void *v;

  link = &l->first;

  if( !l->first || !l->current )
    return NULL;

  while( *link && *link != l->current )
    link = &(*link)->next;

  if( !*link )
    return NULL;

  n = l->current;
  l->current = n->next;
  *link = l->current;

  v = n->value;
  node_destroy(n);

  l->count--;

  if( l->free )
    l->free(v, l->free_arg);

  return v;
}

static int
direct_compare( void *a, const void *b )
{
  return a - b;
}

void *
list_goto( list *l, const void *v, compare cmp )
{
  void *u;

  list_reset(l);

  if( !cmp )
    cmp = direct_compare;

  while( l->current && cmp(l->current->value, v) )
    list_next(l);

  u = l->current ? l->current->value : NULL;

  return u;
}

void *
list_goto_index( list *l, int i )
{
  void *v;

  if( i < 0 || i > (l->count - 1) )
    return NULL;

  list_reset(l);

  while( i-- )
    list_next(l);

  v = l->current ? l->current->value : NULL;

  return v;
}

void *
list_reset( list *l )
{
  void *v;

  l->current = l->first;

  v = l->current ? l->current->value : NULL;

  return v;
}

void *
list_next( list *l )
{
  void *v = NULL;

  if( l->current ) {
    v = l->current->value;
    l->current = l->current->next;
  }

  return v;
}

static node *
node_new( void *value, node *next )
{
	node *n = (node *) calloc(sizeof(node), 1);
	if( !n )
		return NULL;

  n->value = value;
  n->next = next;

  return n;
}

static void
node_destroy( node *n )
{
  free(n);
}

static compare list_sort_cmp = NULL;

int
list_qsort_compare( const void *v1, const void *v2 )
{
  void **vp1 = (void **) v1, **vp2 = (void **) v2;

  if( !list_sort_cmp )
    return -1;

  return (*list_sort_cmp)(*vp1, *vp2);
}

void
list_sort( list *l, compare cmp )
{
  void **sarray, **saptr;
  node *c;

  if( !l || l->count <= 1 ) {
    if( l ) list_reset(l);
    return;
  }

  sarray = (void **) malloc(sizeof(void*) * l->count);
  if( !sarray ) {
    /* out of memory */
    return;
  }

  c = l->first;
  saptr = sarray;
  while( c ) {
    *saptr++ = c->value;
    c = c->next;
  }

  list_sort_cmp = cmp ? cmp : direct_compare;
  qsort(sarray, l->count, sizeof(void*), list_qsort_compare);
  list_sort_cmp = NULL;

  /* clear out the list */
  c = l->first;
  while( c ) {
    node *n = c->next;
    node_destroy(c);
    c = n;
  }
  l->first = NULL;

  /* insert each element in backwards */
  while( saptr-- != sarray ) {
    l->first = node_new(*saptr, l->first);
  }

  list_reset(l);

  free(sarray);
}

void
list_reverse( list *l )
{
  void **rarray, **raptr;
  node *c;

  if( !l || l->count <= 1 ) {
    if( l ) list_reset(l);
    return;
  }

  rarray = (void **) malloc(sizeof(void*) * l->count);
  if( !rarray ) {
    /* out of memory */
    return;
  }

  c = l->first;
  raptr = rarray + l->count;

  while( raptr != rarray ) {
    *--raptr = c->value;
    c = c->next;
  }

  /* raptr == rarray */
  for( c = l->first; c; c = c->next ) {
    c->value = *raptr++;
  }

  list_reset(l);

  free(rarray);
}
