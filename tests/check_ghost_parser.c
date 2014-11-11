/* check_ghost_parser.c
 *
 * Unit tests for the ghost rule parsing logic.
 */

#include <check.h>
#include "../src/ghost_parser.c"

/*
 * Returns a FILE pointer that uses the give C string as input.
 */
FILE *
str_file( char *buffer ){
    return fmemopen( (void *)buffer, strlen(buffer), "r" );
}

/* ############################# INPUT ############################ */

START_TEST( test_default_parser )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;

    /* act */
    ck_assert( parser.input == NULL );
    ck_assert( parser.lastchar == 0 );
    ck_assert( parser.uselastchar == false );
    ck_assert( parser.linenum == 1 );
    ck_assert( parser.charnum == 0 );
    ck_assert( parser.done == false );
    ck_assert( parser.error == false );
}
END_TEST

START_TEST( test_get_char )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "abc" );

    /* act/assert */
    ck_assert_int_eq( 'a', get_char( &parser ));
    ck_assert_int_eq( 'b', get_char( &parser ));
    ck_assert_int_eq( 'c', get_char( &parser ));
    ck_assert_int_eq( EOF, get_char( &parser ));
    ck_assert_int_eq( EOF, get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_get_char_tracks_position )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "a\n\nbcd" );

    /* act/assert */
    ck_assert_int_eq( 1, parser.linenum );
    ck_assert_int_eq( 0, parser.charnum );

    ck_assert_int_eq( 'a', get_char( &parser ));
    ck_assert_int_eq( 1, parser.linenum );
    ck_assert_int_eq( 1, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( '\n', get_char( &parser ));
    ck_assert_int_eq( 1, parser.linenum );
    ck_assert_int_eq( 2, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( '\n', get_char( &parser ));
    ck_assert_int_eq( 2, parser.linenum );
    ck_assert_int_eq( 1, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( 'b', get_char( &parser ));
    ck_assert_int_eq( 3, parser.linenum );
    ck_assert_int_eq( 1, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( 'c', get_char( &parser ));
    ck_assert_int_eq( 3, parser.linenum );
    ck_assert_int_eq( 2, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( 'd', get_char( &parser ));
    ck_assert_int_eq( 3, parser.linenum );
    ck_assert_int_eq( 3, parser.charnum );
    ck_assert_int_eq( false, parser.done );

    ck_assert_int_eq( EOF, get_char( &parser ));
    ck_assert_int_eq( 3, parser.linenum );
    ck_assert_int_eq( 4, parser.charnum );
    ck_assert_int_eq( true, parser.done );

    ck_assert_int_eq( EOF, get_char( &parser ));
    ck_assert_int_eq( 3, parser.linenum );
    ck_assert_int_eq( 4, parser.charnum );
    ck_assert_int_eq( true, parser.done );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_peek_char )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "abc\n" );

    /* act/assert */
    ck_assert_int_eq( 'a', peek_char( &parser ));
    ck_assert_int_eq( 'a', peek_char( &parser ));
    ck_assert_int_eq( 'a', get_char( &parser ));

    ck_assert_int_eq( 'b', peek_char( &parser ));
    ck_assert_int_eq( 'b', peek_char( &parser ));
    ck_assert_int_eq( 'b', get_char( &parser ));

    ck_assert_int_eq( 'c', peek_char( &parser ));
    ck_assert_int_eq( 'c', peek_char( &parser ));
    ck_assert_int_eq( 'c', get_char( &parser ));

    ck_assert_int_eq( '\n', peek_char( &parser ));
    ck_assert_int_eq( '\n', peek_char( &parser ));
    ck_assert_int_eq( '\n', get_char( &parser ));

    ck_assert_int_eq( EOF, peek_char( &parser ));
    ck_assert_int_eq( EOF, peek_char( &parser ));
    ck_assert_int_eq( EOF, get_char( &parser ));

    ck_assert_int_eq( EOF, peek_char( &parser ));
    ck_assert_int_eq( EOF, peek_char( &parser ));
    ck_assert_int_eq( EOF, get_char( &parser ));

    ck_assert_int_eq( true, parser.done );
    ck_assert_int_eq( 2, parser.linenum );
    ck_assert_int_eq( 1, parser.charnum );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_is_valid_str_char )
{
    /* act/assert */
    ck_assert( is_valid_str_char( '_' ));
    ck_assert( is_valid_str_char( 'a' ));
    ck_assert( is_valid_str_char( 'Z' ));
    ck_assert( is_valid_str_char( '1' ));

    ck_assert( !is_valid_str_char( '"' ));
    ck_assert( !is_valid_str_char( '\'' ));
    ck_assert( !is_valid_str_char( ' ' ));
    ck_assert( !is_valid_str_char( '\n' ));
    ck_assert( !is_valid_str_char( EOF ));
}
END_TEST

START_TEST( test_is_valid_str_start_char )
{
    /* act/assert */
    ck_assert( is_valid_str_start_char( '_' ));
    ck_assert( is_valid_str_start_char( 'a' ));
    ck_assert( is_valid_str_start_char( 'Z' ));
    ck_assert( is_valid_str_start_char( '1' ));
    ck_assert( is_valid_str_start_char( '"' ));
    ck_assert( is_valid_str_start_char( '\'' ));

    ck_assert( !is_valid_str_start_char( ' ' ));
    ck_assert( !is_valid_str_start_char( '\n' ));
    ck_assert( !is_valid_str_start_char( EOF ));
}
END_TEST

START_TEST( test_read_str_token )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "abc\nd" );

    /* act */
    int len = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 3, len );
    ck_assert_str_eq( "abc", parser.buffer );

    ck_assert_int_eq( '\n', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_ignores_initial_spaces )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n\t \r\n   abc\nd" );

    /* act */
    int len = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 3, len );
    ck_assert_str_eq( "abc", parser.buffer );

    ck_assert_int_eq( '\n', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_double_quotes )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "\"ab'' c\n\"" );

    /* act */
    int len = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 7, len );
    ck_assert_str_eq( "ab'' c\n", parser.buffer );

    ck_assert_int_eq( EOF, get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST


START_TEST( test_read_str_token_single_quotes )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "'ab\"\" c\n\'" );

    /* act */
    int len = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 7, len );
    ck_assert_str_eq( "ab\"\" c\n", parser.buffer );

    ck_assert_int_eq( EOF, get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_empty_token )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n" );

    /* act */
    int len = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 0, len );
    ck_assert_str_eq( "", parser.buffer );

    ck_assert_int_eq( EOF, get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_multiple_calls )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "abc(de " );

    /* act/assert */
    ck_assert_int_eq( 3, read_str_token( &parser ));
    ck_assert_str_eq( "abc", parser.buffer );

    ck_assert_int_eq( '(', get_char( &parser ));

    ck_assert_int_eq( 2, read_str_token( &parser ));
    ck_assert_str_eq( "de", parser.buffer );

    ck_assert_int_eq( ' ', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_exceeds_max_length )
{
    /* arrange */
    char str[MAX_STR_LEN + 100];
    int i;
    for ( i=0; i < sizeof( str ) - 1; i++ ){
        str[i] = 'A';
    }
    str[ sizeof(str) - 1] = '\0';

    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( str );

    /* act/assert */
    ck_assert_int_eq( MAX_STR_LEN, read_str_token( &parser ));
    ck_assert_int_eq( MAX_STR_LEN, strlen(parser.buffer ));
    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_str_token_unclosed_quote )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "'abc" );

    /* act */
    int result = read_str_token( &parser );

    /* assert */
    ck_assert_int_eq( 3, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_str_eq( "abc", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_has_str_token )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \na" );

    /* act */
    int result = has_str_token( &parser );

    /* assert */
    ck_assert_int_eq( true, result );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_has_str_token_failed )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n" );

    /* act */
    int result = has_str_token( &parser );

    /* assert */
    ck_assert_int_eq( false, result );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "123.45" );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 12345,  (int)( result * 100 ));
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "123.45", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_ignores_initial_spaces )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \r\n  \t\t123.45" );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 12345,  (int)( result * 100 ));
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "123.45", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_stops_at_first_non_digit )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "123.45" );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 12345,  (int)( result * 100 ));
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "123.45", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_multiple_decimal_points )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "123.45.." );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 12345,  (int)( result * 100 ));
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "123.45", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_number_exceeds_max_str_len )
{
    /* arrange */
    char str[MAX_STR_LEN + 100];
    int i;
    for ( i=0; i < sizeof( str ) - 1; i++ ){
        str[i] = '9';
    }
    str[ sizeof(str) - 1] = '\0';

    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( str );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 0,  (int)( result ));
    ck_assert_int_eq( true, parser.error );
    ck_assert_int_eq( MAX_STR_LEN , strlen( parser.buffer ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_no_decimal )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "123" );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 12300,  (int)( result * 100 ));
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "123", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_double_no_digits_fails )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "x" );

    /* act */
    double result = read_double( &parser );

    /* assert */
    ck_assert_int_eq( 0,  (int)( result * 100 ));
    ck_assert_int_eq( true, parser.error );
    ck_assert_str_eq( "", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_consume_space )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "  \n \t\r\na" );

    /* act */
    consume_space( &parser );

    /* assert */
    ck_assert_int_eq( 'a', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_consume_space_no_space_found )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "a" );

    /* act */
    consume_space( &parser );

    /* assert */
    ck_assert_int_eq( 'a', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_char )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "ab" );

    /* act */
    bool result = match_char( &parser, 'a' );

    /* assert */
    ck_assert_int_eq( true, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_int_eq( 'b', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_char_spaces )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n  \t ab" );

    /* act */
    bool result = match_char( &parser, 'a' );

    /* assert */
    ck_assert_int_eq( true, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_int_eq( 'b', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_char_failed )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n  \t cb" );

    /* act */
    bool result = match_char( &parser, 'a' );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_int_eq( 'c', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_char_eof )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "x" );
    get_char( &parser );

    /* act */
    bool result = match_char( &parser, 'a' );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_int_eq( EOF, get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_optional_char )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "ab" );

    /* act */
    bool result = match_optional_char( &parser, 'a' );

    /* assert */
    ck_assert_int_eq( true, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_int_eq( 'b', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_optional_char_failed )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "ab" );

    /* act */
    bool result = match_optional_char( &parser, 'c' );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_int_eq( 'a', get_char( &parser ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_str_token )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "abc xyz" );

    /* act */
    bool result = match_str_token( &parser );

    /* assert */
    ck_assert_int_eq( true, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "abc", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_str_token_spaces )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "  \n\r\tabc xyz" );

    /* act */
    bool result = match_str_token( &parser );

    /* assert */
    ck_assert_int_eq( true, result );
    ck_assert_int_eq( false, parser.error );
    ck_assert_str_eq( "abc", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_str_token_failed )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "  \n\r\t()" );

    /* act */
    bool result = match_str_token( &parser );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_str_eq( "", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_str_token_failed_exceeds_max_length )
{
    /* arrange */
    char str[MAX_STR_LEN + 100];
    int i;
    for ( i=0; i < sizeof( str ) - 1; i++ ){
        str[i] = 'A';
    }
    str[ sizeof(str) - 1] = '\0';

    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( str );

    /* act */
    bool result = match_str_token( &parser );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_int_eq( MAX_STR_LEN, strlen(parser.buffer ));

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_match_str_token_unclosed_quote )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "'abc" );

    /* act */
    bool result = match_str_token( &parser );

    /* assert */
    ck_assert_int_eq( false, result );
    ck_assert_int_eq( true, parser.error );
    ck_assert_str_eq( "abc", parser.buffer );

    /* clean up */
    fclose( parser.input );
}
END_TEST


/* ############################# PARSING ############################ */

START_TEST( test_read_matcher )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm)" );

    /* act */
    ght_matcher_t *matcher = read_matcher( &parser );

    /* assert */
    ck_assert_ptr_ne( NULL, matcher );

    ck_assert_str_eq( "WM_CLASS", matcher->name );
    ck_assert_str_eq( "xterm", matcher->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    free( matcher );
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_matcher_complex )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " \n\"unusual( )\"  (\t'complex term' ) " );

    /* act */
    ght_matcher_t *matcher = read_matcher( &parser );

    /* assert */
    ck_assert_ptr_ne( NULL, matcher );

    ck_assert_str_eq( "unusual( )", matcher->name );
    ck_assert_str_eq( "complex term", matcher->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    free( matcher );
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_matcher_failed )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "name value" );

    /* act */
    ght_matcher_t *matcher = read_matcher( &parser );

    /* assert */
    ck_assert_ptr_eq( NULL, matcher );
    ck_assert_int_eq( true, parser.error );

    /* clean up */
    free( matcher );
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_matcher_list )
{
    printf( "starting test_read_matcher_list\n" );
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm) WM_OTHER ( 'sp a ces' )\n\"SP ACE's\" ( abc ) ");

    ght_rule_t rule;
    rule.matchers.head = NULL;
    rule.matchers.tail = NULL;

    ght_matcher_t *a, *b, *c;

    /* act */
    int result = read_matcher_list( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    a = (ght_matcher_t *) rule.matchers.head;
    ck_assert_ptr_ne( NULL, a );

    b = (ght_matcher_t *) rule.matchers.head->next;
    ck_assert_ptr_ne( NULL, b );

    c = (ght_matcher_t *) rule.matchers.tail;
    ck_assert_ptr_ne( NULL, c );

    ck_assert_str_eq( "WM_CLASS", a->name );
    ck_assert_str_eq( "xterm", a->value );

    ck_assert_str_eq( "WM_OTHER", b->name );
    ck_assert_str_eq( "sp a ces", b->value );

    ck_assert_str_eq( "SP ACE's", c->name );
    ck_assert_str_eq( "abc", c->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    free( a );
    free( b );
    free( c );
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_matcher_list_partial_failure )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm) abc(fj *jf)");

    ght_rule_t rule;
    rule.matchers.head = NULL;
    rule.matchers.tail = NULL;

    /* act */
    int result = read_matcher_list( &parser, &rule );

    /* assert */
    ck_assert_int_eq( false, result );

    ck_assert_ptr_eq( NULL, rule.matchers.head );
    ck_assert_ptr_eq( NULL, rule.matchers.tail );

    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_matcher_list_total_failure )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "*&4");

    ght_rule_t rule;
    rule.matchers.head = NULL;
    rule.matchers.tail = NULL;

    /* act */
    int result = read_matcher_list( &parser, &rule );

    /* assert */
    ck_assert_int_eq( false, result );

    ck_assert_ptr_eq( NULL, rule.matchers.head );
    ck_assert_ptr_eq( NULL, rule.matchers.tail );

    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST


START_TEST( test_read_rule_body )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{\n\tfocus: 0.8;\n\tnormal: 0.4;\n}");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 8, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 4, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_short_form )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{f:0.8;N:0.4;}");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 8, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 4, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_uses_quotes_and_ucase )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{\n\t'FOCUS' : 99.8;\n\t\"NORmal\" : 5.4;\n}");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 998, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 54, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_missing_normal_uses_one )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{ focus:0.4; }");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 4, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_missing_focus_uses_one )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{ normal:0.4; }");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 10, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 4, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_missing_both_uses_one )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{ }");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_int_eq( 10, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_parsing_fails )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{ focus: x; }");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( false, result );

    ck_assert_int_eq( 10, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_body_unknown_parameter )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "{ fake: 0.3; }");

    ght_rule_t rule;

    /* act */
    bool result = read_rule_body( &parser, &rule );

    /* assert */
    ck_assert_int_eq( false, result );

    ck_assert_int_eq( 10, (int)( rule.focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( rule.normal_opacity * 10 ));

    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST


START_TEST( test_read_rule_list )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm) {\n\tfocus: 0.8;\n\tnormal: 0.4;\n} WM_OTHER(Abc) {f:0.2;n:1;}" );

    list_t rules = { NULL, NULL };

    ght_rule_t *a, *b;
    ght_matcher_t *am, *bm;


    /* act */
    bool result = read_rule_list( &parser, &rules );

    /* assert */
    ck_assert_int_eq( true, result );

    /* check the first rule */
    a = (ght_rule_t *) rules.head;
    ck_assert_ptr_ne( NULL, a );
    ck_assert_int_eq( 8, (int)( a->focus_opacity * 10 ));
    ck_assert_int_eq( 4, (int)( a->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, a->matchers.head );
    ck_assert_ptr_eq( a->matchers.head, a->matchers.tail ); /* only one in list */

    am = (ght_matcher_t *) a->matchers.head;
    ck_assert_str_eq( "WM_CLASS", am->name );
    ck_assert_str_eq( "xterm", am->value );


    /* check the second rule */
    b = (ght_rule_t *) rules.tail;
    ck_assert_ptr_ne( NULL, b );
    ck_assert_int_eq( 2, (int)( b->focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( b->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, b->matchers.head );
    ck_assert_ptr_eq( b->matchers.head, b->matchers.tail ); /* only one in list */

    bm = (ght_matcher_t *) b->matchers.head;
    ck_assert_str_eq( "WM_OTHER", bm->name );
    ck_assert_str_eq( "Abc", bm->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_list_combined_rule_body )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm), WM_OTHER(Abc) {f:0.2;n:1;}" );

    list_t rules = { NULL, NULL };

    ght_rule_t *a, *b;
    ght_matcher_t *am, *bm;

    /* act */
    bool result = read_rule_list( &parser, &rules );

    /* assert */
    ck_assert_int_eq( true, result );

    /* check the first rule */
    a = (ght_rule_t *) rules.head;
    ck_assert_ptr_ne( NULL, a );
    ck_assert_int_eq( 2, (int)( a->focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( a->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, a->matchers.head );
    ck_assert_ptr_eq( a->matchers.head, a->matchers.tail ); /* only one in list */

    am = (ght_matcher_t *) a->matchers.head;
    ck_assert_str_eq( "WM_CLASS", am->name );
    ck_assert_str_eq( "xterm", am->value );


    /* check the second rule */
    b = (ght_rule_t *) rules.tail;
    ck_assert_ptr_ne( NULL, b );
    ck_assert_int_eq( 2, (int)( b->focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( b->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, b->matchers.head );
    ck_assert_ptr_eq( b->matchers.head, b->matchers.tail ); /* only one in list */

    bm = (ght_matcher_t *) b->matchers.head;
    ck_assert_str_eq( "WM_OTHER", bm->name );
    ck_assert_str_eq( "Abc", bm->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_list_empty )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " " );

    list_t rules = { NULL, NULL };

    ght_rule_t *a, *b;
    ght_matcher_t *am, *bm;

    /* act */
    bool result = read_rule_list( &parser, &rules );

    /* assert */
    ck_assert_int_eq( true, result );

    ck_assert_ptr_eq( NULL, rules.head );
    ck_assert_ptr_eq( NULL, rules.tail );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_read_rule_list_failed_parsing )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( " WM_CLASS(xterm) , WM_OTHER(Abc) {f:0.2;n:1;} xyz  " );

    list_t rules = { NULL, NULL };

    ght_rule_t *a, *b;
    ght_matcher_t *am, *bm;

    /* act */
    bool result = read_rule_list( &parser, &rules );

    /* assert */
    ck_assert_int_eq( false, result );

    ck_assert_ptr_eq( NULL, rules.head );
    ck_assert_ptr_eq( NULL, rules.tail );

    ck_assert_int_eq( true, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

START_TEST( test_ght_parse_rules_from_string )
{
    /* arrange */
    ght_parser_t parser = DEFAULT_PARSER;
    parser.input = str_file( "WM_CLASS(xterm), WM_OTHER(Abc) {f:0.2;n:1;} WM_CLASS(thunar) WM_NAME(def) {focus:0.8;normal:0.4;}" );

    list_t rules = { NULL, NULL };

    ght_rule_t *a, *b, *c;
    ght_matcher_t *am, *bm, *cm1, *cm2;

    /* act */
    bool result = read_rule_list( &parser, &rules );

    /* assert */
    ck_assert_int_eq( true, result );

    /* check the first rule */
    a = (ght_rule_t *) rules.head;
    ck_assert_ptr_ne( NULL, a );
    ck_assert_int_eq( 2, (int)( a->focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( a->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, a->matchers.head );
    ck_assert_ptr_eq( a->matchers.head, a->matchers.tail ); /* only one in list */

    am = (ght_matcher_t *) a->matchers.head;
    ck_assert_str_eq( "WM_CLASS", am->name );
    ck_assert_str_eq( "xterm", am->value );


    /* check the second rule */
    b = (ght_rule_t *) rules.head->next;
    ck_assert_ptr_ne( NULL, b );
    ck_assert_int_eq( 2, (int)( b->focus_opacity * 10 ));
    ck_assert_int_eq( 10, (int)( b->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, b->matchers.head );
    ck_assert_ptr_eq( b->matchers.head, b->matchers.tail ); /* only one in list */

    bm = (ght_matcher_t *) b->matchers.head;
    ck_assert_str_eq( "WM_OTHER", bm->name );
    ck_assert_str_eq( "Abc", bm->value );

    ck_assert_int_eq( false, parser.error );

    /* check the third rule */
    c = (ght_rule_t *) rules.tail;
    ck_assert_ptr_ne( NULL, c );
    ck_assert_int_eq( 8, (int)( c->focus_opacity * 10 ));
    ck_assert_int_eq( 4, (int)( c->normal_opacity * 10 ));

    ck_assert_ptr_ne( NULL, c->matchers.head );
    ck_assert_ptr_ne( NULL, c->matchers.head->next );
    ck_assert_ptr_eq( c->matchers.head->next, c->matchers.tail ); /* two in list */

    cm1 = (ght_matcher_t *) c->matchers.head;
    ck_assert_str_eq( "WM_CLASS", cm1->name );
    ck_assert_str_eq( "thunar", cm1->value );

    cm2 = (ght_matcher_t *) c->matchers.tail;
    ck_assert_str_eq( "WM_NAME", cm2->name );
    ck_assert_str_eq( "def", cm2->value );

    ck_assert_int_eq( false, parser.error );

    /* clean up */
    fclose( parser.input );
}
END_TEST

Suite *
ghost_parser_suite()
{
	Suite *suite;
	TCase *tc_input, *tc_parsing;

	/* build the suite */
	suite = suite_create( "ghost_parser" );

	/* build the IO test case */
	tc_input = tcase_create( "Input" );

	/* add the individual tests */
	tcase_add_test( tc_input, test_default_parser );

	tcase_add_test( tc_input, test_get_char );
	tcase_add_test( tc_input, test_get_char_tracks_position );

	tcase_add_test( tc_input, test_peek_char );

	tcase_add_test( tc_input, test_is_valid_str_char );

	tcase_add_test( tc_input, test_is_valid_str_start_char );

	tcase_add_test( tc_input, test_read_str_token );
	tcase_add_test( tc_input, test_read_str_token_ignores_initial_spaces );
	tcase_add_test( tc_input, test_read_str_token_double_quotes );
	tcase_add_test( tc_input, test_read_str_token_single_quotes );
	tcase_add_test( tc_input, test_read_str_token_empty_token );
	tcase_add_test( tc_input, test_read_str_token_multiple_calls );
	tcase_add_test( tc_input, test_read_str_token_exceeds_max_length );
	tcase_add_test( tc_input, test_read_str_token_unclosed_quote );

	tcase_add_test( tc_input, test_has_str_token );
	tcase_add_test( tc_input, test_has_str_token_failed );

	tcase_add_test( tc_input, test_read_double );
	tcase_add_test( tc_input, test_read_double_ignores_initial_spaces );
	tcase_add_test( tc_input, test_read_double_stops_at_first_non_digit );
	tcase_add_test( tc_input, test_read_double_multiple_decimal_points );
	tcase_add_test( tc_input, test_read_double_number_exceeds_max_str_len );
	tcase_add_test( tc_input, test_read_double_no_decimal );
	tcase_add_test( tc_input, test_read_double_no_digits_fails );

	tcase_add_test( tc_input, test_consume_space );
	tcase_add_test( tc_input, test_consume_space_no_space_found );

	tcase_add_test( tc_input, test_match_char );
	tcase_add_test( tc_input, test_match_char_spaces );
	tcase_add_test( tc_input, test_match_char_failed );
	tcase_add_test( tc_input, test_match_char_eof );

	tcase_add_test( tc_input, test_match_optional_char );
	tcase_add_test( tc_input, test_match_optional_char_failed );

	tcase_add_test( tc_input, test_match_str_token );
	tcase_add_test( tc_input, test_match_str_token_spaces );
	tcase_add_test( tc_input, test_match_str_token_failed );
	tcase_add_test( tc_input, test_match_str_token_failed_exceeds_max_length );
	tcase_add_test( tc_input, test_match_str_token_unclosed_quote );


	suite_add_tcase( suite, tc_input );

    /* build the Parsing test case */
    tc_parsing = tcase_create( "Parsing" );

    /* add the individual tests */
    tcase_add_test( tc_parsing, test_read_matcher );
    tcase_add_test( tc_parsing, test_read_matcher_complex );
    tcase_add_test( tc_parsing, test_read_matcher_failed );

    tcase_add_test( tc_parsing, test_read_matcher_list );
    tcase_add_test( tc_parsing, test_read_matcher_list_partial_failure );
    tcase_add_test( tc_parsing, test_read_matcher_list_total_failure );

    tcase_add_test( tc_parsing, test_read_rule_body );
    tcase_add_test( tc_parsing, test_read_rule_body_short_form );
    tcase_add_test( tc_parsing, test_read_rule_body_uses_quotes_and_ucase );
    tcase_add_test( tc_parsing, test_read_rule_body_missing_normal_uses_one );
    tcase_add_test( tc_parsing, test_read_rule_body_missing_focus_uses_one );
    tcase_add_test( tc_parsing, test_read_rule_body_missing_both_uses_one );
    tcase_add_test( tc_parsing, test_read_rule_body_parsing_fails );
    tcase_add_test( tc_parsing, test_read_rule_body_unknown_parameter );

    tcase_add_test( tc_parsing, test_read_rule_list );
    tcase_add_test( tc_parsing, test_read_rule_list_combined_rule_body );
    tcase_add_test( tc_parsing, test_read_rule_list_empty );
    tcase_add_test( tc_parsing, test_read_rule_list_failed_parsing );

    tcase_add_test( tc_parsing, test_ght_parse_rules_from_string );

    suite_add_tcase( suite, tc_parsing );

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
