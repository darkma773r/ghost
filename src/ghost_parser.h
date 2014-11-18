/* ghost_parser.h
 *
 * Header file for parsing ghost rules. Rules are given in
 * a format similar to CSS. A series of matchers specifying
 * X properties and corresponding values (not case-sensitive)
 * are given before a set of focus and normal opacities in
 * braces. Properties and values are specified with the name of
 * the X property followed by the desired value in parantheses.
 * Properties and values separated by spaces constitute a logical
 * AND while a comma represents an OR condition. AND is given a
 * higher precendence than OR. For example, the following rule
 * applies an opacity of 0.8 to focused windows and 0.6 to unfocused
 * windows that have either a class of "xterm" and name of "home"
 * or a class of "thunar":
 *
 * WM_CLASS( xterm ) WM_NAME( home ),
 * WM_CLASS( thunar ) {
 * 	focus: 0.8;
 * 	normal: 0.6;
 * } 
 *
 * Lines starting with '#' are considered comments. String tokens
 * can be surrounded with single or double quotes to 
 * allow strings with whitespace or other, non-alphanumeric
 * characters. Ex: 'WM_CLASS'( "some class" ) 
 *
 * The opacity settings "focus" and "normal" can be abbreviated
 * with "f" and "n".
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
