// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-emg.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-27
 *
 *         Purpose:  scoring facility (EMG course)
 *
 *         License:  GPL
 */




#include "scoring-facility.hh"

using namespace std;

extern "C" {

gboolean
daScoringFacEMGProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& Ch = *(SScoringFacility::SChannel*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( Ch.emg_scale < 2500 )
			Ch.emg_scale *= 1.1;
	    break;
	case GDK_SCROLL_DOWN:
		if ( Ch.emg_scale > .001 )
			Ch.emg_scale /= 1.1;
	    break;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}

} // extern "C"

// eof

