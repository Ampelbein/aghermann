/*
 *       File name:  aghermann/main.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-03
 *
 *         Purpose:  Function main
 *
 *         License:  GPL
 */



#ifdef _OPENMP
# include <omp.h>
#endif

#include <gtk/gtk.h>
#include <unique/unique.h>

#include "globals.hh"
#include "expdesign/expdesign.hh"
#include "ui/globals.hh"
#include "ui/ui.hh"
#include "ui/sm/sm.hh"

using namespace agh;

static UniqueResponse
message_received_cb(
	UniqueApp*,
	const UniqueCommand      command,
	UniqueMessageData       *message,
	const guint              time_,
	gpointer)
{
	if ( ui::global::main_window == NULL )
		return UNIQUE_RESPONSE_OK;

	UniqueResponse res;

	switch ( command ) {
	case UNIQUE_ACTIVATE:
		// move the main window to the screen that sent us the command
		gtk_window_set_screen(
			ui::global::main_window,
			unique_message_data_get_screen( message));
		gtk_window_present_with_time(
			ui::global::main_window,
			time_);
		res = UNIQUE_RESPONSE_OK;
	    break;
	default:
		res = UNIQUE_RESPONSE_OK;
	    break;
	}

	return res;
}



void print_version();

static void print_usage( const char*);

int
main( int argc, char **argv)
{
	print_version();

	bool headless = false;
	int	c;
	while ( (c = getopt( argc, argv, "hn")) != -1 )
		switch ( c ) {
		case 'n': // headless
			headless = true;
			break;
		case 'h':
			print_usage( argv[0]);
			return 0;
		}

	if ( headless ) {
		char*& explicit_session = argv[optind];
		if ( (!explicit_session || strlen(explicit_session) == 0) ) {
			fprintf( stderr, "Headless mode requires explicit session dir\n");
			print_usage( argv[0]);
			return -1;
		}

		global::init();
		CExpDesign ED (explicit_session); // essentially a very thoughtful no-op
		global::fini();

	} else {
		gtk_init( &argc, &argv);

		// don't let user get us started twice
		ui::global::unique_app =
			unique_app_new_with_commands(
				"com.johnhommer.Aghermann",
				NULL, "fafa", 1, NULL);
		if ( unique_app_is_running( ui::global::unique_app) ) {
			printf( "There is unique app, switching to it now\n");
			unique_app_send_message( ui::global::unique_app, UNIQUE_ACTIVATE, NULL);
		} else {
			g_signal_connect(
				ui::global::unique_app, "message-received",
				(GCallback)message_received_cb,
				NULL);

			agh::global::init();

			if ( ui::prepare_for_expdesign() ) {
				ui::pop_ok_message(
					NULL,
					"UI failed to initialize",
					"Your install is broken.");
				return 2;
			}

			ui::SSessionChooser chooser (argv[optind]);
			// implicit read sessionrc, run

			gtk_main();

			agh::global::fini();
		}
		// g_object_unref (app); // abandon ship anyway
	}

	return 0;
}

void
print_usage( const char* argv0)
{
	printf( "Usage: %s [-n] [exp_root_dir]\n", argv0);
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
