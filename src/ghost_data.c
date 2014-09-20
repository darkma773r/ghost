/* ghost_data.c
 * Data structures for use in ghost.
 */

#include "ghost_data.h" 

typedef struct test {
	int x;
	int y;
	list_node_t node;
} test;

void
ght_push_node( list_t *list, list_node_t *node ){
	if ( list->tail == NULL ){
		/* the list is empty */
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		/* the list is not empty */
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next = node;
		list->tail = node;
	}
}

void
ght_remove_node( list_t *list, list_node_t *node ){
	if ( list->head == node ){
		list->head = node->next;		
	}
	if ( list->tail == node ){
		list->tail = node->prev;
	}

	if ( node->prev != NULL ){
		node->prev->next = node->next;
	}
	if ( node->next != NULL ){
		node->next->prev = node->prev;
	}
}

list_node_t *
ght_iter_init( list_t *list, list_iter_t *iter ){
	iter->next = list->head;	
	return ght_iter_next( iter );
}

list_node_t *
ght_iter_next( list_iter_t *iter ) {
	iter->current = iter->next;
	if ( iter->current != NULL ){
		iter->next = iter->current->next;
	}	
	return iter->current;
}

