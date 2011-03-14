// ;-*-C-*- *  Time-stamp: "2011-03-15 00:25:43 hmmr"
/*
 *       File name:  ui/scoring-facility.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  scoring facility
 *
 *         License:  GPL
 */




#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include <glade/glade.h>
#include <cairo-svg.h>
#include <samplerate.h>

#include "../libagh/iface-glib.h"
#include "../libexstrom/iface.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"
#include "scoring-facility.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


// saved settings

gboolean
	AghUseSigAnOnNonEEGChannels = FALSE;

guint
	AghSFDAPageHeight = 130,
	AghSFDASpectrumWidth = 100,
	AghSFDAPowerProfileHeight = 65,
	AghSFDAEMGProfileHeight = 26;


//

static gboolean
	__use_resample = TRUE;

// widgets

GtkListStore
	*agh_mScoringPageSize;

GtkWidget
	*wScoringFacility,
	*eScoringFacPageSize,
	*eScoringFacCurrentPage,
	*bScoringFacShowFindDialog,
	*bScoringFacShowPhaseDiffDialog;

static GtkWidget
	*cScoringFacPageViews,
	*daScoringFacHypnogram,
	*bScoringFacBack,
	*bScoringFacForward,
	*lScoringFacTotalPages,
	*lScoringFacClockTime,
	*lScoringFacPercentScored,
	*lScoringFacCurrentPos,
	*lScoreStatsNREMPercent,
	*lScoreStatsREMPercent,
	*lScoreStatsWakePercent,
	*lScoringFacCurrentStage,
	*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
	*bScoreREM,   *bScoreWake,  *bScoreMVT,
	*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
	*cSleepStageStats,
	*lScoringFacHint,
	*sbSF,

	*mSFPage, *mSFPageSelection, *mSFPageSelectionInspectChannels,
	*mSFPower, *mSFScore, *mSFSpectrum,
	*iSFPageShowOriginal, *iSFPageShowProcessed, *iSFPageShowDZCDF, *iSFPageShowEnvelope,

	*iSFAcceptAndTakeNext;



GtkWidget
	*bColourNONE,
	*bColourNREM1,
	*bColourNREM2,
	*bColourNREM3,
	*bColourNREM4,
	*bColourREM,
	*bColourWake,
	*bColourPowerSF,
	*bColourEMG,
	*bColourHypnogram,
	*bColourArtifacts,
	*bColourTicksSF,
	*bColourLabelsSF,
	*bColourCursor,

	*bColourBandDelta,
	*bColourBandTheta,
	*bColourBandAlpha,
	*bColourBandBeta,
	*bColourBandGamma;


enum {
	AGH_TIP_GENERAL,
	AGH_TIP_UNFAZER
};
static const char*
	__tooltips[] = {
"<b>Page views:</b>\n"
"	Wheel:		change signal display scale;\n"
"	Ctrl+Wheel:	change scale for all channels;\n"
"	Click2:		reset display scale;\n"
"  <i>in upper half:</i>\n"
"	Click1, move, release:	mark artifact;\n"
"	Click3, move, release:	unmark artifact;\n"
"  <i>in lower half:</i>\n"
"	Click3:		context menu.\n"
"\n"
"<b>Power profile views:</b>\n"
"	Click1:	position cursor;\n"
"	Click2:	draw bands / discrete freq. bins;\n"
"	Click3:	context menu;\n"
"	Wheel:	cycle focused band / in-/decrement freq. range;\n"
"	Shift+Wheel:	in-/decrement scale.\n"
"\n"
"<b>Freq. spectrum view:</b>\n"
"	Click2:	Toggle absolute/relative y-scale;\n"
"	Wheel:	Scale power (when in abs. mode);\n"
"	Shift+Wheel:	In-/decrease freq. range.\n"
"\n"
"<b>Hypnogram:</b>\n"
"	Click1:	position cursor;\n"
"	Click3:	context menu.",

"<b>Unfazer:</b>\n"
"	Wheel:		adjust factor;\n"
"	Click1:		accept;\n"
"	Click2:		reset factor to 1.;\n"
"	Ctrl+Click2:	remove unfazer;\n"
"	Click3:		cancel.\n",
};




static gboolean daScoringFacPageView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
static gboolean daScoringFacPageView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
static gboolean daScoringFacPageView_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
static gboolean daScoringFacPageView_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);
static gboolean daScoringFacPageView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

static gboolean daScoringFacPSDProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
static gboolean daScoringFacPSDProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
static gboolean daScoringFacPSDProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

static gboolean daScoringFacEMGProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
static gboolean daScoringFacEMGProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
static gboolean daScoringFacEMGProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

static gboolean daScoringFacSpectrumView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
static gboolean daScoringFacSpectrumView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
static gboolean daScoringFacSpectrumView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);

static void iSFPageSelectionInspectMany_activate_cb( GtkMenuItem*, gpointer);

gint
agh_ui_construct_ScoringFacility( GladeXML *xml)
{
	GtkCellRenderer *renderer;

	if ( !(wScoringFacility         = glade_xml_get_widget( xml, "wScoringFacility")) ||
	     !(eScoringFacPageSize      = glade_xml_get_widget( xml, "eScoringFacPageSize")) ||
	     !(cScoringFacPageViews     = glade_xml_get_widget( xml, "cScoringFacPageViews")) ||
	     !(daScoringFacHypnogram    = glade_xml_get_widget( xml, "daScoringFacHypnogram")) ||
	     !(bScoringFacBack          = glade_xml_get_widget( xml, "bScoringFacBack")) ||
	     !(bScoringFacForward       = glade_xml_get_widget( xml, "bScoringFacForward")) ||
	     !(lScoringFacTotalPages    = glade_xml_get_widget( xml, "lScoringFacTotalPages")) ||
	     !(eScoringFacCurrentPage   = glade_xml_get_widget( xml, "eScoringFacCurrentPage")) ||
	     !(lScoringFacClockTime	= glade_xml_get_widget( xml, "lScoringFacClockTime")) ||
	     !(lScoringFacCurrentStage  = glade_xml_get_widget( xml, "lScoringFacCurrentStage")) ||
	     !(lScoringFacCurrentPos    = glade_xml_get_widget( xml, "lScoringFacCurrentPos")) ||
	     !(lScoringFacPercentScored = glade_xml_get_widget( xml, "lScoringFacPercentScored")) ||
	     !(lScoreStatsNREMPercent   = glade_xml_get_widget( xml, "lScoreStatsNREMPercent")) ||
	     !(lScoreStatsREMPercent    = glade_xml_get_widget( xml, "lScoreStatsREMPercent")) ||
	     !(lScoreStatsWakePercent   = glade_xml_get_widget( xml, "lScoreStatsWakePercent")) ||
	     !(bScoreClear		= glade_xml_get_widget( xml, "bScoreClear")) ||
	     !(bScoreNREM1		= glade_xml_get_widget( xml, "bScoreNREM1")) ||
	     !(bScoreNREM2		= glade_xml_get_widget( xml, "bScoreNREM2")) ||
	     !(bScoreNREM3		= glade_xml_get_widget( xml, "bScoreNREM3")) ||
	     !(bScoreNREM4		= glade_xml_get_widget( xml, "bScoreNREM4")) ||
	     !(bScoreREM		= glade_xml_get_widget( xml, "bScoreREM"))   ||
	     !(bScoreWake		= glade_xml_get_widget( xml, "bScoreWake"))  ||
	     !(bScoreMVT		= glade_xml_get_widget( xml, "bScoreMVT"))   ||
	     !(bScoreGotoPrevUnscored	= glade_xml_get_widget( xml, "bScoreGotoPrevUnscored")) ||
	     !(bScoreGotoNextUnscored	= glade_xml_get_widget( xml, "bScoreGotoNextUnscored")) ||
	     !(bScoringFacShowFindDialog
	       				= glade_xml_get_widget( xml, "bScoringFacShowFindDialog")) ||
	     !(bScoringFacShowPhaseDiffDialog
	       				= glade_xml_get_widget( xml, "bScoringFacShowPhaseDiffDialog")) ||
	     !(cSleepStageStats		= glade_xml_get_widget( xml, "cSleepStageStats")) ||
	     !(lScoringFacHint    	= glade_xml_get_widget( xml, "lScoringFacHint")) ||
	     !(sbSF			= glade_xml_get_widget( xml, "sbSF")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eScoringFacPageSize),
				 GTK_TREE_MODEL (agh_mScoringPageSize));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer,
					"text", 0,
					NULL);

      // ------- menus
	if ( !(mSFPage			= glade_xml_get_widget( xml, "mSFPage")) ||
	     !(mSFPageSelection		= glade_xml_get_widget( xml, "mSFPageSelection")) ||
	     !(mSFPageSelectionInspectChannels
					= glade_xml_get_widget( xml, "mSFPageSelectionInspectChannels")) ||
	     !(mSFPower			= glade_xml_get_widget( xml, "mSFPower")) ||
	     !(mSFScore			= glade_xml_get_widget( xml, "mSFScore")) ||
	     !(mSFSpectrum		= glade_xml_get_widget( xml, "mSFSpectrum")) ||
	     !(iSFPageShowOriginal	= glade_xml_get_widget( xml, "iSFPageShowOriginal")) ||
	     !(iSFPageShowProcessed	= glade_xml_get_widget( xml, "iSFPageShowProcessed")) ||
	     !(iSFPageShowDZCDF		= glade_xml_get_widget( xml, "iSFPageShowDZCDF")) ||
	     !(iSFPageShowEnvelope	= glade_xml_get_widget( xml, "iSFPageShowEnvelope")) ||
	     !(iSFAcceptAndTakeNext	= glade_xml_get_widget( xml, "iSFAcceptAndTakeNext")) )
		return -1;

      // ------ colours
	if ( !(bColourNONE	= glade_xml_get_widget( xml, "bColourNONE")) ||
	     !(bColourNREM1	= glade_xml_get_widget( xml, "bColourNREM1")) ||
	     !(bColourNREM2	= glade_xml_get_widget( xml, "bColourNREM2")) ||
	     !(bColourNREM3	= glade_xml_get_widget( xml, "bColourNREM3")) ||
	     !(bColourNREM4	= glade_xml_get_widget( xml, "bColourNREM4")) ||
	     !(bColourREM	= glade_xml_get_widget( xml, "bColourREM")) ||
	     !(bColourWake	= glade_xml_get_widget( xml, "bColourWake")) ||
	     !(bColourPowerSF	= glade_xml_get_widget( xml, "bColourPowerSF")) ||
	     !(bColourEMG	= glade_xml_get_widget( xml, "bColourEMG")) ||
	     !(bColourHypnogram	= glade_xml_get_widget( xml, "bColourHypnogram")) ||
	     !(bColourArtifacts	= glade_xml_get_widget( xml, "bColourArtifacts")) ||
	     !(bColourTicksSF	= glade_xml_get_widget( xml, "bColourTicksSF")) ||
	     !(bColourLabelsSF	= glade_xml_get_widget( xml, "bColourLabelsSF")) ||
	     !(bColourCursor	= glade_xml_get_widget( xml, "bColourCursor")) ||
	     !(bColourBandDelta	= glade_xml_get_widget( xml, "bColourBandDelta")) ||
	     !(bColourBandTheta	= glade_xml_get_widget( xml, "bColourBandTheta")) ||
	     !(bColourBandAlpha	= glade_xml_get_widget( xml, "bColourBandAlpha")) ||
	     !(bColourBandBeta	= glade_xml_get_widget( xml, "bColourBandBeta")) ||
	     !(bColourBandGamma	= glade_xml_get_widget( xml, "bColourBandGamma")) )
		return -1;

	return 0;
}

void
agh_ui_destruct_ScoringFacility()
{
}


static inline
float max_fabs( float* signal, size_t x1, size_t x2)
{
	float m = 0.;
	for ( size_t x = x1; x < x2; ++x )
		if ( m < fabs( signal[x]) )
			m = fabs( signal[x]);
	return m;
}



// ---------- data structures


static void
__destroy_ch( struct SChannelPresentation *Ch)
{
	if ( Ch->signal_filtered )   { free( Ch->signal_filtered);   Ch->signal_filtered   = NULL; }
	if ( Ch->signal_original )   { free( Ch->signal_original);   Ch->signal_original   = NULL; }
	if ( Ch->envelope_upper )    { free( Ch->envelope_upper );   Ch->envelope_upper    = NULL; }
	if ( Ch->envelope_lower )    { free( Ch->envelope_lower );   Ch->envelope_lower    = NULL; }
	if ( Ch->signal_course )     { free( Ch->signal_course  );   Ch->signal_course     = NULL; }
	if ( Ch->signal_breadth )    { free( Ch->signal_breadth );   Ch->signal_breadth    = NULL; }
	if ( Ch->signal_dzcdf )      { free( Ch->signal_dzcdf   );   Ch->signal_dzcdf      = NULL; }
	if ( Ch->spectrum )          { free( Ch->spectrum);          Ch->spectrum          = NULL; }
	if ( Ch->unfazers )  	     { free( Ch->unfazers);          Ch->unfazers          = NULL; }
	if ( Ch->artifacts )  	     { free( Ch->artifacts);         Ch->artifacts         = NULL; }
	if ( Ch->power )             { free( Ch->power);             Ch->power             = NULL; }
	if ( Ch->emg_fabs_per_page ) { free( Ch->emg_fabs_per_page); Ch->emg_fabs_per_page = NULL; }
	if ( Ch->power_in_bands ) {
		for ( gushort b = 0; b < Ch->n_bands; ++b )
			free( Ch->power_in_bands[b]);
		free( Ch->power_in_bands);
		Ch->power_in_bands = NULL;
	}
}





size_t	__total_pages,
	__cur_page_app;
static size_t
	__cur_page,
	__cur_pos_hr, __cur_pos_min, __cur_pos_sec,
	__fft_pagesize;

static float
	__sane_signal_display_scale = NAN,
	__sane_power_display_scale = NAN; // 2.5e-5;




struct SChannelPresentation
	*HH = NULL;
TEDFRef	__source_ref;  // the core structures allow for multiple edf
                       // sources providing signals for a single episode;
                       // keeping only one __source_ref here will, then,
                       // read/write scores in this source's histogram;
// -- but it's essentially not a problem since all edf sources will converge
//    to the same .histogram file
static const struct SEDFFile
	*__source_struct;
static time_t	__start_time;
static char	*__hypnogram;





static void
__queue_redraw_all()
{
	for ( size_t h = 0; h < __n_all_channels; ++h ) {
		if ( HH[h].power ) {
			gtk_widget_queue_draw( HH[h].da_power);
			gtk_widget_queue_draw( HH[h].da_spectrum);
		}
	}
}





guint	__pagesize_item = 4;  // pagesize as currently displayed






static void
__calculate_dirty_percent( struct SChannelPresentation *Ch)
{
	size_t dirty_smpl = 0;
	size_t i, a;
	for ( i = a = 0; i < Ch->n_artifacts; ++i, a += 2 )
		dirty_smpl += (Ch->artifacts[a+1] - Ch->artifacts[a]);
	Ch->dirty_percent = (float) dirty_smpl / Ch->n_samples * 100;
}

static void
__displayscale_fname_in_buf()
{
	char *fname_proper = strrchr( __source_struct->filename, '/') + 1;
	snprintf_buf( "%.*s/.%s.displayscale",
		      (int)(fname_proper - __source_struct->filename - 1), __source_struct->filename,
		      fname_proper);
}


static gboolean __have_unsaved_scores;



#define BUF_ON_STATUS_BAR \
	{ \
	gtk_statusbar_pop( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General); \
	gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General, __buf__); \
	while ( gtk_events_pending() )					\
	 	gtk_main_iteration(); \
	}
#define CH_IS_EXPANDED \
	gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander))
static gboolean __suppress_redraw;


static void __repaint_score_stats();


const struct SSubject *__our_j;
const char *__our_d, *__our_e,
	*__our_e_next;

void
__setup_for_next_e( const struct SSubject *_j, const char *_d, const char *_e)
{
	__our_e_next = NULL;
	for ( size_t d = 0; d < _j->n_sessions; ++d )
		if ( strcmp( _j->sessions[d].name, _d) == 0 )
			for ( size_t e = 0; e < _j->sessions[d].n_episodes; ++e )
				if ( strcmp( _j->sessions[d].episodes[e].name, _e) == 0 )
					if ( e+1 < _j->sessions[d].n_episodes ) {
						__our_e_next = _j->sessions[d].episodes[e+1].name;
						goto out;
					}
out:
	gtk_widget_set_sensitive( iSFAcceptAndTakeNext, __our_e_next != NULL);
}

size_t	__n_all_channels,
	__n_eeg_channels;

gboolean
agh_prepare_scoring_facility( const struct SSubject *_j,
			      const char *_d,
			      const char *_e)
{
	set_cursor_busy( TRUE, wMainWindow);

      // copy arguments into our private variables
	__our_j = _j, __our_d = _d, __our_e = _e;

      // clean up after previous viewing
	gtk_container_foreach( GTK_CONTAINER (cScoringFacPageViews),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	gtk_container_foreach( GTK_CONTAINER (mSFPageSelectionInspectChannels),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	size_t	h, H;
      // iterate all of AghHH, mark our channels
	for ( h = H = 0; H < AghHs; ++H ) {
		TRecRef rec_ref = agh_msmt_find_by_jdeh( _j->name, _d, _e, AghHH[H]);
		if ( rec_ref ) {
			assert (HH = realloc( HH, sizeof(struct SChannelPresentation) * (h+1)));
			HH[h++].rec_ref = rec_ref;
		}
	}
	__n_all_channels = h;
	if ( __n_all_channels == 0 )
		return FALSE;

	__n_eeg_channels = 0;

	for ( h = 0; h < __n_all_channels; ++h ) {
		HH[h].name = agh_msmt_get_signal_name( HH[h].rec_ref);
		HH[h].type = agh_msmt_get_signal_type( HH[h].rec_ref);

		if ( h == 0 ) {
			__source_ref = agh_msmt_get_source( HH[h].rec_ref);
			__source_struct = agh_edf_get_info_from_sourceref( __source_ref, NULL);
			__start_time = __source_struct->start_time;

		      // get scores
			__total_pages =
				agh_edf_get_scores( __source_ref,
						    &__hypnogram, &__fft_pagesize);
			__have_unsaved_scores = FALSE;

		      // get display scales
			__displayscale_fname_in_buf();
			FILE *fd = fopen( __buf__, "r");
			if ( !fd || fscanf( fd, "%g %g", &__sane_signal_display_scale, &__sane_power_display_scale) != 2 )
				__sane_signal_display_scale = __sane_power_display_scale = NAN;
			if ( fd )
				fclose( fd);
		}

	      // get signal data
		snprintf_buf( "(%zu/%zu) %s: read edf...", h+1, __n_all_channels, HH[h].name);
		BUF_ON_STATUS_BAR;

		HH[h].n_samples =
			agh_msmt_get_signal_original_as_float( HH[h].rec_ref,
							       &HH[h].signal_original, &HH[h].samplerate, NULL);
		if ( HH[h].n_samples ) {
			agh_msmt_get_signal_filtered_as_float( HH[h].rec_ref,
							       &HH[h].signal_filtered, NULL, NULL);

			if ( AghUseSigAnOnNonEEGChannels || strcmp( HH[h].type, "EEG") == 0 ) {
				// and signal course
				snprintf_buf( "(%zu/%zu) %s: low-pass...", h+1, __n_all_channels, HH[h].name);
				BUF_ON_STATUS_BAR;
				exstrom_low_pass( HH[h].signal_filtered, HH[h].n_samples, HH[h].samplerate,
						  HH[h].bwf_cutoff = AghBWFCutoff, HH[h].bwf_order = AghBWFOrder, 1,
						  &HH[h].signal_course);

				// and envelope and breadth
				snprintf_buf( "(%zu/%zu) %s: envelope...", h+1, __n_all_channels, HH[h].name);
				BUF_ON_STATUS_BAR;
				signal_envelope( HH[h].signal_filtered, HH[h].n_samples, HH[h].samplerate,
						 &HH[h].envelope_lower,
						 &HH[h].envelope_upper,
						 HH[h].env_tightness = AghEnvTightness,
						 &HH[h].signal_breadth);

				// and dzcdf
				snprintf_buf( "(%zu/%zu) %s: zerocrossings...", h+1, __n_all_channels, HH[h].name);
				BUF_ON_STATUS_BAR;
				signal_dzcdf( HH[h].signal_filtered, HH[h].n_samples, HH[h].samplerate,
					      HH[h].dzcdf_step = AghDZCDFStep,
					      HH[h].dzcdf_sigma = AghDZCDFSigma,
					      HH[h].dzcdf_smooth = AghDZCDFSmooth,
					      &HH[h].signal_dzcdf);
			} else
				HH[h].signal_course =
					HH[h].envelope_upper = HH[h].envelope_lower = HH[h].signal_breadth =
					HH[h].signal_dzcdf = NULL;

		      // artifacts
			HH[h].n_artifacts =
				agh_edf_get_artifacts( __source_ref, HH[h].name,
						       &HH[h].artifacts);

		      // unfazers
			HH[h].n_unfazers =
				agh_edf_get_unfazers( __source_ref,
						      HH[h].name,
						      &HH[h].unfazers);
		      // filters
			HH[h].low_pass_cutoff  = agh_edf_get_lowpass_cutoff ( __source_ref, HH[h].name);
			HH[h].low_pass_order   = agh_edf_get_lowpass_order  ( __source_ref, HH[h].name);
			HH[h].high_pass_cutoff = agh_edf_get_highpass_cutoff( __source_ref, HH[h].name);
			HH[h].high_pass_order  = agh_edf_get_highpass_order ( __source_ref, HH[h].name);


		      // expander and vbox
			gchar *h_escaped = g_markup_escape_text( HH[h].name, -1);
			snprintf_buf( "%s <b>%s</b>", HH[h].type, h_escaped);
			g_free( h_escaped);
			HH[h].expander = gtk_expander_new( __buf__);
			gtk_expander_set_use_markup( GTK_EXPANDER (HH[h].expander), TRUE);

			gtk_box_pack_start( GTK_BOX (cScoringFacPageViews),
					    HH[h].expander, TRUE, TRUE, 0);
			gtk_expander_set_expanded( GTK_EXPANDER (HH[h].expander),
						   TRUE);
			gtk_container_add( GTK_CONTAINER (HH[h].expander),
					   HH[h].vbox = gtk_vbox_new( FALSE, 0));

		      // set up page view
			HH[h].signal_display_scale =
				isfinite( __sane_signal_display_scale)
				? __sane_signal_display_scale
				: __calibrate_display_scale( HH[h].signal_filtered, APSZ * HH[h].samplerate * MIN (__total_pages, 10),
							     AghSFDAPageHeight / 2);

			gtk_container_add( GTK_CONTAINER (HH[h].vbox),
					   HH[h].da_page = gtk_drawing_area_new());
			g_object_set( G_OBJECT (HH[h].da_page),
				      "app-paintable", TRUE,
				      "height-request", AghSFDAPageHeight,
				      NULL);
			g_signal_connect_after( HH[h].da_page, "expose-event",
						G_CALLBACK (daScoringFacPageView_expose_event_cb),
						(gpointer)&HH[h]);
			g_signal_connect_after( HH[h].da_page, "button-press-event",
						G_CALLBACK (daScoringFacPageView_button_press_event_cb),
						(gpointer)&HH[h]);
			g_signal_connect_after( HH[h].da_page, "button-release-event",
						G_CALLBACK (daScoringFacPageView_button_release_event_cb),
						(gpointer)&HH[h]);
			g_signal_connect_after( HH[h].da_page, "motion-notify-event",
						G_CALLBACK (daScoringFacPageView_motion_notify_event_cb),
						(gpointer)&HH[h]);
			g_signal_connect_after( HH[h].da_page, "scroll-event",
						G_CALLBACK (daScoringFacPageView_scroll_event_cb),
						(gpointer)&HH[h]);
			gtk_widget_add_events( HH[h].da_page,
					       (GdkEventMask)
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_DRAG_MOTION);

		      // set up PSD profile view
			if ( agh_signal_type_is_fftable( HH[h].type) ) {

				++__n_eeg_channels;

				snprintf_buf( "(%zu/%zu) %s: power...", h+1, __n_all_channels, HH[h].name);
				BUF_ON_STATUS_BAR;

				// power in a single bin
				// the first call to get power course is *_as_float; others will use *_direct
				assert ( __total_pages == agh_msmt_get_power_course_in_range_as_float(
						 HH[h].rec_ref, HH[h].from, HH[h].upto, &HH[h].power) );
				// power spectrum (for the first page)
				HH[h].n_bins = HH[h].last_spectrum_bin =
					agh_msmt_get_power_spectrum_as_float( HH[h].rec_ref, 0,
									      &HH[h].spectrum, &HH[h].spectrum_max);
				// will be reassigned in REDRAW_ALL
				HH[h].spectrum_upper_freq =
					HH[h].n_bins * (HH[h].binsize = agh_msmt_get_binsize( HH[h].rec_ref));

				// power in bands
				HH[h].n_bands = 0;
				while ( HH[h].n_bands < AGH_BAND__TOTAL )
					if ( AghFreqBands[ HH[h].n_bands ][0] >= HH[h].spectrum_upper_freq )
						break;
					else
						++HH[h].n_bands;
				HH[h].uppermost_band = HH[h].n_bands-1;
				assert (HH[h].power_in_bands = malloc( sizeof(float*) * HH[h].n_bands));
				for ( gushort b = 0; b < HH[h].n_bands; ++b )
					agh_msmt_get_power_course_in_range_as_float( HH[h].rec_ref,
										     AghFreqBands[b][0], AghFreqBands[b][1],
										     &HH[h].power_in_bands[b]);

				// delta comes first, calibrate display scale against it
				HH[h].power_display_scale =
					isfinite( __sane_power_display_scale)
					? __sane_power_display_scale
					: __calibrate_display_scale( HH[h].power_in_bands[0],
								     __total_pages,
								     AghSFDAPageHeight);

				// switches
				HH[h].draw_spectrum_absolute = TRUE;
				HH[h].draw_bands = TRUE;
				HH[h].focused_band = 0; // delta

				__calculate_dirty_percent( &HH[h]);

				HH[h].emg_fabs_per_page = NULL;

				HH[h].from = AghOperatingRangeFrom, HH[h].upto = AghOperatingRangeUpto;

				GtkWidget *hbox;
				gtk_container_add( GTK_CONTAINER (HH[h].vbox),
						   hbox = gtk_hbox_new( FALSE, 0));
				gtk_container_add( GTK_CONTAINER (hbox),
						   HH[h].da_power = gtk_drawing_area_new());
				gtk_container_add_with_properties( GTK_CONTAINER (hbox),
								   HH[h].da_spectrum = gtk_drawing_area_new(),
								   "expand", FALSE,
								   NULL);
			      // profile pane
				g_object_set( G_OBJECT (HH[h].da_power),
					      "app-paintable", TRUE,
					      "height-request", AghSFDAPowerProfileHeight,
					      NULL);
				g_signal_connect_after( HH[h].da_power, "expose-event",
							G_CALLBACK (daScoringFacPSDProfileView_expose_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_power, "button-press-event",
							G_CALLBACK (daScoringFacPSDProfileView_button_press_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_power, "scroll-event",
							G_CALLBACK (daScoringFacPSDProfileView_scroll_event_cb),
							(gpointer)&HH[h]);
				gtk_widget_add_events( HH[h].da_power,
						       (GdkEventMask) GDK_BUTTON_PRESS_MASK);

			      // spectrum pane
				g_object_set( G_OBJECT (HH[h].da_spectrum),
					      "app-paintable", TRUE,
					      "width-request", AghSFDASpectrumWidth,
					      NULL);
				gtk_widget_modify_fg( HH[h].da_spectrum, GTK_STATE_NORMAL, &__fg1__[cSPECTRUM]);
				gtk_widget_modify_bg( HH[h].da_spectrum, GTK_STATE_NORMAL, &__bg1__[cSPECTRUM]);

				g_signal_connect_after( HH[h].da_spectrum, "expose-event",
							G_CALLBACK (daScoringFacSpectrumView_expose_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_spectrum, "button-press-event",
							G_CALLBACK (daScoringFacSpectrumView_button_press_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_spectrum, "scroll-event",
							G_CALLBACK (daScoringFacSpectrumView_scroll_event_cb),
							(gpointer)&HH[h]);
				gtk_widget_add_events( HH[h].da_spectrum, (GdkEventMask) GDK_BUTTON_PRESS_MASK);

			} else {
				HH[h].power = HH[h].spectrum = NULL;
				HH[h].power_in_bands = NULL;
				HH[h].da_power = HH[h].da_spectrum = NULL;
			}

			if ( strcmp( HH[h].type, "EMG") == 0 ) {
				assert (HH[h].emg_fabs_per_page = malloc( sizeof(float) * (__total_pages+1)));
				float largest = 0.;
				size_t i;
				snprintf_buf( "(%zu/%zu) %s: EMG...", h+1, __n_all_channels, HH[h].name);
				BUF_ON_STATUS_BAR;
				for ( i = 0; i < __total_pages; ++i ) {
					float	current = HH[h].emg_fabs_per_page[i]
						= max_fabs( HH[h].signal_original, i * PSZ * HH[h].samplerate, (i+1) * PSZ * HH[h].samplerate);
					if ( largest < current )
						largest = current;
				}

				HH[h].emg_fabs_per_page[i]  // last page, likely incomplete
					= max_fabs( HH[h].signal_original, i * PSZ * HH[h].samplerate, HH[h].n_samples);
				HH[h].emg_scale = AghSFDAEMGProfileHeight/2 / largest;

				GtkWidget *hbox, *da_void;
				gtk_container_add( GTK_CONTAINER (HH[h].vbox),
						   hbox = gtk_hbox_new( FALSE, 0));
				gtk_container_add( GTK_CONTAINER (hbox),
						   HH[h].da_emg_profile = gtk_drawing_area_new());
				gtk_container_add_with_properties( GTK_CONTAINER (hbox),
								   da_void = gtk_drawing_area_new(),
								   "expand", FALSE,
								   NULL);
				g_object_set( G_OBJECT (HH[h].da_emg_profile),
					      "app-paintable", TRUE,
					      "height-request", AghSFDAEMGProfileHeight,
					      NULL);
				g_object_set( G_OBJECT (da_void),
					      "width-request", AghSFDASpectrumWidth,
					      NULL);
				g_signal_connect_after( HH[h].da_emg_profile, "expose-event",
							G_CALLBACK (daScoringFacEMGProfileView_expose_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_emg_profile, "button-press-event",
							G_CALLBACK (daScoringFacEMGProfileView_button_press_event_cb),
							(gpointer)&HH[h]);
				g_signal_connect_after( HH[h].da_emg_profile, "scroll-event",
							G_CALLBACK (daScoringFacEMGProfileView_scroll_event_cb),
							(gpointer)&HH[h]);
				gtk_widget_add_events( HH[h].da_emg_profile,
						       (GdkEventMask) GDK_BUTTON_PRESS_MASK);
			} else {
				HH[h].emg_fabs_per_page = NULL;
				HH[h].da_emg_profile = NULL;
			}

			HH[h].draw_processed_signal = TRUE;
			HH[h].draw_original_signal = FALSE;
			HH[h].draw_dzcdf = FALSE;
			HH[h].draw_envelope = FALSE;


		      // add channel under mSFPageSelectionInspectChannels
			gtk_container_add( GTK_CONTAINER (mSFPageSelectionInspectChannels),
					   HH[h].menu_item = gtk_check_menu_item_new_with_label( HH[h].name));
			g_object_set( G_OBJECT (HH[h].menu_item),
				      "visible", TRUE,
				      NULL);
		} else
			HH[h].da_page = NULL;
	}

      // finish mSFPageSelectionInspectChannels
	GtkWidget *iSFPageSelectionInspectMany = gtk_menu_item_new_with_label( "Inspect these");
	gtk_container_add( GTK_CONTAINER (mSFPageSelectionInspectChannels),
			   iSFPageSelectionInspectMany);
	g_object_set( G_OBJECT (iSFPageSelectionInspectMany),
		      "visible", TRUE,
		      NULL);
	g_signal_connect_after( iSFPageSelectionInspectMany, "select",  // but why the hell not "activate"?? GTK+ <3<3<#<#,3,3
				G_CALLBACK (iSFPageSelectionInspectMany_activate_cb),
				NULL);

	gtk_statusbar_pop( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General);
	set_cursor_busy( FALSE, wMainWindow);

      // recalculate (average) signal and power display scales
	if ( isfinite( __sane_signal_display_scale) ) {
		;  // we've got it saved previously
	} else {
		__sane_signal_display_scale = __sane_power_display_scale = 0.;
		size_t n_with_power = 0;
		for ( h = 0; h < __n_all_channels; ++h ) {
			__sane_signal_display_scale += HH[h].signal_display_scale;
			if ( HH[h].power ) {
				++n_with_power;
				__sane_power_display_scale += HH[h].power_display_scale;
			}
		}
		__sane_signal_display_scale /= __n_all_channels;
		__sane_power_display_scale /= n_with_power;
		for ( h = 0; h < __n_all_channels; ++h ) {
			HH[h].signal_display_scale = __sane_signal_display_scale;
			if ( HH[h].power )
				HH[h].power_display_scale = __sane_power_display_scale;
		}
	}


      // set up other controls
	// set window title
	snprintf_buf( "Scoring: %s\342\200\231s %s in %s",
		      _j->name, _e, _d);
	gtk_window_set_title( GTK_WINDOW (wScoringFacility),
			      __buf__);

	// assign tooltip
	gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_GENERAL]);

	// align empty area next to EMG profile with spectrum panes vertically
	g_object_set( G_OBJECT (cSleepStageStats),
		      "width-request", AghSFDASpectrumWidth,
		      NULL);

	// grey out phasediff button if there are fewer than 2 EEG channels
	gtk_widget_set_sensitive( bScoringFacShowPhaseDiffDialog, (__n_eeg_channels >= 2));

	// desensitize iSFAcceptAndTakeNext unless there are more episodes
	__setup_for_next_e( _j, _d, _e);

	// draw all
	__suppress_redraw = TRUE;
	__cur_page_app = __cur_page = 0;
	gtk_combo_box_set_active( GTK_COMBO_BOX (eScoringFacPageSize),
				  __pagesize_item = AghDisplayPageSizeItem);

	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   1);
	__suppress_redraw = FALSE;
	g_signal_emit_by_name( eScoringFacPageSize, "changed");
//	gtk_widget_queue_draw( cMeasurements);

	__repaint_score_stats();

	return TRUE;
}


static void
agh_cleanup_scoring_facility()
{
      // clean up
	__clicked_channel = NULL;

	if ( __pattern.data ) {
		free( __pattern.data);
		__pattern.data = NULL;
	}

	// save display scales
	__displayscale_fname_in_buf();
	FILE *fd = fopen( __buf__, "w");
	if ( fd ) {
		fprintf( fd, "%g %g", __sane_signal_display_scale, __sane_power_display_scale);
		fclose( fd);
	}

	for ( size_t h = 0; h < __n_all_channels; ++h )
		__destroy_ch( &HH[h]);
	free( HH);
	HH = NULL;
	free( __hypnogram);
	__hypnogram = NULL;
	__source_ref = NULL;
}








// -------------------- Page


float
__calibrate_display_scale( const float *signal, size_t over, float fit)
{
	float max_over = 0.;
	for ( size_t i = 0; i < over; ++i )
		if ( max_over < signal[i] )
			max_over = signal[i];
	return fit / max_over;
}

void
__draw_signal( float *signal, size_t n_samples, float scale,
	       unsigned width, unsigned vdisp,
	       cairo_t *cr, gboolean use_resample)
{
	if ( use_resample ) {
		SRC_DATA samples;
		samples.input_frames = n_samples;
		samples.data_in = signal;
		samples.data_out = (float*)malloc( (samples.output_frames = width) * sizeof(float));
		samples.src_ratio = (double)samples.output_frames / samples.input_frames;
		if ( src_simple( &samples, SRC_SINC_FASTEST /*SRC_LINEAR*/, 1) )
			;
		guint i;
		cairo_move_to( cr, 0,
			       - samples.data_out[0]
			       * scale
			       + vdisp);
		for ( i = 0; i < width; ++i ) {
			cairo_line_to( cr, i,
				       - samples.data_out[i]
				       * scale
				       + vdisp);
		}

		free( (void*)samples.data_out);

	} else {
		guint i;
		cairo_move_to( cr, 0,
			       - signal[0]
			       * scale
			       + vdisp);
		for ( i = 0; i < n_samples; ++i ) {
			cairo_line_to( cr, (float)(i)/n_samples * width,
				       - signal[i]
				       * scale
				       + vdisp);
		}
	}
}









struct SChannelPresentation  // for menus & everything else
	*__clicked_channel = NULL;

// general marquee
GtkWidget *__marking_in_widget;
double __marquee_start, __marquee_virtual_end;






size_t
__marquee_to_az()
{
	int wd;
	gdk_drawable_get_size( __clicked_channel->da_page->window, &wd, NULL);
	float	x1 = __marquee_start,
		x2 = __marquee_virtual_end;
	if ( x1 > x2 ) { float _ = x1; x1 = x2, x2 = _; }
	if ( x1 < 0. ) x1 = 0.;

	__pattern_ia = (__cur_page_app + x1/wd) * APSZ * __clicked_channel->samplerate;
	__pattern_iz = (__cur_page_app + x2/wd) * APSZ * __clicked_channel->samplerate;
	if ( __pattern_ia > __clicked_channel->n_samples )
		return 0;
	if ( __pattern_iz > __clicked_channel->n_samples )
		__pattern_iz = __clicked_channel->n_samples;

	__pattern_wd = (float)(__pattern_iz - __pattern_ia)/(__clicked_channel->samplerate * APSZ) * wd;

	return (__pattern_iz - __pattern_ia);
}




static void
__mark_region_as_artifact( gchar value)
{
#define Ch __clicked_channel

	if ( __marquee_to_az() == 0 )
		return;

	(value == 'x' ? agh_edf_mark_artifact : agh_edf_clear_artifact)( __source_ref, Ch->name,
									 __pattern_ia, __pattern_iz);
	free( (void*)Ch->artifacts);
	Ch->n_artifacts = agh_edf_get_artifacts( __source_ref, Ch->name,
						 &Ch->artifacts);
	__calculate_dirty_percent( Ch);

	agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
					       &Ch->signal_filtered, NULL, NULL);

	if ( Ch->power ) {
		agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
								    Ch->from, Ch->upto,
								    Ch->power);
		for ( gushort b = 0; b <= Ch->uppermost_band; ++b )
			agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
									    AghFreqBands[b][0], AghFreqBands[b][1],
									    Ch->power_in_bands[b]);

		agh_msmt_get_power_spectrum_as_float( Ch->rec_ref, __cur_page,
						      &Ch->spectrum, &Ch->spectrum_max);

		gtk_widget_queue_draw( Ch->da_power);
		gtk_widget_queue_draw( Ch->da_spectrum);
	}
	gtk_widget_queue_draw( Ch->da_page);
#undef Ch
}









enum {
	SEL_UNF_CHANNEL = 1,
	SEL_UNF_CALIBRATE = 2,
};
static gint __select_state = 0;


static struct SChannelPresentation
	*__unfazer_offending_channel;
static float
	__unfazer_factor = 0.1;



static guint __crosshair_at;
static gboolean __draw_crosshair = FALSE;

static gboolean __draw_power = TRUE;














static short int
	__cur_stage;


char *__score_names[] = {
	"blank",
	"NREM1", "NREM2", "NREM3", "NREM4",
	"REM",
	"Wake",
	"MVT"
};



static guint __score_hypn_depth[8] = {
	0, 20, 23, 30, 33, 5, 10, 1
};




// these __percent_* functions operate on a yet unsaved scores, hence
// the duplication of core functions
static float
__percent_scored()
{
	size_t	scored_pages = 0,
		p;
	for ( p = 0; p < __total_pages; ++p )
		scored_pages += (__hypnogram[p] != AghScoreCodes[AGH_SCORE_NONE]);
	return (gfloat) scored_pages / p * 100;
}

static float
__percent_NREM()
{
	size_t	nrem_pages = 0,
		p;
	for ( p = 0; p < __total_pages; ++p )
		nrem_pages += (__hypnogram[p] == AghScoreCodes[AGH_SCORE_NREM1] ||
			       __hypnogram[p] == AghScoreCodes[AGH_SCORE_NREM2] ||
			       __hypnogram[p] == AghScoreCodes[AGH_SCORE_NREM3] ||
			       __hypnogram[p] == AghScoreCodes[AGH_SCORE_NREM4] );
	return (float) nrem_pages / p * 100;
}

static float
__percent_REM()
{
	size_t	rem_pages = 0,
		p;
	for ( p = 0; p < __total_pages; ++p )
		rem_pages += (__hypnogram[p] == AghScoreCodes[AGH_SCORE_REM] );
	return (float) rem_pages / p * 100;
}

static float
__percent_Wake()
{
	size_t	wake_pages = 0,
		p;
	for ( p = 0; p < __total_pages; ++p )
		wake_pages += (__hypnogram[p] == AghScoreCodes[AGH_SCORE_WAKE]);
	return (float) wake_pages / p * 100;
}




static guint __pagesize_ticks[] = {
	5, 5, 3, 4, 6, 12, 24, 30
};




static void
__draw_page( cairo_t *cr, const struct SChannelPresentation *Ch, guint wd, guint ht,
	     gboolean draw_marquee)
{
	guint i;

	double signal_r, signal_g, signal_b;
      // background
	guint c = PS_IS_RIGHT ? SCOREID( __hypnogram[__cur_page]) : cSIGNAL_SCORE_NONE;
	if ( Ch->power ) {
		cairo_pattern_t *cp = cairo_pattern_create_linear( 0., 0., 0., ht);
		cairo_pattern_add_color_stop_rgba( cp, 0.,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536,
						   1.);
		cairo_pattern_add_color_stop_rgba( cp, .5,
						   (double)__bg1__[c].red/65536,
						   (double)__bg1__[c].green/65536,
						   (double)__bg1__[c].blue/65536,
						   .5);
		cairo_pattern_add_color_stop_rgba( cp, 1.,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
						   (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536,
						   1.);
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);
		cairo_stroke( cr);
		cairo_pattern_destroy( cp);

		signal_r = (double)__fg1__[c].red/65536;
		signal_g = (double)__fg1__[c].green/65536;
		signal_b = (double)__fg1__[c].blue/65536;

	      // preceding and next score-coloured fade-in and -out
		if ( PS_IS_RIGHT ) {
			int pattern_set = 0;
			guint c2;
			cp = cairo_pattern_create_linear( 0., 0., wd, 0.);
			if ( __cur_page > 0 && (c2 = SCOREID( __hypnogram[__cur_page-1])) != c && c2 != AGH_SCORE_NONE ) {
				pattern_set = 1;
				cairo_pattern_add_color_stop_rgba( cp, 0.,
								   (double)__bg1__[c2].red/65536,
								   (double)__bg1__[c2].green/65536,
								   (double)__bg1__[c2].blue/65536,
								   .7);
				cairo_pattern_add_color_stop_rgba( cp, 50./wd,
								   (double)__bg1__[c2].red/65536,
								   (double)__bg1__[c2].green/65536,
								   (double)__bg1__[c2].blue/65536,
								   0.);
			}
			if ( __cur_page < __total_pages-1 && (c2 = SCOREID( __hypnogram[__cur_page+1])) != c && c2 != AGH_SCORE_NONE ) {
				pattern_set = 1;
				cairo_pattern_add_color_stop_rgba( cp, 1. - 50./wd,
								   (double)__bg1__[c2].red/65536,
								   (double)__bg1__[c2].green/65536,
								   (double)__bg1__[c2].blue/65536,
								   0.);
				cairo_pattern_add_color_stop_rgba( cp, 1.,
								   (double)__bg1__[c2].red/65536,
								   (double)__bg1__[c2].green/65536,
								   (double)__bg1__[c2].blue/65536,
								   .7);
			}
			if ( pattern_set ) {
				cairo_set_source( cr, cp);
				cairo_rectangle( cr, 0., 0., wd, ht);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
			cairo_pattern_destroy( cp);
		}
	} else {
		cairo_set_source_rgba( cr,
				       (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
				       (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
				       (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536,
				       .7);
		cairo_rectangle( cr, 0., 0., wd, ht);
		cairo_fill( cr);
		cairo_stroke( cr);

		signal_r = (double)__fg1__[cSIGNAL_SCORE_NONE].red/65536;
		signal_g = (double)__fg1__[cSIGNAL_SCORE_NONE].green/65536;
		signal_b = (double)__fg1__[cSIGNAL_SCORE_NONE].blue/65536;
	}


      // waveform: signal_filtered
	gboolean one_signal_drawn = FALSE;
	if ( (Ch->draw_processed_signal && __select_state == 0) ||
	     (Ch->draw_processed_signal && Ch != __clicked_channel) ) {  // only show processed signal when done with unfazing
		cairo_set_line_width( cr, .5);
		cairo_set_source_rgb( cr, signal_r, signal_g, signal_b);
		__draw_signal( &Ch->signal_filtered[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-120, 15);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "filt");
		cairo_show_text( cr, __buf__);
		one_signal_drawn = TRUE;
		cairo_stroke( cr);
	}

      // waveform: signal_original
	if ( Ch->draw_original_signal ||
	     (__select_state == SEL_UNF_CHANNEL && Ch == __clicked_channel) ) {
		if ( one_signal_drawn ) {  // attenuate the other signal
			cairo_set_line_width( cr, .3);
			cairo_set_source_rgba( cr, signal_r, signal_g, signal_b, .4);
		} else {
			cairo_set_line_width( cr, .5);
			cairo_set_source_rgb( cr, signal_r, signal_g, signal_b);
		}
		__draw_signal( &Ch->signal_original[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-120, 25);
		cairo_set_font_size( cr, 10);
		snprintf_buf( "orig");
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}


      // dzcdf
	if ( Ch->signal_dzcdf && Ch->draw_dzcdf ) {
		float	dzcdf_display_scale,
			avg = 0.;
		for ( size_t i = __cur_page_app * Ch->samplerate * APSZ; i < (__cur_page_app+1) * Ch->samplerate * APSZ; ++i )
			avg += Ch->signal_dzcdf[i];
		avg /= (Ch->samplerate * APSZ);
		dzcdf_display_scale = ht/3 / avg;

		cairo_set_source_rgba( cr, .1, .7, .2, .2);
		cairo_set_line_width( cr, 1.5);
		__draw_signal( &Ch->signal_dzcdf[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       dzcdf_display_scale,
			       wd, ht-5, cr, __use_resample);
		cairo_stroke( cr);

		cairo_rectangle( cr, 0, ht-10, wd, ht-9);
		cairo_stroke( cr);

		// scale
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_set_line_width( cr, 1.5);
		cairo_move_to( cr, 20, ht-10);
		cairo_line_to( cr, 20, ht-10 - dzcdf_display_scale);
		cairo_stroke( cr);
	}

      // envelope
	if ( Ch->signal_breadth && Ch->draw_envelope ) {
		cairo_set_source_rgba( cr, .9, .1, .1, .4);
		cairo_set_line_width( cr, .3);

		__draw_signal( &Ch->envelope_upper[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);
		__draw_signal( &Ch->envelope_lower[__cur_page_app * Ch->samplerate * APSZ], Ch->samplerate * APSZ,
			       Ch->signal_display_scale,
			       wd, ht/2, cr, __use_resample);

		cairo_stroke( cr);
	}

      // artifacts (changed bg)
	if ( Ch->n_artifacts ) {
		size_t	lpp = APSZ * Ch->samplerate,
			cur_page_start_s =  __cur_page_app      * lpp,
			cur_page_end_s   = (__cur_page_app + 1) * lpp;
		for ( size_t a = 0; a < Ch->n_artifacts; ++a ) {
			if ( (Ch->artifacts[a*2  ] > cur_page_start_s && Ch->artifacts[a*2  ] < cur_page_end_s) ||
			     (Ch->artifacts[a*2+1] > cur_page_start_s && Ch->artifacts[a*2+1] < cur_page_end_s) ) {
				size_t	aa = (Ch->artifacts[a*2  ] < cur_page_start_s) ? cur_page_start_s : Ch->artifacts[a*2  ],
					az = (Ch->artifacts[a*2+1] > cur_page_end_s  ) ? cur_page_end_s   : Ch->artifacts[a*2+1];
				cairo_set_source_rgba( cr,  // do some gradients perhaps?
						       (double)__fg1__[cARTIFACT].red/65536,
						       (double)__fg1__[cARTIFACT].green/65536,
						       (double)__fg1__[cARTIFACT].blue/65536,
						       .5);
				cairo_rectangle( cr,
						 (float)( aa       % lpp) / lpp * wd, ht*1./3,
						 (float)((az - aa) % lpp) / lpp * wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( Ch->artifacts[a*2  ] <= cur_page_start_s && Ch->artifacts[a*2+1] >= cur_page_end_s ) {
				cairo_set_source_rgba( cr,  // flush solid (artifact covering all page)
						       (double)__fg1__[cARTIFACT].red/65536,
						       (double)__fg1__[cARTIFACT].green/65536,
						       (double)__fg1__[cARTIFACT].blue/65536,
						       .5);
				cairo_rectangle( cr,
						 0, ht*1./3, wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			} else if ( Ch->artifacts[a*2] > cur_page_end_s )  // no more artifacts up to and on current page
				break;
		}
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, wd-70, ht-15);
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f %% dirty", Ch->dirty_percent);
		cairo_show_text( cr, __buf__);
	}


      // ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);

	{
		cairo_set_font_size( cr, 9);
		cairo_set_line_width( cr, .3);
		for ( i = 0; i < __pagesize_ticks[__pagesize_item]; ++i ) {
			guint tick_pos = i * APSZ / __pagesize_ticks[__pagesize_item];
			cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item], 0);
			cairo_line_to( cr, i * wd / __pagesize_ticks[__pagesize_item], ht);

			cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item] + 5, ht-2);
			snprintf_buf_ts_s( tick_pos);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}


      // labels of all kinds
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cLABELS_SF].red/65536,
			      (double)__fg1__[cLABELS_SF].green/65536,
			      (double)__fg1__[cLABELS_SF].blue/65536);
      // unfazer info
	if ( Ch->n_unfazers ) {
		g_string_assign( __ss__, "Unf: ");
		for ( i = 0; i < Ch->n_unfazers; ++i ) {
			g_string_append_printf( __ss__, "%s: %5.3f%c",
						Ch->unfazers[i].channel, Ch->unfazers[i].factor,
						(i+1 == Ch->n_unfazers) ? ' ' : ';');
		}
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 10, ht-4);
		cairo_show_text( cr, __ss__->str);
	}

      // uV scale
	{
		cairo_set_source_rgb( cr, 0., 0., 0.);
		guint dpuV = 1 * Ch->signal_display_scale;
		cairo_set_line_width( cr, 1.5);
		cairo_move_to( cr, 10, 10);
		cairo_line_to( cr, 10, 10 + dpuV);
		cairo_stroke( cr);
		cairo_set_font_size( cr, 9);
		cairo_move_to( cr, 15, 20);
		cairo_show_text( cr, "1 mV");
		cairo_stroke( cr);
	}

      // samples per pixel
	{
		cairo_set_font_size( cr, 8);
		snprintf_buf( "%4.2f spp", (float)Ch->samplerate * APSZ / wd);
		cairo_move_to( cr, wd-40, 15);
		cairo_show_text( cr, __buf__);
	}

      // filters
	cairo_set_font_size( cr, 9);
	if ( Ch->low_pass_cutoff > 0. ) {
		snprintf_buf( "LP: %g/%u", Ch->low_pass_cutoff, Ch->low_pass_order);
		cairo_move_to( cr, wd-100, 15);
		cairo_show_text( cr, __buf__);
	}
	if ( Ch->high_pass_cutoff > 0. ) {
		snprintf_buf( "HP: %g/%u", Ch->high_pass_cutoff, Ch->high_pass_order);
		cairo_move_to( cr, wd-100, 24);
		cairo_show_text( cr, __buf__);
	}
	cairo_stroke( cr);

      // marquee
	if ( draw_marquee ) {
		float vstart = (__marquee_start < __marquee_virtual_end) ? __marquee_start : __marquee_virtual_end,
			vend = (__marquee_start < __marquee_virtual_end) ? __marquee_virtual_end : __marquee_start;
		cairo_set_source_rgba( cr, .7, .7, .7, .3);
		cairo_rectangle( cr,
				 vstart, 0,
				 vend - vstart, ht);
		cairo_fill( cr);

	      // start/end timestamp
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr, 1, .1, .11);

		cairo_text_extents_t extents;
		snprintf_buf( "%5.2fs", vstart/wd * APSZ);
		cairo_text_extents( cr, __buf__, &extents);
		double ido = vstart - 3 - extents.width;
		if ( ido < 0+extents.width+3 )
			cairo_move_to( cr, vstart+3, 30);
		else
			cairo_move_to( cr, ido, 12);
		cairo_show_text( cr, __buf__);

		if ( vend - vstart > 5 ) {  // don't mark end if selection is too short
			snprintf_buf( "%5.2fs", vend/wd * APSZ);
			cairo_text_extents( cr, __buf__, &extents);
			ido = vend+extents.width+3;
			if ( ido > wd )
				cairo_move_to( cr, vend-3-extents.width, 30);
			else
				cairo_move_to( cr, vend+3, 12);
			cairo_show_text( cr, __buf__);

			snprintf_buf( "←%4.2fs→", (vend-vstart)/wd * APSZ);
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, vstart+(vend-vstart)/2 - extents.width/2,
				       extents.width < vend-vstart ? 12 : 30);
			cairo_show_text( cr, __buf__);
		}
	}
}



static void
draw_page_to_widget( GtkWidget *wid, const struct SChannelPresentation *Ch)
{
	cairo_t *cr = gdk_cairo_create( wid->window);

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	__draw_page( cr, Ch, wd, ht, __marking_in_widget == wid);

	cairo_set_line_width( cr, .3);

      // unfazer
	if ( __select_state ) {
		cairo_text_extents_t extents;
		cairo_set_font_size( cr, 15);
		if ( Ch == __clicked_channel ) {
			switch ( __select_state ) {
			case SEL_UNF_CHANNEL:
				snprintf_buf( "Unfaze this channel from...");
				gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_UNFAZER]);
			    break;
			case SEL_UNF_CALIBRATE:
				snprintf_buf( "Unfaze this channel from %s",
					      __unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				SRC_DATA samples;
				float *s1, *s2;
				samples.data_in = &Ch->signal_original[ (samples.input_frames = Ch->samplerate * APSZ) * __cur_page_app ];
				samples.data_out = s1 = (float*)malloc( (samples.output_frames = wd) * sizeof(float));
				samples.src_ratio = (double)samples.output_frames / samples.input_frames;
				if ( src_simple( &samples, SRC_SINC_FASTEST, 1) )
					;

				samples.data_in = &__unfazer_offending_channel->signal_original[ samples.input_frames * __cur_page_app ];
				samples.data_out = s2 = (float*)malloc( samples.output_frames * sizeof(float));
				if ( src_simple( &samples, SRC_LINEAR, 1) )
					;

				cairo_move_to( cr, 0,
					       - (s1[0] - s2[0] * __unfazer_factor)
					       * Ch->signal_display_scale
					       + ht/2);
				for ( size_t i = 0; i < wd; ++i ) {
					cairo_line_to( cr, i,
						       - (s1[i] - s2[i] * __unfazer_factor)
						       * Ch->signal_display_scale
						       + ht/2);
				}
				cairo_stroke( cr);

				free( (void*)s1);
				free( (void*)s2);
				gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_UNFAZER]);
			    break;
			}

			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, wd/2 - extents.width/2, ht-30);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);

		} else if ( Ch == __unfazer_offending_channel ) {
			switch ( __select_state ) {
			case SEL_UNF_CHANNEL:
			    break;
			case SEL_UNF_CALIBRATE:
				snprintf_buf( "Calibrating unfaze factor: %4.2f",
					      __unfazer_factor);
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, wd/2 - extents.width/2, ht-30);
				cairo_show_text( cr, __buf__);
			    break;
			}
		}
	} else
		gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_GENERAL]);

      // crosshair
	if ( __draw_crosshair ) {
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr,
				      (double)__bg1__[cCURSOR].red/65536,
				      (double)__bg1__[cCURSOR].green/65536,
				      (double)__bg1__[cCURSOR].blue/65536);

		float t = (float)__crosshair_at/wd * APSZ;
		cairo_move_to( cr, __crosshair_at, 0);
		cairo_line_to( cr, __crosshair_at, ht);
		snprintf_buf( "(%5.2fs) %4.2f",
			      t,
			      (Ch->draw_processed_signal ? Ch->signal_filtered : Ch->signal_original)
			      [ (size_t)((__cur_page_app*APSZ + t) * Ch->samplerate) ]);
		cairo_move_to( cr, __crosshair_at+2, 12);
		cairo_show_text( cr, __buf__);
	}

	cairo_stroke( cr);
	cairo_destroy( cr);

}
static void
draw_page_to_file( const char *fname, const struct SChannelPresentation *Ch,
		   guint width, guint height)
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
	cairo_t *cr = cairo_create( cs);

	__draw_page( cr, Ch, width, height, FALSE);

	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif

}


gboolean
daScoringFacPageView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)
	if ( !CH_IS_EXPANDED || !Ch->n_samples )
		return TRUE;

	draw_page_to_widget( wid, Ch);

	return TRUE;
#undef Ch
}






gboolean
daScoringFacPageView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
#define	Ch ((struct SChannelPresentation*) userdata)

	gint wd, ht;
	guint h;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	switch ( __select_state ) {

	case SEL_UNF_CHANNEL:
		if ( event->button == 1 )
			if ( strcmp( Ch->name, __clicked_channel->name) != 0 ) {
				__unfazer_offending_channel = Ch;
				float f = agh_edf_get_unfazer_factor( __source_ref,
								      __clicked_channel->name,
								      __unfazer_offending_channel->name);
				__unfazer_factor = ( isnan(f) ) ? 0. : f;
				__select_state = SEL_UNF_CALIBRATE;
			} else {
				__select_state = 0;
			}
		else
			__select_state = 0;
		REDRAW_ALL;
	    break;

	case SEL_UNF_CALIBRATE:
		if ( event->button == 1 && __clicked_channel == Ch ) {
			agh_edf_add_or_mod_unfazer( __source_ref,  // apply
						    __clicked_channel->name,
						    __unfazer_offending_channel->name,
						    __unfazer_factor);
			Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
							       Ch->name,
							       &Ch->unfazers);
			agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
							       &Ch->signal_filtered, NULL, NULL);
			if ( Ch->power )
				agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
										    Ch->from, Ch->upto,
										    Ch->power);
			__select_state = 0;

		} else if ( event->button == 2 )
			if ( event->state & GDK_CONTROL_MASK ) { // remove unfazer
				agh_edf_remove_unfazer( __source_ref,
							__clicked_channel->name,
							__unfazer_offending_channel->name);
				Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
								       Ch->name,
								       &Ch->unfazers);
				agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
								       &Ch->signal_filtered, NULL, NULL);
				if ( Ch->power )
					agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
											    Ch->from, Ch->upto,
											    Ch->power);
				__select_state = 0;
			} else
				__unfazer_factor = 0.;
		else
			__select_state = 0;

		REDRAW_ALL;
	    break;

	case 0:
		switch ( event->button ) {
		case 2:
			if ( event->state & GDK_CONTROL_MASK )
				for ( h = 0; h < __n_all_channels; ++h )
					HH[h].signal_display_scale = __sane_signal_display_scale;
			else
				Ch->signal_display_scale = __sane_signal_display_scale;
			REDRAW_ALL;
		    break;
		case 3:
			__clicked_channel = Ch;  // no other way to mark this channel even though user may not select Unfaze
			gtk_widget_set_sensitive( iSFPageShowDZCDF, Ch->signal_dzcdf != NULL);
			gtk_widget_set_sensitive( iSFPageShowEnvelope, Ch->signal_breadth != NULL);
			gtk_menu_popup( GTK_MENU (mSFPage),
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		case 1:
			__marking_in_widget = wid;
			__marquee_start = __marquee_virtual_end = event->x;
			gtk_widget_queue_draw( wid);
		    break;
		}
	}

	return TRUE;
#undef Ch
}





gboolean
daScoringFacPageView_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	gint wd;
	gdk_drawable_get_size( wid->window, &wd, NULL);

	if ( wid != __marking_in_widget )
		return TRUE;

	switch ( event->button ) {
	case 1:
		if ( __marquee_virtual_end != __marquee_start ) {
			__clicked_channel = Ch;
			gtk_menu_popup( GTK_MENU (mSFPageSelection),
					NULL, NULL, NULL, NULL, 3, event->time);
		}
	    break;
	case 3:
	    break;
	}

	__marking_in_widget = NULL;

	return TRUE;
#undef Ch
}








gboolean
daScoringFacPageView_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	gint wd;
	gdk_drawable_get_size( wid->window, &wd, NULL);

      // update marquee boundaries
	if ( __marking_in_widget == wid && __select_state == 0 )
		__marquee_virtual_end = (event->x > 0. ) ? event->x : 0;

      // update crosshair
	if ( __draw_crosshair ) {
		__crosshair_at = event->x;
		for ( guint h = 0; h < __n_all_channels; ++h ) {
			if ( gtk_expander_get_expanded( GTK_EXPANDER (HH[h].expander)) && HH[h].da_page )
				gtk_widget_queue_draw( HH[h].da_page);
		}
	} else if ( __marking_in_widget == wid )
		gtk_widget_queue_draw( wid);

	return TRUE;
}



gboolean
daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	if ( __select_state == SEL_UNF_CALIBRATE && Ch == __clicked_channel ) {
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( fabs(__unfazer_factor) > .2 )
				__unfazer_factor -= .1;
			else
				__unfazer_factor -= .02;
		    break;
		case GDK_SCROLL_UP:
			if ( fabs(__unfazer_factor) > .2 )
				__unfazer_factor += .1;
			else
				__unfazer_factor += .02;
		    break;
		default:
		    break;
		}
		gtk_widget_queue_draw( __clicked_channel->da_page);
		gtk_widget_queue_draw( __unfazer_offending_channel->da_page);
		return TRUE;
	}

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		Ch->signal_display_scale /= 1.1;
	    break;
	case GDK_SCROLL_UP:
		Ch->signal_display_scale *= 1.1;
	    break;
	default:
	    break;
	}

	if ( event->state & GDK_CONTROL_MASK ) {
		for ( guint h = 0; h < __n_all_channels; ++h )
			HH[h].signal_display_scale =
				Ch->signal_display_scale;
		gtk_widget_queue_draw( cScoringFacPageViews);
	} else
		gtk_widget_queue_draw( wid);

	return TRUE;
#undef Ch
}










// -------------------- PSD profile


gboolean
daScoringFacPSDProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)
	if ( !CH_IS_EXPANDED )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

	cairo_set_source_rgb( cr,
			      (double)__bg1__[cHYPNOGRAM].red/65536,
			      (double)__bg1__[cHYPNOGRAM].green/65536,
			      (double)__bg1__[cHYPNOGRAM].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);

	guint i;

      // profile
	if ( Ch->draw_bands ) {
		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 9);
		for ( gushort b = 0; b <= Ch->uppermost_band; ++b ) {
			cairo_set_source_rgb( cr,
					      (double)__fg1__[cBAND_DELTA + b].red/65536,
					      (double)__fg1__[cBAND_DELTA + b].green/65536,
					      (double)__fg1__[cBAND_DELTA + b].blue/65536);
			cairo_move_to( cr, .5 / __total_pages * wd,
				       -Ch->power_in_bands[b][0] * Ch->power_display_scale + ht);
			for ( i = 1; i < __total_pages; ++i )
				cairo_line_to( cr, (float)(i+.5) / __total_pages * wd,
					       - Ch->power_in_bands[b][i] * Ch->power_display_scale + ht);
			if ( b == Ch->focused_band ) {
				cairo_line_to( cr, wd, ht);
				cairo_line_to( cr, 0., ht);
				cairo_fill( cr);
			}
			cairo_stroke( cr);

			if ( b == Ch->focused_band ) {
				cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
				snprintf_buf( "%s %g–%g",
					      AghFreqBandsNames[b],
					      AghFreqBands[b][0], AghFreqBands[b][1]);
			} else {
				cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				snprintf_buf( "%s", AghFreqBandsNames[b]);
			}
			cairo_move_to( cr, wd - 70, Ch->uppermost_band*12 - 12*b + 12);
			cairo_show_text( cr, __buf__);
		}

	} else {
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cPOWER_SF].red/65536,
				      (double)__fg1__[cPOWER_SF].green/65536,
				      (double)__fg1__[cPOWER_SF].blue/65536);
		cairo_move_to( cr, .5 / __total_pages * wd, Ch->power[0]);
		for ( i = 0; i < __total_pages; ++i )
			cairo_line_to( cr, (double)(i+.5) / __total_pages * wd,
				       - Ch->power[i] * Ch->power_display_scale + ht);
		cairo_line_to( cr, wd, ht);
		cairo_line_to( cr, 0., ht);
		cairo_fill( cr);

		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		snprintf_buf( "%g–%g Hz", Ch->from, Ch->upto);
		cairo_move_to( cr, wd-50, 15);
		cairo_show_text( cr, __buf__);
	}
	cairo_stroke( cr);

      // scale
	cairo_set_source_rgb( cr, 0., 0., 0.);
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_line_width( cr, 1.5);
	// cairo_set_line_cap( cr, CAIRO_LINE_CAP_BUTT); // is default
	cairo_move_to( cr, 10, 10);
	cairo_line_to( cr, 10, 10 + Ch->power_display_scale * 1e6);
	cairo_stroke( cr);

	cairo_move_to( cr, 15, 20);
	cairo_show_text( cr, "1 µV²/Hz");
	cairo_stroke( cr);

      // hour ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, 1);
	cairo_set_font_size( cr, 10);
	float	hours4 = (float)Ch->n_samples / Ch->samplerate / 3600 * 4;
	for ( i = 1; i < hours4; ++i ) {
		guint tick_pos = (float)i / hours4 * wd;
		cairo_move_to( cr, tick_pos, 0);
		cairo_line_to( cr, tick_pos, (i%4 == 0) ? 20 : (i%2 == 0) ? 12 : 5);
		if ( i % 4 == 0 ) {
			snprintf_buf( "%2uh", i/4);
			cairo_move_to( cr, tick_pos+5, 12);
			cairo_show_text( cr, __buf__);
		}
	}
	cairo_stroke( cr);

      // cursor
	cairo_set_source_rgba( cr,
			       (double)__bg1__[cCURSOR].red/65536,
			       (double)__bg1__[cCURSOR].green/65536,
			       (double)__bg1__[cCURSOR].blue/65536,
			       .5);
	cairo_rectangle( cr,
			 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
#undef Ch
}









void bScoringFacForward_clicked_cb();
void bScoringFacBack_clicked_cb();

gboolean
daScoringFacPSDProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	if ( event->state & GDK_SHIFT_MASK ) {
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( Ch->draw_bands ) {
				if ( Ch->focused_band > 0 )
					--Ch->focused_band;
			} else
				if ( Ch->from > 0 ) {
					Ch->from--, Ch->upto--;
					agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
											    Ch->from, Ch->upto,
											    Ch->power);
				}
		    break;
		case GDK_SCROLL_UP:
			if ( Ch->draw_bands ) {
				if ( Ch->focused_band < Ch->uppermost_band )
					++Ch->focused_band;
			} else
				if ( Ch->upto < 10 ) {
					Ch->from++, Ch->upto++;
					agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
											    Ch->from, Ch->upto,
											    Ch->power);
				}
		    break;
		case GDK_SCROLL_LEFT:
		case GDK_SCROLL_RIGHT:
		    break;
		}

	} else {
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			Ch->power_display_scale /= 1.1;
		    break;
		case GDK_SCROLL_UP:
			Ch->power_display_scale *= 1.1;
		    break;
		case GDK_SCROLL_LEFT:
			bScoringFacBack_clicked_cb();
		    break;
		case GDK_SCROLL_RIGHT:
			bScoringFacForward_clicked_cb();
		    break;
		}

		__queue_redraw_all();
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
#undef Ch
}









gboolean
daScoringFacPSDProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	gint wd, ht;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	switch ( event->button ) {
	case 1:
		__cur_page = (event->x / wd) * __total_pages;
		__cur_page_app = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	    break;
	case 2:
		Ch->draw_bands = !Ch->draw_bands;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
		__clicked_channel = Ch;
		gtk_menu_popup( GTK_MENU (mSFPower),
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}

	return TRUE;
#undef Ch
}






// ------------- Spectrum

gboolean
daScoringFacSpectrumView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	if ( !PS_IS_RIGHT )
		return TRUE;

	if ( !CH_IS_EXPANDED )
		return TRUE;

	gint wd, ht;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

	cairo_set_source_rgb( cr,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);

	guint	graph_height = AghSFDAPowerProfileHeight - 4,
		graph_width  = AghSFDASpectrumWidth - 14;

      // grid lines
	cairo_set_source_rgba( cr,
			       (double)__fg1__[cSPECTRUM_GRID].red/65536,
			       (double)__fg1__[cSPECTRUM_GRID].green/65536,
			       (double)__fg1__[cSPECTRUM_GRID].blue/65536,
			       .2);
	cairo_set_line_width( cr, .3);
	for ( gushort i = 1; i < Ch->last_spectrum_bin; ++i ) {
		cairo_move_to( cr, 12 + (float)i/Ch->last_spectrum_bin * graph_width, AghSFDAPowerProfileHeight - 2);
		cairo_line_to( cr, 12 + (float)i/Ch->last_spectrum_bin * graph_width, 2);
	}
	cairo_stroke( cr);

      // spectrum
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cSPECTRUM].red/65536,
			      (double)__fg1__[cSPECTRUM].green/65536,
			      (double)__fg1__[cSPECTRUM].blue/65536);
	cairo_set_line_width( cr, 1);
	guint m;
	gfloat	factor = Ch->draw_spectrum_absolute ? 1/Ch->power_display_scale : Ch->spectrum_max/graph_height;
	cairo_move_to( cr,
		       12, AghSFDAPowerProfileHeight - (2 + Ch->spectrum[0] / factor));
	for ( m = 1; m < Ch->last_spectrum_bin; ++m ) {
		cairo_line_to( cr,
			       12 + (float)(graph_width) / (Ch->last_spectrum_bin) * m,
			       AghSFDAPowerProfileHeight
			       - (2 + Ch->spectrum[m] / factor));
	}
	cairo_stroke( cr);

      // axes
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cSPECTRUM_AXES].red/65536,
			      (double)__fg1__[cSPECTRUM_AXES].green/65536,
			      (double)__fg1__[cSPECTRUM_AXES].blue/65536);
	cairo_set_line_width( cr, .5);
	cairo_move_to( cr, 12, 2);
	cairo_line_to( cr, 12, AghSFDAPowerProfileHeight - 2);
	cairo_line_to( cr, graph_width - 2, AghSFDAPowerProfileHeight - 2);

      // x ticks
	m = 0;
	while ( (++m * 1e6) < graph_height * factor ) {
		cairo_move_to( cr, 6,  AghSFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
		cairo_line_to( cr, 12, AghSFDAPowerProfileHeight - (2 + (float)m*1e6 / factor));
	}
	cairo_stroke( cr);

      // labels
	cairo_text_extents_t extents;
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cLABELS_SF].red/65536,
			      (double)__fg1__[cLABELS_SF].green/65536,
			      (double)__fg1__[cLABELS_SF].blue/65536);
	cairo_set_font_size( cr, 8);

	snprintf_buf( "%g Hz", Ch->last_spectrum_bin * Ch->binsize);
	cairo_text_extents( cr, __buf__, &extents);
	cairo_move_to( cr,
		       AghSFDASpectrumWidth - extents.width - 2,
		       AghSFDAPowerProfileHeight - 2 - extents.height - 2);
	cairo_show_text( cr, __buf__);
	cairo_stroke( cr);

	snprintf_buf( "%c", Ch->draw_spectrum_absolute ? 'A' : 'R');
	cairo_move_to( cr, AghSFDASpectrumWidth - extents.width - 3, 9);
	cairo_show_text( cr, __buf__);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
#undef Ch
}





gboolean
daScoringFacSpectrumView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	switch ( event->button ) {
	case 1:
	    break;
	case 2:
		Ch->draw_spectrum_absolute = !Ch->draw_spectrum_absolute;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
//		gtk_menu_popup( GTK_MENU (mSFSpectrum),  // nothing useful
//				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}

	return TRUE;
#undef Ch
}


gboolean
daScoringFacSpectrumView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( Ch->last_spectrum_bin > 4 )
				Ch->last_spectrum_bin -= 1;
		} else
			Ch->power_display_scale /= 1.1;
	    break;
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( Ch->last_spectrum_bin < Ch->n_bins )
				Ch->last_spectrum_bin += 1;
		} else
			Ch->power_display_scale *= 1.1;
	default:
	    break;
	}

	__queue_redraw_all();

	return TRUE;
#undef Ch
}









// -------------------- EMG profile

gboolean
daScoringFacEMGProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)
	if ( !CH_IS_EXPANDED )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

	cairo_set_source_rgb( cr,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].red/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].green/65536,
			      (double)__bg1__[cSIGNAL_SCORE_NONE].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	guint i;

      // avg EMG
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cEMG].red/65536,
			      (double)__fg1__[cEMG].green/65536,
			      (double)__fg1__[cEMG].blue/65536);
	cairo_set_line_width( cr, .8);
	for ( i = 0; i < __total_pages; ++i ) {
		cairo_move_to( cr, (double)(i+.5) / __total_pages * wd,
			       AghSFDAEMGProfileHeight/2
			       - Ch->emg_fabs_per_page[i] * Ch->emg_scale);
		cairo_line_to( cr, (double)(i+.5) / __total_pages * wd,
			       AghSFDAEMGProfileHeight/2
			       + Ch->emg_fabs_per_page[i] * Ch->emg_scale);
	}
	cairo_stroke( cr);

      // hour ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, 1);

	cairo_set_font_size( cr, 7);
	float	hours = Ch->n_samples / Ch->samplerate / 3600;
	for ( i = 1; i < hours; ++i ) {
		guint tick_pos = (float)i / hours * wd;
		cairo_move_to( cr, tick_pos, 0);
		cairo_line_to( cr, tick_pos, 15);
		snprintf_buf( "%2uh", i);
		cairo_move_to( cr, tick_pos + 5, 9);
		cairo_show_text( cr, __buf__);
	}

      // cursor
	cairo_set_source_rgba( cr,
			       (double)__bg1__[cCURSOR].red/65536,
			       (double)__bg1__[cCURSOR].green/65536,
			       (double)__bg1__[cCURSOR].blue/65536,
			       .7);
	cairo_rectangle( cr,
			 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
#undef Ch
}








gboolean
daScoringFacEMGProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
#define Ch ((struct SChannelPresentation*) userdata)

	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		if ( Ch->emg_scale < 2500 )
			Ch->emg_scale *= 1.3;
	    break;
	case GDK_SCROLL_DOWN:
		if ( Ch->emg_scale > .001 )
			Ch->emg_scale /= 1.3;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
#undef Ch
}







gboolean
daScoringFacEMGProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	//SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	gint wd, ht;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	switch ( event->button ) {
	case 1:
		__cur_page = (event->x / wd) * __total_pages;
		__cur_page_app = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	    break;
	case 2:
	    break;
	case 3:
	    break;
	}

	return TRUE;
}









// -------------------- Hypnogram

gboolean
daScoringFacHypnogram_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer unused)
{
	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	cairo_t *cr = gdk_cairo_create( wid->window);

      // bg
	cairo_set_source_rgb( cr,
			      (double)__bg1__[cHYPNOGRAM].red/65536,
			      (double)__bg1__[cHYPNOGRAM].green/65536,
			      (double)__bg1__[cHYPNOGRAM].blue/65536);
	cairo_rectangle( cr, 0., 0., wd, ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	cairo_set_source_rgba( cr,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].red/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].green/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].blue/65536,
			       .5);
	cairo_set_line_width( cr, .4);
	guint i;
	for ( i = 1; i < 8; ++i ) {
		cairo_move_to( cr, 0,   __score_hypn_depth[i]);
		cairo_line_to( cr, wd,  __score_hypn_depth[i]);
	}
	cairo_stroke( cr);

      // scores
	cairo_set_source_rgba( cr,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].red/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].green/65536,
			       (double)__fg1__[cHYPNOGRAM_SCORELINE].blue/65536,
			       1.);
	cairo_set_line_width( cr, 3.);
	// these lines can be discontinuous
	for ( i = 0; i < __total_pages; ++i ) {
		gchar c;
		if ( (c = __hypnogram[i]) != AghScoreCodes[AGH_SCORE_NONE] ) {
			gint y = __score_hypn_depth[ SCOREID(c) ];
			cairo_move_to( cr, lroundf( (float) i   /__total_pages * wd), y);
			cairo_line_to( cr, lroundf( (float)(i+1)/__total_pages * wd), y);
		}
	}
	cairo_stroke( cr);

	cairo_set_source_rgba( cr,
			       (double)__bg1__[cCURSOR].red/65536,
			       (double)__bg1__[cCURSOR].green/65536,
			       (double)__bg1__[cCURSOR].blue/65536,
			       .7);
	cairo_rectangle( cr,
			 (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			 ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht-1);
	cairo_fill( cr);

	cairo_stroke( cr);
	cairo_destroy( cr);

	return TRUE;
}




gboolean
daScoringFacHypnogram_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer unused)
{
	gint wd;
	gdk_drawable_get_size( wid->window, &wd, NULL);

	switch ( event->button ) {
	case 1:
		__cur_page = (event->x / wd) * __total_pages;
		__cur_page_app = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	    break;
	case 3:
		gtk_menu_popup( GTK_MENU (mSFScore),
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}
	return TRUE;
}








// ---------- page value_changed


void
eScoringFacPageSize_changed_cb()
{
	guint cur_pos = __cur_page_app * APSZ;
	__pagesize_item = gtk_combo_box_get_active( GTK_COMBO_BOX (eScoringFacPageSize));
	__cur_page_app = cur_pos / APSZ;
//	__cur_page = AP2P (__cur_page_app); // shouldn't change

	gtk_spin_button_set_range( GTK_SPIN_BUTTON (eScoringFacCurrentPage), 1, P2AP (__total_pages));
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);

	snprintf_buf( "<small>of</small> %d", P2AP (__total_pages));
	gtk_label_set_markup( GTK_LABEL (lScoringFacTotalPages), __buf__);

	gtk_widget_set_sensitive( bScoreClear, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreNREM1, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreNREM2, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreNREM3, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreNREM4, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreREM,   PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreWake,  PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreMVT,   PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreGotoPrevUnscored, PS_IS_RIGHT);
	gtk_widget_set_sensitive( bScoreGotoNextUnscored, PS_IS_RIGHT);

	if ( !__suppress_redraw )
		REDRAW_ALL;
}





void
eScoringFacCurrentPage_value_changed_cb()
{
	__cur_page_app = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage)) - 1;
	__cur_page = AP2P (__cur_page_app);
//	CLAMP (__cur_page_app, 0, P2AP (__total_pages-1));
	guint __cur_pos = __cur_page_app * APSZ;
	__cur_pos_hr  =  __cur_pos / 3600;
	__cur_pos_min = (__cur_pos - __cur_pos_hr * 3600) / 60;
	__cur_pos_sec =  __cur_pos % 60;

	__cur_stage = SCOREID( __hypnogram[__cur_pos / PSZ]);

	for ( size_t h = 0; h < __n_all_channels; ++h ) {
#define Ch (&HH[h])
		if ( CH_IS_EXPANDED && Ch->da_page ) {
			if ( !__suppress_redraw )
				gtk_widget_queue_draw( Ch->da_page);

			if ( Ch->da_power ) {
				if ( Ch->spectrum ) {
					free( Ch->spectrum);
					agh_msmt_get_power_spectrum_as_float( Ch->rec_ref, __cur_page,
									      &Ch->spectrum, &Ch->spectrum_max);
					if ( !__suppress_redraw )
						gtk_widget_queue_draw( Ch->da_spectrum);
				}
				if ( !__suppress_redraw )
					gtk_widget_queue_draw( Ch->da_power);
			}
			if ( Ch->da_emg_profile )
				if ( !__suppress_redraw )
					gtk_widget_queue_draw( Ch->da_emg_profile);
		}
#undef Ch
	}
	snprintf_buf( "<b><big>%s</big></b>", __score_names[ __cur_stage ]);
	gtk_label_set_markup( GTK_LABEL (lScoringFacCurrentStage), __buf__);

	snprintf_buf( "<b>%2zu:%02zu:%02zu</b>", __cur_pos_hr, __cur_pos_min, __cur_pos_sec);
	gtk_label_set_markup( GTK_LABEL (lScoringFacCurrentPos), __buf__);

	time_t time_at_cur_pos = __start_time + __cur_pos;
	char tmp[10];
	strftime( tmp, 9, "%H:%M:%S", localtime( &time_at_cur_pos));
	snprintf_buf( "<b>%s</b>", tmp);
	gtk_label_set_markup( GTK_LABEL (lScoringFacClockTime), __buf__);

	if ( !__suppress_redraw )
		gtk_widget_queue_draw( daScoringFacHypnogram);
}








// -------------- various buttons


static void
__repaint_score_stats()
{
	snprintf_buf( "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", __percent_NREM());
	gtk_label_set_markup( GTK_LABEL (lScoreStatsNREMPercent), __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", __percent_REM());
	gtk_label_set_markup( GTK_LABEL (lScoreStatsREMPercent), __buf__);

	snprintf_buf( "<small>%3.1f</small> %%", __percent_Wake());
	gtk_label_set_markup( GTK_LABEL (lScoreStatsWakePercent), __buf__);
}



static void
__do_score_forward( char score_ch)
{
	__have_unsaved_scores = TRUE;
	if ( __cur_page < __total_pages ) {
		__hypnogram[__cur_page] = score_ch;
		++__cur_page;
		++__cur_page_app; //  = P2AP (__cur_page);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
		__repaint_score_stats();
	}
}


void bScoreClear_clicked_cb()  { __do_score_forward( AghScoreCodes[AGH_SCORE_NONE]); }
void bScoreNREM1_clicked_cb()  { __do_score_forward( AghScoreCodes[AGH_SCORE_NREM1]); }
void bScoreNREM2_clicked_cb()  { __do_score_forward( AghScoreCodes[AGH_SCORE_NREM2]); }
void bScoreNREM3_clicked_cb()  { __do_score_forward( AghScoreCodes[AGH_SCORE_NREM3]); }
void bScoreNREM4_clicked_cb()  { __do_score_forward( AghScoreCodes[AGH_SCORE_NREM4]); }
void bScoreREM_clicked_cb()    { __do_score_forward( AghScoreCodes[AGH_SCORE_REM]); }
void bScoreWake_clicked_cb()   { __do_score_forward( AghScoreCodes[AGH_SCORE_WAKE]); }
void bScoreMVT_clicked_cb()    { __do_score_forward( AghScoreCodes[AGH_SCORE_MVT]); }





void
bScoringFacForward_clicked_cb()
{
	if ( __cur_page_app < P2AP(__total_pages) ) {
		++__cur_page_app;
		__cur_page = AP2P (__cur_page_app);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	}
}

void
bScoringFacBack_clicked_cb()
{
	if ( __cur_page_app ) {
		--__cur_page_app;
		__cur_page = AP2P (__cur_page_app);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	}
}






void
bScoreGotoPrevUnscored_clicked_cb()
{
	if ( APSZ != PSZ || !(__cur_page > 0) )
		return;
	size_t p = __cur_page - 1;
	while ( __hypnogram[p] != AghScoreCodes[AGH_SCORE_NONE] )
		if ( p > 0 )
			--p;
		else
			break;
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   (__cur_page_app = __cur_page = p)+1);
}

void
bScoreGotoNextUnscored_clicked_cb()
{
	if ( APSZ != PSZ || !(__cur_page < __total_pages) )
		return;
	guint p = __cur_page + 1;
	while ( __hypnogram[p] != AghScoreCodes[AGH_SCORE_NONE] )
		if ( p < __total_pages )
			++p;
		else
			break;
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   (__cur_page_app = __cur_page = p)+1);
}





inline static gboolean
__page_has_artifacts( size_t p)
{
	for ( guint h = 0; h < __n_all_channels; ++h ) {
		for ( size_t a = 0; a < HH[h].n_artifacts; ++a )
			if ( (p * APSZ * HH[h].samplerate < HH[h].artifacts[a*2  ] && HH[h].artifacts[a*2  ] < (p+1) * APSZ * HH[h].samplerate) ||
			     (p * APSZ * HH[h].samplerate < HH[h].artifacts[a*2+1] && HH[h].artifacts[a*2+1] < (p+1) * APSZ * HH[h].samplerate)
			   ||
			     (HH[h].artifacts[a*2  ] < p * APSZ * HH[h].samplerate && (p+1) * APSZ * HH[h].samplerate < HH[h].artifacts[a*2+1]) ) {
				return TRUE;
		}
	}
	return FALSE;
}

void
bScoreGotoPrevArtifact_clicked_cb()
{
	if ( APSZ != PSZ || !(__cur_page > 0) )
		return;
	guint p = __cur_page - 1;
	gboolean p_has_af;
	while ( !(p_has_af = __page_has_artifacts(p)) )

		if ( p > 0 )
			--p;
		else
			break;
	if ( p == 0 && !p_has_af )
		;
	else
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
					   (__cur_page_app = __cur_page = p)+1);
}

void
bScoreGotoNextArtifact_clicked_cb()
{
	if ( APSZ != PSZ || !(__cur_page < __total_pages) )
		return;
	guint p = __cur_page + 1;
	gboolean p_has_af;
	while ( !(p_has_af = __page_has_artifacts(p)) )
		if ( p < __total_pages-1 )
			++p;
		else
			break;
	if ( p == __total_pages-1 && !p_has_af )
		;
	else
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
					   (__cur_page_app = __cur_page = p)+1);
}








void
bScoringFacDrawPower_toggled_cb()
{
	__draw_power = !__draw_power;
	for ( guint h = 0; h < __n_all_channels; ++h ) {
		if ( HH[h].power ) {
			g_object_set( G_OBJECT (HH[h].da_power),
				      "visible", __draw_power ? TRUE : FALSE,
				      NULL);
			g_object_set( G_OBJECT (HH[h].da_spectrum),
				      "visible", __draw_power ? TRUE : FALSE,
				      NULL);
		}
	}
	REDRAW_ALL;
}

void
bScoringFacDrawCrosshair_toggled_cb()
{
	__draw_crosshair = !__draw_crosshair;
	REDRAW_ALL;
}


void
bScoringFacUseResample_toggled_cb()
{
	__use_resample = !__use_resample;
	REDRAW_ALL;
}



void
bScoringFacShowFindDialog_toggled_cb( GtkToggleButton *togglebutton,
				      gpointer         user_data)
{
	if ( gtk_toggle_button_get_active( togglebutton) ) {
		gtk_widget_show_all( wPattern);
	} else
		gtk_widget_hide( wPattern);
}



void
bScoringFacShowPhaseDiffDialog_toggled_cb( GtkToggleButton *togglebutton,
					   gpointer         user_data)
{
	if ( gtk_toggle_button_get_active( togglebutton) ) {
		gtk_widget_show_all( wPhaseDiff);
	} else
		gtk_widget_hide( wPhaseDiff);
}







// ------ menu callbacks

// -- Page
void
mSFPage_show_cb()
{
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowOriginal),
					__clicked_channel->draw_original_signal);
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowProcessed),
					__clicked_channel->draw_processed_signal);
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowDZCDF),
					__clicked_channel->draw_dzcdf);
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowEnvelope),
					__clicked_channel->draw_envelope);
}


void
iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->draw_original_signal = gtk_check_menu_item_get_active( checkmenuitem);
      // prevent both being switched off
	if ( !__clicked_channel->draw_original_signal && !__clicked_channel->draw_processed_signal )
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowProcessed),
						__clicked_channel->draw_processed_signal = TRUE);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}


void
iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->draw_processed_signal = gtk_check_menu_item_get_active( checkmenuitem);
	if ( !__clicked_channel->draw_processed_signal && !__clicked_channel->draw_original_signal )
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFPageShowOriginal),
						__clicked_channel->draw_original_signal = TRUE);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}


void
iSFPageShowDZCDF_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->draw_dzcdf = gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}

void
iSFPageShowEnvelope_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->draw_envelope = gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}



void
iSFPageClearArtifacts_activate_cb()
{
	if ( pop_question( GTK_WINDOW (wScoringFacility),
			   "All marked artifacts will be lost in this channel.  Continue?") != GTK_RESPONSE_YES )
		return;

	agh_edf_clear_artifact( __source_ref, __clicked_channel->name,
				0, __clicked_channel->n_samples);
	__clicked_channel->n_artifacts = agh_edf_get_artifacts( __source_ref, __clicked_channel->name,
								&__clicked_channel->artifacts);
	agh_msmt_get_signal_filtered_as_float( __clicked_channel->rec_ref,
					       &__clicked_channel->signal_filtered, NULL, NULL);
	if ( __clicked_channel->power ) {
		agh_msmt_get_power_course_in_range_as_float_direct( __clicked_channel->rec_ref,
								    __clicked_channel->from, __clicked_channel->upto,
								    __clicked_channel->power);
		for ( gushort b = 0; b <= __clicked_channel->uppermost_band; ++b )
			agh_msmt_get_power_course_in_range_as_float_direct( __clicked_channel->rec_ref,
									    AghFreqBands[b][0], AghFreqBands[b][1],
									    __clicked_channel->power_in_bands[b]);

		gtk_widget_queue_draw( __clicked_channel->da_power);
	}

	gtk_widget_queue_draw( __clicked_channel->da_page);
}


void
iSFPageUnfazer_activate_cb()
{
	__select_state = SEL_UNF_CHANNEL;

	REDRAW_ALL;
}



void
iSFPageSaveAs_activate_cb()
{
	gint ht, wd;
	gdk_drawable_get_size( __clicked_channel->da_page->window,
			       &wd, &ht);
	char *p;
	agh_subject_get_path( __our_j->name, &p);
	snprintf_buf( "%s/%s/%s-p%zu@%u.svg", p, AghD, AghT, __cur_page_app, APSZ);
	free( p);
	p = g_strdup( __buf__);

	draw_page_to_file( p, __clicked_channel, wd, ht);

	g_free( p);
}


void
iSFPageExportSignal_activate_cb()
{
	char *fname_base = agh_msmt_fname_base( __clicked_channel->rec_ref);
	snprintf_buf( "%s-orig.tsv", fname_base);
	agh_edf_export_signal( __source_ref, __clicked_channel->name, __buf__, 1);
	snprintf_buf( "%s-filt.tsv", fname_base);
	agh_edf_export_signal( __source_ref, __clicked_channel->name, __buf__, 0);
	snprintf_buf( "Wrote %s-{filt,orig}.tsv", fname_base);
	free( fname_base);
	gtk_statusbar_pop( GTK_STATUSBAR (sbSF), agh_sb_context_id_General);
	gtk_statusbar_push( GTK_STATUSBAR (sbSF), agh_sb_context_id_General, __buf__);
}



void
iSFPageUseThisScale_activate_cb()
{
	__sane_signal_display_scale = __clicked_channel->signal_display_scale;
	for ( guint h = 0; h < __n_all_channels; ++h ) {
		HH[h].signal_display_scale = __sane_signal_display_scale;
		gtk_widget_queue_draw( HH[h].da_page);
	}
}






// -- PageSelection


void
iSFPageSelectionMarkArtifact_activate_cb()
{
	__mark_region_as_artifact( 'x');
}

void
iSFPageSelectionClearArtifacts_activate_cb()
{
	__mark_region_as_artifact( '.');
}

void
iSFPageSelectionFindPattern_activate_cb()
{
	__mark_region_as_pattern();
}


void
iSFPageSelectionInspectOne_activate_cb()
{
	FAFA;
}

void
iSFPageSelectionInspectMany_activate_cb( GtkMenuItem *menuitem,
					 gpointer     user_data)
{
	FAFA;
}



// -- Power

void
iSFPowerExportRange_activate_cb()
{
	char *fname_base = agh_msmt_fname_base( __clicked_channel->rec_ref);
	snprintf_buf( "%s_%g-%g.tsv",
		      fname_base, __clicked_channel->from, __clicked_channel->upto);
	agh_msmt_export_power_in_range( __clicked_channel->rec_ref, __clicked_channel->from, __clicked_channel->upto,
					__buf__);
	gtk_statusbar_pop( GTK_STATUSBAR (sbSF), agh_sb_context_id_General);
	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      fname_base, __clicked_channel->from, __clicked_channel->upto);
	gtk_statusbar_push( GTK_STATUSBAR (sbSF), agh_sb_context_id_General, __buf__);
	free( fname_base);
}

void
iSFPowerExportAll_activate_cb()
{
	char *fname_base = agh_msmt_fname_base( __clicked_channel->rec_ref);
	snprintf_buf( "%s.tsv", fname_base);
	agh_msmt_export_power( __clicked_channel->rec_ref,
			       __buf__);
	gtk_statusbar_pop( GTK_STATUSBAR (sbSF), agh_sb_context_id_General);
	snprintf_buf( "Wrote %s.tsv", fname_base);
	gtk_statusbar_push( GTK_STATUSBAR (sbSF), agh_sb_context_id_General, __buf__);
	free( fname_base);
}

void
iSFPowerUseThisScale_activate_cb()
{
	__sane_power_display_scale = __clicked_channel->power_display_scale;
	for ( size_t h = 0; h < __n_all_channels; ++h )
		HH[h].power_display_scale = __sane_power_display_scale;
	__queue_redraw_all();
}





// -- Score

void
iSFScoreAssist_activate_cb()
{
	if ( agh_episode_assisted_score_by_jde( __our_j->name, __our_d, __our_e) == 0 ) {
		__have_unsaved_scores = TRUE;
		free( __hypnogram);
		agh_edf_get_scores( __source_ref, &__hypnogram, NULL);

		__repaint_score_stats();
		REDRAW_ALL;
	}
}



void
iSFScoreImport_activate_cb()
{
	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Import Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_OPEN,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
		agh_edf_import_scores_custom( __source_ref,
					      fname,
					      AghExtScoreCodes);
	}
	gtk_widget_destroy( f_chooser);
	free( __hypnogram);
	agh_edf_get_scores( __source_ref,
			    &__hypnogram, NULL);
	REDRAW_ALL;
	__repaint_score_stats();
}

void
iSFScoreExport_activate_cb()
{
	agh_edf_put_scores( __source_ref,
			    __hypnogram);
	GtkWidget *f_chooser = gtk_file_chooser_dialog_new( "Export Scores",
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_SAVE,
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							    NULL);
	if ( gtk_dialog_run( GTK_DIALOG (f_chooser)) == GTK_RESPONSE_ACCEPT ) {
		gchar *fname = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER (f_chooser));
		agh_edf_export_scores( __source_ref, fname);
	}
	gtk_widget_destroy( f_chooser);
}



void
iSFScoreRevert_activate_cb()
{
	free( __hypnogram);
	agh_edf_get_scores( __source_ref, &__hypnogram, NULL);

	REDRAW_ALL;
}

void
iSFScoreClear_activate_cb()
{
	memset( __hypnogram, (int)AghScoreCodes[AGH_SCORE_NONE], __total_pages);

	REDRAW_ALL;

	snprintf_buf( "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);
}







void
bSFAccept_clicked_cb()
{
	agh_edf_put_scores( __source_ref, __hypnogram);

	agh_cleanup_scoring_facility();

	gtk_widget_hide( wPattern);
	gtk_widget_hide( wScoringFacility);
	gtk_widget_queue_draw( cMeasurements);
}


void
iSFAcceptAndTakeNext_activate_cb()
{
	agh_edf_put_scores( __source_ref, __hypnogram);

	set_cursor_busy( TRUE, wScoringFacility);
	agh_cleanup_scoring_facility();

	agh_prepare_scoring_facility( __our_j, __our_d, __our_e_next);
	gtk_widget_show_all( wScoringFacility);
	set_cursor_busy( FALSE, wScoringFacility);

	REDRAW_ALL;
}



// ------- cleanup

gboolean
wScoringFacility_delete_event_cb( GtkWidget *widget,
				  GdkEvent  *event,
				  gpointer   user_data)
{
	if ( __have_unsaved_scores &&
	     pop_question( GTK_WINDOW (wScoringFacility), "Save your scorings?") == GTK_RESPONSE_YES )
		agh_edf_put_scores( __source_ref, __hypnogram);

	agh_cleanup_scoring_facility();

	gtk_widget_hide( wPattern);
	gtk_widget_hide( wScoringFacility);
	gtk_widget_queue_draw( cMeasurements);

	return TRUE; // to stop other handlers from being invoked for the event
}





// -------- colours

GdkColor
	__fg1__[cTOTAL_SF],
	__bg1__[cTOTAL_SF];


static void
change_fg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( agh_cmap, &__fg1__[c], 1);
	gtk_color_button_get_color( cb, &__fg1__[c]);
	gdk_colormap_alloc_color( agh_cmap, &__fg1__[c], FALSE, TRUE);
}
static void
change_bg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( agh_cmap, &__bg1__[c], 1);
	gtk_color_button_get_color( cb, &__bg1__[c]);
	gdk_colormap_alloc_color( agh_cmap, &__bg1__[c], FALSE, TRUE);

	__fg1__[c] = *contrasting_to( &__bg1__[c]);
//	printf( "%4d:  %5d %5d %5d :: %5d %5d %5d\n", c, __bg1__[c].red, __bg1__[c].green, __bg1__[c].blue, __fg1__[c].red, __fg1__[c].green, __fg1__[c].blue);
}



void
bColourNONE_color_set_cb( GtkColorButton *widget,
			  gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_NONE, widget);
}

void
bColourNREM1_color_set_cb( GtkColorButton *widget,
			   gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_NREM1, widget);
}


void
bColourNREM2_color_set_cb( GtkColorButton *widget,
			   gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_NREM2, widget);
}


void
bColourNREM3_color_set_cb( GtkColorButton *widget,
			   gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_NREM3, widget);
}


void
bColourNREM4_color_set_cb( GtkColorButton *widget,
			   gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_NREM4, widget);
}

void
bColourREM_color_set_cb( GtkColorButton *widget,
			 gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_REM, widget);
}

void
bColourWake_color_set_cb( GtkColorButton *widget,
			  gpointer        user_data)
{
	change_bg_colour( cSIGNAL_SCORE_WAKE, widget);
}



void
bColourPowerSF_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_fg_colour( cPOWER_SF, widget);
}


void
bColourHypnogram_color_set_cb( GtkColorButton *widget,
			       gpointer        user_data)
{
	change_bg_colour( cHYPNOGRAM, widget);
}

void
bColourArtifacts_color_set_cb( GtkColorButton *widget,
			       gpointer        user_data)
{
	change_fg_colour( cARTIFACT, widget);
}



void
bColourTicksSF_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_fg_colour( cTICKS_SF, widget);
}

void
bColourLabelsSF_color_set_cb( GtkColorButton *widget,
			      gpointer        user_data)
{
	change_fg_colour( cLABELS_SF, widget);
}

void
bColourCursor_color_set_cb( GtkColorButton *widget,
			    gpointer        user_data)
{
	change_bg_colour( cCURSOR, widget);
}


void
bColourBandDelta_color_set_cb( GtkColorButton *widget,
				gpointer        user_data)
{
	change_fg_colour( cBAND_DELTA, widget);
}
void
bColourBandTheta_color_set_cb( GtkColorButton *widget,
				gpointer        user_data)
{
	change_fg_colour( cBAND_THETA, widget);
}
void
bColourBandAlpha_color_set_cb( GtkColorButton *widget,
				gpointer        user_data)
{
	change_fg_colour( cBAND_ALPHA, widget);
}
void
bColourBandBeta_color_set_cb( GtkColorButton *widget,
				gpointer        user_data)
{
	change_fg_colour( cBAND_BETA, widget);
}
void
bColourBandGamma_color_set_cb( GtkColorButton *widget,
				gpointer        user_data)
{
	change_fg_colour( cBAND_GAMMA, widget);
}



// EOF

