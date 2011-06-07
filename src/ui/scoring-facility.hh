// ;-*-C++-*- *  Time-stamp: "2011-06-08 02:38:15 hmmr"
/*
 *       File name:  ui/scoring-facility.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility bits shared between scoring-facility{,-patterns}.c
 *
 *         License:  GPL
 */

#ifndef _AGH_SCORING_FACILITY_H
#define _AGH_SCORING_FACILITY_H

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <gtk/gtk.h>

#include "libexstrom/exstrom.hh"
#include "libexstrom/signal.hh"
#include "ui.hh"
#include "draw-signal-generic.hh"
#include "settings.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;

namespace aghui {
namespace sf {

// all construct's in sf:: are partial: many widgets are now
// members of SScoringFacility and get constructed in ctor
int	construct_once();
void	destruct();
namespace filter {
	int	construct_once();
}
namespace patterns {
	int	construct_once();
	extern GtkListStore
		*mPatterns;
}
namespace phasediff {
	int	construct_once();
}


// structures^H

struct SScoringFacility {
	struct SChannel {
		const char
			*name,
			*type;
		agh::CRecording&
			recording;

		SScoringFacility&
			sf;

	      // signal waveforms, cached here
		valarray<float>
			signal_filtered,
			signal_original;
	      // filters
		struct SFilterInfo {
			float	cutoff;
			unsigned
				order;
			bool is_sane() const
				{
					return cutoff >= 0. && order < 6;
				}
		};
		SFilterInfo
			low_pass,
			high_pass;
		bool have_low_pass() const
			{
				return low_pass.cutoff > 0 && low_pass.order > 0;
			}
		bool have_high_pass() const
			{
				return high_pass.cutoff > 0 && high_pass.order > 0;
			}

		size_t n_samples() const
			{
				return signal_filtered.size();
			}
		size_t samplerate() const
			{
				return recording.F().samplerate(name);
			}

		float	signal_display_scale;

		bool	draw_original_signal:1,
			draw_processed_signal:1,
			draw_envelope:1,
			draw_course:1,
			draw_dzcdf:1,
			draw_bands:1,
			draw_spectrum_absolute:1,
			use_resample:1;

	      // artifacts
		float calculate_dirty_percent();
		float	percent_dirty;

	      // signal features
		struct SSFLowPassCourse {
			float	cutoff;
			unsigned
				order;
			valarray<float>
				data;
			float& operator[]( size_t i)
				{
					return data[i];
				}
			// SSFLowPassCourse( float _cutoff, unsigned _order, const valarray<float>& signal,
			// 		  unsigned samplerate)
			//       : cutoff (_cutoff), order (_order),
			// 	data (exstrom::low_pass( signal, samplerate, cutoff, order, true))
			// 	{}
			SSFLowPassCourse() = default;
		};
		SSFLowPassCourse
			signal_lowpass;
		void compute_lowpass( float _cutoff, unsigned _order)
			{
				if ( signal_lowpass.data.size() == 0 ||
				     signal_lowpass.cutoff != _cutoff || signal_lowpass.order != _order )
					signal_lowpass.data =
						exstrom::low_pass( signal_filtered, samplerate(),
								   signal_lowpass.cutoff = _cutoff,
								   signal_lowpass.order = _order, true);
			}

		struct SSFEnvelope {
			unsigned
				tightness;
			valarray<float>
				upper,
				lower;
			float breadth( size_t i) const
				{
					return upper[i] - lower[i];
				}
			valarray<float> breadth() const
				{
					return upper - lower;
				}
			// SSFEnvelope( unsigned _tightness,
			// 	     const valarray<float>& data_in, unsigned samplerate)
			//       : tightness (_tightness)
			// 	{
			// 		sigproc::envelope( data_in, tightness, samplerate,
			// 				   1./samplerate,
			// 				   lower, upper); // don't need anchor points, nor their count
			// 	}
			SSFEnvelope() = default;
		};
		SSFEnvelope
			signal_envelope;
		void compute_tightness( unsigned _tightness)
			{
				if ( signal_envelope.lower.size() == 0 ||
				     signal_envelope.tightness != _tightness )
					sigproc::envelope( signal_filtered,
							   signal_envelope.tightness = _tightness, samplerate(),
							   1./samplerate(),
							   signal_envelope.lower,
							   signal_envelope.upper); // don't need anchor points, nor their count
			}

		struct SSFDzcdf {
			float	step,
				sigma;
			unsigned
				smooth;
			valarray<float>
				data;
			float& operator[]( size_t i)
				{
					return data[i];
				}
			// SSFDzcdf( float _step, float _sigma, unsigned _smooth,
			// 	  const valarray<float>& data_in, unsigned samplerate)
			//       : step (_step), sigma (_sigma), smooth (_smooth),
			// 	data (sigproc::dzcdf( data_in, samplerate,
			// 			      step, sigma, smooth))
			// 	{}
			SSFDzcdf() = default;
		};
		SSFDzcdf
			signal_dzcdf;
		void compute_dzcdf( float _step, float _sigma, unsigned _smooth)
			{
				if ( signal_dzcdf.data.size() == 0 ||
				     signal_dzcdf.step != _step || signal_dzcdf.sigma != _sigma || signal_dzcdf.smooth != _smooth )
					signal_dzcdf.data =
						sigproc::dzcdf( signal_filtered, samplerate(),
								signal_dzcdf.step = _step,
								signal_dzcdf.sigma = _sigma,
								signal_dzcdf.smooth = _smooth);
			}

	      // power courses
		valarray<float>
			power; // can possibly live outside in core, no?
		float	from, upto;
		float	power_display_scale;
		bool have_power() const
			{
				return power.size() > 0;
			}

		array<valarray<float>, (size_t)agh::TBand::_total>
			power_in_bands;
		agh::TBand
			focused_band,
			uppermost_band;

	      // spectrum
		valarray<float>
			spectrum;  // per page, is volatile
		float	spectrum_upper_freq;
		unsigned
			n_bins,
			last_spectrum_bin;

	      // unsorted
		valarray<float>
			emg_fabs_per_page;
		float	emg_scale;

		bool is_expanded() const
			{
				return (bool)gtk_expander_get_expanded( expander);
			}
		GtkExpander
			*expander;
		GtkVBox
			*vbox;
		// GtkMenuItem
		// 	*menu_item;
		GtkDrawingArea
			*da_page,
			*da_power,
			*da_spectrum,
			*da_emg_profile;
		int	da_page_wd,
			da_page_ht,
			da_power_wd,
			da_power_ht,
			da_spectrum_wd,
			da_spectrum_ht,
			da_emg_profile_wd,
			da_emg_profile_ht;

	      // draw entire page
		void draw_page( const char *fname, int width, int height) // to a file
			{
#ifdef CAIRO_HAS_SVG_SURFACE
				cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
				cairo_t *cr = cairo_create( cs);
				draw_page( cr, width, height, false);
				cairo_destroy( cr);
				cairo_surface_destroy( cs);
#endif
			}
		void draw_page( cairo_t*); // to own da_page
		void draw_page()
			{
				gtk_widget_queue_draw( (GtkWidget*)da_page);
			}

	      // draw signal to a cairo_t canvas
		void draw_signal_original( unsigned width, int vdisp, cairo_t *cr)
			{
				draw_signal( signal_original, width, vdisp, cr);
			}
		void draw_signal_filtered( unsigned width, int vdisp, cairo_t *cr)
			{
				draw_signal( signal_filtered, width, vdisp, cr);
			}

		void mark_region_as_artifact( bool do_mark);
		void mark_region_as_pattern();

	      // convenience shortcuts
		void get_signal_original(); // also apply display filters
		void get_signal_filtered();
		void get_power()
			{
				power = recording.power_course<float>( from, upto);
			}
		void get_spectrum( size_t p)
			{
				spectrum = recording.power_spectrum<float>( p);
			}
		void get_power_in_bands()
			{
				for ( size_t b = 0; b < (size_t)uppermost_band; ++b )
					power_in_bands[b] =
						recording.power_course<float>( settings::FreqBands[b][0],
									       settings::FreqBands[b][1]);
			}

	      // ctor, dtor
		SChannel( agh::CRecording& r, SScoringFacility&);
	       ~SChannel();

		int h() const
			{
				return _h;
			}
		agh::CEDFFile::SSignal& ssignal()
			{
				return _ssignal;
			}

	    private:
		int	_h;
		agh::CEDFFile::SSignal&
			_ssignal;

		void draw_page( cairo_t*, int wd, int ht, bool draw_marquee);
		void draw_signal( const valarray<float>& signal,
				  unsigned width, int vdisp, cairo_t *cr)
			{
				::draw_signal( signal,
					       sf.cur_vpage_start() * samplerate(),
					       sf.cur_vpage_end() * samplerate(),
					       width, vdisp, signal_display_scale, cr, use_resample);
			}

		static float calibrate_display_scale( const valarray<float>&, size_t over, float fit);
	};
	list<SChannel>
		channels;
	time_t start_time() const
		{
			return channels.front().recording.F().start_time;
		}

	vector<char>
		hypnogram;
	size_t total_pages() const
		{
//			return channels.front().recording.F().n_pages();
			return hypnogram.size();
		}
	size_t total_vpages() const
		{
			return p2ap( total_pages());
		}
	void get_hypnogram()
		{
			// just get from the first source,
			// trust other sources are no different
			const CEDFFile& F = channels.begin()->recording.F();
			hypnogram.resize( F.agh::CHypnogram::length());
			for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
				hypnogram[p] = F.nth_page(p).score_code();
		}
	void put_hypnogram()
		{
			// but put to all
			for_each( _sepisode.sources.begin(), _sepisode.sources.end(),
				  [&] ( agh::CEDFFile& F)
				  {
					  for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
						  F.nth_page(p).mark( hypnogram[p]);
				  });
		}

	float	scored_percent,
		scored_percent_nrem,
		scored_percent_rem,
		scored_percent_wake;

	void calculate_scored_percent()
		{
			scored_percent = channels.front().recording.F().percent_scored(
				&scored_percent_nrem,
				&scored_percent_rem,
				&scored_percent_wake);
		}

      // ctor, dtor
	SScoringFacility( agh::CSubject&, const string& d, const string& e);
    private:
	agh::CSubject&
		_csubject;
	agh::CSubject::SEpisode&
		_sepisode;
    public:
	agh::CSubject& csubject()
		{
			return _csubject;
		}
	agh::CSubject::SEpisode& sepisode()
		{
			return _sepisode;
		}
       ~SScoringFacility();

	float	sane_signal_display_scale,
		sane_power_display_scale; // 2.5e-5;

	bool	draw_crosshair:1,
		draw_power:1;

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

	size_t cur_vpage_start() const // in seconds
		{
			return _cur_vpage * vpagesize();
		}
	size_t cur_vpage_end() const // in seconds
		{
			return (_cur_vpage + 1) * vpagesize();
		}

	agh::TScore cur_page_score() const
		{
			return agh::SPage::char2score( hypnogram[_cur_page]);
		}
	bool page_has_artifacts( size_t);

	static size_t pagesize()
		{
			return DisplayPageSizeValues[DisplayPageSizeCurrentItem];
		}
	size_t vpagesize() const
		{
			return DisplayPageSizeValues[pagesize_item];
		}
	bool pagesize_is_right() const
		{
			return DisplayPageSizeCurrentItem == pagesize_item;
		}

	void set_pagesize( int item); // touches a few wisgets

      // selection and marquee
	GtkDrawingArea
		*marking_in_widget;
	double	marquee_start,
		marquee_virtual_end;
	size_t	selection_start,
		selection_end;
	size_t marquee_to_selection();
	size_t selection_size() const
		{
			return selection_end - selection_start;
		}

      // menu support
	SChannel
		*using_channel;

      // misc supporting functions
	void queue_redraw_all() const;
	void repaint_score_stats() const;
	void do_score_forward( char score_ch);

      // unfazer
	enum class TUnfazerMode {
		none,
		channel_select,
		calibrate,
	};
	TUnfazerMode
		unfazer_mode;
	SChannel
		*unfazer_offending_channel;
	float
		unfazer_factor;  // as currently being tried

      // tips
	enum class TTipIdx {
		general,
		unfazer
	};
	void set_tooltip( TTipIdx i)
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
		float	a, b, c;

	      // loadable
		valarray<float>
			pattern;
		size_t	samplerate;
		size_t	context_before,
			context_after;
		static const size_t
			context_pad = 100;
		size_t pattern_size_essential() const
			{
				return pattern.size()
					- context_before - context_after;
			}

	      // finding tool
		sigproc::CPattern<float>
			*cpattern;
		size_t	last_find;
		int	increment;

		void load_pattern( SScoringFacility::SChannel&); // load selection on this channel
		void load_pattern( const char* name, bool globally); // load named
		void save_pattern( const char* name, bool globally);
		void discard_pattern( const char *label, bool globally);

		SScoringFacility::SChannel
			*field_channel;

		bool search( ssize_t from);
		float	match_a,
			match_b,
			match_c;

	      // ctor, dtor
		SFindDialog( SScoringFacility& parent);
	       ~SFindDialog();

	      // more settings
		bool	draw_details:1;

		void enumerate_patterns_to_combo();
		void preselect_entry( const char*, bool globally);
		void preselect_channel( const char*);
		void enable_controls( bool);
		void acquire_parameters();
		void update_displayed_parameters();

		float	display_scale;

	    private:
		SScoringFacility&
			_parent;
	      // widgets
		int construct_widgets();
	    public:
		GtkDialog
			*wPattern;
		GtkComboBox
			*ePatternChannel,
			*ePatternList;
		GtkDrawingArea
			*daPatternSelection;
		GtkButton
			*bPatternFindNext,
			*bPatternFindPrevious,
			*bPatternSave,
			*bPatternDiscard;
		GtkSpinButton
			*ePatternEnvTightness,
			*ePatternFilterCutoff,
			*ePatternFilterOrder,
			*ePatternDZCDFStep,
			*ePatternDZCDFSigma,
			*ePatternDZCDFSmooth,
			*ePatternParameterA,
			*ePatternParameterB,
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
		SFiltersDialog( SScoringFacility& parent)
		      : _parent (parent)
			{
				if ( construct_widgets() )
					throw runtime_error( "SScoringFacility::SFiltersDialog(): Failed to construct own wisgets");
			}
	       ~SFiltersDialog()
			{
				gtk_widget_destroy( (GtkWidget*)wFilters);
			}
	    private:
		SScoringFacility&
			_parent;

		int construct_widgets();
	    public:
		GtkDialog
			*wFilters;
		GtkLabel
			*lFilterCaption;
		GtkSpinButton
			*eFilterLowPassCutoff,
			*eFilterHighPassCutoff,
			*eFilterLowPassOrder,
			*eFilterHighPassOrder;
		GtkButton
			*bFilterOK;
	};
	SFiltersDialog
		filters_dialog;

	struct SPhasediffDialog {
		const SChannel
			*channel1,
			*channel2;
		bool	use_original_signal:1;
		float	from,
			upto;

		unsigned
			bwf_order,
			scope;
		float	display_scale;

		valarray<float>
			course;
		void update_course();

		const SChannel* channel_from_cbox( GtkComboBox *cbox);
		void preselect_channel( GtkComboBox *cbox, const char *ch);

		SPhasediffDialog( SScoringFacility& parent)
		      : channel1 (NULL),
			channel2 (NULL),
			use_original_signal (false),
			from (1.), upto (2.),
			bwf_order (1),
			scope (10),
			display_scale (1.),
			course (0), // have no total_pages() known yet
			_parent (parent)
			{
				if ( construct_widgets() )
					throw runtime_error( "SScoringFacility::SPhasediffDialog(): Failed to construct own wisgets");
			}
	       ~SPhasediffDialog()
			{
				gtk_widget_destroy( (GtkWidget*)wPhaseDiff);
			}
		SScoringFacility&
			_parent;

	    private:
		int construct_widgets();
	    public:
		GtkDialog
			*wPhaseDiff;
		GtkComboBox
			*ePhaseDiffChannelA,
			*ePhaseDiffChannelB;
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

	bool suppress_redraw;

      // own widgets
	int construct_widgets();
    public:
	// main Scoring Facility window
	GtkWindow
		*wScoringFacility;
	GtkComboBox
		*eScoringFacPageSize;
	GtkSpinButton
		*eScoringFacCurrentPage;
	GtkMenu
		*mSFPage,  // sets some GtkCheckMenuItem's
		*mSFPageSelection, // rest can have no user_data
		*mSFPower,
		*mSFScore,
		*mSFSpectrum;
	GtkCheckMenuItem
		*iSFPageShowOriginal,
		*iSFPageShowProcessed,
		*iSFPageUseResample;

		// *iSFPageShowDZCDF,
		// *iSFPageShowEnvelope;
	GtkMenuItem
		*iSFPageSelectionMarkArtifact,
		*iSFPageSelectionClearArtifact,
		*iSFPageSelectionFindPattern,
		*iSFPageUnfazer,
		*iSFPageFilter,
		*iSFPageSaveAs,
		*iSFPageExportSignal,
		*iSFPageUseThisScale,

		*iSFPowerExportAll,
		*iSFPowerExportRange,
		*iSFPowerUseThisScale,

		*iSFScoreAssist,
		*iSFScoreImport,
		*iSFScoreExport,
		*iSFScoreClear,

		*iSFAcceptAndTakeNext;
	GtkToggleButton
		*bScoringFacDrawPower,
		*bScoringFacDrawCrosshair,
		*bScoringFacShowFindDialog,
		*bScoringFacShowPhaseDiffDialog;
	GtkStatusbar
		*sbSF;
    private:
	GtkVBox
		*cScoringFacPageViews;
	GtkDrawingArea
		*daScoringFacHypnogram;
	GtkButton
		*bScoringFacBack,
		*bScoringFacForward;
	GtkToolButton  // there's no reason for these to be different from those two above; just they happen to be toolbuttons in glade
		*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
		*bScoreREM,   *bScoreWake,  *bScoreMVT,
		*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
		*bScoreGotoPrevArtifact, *bScoreGotoNextArtifact,
		*bSFAccept;
	GtkLabel
		*lScoringFacTotalPages,
		*lScoringFacClockTime,
		*lScoringFacPercentScored,
		*lScoringFacCurrentPos,
		*lScoreStatsNREMPercent,
		*lScoreStatsREMPercent,
		*lScoreStatsWakePercent,
		*lScoringFacCurrentStage,
		*lScoringFacHint;
	GtkTable
		*cScoringFacSleepStageStats;

    public:
	// here's hoping configure-event comes before expose-event
	gint	daScoringFacHypnogram_wd,
		daScoringFacHypnogram_ht;
};





} // namespace sf



// forward declarations of callbacks
extern "C" {

	gboolean da_page_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
	gboolean da_power_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
	gboolean da_spectrum_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
	gboolean da_emg_profile_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

	gboolean daScoringFacPageView_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacPageView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
	gboolean daScoringFacPageView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacPSDProfileView_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacPSDProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPSDProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacEMGProfileView_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacEMGProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacEMGProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacSpectrumView_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacSpectrumView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacSpectrumView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	void eScoringFacPageSize_changed_cb( GtkComboBox*, gpointer);
	void eScoringFacCurrentPage_value_changed_cb( GtkSpinButton*, gpointer);

	void bScoreClear_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM1_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM2_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM3_clicked_cb( GtkButton*, gpointer);
	void bScoreNREM4_clicked_cb( GtkButton*, gpointer);
	void bScoreREM_clicked_cb  ( GtkButton*, gpointer);
	void bScoreWake_clicked_cb ( GtkButton*, gpointer);
	void bScoreMVT_clicked_cb  ( GtkButton*, gpointer);

	void bScoringFacForward_clicked_cb( GtkButton*, gpointer);
	void bScoringFacBack_clicked_cb( GtkButton*, gpointer);
	void bScoreGotoPrevUnscored_clicked_cb( GtkButton*, gpointer);
	void bScoreGotoNextUnscored_clicked_cb( GtkButton*, gpointer);
	void bScoreGotoPrevArtifact_clicked_cb( GtkButton*, gpointer);
	void bScoreGotoNextArtifact_clicked_cb( GtkButton*, gpointer);
	void bScoringFacDrawPower_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacShowFindDialog_toggled_cb( GtkToggleButton*, gpointer);
	void bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton*, gpointer);

	void bSFAccept_clicked_cb( GtkButton*, gpointer);

	void mSFPage_show_cb( GtkWidget*, gpointer);
	void iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageUseResample_toggled_cb( GtkCheckMenuItem*, gpointer);
	// void iSFPageShowEnvelope_toggled_cb( GtkCheckMenuItem*, gpointer);
	void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageFilter_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageUnfazer_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSaveAs_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageExportSignal_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageUseThisScale_activate_cb( GtkMenuItem*, gpointer);

	void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem*, gpointer);
	//	void iSFPageSelectionInspectMany_activate_cb( GtkMenuItem*, gpointer);
	void iSFPageSelectionFindPattern_activate_cb( GtkMenuItem*, gpointer);

	void iSFPowerExportRange_activate_cb( GtkMenuItem*, gpointer);
	void iSFPowerExportAll_activate_cb( GtkMenuItem*, gpointer);
	void iSFPowerUseThisScale_activate_cb( GtkMenuItem*, gpointer);

	gboolean daScoringFacHypnogram_draw_cb( GtkWidget*, cairo_t*, gpointer);
	gboolean daScoringFacHypnogram_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
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
