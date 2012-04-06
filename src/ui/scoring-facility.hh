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

#ifndef _AGH_UI_SCORING_FACILITY_H
#define _AGH_UI_SCORING_FACILITY_H

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <gtk/gtk.h>

#include "../libsigproc/exstrom.hh"
#include "../libsigproc/sigproc.hh"
#include "../libsigfile/page-metrics-base.hh"
#include "../core/primaries.hh"
#include "../libica/ica.hh"
#include "misc.hh"
#include "draw-signal-generic.hh"
#include "expdesign.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace aghui {

// structures^H

struct SScoringFacility {
      // ctor, dtor
	SScoringFacility( agh::CSubject&, const string& d, const string& e,
			  SExpDesignUI& parent);
       ~SScoringFacility();

      // link to parent
	SExpDesignUI&
		_p;
      // to whom we belong
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

      // channels
	struct SChannel {
		const char
			*name;
		sigfile::SChannel::TType
			type;
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
		sigfile::SFilterPack&
			filters;
		list<sigfile::SAnnotation>&
			annotations;
		sigfile::SArtifacts&
			artifacts;

		SScoringFacility&
			_p;

	      // signal waveforms, cached here
		valarray<TFloat>
			signal_original,
			signal_filtered,
			signal_reconstituted;  // while it's hot
	      // filters
		//bool validate_filters();
		bool have_low_pass() const;
		bool have_high_pass() const;
		bool have_notch_filter() const;
		size_t n_samples() const;
		size_t samplerate() const;

	      // artifacts
		float calculate_dirty_percent();
		float	percent_dirty;

	      // annotations
		list<sigfile::SAnnotation*>
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

	      // profiles
		// PSD
		valarray<TFloat>
			power; // can possibly live outside in core, no?
		float	from, upto;
		float	power_display_scale;
		array<valarray<TFloat>, (size_t)sigfile::TBand::_total>
			power_in_bands;
		unsigned short
			focused_band,
			uppermost_band;
		// uCont
		valarray<TFloat>
			ucont;

	      // spectrum
		valarray<TFloat>
			spectrum;  // per page, is volatile
		float	spectrum_upper_freq;
		unsigned
			spectrum_bins,
			last_spectrum_bin;
		void get_power( bool force);
		void get_power_in_bands( bool force);
		void get_spectrum( size_t p);

	      // emg
		valarray<TFloat>
			emg_profile;
		float	emg_scale;

	      // region
		void mark_region_as_artifact( bool do_mark);
		void mark_region_as_annotation( const char*);
		void mark_region_as_pattern();

	      // convenience shortcuts
		void get_signal_original()
			{
				signal_original =
					crecording.F().get_signal_original( name);
				if ( zeromean_original )
					signal_original -=
						signal_original.sum() / signal_original.size();
			}
		void get_signal_filtered()
			{
				signal_filtered =
					crecording.F().get_signal_filtered( name);
				// filtered is already zeromean as shipped

			}

	      // ctor, dtor
		SChannel( agh::CRecording& r, SScoringFacility&, size_t y);

		int h() const
			{
				return _h;
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

		// saved flags
		bool	hidden,
			draw_zeroline,
			draw_original_signal,
			draw_filtered_signal,
			zeromean_original,
			zeromean_filtered,
			draw_power,
			draw_emg,
			draw_bands,
			draw_spectrum_absolute,
			resample_signal,
			resample_power;
		sigfile::TProfileType
			display_profile_type;
		bool	discard_marked,
			apply_reconstituted;
		void update_channel_check_menu_items();
		void update_power_check_menu_items();

	      // selection and marquee
		float	marquee_mstart,
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
				return ((not resample_signal) and spp() > 1.) ? .6 / (spp() + .2) : .6;
			}
		int sample_at_click( double x) const
			{
				return _p.time_at_click( x) * samplerate();
			}

		GtkMenuItem
			*menu_item_when_hidden;

	    protected:
		int	_h;

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
		friend class SScoringFacility;
		void draw_signal_reconstituted( unsigned width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_reconstituted, width, vdisp, cr);
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
	size_t	n_eeg_channels;
	SChannel& operator[]( const char *ch)
		{
			auto iter = find( channels.begin(), channels.end(), ch);
			if ( iter == channels.end() )
				throw invalid_argument( string ("SScoringFacility::operator[]: bad channel: ") + ch);
			return *iter;
		}
	SChannel& channel_by_idx( size_t i)
		{
			for ( auto &H : channels )
				if ( i-- == 0 )
					return H;
			throw invalid_argument( string ("SScoringFacility::operator[]: bad channel idx: ") + to_string(i));
		}

      // ICA support
	ica::CFastICA
		*ica;
	itpp::mat  // looks like it has to be double
		ica_components;
	// map<size_t, itpp::vec>
	// 	ica_components2;
	size_t n_ics() const
		{
			return ica->obj() . get_nrof_independent_components();
		}
	enum TICMapFlags : int { apply_normally = 0, dont_apply = 1 };
	struct SICMapOptions { int m; };
	vector<SICMapOptions>
		ica_map;
	typedef function<valarray<TFloat>()> TICASetupFun;
	int setup_ica();
	int run_ica();
	enum class TICARemixMode { map, punch, zero };
	TICARemixMode remix_mode;
	static const char
		*ica_unmapped_menu_item_label;
	int remix_ics();
	int restore_ics();
	int ic_near( double y) const;
	int ic_of( const SChannel*) const;
	int using_ic;
	int apply_remix( bool do_backup);

      // timeline
	time_t start_time() const
		{
			return channels.front().crecording.F().start_time();
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

      // state and flags
	// volatile
	bool	suppress_redraw:1,
		suppress_set_vpage_from_cb:1;
	enum TMode {
		scoring,
		marking, shuffling_channels,
		separating,
		showing_ics,
		showing_remixed
	};
	TMode	mode;
	size_t	crosshair_at;
	double	crosshair_at_time;
	// persistent
	bool	draw_crosshair,
		draw_spp,
		alt_hypnogram;

      // page and vpage index
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

	sigfile::SPage::TScore
	cur_page_score() const
		{
			return sigfile::SPage::char2score( hypnogram[_cur_page]);
		}
	bool page_has_artifacts( size_t);

      // pagesize
	size_t pagesize() const
		{
			return _p.pagesize();
		}
	static const array<unsigned, 9>
		DisplayPageSizeValues;
	static size_t
	figure_display_pagesize_item( size_t seconds);
    private:
	int	pagesize_item;

    public:
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
	size_t
	p2ap( size_t p) const // page to visible_page
		{
			return (size_t)((p) * (float)pagesize() / vpagesize());
		}

	size_t
	ap2p( size_t p) const
		{
			return (size_t)((p) * (float)vpagesize() / pagesize());
		}
    private:
	size_t	_cur_page,
		_cur_vpage;

    public:
      // page location adjusted for pre- and post margins
	float	skirting_run_per1;
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

    public:
      // channel slots
	template <class T>
	int channel_y0( const T& h) const
		{
			auto H = find( channels.begin(), channels.end(), h);
			return ( H != channels.end() ) ? H->zeroy : -1;
		}
	SChannel*
	channel_near( int y);

	int	interchannel_gap;
	size_t
	__attribute__ ((pure))
	montage_est_height() const
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

      // misc supporting members
	float	sane_signal_display_scale,
		sane_power_display_scale; // 2.5e-5;

	void draw_montage( cairo_t*);
    private:
	template <class T>
	void _draw_matrix_to_montage( cairo_t*, const itpp::Mat<T>&);
    public:
	void draw_hypnogram( cairo_t*);
	void repaint_score_stats() const;
	void queue_redraw_all() const;

	void do_score_forward( char score_ch);
	void do_score_back( char score_ch);

      // tips
	enum TTipIdx : size_t {
		scoring_mode,
		ica_mode
	};
	void set_tooltip( TTipIdx i) const
		{
			gtk_widget_set_tooltip_markup( (GtkWidget*)lSFHint, tooltips[i]);
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
	       ~SFiltersDialog()
			{
				gtk_widget_destroy( (GtkWidget*)wFilters);
			}
		SFiltersDialog( SScoringFacility& parent)
		      : _p (parent)
			{}
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
		GtkComboBox
			*eFilterNotchFilter;
		GtkListStore
			*mFilterNotchFilter;
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

      // menu support
	SChannel
		*using_channel;
	list<sigfile::SAnnotation*>
		over_annotations;
	sigfile::SAnnotation*
	interactively_choose_annotation() const;

    private:
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
		HypnogramHeight,
		EMGProfileHeight;

	// storage
	GtkListStore
		*mAnnotationsAtCursor;

	// window
	GtkWindow
		*wScoringFacility;
	// control bar
	GtkLabel
		*lSFHint;
	GtkHBox
		*cSFControlBar;
	GtkComboBox
		*eSFPageSize;
	GtkSpinButton
		*eSFCurrentPage;
	GtkAdjustment
		*jPageNo;
	GtkLabel
		*lSFTotalPages;
	GtkBox
		*cSFScoringModeContainer,
		*cSFICAModeContainer;
	// 1. scoring mode
	GtkLabel
		*lSFClockTime,
		*lSFCurrentPos;
	GtkButton
		*bSFBack, *bSFForward,
		*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
		*bScoreREM, *bScoreWake,
		*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
		*bScoreGotoPrevArtifact, *bScoreGotoNextArtifact;
	GtkToggleButton
		*bSFDrawCrosshair,
		*bSFShowFindDialog, *bSFShowPhaseDiffDialog;
	GtkButton
	//*bSFResetMontage,
		*bSFRunICA;
	GtkTable
		*cSFSleepStageStats;
	GtkLabel
		*lScoreStatsNREMPercent, *lScoreStatsREMPercent, *lScoreStatsWakePercent,
		*lSFPercentScored;
	GtkStatusbar
		*sbSF;

	// 2. ICA mode
	GtkComboBox
		*eSFICARemixMode,
		*eSFICANonlinearity,
		*eSFICAApproach;
	GtkListStore
		*mSFICARemixMode,
		*mSFICANonlinearity,
		*mSFICAApproach;
	GtkCheckButton
		*eSFICAFineTune,
		*eSFICAStabilizationMode;
	GtkSpinButton
		*eSFICAa1,
		*eSFICAa2,
		*eSFICAmu,
		*eSFICAepsilon,
		*eSFICANofICs,
		*eSFICAEigVecFirst,
		*eSFICAEigVecLast,
		*eSFICASampleSizePercent,
		*eSFICAMaxIterations;
	GtkAdjustment
		*jSFICANofICs,
		*jSFICAEigVecFirst,
		*jSFICAEigVecLast;
	GtkButton
		*bSFICATry,
		*bSFICAApply,
		*bSFICACancel;
	GtkToggleButton
		*bSFICAPreview,
		*bSFICAShowMatrix;
	GtkTextView
		*tSFICAMatrix;
	GtkDialog
		*wSFICAMatrix;

	// common controls (contd)
	GtkMenuToolButton
		*bSFAccept;
	GtkMenu
		*mSFAccept;

	// montage area
	GtkDrawingArea
		*daSFMontage,
		*daSFHypnogram;
	GtkExpander
		*cSFHypnogram;
	GtkLabel
		*lSFOverChannel;
	// menus
	GtkMenu
		*mSFPage,
		*mSFPageSelection,
		*mSFPageAnnotation,
		*mSFPageHidden,
		*mSFPower,
		*mSFScore,
		*mSFICAPage;
	GtkCheckMenuItem
		*iSFPageShowOriginal, *iSFPageShowProcessed,
		*iSFPageUseResample, *iSFPageDrawZeroline,
		*iSFPageDrawPSDProfile,
		*iSFPageDrawEMGProfile,
		*iSFPowerDrawBands,
		*iSFPowerSmooth;
	GtkMenuItem
		*iSFPageFilter, *iSFPageSaveAs,
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

		*iSFPowerExportAll, *iSFPowerExportRange,
		*iSFPowerUseThisScale,

		*iSFScoreAssist, *iSFScoreImport, *iSFScoreExport, *iSFScoreClear,

		*iSFAcceptAndTakeNext;

	// less important dialogs
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


inline bool
SScoringFacility::SChannel::have_low_pass() const
{
	return isfinite(filters.low_pass_cutoff)
		&& filters.low_pass_cutoff > 0.
		&& filters.low_pass_order > 0;
}

inline bool
SScoringFacility::SChannel::have_high_pass() const
{
	return isfinite(filters.high_pass_cutoff)
		&& filters.high_pass_cutoff > 0.
		&& filters.high_pass_order > 0;
}
inline bool
SScoringFacility::SChannel::have_notch_filter() const
{
	return filters.notch_filter != sigfile::SFilterPack::TNotchFilter::none;
}

inline size_t
SScoringFacility::SChannel::n_samples() const
{
	return signal_filtered.size();
}

inline size_t
SScoringFacility::SChannel::samplerate() const
{
	return crecording.F().samplerate(_h);
}



// forward declarations of callbacks
extern "C" {

gboolean daSFMontage_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

gboolean daSFMontage_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFMontage_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_leave_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

void eSFPageSize_changed_cb( GtkComboBox*, gpointer);
void eSFCurrentPage_value_changed_cb( GtkSpinButton*, gpointer);

void bScoreClear_clicked_cb( GtkButton*, gpointer);
void bScoreNREM1_clicked_cb( GtkButton*, gpointer);
void bScoreNREM2_clicked_cb( GtkButton*, gpointer);
void bScoreNREM3_clicked_cb( GtkButton*, gpointer);
void bScoreNREM4_clicked_cb( GtkButton*, gpointer);
void bScoreREM_clicked_cb  ( GtkButton*, gpointer);
void bScoreWake_clicked_cb ( GtkButton*, gpointer);

void bSFForward_clicked_cb( GtkButton*, gpointer);
void bSFBack_clicked_cb( GtkButton*, gpointer);
void bScoreGotoPrevUnscored_clicked_cb( GtkButton*, gpointer);
void bScoreGotoNextUnscored_clicked_cb( GtkButton*, gpointer);
void bScoreGotoPrevArtifact_clicked_cb( GtkButton*, gpointer);
void bScoreGotoNextArtifact_clicked_cb( GtkButton*, gpointer);
void bSFDrawCrosshair_toggled_cb( GtkToggleButton*, gpointer);
void bSFShowFindDialog_toggled_cb( GtkToggleButton*, gpointer);
void bSFShowPhaseDiffDialog_toggled_cb( GtkToggleButton*, gpointer);
void bSFRunICA_clicked_cb( GtkButton*, gpointer);
//void bSFResetMontage_clicked_cb( GtkButton*, gpointer);


void eSFICARemixMode_changed_cb( GtkComboBox*, gpointer);
void eSFICANonlinearity_changed_cb( GtkComboBox*, gpointer);
void eSFICAApproach_changed_cb( GtkComboBox*, gpointer);
void eSFICAFineTune_toggled_cb( GtkCheckButton*, gpointer);
void eSFICAStabilizationMode_toggled_cb( GtkCheckButton*, gpointer);
void eSFICAa1_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAa2_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAmu_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAepsilon_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICASampleSizePercent_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICANofICs_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAEigVecFirst_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAEigVecLast_value_changed_cb( GtkSpinButton*, gpointer);
void eSFICAMaxIterations_value_changed_cb( GtkSpinButton*, gpointer);
void bSFICATry_clicked_cb( GtkButton*, gpointer);
void bSFICAPreview_toggled_cb( GtkToggleButton*, gpointer);
void bSFICAShowMatrix_toggled_cb( GtkToggleButton*, gpointer);
void wSFICAMatrix_hide_cb( GtkWidget*, gpointer);
void bSFICAApply_clicked_cb( GtkButton*, gpointer);
void bSFICACancel_clicked_cb( GtkButton*, gpointer);


void bSFAccept_clicked_cb( GtkToolButton*, gpointer);
void iSFAcceptAndTakeNext_activate_cb( GtkMenuItem*, gpointer);

void iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageUseResample_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawZeroline_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iSFPageFilter_activate_cb( GtkMenuItem*, gpointer);
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

void iSFICAPageMapIC_activate_cb( GtkRadioMenuItem*, gpointer);

void iSFPageAnnotationDelete_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationEdit_activate_cb( GtkMenuItem*, gpointer);

void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionFindPattern_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionAnnotate_activate_cb( GtkMenuItem*, gpointer);

void iSFPowerExportRange_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerExportAll_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerSmooth_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerDrawBands_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerUseThisScale_activate_cb( GtkMenuItem*, gpointer);

gboolean daSFHypnogram_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFHypnogram_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);

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

} // extern "C"

} // namespace aghui

#endif

// eof
