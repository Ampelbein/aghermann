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
#include <omp.h>
#endif

#include <gtk/gtk.h>
#include <unique/unique.h>

#include "globals.hh"
#include "ui/globals.hh"
#include "ui/ui.hh"
#include "ui/sm/sm.hh"



static UniqueResponse
message_received_cb( UniqueApp         *,
                     UniqueCommand      command,
                     UniqueMessageData *message,
                     guint              time_,
                     gpointer           )
{
	if ( aghui::__main_window__ == NULL )
		return UNIQUE_RESPONSE_OK;

	UniqueResponse res;

	switch ( command ) {
	case UNIQUE_ACTIVATE:
		// move the main window to the screen that sent us the command
		gtk_window_set_screen( aghui::__main_window__, unique_message_data_get_screen( message));
		gtk_window_present_with_time( aghui::__main_window__, time_);
		res = UNIQUE_RESPONSE_OK;
	    break;
	default:
		res = UNIQUE_RESPONSE_OK;
	    break;
	}

	return res;
}



void print_version();

int
main( int argc, char **argv)
{
	print_version();

	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s [exp_root_dir]\n", argv[0]);
			return 0;
		}

	gtk_init( &argc, &argv);

	// don't let user get us started twice
	aghui::__unique_app__ =
		unique_app_new_with_commands( "com.johnhommer.Aghermann", NULL,
					      "fafa", 1,
					      NULL);
	if ( unique_app_is_running( aghui::__unique_app__) ) {
		printf( "There is unique app, switching to it now\n");
		unique_app_send_message( aghui::__unique_app__, UNIQUE_ACTIVATE, NULL);
	} else {
		g_signal_connect( aghui::__unique_app__, "message-received",
				  (GCallback)message_received_cb,
				  NULL);

		agh::global::init();

		if ( aghui::prepare_for_expdesign() ) {
			aghui::pop_ok_message( NULL, "UI failed to initialize", "Your install is broken.");
			return 2;
		}

		aghui::SSessionChooser chooser (argv[optind]);
		// implicit read sessionrc, run

		gtk_main();
	}
	// g_object_unref (app); // abandon ship anyway

	return 0;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
