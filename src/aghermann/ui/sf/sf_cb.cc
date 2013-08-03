/*
 *       File name:  aghermann/ui/sf/sf_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  scoring facility widget callbacks
 *
 *         License:  GPL
 */

#include "sf.hh"

using namespace std;
using namespace aghui;

extern "C" {


gboolean
wSF_delete_event_cb(
	GtkWidget*,
	GdkEvent*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF._p.close_this_SF_now = &SF;
	g_signal_emit_by_name( SF._p.bMainCloseThatSF, "clicked");

	return TRUE; // to stop other handlers from being invoked for the event
}

} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
