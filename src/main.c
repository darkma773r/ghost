/* main.c
 *
 * Driver for ghost, a simple program to apply transparency to
 * windows in the X11 windowing system.
 */

#include <string.h>
#include "ghost.h"

/* Struct for passing around command line arguments */
typedef struct cmdargs_t {
    bool help;
    bool monitor;
    char *rulefile;
    char *rulestr;
} cmdargs_t;

/* Struct containing command line argument defaults */
cmdargs_t DEFAULT_ARGS = {
    0,
    0,
    NULL,
    NULL
};

/*
 * Prints a usage message and exits.
 */
static void
usage()
{
    fprintf( stderr,
             "\n##### GHOST #####\nA simple program for adding transparency to X windows.\n");
    fprintf( stderr,
             "Written by Matt Juntunen, 2014\n");
    fprintf( stderr, "(Log level set to %d)\n\n", LOG_LEVEL );
    fprintf( stderr,
             "USAGE: ghost [OPTIONS] [opacity rule string]\n" );
    fprintf( stderr,
             "   -h, --help      Display this message\n");
    fprintf( stderr,
             "   -f, --file      Indicates that the next argument is the name of a file containing "
             "ghost opacity rules.\n");
    fprintf( stderr,
             "   -m, --monitor   Enter monitoring mode. In this mode, the program will continuously "
             "monitor events from the X windowing system and apply opacity rules as needed.\n");

    fprintf( stderr, "\n" );
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
        } else if( FLAG_COMPARE( "-f", "--file", argv[i] )) {
            if ( i >= argc - 1 || argv[i+1][0] == '-' ) {
                error( "File flag given but no name specified!\n" );
                usage();
            }
            args.rulefile = argv[++i];
        } else if ( i == argc - 1 ) {
            /* if nothing else, use the last argument as the rule string */
            args.rulestr = argv[i];
        } else {
            error( "Unknown argument: %s\n", argv[i] );
            usage();
        }
    }

    if ( argc < 2 || args.help
            || ( args.rulefile == NULL && args.rulestr == NULL )) {
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

    info( "[main] ghost initialized\n", ghost->conn );

    /* load the rules */
    bool loaded = false;
    if ( args.rulefile != NULL ) {
        info( "[main] Loading rules from file %s\n", args.rulefile );
        loaded = ght_load_rule_file( ghost, args.rulefile );
    } else {
        info( "[main] Loading rules from command line argument\n" );
        debug( "[main] Rule str: %s\n", args.rulestr );
        loaded = ght_load_rule_str( ghost, args.rulestr );
    }

    if ( !loaded ) {
        error( "Failed to load ghost rules! Program exiting.\n" );
        exit( EXIT_FAILURE );
    }

    /* load windows */
    info( "[main] Loading windows...\n" );

    ght_load_windows( ghost );

    if ( !args.monitor ) {
        /* perform a "once-and-done opacity setting */
        info( "[main] Applying normal opacity rules\n" );
        ght_apply_opacity_settings( ghost, false );
    } else {
        /* enter monitor mode */
        info( "[main] Entering monitor mode...\n" );
        ght_apply_opacity_settings( ghost, true );

        /* down the rabbit hole, never to return ... */
        ght_monitor( ghost );
    }

    return EXIT_SUCCESS;
}
