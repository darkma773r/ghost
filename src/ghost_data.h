/* ghost_data.h
 * Header file for ghost data structures.
 */

#ifndef _GHOST_DATA_H_
#define _GHOST_DATA_H_

#include <stddef.h>
#include <xcb/xcb.h>

/* ################## GENERAL ######################## */

/* A version of malloc that fails the program if it can't
allocate memory. */
void *
checked_malloc( size_t size );

/* ################### Lists ########################## */

/* Macro that converts a pointer to a structure member into a
pointer to the containing structure. If the member point is NULL,
then the macro evaluates to NULL. The ELEM_PTR expression is evaluated
twice in this process so it should not contain any side effects. */
#define container_of( ELEM_PTR, TYPE, MEMBER ) \
	((ELEM_PTR) != NULL ?\
	(TYPE *)(((char *)(ELEM_PTR)) - offsetof(TYPE, MEMBER)) : NULL )

/* Macro for adding elements to the end of a list. The
structure pointed to by ELEM_PTR is expected to have a list_node_t
member named "node". */
#define ght_push( LIST_PTR, ELEM_PTR ) \
	( ght_push_node( LIST_PTR, &((ELEM_PTR)->node) ))

/* Macro for removing elements from a list. The 
structure pointed to by ELEM_PTR is expected to have a list_node_t
member named "node". */
#define ght_remove( LIST_PTR, ELEM_PTR ) \
	( ght_remove_node( LIST_PTR, &((ELEM_PTR)->node) ))

/* Macro for iterating over list elements. LIST_PTR must a a pointer to 
a list_t structure, ELEM_PTR_VAR must be an assignable variable in the
current scope of type TYPE *, and TYPE should be the type of the 
elements in the list. */
#define ght_for_each( LIST_PTR, ELEM_PTR_VAR, TYPE ) \
	for ( ELEM_PTR_VAR = container_of( (LIST_PTR)->head, TYPE, node );\
		ELEM_PTR_VAR != NULL;\
		ELEM_PTR_VAR = container_of( (ELEM_PTR_VAR)->node.next, TYPE, node))   

/* Macro for iterating over list elements in such a way as to allow
the list elements to be removed and their memory freed as the list 
is tranversed. This macro is less efficient than ght_for_each and 
should only be used when items may need to be removed. */
#define ght_mod_for_each( LIST_PTR, ITER_PTR, ELEM_PTR_VAR, TYPE )\
	for ( ght_iter_init( (LIST_PTR), (ITER_PTR) ), \
		ELEM_PTR_VAR = container_of( (ITER_PTR)->current, TYPE, node );\
		ELEM_PTR_VAR != NULL;\
		ght_iter_next( &iter ), \
		ELEM_PTR_VAR = container_of( (ITER_PTR)->current, TYPE, node )) 

/* List node structure. Structures that will be stored in
lists should include this as a member named "node". */
typedef struct list_node_t {
	struct list_node_t * next;
	struct list_node_t * prev;
} list_node_t;

/* List structure. */
typedef struct list_t {
	list_node_t * head;
	list_node_t * tail;
} list_t;

/* List iterator structure */
typedef struct list_iter_t {
	list_node_t * next;
	list_node_t * current;
} list_iter_t;

/* Adds a node to the end of the list */
void
ght_push_node( list_t *list, list_node_t *node );

/* Removes the given node from the list. The node must be known to
belong to the given list or the structure will become corrupted. */
void
ght_remove_node( list_t *list, list_node_t *node );

/* Initializes the iterator to the start of the given list. The iterator's 
current pointer will point to the head of the list. A pointer to the 
current node is returned. */
list_node_t *
ght_iter_init( list_t *list, list_iter_t *iter );

/* Advances the iterator to the next node and returns a pointer to 
the current node. */
list_node_t *
ght_iter_next( list_iter_t *iter );

/* ####################### MAPS ########################## */

/* Pre-defined, prime map bucket array sizes */
#define MAP_SIZE_SM 17
#define MAP_SIZE_MD 83 
#define MAP_SIZE_LG 257

/* Macro for calculating the hash bucket index from the key */
#define ght_map_hash_idx( MAP_PTR, KEY_PTR ) \
	( (MAP_PTR)->key_hash( (KEY_PTR) ) % (MAP_PTR)->buckets_size )

/* Macro for iterating over the map_entry_t elements in a map. */
#define ght_map_for_each_entry( MAP_PTR, ITER_PTR, ENTRY_PTR_VAR ) \
	for ( ENTRY_PTR_VAR = ght_map_iter_init( (MAP_PTR), (ITER_PTR) ); \
		ENTRY_PTR_VAR != NULL; \
		ENTRY_PTR_VAR = ght_map_iter_next( (ITER_PTR ) ))
		
/* macro for iterating over the values in a map. Note that the values may be null. */
#define ght_map_for_each( MAP_PTR, ITER_PTR, VALUE_PTR_VAR, VALUE_PTR_TYPE ) \
	for ( ght_map_iter_init( (MAP_PTR), (ITER_PTR) ), \
		VALUE_PTR_VAR = (ITER_PTR)->current != NULL ? (VALUE_PTR_TYPE)((ITER_PTR)->current->value) : NULL; \
		(ITER_PTR)->current != NULL; \
		ght_map_iter_next( (ITER_PTR ) ), \
		VALUE_PTR_VAR = (ITER_PTR)->current != NULL ? (VALUE_PTR_TYPE)((ITER_PTR)->current->value) : NULL )
	

/* Map helper function for computing a key hash value */
typedef int (*hash_fn)( void *key );

/* Map helper function for comparing two keys. The function should
return true if the two keys are equal. */
typedef int (*equals_fn)( void *key, void *entry_key );

/* Map helper function for creating a copy of a key in dynamic memory. The
memory for the key must be able to be freed with a single call to free().
The releasing of this memory is handled internally by the map functions. */
typedef void * (*copy_fn)( void *key );

/* Generic struct for storing a key-value pair map entry. The
entry is intended to live in a linked list so it includes a
list_node_t member */
typedef struct map_entry_t {
	void *key;
	void *value;
	list_node_t node;
} map_entry_t;


/* Struct containing top-level map elements */
typedef struct map_t {
	int buckets_size; /* the size of the hash bucket array */ 
	list_t *buckets; /* the hash bucket array; each bucket
		contains a list of map_entry_t structs */	
	hash_fn key_hash; /* function for hashing keys */
	equals_fn key_equals; /* function for testing if two keys are equal. */
	copy_fn key_copy; /* function for copying keys */
} map_t;

/* Struct for iterating over a map */
typedef struct map_iter_t {
	map_t *map; /* the map being iterated over */
	int bucket_idx; /* the index of the bucket we're on */	
	map_entry_t *current; /* the current entry */
	list_iter_t list_iter; /* the current list iterator */
} map_iter_t;

/* Creates a new map with the given size and helper functions. 
The buckets_size should be a prime number. */
map_t *
ght_map_create( int buckets_size, hash_fn key_hash, equals_fn key_equals, copy_fn key_copy );

/* Releases all memory associated with the map, including the
map itself. The memory for stored keys is released but not
the memory for the data elements. That must be done by the caller
prior to calling this method. */
void
ght_map_free( map_t *map );

/* Places a new entry into the map with the given key and value.
The key is copied using the map copy_key function and its memory
is managed by the map itself. The memory for the value must be
managed by the caller. If an entry already exists for the key,
the value is updated and the old value is returned. If no entry
exists, then NULL is returned. */
void *
ght_map_put( map_t *map, void *key, void *value );

/* Returns the value stored with the given key or NULL if
it does not exist. */
void *
ght_map_get( map_t *map, void *key );

/* Returns the entire map entry stored with the given key or
NULL if it does not exist. */
map_entry_t *
ght_map_get_entry( map_t *map, void *key );

/* Removes the map entry with the given key, returning its
value. Returns NULL if the entry cannot be found. The memory
for the map_entry_t and key is freed. The memory for the value
must be freed by the caller. */
void *
ght_map_remove( map_t *map, void *key );

/* Initializes the given map iterator and returns a pointer to 
the first available map_entry_t. */
map_entry_t *
ght_map_iter_init( map_t *map, map_iter_t *iter );

/* Moves the map iterator to the next available map_entry_t and
returns a pointer to it. */ 
map_entry_t *
ght_map_iter_next( map_iter_t *iter );

/* C string hashing function. */
int
ght_strmap_key_hash( void *key );

/* C string comparision function. */
int
ght_strmap_key_equals( void *key_a, void *key_b );

/* C string copy function. */
void *
ght_strmap_key_copy( void * key );

/* Creates a map where the keys are interpreted as C strings. */
map_t *
ght_strmap_create( int buckets_size );

/* xcb_window_t hashing function. This just returns the key as an int. */
int
ght_winmap_key_hash( void *key );

/* xcb_window_t equals function. */
int
ght_winmap_key_equals( void *key_a, void *key_b );

/* xcb_window_t copy function. */
void *
ght_winmap_key_copy( void *key );

/* Creates a map where the keys are interpreted as xcb_window_t. */
map_t *
ght_winmap_create( int buckets_size );

#endif
