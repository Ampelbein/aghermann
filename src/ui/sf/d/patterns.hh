// ;-*-C++-*-
/*
 *       File name:  ui/sf/dialogs/patterns.hh
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
#include "sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace aghui {

struct SScoringFacility::SFindDialog {
	DELETE_DEFAULT_METHODS (SFindDialog);

      // ctor, dtor
	SFindDialog (SScoringFacility& parent);
       ~SFindDialog ();

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

} // namespace aghui

// eof
