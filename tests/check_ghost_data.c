/* check_ghost_data.c
 * Unit test for the ghost_data module.
 */

#include <stdlib.h>
#include <string.h>
#include <check.h>
#include "../src/ghost_data.h"

/* ################## LISTS ################# */

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

/* ################### MAPS ###################### */

START_TEST( test_ght_map_put_and_get ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_SM );

	/* keys */
	char *apple = "apple",
		*cat = "cat";

	int a = 1, 
	    b = 2;

	/* act/assert */

	/* check that the items aren't found */
	ck_assert( ght_map_get( map, apple ) == NULL );
	ck_assert( ght_map_get( map, cat ) == NULL );

	/* add them */
	ght_map_put( map, apple, &a );
	ght_map_put( map, cat, &b );

	/* make sure they're found now */
	ck_assert( ght_map_get( map, apple ) == &a );
	ck_assert( ght_map_get( map, cat ) == &b );

	/* ensure that a fake key is still not found */
	ck_assert( ght_map_get( map, "fake" ) == NULL );

	/* clean up */
	ght_map_free( map );

}
END_TEST

START_TEST( test_ght_map_put_replaces_other_value ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_SM );

	char *apple = "apple";
	int a = 1, b = 2;

	/* act/assert */

	/* add the first item */
	ck_assert( ght_map_put( map, apple, &a ) == NULL );	
	map_entry_t *orig_entry = ght_map_get_entry( map, apple );

	/* add the second item under the same key. This should return
	the original, replaced value. */
	ck_assert( ght_map_put( map, apple, &b ) == &a );
	ck_assert( ght_map_get( map, apple ) == &b );

	/* make sure that the entry is the same as the previous one */ 
	map_entry_t *new_entry = ght_map_get_entry( map, apple );	
	ck_assert( orig_entry == new_entry );

	/* clean up */
	ght_map_free( map );
}
END_TEST

START_TEST( test_ght_map_put_copies_key ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_LG );

	char apple[] = "apple";
	int a = 1;

	/* act */
	ght_map_put( map, apple, &a );

	/* assert */
	map_entry_t *entry = ght_map_get_entry( map, apple );
	ck_assert( entry->key != apple );

	/* clean up */
	ght_map_free( map );
}
END_TEST

START_TEST( test_ght_map_get_entry ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_MD );

	char apple[] = "apple";
	char a = 1;

	ght_map_put( map, apple, &a );

	/* act */
	map_entry_t *good = ght_map_get_entry( map, apple );
	map_entry_t *bad = ght_map_get_entry( map, "fake" );

	/* assert */
	ck_assert( good != NULL );
	ck_assert( strcmp( good->key, apple ) == 0 );
	ck_assert( good->value == &a );

	ck_assert( bad == NULL );

	/* clean up */
	ght_map_free( map );
}
END_TEST

START_TEST( test_ght_map_remove ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_SM );

	char apple[] = "apple";
	char a = 1;

	ght_map_put( map, apple, &a );

	/* act */
	void *result = ght_map_remove( map, apple );

	/* assert */
	ck_assert( result == &a );
	ck_assert( ght_map_get( map, apple ) == NULL );
}
END_TEST

START_TEST( test_ght_map_remove_not_found ){
	/* arrange */
	map_t *map = ght_strmap_create( MAP_SIZE_SM );

	/* act */
	void *result = ght_map_remove( map, "not found" );

	/* assert */
	ck_assert( result == NULL );
}
END_TEST


/* ##################### TEST SETUP ################### */

Suite *
ghost_data_suite(){
	Suite *suite;
	TCase *tc_list, *tc_map;

	suite = suite_create( "ghost_data" );
	
	/* buid the list test case */
	tc_list = tcase_create( "List" );

	/* add individual tests */
	tcase_add_test( tc_list, test_create_list );
	tcase_add_test( tc_list, test_ght_push );
	tcase_add_test( tc_list, test_ght_remove );
	tcase_add_test( tc_list, test_ght_remove_all );
	tcase_add_test( tc_list, test_ght_mod_for_each );
	tcase_add_test( tc_list, test_remove_ght_mod_for_each );

	suite_add_tcase( suite, tc_list );

	/* build the map test_case */
	tc_map = tcase_create( "Map" );

	/* add individual tests */
	tcase_add_test( tc_map, test_ght_map_put_and_get );
	tcase_add_test( tc_map, test_ght_map_put_replaces_other_value );
	tcase_add_test( tc_map, test_ght_map_put_copies_key );
	tcase_add_test( tc_map, test_ght_map_get_entry );
	tcase_add_test( tc_map, test_ght_map_remove );
	tcase_add_test( tc_map, test_ght_map_remove_not_found );
	
	suite_add_tcase( suite, tc_map );


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
