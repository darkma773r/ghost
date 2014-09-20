/* ghost_data.c
 * Data structures for use in ghost.
 */

#include "ghost_data.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

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

/*
void
test_print( list_t *list ){
	test *iter;
	ght_for_each ( list, iter, test ){
		printf( ">> x = %d\n", iter->x );
	}
}

int
main(){
	
	test t;
	t.x = 1;
	t.y = 2;

	printf( "t = 0x%x, container from node = 0x%x\n", &t, 
		container_of( &t.node, test, node ));	
	printf( "t = 0x%x, container from y = 0x%x\n", &t, 
		container_of( &t.y, test, y));	
	printf( "t = 0x%x, container from null = 0x%x\n", &t,
		container_of( 0, test, y ));	

	list_t list = EMPTY_LIST;
	test *iter;

	printf( "printing empty list\n" );
	test_print( &list );

	test a;
	a.x = 1;
	
	test b;
	b.x = 2;

	ght_push( &list, &a );
	ght_push( &list, &b );

	printf( "printing non-empty list\n" );
	test_print( &list );

	printf( "removing items where x is odd\n" ); 
	ght_for_each( &list, iter, test ){
		if ( iter->x % 2 != 0 ){
			ght_remove( &list, iter );
		}
	}

	printf( "printing remaining items\n" );
	test_print( &list );

	printf( "adding a bunch more items\n" );

	test c,d,e,f,g;

	c.x = 4;
	d.x = 6;
	e.x = 8;
	f.x = 10;
	g.x = 12;

	ght_push( &list, &c );
	ght_push( &list, &d );
	ght_push( &list, &e );
	ght_push( &list, &f );
	ght_push( &list, &g );

	test_print( &list );

	printf( "removing items where x is even\n" );
	ght_for_each( &list, iter, test ){
		if ( iter->x %2 == 0 ){
			ght_remove( &list, iter );
		}	
	}

	printf( "printing list\n" );
	test_print( &list );
	
	return 0;
}

*/
