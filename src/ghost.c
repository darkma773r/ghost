/* ghost.c
 *
 * A simple program to apply transparency to windows in the X11 windowing system.
 */

#include <string.h>

#define LOG_LEVEL 4

#include "ghost.h"

#define OPAQUE 0xffffffff
#define OPACITY "_NET_WM_WINDOW_OPACITY"

/* ################ Helper functions ################### */

/**
 * Returns an atom for the given name.
 */
static xcb_atom_t
atom_for_name( ghost_t *ghost, const char *name){
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;
    xcb_atom_t result;

    cookie = xcb_intern_atom( ghost->conn,
        0, /* only_if_exists; set to false to create the atom if needed */
        strlen(name), /* data length */
        name	/* the data */
    );

    reply = xcb_intern_atom_reply( ghost->conn,
        cookie,
        NULL /* error pointer */
    );

    if ( !reply ){
        error( "Unable to intern atom with name %s\n", name );
        return XCB_ATOM_NONE;
    }

    result = reply->atom;

    free( reply );

    return result;
}

/**
 * Applies the given float opacity to the window.
 */
static void
apply_opacity( ghost_t *ghost, ght_window_t *win, double opacity ){
    uint32_t val = (uint32_t) (opacity * OPAQUE);

    info( "setting opacity for window 0x%x to %d (%f)\n", win->target_win, val, opacity );

    xcb_change_property( ghost->conn, /* connection */
        XCB_PROP_MODE_REPLACE,	/* mode */
        win->target_win,	/* window */
        ghost->opacity_atom, /* atom to change */
        XCB_ATOM_CARDINAL,	/* property type */
        32,	/* format, meaning whether the data should be considered as a list of 8-bit, 16-bit, or 32-bit quantities */
        1,	/* data length */
        (unsigned char *) &val	/* the data for the property */
    );
    xcb_flush( ghost->conn );
}

/**
 * Returns a string property for the given window. The returned string must be
 * freed by the caller.
 */
static char *
get_string_property( ghost_t *ghost, xcb_window_t win, xcb_atom_t prop) {
    xcb_get_property_cookie_t prop_cookie;
    xcb_get_property_reply_t *reply;
    void *data;
    int len;
    char *result;

    prop_cookie = xcb_get_property( ghost->conn,
        0, /* delete */
        win,	/* the window */
        prop,	/* the property */
        XCB_ATOM_STRING, /* the property type */
        0,	/* data offset */
        MAX_STR_LEN /* the max length of the data */
    );

    xcb_flush( ghost->conn );

    reply = xcb_get_property_reply( ghost->conn,
        prop_cookie, /* the cookie */
        NULL	/* error pointer */
    );

    if ( !reply ){
        warn( "Unable to get property 0x%x from window 0x%x\n", prop, win);
        return NULL;
    }

    if ( reply->type == XCB_ATOM_NONE ){
        free( reply );
        return NULL;
    }

    data = xcb_get_property_value( reply );
    len = xcb_get_property_value_length( reply );

    if ( len < 1 ){
        return NULL;
    }

    result = checked_malloc( len + 1 );

    strncpy( result, data, len );

    free( reply );

    return result;
}

/**
 * Returns the xcb_window_t with the current input focus or 0
 * if it cannot be determined.
 */
static xcb_window_t
get_focused_window( ghost_t *ghost ){
    xcb_get_input_focus_cookie_t cookie;
    xcb_get_input_focus_reply_t *reply;
    xcb_window_t focused_window;

    cookie = xcb_get_input_focus( ghost->conn );

    reply = xcb_get_input_focus_reply( ghost->conn, cookie, NULL );

    if ( !reply ){
        warn( "Unable to determine current focused window\n" );
        return 0;
    }

    focused_window = reply->focus;

    free( reply );

    debug( "Found focused window: 0x%x\n", focused_window );

    return focused_window;
}

/**
 * Registers this client for events from the given window.
 */
static void
register_for_events( ghost_t *ghost, xcb_window_t win, uint32_t events ){
    uint32_t values[1];
    values[0] = events;

    xcb_change_window_attributes( ghost->conn, win, XCB_CW_EVENT_MASK, values );
    xcb_flush( ghost->conn );
}

/**
 * Gets the highest parent window that is not the root
 */
static xcb_window_t
get_top_window( ghost_t *ghost, xcb_window_t win ){
    xcb_window_t current;
    xcb_window_t parent;
    xcb_window_t root;
    xcb_query_tree_cookie_t tree_cookie;
    xcb_query_tree_reply_t *reply;

    current = win;
    while ( 1 ){
        tree_cookie = xcb_query_tree( ghost->conn, current );
        reply = xcb_query_tree_reply( ghost->conn,
            tree_cookie,
            NULL /* error pointer */
        );

        if ( !reply ){
            error( "Failed to query tree for window 0x%x\n", win );
            return 0;
        }

        parent = reply->parent;
        root = reply->root;

        free( reply );

        if ( !parent ) {
            /* no parent window, this must be the root */
            return 0;
        } else if ( parent == root ) {
            /* we found it! */
            return current;
        }

        current = parent;
    }

    return 0;
}

/**
 * Checks the given window against the rule and returns a configured
 * ght_window_t pointer if the window matches the rule. The caller is responsible
 * for freeing the ght_window_t memory.
 */
static ght_window_t *
check_window_against_rule( ghost_t *ghost, xcb_window_t win, ght_rule_t *rule ){
    /* go through each matcher in the rule to see if they all match */
    char *win_value;
    int matched = 0;

    ght_matcher_t *matcher;
    ght_list_for_each( &(rule->matchers), matcher, ght_matcher_t ){
        win_value = get_string_property( ghost,
            win, matcher->name_atom );

        matched = win_value != NULL
            && strncmp( win_value, matcher->value, MAX_STR_LEN ) == 0;

        free( win_value );

        if ( !matched ){
            return NULL;
        }
    }

    /* This window matched all values! Create a ghost window struct. */
    ght_window_t *ght_win = checked_malloc( sizeof( ght_window_t ));
    ght_win->win = win;
    ght_win->target_win = get_top_window( ghost, win );
    ght_win->focus_opacity = rule->focus_opacity;
    ght_win->normal_opacity = rule->normal_opacity;

    return ght_win;
}

/**
 * Returns a new ght_window_t if this window matches one of the configured
 * rules.
 */
static ght_window_t *
check_window( ghost_t *ghost, xcb_window_t win ){
    /* find the first rule that matches */
    ght_window_t *ght_win;
    int idx = 0;
    ght_rule_t *rule;
    ght_list_for_each( &(ghost->rules), rule, ght_rule_t ){
        ght_win = check_window_against_rule( ghost, win, rule );
        if ( ght_win != NULL ){
            debug( "[check_window] Found rule match for window 0x%x at index %d: "
                "normal=%f, focus=%f\n",
                win, idx,
                ght_win->normal_opacity,
                ght_win->focus_opacity );

            return ght_win;
        }

        ++idx;
    }

    return NULL;
}

/**
 * Returns the ght_window_t with the given xcb window id or NULL if not found.
 */
static ght_window_t *
find_window( ghost_t *ghost, xcb_window_t win ){
    return (ght_window_t *) ght_map_get( ghost->win_map, &win );
}

/**
 * Returns the ght_window_t with the given xcb target window or NULL if not found.
 * ght_window_t *.
 */
static ght_window_t *
find_window_by_target( ghost_t *ghost, xcb_window_t target ){
    return (ght_window_t *) ght_map_get( ghost->target_win_map, &target );
}

/**
 * Removes the given ght_window_t from the lookup maps and frees its memory.
 */
static void
untrack_window( ghost_t *ghost, ght_window_t *ght_win ){
    if ( ght_win == NULL ){
        return;
    }

    /* remove the window from the target map */
    ght_map_remove( ghost->target_win_map, &(ght_win->target_win));

    /* remove the window from the win map */
    ght_map_remove( ghost->win_map, &(ght_win->win));

    /* free the window memory */
    free( ght_win );
}

/**
 * Adds the given ght_window_t to the tracked window maps, freeing any
 * previous entry that may have been there.
 */
static void
track_window( ghost_t *ghost, ght_window_t *ght_win ){
    if ( ght_win == NULL ){
        return;
    }

    debug("[track_window] Adding window to tracked list: "
        "win=0x%x, target_win=0x%x, "
        "normal_opacity=%f, focus_opacity=%f\n",
        ght_win->win, ght_win->target_win,
        ght_win->normal_opacity, ght_win->focus_opacity );

    ght_window_t *prev = ght_map_put( ghost->win_map,
        &(ght_win->win),
        ght_win );

    /* free the previous version if we had one */
    if ( prev != NULL ){
        ght_map_remove( ghost->target_win_map,
                &(prev->target_win));
        free( prev );
    }

    /* add this entry to the parent window map */
    ght_map_put( ghost->target_win_map,
        &(ght_win->target_win),
        ght_win );
}

/**
 * Changes the parent/target window of the ght_window_t, updating the lookup maps
 * as needed.
 */
void
reparent_window( ghost_t *ghost, ght_window_t *ght_win, xcb_window_t new_parent ){
    if ( ght_win == NULL ){
        return;
    }

    /* remove the old entry */
    ght_map_remove( ghost->target_win_map, &(ght_win->target_win));

    /* set the new value */
    ght_win->target_win = new_parent;

    /* add the new entry to the target lookup map */
    ght_map_put( ghost->target_win_map, &new_parent, ght_win );
}

/**
 * Checks the given window and all child windows recursively.
 */
void
load_windows_recursive( ghost_t *ghost, xcb_window_t win ){
    xcb_query_tree_cookie_t tree_cookie;
    xcb_query_tree_reply_t *reply;
    int child_count;
    xcb_window_t *child;

    /* check this window */
    ght_window_t *ght_win = check_window( ghost, win );
    if ( ght_win != NULL ){
        /* start tracking the window */
        track_window( ghost, ght_win );
    }

    /* visit the child windows */
    tree_cookie = xcb_query_tree( ghost->conn, win );

    reply = xcb_query_tree_reply( ghost->conn,
        tree_cookie,
        NULL /* error pointer */
    );

    if ( !reply ){
        error( "Failed to query tree for window 0x%x\n", win );
        return;
    }

    child_count = xcb_query_tree_children_length( reply );

    child = xcb_query_tree_children( reply );
    int i;
    for ( i=0; i<child_count; i++ ){
        load_windows_recursive( ghost, child[i] );
    }

    free( reply );
}

/**
 * Clears all entries in the given map. If free_values is true, the
 * value memory will also be freed.
 */
static void
clear_dynamic_map( map_t *map, bool free_values){
    map_entry_t *entry;
    map_iter_t iter;
    ght_map_for_each_entry( map, &iter, entry ){
        if ( free_values ){
            free( entry->value );
        }
        ght_map_remove_entry( map, entry );
    }
}

/**
 * Clears and frees all memory associated with a list of matchers.
 */
static void
clear_matcher_list( list_t *matcher_list){
    list_iter_t iter;
    ght_matcher_t *matcher;
    ght_list_mod_for_each( matcher_list, &iter, matcher, ght_matcher_t ){
        ght_list_remove( matcher_list, matcher );
        free( matcher );
    }
}

/**
 * Clears and frees all memory associated with a list of rules.
 */
static void
clear_rule_list( list_t *rule_list ){
    list_iter_t iter;
    ght_rule_t *rule;
    ght_list_mod_for_each( rule_list, &iter, rule, ght_rule_t ){
        clear_matcher_list( &(rule->matchers) );
        ght_list_remove( rule_list, rule );
        free( rule );
    }
}

/* ##################### Ghost functions ################## */

static const list_t EMPTY_LIST = { NULL };

ghost_t *
ght_create( const char *displayname, int *screenp ){
    ghost_t *ghost = checked_malloc( sizeof( ghost_t ));

    /* initialize members */
    ghost->rules = EMPTY_LIST;
    ghost->win_map = ght_winmap_create( MAP_SIZE_LG );
    ghost->target_win_map = ght_winmap_create( MAP_SIZE_LG );

    /* connect to the x server */
    ghost->conn = xcb_connect( displayname, screenp );

    /* get the root window */
    const xcb_setup_t *setup = xcb_get_setup( ghost->conn );
    ghost->winroot = xcb_setup_roots_iterator( setup ).data->root;

    /* get the opacity atom */
    ghost->opacity_atom = atom_for_name( ghost, OPACITY );

    return ghost;
}

void
ght_destroy( ghost_t *ghost ){
    /* disconnect from the x server */
    xcb_disconnect( ghost->conn );

    debug( "disconnected\n" );

    /* clear the rules list */
    clear_rule_list( &(ghost->rules) );

    debug( "rules cleared\n" );

    /*
     * Clear the target window lookup map.
     * Dont' free the values here since they'll
     * be freed through the win_map.
     */
    clear_dynamic_map( ghost->target_win_map,  false );
    ght_map_free( ghost->target_win_map );

    debug( "target win map cleared\n" );

    /* clear and release the window map */
    clear_dynamic_map( ghost->win_map, true );
    ght_map_free( ghost->win_map );

    debug( "win map cleared\n" );


    /* free the ghost itself */
    free( ghost );

    debug( "ghost cleared\n" );
}

void
ght_load_windows( ghost_t *ghost ){
    /* clear the current maps */
    clear_dynamic_map( ghost->target_win_map, 0 );
    clear_dynamic_map( ghost->win_map, 1 );

    /* scan the whole window tree */
    load_windows_recursive( ghost, ghost->winroot );
}

void
ght_apply_opacity_settings( ghost_t *ghost, bool consider_focused_states ){
    xcb_window_t focus = 0;
    float opacity;
    map_iter_t iter;
    ght_window_t *ght_win;

    if ( consider_focused_states ){
        focus = get_focused_window( ghost );
    }

    ght_map_for_each( ghost->win_map, &iter, ght_win, ght_window_t * ){
        /* decide if we should use the normal or focused opacity setting */
        opacity = ght_win->normal_opacity;
        if ( focus == ght_win->win || focus == ght_win->target_win ){
            opacity = ght_win->focus_opacity;
        }

        apply_opacity( ghost, ght_win, opacity );
    }
}

void
ght_monitor( ghost_t *ghost ){
    register_for_events( ghost, ghost->winroot, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY );

    /* go through any existing windows and register for their events */
    ght_window_t *existing_win;
    map_iter_t iter;
    ght_map_for_each( ghost->win_map, &iter, existing_win, ght_window_t * ){
        register_for_events( ghost, existing_win->target_win, XCB_EVENT_MASK_FOCUS_CHANGE );
    }

    /* wait for new window events */
    xcb_generic_event_t *event;
    while ( event = xcb_wait_for_event( ghost->conn )){
        switch ( event->response_type & ~0x80 ){
            /* apply settings to new windows */
            case XCB_CREATE_NOTIFY: {
                xcb_create_notify_event_t *create_evt =
                    (xcb_create_notify_event_t *) event;
                debug( "Window created! 0x%x\n", create_evt->window );

                /* check the window and any of it's children for matches
                load_windows_recursive( ghost, create_evt->window ); */

                /* check this window */
                ght_window_t *ght_win = check_window( ghost, create_evt->window );
                if ( ght_win != NULL ){
                    track_window( ghost, ght_win );

                    /* register for focus events from the target window */
                    register_for_events( ghost, ght_win->target_win, XCB_EVENT_MASK_FOCUS_CHANGE );

                    /* apply the initial normal opacity */
                    apply_opacity( ghost, ght_win, ght_win->normal_opacity );
                }
                break;
            }
            /* this is needed for use with reparenting window managers since we
            may not be able to apply opacity settings to the correct window when
            the window is first created. */
            case XCB_REPARENT_NOTIFY: {
                xcb_reparent_notify_event_t *reparent_evt =
                    (xcb_reparent_notify_event_t *) event;
                debug( "Window reparented! 0x%x\n", reparent_evt->window );

                /* check if this is a tracked window */
                ght_window_t *ght_win = find_window( ghost, reparent_evt->window );
                if ( ght_win != NULL ){
                    debug( "old top window 0x%x\n", ght_win->target_win );

                    reparent_window( ghost, ght_win, get_top_window( ghost, ght_win->win ));

                    debug( "new top window 0x%x\n", ght_win->target_win );

                    /* register for focus events from the target window */
                    register_for_events( ghost, ght_win->target_win, XCB_EVENT_MASK_FOCUS_CHANGE );

                    /* apply the initial normal opacity */
                    apply_opacity( ghost, ght_win, ght_win->normal_opacity );
                }
                break;
            }
            case XCB_FOCUS_IN : {
                xcb_focus_in_event_t *in =
                    (xcb_focus_in_event_t *) event;
                debug( "Focus in 0x%x\n", in->event );

                ght_window_t *ght_win = find_window_by_target( ghost, in->event );
                if ( ght_win != NULL ){
                    apply_opacity( ghost, ght_win, ght_win->focus_opacity );
                }
                break;
            }
            case XCB_FOCUS_OUT : {
                xcb_focus_out_event_t *out =
                    (xcb_focus_out_event_t *) event;
                debug( "Focus out 0x%x\n", out->event );

                ght_window_t *ght_win = find_window_by_target( ghost, out->event );
                if ( ght_win != NULL ){
                    apply_opacity( ghost, ght_win, ght_win->normal_opacity );
                }

                break;
            }
            case XCB_DESTROY_NOTIFY : {
                xcb_destroy_notify_event_t *destroy_evt =
                    (xcb_destroy_notify_event_t *) event;
                debug( "Window destroyed 0x%x\n", destroy_evt->window );

                ght_window_t *ght_win = find_window( ghost, destroy_evt->window );
                if ( ght_win == NULL ){
                    ght_win = find_window_by_target( ghost, destroy_evt->window );
                }

                if ( ght_win != NULL ){
                    debug( "Untracking window. win= 0x%x, target_win= 0x%x\n",
                            ght_win->win, ght_win->target_win );
                    untrack_window( ghost, ght_win );
                }
                break;
            }
        }
        free( event );
    }
}


const char *matcher_class = "WM_CLASS";
const char *matcher_value = "xterm";

/* main method for testing purposes */
int
main(void){
    info( "Starting ghost\n" );
    /* create the ghost */
    ghost_t *ghost = ght_create( NULL, NULL );

    info( "Created ghost. conn=0x%x\n", ghost->conn );

    /* adding criteria */
    ght_matcher_t *xterm = checked_malloc( sizeof( ght_matcher_t ));
    xterm->name_atom = atom_for_name( ghost, matcher_class );
    info( "xterm name atom = 0x%x\n", xterm->name_atom );
    strcpy( xterm->value, matcher_value );

    ght_rule_t *rule = checked_malloc( sizeof( ght_rule_t ));
    rule->matchers = EMPTY_LIST;
    rule->focus_opacity = 0.8f;
    rule->normal_opacity = 0.7f;

    ght_list_push( &(rule->matchers), xterm );

    ght_list_push( &(ghost->rules), rule );

    /* loading windows */
    info( "Loading windows...\n" );

    ght_load_windows( ghost );

    info( "Done loading windows\n" );

    info( "Applying normal opacity rules\n" );

    ght_apply_opacity_settings( ghost, true );

    info( "Done applying normal settings\n" );

    info( "Monitoring the window events\n" );

    ght_monitor( ghost );

    /* destroy the ghost */
    ght_destroy( ghost );

    info( "Ghost destroyed\n" );

    return EXIT_SUCCESS;
}
