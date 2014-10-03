/* ghost.h
 * Primary ghost header file.
 */

#ifndef _GHOST_H_
#define _GHOST_H_

#include <xcb/xcb.h>
#include "ghost_data.h"

typedef struct ght_window_t {
	/* the window monitored by ghost */
	xcb_window_t win;

	/* the window that should have opacity adjustments made to it */
	xcb_window_t target_win;

	/* opacity settings */
	float focus_opacity;
	float normal_opacity;
} ght_window_t;


typedef struct ghost_t {
	/* the x11 connection */
	xcb_connection_t *conn;

	/* Mapping between xcb_window_t and ght_window_t. */
	map_t win_map;			
} ghost_t;

/* Creates a new ghost object. */
ghost_t *
ght_create( const char *displayname, int *screenp );

/* Destroys the given ghost object and frees its memory. */
void
ght_destroy( ghost_t *ghost );

/* Loads criteria from the given file. */
int
ght_load_criteria_file( const char *filepath );

/* Loads criteria from the given string. */
int
ght_load_criteria_str( const char *criteria );

/* Searches all x windows for ones matching the criteria and adds them to the tracked list. */
int
ght_load_windows();

/* Applies the rules in this ghost object to tracked windows. */
int
ght_apply_rules( ghost_t *ghost );

/* Enters a loop where x events are tracked and rules applied dynamically. */
void
ght_monitor();

#endif
