// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility class
 *
 *         License:  GPL
 */

#ifndef _AGH_SCORING_FACILITY_H
#define _AGH_SCORING_FACILITY_H

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <gtk/gtk.h>

#include "../libexstrom/exstrom.hh"
#include "../libexstrom/signal.hh"
#include "ui.hh"
#include "draw-signal-generic.hh"
#include "expdesign.hh"
#include "../libagh/primaries.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;

namespace aghui {

// structures^H

struct SScoringFacility {
	struct SChannel {
		const char
			*name,
			*type;
		bool operator==( const char *_name) const
			{
				return 0 == strcmp( name, _name);
			}
		bool operator==( const SChannel& rv) const
			{
				return 0 == strcmp( name, rv.name);
			}

		agh::CRecording&
			crecording;

		SScoringFacility&
			_p;

	      // signal waveforms, cached here
		valarray<TFloat>
			signal_filtered,
			signal_original;
	      // filters
		struct SFilterInfo {
			float	cutoff;
			int	order;
		};
		SFilterInfo
			low_pass,
			high_pass;
		bool validate_filters();
		bool have_low_pass() const
			{
				return isfinite(low_pass.cutoff) && low_pass.cutoff > 0. && low_pass.order > 0;
			}
		bool have_high_pass() const
			{
				return isfinite(high_pass.cutoff) && high_pass.cutoff > 0. && high_pass.order > 0;
			}

		size_t n_samples() const
			{
				return signal_filtered.size();
			}
		size_t samplerate() const
			{
				return crecording.F().samplerate(name);
			}

	      // artifacts
		float calculate_dirty_percent();
		float	percent_dirty;

	      // annotations
		list<agh::CEDFFile::SSignal::SAnnotation*>
		in_annotations( double time) const;

	      // signal metrics
		struct SSFLowPassCourse {
			float	cutoff;
			unsigned
				order;
			valarray<TFloat>
				data;
			TFloat& operator[]( size_t i)
				{
					return data[i];
				}
			SSFLowPassCourse() = default;
		};
		SSFLowPassCourse
			signal_lowpass;
		void compute_lowpass( float _cutoff, unsigned _order);

		struct SSFEnvelope {
			unsigned
				tightness;
			valarray<TFloat>
				upper,
				lower;
			float breadth( size_t i) const
				{
					return upper[i] - lower[i];
				}
			valarray<TFloat> breadth() const
				{
					return upper - lower;
				}
			SSFEnvelope() = default;
		};
		SSFEnvelope
			signal_envelope;
		void compute_tightness( unsigned _tightness);

		struct SSFDzcdf {
			float	step,
				sigma;
			unsigned
				smooth;
			valarray<TFloat>
				data;
			TFloat& operator[]( size_t i)
				{
					return data[i];
				}
			SSFDzcdf() = default;
		};
		SSFDzcdf
			signal_dzcdf;
		void compute_dzcdf( float _step, float _sigma, unsigned _smooth);

	      // power courses
		valarray<TFloat>
			power; // can possibly live outside in core, no?
		float	from, upto;
		float	power_display_scale;

		array<valarray<TFloat>, (size_t)agh::TBand::_total>
			power_in_bands;
		agh::TBand
			focused_band,
			uppermost_band;

	      // spectrum
		valarray<TFloat>
			spectrum;  // per page, is volatile
		float	spectrum_upper_freq;
		unsigned
			n_bins,
			last_spectrum_bin;

	      // emg
		valarray<TFloat>
			emg_profile;
		float	emg_scale;

	      // region
		void mark_region_as_artifact( bool do_mark);
		void mark_region_as_annotation( const char*);
		void mark_region_as_pattern();

	      // convenience shortcuts
		void get_signal_original(); // also apply display filters
		void get_signal_filtered();
		void get_power();
		void get_spectrum( size_t p);
		void get_power_in_bands();

	      // ctor, dtor
		SChannel( agh::CRecording& r, SScoringFacility&, size_t y);
	       ~SChannel();

		int h() const
			{
				return _h;
			}
		agh::CEDFFile::SSignal& ssignal()
			{
				return _ssignal;
			}

		size_t	zeroy;
		bool operator<( const SChannel& rv) const
			{
				return zeroy < rv.zeroy;
			}

	      // comprehensive draw
		void draw_page( const char *fname, int width, int height); // to a file
		void draw_page( cairo_t*); // to montage

		float	signal_display_scale;

		bool	hidden,
			draw_zeroline,
			draw_original_signal,
			draw_filtered_signal,
			draw_power,
			draw_emg,
			draw_bands,
			draw_spectrum_absolute,
			use_resample;
		void
		update_channel_check_menu_items();

	      // selection and marquee
		double	marquee_mstart,
			marquee_mend,        // in terms of event->x
			marquee_start,
			marquee_end;         // set on button_release
		double	selection_start_time,
			selection_end_time;  // in seconds
		size_t	selection_start,
			selection_end;       // in samples
		size_t marquee_to_selection();
		size_t selection_size() const
			{
				return selection_end - selection_start;
			}
		void put_selection( size_t a, size_t e)
			{
				selection_start = a, selection_end = e;
				selection_start_time = (double)a / samplerate();
				selection_end_time = (double)e / samplerate();
			}
		void put_selection( double a, double e)
			{
				selection_start_time = a, selection_end_time = e;
				selection_start = a * samplerate();
				selection_end = e * samplerate();
			}

		float spp() const
			{
				return (float)samplerate() * _p.vpagesize() / _p.da_wd;
			}
		float fine_line() const
			{
				return ((not use_resample) and spp() > 1.) ? .6 / (spp() + .2) : .6;
			}
		int sample_at_click( double x) const
			{
				return _p.time_at_click( x) * samplerate();
			}

		GtkMenuItem
			*menu_item_when_hidden;

	    private:
		int	_h;
		agh::CEDFFile::SSignal&
			_ssignal;

	      // strictly draw the signal waveform bare
	      // (also used as such in some child dialogs)
		void draw_signal_original( unsigned width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_original, width, vdisp, cr);
			}
		void draw_signal_filtered( unsigned width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_filtered, width, vdisp, cr);
			}
	      // generic draw_signal wrapper
		void draw_signal( const valarray<TFloat>& signal,
				  unsigned width, int vdisp, cairo_t *cr) const;

	      // draw more details, all except volatile parts such as crosshair and unfazer
		void draw_page_static( cairo_t*, int wd, int zeroy, // writers to an svg file override zeroy (with 0)
				       bool draw_marquee) const;

		static float calibrate_display_scale( const valarray<TFloat>&, size_t over, float fit);
	};
	list<SChannel>
		channels;
	SChannel& operator[]( const char *ch)
		{
			auto iter = find( channels.begin(), channels.end(), ch);
			if ( iter == channels.end() )
				throw invalid_argument( string ("SScoringFacility::operator[]: bad channel: ") + ch);
			return *iter;
		}

	time_t start_time() const
		{
			return channels.front().crecording.F().start_time;
		}

	vector<char>
		hypnogram;
	size_t total_pages() const
		{
//			return channels.front().crecording.F().n_pages();
			return hypnogram.size();
		}
	size_t total_vpages() const
		{
			return p2ap( total_pages());
		}
	void get_hypnogram();
	void put_hypnogram();

	float	scored_percent,
		scored_percent_nrem,
		scored_percent_rem,
		scored_percent_wake;

	void calculate_scored_percent();

      // ctor, dtor
	SScoringFacility( agh::CSubject&, const string& d, const string& e,
			  SExpDesignUI& parent);
       ~SScoringFacility();

	SExpDesignUI&
		_p;
    private:
	agh::CSubject&
		_csubject;
	string	_session;
	agh::CSubject::SEpisode&
		_sepisode;
    public:
	agh::CSubject& csubject() const
		{
			return _csubject;
		}
	agh::CSubject::SEpisode& sepisode() const
		{
			return _sepisode;
		}
	const string& session() const
		{
			return _session;
		}

	float	sane_signal_display_scale,
		sane_power_display_scale; // 2.5e-5;

	bool	suppress_redraw:1,
		marking_now:1,
		shuffling_channels_now:1;
	bool	draw_crosshair,
		draw_power, // overridden already in individual channels' flag
		draw_spp;

	float	skirting_run_per1;

	size_t	crosshair_at;

      // page and vpage index
	size_t p2ap( size_t p) const // page to visible_page
		{
			return (size_t)((p) * (float)pagesize() / vpagesize());
		}

	size_t ap2p( size_t p) const
		{
			return (size_t)((p) * (float)vpagesize() / pagesize());
		}

	size_t cur_page() const		{ return _cur_page;  }
	size_t cur_vpage() const	{ return _cur_vpage; }
	size_t set_cur_page( size_t p);
	size_t set_cur_vpage( size_t p);

	size_t cur_page_start() const // in seconds
		{
			return _cur_page * pagesize();
		}
	size_t cur_page_end() const // in seconds
		{
			return (_cur_page + 1) * pagesize();
		}

	agh::SPage::TScore cur_page_score() const
		{
			return agh::SPage::char2score( hypnogram[_cur_page]);
		}
	bool page_has_artifacts( size_t);

	size_t pagesize() const
		{
			return _p.pagesize();
		}
	static const array<unsigned, 9>
		DisplayPageSizeValues;
	static size_t figure_display_pagesize_item( size_t seconds);

	size_t vpagesize() const
		{
			return DisplayPageSizeValues[pagesize_item];
		}
	bool pagesize_is_right() const
		{
			return pagesize() == vpagesize();
		}

	size_t cur_vpage_start() const // in seconds
		{
			return _cur_vpage * vpagesize();
		}
	size_t cur_vpage_end() const // in seconds
		{
			return (_cur_vpage + 1) * vpagesize();
		}

	float xvpagesize() const
		{
			return (1. + 2*skirting_run_per1) * vpagesize();
		}
	double cur_xvpage_start() const
		{
			return cur_vpage_start() - skirting_run_per1 * vpagesize();
		}
	double cur_xvpage_end() const
		{
			return cur_vpage_end() + skirting_run_per1 * vpagesize();
		}
	double time_at_click( double x) const
		{
			return cur_xvpage_start() + x/da_wd * xvpagesize();
		}


	void set_pagesize( int item); // touches a few wisgets

      // menu support
	SChannel
		*using_channel;
	list<agh::CEDFFile::SSignal::SAnnotation*>
		over_annotations;
	agh::CEDFFile::SSignal::SAnnotation*
	interactively_choose_annotation() const;

      // channel slots
	template <class T>
	int channel_y0( const T& h) const
		{
			auto H = find( channels.begin(), channels.end(), h);
			return ( H != channels.end() ) ? H->zeroy : -1;
		}
	SChannel* channel_near( int y);
	int	interchannel_gap;
	size_t montage_est_height() const
		{
			return channels.size() * interchannel_gap;
		}
	int find_free_space();
	void space_evenly();
	void expand_by_factor( double);
	int	n_hidden;
      // shuffling manually
	double	event_y_when_shuffling;
	int	zeroy_before_shuffling;

      // misc supporting functions
	void draw_montage( cairo_t*);
	void draw_hypnogram( cairo_t*);
	void repaint_score_stats() const;
	void queue_redraw_all() const;

	void do_score_forward( char score_ch);
	void do_score_back( char score_ch);

      // unfazer
	enum class TUnfazerMode {
		none,
		channel_select,
		calibrate,
	};
	TUnfazerMode
		unfazer_mode;
	SChannel
		*unfazer_affected_channel,
		*unfazer_offending_channel;
	float
		unfazer_factor;  // as currently being tried

      // tips
	enum class TTipIdx {
		general,
		unfazer
	};
	void set_tooltip( TTipIdx i) const
		{
			gtk_widget_set_tooltip_markup( (GtkWidget*)lScoringFacHint, tooltips[(int)i]);
		}

      // child dialogs:
      // pattern find dialog
	struct SFindDialog {
	      // own copies of parent's same
		unsigned
			bwf_order;
		float	bwf_cutoff;
		bool	bwf_scale;
		float 	dzcdf_step,
			dzcdf_sigma;
		unsigned
			dzcdf_smooth,
			env_tightness;
		float	tolerance_a,
			tolerance_b,
			tolerance_c;

	      // loadable
		valarray<TFloat>
			pattern;
		size_t	samplerate;
		size_t	context_before,
			context_after;
		static const size_t
			context_pad = 100;
		size_t pattern_size_essential() const
			{
				return pattern.size() - context_before - context_after;
			}
		double pattern_length() const
			{
				return (double)pattern.size() / samplerate;
			}
		double pattern_length_essential() const
			{
				return (double)pattern_size_essential() / samplerate;
			}

	      // finding tool
		sigproc::CPattern<TFloat>
			*cpattern;
		size_t	last_find;
		int	increment;

		SScoringFacility::SChannel
			*field_channel;

		bool search( ssize_t from);
		float	match_a,
			match_b,
			match_c;

	      // ctor, dtor
		SFindDialog( SScoringFacility& parent);
	       ~SFindDialog();

		bool	draw_details:1;
		void draw( cairo_t*);

		void enumerate_patterns_to_combo();
		void preselect_entry( const char*, bool globally);
		void preselect_channel( const char*);
		void enable_controls( bool);
		void acquire_parameters();
		void update_displayed_parameters();

		float	display_scale;

		static const char
			*globally_marker;
		void load_pattern( SScoringFacility::SChannel&); // load selection on this channel
		void load_pattern( const char* name, bool globally); // load named
		void save_pattern( const char* name, bool globally);
		void discard_pattern( const char *label, bool globally);

		SScoringFacility&
			_p;
	      // widgets
		static const int
			da_ht = 280;
		int	da_wd;
		void set_pattern_da_width( int);

		int construct_widgets();
		GtkListStore
			*mPatterns;
		GtkDialog
			*wPattern;
		GtkComboBox
			*ePatternChannel,
			*ePatternList;
		GtkScrolledWindow
			*vpPatternSelection;
		GtkDrawingArea
			*daPatternSelection;
		GtkButton
			*bPatternFindNext, *bPatternFindPrevious,
			*bPatternSave, *bPatternDiscard;
		GtkSpinButton
			*ePatternEnvTightness, *ePatternFilterCutoff,
			*ePatternFilterOrder, *ePatternDZCDFStep,
			*ePatternDZCDFSigma, *ePatternDZCDFSmooth,
			*ePatternParameterA, *ePatternParameterB,
			*ePatternParameterC;
		GtkHBox
			*cPatternLabelBox;
		GtkLabel
			*lPatternSimilarity;
		GtkDialog
			*wPatternName;
		GtkEntry
			*ePatternNameName;
		GtkCheckButton
			*ePatternNameSaveGlobally;
		gulong	ePatternChannel_changed_cb_handler_id,
			ePatternList_changed_cb_handler_id;


	};
	SFindDialog
		find_dialog;

	struct SFiltersDialog {
		SFiltersDialog( SScoringFacility& parent);
	       ~SFiltersDialog()
			{
				gtk_widget_destroy( (GtkWidget*)wFilters);
			}
	    private:
		SScoringFacility&
			_p;
	    public:
		int construct_widgets();
		GtkDialog
			*wFilters;
		GtkLabel
			*lFilterCaption;
		GtkSpinButton
			*eFilterLowPassCutoff, *eFilterHighPassCutoff,
			*eFilterLowPassOrder, *eFilterHighPassOrder;
		GtkButton
			*bFilterOK;
	};
	SFiltersDialog
		filters_dialog;

	struct SPhasediffDialog {
		const SChannel
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
		void update_course();

		const SChannel* channel_from_cbox( GtkComboBox *cbox);
		void preselect_channel( GtkComboBox *cbox, const char *ch);

		SPhasediffDialog( SScoringFacility&);
	       ~SPhasediffDialog()
			{
				gtk_widget_destroy( (GtkWidget*)wPhaseDiff);
			}
		SScoringFacility&
			_p;

		int construct_widgets();
		GtkDialog
			*wPhaseDiff;
		GtkComboBox
			*ePhaseDiffChannelA, *ePhaseDiffChannelB;
		GtkDrawingArea
			*daPhaseDiff;
		GtkSpinButton
			*ePhaseDiffFreqFrom,
			*ePhaseDiffFreqUpto;
		GtkButton
			*bPhaseDiffApply;
		gulong
			ePhaseDiffChannelA_changed_cb_handler_id,
			ePhaseDiffChannelB_changed_cb_handler_id;
	};
	SPhasediffDialog
		phasediff_dialog;

    private:
	size_t	_cur_page,  // need them both
		_cur_vpage; // apparent

	size_t	n_eeg_channels;

	int	pagesize_item;

	static const char* const tooltips[2];

      // own widgets
	// we load and construct own widget set (wScoringFacility and all its contents)
	// ourself, for every SScoringFacility instance being created, so
	// construct_widgets below takes an arg
	GtkBuilder *builder;
	int construct_widgets();
    public:
	aghui::SGeometry
		geometry;

	static size_t
		IntersignalSpace,
		SpectrumWidth,
		HypnogramHeight,
		EMGProfileHeight;
	static float
		NeighPagePeek;

	// storage
	GtkListStore
		*mAnnotationsAtCursor;

	GtkWindow
		*wScoringFacility;
	GtkComboBox
		*eScoringFacPageSize;
	GtkSpinButton
		*eScoringFacCurrentPage;
	GtkMenu
		*mSFPage,
		*mSFPageSelection,
		*mSFPageAnnotation,
		*mSFPageHidden,
		*mSFPower,
		*mSFScore;
	//		*mSFSpectrum;
	GtkCheckMenuItem
		*iSFPageShowOriginal, *iSFPageShowProcessed,
		*iSFPageUseResample, *iSFPageDrawZeroline,
		*iSFPageDrawPSDProfile,
		*iSFPageDrawEMGProfile;
	GtkMenuItem
		*iSFPageUnfazer, *iSFPageFilter, *iSFPageSaveAs,
		*iSFPageExportSignal, *iSFPageUseThisScale,
		*iSFPageClearArtifacts, *iSFPageHide,
		*iSFPageHidden,  // has a submenu
		*iSFPageSpaceEvenly,
		*iSFPageAnnotationSeparator,
		*iSFPageAnnotationDelete,
		*iSFPageAnnotationEdit,

		*iSFPageSelectionMarkArtifact, *iSFPageSelectionClearArtifact,
		*iSFPageSelectionFindPattern,
		*iSFPageSelectionAnnotate,

		*iSFPowerExportAll, *iSFPowerExportRange, *iSFPowerUseThisScale,

		*iSFScoreAssist, *iSFScoreImport, *iSFScoreExport, *iSFScoreClear,

		*iSFAcceptAndTakeNext;
	GtkExpander
		*cScoringFacHypnogram;
	GtkHBox
		*cScoringFacControlBar;
	GtkToggleButton
		*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
		*bScoreGotoPrevArtifact, *bScoreGotoNextArtifact,
		*bScoringFacDrawPower, *bScoringFacDrawCrosshair,
		*bScoringFacShowFindDialog, *bScoringFacShowPhaseDiffDialog;
	GtkStatusbar
		*sbSF;
	GtkDrawingArea
		*daScoringFacMontage,
		*daScoringFacHypnogram;
	GtkButton
		*bScoringFacBack,
		*bScoringFacForward;
	GtkAdjustment
		*jPageNo;
	GtkToolButton  // there's no reason for these to be different from those two above; just they happen to be toolbuttons in glade
		*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
		*bScoreREM,   *bScoreWake,
		*bSFAccept;
	GtkLabel
		*lScoringFacTotalPages, *lScoringFacClockTime,
		*lScoringFacPercentScored, *lScoringFacCurrentPos,
		*lScoreStatsNREMPercent, *lScoreStatsREMPercent, *lScoreStatsWakePercent,
		*lScoringFacHint;
	GtkTable
		*cScoringFacSleepStageStats;

	GtkDialog
		*wAnnotationLabel,
		*wAnnotationSelector;
	GtkEntry
		*eAnnotationLabel;
	GtkComboBox
		*eAnnotationSelectorWhich;

    public:
	// here's hoping configure-event comes before expose-event
	gint	da_wd,
		da_ht;  // not subject to window resize, this
};





// forward declarations of callbacks
extern "C" {

	gboolean daScoringFacMontage_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

	gboolean daScoringFacMontage_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacMontage_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacMontage_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacMontage_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
	gboolean daScoringFacMontage_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	void eScoringFacPageSize_changed_cb( GtkComboBox*, gpointer);
	void eScoringFacCurrentPage_value_changed_cb( GtkSpinButton*, gpointer);

	void bScoreClear_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM1_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM2_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM3_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM4_clicked_cb( GtkButton*, gpointer);
	void bScoreREM_clicked_cb  ( GtkButton*, gpointer);
	void bScoreWake_clicked_cb ( GtkButton*, gpointer);

	void bScoringFacForward_clicked_cb( GtkButton*, gpointer);
	void bScoringFacBack_clicked_cb( GtkButton*, gpointer);
	void bScoreGotoPrevUnscored_clicked_cb( GtkToggleButton*, gpointer);
	void bScoreGotoNextUnscored_clicked_cb( GtkToggleButton*, gpointer);
	void bScoreGotoPrevArtifact_clicked_cb( GtkToggleButton*, gpointer);
	void bScoreGotoNextArtifact_clicked_cb( GtkToggleButton*, gpointer);
	void bScoringFacDrawPower_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacShowFindDialog_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton*, gpointer);

	void bSFAccept_clicked_cb( GtkButton*, gpointer);

	void iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageUseResample_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageDrawZeroline_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageFilter_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageUnfazer_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSaveAs_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageExportSignal_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageUseThisScale_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageHide_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageHidden_select_cb( GtkMenuItem*, gpointer);
	void iSFPageHidden_deselect_cb( GtkMenuItem*, gpointer);
	void iSFPageShowHidden_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSpaceEvenly_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageDrawPSDProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageDrawEMGProfile_toggled_cb( GtkCheckMenuItem*, gpointer);

	void iSFPageAnnotationDelete_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageAnnotationEdit_activate_cb( GtkMenuItem*, gpointer);

	void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSelectionFindPattern_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSelectionAnnotate_activate_cb( GtkMenuItem*, gpointer);

	void iSFPowerExportRange_activate_cb( GtkMenuItem*, gpointer);
	void iSFPowerExportAll_activate_cb( GtkMenuItem*, gpointer);
	void iSFPowerUseThisScale_activate_cb( GtkMenuItem*, gpointer);

	gboolean daScoringFacHypnogram_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacHypnogram_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);

	void iSFScoreAssist_activate_cb( GtkMenuItem*, gpointer);
	void iSFScoreImport_activate_cb( GtkMenuItem*, gpointer);
	void iSFScoreExport_activate_cb( GtkMenuItem*, gpointer);
	void iSFScoreClear_activate_cb( GtkMenuItem*, gpointer);

	void ePatternList_changed_cb( GtkComboBox*, gpointer);
	void ePatternChannel_changed_cb( GtkComboBox*, gpointer);
	gboolean daPatternSelection_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daPatternSelection_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
	void bPatternFind_clicked_cb( GtkButton*, gpointer);
	void bPatternSave_clicked_cb( GtkButton*, gpointer);
	void bPatternDiscard_clicked_cb( GtkButton*, gpointer);
	void ePattern_any_value_changed_cb( GtkSpinButton*, gpointer);
	void wPattern_show_cb( GtkWidget*, gpointer);
	void wPattern_hide_cb( GtkWidget*, gpointer);

	void eFilterHighPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);
	void eFilterLowPassCutoff_value_changed_cb( GtkSpinButton*, gpointer);

	void ePhaseDiffChannelA_changed_cb( GtkComboBox*, gpointer);
	void ePhaseDiffChannelB_changed_cb( GtkComboBox*, gpointer);
	gboolean daPhaseDiff_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daPhaseDiff_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
	void ePhaseDiffChannelA_changed_cb( GtkComboBox*, gpointer);
	void ePhaseDiffChannelB_changed_cb( GtkComboBox*, gpointer);
	void ePhaseDiffFreqFrom_value_changed_cb( GtkSpinButton*, gpointer);
	void ePhaseDiffFreqUpto_value_changed_cb( GtkSpinButton*, gpointer);
	void bPhaseDiffApply_clicked_cb( GtkButton*, gpointer);
	void wPhaseDiff_show_cb( GtkWidget*, gpointer);
	void wPhaseDiff_hide_cb( GtkWidget*, gpointer);

	gboolean wScoringFacility_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
}

} // namespace aghui

#endif

// eof
