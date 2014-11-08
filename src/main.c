/* main.c
 *
 * Driver for ghost, a simple program to apply transparency to
 * windows in the X11 windowing system.
 */

#define LOG_LEVEL 4

#include <string.h>
#include "ghost.h"

/* Struct for passing around command line arguments */
typedef struct cmdargs_t {
    bool help;
    bool monitor;
} cmdargs_t;

/* Struct containing command line argument defaults */
cmdargs_t DEFAULT_ARGS = {
    0,
    0
};

/*
 * Prints a usage message and exits.
 */
static void
usage()
{
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

/*
 * Macro for comparing the long and short versions of command-line
 * flags at once. It's just a lot of typing otherwise!
 */
#define FLAG_COMPARE(X,Y,Z) (strcmp((X),(Z)) == 0 || strcmp((Y),(Z)) == 0)

/*
 * Parses the command line arguments and displays the usage message if requested
 * or no arguments were given.
 */
cmdargs_t
parse_args( int argc, char **argv )
{
    cmdargs_t args = DEFAULT_ARGS;

    /* the first argument is the executable name so start at 1 */
    int i;
    for ( i=1; i<argc; i++ ) {
        if ( FLAG_COMPARE( "-h", "--help", argv[i] )) {
            args.help = 1;
        } else if ( FLAG_COMPARE( "-m", "--monitor", argv[i] )) {
            args.monitor = 1;
        } else {
            fprintf( stderr, "Unknown argument: %s\n", argv[i] );
            usage();
        }
    }

    if ( argc < 2 || args.help ) {
        usage();
    }

    return args;
}

/* Constant to help initialize lists */
static const list_t EMPTY_LIST = { NULL };

/* Main ghost reference */
ghost_t * ghost;

/*
 * If it's an entry point ye seek, then look no further.
 */
int
main( int argc, char **argv )
{
    /* check the command line */
    cmdargs_t args = parse_args( argc, argv );

    /* initialize ghost */
    ghost = ght_create( NULL, NULL );

    info( "Created ghost. conn = 0x%x\n", ghost->conn );

    /* TODO: load rules from the command line or a file here */
    ght_matcher_t *xterm = checked_malloc( sizeof( ght_matcher_t ));
    xterm->name_atom = atom_for_name( ghost, "WM_CLASS" );
    strcpy( xterm->value, "xterm" );

    ght_rule_t *rule = checked_malloc( sizeof( ght_rule_t ));
    rule->matchers = EMPTY_LIST;
    rule->focus_opacity = 0.8f;
    rule->normal_opacity = 0.7f;

    ght_list_push( &(rule->matchers), xterm );

    ght_list_push( &(ghost->rules), rule );

    /* load windows */
    info( "Loading windows...\n" );

    ght_load_windows( ghost );

    if ( !args.monitor ) {
        /* perform a "once-and-done opacity setting */
        info( "Applying normal opacity rules\n" );
        ght_apply_opacity_settings( ghost, false );
    } else {
        /* enter monitor mode */
        info( "Entering monitor mode...\n" );
        ght_apply_opacity_settings( ghost, true );

        /* down the rabbit hole, never to return ... */
        ght_monitor( ghost );
    }

    return EXIT_SUCCESS;
}
