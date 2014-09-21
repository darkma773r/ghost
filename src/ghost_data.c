/* ghost_data.c
 * Data structures for use in ghost.
 */

#include "ghost_data.h" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ################### GENERAL ####################### */

void *
checked_malloc( size_t size ){
	void *result = malloc( size );
	if ( result == NULL ){
		fprintf( stderr,
			"Fatal Error: Failed to allocate dynamic memory!\n" );
		exit( 1 );
	}	
	
	return result;
}

/* ####################### LISTS ###################### */

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

/* ########################## MAPS ###################### */

map_t *
ght_map_create( int buckets_size, hash_fn key_hash, equals_fn key_equals, copy_fn key_copy ){
	/* allocate and zero the map */
	map_t *map = (map_t *) checked_malloc( sizeof( map_t ));
	memset( map, 0, sizeof( map_t ));

	/* allocate and zero the bucket array */ 
	map->buckets = (list_t *) malloc( buckets_size * sizeof( list_t ));
	memset( map->buckets, 0, buckets_size * sizeof( list_t ));

	/* copy other members */
	map->buckets_size = buckets_size;

	map->key_hash = key_hash;
	map->key_equals = key_equals;
	map->key_copy = key_copy; 

	return map;
}

void
ght_map_free( map_t *map ){
	/* TODO: release the memory for keys here */
	free( map->buckets );
	free( map );
}

void *
ght_map_put( map_t *map, void *key, void *value ){
	int idx = ght_map_hash_idx( map, key ); 

	/* check if we have an entry for this already */
	map_entry_t *entry;
	void *old_value;
	ght_for_each( &(map->buckets[idx]), entry, map_entry_t ){
		if ( map->key_equals( key, entry->key )){
			/* we found an existing entry! replace
			the value and return the old value. */
			old_value = entry->value;
			entry->value = value;
			return old_value;		
		}
	}

	/* no existing entry :-( I guess we'll need to make a new one. */
	entry = (map_entry_t *) checked_malloc( sizeof( map_entry_t ));
	
	/* store a copy of the key and not the original */
	entry->key = map->key_copy( key );
	entry->value = value;

	ght_push( &(map->buckets[idx]), entry ); 

	return NULL; /* return NULL to signal that we didn't replace an
		existing entry */
}

void *
ght_map_get( map_t *map, void *key ){
	map_entry_t *entry = ght_map_get_entry( map, key );	
	return entry != NULL ? entry->value : NULL;
}

map_entry_t *
ght_map_get_entry( map_t *map, void *key ){
	int idx = ght_map_hash_idx( map, key );

	map_entry_t *entry;
	ght_for_each( &(map->buckets[idx]), entry, map_entry_t ){
		if ( map->key_equals( key, entry->key )){
			return entry;
		}
	}	
	return NULL;
}

/* Map<char[], void *> */

int
ght_strmap_key_hash( void *key ){
	char *char_key = (char *) key;
	int hash = 17;
	while ( char_key ){
		hash = ( hash << 1 ) ^ *(char_key++);
	}
	return hash;
}

int
ght_strmap_key_equals( void *key_a, void *key_b ){
	return strcmp( (char *) key_a, (char *) key_b ) == 0;
}

void *
ght_strmap_key_copy( void *key ){
	char *char_key = (char *) key;
	int len = strlen( char_key );

	char * copy = (char *) checked_malloc( len + 1 );
	strncpy( copy, char_key, len );
	copy[len] = '\0'; 

	return copy;
}

map_t *
ght_strmap_create( int buckets_size ){
	return ght_map_create( buckets_size, 
		ght_strmap_key_hash,
		ght_strmap_key_equals,
		ght_strmap_key_copy);
}

/* Map<xcb_window_t, void *> */

int
ght_winmap_key_hash( void *key ){
	return (int)*((xcb_window_t *) key);
}

int
ght_winmap_key_equals( void *key_a, void *key_b ){
	return *((xcb_window_t *) key_a) == *((xcb_window_t *) key_b);
}

void *
ght_winmap_key_copy( void *key ){
	xcb_window_t *win_key = (xcb_window_t *) key;

	xcb_window_t *copy = (xcb_window_t *) checked_malloc( sizeof( xcb_window_t ));
	
	*copy = *win_key;

	return copy;
}

map_t *
ght_winmap_create( int buckets_size ){
	return ght_map_create( buckets_size,
		ght_winmap_key_hash,
		ght_winmap_key_equals,
		ght_winmap_key_copy );
}
