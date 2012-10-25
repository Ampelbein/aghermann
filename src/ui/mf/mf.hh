// ;-*-C++-*-
/*
 *       File name:  ui/mf/mf.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-14
 *
 *         Purpose:  modelrun facility class and declarations
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_MODELRUN_FACILITY_H
#define _AGH_UI_MODELRUN_FACILITY_H

#include "model/forward-decls.hh"
#include "ui/mw/mw.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

struct SModelrunFacility {

	DELETE_DEFAULT_METHODS (SModelrunFacility);

	SModelrunFacility (agh::ach::CModelRun&, SExpDesignUI&);
       ~SModelrunFacility ();

	agh::ach::CModelRun&
		csimulation;
	agh::CSubject&
		csubject;

	double	cf;
	double snapshot()
		{
			return cf = csimulation.snapshot();
		}

	void draw_episode( cairo_t*, size_t ep,
			   size_t ep_start, size_t ep_end,
			   size_t tl_start, size_t tl_end);
	void draw_ticks( cairo_t*, size_t start, size_t end);
	void draw_timeline( cairo_t*);
	void update_infobar();
	void siman_param_printer( void*);

	double	SWA_max;
	float	display_factor;
	int	zoomed_episode;
	static size_t
		swa_smoothover;  // one side

	bool	_suppress_Vx_value_changed:1,
		_tunables_header_printed:1,
		highlight_nrem:1,
		highlight_rem:1,
		highlight_wake:1;

	SGeometry
		GeometryModRunFac;

      // widgets
	GtkBuilder
		*builder;
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
		*eMFClassicFit;
	GtkLabel
		*lMFClassicFit;
	GtkScaleButton
		*eMFSmooth;
	GtkCheckButton
		*eMFLiveUpdate,
		*eMFHighlightWake,
		*eMFHighlightNREM,
		*eMFHighlightREM;
	map<GtkSpinButton*, agh::ach::TTunable>
		eMFVx;
	GtkCheckButton
		*eMFDB1,
		*eMFDB2,
		*eMFAZ1,
		*eMFAZ2;
	GtkHBox
		*cMFControls;
	GtkButton
		*bMFRun,
		*bMFReset;
	GtkToolButton
		*bMFAccept;

	GtkTextBuffer
		*log_text_buffer;

	int	da_wd, da_ht;
	static const int tl_pad = 20;
	static const int lgd_margin = 20;
	static const int hypn_depth = 35;
	int da_wd_actual() const
		{
			return da_wd - 2 * tl_pad;
		}

	SExpDesignUI&
		_p;
};


} // namespace aghui

#endif // _AGH_UI_MODELRUN_FACILITY_H

// eof
