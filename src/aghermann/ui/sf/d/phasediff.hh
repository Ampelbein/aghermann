/*
 *       File name:  aghermann/ui/sf/d/phasediff.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Phasediff child dialog
 *
 *         License:  GPL
 */

#ifndef AGHERMANN_UI_SF_D_PHASEDIFF_H_
#define AGHERMANN_UI_SF_D_PHASEDIFF_H_

#include "aghermann/ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace ui {

struct SPhasediffDialogWidgets {

	SPhasediffDialogWidgets (SScoringFacility&);
       ~SPhasediffDialogWidgets ();

	GtkBuilder *builder;

	GtkListStore
		*mSFPDChannels;

	// phasediff dialog
	GtkDialog
		*wSFPD;
	GtkComboBox
		*eSFPDChannelA, *eSFPDChannelB;
	GtkDrawingArea
		*daSFPD;
	GtkSpinButton
		*eSFPDFreqFrom,
		*eSFPDBandwidth;
	GtkScaleButton
		*eSFPDSmooth;
	gulong
		eSFPDChannelA_changed_cb_handler_id,
		eSFPDChannelB_changed_cb_handler_id;
};

struct SScoringFacility::SPhasediffDialog
  : public SPhasediffDialogWidgets {

	DELETE_DEFAULT_METHODS (SPhasediffDialog);

	SPhasediffDialog (SScoringFacility&);

	const SScoringFacility::SChannel
		*channel1,
		*channel2;
	bool	use_original_signal;
	float	from,
		upto;

	unsigned
		bwf_order,
		scope;
	float	display_scale;

	valarray<TFloat>
		course;
	size_t	smooth_side;
	void update_course();

	const SScoringFacility::SChannel* channel_from_cbox( GtkComboBox *cbox);
	void preselect_channel( GtkComboBox *cbox, const string& ch);

	void draw( cairo_t* cr, int wd, int ht);

	bool suspend_draw;

	SScoringFacility&
		_p;
};

}
} // namespace agh::ui

extern "C" {
void eSFPDChannelA_changed_cb( GtkComboBox*, gpointer);
void eSFPDChannelB_changed_cb( GtkComboBox*, gpointer);
gboolean daSFPD_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFPD_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void eSFPDChannelA_changed_cb( GtkComboBox*, gpointer);
void eSFPDChannelB_changed_cb( GtkComboBox*, gpointer);
void eSFPDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
void eSFPDBandwidth_value_changed_cb( GtkSpinButton*, gpointer);
void eSFPDSmooth_value_changed_cb( GtkScaleButton*, gdouble, gpointer);
void wSFPD_show_cb( GtkWidget*, gpointer);
void wSFPD_hide_cb( GtkWidget*, gpointer);
}


#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
