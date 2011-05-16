// ;-*-C++-*- *  Time-stamp: "2011-05-15 15:10:01 hmmr"
/*
 *       File name:  ui/modelrun-facility.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-14
 *
 *         Purpose:  modelrun facility class and declarations
 *
 *         License:  GPL
 */

#ifndef _AGH_MODELRUN_FACILITY_H
#define _AGH_MODELRUN_FACILITY_H

#include "../libagh/enums.hh"
#include "misc.hh"
#include "ui.hh"
#include "settings.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace aghui {
namespace mf {

struct SModelrunFacility {

	agh::CSimulation&
		csimulation;
	agh::CSubject&
		csubject;

	double	cf;
	double snapshot()
		{
			return cf = csimulation.snapshot();
		}

	double	SWA_max;
	float	display_factor;
	int	zoomed_episode;
	size_t	SWA_smoothover;  // one side

	bool	_suppress_Vx_value_changed:1;

	SGeometry
		GeometryModRunFac;

	SModelrunFacility( agh::CSimulation&);
       ~SModelrunFacility();

	static void MF_siman_param_printer( void *xp);
	void siman_param_printer( void*);

	void draw_episode( cairo_t*, size_t ep,
			   size_t ep_start, size_t ep_end,
			   size_t tl_start, size_t tl_end);
	void draw_ticks( cairo_t*, size_t start, size_t end);
	void draw_timeline( cairo_t*);

	void update_infobar();

      // widgets
	int construct_widgets();
	GtkWindow
		*wModelrunFacility;

	GtkDrawingArea
		*daMFProfile;
	GtkTextView
		*lMFLog;
	GtkLabel
		*lMFCostFunction;
	GtkCheckButton
		*eMFLiveUpdate;
	map<GtkSpinButton*, TTunable>
		eMFVx;
	GtkHBox
		*cMFControls;
	GtkButton
		*bMFRun,
		*bMFReset,
		*bMFAccept;

	GtkTextBuffer
		*log_text_buffer;

	int	da_wd, da_ht;
	static const int tl_pad = 20;
	static const int lgd_margin = 20;
	static const int hypn_depth = 30;
	constexpr int	da_wd_actual() const
		{
			return da_wd - 2 * tl_pad;
		}
};


// gsl siman param printer bottleneck
extern SModelrunFacility*
	__MF;


} // namespace mf

extern "C" {
	gboolean daMFProfile_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
	gboolean daMFProfile_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daMFProfile_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daMFProfile_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	void bMFRun_clicked_cb( GtkButton*, gpointer);
	void bMFReset_clicked_cb( GtkButton*, gpointer);
	void bMFAccept_clicked_cb( GtkButton*, gpointer);

	void eMFVx_value_changed_cb  ( GtkSpinButton*, gpointer);

	void wModelrunFacility_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
}

} // namespace aghui

#endif // _AGH_MODELRUN_FACILITY_H

// eof
