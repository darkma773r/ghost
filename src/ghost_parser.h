/* ghost_parser.h
 *
 * Header file for parsing ghost rules.
 */

#include "ghost.h"
#include "ghost_data.h"

/*
 * Parses rules from the given input file and adds them to the
 * rule list, in the order found. Returns the number of rules
 * added.
 */
int
ght_parse_rules_from_file( char *filename, list_t *rules );

/*
 * Parses rules from the given string and adds them to the
 * rule list, in the order found. Returns the number of rules
 * added.
 */
int
ght_parse_rules_from_string( char *filename, list_t *rules );
