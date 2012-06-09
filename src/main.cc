// ;-*-C++-*- *
/*
 *       File name:  main.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-05-03
 *
 *         Purpose:  Function main
 *
 *         License:  GPL
 */




#include <gtk/gtk.h>
#include <unique/unique.h>

#include "ui/misc.hh"
#include "ui/expdesign.hh"



static GtkWindow *main_window = NULL;

static UniqueResponse
message_received_cb( UniqueApp         *app,
                     UniqueCommand      command,
                     UniqueMessageData *message,
                     guint              time_,
                     gpointer           user_data)
{
	if ( main_window == NULL )
		return UNIQUE_RESPONSE_OK;

	UniqueResponse res;

	switch ( command ) {
	case UNIQUE_ACTIVATE:
		/* move the main window to the screen that sent us the command */
		gtk_window_set_screen( main_window, unique_message_data_get_screen( message));
		gtk_window_present_with_time( main_window, time_);
		res = UNIQUE_RESPONSE_OK;
	    break;
	default:
		res = UNIQUE_RESPONSE_OK;
	    break;
	}

	return res;
}





int
main( int argc, char **argv)
{
	printf( PACKAGE_STRING " built " __DATE__ " " __TIME__ " by " BUILT_BY "\n");

	int	c;
	while ( (c = getopt( argc, argv, "h")) != -1 )
		switch ( c ) {
		case 'h':
			printf( "Usage: %s [exp_root_dir]\n", argv[0]);
			return 0;
		}

	gtk_init( &argc, &argv);

	// don't let user get us started twice
	UniqueApp *app =
		unique_app_new_with_commands( "com.johnhommer.Aghermann", NULL,
					      "fafa", 1,
					      NULL);

	if ( unique_app_is_running( app) )
		unique_app_send_message( app, UNIQUE_ACTIVATE, NULL);
	else {
		if ( aghui::prepare_for_expdesign() ) {
			aghui::pop_ok_message( NULL, "UI failed to initialize (try running " PACKAGE_NAME " in a terminal to see why)\n");
			return 2;
		}

		aghui::SExpDesignUI *ed;
		try {
			ed = new aghui::SExpDesignUI(
				(optind < argc)
				? argv[optind]
				: ""); // let ctor figure this from histfile
		} catch (runtime_error ex) {
			aghui::pop_ok_message( nullptr, "%s", ex.what());

			string new_experiment_dir = string(getenv("HOME")) + "/NewExperiment";
			if ( fs::mkdir_with_parents( new_experiment_dir.c_str()) ) {
				aghui::pop_ok_message( nullptr, "Failed to create a new directory in your $HOME."
						       " There's nothing we can do about that.");
				return 1;
			}
			ed = new aghui::SExpDesignUI( new_experiment_dir);
			// if HOME is non-writable, then don't catch: it's too seriously broken
		}

		main_window = ed->wMainWindow;

		unique_app_watch_window( app, (GtkWindow*)main_window);
		g_signal_connect( app, "message-received",
				  (GCallback)message_received_cb,
				  NULL);

		gtk_main();

		delete ed;
	}
	// g_object_unref (app); // abandon ship anyway

	return 0;
}

// eof
