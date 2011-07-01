// ;-*-C++-*- *  Time-stamp: "2011-07-01 02:46:53 hmmr"
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

#include <list>
#include <forward_list>
#include <map>
#include <stdexcept>

#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "../libagh/boost-config-validate.hh"
#include "ui.hh"
#include "managed-colour.hh"
#include "libagh/primaries.hh"


#ifndef _AGH_EXPDESIGN_UI_H
#define _AGH_EXPDESIGN_UI_H

using namespace std;

namespace aghui {

      // ui structures
	struct SExpDesignUI {
		agh::CExpDesign
			*ED;

	      // forward decl
		struct SGroupPresentation;

	      // contained classes
		struct SSubjectPresentation {
			agh::CSubject&  // can't have it declared const due to CMSessionSet operator[] not permitting
				csubject;
		      // this is a little overkill, but whatever
			agh::CSCourse
				*cscourse;
			time_t	tl_start;

			typedef list<agh::CSubject::SEpisode>::iterator TEpisodeIter;
			TEpisodeIter
				episode_focused;
			GtkWidget
				*da;
			bool	is_focused;

			bool get_episode_from_timeline_click( unsigned along);  // possibly sets episode_focused

			void draw_timeline( cairo_t *cr) const;
			void draw_timeline( const char *fname) const;

			SGroupPresentation& _p;
			SSubjectPresentation( agh::CSubject& _j, SGroupPresentation& parent);
		       ~SSubjectPresentation();
		};
		struct SGroupPresentation
		      : public forward_list<SSubjectPresentation> {

			agh::CExpDesign::TJGroups::iterator cjgroup;
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

			SGroupPresentation( SExpDesignUI& parent)
			      : // cjgroup (_g),
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

		SExpDesignUI( const string& dir);
	       ~SExpDesignUI();

		int populate( bool do_load);
		void depopulate( bool do_save);
		int populate_1();  // measurements
		int populate_2();  // simulations
		void cleanup_2();
		void do_rescan_tree();

		void show_empty_experiment_blurb();

		list<string>
			AghDD,	AghGG,	AghEE;
		list<agh::SChannel>
			AghHH,	AghTT;
		list<agh::SChannel>::iterator
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

		float	operating_range_from,
			operating_range_upto;

		static const array<unsigned, 4>
			FFTPageSizeValues;
		unsigned short
			pagesize_item;
		size_t pagesize() const
			{
				return FFTPageSizeValues[pagesize_item];
			}

		// agh::SFFTParamSet::TWinType
		// 	fft_window_type,
		// 	af_damping_window_type;

		agh::CHypnogram::TCustomScoreCodes
			ext_score_codes;

		static const char
			*const FreqBandNames[(size_t)agh::TBand::_total];
		float	freq_bands[(size_t)agh::TBand::_total][2];

		float	ppuv2; // let it be common for all
		size_t	timeline_height;
		size_t	timeline_pph;

		time_t	timeline_start,
			timeline_end;
		int T2P( time_t t) const
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
	    private:
		string	_geometry_placeholder,
			_aghdd_placeholder,
			_aghtt_placeholder;
		forward_list<SValidator<string>>	config_keys_s;
		forward_list<SValidator<size_t>>	config_keys_z;
		forward_list<SValidator<float>>		config_keys_g;
		forward_list<SValidator<bool>>		config_keys_b;
	    public:
		int load_settings();
		int save_settings();

		void progress_indicator( const char*, size_t n, size_t i);
		void buf_on_status_bar();
		guint	sbContextIdGeneral;

		struct SDndIface {
			static int construct_once();
			static void destruct();
			GtkTargetEntry
				target_list[];
			size_t	n_targets;
		};


		SGeometry
			geometry;

		int construct_widgets();
		static int construct_once();

	      // colours
		typedef unsigned TColour_underlying_type;
		enum class TColour : TColour_underlying_type {
			power_mt,	ticks_mt,	bounds,
			labels_mt,	jinfo,

			score_none,	score_nrem1,	score_nrem2,
			score_nrem3,	score_nrem4,	score_rem,
			score_wake,	score_mvt,

			signal_unfazer,

			artifact,

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
		static TColour score2colour( agh::SPage::TScore s)
			{
				return (TColour)((unsigned)s + (unsigned)TColour::score_none);
			}

	      // ---- constructibles
	      // storage
		GtkListStore
			*mSessions,
			*mEEGChannels,
			*mAllChannels,
			*mSimulations;

		void populate_mSessions();
		void populate_mChannels();
		void __reconnect_channels_combo();
		void __reconnect_sessions_combo();
		gulong 	eMsmtPSDFreqFrom_value_changed_cb_handler_id,
			eMsmtPSDFreqWidth_value_changed_cb_handler_id,
			eMsmtSession_changed_cb_handler_id,
			eMsmtChannel_changed_cb_handler_id;

		static GtkListStore // static
			*mScoringPageSize,
			*mFFTParamsPageSize,
			*mFFTParamsWindowType,
			*mAfDampingWindowType;
		static const auto
			tv_simulations_visibility_switch_col = 14,
			tv_simulations_modref_col = 15;

	      // main toplevel
		GtkWindow
			*wMainWindow;
	      // 1. Measurements
		GtkLabel
			*lMsmtHint,
			*lMsmtInfo;
		GtkVBox
			*cMeasurements;
		GtkSpinButton
			*eMsmtPSDFreqFrom,
			*eMsmtPSDFreqWidth;
		GtkComboBox
			*eMsmtChannel,
			*eMsmtSession;

		GtkButton
			*bExpChange;
		GtkStatusbar
			*sbMainStatusBar;

		// settings
		GtkSpinButton
			*eFFTParamsBinSize;
		GtkComboBox
			*eFFTParamsWindowType,		*eFFTParamsPageSize,
			*eArtifWindowType;
		GtkEntry
			*eScoreCode[(size_t)agh::SPage::TScore::_total];
		GtkSpinButton
			*eSFNeighPagePeekPercent,	*eDAPageHeight,
			*eDAHypnogramHeight,		*eDASpectrumWidth,
			*eDAEMGHeight;
		GtkSpinButton
			*eBand[(size_t)agh::TBand::_total][2];

	      // 2. Simulations
		GtkTreeView
			*tvSimulations;
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
			*eCtlParamAZAmendment;

		GtkRadioButton
			*eCtlParamScoreMVTAsWake,	*eCtlParamScoreUnscoredAsWake;

		GtkSpinButton
			*eTunable[(size_t)agh::TTunable::_basic_tunables][4];

	      // other toplevels
		GtkDialog
			*wEDFFileDetails;
		GtkTextView
			*lEDFFileDetailsReport;

		GtkDialog
			*wScanLog;
		GtkTextView
			*lScanLog,
			*tREADME;

	};


	inline int
	SExpDesignUI::AghTi() const
	{
		int i = 0;
		for ( auto Ti = AghTT.begin(); Ti != AghTT.end(); ++Ti, ++i )
			if ( Ti == _AghTi )
				return i;
		return -1;
	}
	inline int
	SExpDesignUI::AghDi() const
	{
		int i = 0;
		for ( auto Di = AghDD.begin(); Di != AghDD.end(); ++Di, ++i )
			if ( Di == _AghDi )
				return i;
		return -1;
	}




	struct SExpDesignChooser {
		SExpDesignChooser();
	       ~SExpDesignChooser();

		string	hist_filename;

		void	read_histfile();
		void	write_histfile();

	      // saved variables
		string	LastExpdesignDir;
		int	LastExpdesignDirNo;

		int construct_widgets();
		static int construct_once();
		GtkDialog
			*wExpDesignChooser;
		GtkTreeView
			*tvExpDesignList;
		GtkButton
			*bExpDesignSelect;
	};



      // forward declarations of callbacks
	extern "C" {
		void eMsmtSession_changed_cb( GtkComboBox*, gpointer);
		void eMsmtChannel_changed_cb( GtkComboBox*, gpointer);
		void eMsmtPSDFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
		void eMsmtPSDFreqWidth_value_changed_cb( GtkSpinButton*, gpointer);

		gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
		gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
		gboolean daSubjectTimeline_draw_cb( GtkWidget*, cairo_t*, gpointer);
//		gboolean daSubjectTimeline_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
		gboolean daSubjectTimeline_enter_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
		gboolean daSubjectTimeline_leave_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
		gboolean daSubjectTimeline_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

		gboolean check_gtk_entry_nonempty( GtkWidget*, GdkEventKey*, gpointer);
		gboolean cMeasurements_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
		gboolean cMeasurements_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
	}

} // namespace aghui

#endif

// eof
