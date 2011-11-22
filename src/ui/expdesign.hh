// ;-*-C++-*-
/*
 *       File name:  ui/expdesign.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-04-13
 *
 *         Purpose:  class SExpDesignUI
 *
 *         License:  GPL
 */

#include <string>
#include <list>
#include <forward_list>
#include <map>
#include <stdexcept>

#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "../libagh/boost-config-validate.hh"
#include "../libagh/primaries.hh"
#include "ui.hh"
#include "managed-colour.hh"
#include "forward-decls.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


#ifndef _AGH_EXPDESIGN_UI_H
#define _AGH_EXPDESIGN_UI_H

using namespace std;

namespace aghui {

// ui structures everything is public, mainly to give access to the
// bulk of extern "C" callbacks

class SExpDesignUI {
	SExpDesignUI( const SExpDesignUI&) = delete;
	SExpDesignUI& operator=( const SExpDesignUI&) = delete;

    public:
	agh::CExpDesign
		*ED;

      // forward decl
	class SGroupPresentation;

      // contained classes
	class SSubjectPresentation {
	    public:
		agh::CSubject&  // can't have it declared const due to CMSessionSet operator[] not permitting
			csubject;
		agh::CSCourse // a shortcut
			*cscourse;
		list<agh::CSubject::SEpisode>&
		sepisodesequence() const
			{
				return csubject.measurements[*_p._p._AghDi].episodes;
			}

		typedef list<agh::CSubject::SEpisode>::iterator TEpisodeIter;
		TEpisodeIter
			using_episode;
		bool is_episode_focused() const
			{
				return using_episode != sepisodesequence().end();
			}
		bool	is_focused;

		bool get_episode_from_timeline_click( unsigned along);  // possibly sets episode_focused

		time_t	tl_start;

		time_t timeline_start() const	{ return _p._p.timeline_start; }
		time_t timeline_end() const	{ return _p._p.timeline_end; }
		size_t timeline_width() const	{ return _p._p.timeline_width; }
		size_t timeline_height() const	{ return _p._p.timeline_height; }
		size_t tl_left_margin() const	{ return _p._p.tl_left_margin; }
		size_t tl_right_margin() const	{ return _p._p.tl_right_margin; }

		void draw_timeline( cairo_t *cr) const;
		void draw_timeline( const char *fname) const;

		SGroupPresentation& _p;
		SSubjectPresentation( agh::CSubject& _j, SGroupPresentation& parent);
	       ~SSubjectPresentation();

		GtkWidget
			*da;
	};
	class SGroupPresentation
	      : public list<SSubjectPresentation> {
		friend class aghui::SSubjectPresentation;
		friend class aghui::SExpDesignUI;

		agh::CExpDesign::TJGroups::iterator cjgroup;
	    public:
		const char* name() const
			{
				return cjgroup->first.c_str();
			}
		agh::CJGroup& group()
			{
				return cjgroup->second;
			}
		SGroupPresentation( agh::CExpDesign::TJGroups::iterator& _g,
				    SExpDesignUI& parent)
		      : cjgroup (_g),
			_p (parent),
			visible (true)
			{}

		SExpDesignUI&
			_p;

		bool	visible;
		GtkExpander
			*expander,
			*vbox;
	};
	list<SGroupPresentation>
		groups;
	SSubjectPresentation
		*using_subject;

      // ctor, dtor
	SExpDesignUI( const string& dir);
       ~SExpDesignUI();
	bool	finalize_ui:1,
		nodestroy_by_cb:1;

      // populate
	int populate( bool do_load);
	void depopulate( bool do_save);
	void populate_1();  // measurements
	void populate_2();  // simulations
	void cleanup_2();
	void do_rescan_tree( bool ensure = true); // with while ... gtk_main_iteration ...

	void show_empty_experiment_blurb();
	int try_download();

      // collected ED strings (channels, sessions, etc)
	list<string>
		AghDD,	AghGG,	AghEE;
	list<sigfile::SChannel>
		AghHH,	AghTT;
	list<sigfile::SChannel>::iterator
		_AghHi,	_AghTi;
	list<string>::iterator
		_AghGi,	_AghDi,	_AghEi;
	const char* AghH() const { return (_AghHi != AghHH.end()) ? _AghHi->c_str() : "(no channels)"; }
	const char* AghT() const { return (_AghTi != AghTT.end()) ? _AghTi->c_str() : "(no channels)"; }
	const char* AghG() const { return (_AghGi != AghGG.end()) ? _AghGi->c_str() : "(no groups)"; }
	const char* AghD() const { return (_AghDi != AghDD.end()) ? _AghDi->c_str() : "(no sessions)"; }
	const char* AghE() const { return (_AghEi != AghEE.end()) ? _AghEi->c_str() : "(no episodes)"; }
	int AghTi() const;
	int AghDi() const;

      // collected full-path annotations
	struct SAnnotation
	      : public agh::CSubject::SEpisode::SAnnotation {
		agh::CSubject& csubject;
		const string& session;
		agh::CSubject::SEpisode& sepisode;
		SAnnotation( agh::CSubject& j, const string& d, agh::CSubject::SEpisode& e,
			     agh::CSubject::SEpisode::SAnnotation& a)
		      : agh::CSubject::SEpisode::SAnnotation (a),
			csubject (j), session (d), sepisode (e)
			{}
	       ~SAnnotation() = default;
	};
	forward_list<SAnnotation>
		global_annotations;

	list<aghui::SScoringFacility*>
		open_scoring_facilities;

      // own variables aka saved settings
	float	operating_range_from,
		operating_range_upto;

	static const array<unsigned, 4>
		FFTPageSizeValues;
	static const array<double, 3>
		FFTBinSizeValues;
	unsigned short
		pagesize_item,
		binsize_item;
	size_t pagesize() const
		{
			return FFTPageSizeValues[pagesize_item];
		}
	double binsize() const
		{
			return FFTBinSizeValues[binsize_item];
		}

	sigfile::CHypnogram::TCustomScoreCodes
		ext_score_codes;

	static const char
		*const FreqBandNames[(size_t)sigfile::TBand::_total];
	float	freq_bands[(size_t)sigfile::TBand::_total][2];

	float	ppuv2; // let it be common for all
	size_t	timeline_height;
	size_t	timeline_pph;

	time_t	timeline_start,
		timeline_end;
	size_t T2P( time_t t) const
		{
			return difftime( t, timeline_start) / 3600. * timeline_pph;
		}
	time_t P2T( int p) const
		{
			return p * 3600. / timeline_pph + timeline_start;
		}
	size_t	tl_left_margin,
		tl_right_margin,
		timeline_width,
		timeline_pages;

	bool	runbatch_include_all_channels,
		runbatch_include_all_sessions,
		runbatch_iterate_ranges;

      // config
	string	_geometry_placeholder,
		_aghdd_placeholder,
		_aghtt_placeholder;
	forward_list<SValidator<string>>	config_keys_s;
	forward_list<SValidator<bool>>		config_keys_b;
	forward_list<SValidator<size_t>>	config_keys_z;
	forward_list<SValidator<float>>		config_keys_g;
	int load_settings();
	int save_settings();

	size_t	pagesize_item_saved,
		binsize_item_saved;
	sigfile::SFFTParamSet::TWinType
		FFTWindowType_saved,
		AfDampingWindowType_saved;
	//float	FFTFreqTrunc_saved;

      // status bar bits
	void sb_progress_indicator( const char*, size_t n, size_t i);
	void buf_on_status_bar( bool ensure = true);
	guint	sbContextIdGeneral;

      // tooltips
	static const char* const tooltips[2];
	enum class TTip { measurements = 0, simulations = 1, };

      // dnd
	struct SDndIface {
		GtkTargetEntry
			target_list[];
		size_t	n_targets;
	};
	SDndIface
		dnd;
	int dnd_maybe_admit_one( const char* fname);

	string	browse_command;

	SGeometry
		geometry;

      // subject details
	void update_subject_details_interactively( agh::CSubject&);

      // sister widget
	struct SExpDesignChooser {
		string	title;
		string	hist_filename;
		int	last_dir_no;
	};
	SExpDesignChooser
		chooser;
	string chooser_get_selected_dir(); // and assign last_dir_no
	void chooser_read_histfile();
	void chooser_write_histfile();
	string chooser_get_dir( int);
	string chooser_get_dir()
		{
			return chooser_get_dir( chooser.last_dir_no);
		}

	int construct_widgets();

      // colours
	typedef unsigned TColour_underlying_type;
	enum class TColour : TColour_underlying_type {
		power_mt,	ticks_mt,	bounds,
		labels_mt,	jinfo,

		score_none,	score_nrem1,	score_nrem2,
		score_nrem3,	score_nrem4,	score_rem,
		score_wake,	score_mvt,
		score_invalid,  // has no color chooser

		signal_unfazer,

		artifact,
		annotations,

		labels_sf,	ticks_sf,	power_sf,
		hypnogram,	hypnogram_scoreline,
		cursor,

		spectrum,	spectrum_axes,	spectrum_grid,

		emg,

		band_delta,	band_theta,	band_alpha,
		band_beta,	band_gamma,

		swa,		swa_sim,	process_s,
		paper_mr,
		labels_mr,
		ticks_mr,
	};
	map<TColour, SManagedColor>
		CwB;

	static TColour
	score2colour( sigfile::SPage::TScore s)
		{
			return (TColour)((unsigned)s + (unsigned)TColour::score_none);
		}
	static TColour
	band2colour( sigfile::TBand b)
		{
			return (TColour)((unsigned)b + (unsigned)TColour::band_delta);
		}

      // ---- constructibles
      // storage
	GtkListStore
		*mSessions,
		*mEEGChannels,
		*mAllChannels;
	GtkTreeStore
		*mGlobalAnnotations,
		*mSimulations;

	void populate_mSessions();
	void populate_mChannels();
	void __reconnect_channels_combo();
	void __reconnect_sessions_combo();
	gulong 	wMainWindow_delete_event_cb_handler_id,
		eMsmtPSDFreqFrom_value_changed_cb_handler_id,
		eMsmtPSDFreqWidth_value_changed_cb_handler_id,
		eMsmtSession_changed_cb_handler_id,
		eMsmtChannel_changed_cb_handler_id;
	void populate_mGlobalAnnotations();

	GtkListStore
		*mScoringPageSize,
		*mFFTParamsPageSize,
		*mFFTParamsBinSize,
		*mFFTParamsWindowType;
	static const auto
		msimulations_visibility_switch_col = 14,
		msimulations_modref_col = msimulations_visibility_switch_col + 1;
	static const char* const msimulations_column_names[];
	static const auto
		mannotations_visibility_switch_col = 4,
		mannotations_ref_col = mannotations_visibility_switch_col + 1;
	static const char* const mannotations_column_names[];

      // misc
	PangoFontDescription*
		monofont;

      // main toplevel
	GtkWindow
		*wMainWindow;
	void
	set_wMainWindow_interactive( bool indeed, bool flush = true);

	// tabs
	GtkNotebook
		*tTaskSelector,
		*tDesign, *tSimulations,
		*tSettings;
	GtkLabel
		*lTaskSelector1, *lTaskSelector2,
		*lSettings;
      // 1. Measurements
	GtkButton
		*bScanTree;
	GtkButton
		*bGlobalAnnotations;
	// annotations
	GtkTreeView
		*tvGlobalAnnotations;
	GtkDialog
		*wGlobalAnnotations;
	GtkLabel
		*lMsmtHint,
		*lMsmtInfo;
	GtkVBox
		*cMeasurements;
	GtkSpinButton
		*eMsmtPSDFreqFrom,
		*eMsmtPSDFreqWidth;
	GtkHBox
		*cMsmtFreqRange;
	GtkComboBox
		*eMsmtChannel,
		*eMsmtSession;

	GtkButton
		*bExpChange;
	GtkStatusbar
		*sbMainStatusBar;
	// menus
	GtkMenu
		*iiSubjectTimeline;
	GtkMenuItem
		*iSubjectTimelineScore,
		*iSubjectTimelineSubjectInfo,
		*iSubjectTimelineEDFInfo,
		*iSubjectTimelineSaveAsSVG,
		*iSubjectTimelineBrowse,
		*iSubjectTimelineResetMontage;

	// settings
	GtkSpinButton
		*eFFTParamsFreqTrunc;
	GtkComboBox
		*eFFTParamsWindowType,		*eFFTParamsPageSize,
		*eFFTParamsBinSize,		*eArtifWindowType;
	GtkEntry
		*eScoreCode[(size_t)sigfile::SPage::TScore::_total];
	GtkSpinButton
		*eSFNeighPagePeekPercent,	*eDAPageHeight,
		*eDAHypnogramHeight,		*eDASpectrumWidth,
		*eDAEMGHeight;
	GtkSpinButton
		*eBand[(size_t)sigfile::TBand::_total][2];
	GtkEntry
		*eBrowseCommand;

      // 2. Simulations
	GtkTreeView
		*tvSimulations;
	GtkToolButton
		*bSimulationsRun;
	GtkButton
		*bSimulationsSummary;
	GtkLabel
		*lSimulationsChannel,
		*lSimulationsSession;

	// settings
	GtkSpinButton
		*eCtlParamAnnlNTries,		*eCtlParamAnnlItersFixedT,
		*eCtlParamAnnlStepSize,		*eCtlParamAnnlBoltzmannk,
		*eCtlParamAnnlTInitialMantissa,	*eCtlParamAnnlTInitialExponent,
		*eCtlParamAnnlDampingMu,	*eCtlParamAnnlTMinMantissa,
		*eCtlParamAnnlTMinExponent,	*eCtlParamNSWAPpBeforeSimStart,
		*eCtlParamReqScoredPercent;
	GtkCheckButton
		*eCtlParamDBAmendment1,		*eCtlParamDBAmendment2,
		*eCtlParamAZAmendment1,		*eCtlParamAZAmendment2;
	GtkLabel
		*lCtlParamDBAmendment1,		*lCtlParamDBAmendment2,
		*lCtlParamAZAmendment1,		*lCtlParamAZAmendment2;

	GtkRadioButton
		*eCtlParamScoreMVTAsWake,	*eCtlParamScoreUnscoredAsWake;

	GtkSpinButton
		*eTunable[(size_t)agh::TTunable::_basic_tunables][4];
	GtkAdjustment
		*jTunable[(size_t)agh::TTunable::_basic_tunables][4];
	GtkButton
		*bSimParamRevertTunables;

      // other toplevels
	// edf header
	GtkDialog
		*wEDFFileDetails;
	GtkTextView
		*lEDFFileDetailsReport;
	GtkTextBuffer
		*tEDFFileDetailsReport;

	// scan log
	GtkDialog
		*wScanLog;
	GtkTextView
		*tScanLog;

	// about
	GtkTextView
		*tREADME;

	// edf dnd import
	GtkDialog
		*wEdfImport;
	GtkComboBox
		*eEdfImportGroup,
		*eEdfImportSession,
		*eEdfImportEpisode;
	GtkEntry
		*eEdfImportGroupEntry,
		*eEdfImportSessionEntry,
		*eEdfImportEpisodeEntry;
	GtkLabel
		*lEdfImportSubject,
		*lEdfImportCaption;
	GtkTextView
		*lEdfImportFileInfo;
	// GtkTextBuffer
	// 	*tEdfImportDetailsReport;
	GtkButton
		*bEdfImportAdmit,
		*bEdfImportScoreSeparately,
		*bEdfImportAttachCopy,
		*bEdfImportAttachMove;

	// subject details
	GtkDialog
		*wSubjectDetails;
	GtkEntry
		*eSubjectDetailsName,
		*eSubjectDetailsComment;
	GtkSpinButton
		*eSubjectDetailsAge;
	GtkRadioButton
		*eSubjectDetailsGenderMale,
		*eSubjectDetailsGenderFemale;

      // chooser
	GtkListStore
		*mExpDesignChooserList;
	GtkDialog
		*wExpDesignChooser;
	GtkTreeView
		*tvExpDesignChooserList;
	GtkButton
		*bExpDesignChooserSelect,
		*bExpDesignChooserCreateNew,
		*bExpDesignChooserRemove,
		*bExpDesignChooserQuit;
};


inline int
__attribute__ ((pure))
SExpDesignUI::AghTi()
const
{
	int i = 0;
	for ( auto Ti = AghTT.begin(); Ti != AghTT.end(); ++Ti, ++i )
		if ( Ti == _AghTi )
			return i;
	return -1;
}
inline int
__attribute__ ((pure))
SExpDesignUI::AghDi()
const
{
	int i = 0;
	for ( auto Di = AghDD.begin(); Di != AghDD.end(); ++Di, ++i )
		if ( Di == _AghDi )
			return i;
	return -1;
}




// forward declarations of callbacks
extern "C" {
gboolean wMainWindow_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
gboolean wMainWindow_configure_event_cb( GtkWidget*, GdkEvent*, gpointer);

void bExpChange_clicked_cb( GtkButton*, gpointer);

void bDownload_clicked_cb( GtkButton*, gpointer);

void bScanTree_clicked_cb( GtkButton*, gpointer);
void eMsmtSession_changed_cb( GtkComboBox*, gpointer);
void eMsmtChannel_changed_cb( GtkComboBox*, gpointer);
void eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
void eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton*, gpointer);

void bGlobalAnnotations_clicked_cb( GtkButton*, gpointer);
void tvGlobalAnnotations_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

void iiSubjectTimeline_show_cb( GtkWidget*, gpointer);
void iSubjectTimelineScore_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineSubjectInfo_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineEDFInfo_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineSaveAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineBrowse_activate_cb( GtkMenuItem*, gpointer);
void iSubjectTimelineResetMontage_activate_cb( GtkMenuItem*, gpointer);

gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSubjectTimeline_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSubjectTimeline_enter_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_leave_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

void bSimulationsRun_clicked_cb( GtkToolButton*, gpointer);
void bSimulationsSummary_clicked_cb( GtkButton*, gpointer);
void tvSimulations_row_activated_cb( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

gboolean check_gtk_entry_nonempty( GtkEditable*, gpointer);
gboolean cMeasurements_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
gboolean cMeasurements_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);

void tTaskSelector_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void tDesign_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void tSimulations_switch_page_cb( GtkNotebook*, gpointer, guint, gpointer);
void bSimParamRevertTunables_clicked_cb( GtkButton*, gpointer);

void bColourX_color_set_cb( GtkColorButton*, gpointer);

void wExpDesignChooser_show_cb( GtkWidget*, gpointer);
void wExpDesignChooser_hide_cb( GtkWidget*, gpointer);
void wExpDesignChooserChooser_hide_cb( GtkWidget*, gpointer);
void tvExpDesignChooserList_changed_cb( GtkTreeSelection*, gpointer);
void bExpDesignChooserSelect_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserQuit_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserCreateNew_clicked_cb( GtkButton*, gpointer);
void bExpDesignChooserRemove_clicked_cb( GtkButton*, gpointer);

void eCtlParamDBAmendment1_toggled_cb( GtkToggleButton*, gpointer);
void eCtlParamDBAmendment2_toggled_cb( GtkToggleButton*, gpointer);
void eCtlParamAZAmendment1_toggled_cb( GtkToggleButton*, gpointer);
void eCtlParamAZAmendment2_toggled_cb( GtkToggleButton*, gpointer);
} // extern "C"

} // namespace aghui

#endif

// eof
