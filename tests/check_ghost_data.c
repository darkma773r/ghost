/* check_ghost_data.c
 * Unit test for the ghost_data module.
 */

#include <stdlib.h>
#include <string.h>
#include <check.h>
#include "../src/ghost_data.h"

static const list_t EMPTY_LIST = { NULL };

typedef struct test_t {
	int a;
	double b;
	list_node_t node;
} test_t;

START_TEST( test_create_list ) {
	/* act */
	list_t list = EMPTY_LIST;
	
	/* assert */
	ck_assert( list.head == NULL );
	ck_assert( list.tail == NULL );
}
END_TEST


START_TEST( test_ght_push ) {
	/* arrange */
	list_t list = EMPTY_LIST;

	test_t first = {
		1, 2.1, { (list_node_t *) 1, (list_node_t *) 48384 }
	};

	test_t second = {
		2, 3.1, { (list_node_t *) 100, (list_node_t *) 200 }	
	};

	test_t *third = (test_t *) malloc( sizeof( test_t ));
	*third = (test_t) {
		3, 4.1, { (list_node_t *) 1000, (list_node_t *) 29382 }
	};

	/* act */
	ght_push( &list, &first );
	ght_push( &list, &second );
	ght_push( &list, third );
	
	/* assert */
	ck_assert( &first.node == list.head );
	ck_assert( &third->node == list.tail );

	int i = 1;
	double d = 2.1;
	test_t *cur;

	/* run through the list once before the test to make sure
	 we can iterate through it multiple times */
	ght_for_each( &list, cur, test_t ){}

	/* do the test */
	ght_for_each( &list, cur, test_t ) {
		ck_assert( cur->a == i );
		ck_assert( cur->b == d ); 
		i++;
		d += 1;
	} 	

	ck_assert( i == 4 );

	free( third );
} 
END_TEST

START_TEST( test_ght_remove ) {
	/* arrange */
	list_t list = EMPTY_LIST;

	test_t first = {
		1, 2.1, { (list_node_t *) 1, (list_node_t *) 48384 }
	};

	test_t second = {
		2, 3.1, { (list_node_t *) 100, (list_node_t *) 200 }	
	};

	test_t *third = (test_t *) malloc( sizeof( test_t ));
	*third = (test_t) {
		3, 4.1, { (list_node_t *) 1000, (list_node_t *) 29382 }
	};

	ght_push( &list, &first );
	ght_push( &list, &second );
	ght_push( &list, third );

	/* act */
	ght_remove( &list, &first );
	ght_remove( &list, third );
	
	/* assert */
	ck_assert( &second.node == list.head );
	ck_assert( &second.node == list.tail );

	int i = 2;
	double d = 3.1;
	test_t *cur;

	ght_for_each( &list, cur, test_t ) {
		ck_assert( cur->a == i );
		ck_assert( cur->b == d ); 
		i++;
		d += 1;
	} 	

	ck_assert( i == 3 );

	free( third );
} 
END_TEST

START_TEST( test_ght_remove_all ) {
	/* arrange */
	list_t list = EMPTY_LIST;

	test_t first = {
		1, 2.1, { (list_node_t *) 1, (list_node_t *) 48384 }
	};

	test_t second = {
		2, 3.1, { (list_node_t *) 100, (list_node_t *) 200 }	
	};

	test_t *third = (test_t *) malloc( sizeof( test_t ));
	*third = (test_t) {
		3, 4.1, { (list_node_t *) 1000, (list_node_t *) 29382 }
	};

	ght_push( &list, &first );
	ght_push( &list, &second );
	ght_push( &list, third );

	/* act */
	ght_remove( &list, &second );
	ght_remove( &list, &first );
	ght_remove( &list, third );
	
	/* assert */
	ck_assert( NULL == list.head );
	ck_assert( NULL == list.tail );

	int i = 0;
	test_t *cur;

	ght_for_each( &list, cur, test_t ) {
		i++;
	} 	

	ck_assert( i == 0 );

	free( third );
} 
END_TEST

START_TEST( test_ght_mod_for_each ) {
	/* arrange */
	list_t list = EMPTY_LIST;

	test_t *first = (test_t *) malloc( sizeof( test_t ));
	*first = (test_t) {
		1, 2.1, { (list_node_t *) 1, (list_node_t *) 48384 }
	};

	test_t *second = (test_t *) malloc( sizeof( test_t ));
	*second = (test_t) {
		2, 3.1, { (list_node_t *) 100, (list_node_t *) 200 }	
	};

	test_t *third = (test_t *) malloc( sizeof( test_t ));
	*third = (test_t) {
		3, 4.1, { (list_node_t *) 1000, (list_node_t *) 29382 }
	};

	ght_push( &list, first );
	ght_push( &list, second );
	ght_push( &list, third );

	/* act/assert */
	test_t *cur;
	list_iter_t iter;
	int i = 1;
	double d = 2.1;

	/* run through the list once before the test to make sure we can iterate through it multiple times */
	ght_mod_for_each( &list, &iter, cur, test_t ){}

	/* do the test */
	ght_mod_for_each( &list, &iter, cur, test_t ){
		ck_assert( cur->a == i );
		ck_assert( cur->b == d ); 

		i++;
		d += 1;
	} 	

	ck_assert( i == 4 );
	
	free( first );
	free( second );
	free( third );
}
END_TEST


START_TEST( test_remove_ght_mod_for_each ) {
	/* arrange */
	list_t list = EMPTY_LIST;

	test_t *first = (test_t *) malloc( sizeof( test_t ));
	*first = (test_t) {
		1, 2.1, { (list_node_t *) 1, (list_node_t *) 48384 }
	};

	test_t *second = (test_t *) malloc( sizeof( test_t ));
	*second = (test_t) {
		2, 3.1, { (list_node_t *) 100, (list_node_t *) 200 }	
	};

	test_t *third = (test_t *) malloc( sizeof( test_t ));
	*third = (test_t) {
		3, 4.1, { (list_node_t *) 1000, (list_node_t *) 29382 }
	};

	ght_push( &list, first );
	ght_push( &list, second );
	ght_push( &list, third );

	/* act/assert */
	test_t *cur;
	list_iter_t iter;
	int i = 1;
	double d = 2.1;

	/* run through the list once before the test to make sure we can iterate through it multiple times */
	ght_mod_for_each( &list, &iter, cur, test_t ){}

	/* run the test */
	ght_mod_for_each( &list, &iter, cur, test_t ){
		ck_assert( cur->a == i );
		ck_assert( cur->b == d ); 

		/* remove and free the node */
		ght_remove( &list, cur );
		cur->node.next = NULL;
		cur->node.prev = NULL;
		free( cur );

		i++;
		d += 1;
	} 	

	ck_assert( i == 4 );

	int count = 0;
	ght_mod_for_each( &list, &iter, cur, test_t) {
		count++;
	}

	ck_assert( count == 0 );
}
END_TEST

int
test_hash( void *key ){
	char *char_key = (char *) key;
	int hc = 17;
	while ( *char_key){
		hc += *(char_key++);
	}
	return hc;
}

int
test_comp( void *key, void *test_key ){
	return strcmp( (char *) key, (char *) test_key ) == 0;
}

START_TEST( test_ght_map_put_and_get ){
	/* arrange */
	map_t *map = ght_map_create( test_hash, test_comp );

	/* keys */
	char *apple = "apple",
		*cat = "cat",
		*dog = "dog",
		*zebra = "zebra";

	int a = 1, 
	    b = 2,
	    c = 3,
	    d = 4;

	/* act/assert */
	ck_assert( ght_map_get( map, apple ) == NULL );
	ck_assert( ght_map_get( map, cat ) == NULL );
	ck_assert( ght_map_get( map, dog ) == NULL );
	ck_assert( ght_map_get( map, zebra ) == NULL );

	ght_map_put( map, apple, &a );
	ght_map_put( map, cat, &b );
	ght_map_put( map, dog, &c );
	ght_map_put( map, zebra, &d );

	ck_assert( ght_map_get( map, apple ) == &a );
	ck_assert( ght_map_get( map, cat ) == &b );
	ck_assert( ght_map_get( map, dog ) == &c );
	ck_assert( ght_map_get( map, zebra ) == &d );
}
END_TEST


Suite *
ghost_data_suite(){
	Suite *suite;
	TCase *tc_core;

	suite = suite_create( "ghost_data" );
	
	/* buid the core test case */
	tc_core = tcase_create( "Core" );

	/* add individual tests */
	tcase_add_test( tc_core, test_create_list );
	tcase_add_test( tc_core, test_ght_push );
	tcase_add_test( tc_core, test_ght_remove );
	tcase_add_test( tc_core, test_ght_remove_all );
	tcase_add_test( tc_core, test_ght_mod_for_each );
	tcase_add_test( tc_core, test_remove_ght_mod_for_each );
	tcase_add_test( tc_core, test_ght_map_put_and_get );

	suite_add_tcase( suite, tc_core );

	return suite;
}

int main(void){
	int number_failed;
	Suite *suite;
	SRunner *runner;	
	
	suite = ghost_data_suite();
	runner = srunner_create( suite );

	srunner_run_all( runner, CK_NORMAL );
	number_failed = srunner_ntests_failed( runner );
	srunner_free( runner );

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
