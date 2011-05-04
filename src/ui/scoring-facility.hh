// ;-*-C++-*- *  Time-stamp: "2011-05-04 03:17:30 hmmr"
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

#include <cairo.h>
#include <gtk/gtk.h>

#include "libexstrom/exstrom.hh"
#include "libexstrom/signal.hh"


#if HAVE_CONFIG_H
#  include <config.h>
#endif


using namespace std;

namespace aghui {

namespace sf {


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
		};
		SFilterInfo
			low_pass,
			high_pass;

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
			SSFLowPassCourse( float _cutoff, unsigned _order, const valarray<float>& signal,
					  unsigned samplerate)
			      : cutoff (_cutoff), order (_order),
				data (exstrom::low_pass( signal, samplerate, cutoff, order, true))
				{}
			SSFLowPassCourse() = default;
		};
		SSFLowPassCourse
			signal_lowpass;

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
			SSFEnvelope( unsigned _tightness,
				     const valarray<float>& data_in, unsigned samplerate)
			      : tightness (_tightness)
				{
					sigproc::envelope( data_in, tightness, samplerate,
							   1./samplerate,
							   lower, upper); // don't need anchor points, nor their count
				}
			SSFEnvelope() = default;
		};
		SSFEnvelope
			signal_breadth;

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
			SSFDzcdf( float _step, float _sigma, unsigned _smooth,
				  const valarray<float>& data_in, unsigned samplerate)
			      : step (_step), sigma (_sigma), smooth (_smooth),
				data (sigproc::dzcdf( data_in, samplerate,
						      step, sigma, smooth))
				{}
			SSFDzcdf() = default;
		};
		SSFDzcdf
			signal_dzcdf;
		bool have_sa_features() const
			{
				return /* for example */ signal_dzcdf.data.size() > 0;
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

		array<valarray<float>, (size_t)TBand::_total>
			power_in_bands;
		TBand	focused_band,
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
		void draw_page(); // to own da_page

	      // draw signal to a cairo_t canvas
		void draw_signal_original( unsigned width, int vdisp, cairo_t *cr)
			{
				draw_signal( signal_original, width, vdisp, cr);
			}
		void draw_signal_filtered( unsigned width, int vdisp, cairo_t *cr)
			{
				draw_signal( signal_filtered, width, vdisp, cr);
			}

	      // draw arbitrary region
		void draw_signal( const valarray<float>& signal,
				  size_t start, size_t end,
				  unsigned, int, float display_scale,
				  cairo_t*);

		void mark_region_as_artifact( size_t start, size_t end, bool do_mark);

	      // convenience shortcuts
		void get_signal_original()
			{
				signal_original  = recording.F().get_signal_original<const char*, float>( name);
			}
		void get_signal_filtered()
			{
				signal_filtered  = recording.F().get_signal_filtered<const char*, float>( name);
			}
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
				draw_signal( signal,
					     sf.cur_vpage_start() * samplerate(),
					     sf.cur_vpage_end() * samplerate(),
					     width, vdisp, signal_display_scale, cr);
			}

		float* _resample_buffer;
		size_t _resample_buffer_size;

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
       ~SScoringFacility();

	float	sane_signal_display_scale,
		sane_power_display_scale; // 2.5e-5;

	bool	draw_crosshair:1,
		draw_power:1;

	size_t	crosshair_at;

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

	size_t cur_vpage_start() const // in seconds
		{
			return _cur_vpage * vpagesize();
		}
	size_t cur_vpage_end() const // in seconds
		{
			return (_cur_vpage + 1) * vpagesize();
		}

	TScore cur_page_score() const
		{
			return agh::SPage::char2score( hypnogram[_cur_page]);
		}
	bool page_has_artifacts( size_t);

	static size_t pagesize()
		{
			return settings::DisplayPageSizeValues[settings::DisplayPageSizeItem];
		}
	size_t vpagesize() const
		{
			return settings::DisplayPageSizeValues[pagesize_item];
		}
	bool pagesize_is_right() const
		{
			return settings::DisplayPageSizeItem == pagesize_item;
		}

	void set_pagesize( int item); // touches a few wisgets

	size_t selection_size() const
		{
			return selection_end - selection_start;
		}

	void do_score_forward( char score_ch);

	GtkDrawingArea
		*marking_in_widget;
	double	marquee_start,
		marquee_virtual_end;
	size_t	selection_start,
		selection_end;
	size_t marquee_to_selection();

	SChannel
		*using_channel;

	void queue_redraw_all() const;

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

	enum class TTipIdx {
		general,
		unfazer
	};
	void set_tooltip( TTipIdx i)
		{
			gtk_widget_set_tooltip_markup( (GtkWidget*)lScoringFacHint, tooltips[(int)i]);
		}

    private:
	size_t	_cur_page,  // need them both
		_cur_vpage; // apparent

	size_t	n_eeg_channels;

	int	pagesize_item;

	static const char* const tooltips[2];

	bool suppress_redraw;

	void repaint_score_stats();

      // own widgets
	int construct_widgets();
    public:
	GtkWindow
		*wScoringFacility;
	GtkDialog
		*wFilter,
		*wPattern,
		*wPhaseDiff;
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
		*iSFPageShowDZCDF,
		*iSFPageShowEnvelope;
	GtkMenuItem
		*iSFPageUnfazer,
		*iSFPageSelectionMarkArtifact,
		*iSFPageSelectionClearArtifact,
		*iSFPageSaveAs,
		*iSFPageExportSignal,
		*iSFPageUseThisScale,
		*iSFAcceptAndTakeNext;
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
	GtkToggleButton
		*bScoringFacDrawPower,
		*bScoringFacDrawCrosshair,
		*bScoringFacShowFindDialog,
		*bScoringFacShowPhaseDiffDialog;
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
};





} // namespace sf



// forward declarations of callbacks
extern "C" {

	gboolean da_page_configure_event_cb( GtkWidget *widget, GdkEventConfigure *event, gpointer userdata);
	gboolean da_power_configure_event_cb( GtkWidget *widget, GdkEventConfigure *event, gpointer userdata);
	gboolean da_spectrum_configure_event_cb( GtkWidget *widget, GdkEventConfigure *event, gpointer userdata);
	gboolean da_emg_profile_configure_event_cb( GtkWidget *widget, GdkEventConfigure *event, gpointer userdata);

	gboolean daScoringFacPageView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacPageView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPageView_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
	gboolean daScoringFacPageView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacPSDProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacPSDProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacPSDProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacEMGProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacEMGProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacEMGProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	gboolean daScoringFacSpectrumView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
	gboolean daScoringFacSpectrumView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
	gboolean daScoringFacSpectrumView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

	void eScoringFacPageSize_changed_cb( GtkComboBox *widget, gpointer user_data);
	void eScoringFacCurrentPage_value_changed_cb( GtkSpinButton *spinbutton, gpointer user_data);

	void bScoreClear_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreNREM1_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreNREM2_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreNREM3_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreNREM4_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreREM_clicked_cb  ( GtkButton *button, gpointer user_data);
	void bScoreWake_clicked_cb ( GtkButton *button, gpointer user_data);
	void bScoreMVT_clicked_cb  ( GtkButton *button, gpointer user_data);
	void bSFAccept_clicked_cb( GtkButton *button, gpointer user_data);

	void bScoringFacForward_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoringFacBack_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreGotoPrevUnscored_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreGotoNextUnscored_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreGotoPrevArtifact_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoreGotoNextArtifact_clicked_cb( GtkButton *button, gpointer user_data);
	void bScoringFacDrawPower_toggled_cb( GtkToggleButton *button, gpointer user_data);
	void bScoringFacDrawCrosshair_toggled_cb( GtkToggleButton *button, gpointer user_data);
	void bScoringFacShowFindDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer user_data);
	void bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton, gpointer user_data);

	void mSFPage_show_cb( GtkWidget *widget, gpointer userdata);
	void iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused);
	void iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused);
	void iSFPageShowDZCDF_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused);
	void iSFPageShowEnvelope_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused);
	void iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	void iSFPageUnfazer_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	void iSFPageSaveAs_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	void iSFPageExportSignal_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	void iSFPageUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer user_data);

	void iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	void iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem *menuitem, gpointer user_data);
	//	void iSFPageSelectionInspectMany_activate_cb( GtkMenuItem*, gpointer);
}

} // namespace aghui

#endif

// eof
