/* ghost_data.h
 * Header file for ghost data structures.
 */

#ifndef _GHOST_DATA_H_
#define _GHOST_DATA_H_

#include <stddef.h>

/* Macro that converts a pointer to a structure member into a
pointer to the containing structure. If the member point is NULL,
then the macro evaluates to NULL. */
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
the list elements to be removed and their memory freed as the list is tranversed. This
macro is less efficient than ght_for_each and should only be used when items may need to be removed. */
#define ght_mod_for_each( LIST_PTR, ITER_PTR, ELEM_PTR_VAR, TYPE )\
	for ( ght_iter_init( (LIST_PTR), (ITER_PTR) ), ELEM_PTR_VAR = container_of( (ITER_PTR)->current, TYPE, node );\
		ELEM_PTR_VAR != NULL;\
		ght_iter_next( &iter ), ELEM_PTR_VAR = container_of( (ITER_PTR)->current, TYPE, node )) 

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

/* Initializes the iterator to the start of the given list. The iterator's current pointer will point to the head of the list. A pointer to the current node is returned. */
list_node_t *
ght_iter_init( list_t *list, list_iter_t *iter );

/* Advances the iterator to the next node and returns a pointer to the current node. */
list_node_t *
ght_iter_next( list_iter_t *iter );

#endif
