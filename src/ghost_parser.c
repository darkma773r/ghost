/* ghost_parser.c
 * Contains logic for parsing ghost rules.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ghost.h"
#include "ghost_data.h"
#include "ghost_parser.h"

/* Define character constants */
enum {
    PAREN_OPEN = '(',
    PAREN_END = ')',

    BRACE_OPEN = '{',
    BRACE_END = '}',

    COLON = ':',
    SEMICOLON = ';',

    DOUBLE_QUOTE = '"',
    SINGLE_QUOTE = '\'',

    UNDERSCORE = '_'
};

/* Structure to hold parsing information. */
typedef struct {
    FILE *input;

    int lastchar;
    bool uselastchar;

    int linenum;
    int charnum;

    bool done;
    bool error;

    char buffer[MAX_STR_LEN + 1];

} ght_parser_t;

/* Default ght_parser_t value for initialization. */
static const ght_parser_t DEFAULT_PARSER = {
    NULL,
    0,
    false,
    1, /* line numbers start at 1 */
    0, /* char numbering starts at 1; this indicates the position before the first char */
    false,
    false
};

/*
 * Returns the next character from the input stream and advances the
 * stream to the next position. The parser linenum, charnum, and
 * done variables are updated.
 */
static char
get_char( ght_parser_t *p )
{
    if ( p->uselastchar ) {
        p->uselastchar = false;
        return p->lastchar;
    }

    int c = fgetc( p->input );

    /* update the parser position variables */
    if ( p->lastchar == '\n' ){
        /* start of new line */
        p->linenum++;
        p->charnum = 1;
    } else if ( p->lastchar != EOF ){
        /* advance character counter; do not advance when past content */
        p->charnum++;
    }

    if ( c == EOF ){
        p->done = true;
    }

    p->lastchar = c;

    return c;
}

/*
 * Returns the next character that will be returned by a call to get_char.
 */
static char
peek_char( ght_parser_t *p )
{
    int c;
    c = get_char( p );
    p->uselastchar = true;

    return c;
}

/*
 * Advances the input stream to the next non-whitespace character.
 */
static void
consume_space( ght_parser_t *p ){
    while ( isspace( peek_char( p ))){
        get_char( p );
    }
}

/*
 * Returns true if the given character should be considered
 * part of a string token.
 */
static inline bool
is_valid_str_char( int c ){
    return c == UNDERSCORE || isalnum(c);
}

/*
 * Returns true if the given character can be used to start a string token.
 */
static inline bool
is_valid_str_start_char( int c ){
    return c == DOUBLE_QUOTE
        || c == SINGLE_QUOTE
        || is_valid_str_char( c );
}


/*
 * Reads a string token into the parser buffer and returns its length.
 * Initial whitespace characters are ignored. If the first non-whitespace
 * character read from the stream is a single or double quote, characters
 * are added to the buffer until a matching quote character is found. Otherwise,
 * characters are read from the input stream until a character is found
 * that returns false in a call to is_valid_str_char. If the length of the
 * read string exceeds the value of MAX_STR_LEN, a message is printed and
 * parser.error is set to true.
 */
static int
read_str_token( ght_parser_t *p ){
    bool inquotes = false;
    char quote_char = 0;
    int len = 0, c;

    /* ignore initial spaces */
    consume_space( p );

    /* check if the first character is a quote */
    c = peek_char( p );
    if ( c == DOUBLE_QUOTE || c == SINGLE_QUOTE ){
        inquotes = true;
        quote_char = get_char( p );
    }

    /* read characters into the parser buffer */
    while (( c = peek_char( p )) != EOF ){
        if ( inquotes && c == quote_char ){
            /*
             * We've found the end of the quoted section.
             * Consume the end quote character and exit the loop.
             */
            get_char( p );
            break;
        } else if ( inquotes || is_valid_str_char( c )){
            if ( len >= MAX_STR_LEN ){
                error( "Error parsing ghost rules at line %d, char %d: String token exceeded maximum length of %d\n",
                    p->linenum, p->charnum, MAX_STR_LEN );
                p->error = true;
                break;
            }
            p->buffer[len++] = get_char( p );
        } else {
            break;
        }
    }

    /* terminate the string in the buffer */
    p->buffer[len] = '\0';

    return len;
}

/*
 * Compares expected to the next character in the stream and returns true
 * and consumes the character if the two match. Otherwise, reports an error
 * and returns false. Extra whitespace is consumed before the match is performed.
 */
static bool
match_char( ght_parser_t *p, int expected ){
    consume_space( p );

    int c = peek_char( p );
    if  ( c != expected ){
        if ( c == EOF ){
            error( "Error parsing ghost rules: Reached end of file while parsing: Expected '%c'\n", expected );
        } else {
            error( "Error parsing ghost rules at line %d, char %d: Expected '%c' but received '%c'\n",
                p->linenum, p->charnum, expected, c );
        }
        p->error = true;
        return false;
    }

    get_char( p );
    return true;
}

/*
 * Attempts to match a string token from the stream, returning true if successful.
 */
static bool
match_str_token( ght_parser_t *p ){
    if ( read_str_token( p ) < 1 ) {
        error( "Error parsing ghost rules at line %d, char %d: Expected string token but found %c\n",
            p->linenum, p->charnum, peek_char( p ));
        p->error = true;
    }
    return !p->error;
}

static void
read_rule_body( ght_parser_t *p, ght_rule_t *r )
{

}

/*
 * Reads a matcher from the input stream. A matcher consists of a string token
 * followed by another string token within parentheses. The matcher is returned
 * if found, otherwise NULL is returned. The caller is responsible for freeing
 * the matcher memory.
 *
 * matcher = <strtoken> ( <strtoken> )
 */
static ght_matcher_t *
read_matcher( ght_parser_t *p )
{
    ght_matcher_t *m = checked_malloc( sizeof( ght_matcher_t ));

    if ( match_str_token( p )
        && strncpy( m->name, p->buffer, MAX_STR_LEN )
        && match_char( p, PAREN_OPEN )
        && match_str_token( p )
        && strncpy( m->value, p->buffer, MAX_STR_LEN )
        && match_char( p, PAREN_END )){
        return m;
    }

    free( m );
    return NULL;
}

/*
 * Reads a list of one or more matchers. If the operation is successful,
 * parsed matchers are added to the matchers list in rule. Returns true
 * if the operation succeeded.
 *
 * matcher_list = <matcher>+
 */
static bool
read_matcher_list( ght_parser_t *p, ght_rule_t *rule )
{
    ght_matcher_t *matcher;
    list_t list = { NULL, NULL };

    /* we need at least one matcher so read it directly */
    matcher = read_matcher( p );
    if ( p->error ){
        return !p->error;
    }
    ght_list_push( &list, matcher );

    /* read any remaining matchers into the local list */
    consume_space( p );
    while ( is_valid_str_start_char( peek_char( p ))
            && (matcher = read_matcher( p ))){
        ght_list_push( &list, matcher );
        consume_space( p );
    }

    if ( p->error ){
        /* free the matchers we created if there was an error */
        list_iter_t iter;
        ght_matcher_t *cur;
        ght_list_mod_for_each( &list, &iter, cur, ght_matcher_t ){
            ght_list_remove( &list, cur );
            free( cur );
        }
    } else {
        /* we've made at least one matcher without an error; add it to the rule list */
        list_iter_t iter;
        ght_matcher_t *cur;
        ght_list_mod_for_each( &list, &iter, cur, ght_matcher_t ){
            ght_list_remove( &list, cur );
            debug( "[read_matcher_list] adding matcher [name= %s, value= %s]\n", cur->name, cur->value );
            ght_list_push( &(rule->matchers), cur );
        }
    }

    return !p->error;
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

    while ( peek_char( p ) != EOF &&
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
