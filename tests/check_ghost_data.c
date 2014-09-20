/* check_ghost_data.c
 * Unit test for the ghost_data module.
 */

#include <stdlib.h>
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

START_TEST( test_ght_remove_local ) {
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
	tcase_add_test( tc_core, test_ght_remove_local );
	tcase_add_test( tc_core, test_ght_remove_all );

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
