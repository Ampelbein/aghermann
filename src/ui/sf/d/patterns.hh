/*
 *       File name:  ui/sf/d/patterns.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Patterns child dialog
 *
 *         License:  GPL
 */

#ifndef _AGH_UI_SF_PATTERNS_H
#define _AGH_UI_SF_PATTERNS_H

#include "patterns/patterns.hh"
#include "ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {


struct SPatternsDialogWidgets {

	SPatternsDialogWidgets (SScoringFacility&); // need access to mAllChannels
       ~SPatternsDialogWidgets ();

	GtkBuilder *builder;

	// find/patterns dialog
	GtkListStore
		*mSFFDPatterns;
	GtkDialog
		*wSFFD;
	GtkComboBox
		*eSFFDChannel,
		*eSFFDPatternList;
	GtkScrolledWindow
		*swSFFDThing,
		*swSFFDField;
	GtkTable
		*cSFFDParameters,
		*cSFFDCriteria,
		*cSFFDSearchButton,
		*cSFFDAgainButton;
	GtkBox	*cSFFDSearching;
	GtkDrawingArea
		*daSFFDThing,
		*daSFFDField;
	GtkMenu	*iiSFFDField;
	GtkButton
		*bSFFDSearch, *bSFFDAgain,
		*bSFFDProfileSave, *bSFFDProfileDiscard, *bSFFDProfileRevert;
	GtkSpinButton
		*eSFFDEnvTightness,
		*eSFFDBandPassFrom, *eSFFDBandPassUpto, *eSFFDBandPassOrder,
		*eSFFDDZCDFStep, *eSFFDDZCDFSigma, *eSFFDDZCDFSmooth,
		*eSFFDParameterA, *eSFFDParameterB,
		*eSFFDParameterC, *eSFFDParameterD;
	GtkHBox
		*cSFFDLabelBox;
	GtkLabel
		*lSFFDParametersBrief,
		*lSFFDFoundInfo;
	GtkDialog
		*wSFFDPatternSave;
	GtkEntry
		*eSFFDPatternSaveName;
	GtkToggleButton
		*eSFFDPatternSaveOriginSubject,
		*eSFFDPatternSaveOriginExperiment,
		*eSFFDPatternSaveOriginUser;
	gulong	eSFFDChannel_changed_cb_handler_id,
		eSFFDPatternList_changed_cb_handler_id;
};


struct SScoringFacility::SPatternsDialog
  : public SPatternsDialogWidgets{

	DELETE_DEFAULT_METHODS (SPatternsDialog);

      // ctor, dtor
	SPatternsDialog (SScoringFacility& parent);
       ~SPatternsDialog ();

      // saved patterns
	list<pattern::SPattern<TFloat>>
		patterns;
	list<pattern::SPattern<TFloat>>::iterator
		current_pattern;
	list<pattern::SPattern<TFloat>>::iterator
	pattern_by_idx( size_t);

	void import_from_selection( SScoringFacility::SChannel&);
	void load_patterns();
	void save_patterns();
	void discard_current_pattern();
	void populate_combo();

      // finding tool
  	pattern::SPatternPPack<TFloat>
		Pp2;
	pattern::CPatternTool<TFloat>
		*cpattern;
	double	increment; // in seconds

      // matches
	pattern::CMatch<TFloat>
		criteria;
	vector<pattern::CMatch<TFloat>>
		diff_line;
	vector<size_t>
		occurrences;
	size_t	highlighted_occurrence;
	void search();
	size_t find_occurrences();
	size_t nearest_occurrence( double) const;

      // field
	SScoringFacility::SChannel
		*field_channel,
		*field_channel_saved;
	list<sigfile::SAnnotation>
		saved_annotations;
	void occurrences_to_annotations();
	void save_annotations();
	void restore_annotations();

	metrics::TType
		field_profile_type; // where appropriate; otherwise draw compressed raw

      // draw
	bool	draw_details:1,
		suppress_w_v:1;
	void draw_thing( cairo_t*);
	void draw_field( cairo_t*);
	float	thing_display_scale,
		field_display_scale;

      // widgets
	SUIVarCollection
		W_V;

	void preselect_channel( const char*);

	void setup_controls_for_find();
	void setup_controls_for_wait();
	void setup_controls_for_tune();
	void set_profile_manage_buttons_visibility();

	static const int
		da_thing_ht = 200,
		da_field_ht = 130;
	int	da_thing_wd,
		da_field_wd;
	void set_thing_da_width( int);
	void set_field_da_width( int);

	aghui::SScoringFacility&
		_p;
};


} // namespace aghui

extern "C" {
void eSFFDPatternList_changed_cb( GtkComboBox*, gpointer);
void eSFFDChannel_changed_cb( GtkComboBox*, gpointer);
gboolean daSFFDField_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFFDField_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSFFDField_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFFDField_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFFDThing_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFFDThing_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void bSFFDSearch_clicked_cb( GtkButton*, gpointer);
void bSFFDAgain_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileSave_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileDiscard_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileRevert_clicked_cb( GtkButton*, gpointer);
void eSFFD_any_pattern_value_changed_cb( GtkSpinButton*, gpointer);
void eSFFD_any_criteria_value_changed_cb( GtkSpinButton*, gpointer);
void wSFFD_show_cb( GtkWidget*, gpointer);
void wSFFD_hide_cb( GtkWidget*, gpointer);
gboolean wSFFD_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
}

#endif // _AGH_UI_SF_PATTERNS_H

// eof
