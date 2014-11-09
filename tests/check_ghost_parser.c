/* check_ghost_parser.c
 *
 * Unit tests for the ghost rule parsing logic.
 */

#include <check.h>
#include "../src/ghost_parser.c"

START_TEST( test_get_char )
{

}
END_TEST

Suite *
ghost_parser_suite()
{
	Suite *suite;
	TCase *tc_input;

	/* build the IO test case */
	tc_input = tcase_create( "Input" );

	/* add the individual tests */
	tcase_add_test( tc_input, test_get_char );

	suite_add_tcase( suite, tc_input );


	return suite;
}

int main(void)
{
	int number_failed;
	Suite *suite;
	SRunner *runner;

	suite = ghost_parser_suite();
	runner = srunner_create( suite );

	srunner_run_all( runner, CK_NORMAL );
	number_failed = srunner_ntests_failed( runner );
	srunner_free( runner );

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
