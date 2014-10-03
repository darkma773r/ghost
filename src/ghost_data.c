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
ght_list_push_node( list_t *list, list_node_t *node ){
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
ght_list_remove_node( list_t *list, list_node_t *node ){
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
ght_list_iter_init( list_t *list, list_iter_t *iter ){
	iter->next = list->head;	
	return ght_list_iter_next( iter );
}

list_node_t *
ght_list_iter_next( list_iter_t *iter ) {
	iter->current = iter->next;
	if ( iter->current != NULL ){
		iter->next = iter->current->next;
	}	
	return iter->current;
}

/* ########################## MAPS ###################### */

/* Helper function for creating a map_entry_t element. */
static map_entry_t * 
ght_map_create_entry( map_t *map, void *key, void *value ){
	map_entry_t *entry = (map_entry_t *) checked_malloc( sizeof( map_entry_t ));
	
	/* The key_copy function is expected to allocate dynamic memory for the
	key. This will be freed later in ght_map_free_entry. */
	entry->key = map->key_copy( key );

	/* The value is expected to be freed by the caller. */
	entry->value = value;

	return entry;
}

/* Helper function for releasing map_entry_t memory. The caller is responsible
for freeing any memory associated with the entry value. */
static void
ght_map_free_entry( map_entry_t *entry ){
	/* free the key */
	free( entry->key );

	/* free the rest of the struct */
	free( entry );
}

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
	/* release the memory for each entry  */
	map_iter_t iter;
	map_entry_t *entry;
	ght_map_for_each_entry( map, &iter, entry ){
		ght_map_remove_entry( map, entry ); 
	}
	
	/* free all of the bucket lists */
	free( map->buckets );

	/* free the map itself */
	free( map );
}

void *
ght_map_put( map_t *map, void *key, void *value ){
	int idx = ght_map_hash_idx( map, key ); 

	/* check if we have an entry for this already */
	map_entry_t *entry;
	void *old_value;
	ght_list_for_each( &(map->buckets[idx]), entry, map_entry_t ){
		if ( map->key_equals( key, entry->key )){
			/* we found an existing entry! replace
			the value and return the old value. */
			old_value = entry->value;
			entry->value = value;
			return old_value;		
		}
	}

	/* no existing entry :-( I guess we'll need to make a new one. */
	entry = ght_map_create_entry( map, key, value );

	ght_list_push( &(map->buckets[idx]), entry ); 

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
	ght_list_for_each( &(map->buckets[idx]), entry, map_entry_t ){
		if ( map->key_equals( key, entry->key )){
			return entry;
		}
	}	
	return NULL;
}

void *
ght_map_remove( map_t *map, void *key ){
	return ght_map_remove_entry( map, ght_map_get_entry( map, key ));
}

void *
ght_map_remove_entry( map_t *map, map_entry_t *entry ){
	if ( entry == NULL ){
		return NULL;
	}

	/* store the value to return later */
	void *value = entry->value;

	/* get the list containing this entry */
	int idx = ght_map_hash_idx( map, entry->key );		
	list_t *list = &(map->buckets[idx]);

	/* remove the entry from the list */
	ght_list_remove( list, entry );

	/* free the memory */
	ght_map_free_entry( entry );	

	return value;
}

map_entry_t *
ght_map_iter_init( map_t *map, map_iter_t *iter ){
	/* remember the map we're using */
	iter->map = map;

	/* position the index before the first bucket */
	iter->bucket_idx = -1;

	/* clear the current entry */
	iter->current = NULL;

	/* zero out the current list iterator */
	iter->list_iter.next = NULL;
	iter->list_iter.current = NULL;

	/* get the next available entry */
	return ght_map_iter_next( iter ); 
}

map_entry_t *
ght_map_iter_next( map_iter_t *iter ){
	/* try to stay on the same list and just advance one */
	list_node_t *list_node  = ght_list_iter_next( &(iter->list_iter) );

	if ( list_node == NULL ){
		/* nothing in the current list, try other buckets */
		iter->bucket_idx++;
		for (;iter->bucket_idx < iter->map->buckets_size; 
			iter->bucket_idx++){

			list_node = ght_list_iter_init( 
				&(iter->map->buckets[iter->bucket_idx]),
				&(iter->list_iter) );	
			if ( list_node != NULL ){
				break;
			}
		}
	}
	
	if ( list_node != NULL ){	
		iter->current = container_of( list_node, map_entry_t, node );
	} else {
		iter->current = NULL;
	}

	return iter->current;
}


/* Map<char[], void *> */

int
ght_strmap_key_hash( void *key ){
	/* Using the djb2 hash algorithm here. Thank you, Daniel J Bernstein! */ 
	char *char_key = (char *) key;
	
	int hash = 5381, c;
	while ( c = *char_key++ ){
		hash = (( hash << 5 ) + hash ) + c;
			/* this is equivalent to hash * 33 + c */
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
