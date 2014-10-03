/* ghost.c
 * 
 * A simple program to apply transparency to windows in the X11 windowing system.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>
#include <inttypes.h>

#define MAX_STR_LEN 512
#define OPAQUE 0xffffffff
#define OPACITY "_NET_WM_WINDOW_OPACITY"


/* Returns an atom for the given name */
xcb_atom_t 
atom_for_name(xcb_connection_t *conn, const char *name){
	xcb_intern_atom_cookie_t cookie;
	xcb_intern_atom_reply_t *reply;
	xcb_atom_t result;
	
	cookie = xcb_intern_atom( conn,
		0, /* only_if_exists; set to false to create the atom if needed */
		strlen(name), /* data length */
		name	/* the data */
	);

	reply = xcb_intern_atom_reply( conn, 
		cookie, 
		NULL /* error pointer */
	);

	if ( !reply ){
		fprintf( stderr, "Unable to intern atom with name %s\n", name);
		return XCB_ATOM_NONE;
	}	

	result = reply->atom;

	free( reply );

	return result;	
}

/* Applies the given float opacity to the window. */
void 
apply_opacity( xcb_connection_t *conn, xcb_window_t win, double opacity ){
	uint32_t val = (uint32_t) (opacity * OPAQUE);	
	printf( "setting opacity for window 0x%x to %d\n", win, val );
	xcb_change_property( conn, /* connection */
		XCB_PROP_MODE_REPLACE,	/* mode */
		win,	/* window */
		atom_for_name( conn, OPACITY ), /* atom to change */
		XCB_ATOM_CARDINAL,	/* property type */
		32,	/* format, meaning whether the data should be considered as a list of 8-bit, 16-bit, or 32-bit quantities */
		1,	/* data length */
		(unsigned char *) &val	/* the data for the property */
	);
	xcb_flush( conn );
}

/* Returns a string property for the given window. The returned string must be
freed by the caller. */
char *
get_string_property( xcb_connection_t *conn, xcb_window_t win, xcb_atom_t prop) {
	xcb_get_property_cookie_t prop_cookie;
	xcb_get_property_reply_t *reply;
	void *data;
	int len;
	char *result;

	prop_cookie = xcb_get_property( conn,
		0, /* _delete */
		win,	/* the window */
		prop,	/* the property */
		XCB_ATOM_STRING, /* the property type */
		0,	/* data offset */
		MAX_STR_LEN /* the max length of the data */
	);

	xcb_flush( conn );

	reply = xcb_get_property_reply( conn, 
		prop_cookie, /* the cookie */ 
		NULL	/* error pointer */
	); 

	if ( !reply ){
		fprintf( stderr, "Unable to get property 0x%x from window 0x%x\n", prop, win);
		return NULL;
	}

	if ( reply->type == XCB_ATOM_NONE ){
		return NULL;
	}

	data = xcb_get_property_value( reply );	
	len = xcb_get_property_value_length( reply );

	if ( len < 1 ){
		return NULL;
	}

	result = malloc( len + 1);
	
	if ( !result ){	
		fprintf( stderr, "Unable to allocate memory for string of length %d\n", len + 1); 
		return NULL;
	}

	strncpy( result, data, len );

	return result;
}

/* Registers this client for events from the given window. */
void
register_for_events( xcb_connection_t *conn, xcb_window_t win, uint32_t events ){
	uint32_t values[1];
	values[0] = events;

	xcb_change_window_attributes( conn, win, XCB_CW_EVENT_MASK, values );
	xcb_flush( conn );
} 
 

/* list the properties available on the given window */
void
list_properties( xcb_connection_t *conn, xcb_window_t win ){
	xcb_list_properties_cookie_t cookie;
	xcb_list_properties_reply_t *reply;

	xcb_atom_t *atoms;
	int atoms_len;

	xcb_get_atom_name_cookie_t name_cookie;
	xcb_get_atom_name_reply_t *name_reply;
	int name_len;
	char *name_buffer;

	char *value;
	
	cookie = xcb_list_properties( conn, win );
	reply = xcb_list_properties_reply( conn, 
		cookie, 
		NULL /* error pointer */ 
	);

	atoms = xcb_list_properties_atoms( reply );
	atoms_len = xcb_list_properties_atoms_length( reply );

	int i;
	for ( i=0; i<atoms_len; i++ ){
		name_cookie = xcb_get_atom_name( conn, atoms[i] );	
		name_reply = xcb_get_atom_name_reply( conn,
			name_cookie,
			NULL /* error pointer */
		);
		name_len = xcb_get_atom_name_name_length( name_reply );	
		name_buffer = malloc( name_len + 1 );

		strncpy( name_buffer, 
			xcb_get_atom_name_name( name_reply ),
			name_len
		);

		printf( "->%s\n", name_buffer );

		free( name_buffer );

		value = get_string_property( conn, win, atoms[i] );	

		if ( value ){
			printf( "\t%s\n", value );
			free( value );
		}
	}

	free( reply );
}

/* Gets the highest parent window that is not the root */
xcb_window_t
get_top_window( xcb_connection_t *conn, xcb_window_t win ){
	xcb_window_t current;
	xcb_window_t parent;
	xcb_window_t root;
	xcb_query_tree_cookie_t tree_cookie;
	xcb_query_tree_reply_t *reply;

	current = win;
	while ( 1 ){
		tree_cookie = xcb_query_tree( conn, current );
		reply = xcb_query_tree_reply( conn,
			tree_cookie,
			NULL /* error pointer */
		);

		if ( !reply ){
			fprintf( stderr, "Failed to query tree for window 0x%x\n", win );		
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

char *checked_properties[] = {
	"WM_CLASS",
	"_NET_WM_CLASS",
	"WM_NAME",
	"_NET_WM_NAME"
};

/* Checks the configuration for a rule matching this window and
applies the specified opacity setting if it exists. */
void 
check_window( xcb_connection_t *conn, xcb_window_t win ){
	printf( "checking window 0x%x\n", win );
	char *str;
	int len = sizeof( checked_properties ) / sizeof( char * );
	xcb_window_t target_win;

	int i, done = 0;
	for ( i=0; i<len; i++ ){
		str = get_string_property( conn,
				win,
				atom_for_name( conn, checked_properties[i] )
			);
		if ( str ){
			if ( strcmp( str, "xterm") == 0 ){
				printf( "found xterm! i = %d\n", i );
					
				if (target_win = get_top_window( conn, win)) {
					apply_opacity( conn, target_win, 0.85 );	
					list_properties( conn, win );
				} else {
					fprintf( stderr, "Unable to find top level window for window 0x%x",
						win);
				}	
				done = 1;
			}	
			free( str );
		}

		if ( done ){
			break;
		}		
	}	
}

/* Checks the given window and all child windows recursively. */
void 
check_windows_recursive(xcb_connection_t *conn, xcb_window_t win){
	xcb_query_tree_cookie_t tree_cookie;
	xcb_query_tree_reply_t *reply;
	int child_count;
	xcb_window_t *child;

	/* visit this window */
	check_window( conn, win );

	/* visit the child windows */
	tree_cookie = xcb_query_tree(conn, win);

	reply = xcb_query_tree_reply( conn, 
		tree_cookie, 
		NULL /* error pointer */
	);

	if ( !reply ){
		fprintf( stderr, "Failed to query tree for window 0x%x\n", win );		
		return;
	}

	child_count = xcb_query_tree_children_length( reply );

	child = xcb_query_tree_children( reply );
	int i;
	for ( i=0; i<child_count; i++ ){
		check_windows_recursive( conn, child[i] );	
	}

	free( reply );
}

/* Continuously waits for window events and applies opacity settings as needed. 
This method does not return. */
void
monitor_window_events( xcb_connection_t *conn, xcb_window_t winroot ){
	/* register to receive new window events */
	uint32_t event_mask = XCB_CW_EVENT_MASK;
	uint32_t event_values[2] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
	};

	xcb_change_window_attributes_checked( conn,
		winroot,
		event_mask,
		event_values
	);
	xcb_flush( conn );
	
	/* wait for new window events */
	xcb_generic_event_t *event;
	while ( event = xcb_wait_for_event( conn )){
		switch ( event->response_type & ~0x80 ){
			/* apply settings to new windows */
			case XCB_CREATE_NOTIFY: {
				xcb_create_notify_event_t *create_evt = 
					(xcb_create_notify_event_t *) event;
				printf( "Window created! 0x%x\n", create_evt->window );
				check_window( conn, create_evt->window );
				break;
			}
			/* this is needed for use with reparenting window managers since we
			may not be able to apply opacity settings to the correct window when
			the window is first created. */
			case XCB_REPARENT_NOTIFY: {
				xcb_reparent_notify_event_t *reparent_evt =
					(xcb_reparent_notify_event_t *) event;
				printf( "Window reparented! 0x%x\n", reparent_evt->window );
				check_window( conn, reparent_evt->window );
				break;
			}		
		}
		free( event );
	}		
}
		
/* struct for passing around command line arguments */
typedef struct cmdargs_t {
	int help;
	int monitor;
} cmdargs_t;

/* struct containing command line argument defaults */
cmdargs_t DEFAULT_ARGS = {
	0,
	0
};

/* Prints a usage message and exits. */
void
usage(){
	fprintf( stderr,
		"GHOST\nA simple program for adding transparency to X windows.\n");
	fprintf( stderr,
		"Written by Matt Juntunen, 2014\n\n");
	fprintf( stderr,
		"USAGE: ghost [OPTIONS]\n" );
	fprintf( stderr,
		"   -h, --help      Display this message\n");
	fprintf( stderr,
		"   -m, --monitor   Enter monitoring mode. In this mode, the program will continuously "
		"monitor events from the X windowing system and apply opacity rules as needed.\n");
	
	exit( 1 );
}

/* Macro for comparing the long and short versions of command-line
flags at once. It's just a lot of typing otherwise! */
#define FLAG_COMPARE(X,Y,Z) (strcmp((X),(Z)) == 0 || strcmp((Y),(Z)) == 0) 

/* Parses the command line arguments and displays the usage message if requested
or no arguments were given. */
cmdargs_t
parse_args( int argc, char **argv ){
	cmdargs_t args = DEFAULT_ARGS;

	/* the first argument is the executable name so start at 1 */
	int i;
	for ( i=1; i<argc; i++ ){
		if ( FLAG_COMPARE( "-h", "--help", argv[i] )){
			args.help = 1;	
		} else if ( FLAG_COMPARE( "-m", "--monitor", argv[i] )){
			args.monitor = 1;
		} else {
			fprintf( stderr, "Unknown argument: %s\n", argv[i] );	
			usage();
		}		
	}	 

	if ( argc < 2 || args.help ){
		usage();
	}	

	return args;
} 

/* If it's an entry point ye seek, then look no further. */
int 
main( int argc, char **argv ){
	/* check the command line */
	cmdargs_t args = parse_args( argc, argv );

	xcb_connection_t *conn;
	const xcb_setup_t *setup;
	xcb_window_t winroot;

	/* connect to the X server */
	conn = xcb_connect( NULL, NULL );
	setup = xcb_get_setup( conn );

	/* get a reference to the root screen */
	winroot = xcb_setup_roots_iterator( setup ).data->root;

	/* check the currently open windows */
	check_windows_recursive( conn, winroot );	

	if ( args.monitor ){
		/* wait forever and apply settings based on window events */
		monitor_window_events( conn, winroot );
	}

	/* close the connection */
	xcb_disconnect( conn );

	return 0;
}
