/* ghost.h
 * Primary ghost header file.
 */

#ifndef _GHOST_H_
#define _GHOST_H_

#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>
#include "ghost_data.h"

/* debug levels
0 - none
1 - info
2 - debug
*/
#define DEBUG_LEVEL 3 
#define debug( LEVEL, ...) if ( LEVEL <= DEBUG_LEVEL ) { printf( __VA_ARGS__ ); }

#define error( ... ) fprintf( stderr, __VA_ARGS__ );

#define MAX_STR_LEN 128

/* Primary struct for tracking windows in ghost */
typedef struct ght_window_t {
	/* the window monitored by ghost */
	xcb_window_t win;

	/* the window that should have opacity adjustments made to it */
	xcb_window_t target_win;

	/* opacity settings */
	float focus_opacity;
	float normal_opacity;
} ght_window_t;

/* Contains a name-value pair for matching against string window properties */
typedef struct ght_matcher_t {
	char name[MAX_STR_LEN];
	char value[MAX_STR_LEN]; 

	/* required for use in lists */
	list_node_t node;
} ght_matcher_t;

typedef struct ght_rule_t {
	/* list of matchers */
	list_t matchers;

	/* opacity settings */
	float focus_opacity;
	float normal_opacity;

	/* required for use in lists */
	list_node_t node;
} ght_rule_t;

typedef struct ghost_t {
	/* the x11 connection */
	xcb_connection_t *conn;

	/* the x11 root window */
	xcb_window_t winroot;

	/* the list of rules for applying to windows */
	list_t rules;

	/* Mapping between xcb_window_t and ght_window_t. */
	map_t *win_map;			

	/* Mapping between string names and xcb_atom_t */
	map_t *atom_cache;
} ghost_t;

/* Creates a new ghost object. */
ghost_t *
ght_create( const char *displayname, int *screenp );

/* Destroys the given ghost object and frees its memory. */
void
ght_destroy( ghost_t *ghost );

/* Loads rule from the given file. */
int
ght_load_rule_file( ghost_t *ghost, const char *filepath );

/* Loads rule from the given string. */
int
ght_load_rule_str( ghost_t *ghost, const char *rule );

/* Searches all x windows for ones matching the rule and adds them to the tracked list. */
int
ght_load_windows( ghost_t *ghost );

/* Applies the normal rules in this ghost object to tracked windows. Focused and
unfocused states are not considered. */
int
ght_apply_normal_settings( ghost_t *ghost );

/* Enters a loop where x events are tracked and rules applied dynamically. */
void
ght_monitor( ghost_t *ghost );

#endif
