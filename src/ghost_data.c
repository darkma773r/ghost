/* ghost_data.c
 * Data structures for use in ghost.
 */

#include "ghost_data.h" 
#include <stdlib.h>
#include <string.h>

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

map_t *
ght_map_create( hash_fn hash, comp_fn comp ){
	map_t *map = (map_t *) malloc( sizeof( map_t ));
	if ( map == NULL ){
		return NULL; /* failed */
	}

	memset( map, 0, sizeof( map_t ));

	map->hash = hash;
	map->comp = comp;
	map->arr = (list_t *) malloc( MAP_ARR_SIZE * sizeof( list_t ));

	memset( map->arr, 0, MAP_ARR_SIZE * sizeof( list_t ));

	return map;
}

void
ght_map_free( map_t *map ){
	free( map->arr );
	free( map );
}

void *
ght_map_put( map_t *map, void *key, void *value ){
	int idx = map->hash( key ) % MAP_ARR_SIZE;	

	map_entry_t *entry = (map_entry_t *) malloc( sizeof( map_entry_t ));
	if ( entry == NULL ){
		return NULL; /* failed */
	}
	
	entry->key = key;
	entry->value = value;

	ght_push( &(map->arr[idx]), entry ); 

	return value;
}

void *
ght_map_get( map_t *map, void *key ){
	map_entry_t *entry = ght_map_get_entry( map, key );	
	return entry != NULL ? entry->value : NULL;
}

map_entry_t *
ght_map_get_entry( map_t *map, void *key ){
	int idx = map->hash( key ) % MAP_ARR_SIZE;	

	map_entry_t *entry;
	ght_for_each( &(map->arr[idx]), entry, map_entry_t ){
		if ( map->comp( key, entry->key )){
			return entry;
		}
	}	
	return NULL;
}

