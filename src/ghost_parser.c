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

    UNDERSCORE = '_',

    PERIOD = '.',
    COMMA = ','
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
            inquotes = false;
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

    if ( inquotes && c == EOF ){
        /* unclosed quote */
        error( "Error parsing ghost rules at line %d, char %d: Unclosed quote: Expected %c but found end of file\n",
                    p->linenum, p->charnum, quote_char );
        p->error = true;
    }

    /* terminate the string in the buffer */
    p->buffer[len] = '\0';

    return len;
}

/*
 * Returns true is the next non-whitespace character looks like the beginning
 * of a valid string token.
 */
static bool
has_str_token( ght_parser_t * p ){
    consume_space( p );
    return is_valid_str_start_char( peek_char( p ));
}

/*
 * Reads a double from the input. The double is expected to contain at
 * least one digit with an optional decimal point and fraction component.
 * Initial whitespace is ignored and parsing ends at the first non-digit
 * character, with the exception of the single allowed decimal point.
 */
static double
read_double( ght_parser_t *p ){
    int c, idx = 0;
    bool found_decimal = false;

    /* consume initial spaces */
    consume_space( p );

    /* read the initial digit; this one is required */
    c = peek_char( p );
    if ( !isdigit( c )){
        error( "Error parsing ghost rules at line %d, char %d: Expected digit but received '%c'\n",
            p->linenum, p->charnum, c );
        p->error = true;
        return 0.0;
    }
    p->buffer[idx++] = get_char( p );

    c = peek_char( p );
    while ( isdigit( c ) || ( c == PERIOD && !found_decimal ) ){
        if ( c == PERIOD ){
            found_decimal = true;
        }

        if ( idx >= MAX_STR_LEN ){
            error( "Error parsing ghost rules at line %d, char %d: Number string exceeded maximum length of %d\n",
                    p->linenum, p->charnum, MAX_STR_LEN );
            p->error = true;
            return 0.0;
        }

        p->buffer[idx++] = get_char( p );

        c = peek_char( p );
    }

    /* null terminate the buffer string */
    p->buffer[idx] = '\0';

    return atof( p->buffer );
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
            error( "Error parsing ghost rules at line %d, char %d: Expected '%c' but found end of file\n",
                p->linenum, p->charnum, expected );
        } else {
            error( "Error parsing ghost rules at line %d, char %d: Expected '%c' but found '%c'\n",
                p->linenum, p->charnum, expected, c );
        }
        p->error = true;
        return false;
    }

    get_char( p );
    return true;
}

/*
 * Same as match_char but does not report an error on a mismatch.
 */
static bool
match_optional_char( ght_parser_t *p, int optional ){
    consume_space( p );
    if( peek_char( p ) != optional ){
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
        error( "Error parsing ghost rules at line %d, char %d: Expected string token but found '%c'\n",
            p->linenum, p->charnum, peek_char( p ));
        p->error = true;
    }
    return !p->error;
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
 * Reads a list of one or more matchers separated by whitespace. If the
 * operation is successful, parsed matchers are added to the matchers
 * list in rule. Returns true if the operation succeeded.
 *
 * matcher_list = <matcher> +
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
    while ( has_str_token( p )
            && (matcher = read_matcher( p ))){
        ght_list_push( &list, matcher );
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

/*
 * Reads a rule body from the input. A rule body consists of a set of
 * name-number pairs surrounded by curly braces. The allowed names are
 * "focus"/"f" and "normal"/"n". The names are not case-sensitive. The
 * format is:
 *
 * rule_body = { ((focus|f|normal|n) : <floatvalue>;) * }
 */
static bool
read_rule_body( ght_parser_t *p, ght_rule_t *r )
{
    /*
     * Set opacity defaults to opaque. This will be the least intrusive
     * to the window manager if something goes wrong.
     */
    r->normal_opacity = 1.0f;
    r->focus_opacity = 1.0f;

    /* start body */
    if ( !match_char( p, BRACE_OPEN )){
        return false;
    }

    /* read rule contents */
    while ( has_str_token( p )
           && match_str_token( p )){
        float *setting;

        /* read the parameter name */
        if ( strncasecmp( "focus", p->buffer, MAX_STR_LEN ) == 0
            || strncasecmp( "f", p->buffer, MAX_STR_LEN ) == 0){
            setting = &( r->focus_opacity );
        } else if ( strncasecmp( "normal", p->buffer, MAX_STR_LEN ) == 0
            || strncasecmp( "n", p->buffer, MAX_STR_LEN ) == 0) {
            setting = &( r->normal_opacity );
        } else {
            error( "Error parsing ghost rules at line %d, char %d: Unknown rule parameter '%s'\n",
                p->linenum, p->charnum, p->buffer );
            p->error = true;
            return false;
        }

        if ( !match_char( p , COLON )) {
            return false;
        }

        double dval = read_double( p );

        if ( p->error || !match_char( p, SEMICOLON )){
            return false;
        }

        /* everything matches; assign the value */
        *setting = (float) dval;
    }

    /* everything matched so far, so if we match the end, we're golden */
    return match_char( p, BRACE_END );
}

/*
 * Moves the ght_rule_t items from src to dst.
 */
static void
move_rule_list( list_t *src, list_t *dst ){
    ght_rule_t *rule;
    list_iter_t iter;
    ght_list_mod_for_each( src, &iter, rule, ght_rule_t ){
        ght_list_remove( src, rule );
        ght_list_push( dst, rule );
    }
}

/*
 * Clears the ght_rule_t items from the list and frees their memory.
 */
static void
free_rule_list( list_t *list ){
    ght_rule_t *rule;
    list_iter_t iter;
    ght_list_mod_for_each( list, &iter, rule, ght_rule_t ){
        ght_list_remove( list, rule );
        free( rule );
    }
}

/*
 * Creates and adds a rule for each matcher list found. Expects at least
 * one to be found.
 */
static bool
add_rule_for_each_matcher_list( ght_parser_t *p, list_t *rules )
{
    list_t temp = { NULL, NULL };
    list_iter_t iter;
    ght_rule_t *rule;

    do {
        rule = checked_malloc( sizeof( ght_rule_t ));
        ght_list_push( &temp, rule );
    } while ( read_matcher_list( p, rule )
             && match_optional_char( p, COMMA ));

    if ( p->error ){
        /* free the rules we've created */
        free_rule_list( &temp );
    } else {
        /* copy the rules to the final list */
        move_rule_list( &temp, rules );
    }

    return !p->error;
}

static bool
read_rule_list( ght_parser_t *p, list_t *rules)
{
    list_t empty = { NULL, NULL };
    list_t finished_rules = { NULL, NULL };
    list_t temp_rules;

    list_iter_t iter;

    ght_rule_t *rule;
    ght_rule_t body;
    int c;

    while ( has_str_token( p )){
        temp_rules = empty;

        /* read the rule matchers and body */
        if ( !add_rule_for_each_matcher_list( p, &temp_rules ) ||
            !read_rule_body( p, &body )){
            break;
        }

        /*
         * Since the rule body was valid, copy the settings to all
         * of the temporarily stored rules and add them to the final list.
         */
        ght_list_mod_for_each( &temp_rules, &iter, rule, ght_rule_t ){
            rule->focus_opacity = body.focus_opacity;
            rule->normal_opacity = body.normal_opacity;

            ght_list_remove( &temp_rules, rule );
            ght_list_push( &finished_rules, rule );
        }
    }

    if ( p->error ){
        /* clean up after errors */
        free_rule_list( &finished_rules );
        free_rule_list( &temp_rules );
    } else {
        /* copy successfully created rules to the final list */
        move_rule_list( &finished_rules, rules );
    }

    return !p->error;
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
