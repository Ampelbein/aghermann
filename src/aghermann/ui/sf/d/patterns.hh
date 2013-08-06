/*
 *       File name:  aghermann/ui/sf/d/patterns.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-01-24
 *
 *         Purpose:  scoring facility Patterns child dialog
 *
 *         License:  GPL
 */

#ifndef AGH_AGHERMANN_UI_SF_D_PATTERNS_H_
#define AGH_AGHERMANN_UI_SF_D_PATTERNS_H_

#include "aghermann/patterns/patterns.hh"
#include "aghermann/ui/sf/sf.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif

using namespace std;

namespace agh {
namespace ui {


struct SPatternsDialogWidgets {

	SPatternsDialogWidgets (SScoringFacility&); // need access to mAllChannels
       ~SPatternsDialogWidgets ();

	GtkBuilder *builder;

	GtkListStore
		*mSFFDPatterns,
		*mSFFDChannels;
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
	GtkMenuBar
		*iibSFFDMenu;
	GtkMenu	*iiSFFDField,
		*iiSFFDFieldProfileTypes;
	GtkCheckMenuItem
		*iSFFDFieldDrawMatchIndex;
	GtkMenuItem
		*iSFFDMarkPhasicEventSpindles,
		*iSFFDMarkPhasicEventKComplexes,
		*iSFFDMarkPlain;
	GtkRadioMenuItem
		*iSFFDFieldProfileTypeRaw,
		*iSFFDFieldProfileTypePSD,
		*iSFFDFieldProfileTypeMC,
		*iSFFDFieldProfileTypeSWU;
	GtkButton
		*bSFFDSearch, *bSFFDAgain,
		*bSFFDProfileSave, *bSFFDProfileDiscard, *bSFFDProfileRevert;
	GtkSpinButton
		*eSFFDEnvTightness,
		*eSFFDBandPassFrom, *eSFFDBandPassUpto, *eSFFDBandPassOrder,
		*eSFFDDZCDFStep, *eSFFDDZCDFSigma, *eSFFDDZCDFSmooth,
		*eSFFDParameterA, *eSFFDParameterB,
		*eSFFDParameterC, *eSFFDParameterD,
		*eSFFDIncrement;
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
	GtkButton
		*bSFFDPatternSaveOK;
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

	int import_from_selection( SScoringFacility::SChannel&);
	void load_patterns();
	void save_patterns();
	void discard_current_pattern();
	void populate_combo();

      // finding tool
  	pattern::SPatternPPack<TFloat>
		Pp2;
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
	int	now_tweaking; // limit draw similarity index to this item

      // field
	SScoringFacility::SChannel
		*field_channel,
		*field_channel_saved;
	list<sigfile::SAnnotation>
		saved_annotations;
	void occurrences_to_annotations( sigfile::SAnnotation::TType = sigfile::SAnnotation::TType::plain);
	void save_annotations();
	void restore_annotations();

	metrics::TType
		field_profile_type; // where appropriate; otherwise draw compressed raw
	void update_field_check_menu_items();

      // draw
	bool	suppress_w_v:1,
		suppress_redraw:1,
		draw_details:1,
		draw_match_index:1;
	void draw_thing( cairo_t*);
	void draw_field( cairo_t*);
	float	thing_display_scale,
		field_display_scale;

      // widgets
	SUIVarCollection
		W_V;
	void atomic_up()
		{
			suppress_w_v = true;
			W_V.up();
			suppress_w_v = false;
		}

	void preselect_channel( int) const;

	void setup_controls_for_find();
	void setup_controls_for_wait();
	void setup_controls_for_tune();
	void set_profile_manage_buttons_visibility();

	static const int
		da_thing_ht = 200,
		da_field_ht = 160;
	int	da_thing_wd,
		da_field_wd;
	void set_thing_da_width( int);
	void set_field_da_width( int);

	agh::ui::SScoringFacility&
		_p;
};


}
} // namespace agh::ui

extern "C" {
void eSFFDPatternList_changed_cb( GtkComboBox*, gpointer);
void eSFFDChannel_changed_cb( GtkComboBox*, gpointer);
gboolean daSFFDField_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFFDField_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSFFDField_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFFDField_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFFDThing_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFFDThing_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFFDThing_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
void bSFFDSearch_clicked_cb( GtkButton*, gpointer);
void bSFFDAgain_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileSave_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileDiscard_clicked_cb( GtkButton*, gpointer);
void bSFFDProfileRevert_clicked_cb( GtkButton*, gpointer);
gboolean eSFFD_any_criteria_focus_in_event_cb(GtkWidget*, GdkEvent*, gpointer);
void wSFFD_show_cb( GtkWidget*, gpointer);
void wSFFD_hide_cb( GtkWidget*, gpointer);
gboolean wSFFD_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
void iSFFDFieldDrawMatchIndex_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFFDMarkPhasicEventSpindles_activate_cb( GtkMenuItem*, gpointer);
void iSFFDMarkPhasicEventKComplexes_activate_cb( GtkMenuItem*, gpointer);
void iSFFDMarkPlain_activate_cb( GtkMenuItem*, gpointer);
void eSFFDPatternSaveName_changed_cb(GtkEditable*, gpointer);

void eSFFD_any_pattern_origin_toggled_cb(GtkRadioButton*, gpointer);
void eSFFD_any_pattern_value_changed_cb( GtkSpinButton*, gpointer);
void eSFFD_any_criteria_value_changed_cb( GtkSpinButton*, gpointer);
void iSFFD_any_field_profile_type_toggled_cb( GtkRadioMenuItem*, gpointer);
}

#endif // AGH_AGHERMANN_UI_SF_D_PATTERNS_H_

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
