/*
 *       File name:  ui/sf/sf.hh
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

#include <map>
#include <list>

#include <cairo/cairo.h>
#include <gtk/gtk.h>

#include "common/alg.hh"
#include "common/config-validate.hh"
#include "sigproc/winfun.hh"
#include "sigproc/sigproc.hh"
#include "patterns/patterns.hh"
#include "metrics/mc-artifacts.hh"
#include "expdesign/primaries.hh"
#include "ica/ica.hh"
#include "ui/globals.hh"
#include "ui/ui++.hh"
#include "ui/mw/mw.hh"
#include "widgets.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace aghui {

class SScoringFacility
  : public SScoringFacilityWidgets {

	DELETE_DEFAULT_METHODS (SScoringFacility);

    public:
      // ctor, dtor
	SScoringFacility (agh::CSubject&, const string& d, const string& e,
			  SExpDesignUI& parent);
       ~SScoringFacility ();

      // link to parent
	SExpDesignUI&
		_p;
	void redraw_ssubject_timeline() const;

    private:
	agh::CSubject&
		_csubject;
	string	_session;
	agh::CSubject::SEpisode&
		_sepisode;
    public:
	agh::CSubject&			csubject() const { return _csubject; }
	agh::CSubject::SEpisode&	sepisode() const { return _sepisode; }
	const string&			session()  const { return _session;  }

      // channels
	struct SChannel {

		DELETE_DEFAULT_METHODS (SChannel);

		string	name;
		sigfile::SChannel::TType
			type;
		bool operator==( const string&) const;
		bool operator==( const SChannel&) const;

		agh::CRecording&
			crecording;
		int	_h;
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
		void get_signal_original();
		void get_signal_filtered();

	      // filters
		bool have_low_pass() const;
		bool have_high_pass() const;
		bool have_notch_filter() const;
		size_t n_samples() const;
		size_t samplerate() const;

	      // artifacts
		float	percent_dirty;
		float calculate_dirty_percent();
		void detect_artifacts( const metrics::mc::SArtifactDetectionPP&);
		pair<double, double>
		mark_flat_regions_as_artifacts( double at_least_this_long, double pad);

	      // annotations
		list<sigfile::SAnnotation*>
		in_annotations( double time) const;

	      // signal metrics
	  	static pattern::SPatternPPack<TFloat>
			pattern_params;
		sigproc::SCachedLowPassCourse<TFloat>
			signal_lowpass;
		sigproc::SCachedBandPassCourse<TFloat>
			signal_bandpass;
		sigproc::SCachedEnvelope<TFloat>
			signal_envelope;
		sigproc::SCachedDzcdf<TFloat>
			signal_dzcdf;
		void
		drop_cached_signal_properties()
			{
				signal_lowpass.drop();
				signal_bandpass.drop();
				signal_envelope.drop();
				signal_dzcdf.drop();
			}

	      // profiles
		// psd
		struct SProfilePSD {
			valarray<TFloat>
				course;
			double	from, upto;
			double	display_scale; // saved via libconfig, requiring it to be double
			array<valarray<TFloat>, metrics::psd::TBand::TBand_total>
				course_in_bands;
			size_t	focused_band,
				uppermost_band;
		};
		void get_psd_course();
		void get_psd_in_bands();
		SProfilePSD
			psd;
		// swu
		struct SProfileSWU {
			valarray<TFloat>
				course;
			double	f0;
			double	display_scale;
		};
		void get_swu_course();
		SProfileSWU
			swu;
		// mc
		struct SProfileMC {
			valarray<TFloat>
				course;
			double	display_scale;
			double	f0;
		};
		SProfileMC
			mc;
		void get_mc_course();

		tuple<metrics::TType, valarray<TFloat>&>
		which_profile( metrics::TType);

		void update_profile_display_scales();

	      // spectrum
		valarray<TFloat>
			spectrum;  // per page, is volatile
		float	spectrum_upper_freq;
		unsigned
			spectrum_bins,
			last_spectrum_bin;
		void get_spectrum(); // at current page
		void get_spectrum( size_t p);

	      // raw profile
		valarray<TFloat>
			raw_profile;
		void get_raw_profile();

	      // emg
		double	emg_display_scale;

	      // region
		void mark_region_as_artifact( bool do_mark);
		void mark_region_as_annotation( const string&, sigfile::SAnnotation::TType);
		void mark_region_as_pattern();

	      // ctor, dtor
		SChannel( agh::CRecording& r, SScoringFacility&, size_t y, char seq);

		int h() const
			{
				return _h;
			}

		double	zeroy;
		bool operator<( const SChannel&) const;

		double	signal_display_scale;

		// saved flags
		bool	hidden,
			draw_zeroline,
			draw_original_signal,
			draw_filtered_signal,
			zeromean_original,
			zeromean_filtered,
			draw_psd,
			draw_swu,
			draw_mc,
			draw_emg,
			draw_psd_bands,
			autoscale_profile,
			draw_spectrum,
			resample_signal,
			resample_power,
			draw_selection_course,
			draw_selection_envelope,
			draw_selection_dzcdf,
			draw_phasic_spindle,
			draw_phasic_Kcomplex,
			draw_phasic_eyeblink;
		bool	discard_marked,
			apply_reconstituted;

		forward_list<confval::SValidator<bool>>		config_keys_b;
		forward_list<confval::SValidator<int>>		config_keys_d;
		forward_list<confval::SValidator<double>>	config_keys_g;
		void update_channel_menu_items( double x);
		void update_power_menu_items();
		void selectively_enable_selection_menu_items();

	      // selection and marquee
		float	marquee_mstart,
			marquee_mend,        // in terms of event->x
			marquee_start,
			marquee_end;         // set on button_release
		double	selection_start_time,
			selection_end_time;  // in seconds
		size_t	selection_start,
			selection_end;       // in samples
		TFloat	selection_SS,
			selection_SU;
		size_t marquee_to_selection();
		void put_selection( size_t a, size_t e);
		void put_selection( double a, double e);
	    private:
		void _put_selection();

	    public:
		float spp() const;
		float fine_line() const;
		int sample_at_click( double) const;

		GtkMenuItem
			*menu_item_when_hidden;

	      // comprehensive draw
		void draw_for_montage( const string& fname, int width, int height); // to a file
		void draw_for_montage( cairo_t*); // to montage

	    protected:
		void draw_page( cairo_t*, int wd, float zeroy, // writers to an svg file override zeroy (with 0)
				bool draw_marquee) const;
		void draw_overlays( cairo_t*, int wd, float zeroy) const;

	      // strictly draw the signal waveform bare
	      // (also used as such in some child dialogs)
		void draw_signal_original( size_t width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_original, width, vdisp, cr);
			}
		void draw_signal_filtered( size_t width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_filtered, width, vdisp, cr);
			}
		friend class SScoringFacility;
		void draw_signal_reconstituted( size_t width, int vdisp, cairo_t *cr) const
			{
				draw_signal( signal_reconstituted, width, vdisp, cr);
			}
	      // generic draw_signal wrapper
		void draw_signal( const valarray<TFloat>& signal,
				  size_t width, int vdisp, cairo_t *cr) const;
	};
	list<SChannel>
		channels;
	size_t	n_eeg_channels;
	SChannel& operator[]( const string&);
	SChannel& operator[]( size_t i)
		{
			return channel_by_idx(i);
		}
	SChannel& channel_by_idx( size_t i);

	void
	update_all_channels_profile_display_scale();

      // common annotations
	list<sigfile::SAnnotation>
		common_annotations;

      // timeline
	time_t start_time() const
		{
			return channels.front().crecording.F().start_time();
		}

	vector<char>
		hypnogram;
	size_t total_pages() const
		{
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
		hypnogram_button_down:1;
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
	bool	show_cur_pos_time_relative,
		draw_crosshair,
		alt_hypnogram;

      // page and vpage index
	size_t cur_page() const		{ return _cur_page;  }
	size_t cur_vpage() const	{ return _cur_vpage; }
	void set_cur_vpage( size_t p, bool touch_self = true);

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
	bool page_has_artifacts( size_t, bool check_all_channels = true) const;
	bool page_has_annotations( size_t, const SChannel&) const;

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
	size_t vpagesize() const;
	bool pagesize_is_right() const;
	size_t cur_vpage_start() const; // in seconds
	size_t cur_vpage_end() const; // in seconds
	size_t p2ap( size_t p) const; // page to visible_page
	size_t ap2p( size_t p) const;
    private:
	size_t	_cur_page,
		_cur_vpage;

    public:
      // page location adjusted for pre- and post margins
	float	skirting_run_per1;
	float xvpagesize() const;
	double cur_xvpage_start() const;
	double cur_xvpage_end() const;
	double time_at_click( double) const;

	void set_vpagesize_item( int item, bool touch_self = true); // touches a few wisgets
	void set_vpagesize( size_t seconds, bool touch_self = true);

    public:
      // ICA support
	ica::CFastICA
		*ica;
	itpp::mat  // looks like it has to be double
		ica_components;
	size_t n_ics() const;

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

    public:
      // channel slots
	template <class T>
	float channel_y0( const T& h) const;

	SChannel*
	channel_near( int y);

	float	interchannel_gap;
	void estimate_montage_height();
	int find_free_space();
	void space_evenly();
	void expand_by_factor( double);

	int	n_hidden;
      // shuffling manually
	double	event_y_when_shuffling;
	float	zeroy_before_shuffling;

    public:
      // montage
	// load/save/reset
	forward_list<confval::SValidator<bool>>		config_keys_b;
	forward_list<confval::SValidator<int>>		config_keys_d;
	forward_list<confval::SValidator<float>>	config_keys_g;
	void load_montage();
	void save_montage(); // using libconfig
	void reset_montage();
	// draw
	void draw_montage( cairo_t*);
	void draw_montage( const string& fname); // to a file (uses da_wd and da_ht
    private:
	template <class T>
	void _draw_matrix_to_montage( cairo_t*, const itpp::Mat<T>&);
	void _draw_hour_ticks( cairo_t*, int, int, bool with_cursor = true);
    public:
	void draw_hypnogram( cairo_t*);
	void draw_score_stats() const;
	void draw_current_pos( double) const;
	void queue_redraw_all() const;

	void do_score_forward( char score_ch);
	void do_score_back( char score_ch);

      // status bar
	void sb_message( const string&) const;
	void sb_clear() const;

      // tips
	enum TTipIdx : size_t {
		scoring_mode,
		ica_mode
	};
	void set_tooltip( TTipIdx i) const;

      // child dialogs:
    public:
	struct SPatternsDialog;
	SPatternsDialog&
	patterns_d();

	struct SFiltersDialog;
	SFiltersDialog&
	filters_d();

	struct SPhasediffDialog;
	SPhasediffDialog&
	phasediff_d();

	struct SArtifactsDialog;
	SArtifactsDialog&
	artifacts_d();

	struct SArtifactsSimpleDialog;
	SArtifactsSimpleDialog&
	artifacts_simple_d();

    private:
	SPatternsDialog
		*_patterns_d;
	SFiltersDialog
		*_filters_d;
	SPhasediffDialog
		*_phasediff_d;
	SArtifactsDialog
		*_artifacts_d;
	SArtifactsSimpleDialog
		*_artifacts_simple_d;

    public:
      // menu support
	SChannel
		*using_channel;
	int using_channel_idx() const
		{
			return channel_idx( using_channel);
		}
	int channel_idx( SChannel* h) const
		{
			int i = 0;
			for ( auto& H : channels )
				if ( &H == h )
					return i;
				else ++i;
			return -1;
		}
	list<sigfile::SAnnotation*>
		over_annotations;
	sigfile::SAnnotation*
	interactively_choose_annotation() const;

    private:
	static const char* const tooltips[2];

    public:
	// SGeometry
	// 	geometry;

	static size_t
		IntersignalSpace,
		HypnogramHeight,
		EMGProfileHeight;

    public:
	// here's hoping configure-event comes before expose-event
	gint	da_wd;
	float	da_ht;  // not subject to window resize, this, but should withstand / 3 * 3
};


inline bool
SScoringFacility::SChannel::
operator==( const string& _name) const
{
	return name == _name;
}
inline bool
SScoringFacility::SChannel::
operator==( const SChannel& rv) const
{
	return name == rv.name;
}


inline bool
SScoringFacility::SChannel::
have_low_pass() const
{
	return isfinite(filters.low_pass_cutoff)
		&& filters.low_pass_cutoff > 0.
		&& filters.low_pass_order > 0;
}

inline bool
SScoringFacility::SChannel::
have_high_pass() const
{
	return isfinite(filters.high_pass_cutoff)
		&& filters.high_pass_cutoff > 0.
		&& filters.high_pass_order > 0;
}
inline bool
SScoringFacility::SChannel::
have_notch_filter() const
{
	return filters.notch_filter != sigfile::SFilterPack::TNotchFilter::none;
}

inline size_t
SScoringFacility::SChannel::
n_samples() const
{
	return signal_filtered.size();
}


inline bool
SScoringFacility::SChannel::
operator<( const SChannel& rv) const
{
	return zeroy < rv.zeroy;
}


inline float
SScoringFacility::SChannel::
spp() const
{
	return (float)samplerate() * _p.vpagesize() / _p.da_wd;
}
inline float
SScoringFacility::SChannel::
fine_line() const
{
	return ((not resample_signal) and spp() > 1.)
		? agh::alg::value_within( .6 / (spp() + .2), .1, 3.)
		: .6;
}
inline int
SScoringFacility::SChannel::
sample_at_click( double x) const
{
	return _p.time_at_click( x) * samplerate();
}




inline size_t
SScoringFacility::SChannel::
samplerate() const
{
	return crecording.F().samplerate(_h);
}



inline size_t
SScoringFacility::
vpagesize() const
{
	return DisplayPageSizeValues[pagesize_item];
}
inline bool
SScoringFacility::
pagesize_is_right() const
{
	return pagesize() == vpagesize();
}

inline size_t
SScoringFacility::
cur_vpage_start() const // in seconds
{
	return _cur_vpage * vpagesize();
}
inline size_t
SScoringFacility::
cur_vpage_end() const // in seconds
{
	return (_cur_vpage + 1) * vpagesize();
}
inline size_t
SScoringFacility::
p2ap( size_t p) const // page to visible_page
{
	return (size_t)(p * (float)pagesize() / vpagesize());
}

inline size_t
SScoringFacility::
ap2p( size_t p) const
{
	return (size_t)((p) * (float)vpagesize() / pagesize());
}



inline float
SScoringFacility::
xvpagesize() const
{
	return (1. + 2*skirting_run_per1) * vpagesize();
}
inline double
SScoringFacility::
cur_xvpage_start() const
{
	return cur_vpage_start() - skirting_run_per1 * vpagesize();
}
inline double
SScoringFacility::
cur_xvpage_end() const
{
	return cur_vpage_end() + skirting_run_per1 * vpagesize();
}
inline double
SScoringFacility::
time_at_click( double x) const
{
	return cur_xvpage_start() + x/da_wd * xvpagesize();
}

inline void
SScoringFacility::
set_vpagesize( size_t seconds, bool touch_self)
{
	set_vpagesize_item( figure_display_pagesize_item( seconds), touch_self);
}



inline size_t
SScoringFacility::
n_ics() const
{
	return ica->obj() . get_nrof_independent_components();
}



template <class T>
float
__attribute__ ((pure))
SScoringFacility::
channel_y0( const T& h) const
{
	auto H = find( channels.begin(), channels.end(), h);
	return ( H != channels.end() ) ? H->zeroy : NAN;
}


} // namespace aghui


extern "C" {

//gboolean wScoringFacility_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);
gboolean daSFMontage_configure_event_cb( GtkWidget*, GdkEventConfigure*, gpointer);

gboolean daSFMontage_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFMontage_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFMontage_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_leave_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
gboolean daSFMontage_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

void eSFPageSize_changed_cb( GtkComboBox*, gpointer);
void eSFCurrentPage_value_changed_cb( GtkSpinButton*, gpointer);

void bSFScoreClear_clicked_cb( GtkButton*, gpointer);
void bSFScoreNREM1_clicked_cb( GtkButton*, gpointer);
void bSFScoreNREM2_clicked_cb( GtkButton*, gpointer);
void bSFScoreNREM3_clicked_cb( GtkButton*, gpointer);
void bSFScoreNREM4_clicked_cb( GtkButton*, gpointer);
void bSFScoreREM_clicked_cb  ( GtkButton*, gpointer);
void bSFScoreWake_clicked_cb ( GtkButton*, gpointer);

void eSFCurrentPos_clicked_cb( GtkButton*, gpointer);
void bSFForward_clicked_cb( GtkButton*, gpointer);
void bSFBack_clicked_cb( GtkButton*, gpointer);
void bSFGotoPrevUnscored_clicked_cb( GtkButton*, gpointer);
void bSFGotoNextUnscored_clicked_cb( GtkButton*, gpointer);
void bSFGotoPrevArtifact_clicked_cb( GtkButton*, gpointer);
void bSFGotoNextArtifact_clicked_cb( GtkButton*, gpointer);
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
void iSFPageArtifactsDetect_activate_cb( GtkMenuItem*, gpointer);
void iSFPageArtifactsMarkFlat_activate_cb( GtkMenuItem*, gpointer);
void iSFPageArtifactsClear_activate_cb( GtkMenuItem*, gpointer);
void iSFPageFilter_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSaveChannelAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSaveMontageAsSVG_activate_cb( GtkMenuItem*, gpointer);
void iSFPageExportSignal_activate_cb( GtkMenuItem*, gpointer);
void iSFPageUseThisScale_activate_cb( GtkMenuItem*, gpointer);
void iSFPageClearArtifacts_activate_cb( GtkMenuItem*, gpointer);
void iSFPageHide_activate_cb( GtkMenuItem*, gpointer);
void iSFPageHidden_select_cb( GtkMenuItem*, gpointer);
void iSFPageHidden_deselect_cb( GtkMenuItem*, gpointer);
void iSFPageShowHidden_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSpaceEvenly_activate_cb( GtkMenuItem*, gpointer);
void iSFPageLocateSelection_activate_cb( GtkMenuItem*, gpointer);
void iSFPageDrawPSDProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawPSDSpectrum_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawSWUProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawMCProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawEMGProfile_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawPhasicSpindles_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawPhasicKComplexes_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageDrawPhasicEyeBlinks_toggled_cb( GtkCheckMenuItem*, gpointer);

void iSFPageSelectionDrawCourse_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageSelectionDrawEnvelope_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPageSelectionDrawDzxdf_toggled_cb( GtkCheckMenuItem*, gpointer);

void iSFICAPageMapIC_activate_cb( GtkRadioMenuItem*, gpointer);

void iSFPageAnnotationDelete_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationEdit_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationClearAll_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationGotoPrev_activate_cb( GtkMenuItem*, gpointer);
void iSFPageAnnotationGotoNext_activate_cb( GtkMenuItem*, gpointer);

void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionFindPattern_activate_cb( GtkMenuItem*, gpointer);
void iSFPageSelectionAnnotate_activate_cb( GtkMenuItem*, gpointer);

void iSFPowerExportRange_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerExportAll_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerSmooth_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerDrawBands_toggled_cb( GtkCheckMenuItem*, gpointer);
void iSFPowerUseThisScale_activate_cb( GtkMenuItem*, gpointer);
void iSFPowerAutoscale_toggled_cb( GtkCheckMenuItem*, gpointer);

gboolean daSFHypnogram_draw_cb( GtkWidget*, cairo_t*, gpointer);
gboolean daSFHypnogram_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFHypnogram_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSFHypnogram_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);

void iSFScoreAssist_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreImport_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreExport_activate_cb( GtkMenuItem*, gpointer);
void iSFScoreClear_activate_cb( GtkMenuItem*, gpointer);

gboolean wSF_delete_event_cb( GtkWidget*, GdkEvent*, gpointer);
gboolean wSF_key_press_event_cb( GtkWidget*, GdkEventKey*, gpointer);

} // extern "C"



#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

