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




#include <cairo/cairo-svg.h>

#include "misc.hh"
#include "ui.hh"
#include "settings.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace sf {


inline namespace {

}




} // namespace sf



// callbacks


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

} // namespace aghui


// eof

