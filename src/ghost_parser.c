/* ghost_parser.c
 * Contains logic for parsing ghost rules.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ghost.h"
#include "ghost_data.h"
#include "ghost_parser.h"

static const char MATCHER_VALUE_START = '(';
static const char MATCHER_VALUE_END = '(';
static const char RULE_BODY_START = '{';
static const char RULE_BODY_END = '}';
static const char OPACITY_DELIMITER = ':';
static const char OPACITY_END = ';';

typedef struct {
    FILE *input;

    int lastchar;
    bool uselastchar;

    int linenum;
    int charnum;

    bool done;
    bool error;

    char buffer[MAX_STR_LEN];

} ght_parser_t;

static ght_parser_t DEFAULT_PARSER = {
    NULL,
    0,
    false,
    1,
    0,
    false,
    false
};

static char
get_char( ght_parser_t *p )
{
    if ( p->uselastchar ) {
        p->uselastchar = false;
        return p->lastchar;
    }

    int c = fgetc( p->input );

    if ( c == EOF ) {
        p->done = true;
    } else {
        if ( c == '\n' ) {
            p->linenum++;
            p->charnum = 1;
        } else {
            p->charnum++;
        }
    }

    p->lastchar = c;

    return c;
}

static char
get( ght_parser_t *p )
{
    int c;
    while ( isspace( c = get_char( p )));
    return c;
}

static char
peek( ght_parser_t *p )
{
    int c;
    c = get( p );
    p->uselastchar = true;

    return c;
}

static void
report_failure( ght_parser_t *p, char *msg )
{
    error( "Error parsing ghost rules at line %d, char %d: %s",
           p->linenum, p->charnum, msg );

    p->error = true;
}

static void
read_str_until( ght_parser_t *p, char terminator )
{
    int c;
    int idx = 0;

    c = peek( p );
    while ( c != terminator && c != EOF && idx < MAX_STR_LEN) {
        p->buffer[idx++] = get( p );

        c = peek( p );
    }

    /* null-terminate the buffer string */
    p->buffer[idx] = '\0';

    if ( c == EOF ) {
        /* we reached the end of the file before we found our
        expected terminator */
        report_failure( p, "Reached end of file while parsing" );
    }
    if ( idx >= MAX_STR_LEN ) {
        /* we attempted to read a string that was too big */
        report_failure( p, "String value exceeds maximum length" );
    }
}

static void
read_rule_body( ght_parser_t *p, ght_rule_t *r )
{

}

static void
read_matcher_value( ght_parser_t *p, ght_matcher_t *m )
{

}

static void
read_matcher_name( ght_parser_t *p, ght_matcher_t *m )
{

}

static ght_matcher_t *
read_matcher( ght_parser_t *p )
{

}

static void
read_matcher_list( ght_parser_t *p, ght_rule_t *rule )
{
    int count = 0;
    ght_matcher_t *matcher;

    while ( peek( p ) != RULE_BODY_START &&
            ( matcher = read_matcher( p )) != NULL ) {
        ght_list_push( &(rule->matchers), matcher );
    }

    if ( count < 1 ) {
        report_failure( p, "No matchers configured for rule!" );
    }
}

static ght_rule_t *
read_rule( ght_parser_t *p )
{
    ght_rule_t *new_rule = checked_malloc( sizeof( ght_rule_t ));

    read_matcher_list( p, new_rule );
    read_rule_body( p, new_rule );

    if ( p->error ) {
        /* TODO: free the memory from the rule */

        free( new_rule );
        return NULL;
    }
    return new_rule;
}

static int
read_rule_list( ght_parser_t *p, list_t *rules)
{
    int rulecount = 0;
    ght_rule_t *r;

    while ( peek( p ) != EOF &&
            ( r = read_rule( p )) != NULL ) {
        ght_list_push( rules, r );
        rulecount++;
    }

    return rulecount;
}


/* ################## Public Methods ################## */

bool
ght_parse_rules_from_file( char *filename, list_t *rules )
{
    ght_parser_t p = DEFAULT_PARSER;

    p.input = fopen( filename, "r" );
    if ( p.input == NULL ) {
        error( "Unable to open file with name %s\n" );
        return false;
    }

    read_rule_list( &p, rules );

    fclose( p.input );

    return p.error;
}


bool
ght_parse_rules_from_string( char *input, list_t *rules )
{
    ght_parser_t p = DEFAULT_PARSER;
    p.input = fmemopen( (void *)input, strlen(input), "r" );

    read_rule_list( &p, rules );

    fclose( p.input );

    return p.error;
}
