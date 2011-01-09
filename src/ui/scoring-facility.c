// ;-*-C-*- *  Time-stamp: "2011-01-09 04:07:40 hmmr"
/*
 *       File name:  ui/scoring-facility.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  scoring facility
 *
 *         License:  GPL
 */




#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glade/glade.h>
#include <cairo-svg.h>

#include <samplerate.h>

#include "../libagh/iface.h"
#include "../libagh/iface-glib.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"


// saved settings

gboolean
	AghUseSigAnOnNonEEGChannels = FALSE;

unsigned
	AghBWFOrder	= 1;
float	AghBWFCutoff	= 3.0;

//int	AghDZCDFStepFine = 1;
float	AghDZCDFStep	= .125,
	AghDZCDFSigma	= 2;
size_t	AghDZCDFSmooth  = 3;

size_t	AghEnvTightness = 4;

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
	*eScoringFacPageSize;

static GtkWidget
	*cScoringFacPageViews,
	*daScoringFacHypnogram,
	*bScoringFacBack,
	*bScoringFacForward,
	*lScoringFacTotalPages,
	*lScoringFacClockTime,
	*lScoringFacPercentScored,
	*eScoringFacCurrentPage,
	*lScoringFacCurrentPos,
	*lScoreStatsNREMPercent,
	*lScoreStatsREMPercent,
	*lScoreStatsWakePercent,
	*lScoringFacCurrentStage,
	*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
	*bScoreREM,   *bScoreWake,  *bScoreMVT,
	*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored,
	*lScoringFacHint,
	*sbSF,

	*mSFPage, *mSFPageSelection, *mSFPageSelectionInspectChannels,
	*mSFPower, *mSFScore, *mSFSpectrum,
	*iSFPageShowOriginal, *iSFPageShowProcessed, *iSFPageShowDZCDF, *iSFPageShowEnvelope,

	*wPattern,
	*daPatternSelection,
	*lPatternCaption,
	*bPatternFindNext,
	*bPatternFindPrevious,
	*ePatternFilterOrder,
	*ePatternFilterCutoff,
	*ePatternEnvTightness,
//	*ePatternDZCDFStepFine,
	*ePatternDZCDFStep,
	*ePatternDZCDFSigma,
	*ePatternDZCDFSmooth,
	*lPatternSimilarity,
	*ePatternParameterA,
	*ePatternParameterB,
	*ePatternParameterC;

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


GdkColor
	__fg1__[cTOTAL_SF],
	__bg1__[cTOTAL_SF];

static GdkColormap *__cmap;




static void
change_fg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( __cmap, &__fg1__[c], 1);
	gtk_color_button_get_color( cb, &__fg1__[c]);
	gdk_colormap_alloc_color( __cmap, &__fg1__[c], FALSE, TRUE);
}
static void
change_bg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( __cmap, &__bg1__[c], 1);
	gtk_color_button_get_color( cb, &__bg1__[c]);
	gdk_colormap_alloc_color( __cmap, &__bg1__[c], FALSE, TRUE);

	__fg1__[c] = *contrasting_to( &__bg1__[c]);
//	printf( "%4d:  %5d %5d %5d :: %5d %5d %5d\n", c, __bg1__[c].red, __bg1__[c].green, __bg1__[c].blue, __fg1__[c].red, __fg1__[c].green, __fg1__[c].blue);
}





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

	__cmap = gtk_widget_get_colormap( daScoringFacHypnogram);

//	gtk_widget_modify_bg( daScoringFacHypnogram, GTK_STATE_NORMAL, &__bg1__[cHYPNOGRAM]);

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
	     !(iSFPageShowEnvelope	= glade_xml_get_widget( xml, "iSFPageShowEnvelope")) )
		return -1;

      // ----- wPattern
	if ( !(wPattern			= glade_xml_get_widget( xml, "wPattern")) ||
	     !(daPatternSelection	= glade_xml_get_widget( xml, "daPatternSelection")) ||
	     !(bPatternFindPrevious	= glade_xml_get_widget( xml, "bPatternFindPrevious")) ||
	     !(bPatternFindNext		= glade_xml_get_widget( xml, "bPatternFindNext")) ||
	     !(lPatternCaption		= glade_xml_get_widget( xml, "lPatternCaption")) ||
	     !(ePatternFilterOrder	= glade_xml_get_widget( xml, "ePatternFilterOrder")) ||
	     !(ePatternFilterCutoff	= glade_xml_get_widget( xml, "ePatternFilterCutoff")) ||
//	     !(ePatternDZCDFStepFine	= glade_xml_get_widget( xml, "ePatternDZCDFStepFine")) ||
	     !(ePatternDZCDFStep	= glade_xml_get_widget( xml, "ePatternDZCDFStep")) ||
	     !(ePatternDZCDFSigma	= glade_xml_get_widget( xml, "ePatternDZCDFSigma")) ||
	     !(ePatternDZCDFSmooth	= glade_xml_get_widget( xml, "ePatternDZCDFSmooth")) ||
	     !(lPatternSimilarity	= glade_xml_get_widget( xml, "lPatternSimilarity")) ||
	     !(ePatternEnvTightness	= glade_xml_get_widget( xml, "ePatternEnvTightness")) ||
	     !(ePatternParameterA	= glade_xml_get_widget( xml, "ePatternParameterA")) ||
	     !(ePatternParameterB	= glade_xml_get_widget( xml, "ePatternParameterB")) ||
	     !(ePatternParameterC	= glade_xml_get_widget( xml, "ePatternParameterC")) )
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

typedef struct {
	const gchar
	          *name,
		  *type;
	TRecRef    rec_ref;

	float	  *signal_filtered,
		  *signal_original;
	size_t	   n_samples,
		   samplerate;

	float	   signal_display_scale;
	GtkWidget *da_page;
	gboolean   draw_original_signal,
		   draw_processed_signal;

	float     *envelope_upper,
		  *envelope_lower;
	gboolean   draw_envelope;

	float	  *signal_course;
	float	   bwf_cutoff;
	unsigned   bwf_order;
	gboolean   draw_course;

	float	  *signal_breadth;
	unsigned   env_tightness;

	float	  *signal_dzcdf;
	float	   dzcdf_step,
		   dzcdf_sigma;
	size_t	   dzcdf_smooth;
	gboolean   draw_dzcdf;

	struct SUnfazer
		  *unfazers;
	guint	   n_unfazers;

	float	   binsize;
	GArray	  *power;
	float	   from, upto;
	float	   power_display_scale;

	GArray	  *power_in_bands;
	gshort	   focused_band,
		   uppermost_band;
	GtkWidget *da_power;
	gboolean   draw_bands;

	float	  *spectrum,  // per page, is volatile
		   spectrum_max;
	guint	   spectrum_upper_freq;
	gushort	   n_bins,
		   last_spectrum_bin;
	GtkWidget *da_spectrum;
	gboolean   draw_spectrum_absolute;

	size_t	  *artifacts;
	size_t     n_artifacts;
	gfloat	   dirty_percent;

	GArray    *emg_fabs_per_page;
	GtkWidget *da_emg_profile;
	gfloat     emg_scale;

	GtkWidget *expander,
		  *vbox,
		  *menu_item;
	gulong     expose_handler_id;

} SChannelPresentation;


static void
__destroy_ch( SChannelPresentation *Ch)
{
	if ( Ch->signal_filtered )  { free( Ch->signal_filtered);  Ch->signal_filtered = NULL; }
	if ( Ch->signal_original )  { free( Ch->signal_original);  Ch->signal_original = NULL; }
	if ( Ch->envelope_upper )   { free( Ch->envelope_upper );  Ch->envelope_upper  = NULL; }
	if ( Ch->envelope_lower )   { free( Ch->envelope_lower );  Ch->envelope_lower  = NULL; }
	if ( Ch->signal_course )    { free( Ch->signal_course  );  Ch->signal_course   = NULL; }
	if ( Ch->signal_breadth )   { free( Ch->signal_breadth );  Ch->signal_breadth  = NULL; }
	if ( Ch->signal_dzcdf )     { free( Ch->signal_dzcdf   );  Ch->signal_dzcdf    = NULL; }
	if ( Ch->spectrum )         { free( Ch->spectrum);         Ch->spectrum        = NULL; }
	if ( Ch->unfazers )  	    { free( Ch->unfazers);         Ch->unfazers        = NULL; }
	if ( Ch->artifacts )  	    { free( Ch->artifacts);        Ch->artifacts       = NULL; }
	if ( Ch->power )             { g_array_free( Ch->power,             TRUE); Ch->power             = NULL; }
	if ( Ch->emg_fabs_per_page ) { g_array_free( Ch->emg_fabs_per_page, TRUE); Ch->emg_fabs_per_page = NULL; }
	if ( Ch->power_in_bands ) {
		for ( gushort b = 0; b < Ch->power_in_bands->len; ++b )
			g_array_free( Ai (Ch->power_in_bands, GArray*, b), TRUE);
		g_array_free( Ch->power_in_bands, TRUE);
		Ch->power_in_bands = NULL;
	}
}





static size_t	__signal_length; // in sec
static size_t	__total_pages, __fft_pagesize;
static size_t	__cur_page,
		__cur_page_app,
		__cur_pos_hr, __cur_pos_min, __cur_pos_sec;
static float
	__sane_signal_display_scale,
	__sane_power_display_scale = 2.5e-5;




static GArray	*HH;
static TEDFRef	__source_ref;  // the core structures allow for multiple edf
                               // sources providing signals for a single episode;
                               // keeping only one __source_ref here will, then,
                               // read/write scores in this source's histogram;
// -- but it's essentially not a problem since all edf sources will still converge
//    to the same .histogram file
static time_t	__start_time;
static GArray	*__hypnogram;







static guint	__pagesize_item = 4;  // pagesize as currently displayed

#define PSZ  AghDisplayPageSizeValues[AghDisplayPageSizeItem]
#define APSZ AghDisplayPageSizeValues[__pagesize_item]
#define PS_IS_RIGHT (AghDisplayPageSizeItem==__pagesize_item)

#define P2AP(p)  (guint)((p) * (float)PSZ / APSZ)
#define AP2P(p)  (guint)((p) * (float)APSZ / PSZ)




static float
__calibrate_display_scale( const float *signal, size_t over, float fit)
{
	float max_over = 0.;
	for ( size_t i = 0; i < over; ++i )
		if ( max_over < signal[i] )
			max_over = signal[i];
	return fit / max_over;
}


static void
__calculate_dirty_percent( SChannelPresentation *Ch)
{
	size_t dirty_smpl = 0;
	size_t i, a;
	for ( i = a = 0; i < Ch->n_artifacts; ++i, a += 2 )
		dirty_smpl += (Ch->artifacts[a+1] - Ch->artifacts[a]);
	Ch->dirty_percent = (float) dirty_smpl / Ch->n_samples * 100;
}




static gboolean __have_unsaved_scores;


#define CH_IS_EXPANDED \
	gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander))
#define REDRAW_ALL \
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed")
static gboolean __suppress_redraw;


static void __repaint_score_stats();

gboolean cScoringFacPageViewExpander_activate_cb( GtkExpander*, gpointer);
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

void iSFPageSelectionInspectMany_activate_cb( GtkMenuItem*, gpointer);

// vain attempt to find ways to enable multiple scoring facilities
static struct SSubject *__our_j;
static const char *__our_d, *__our_e;
static size_t __n_visible;

gboolean
agh_prepare_scoring_facility( struct SSubject *_j, const char *_d, const char *_e)
{
	set_cursor_busy( TRUE, wMainWindow);

      // prepare structures for the first viewing
	if ( !HH ) {
		HH  = g_array_new( FALSE, TRUE, sizeof(SChannelPresentation));
		__hypnogram = g_array_new( FALSE,  TRUE, sizeof(gchar));
	}

      // clean up after previous viewing
	gtk_container_foreach( GTK_CONTAINER (cScoringFacPageViews),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	gtk_container_foreach( GTK_CONTAINER (mSFPageSelectionInspectChannels),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	guint	h, our_h;
	for ( h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		__destroy_ch(Ch);
	}
	__n_visible = 0;

	const struct SEDFFile *edf_file __attribute__ ((unused));
      // copy arguments into our private variables
	__our_j = _j, __our_d = _d, __our_e = _e;  // deadbeef

      // set up channel representations
	g_array_set_size( HH, AghHs);

	__signal_length = 0; // is set to the longest signal, below
	__source_ref = NULL; // if no measurements are found at all, this will remain NULL
	__sane_signal_display_scale = __sane_power_display_scale = 0.;

	for ( h = our_h = 0; h < AghHs; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, our_h);
		Ch->rec_ref = agh_msmt_find_by_jdeh( _j->name, _d, _e, AghHH[h]);
		if ( Ch->rec_ref == NULL ) {
//			fprintf( stderr, "agh_prepare_scoring_facility(): no measurement matching (%s, %s, %s, %s)\n",
//				 _j->name, _d, _e, AghHH[h]);
//			Ch->signal_filtered = Ch->signal_original = Ch->spectrum = NULL;
//			Ch->power = Ch->power_in_bands = Ch->af_track = Ch->emg_fabs_per_page = NULL;
//			Ch->unfazers = NULL;
			continue;
		}
		Ch->name = AghHH[h];

		Ch->type = agh_msmt_get_signal_type( Ch->rec_ref);

		if ( our_h == 0 ) {
			__source_ref = agh_msmt_get_source( Ch->rec_ref);
			edf_file = agh_edf_get_info_from_sourceref( __source_ref, NULL);
			__start_time = edf_file->start_time;

		      // get scores
			__total_pages = agh_edf_get_scores_as_garray( __source_ref,
								      __hypnogram, &__fft_pagesize);
			__have_unsaved_scores = FALSE;
		}
		++our_h;

	      // get signal data
		snprintf_buf( "(%u/%zu) %s: read edf...", our_h, edf_file->NSignals, Ch->name);
		BUF_ON_STATUS_BAR;

		Ch->n_samples =
			agh_msmt_get_signal_original_as_float( Ch->rec_ref,
							       &Ch->signal_original, &Ch->samplerate, NULL);
		if ( Ch->n_samples ) {
			__n_visible++;

			agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
							       &Ch->signal_filtered, NULL, NULL);

			if ( AghUseSigAnOnNonEEGChannels || strcmp( Ch->type, "EEG") == 0 ) {
				// and signal course
				snprintf_buf( "(%u/%zu) %s: low-pass...", our_h, edf_file->NSignals, Ch->name);
				BUF_ON_STATUS_BAR;
				agh_signal_get_course( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
						       Ch->bwf_cutoff = AghBWFCutoff, Ch->bwf_order = AghBWFOrder, 1,
						       &Ch->signal_course);

				// and envelope and breadth
				snprintf_buf( "(%u/%zu) %s: envelope...", our_h, edf_file->NSignals, Ch->name);
				BUF_ON_STATUS_BAR;
				agh_signal_get_envelope( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
							 &Ch->envelope_lower,
							 &Ch->envelope_upper,
							 Ch->env_tightness = AghEnvTightness,
							 &Ch->signal_breadth);

				// and dzcdf
				snprintf_buf( "(%u/%zu) %s: zerocrossings...", our_h, edf_file->NSignals, Ch->name);
				BUF_ON_STATUS_BAR;
				agh_signal_get_dzcdf( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
						      &Ch->signal_dzcdf,
						      Ch->dzcdf_step = AghDZCDFStep,
						      Ch->dzcdf_sigma = AghDZCDFSigma,
						      Ch->dzcdf_smooth = AghDZCDFSmooth);
			} else
				Ch->signal_course =
					Ch->envelope_upper = Ch->envelope_lower = Ch->signal_breadth =
					Ch->signal_dzcdf = NULL;

		      // artifacts
			Ch->n_artifacts = agh_edf_get_artifacts( __source_ref, Ch->name,
								 &Ch->artifacts);

		      // unfazers
			Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
							       Ch->name,
							       &Ch->unfazers);

		      // expander and vbox
			snprintf_buf( "%s <b>%s</b>", Ch->type, Ch->name);
			Ch->expander = gtk_expander_new( __buf__);
			gtk_expander_set_use_markup( GTK_EXPANDER (Ch->expander), TRUE);

			gtk_box_pack_start( GTK_BOX (cScoringFacPageViews),
					    Ch->expander, TRUE, TRUE, 0);
			gtk_expander_set_expanded( GTK_EXPANDER (Ch->expander),
						   TRUE);
			g_signal_connect_after( Ch->expander, "activate",
					  G_CALLBACK (cScoringFacPageViewExpander_activate_cb),
					  (gpointer)Ch);

			gtk_container_add( GTK_CONTAINER (Ch->expander),
					   Ch->vbox = gtk_vbox_new( FALSE, 0));

		      // set up page view
			__signal_length = MAX( __signal_length, Ch->n_samples / Ch->samplerate);

			Ch->signal_display_scale = __calibrate_display_scale( Ch->signal_filtered, APSZ * Ch->samplerate,
									      AghSFDAPageHeight / 2);

			gtk_container_add( GTK_CONTAINER (Ch->vbox),
					   Ch->da_page = gtk_drawing_area_new());
			g_object_set( G_OBJECT (Ch->da_page),
				      "app-paintable", TRUE,
				      "height-request", AghSFDAPageHeight,
				      NULL);
			Ch->expose_handler_id =
				g_signal_connect_after( Ch->da_page, "expose-event",
							G_CALLBACK (daScoringFacPageView_expose_event_cb),
							(gpointer)Ch);
			g_signal_connect_after( Ch->da_page, "button-press-event",
						G_CALLBACK (daScoringFacPageView_button_press_event_cb),
						(gpointer)Ch);
			g_signal_connect_after( Ch->da_page, "button-release-event",
						G_CALLBACK (daScoringFacPageView_button_release_event_cb),
						(gpointer)Ch);
			g_signal_connect_after( Ch->da_page, "motion-notify-event",
						G_CALLBACK (daScoringFacPageView_motion_notify_event_cb),
						(gpointer)Ch);
			g_signal_connect_after( Ch->da_page, "scroll-event",
						G_CALLBACK (daScoringFacPageView_scroll_event_cb),
						(gpointer)Ch);
			gtk_widget_add_events( Ch->da_page,
					       (GdkEventMask)
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_DRAG_MOTION);

		      // set up PSD profile view
			if ( agh_signal_type_is_fftable( Ch->type) ) {

				snprintf_buf( "(%u/%zu) %s: power...", our_h, edf_file->NSignals, Ch->name);
				BUF_ON_STATUS_BAR;

				// power in a single bin
				Ch->power = g_array_new( FALSE, FALSE, sizeof(float));
				// the first call to get power course is *_as_garray; others will use *_direct
				agh_msmt_get_power_course_in_range_as_float_garray( Ch->rec_ref,
										    Ch->from, Ch->upto,
										    Ch->power);
				// power spectrum (for the first page)
				Ch->n_bins = Ch->last_spectrum_bin =
					agh_msmt_get_power_spectrum_as_float( Ch->rec_ref, 0,
									      &Ch->spectrum, &Ch->spectrum_max);
				// will be reassigned in REDRAW_ALL
				Ch->spectrum_upper_freq =
					Ch->n_bins * (Ch->binsize = agh_msmt_get_binsize( Ch->rec_ref));

				// power in bands
				gshort n_bands = 0;
				while ( n_bands < AGH_BAND__TOTAL )
					if ( AghFreqBands[n_bands][0] >= Ch->spectrum_upper_freq )
						break;
					else
						++n_bands;
				Ch->uppermost_band = n_bands-1;
				Ch->power_in_bands = g_array_new( FALSE, FALSE, sizeof(GArray*));
				g_array_set_size( Ch->power_in_bands, n_bands);
				for ( gushort b = 0; b < n_bands; ++b ) {
					GArray *this_band = Ai (Ch->power_in_bands, GArray*, b) =
						g_array_new( FALSE, FALSE, sizeof(float));
					agh_msmt_get_power_course_in_range_as_float_garray( Ch->rec_ref,
											    AghFreqBands[b][0], AghFreqBands[b][1],
											    this_band);
				}

				// delta comes first, calibrate display scale against it
				Ch->power_display_scale = __calibrate_display_scale( (float*)Ai (Ch->power_in_bands, GArray*, 0)->data,
										     Ch->power->len,
										     AghSFDAPageHeight);

				// switches
				Ch->draw_spectrum_absolute = TRUE;
				Ch->draw_bands = TRUE;
				Ch->focused_band = 0; // delta

				__calculate_dirty_percent( Ch);

				Ch->emg_fabs_per_page = NULL;

				Ch->from = AghOperatingRangeFrom, Ch->upto = AghOperatingRangeUpto;

				__signal_length = MAX( __signal_length,
						       Ch->power->len * PSZ);

				GtkWidget *hbox;
				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   hbox = gtk_hbox_new( FALSE, 0));
				gtk_container_add( GTK_CONTAINER (hbox),
						   Ch->da_power = gtk_drawing_area_new());
				gtk_container_add_with_properties( GTK_CONTAINER (hbox),
								   Ch->da_spectrum = gtk_drawing_area_new(),
								   "expand", FALSE,
								   NULL);
			      // profile pane
				g_object_set( G_OBJECT (Ch->da_power),
					      "app-paintable", TRUE,
					      "height-request", AghSFDAPowerProfileHeight,
					      NULL);
				g_signal_connect_after( Ch->da_power, "expose-event",
							G_CALLBACK (daScoringFacPSDProfileView_expose_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_power, "button-press-event",
							G_CALLBACK (daScoringFacPSDProfileView_button_press_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_power, "scroll-event",
							G_CALLBACK (daScoringFacPSDProfileView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_power,
						       (GdkEventMask) GDK_BUTTON_PRESS_MASK);

			      // spectrum pane
				g_object_set( G_OBJECT (Ch->da_spectrum),
					      "app-paintable", TRUE,
					      "width-request", AghSFDASpectrumWidth,
					      NULL);
				gtk_widget_modify_fg( Ch->da_spectrum, GTK_STATE_NORMAL, &__fg1__[cSPECTRUM]);
				gtk_widget_modify_bg( Ch->da_spectrum, GTK_STATE_NORMAL, &__bg1__[cSPECTRUM]);

				g_signal_connect_after( Ch->da_spectrum, "expose-event",
							G_CALLBACK (daScoringFacSpectrumView_expose_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_spectrum, "button-press-event",
							G_CALLBACK (daScoringFacSpectrumView_button_press_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_spectrum, "scroll-event",
							G_CALLBACK (daScoringFacSpectrumView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_spectrum, (GdkEventMask) GDK_BUTTON_PRESS_MASK);

			} else {
				Ch->power = Ch->power_in_bands = NULL;
				Ch->spectrum = NULL;
			}

			if ( strcmp( Ch->type, "EMG") == 0 ) {
				Ch->emg_fabs_per_page = g_array_new( FALSE, FALSE, sizeof(float));
				g_array_set_size( Ch->emg_fabs_per_page, __total_pages);
				float largest = 0.;
				size_t i;
				snprintf_buf( "(%u/%zu) %s: EMG...", our_h, edf_file->NSignals, Ch->name);
				BUF_ON_STATUS_BAR;
				for ( i = 0; i < __total_pages; ++i ) {
					float	current = Ai (Ch->emg_fabs_per_page, float, i)
						= max_fabs( Ch->signal_original, i * PSZ * Ch->samplerate, (i+1) * PSZ * Ch->samplerate);
					if ( largest < current )
						largest = current;
				}
				Ai (Ch->emg_fabs_per_page, float, i)  // last page, likely incomplete
					= max_fabs( Ch->signal_original, i * PSZ * Ch->samplerate, Ch->n_samples);

				Ch->emg_scale = AghSFDAEMGProfileHeight/2 / largest;

				GtkWidget *hbox, *da_void;
				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   hbox = gtk_hbox_new( FALSE, 0));
				gtk_container_add( GTK_CONTAINER (hbox),
						   Ch->da_emg_profile = gtk_drawing_area_new());
				gtk_container_add_with_properties( GTK_CONTAINER (hbox),
								   da_void = gtk_drawing_area_new(),
								   "expand", FALSE,
								   NULL);
				g_object_set( G_OBJECT (Ch->da_emg_profile),
					      "app-paintable", TRUE,
					      "height-request", AghSFDAEMGProfileHeight,
					      NULL);
				g_object_set( G_OBJECT (da_void),
					      "width-request", AghSFDASpectrumWidth,
					      NULL);
				g_signal_connect_after( Ch->da_emg_profile, "expose-event",
							G_CALLBACK (daScoringFacEMGProfileView_expose_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_emg_profile, "button-press-event",
							G_CALLBACK (daScoringFacEMGProfileView_button_press_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_emg_profile, "scroll-event",
							G_CALLBACK (daScoringFacEMGProfileView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_emg_profile,
						       (GdkEventMask) GDK_BUTTON_PRESS_MASK);
			} else {
				Ch->emg_fabs_per_page = NULL;
				Ch->da_emg_profile = NULL;
			}

			Ch->draw_processed_signal = TRUE;
			Ch->draw_original_signal = FALSE;
			Ch->draw_dzcdf = FALSE;
			Ch->draw_envelope = FALSE;


		      // add channel under mSFPageSelectionInspectChannels
			gtk_container_add( GTK_CONTAINER (mSFPageSelectionInspectChannels),
					   Ch->menu_item = gtk_check_menu_item_new_with_label( Ch->name));
			g_object_set( G_OBJECT (Ch->menu_item),
				      "visible", TRUE,
				      NULL);
		} else
			Ch->da_page = NULL;
	}

	if ( __source_ref == NULL )
		return FALSE;

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
	__sane_signal_display_scale = __sane_power_display_scale = 0.;
	size_t n_with_power = 0;
	for ( h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		__sane_signal_display_scale += Ch->signal_display_scale;
		if ( Ch->power ) {
			++n_with_power;
			__sane_power_display_scale += Ch->power_display_scale;
		}
	}
	__sane_signal_display_scale /= __n_visible;
	__sane_power_display_scale /= n_with_power;
	for ( h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		Ch->signal_display_scale = __sane_signal_display_scale;
		if ( Ch->power )
			Ch->power_display_scale = __sane_power_display_scale;
	}


      // set up other controls
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

	gtk_widget_set_tooltip_markup( lScoringFacHint, __tooltips[AGH_TIP_GENERAL]);

	return TRUE;
}











// -------------------- Page


static void
__draw_signal( float *signal, size_t n_samples, float scale,
	       guint width, guint vdisp,
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









static SChannelPresentation  // for menus & everything else
	*__clicked_channel;

// general marquee
static GtkWidget *__marking_in_widget;
static double __marquee_start, __marquee_virtual_end;

#define MARK_REGION_PRE \
	SChannelPresentation *Ch = __clicked_channel;	\
	int wd;							\
	gdk_drawable_get_size( __clicked_channel->da_page->window, &wd, NULL); \
	float x1 = __marquee_start, x2 = __marquee_virtual_end;		\
	if ( x1 > x2 ) { float _ = x1; x1 = x2, x2 = _; }		\
	if ( x1 < 0. ) x1 = 0.;						\
	size_t	sa = (__cur_page_app + x1/wd) * APSZ * Ch->samplerate,	\
		sz = (__cur_page_app + x2/wd) * APSZ * Ch->samplerate;	\
	if ( sa > __signal_length * Ch->samplerate )			\
		return;							\
	if ( sz > __signal_length * Ch->samplerate )			\
		sz = __signal_length * Ch->samplerate;


static void
__mark_region_as_artifact( gchar value)
{
	// get boundaries in correct order and convert to samples
	MARK_REGION_PRE;

	(value == 'x' ? agh_edf_mark_artifact : agh_edf_clear_artifact)( __source_ref, Ch->name,
									 sa, sz);
	free( (void*)Ch->artifacts);
	Ch->n_artifacts = agh_edf_get_artifacts( __source_ref, Ch->name,
						 &Ch->artifacts);
	__calculate_dirty_percent( Ch);

	agh_msmt_get_signal_filtered_as_float( Ch->rec_ref,
					       &Ch->signal_filtered, NULL, NULL);

	if ( Ch->power ) {
		agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
								    Ch->from, Ch->upto,
								    (float*)Ch->power->data);
		for ( gushort b = 0; b <= Ch->uppermost_band; ++b ) {
			GArray *this_band = Ai (Ch->power_in_bands, GArray*, b);
			agh_msmt_get_power_course_in_range_as_float_direct( Ch->rec_ref,
									    AghFreqBands[b][0], AghFreqBands[b][1],
									    (float*)this_band->data);
		}

		agh_msmt_get_power_spectrum_as_float( Ch->rec_ref, __cur_page,
						      &Ch->spectrum, &Ch->spectrum_max);

		gtk_widget_queue_draw( Ch->da_power);
		gtk_widget_queue_draw( Ch->da_spectrum);
	}
	gtk_widget_queue_draw( Ch->da_page);
}






static size_t __context_pad = 100;

static size_t
	__pattern_sa, __pattern_sz, __pattern_wd,
	__pattern_context_before, __pattern_context_after;
static float *__pattern_snippet = NULL;

static int __da_PatternSelection_height;
static float __pattern_display_scale = 1.2;


static size_t __last_find = 0;


static void
__mark_region_as_pattern()
{
	MARK_REGION_PRE;

	__pattern_sa = sa;
	__pattern_sz = sz;
	__pattern_context_before = (sa < __context_pad) ? sa : __context_pad;
	__pattern_context_after  = (sz + __context_pad > Ch->n_samples) ? Ch->n_samples - sz : __context_pad;

	size_t	run = (sz - sa),
		full_sample = __pattern_context_before + run + __pattern_context_after;

	printf( "run = %zu\n", run);
	if ( __pattern_snippet )
		free( __pattern_snippet);
	assert (__pattern_snippet = malloc( full_sample * sizeof(float)));
	memcpy( __pattern_snippet,
		&Ch->signal_filtered[sa - __pattern_context_before],
		full_sample * sizeof(float));

	FAFA;
	__pattern_wd = (float)run/(Ch->samplerate * APSZ) * wd;

	__last_find = (size_t)-1;

	g_object_set( G_OBJECT (daPatternSelection),
		      "height-request", __da_PatternSelection_height = AghSFDAPageHeight * __pattern_display_scale,
		      "width-request", (int)__pattern_wd*__pattern_display_scale,
		      NULL);

//	gtk_widget_queue_draw( daPatternSelection);
	gtk_widget_show_all( wPattern);
}

#undef MARK_REGION_PRE





gboolean
daPatternSelection_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = __clicked_channel;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	size_t	run = (__pattern_sz - __pattern_sa),
		full_sample = __pattern_context_before + run + __pattern_context_after;

	float	display_scale = Ch->signal_display_scale* __pattern_display_scale;

	cairo_t *cr = gdk_cairo_create( wid->window);
	cairo_set_source_rgb( cr, 1., 1., 1.);
	cairo_paint( cr);
	cairo_stroke( cr);

      // snippet
	cairo_set_source_rgb( cr, 0.1, 0.1, 0.1);
	cairo_set_line_width( cr, .8);
	__draw_signal( &__pattern_snippet[__pattern_context_before], run, display_scale,
		       wd, ht/3, cr, FALSE);
	if ( __last_find != (size_t)-1 )
		__draw_signal( &Ch->signal_filtered[__last_find], run, display_scale,
			       wd, ht*2/3, cr, FALSE);

	cairo_stroke( cr);

      // envelope
	float	*breadth = NULL,
		*env_u = NULL, *env_l = NULL,
		*course = NULL,
		*dzcdf = NULL;

	{
		size_t	tightness = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightness));
		if ( agh_signal_get_envelope( __pattern_snippet, full_sample, Ch->samplerate,
					      &env_l, &env_u,
					      tightness,
					      &breadth) == 0 ) {
			gtk_widget_set_sensitive( bPatternFindNext, FALSE);
			gtk_widget_set_sensitive( bPatternFindPrevious, FALSE);
			cairo_set_source_rgba( cr, 0., 0., 0., .6);
			cairo_set_font_size( cr, 15);
			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_text_extents_t extents;
			snprintf_buf( "%s", "The sample is too short");
			cairo_text_extents( cr, __buf__, &extents);
			cairo_move_to( cr, wd/2 - extents.width/2, 18);
			cairo_show_text( cr, __buf__);
			cairo_stroke( cr);
			goto out;
		} else {
			gtk_widget_set_sensitive( bPatternFindNext, TRUE);
			gtk_widget_set_sensitive( bPatternFindPrevious, TRUE);
		}

		cairo_set_source_rgba( cr, 0.3, 0.2, 0.8, .5);
		cairo_set_line_width( cr, .5);
		__draw_signal( &env_u[__pattern_context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		__draw_signal( &env_l[__pattern_context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		cairo_stroke( cr);
	}

      // low-pass filter
	{
		unsigned order = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrder));
		float cutoff = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoff));
		agh_signal_get_course( __pattern_snippet, full_sample, Ch->samplerate,
				       order, cutoff, 1,
				       &course);

		cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
		cairo_set_line_width( cr, 3.);
		__draw_signal( &course[__pattern_context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		cairo_stroke( cr);
	}

      // dzcdf
	{
		float	step   = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStep)),
			sigma  = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma)),
			smooth = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth));
		agh_signal_get_dzcdf( __pattern_snippet, full_sample, Ch->samplerate,
				      &dzcdf,
				      step, sigma, smooth);
		float	dzcdf_display_scale,
			avg = 0.;
		for ( size_t i = __pattern_sa; i < __pattern_sz; ++i )
			avg += dzcdf[i];
		avg /= run;
		dzcdf_display_scale = ht/5 / avg;

		cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
		cairo_set_line_width( cr, 1.);
		__draw_signal( &dzcdf[__pattern_context_before], run, display_scale,
			       wd, ht/2-5, cr, FALSE);
 		cairo_stroke( cr);
		cairo_set_line_width( cr, .5);
		cairo_rectangle( cr, 0, ht/2-5, wd, ht/2-4);
		cairo_stroke( cr);
	}
out:
	cairo_destroy( cr);

	free( dzcdf);
	free( breadth);
	free( course);
	free( env_l);
	free( env_u);

	return TRUE;
}




/*
void
ePatternDZCDFStepFine_toggled_cb( GtkToggleButton *togglebutton, gpointer unused)
{
	gtk_widget_set_sensitive( ePatternDZCDFStep,
				  !gtk_toggle_button_get_active( togglebutton));
}
*/


void
bPatternFind_clicked_cb( GtkButton *button, gpointer unused)
{
	set_cursor_busy( TRUE, wPattern);

	SChannelPresentation *Ch = __clicked_channel;

	size_t	run = __pattern_sz - __pattern_sa;
	struct SSignalPatternPrimer primer = {
		.data		= __pattern_snippet,
		.n_samples	= __pattern_context_before + run + __pattern_context_after,
		.context_before	= __pattern_context_before,
		.context_after	= __pattern_context_after,
		.samplerate	= Ch->samplerate,
		.bwf_order	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrder)),
		.bwf_cutoff	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoff)),
		.bwf_scale	= 1,
		.dzcdf_step	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStep)),
		.dzcdf_sigma	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma)),
		.dzcdf_smooth	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth)),
		.env_tightness	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightness)),
	};
      // reprocess the field if the pattern is set up with different tightness etc
	if ( primer.bwf_order  != Ch->bwf_order ||
	     primer.bwf_cutoff != Ch->bwf_cutoff ) {
		free( Ch->signal_course);
		agh_signal_get_course( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
				       Ch->bwf_cutoff = primer.bwf_cutoff, Ch->bwf_order = primer.bwf_order, 1,
				       &Ch->signal_course);
	}
	if ( primer.env_tightness != Ch->env_tightness ) {
		free( Ch->envelope_lower);
		free( Ch->envelope_upper);
		free( Ch->signal_breadth);
		agh_signal_get_envelope( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
					 &Ch->envelope_lower,
					 &Ch->envelope_upper,
					 primer.env_tightness,
					 &Ch->signal_breadth);
	}
	if ( primer.dzcdf_sigma  != Ch->dzcdf_sigma ||
	     primer.dzcdf_step   != Ch->dzcdf_step ||
	     primer.dzcdf_smooth != Ch->dzcdf_smooth ) {
		free( Ch->signal_dzcdf);
		agh_signal_get_dzcdf( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
				      &Ch->signal_dzcdf,
				      Ch->dzcdf_step = primer.dzcdf_step,
				      Ch->dzcdf_sigma = primer.dzcdf_sigma,
				      Ch->dzcdf_smooth = primer.dzcdf_smooth);
	}

	double	a = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterA)),
		b = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterB)),
		c = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterC));

	gboolean
		go_forward = strcmp( gtk_widget_get_name( GTK_WIDGET (button)), "bPatternFindNext") == 0;
	size_t from;
	if ( __last_find == (size_t)-1 )
		from = go_forward
			? __pattern_context_before
			: Ch->n_samples - __pattern_context_before - __pattern_context_after - (__pattern_sz - __pattern_sa);
	else
		from = __last_find + (go_forward ? 10 : -10);
	__last_find =
		agh_signal_find_pattern_( &primer,
					  Ch->signal_course,
					  Ch->signal_breadth,
					  Ch->signal_dzcdf,
					  Ch->n_samples,
					  a, b, c,
					  from,
					  go_forward ? 1 : -1);

	if ( __last_find == (size_t)-1 )
		pop_ok_message( GTK_WINDOW (wPattern), "Not found");
	else {
		__cur_page_app = (__last_find / Ch->samplerate / APSZ);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
		REDRAW_ALL;

		gint wd;
		gdk_drawable_get_size( Ch->da_page->window, &wd, NULL);
		cairo_t *cr = gdk_cairo_create( Ch->da_page->window);
		cairo_set_source_rgb( cr, 1, 0., 0.);
		double vpos = __last_find - (__cur_page_app * APSZ * Ch->samplerate);
		cairo_move_to( cr, vpos/(APSZ*Ch->samplerate) * wd, 15);
		cairo_set_font_size( cr, 18);
		cairo_show_text( cr, "*");
		cairo_stroke( cr);
		cairo_destroy( cr);

		snprintf_buf( "at p. %zu (a = %4.2f, b = %4.2f, c = %4.2f)\n",
			      __cur_page_app+1, primer.match_a, primer.match_b, primer.match_c);
		gtk_label_set_markup( GTK_LABEL (lPatternSimilarity), __buf__);

		gtk_widget_queue_draw( lPatternSimilarity);
		gtk_widget_queue_draw( daPatternSelection);
	}

	set_cursor_busy( FALSE, wPattern);
}




void
bPatternSave_clicked_cb()
{
}







enum {
	SEL_UNF_CHANNEL = 1,
	SEL_UNF_CALIBRATE = 2,
};
static gint __select_state = 0;


static SChannelPresentation
	*__unfazer_offending_channel;
static float
	__unfazer_factor = 0.1;



static guint __crosshair_at;
static gboolean __draw_crosshair = FALSE;

static gboolean __draw_power = TRUE;









gboolean
cScoringFacPageViewExpander_activate_cb( GtkExpander *expander, gpointer userdata)
{
	REDRAW_ALL;
	return TRUE;
}






static gint	__cur_stage;

inline guint SCOREID( gchar c)
{
	guint i = AGH_SCORE_MVT;
	while ( i && c != AghScoreCodes[i] )
		--i;
	return i;
}

gchar *__score_names[] = {
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
static gfloat
__percent_scored()
{
	guint	scored_pages = 0,
		p;
	for ( p = 0; p < __hypnogram->len; ++p )
		scored_pages += (Ai( __hypnogram, gchar, p) != AghScoreCodes[AGH_SCORE_NONE]);
	return (gfloat) scored_pages / p * 100;
}

static gfloat
__percent_NREM()
{
	guint	nrem_pages = 0,
		p;
	for ( p = 0; p < __hypnogram->len; ++p )
		nrem_pages += (Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_NREM1] ||
			       Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_NREM2] ||
			       Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_NREM3] ||
			       Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_NREM4] );
	return (gfloat) nrem_pages / p * 100;
}

static gfloat
__percent_REM()
{
	guint	rem_pages = 0,
		p;
	for ( p = 0; p < __hypnogram->len; ++p )
		rem_pages += (Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_REM] );
	return (gfloat) rem_pages / p * 100;
}

static gfloat
__percent_Wake()
{
	guint	wake_pages = 0,
		p;
	for ( p = 0; p < __hypnogram->len; ++p )
		wake_pages += (Ai (__hypnogram, gchar, p) == AghScoreCodes[AGH_SCORE_WAKE]);
	return (gfloat) wake_pages / p * 100;
}




static guint __pagesize_ticks[] = {
	5, 5, 3, 4, 6, 12, 24, 30
};




static void
__draw_page( cairo_t *cr, SChannelPresentation *Ch, guint wd, guint ht)
{
	guint i;

	double signal_r, signal_g, signal_b;
      // background
	guint c = PS_IS_RIGHT ? SCOREID (Ai (__hypnogram, gchar, __cur_page)) : cSIGNAL_SCORE_NONE;
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
	}

	cairo_stroke( cr);

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

      // unfazer info
	if ( Ch->n_unfazers ) {
		GString *unf_buf = g_string_sized_new( 128);
		g_string_assign( unf_buf, "Unf: ");
		for ( i = 0; i < Ch->n_unfazers; ++i ) {
			g_string_append_printf( unf_buf, "%s: %5.3f%c",
						Ch->unfazers[i].channel, Ch->unfazers[i].factor,
						(i+1 == Ch->n_unfazers) ? ' ' : ';');
		}
		cairo_set_font_size( cr, 9);
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		cairo_move_to( cr, 10, ht-4);
		cairo_show_text( cr, unf_buf->str);
		g_string_free( unf_buf, TRUE);
	}

      // artifacts (changed bg)
	if ( Ch->n_artifacts ) {
		cairo_set_source_rgba( cr,
				       (double)__fg1__[cARTIFACT].red/65536,
				       (double)__fg1__[cARTIFACT].green/65536,
				       (double)__fg1__[cARTIFACT].blue/65536,
				       .6);
		size_t	lpp = APSZ * Ch->samplerate,
			cur_page_start_s =  __cur_page_app      * lpp,
			cur_page_end_s   = (__cur_page_app + 1) * lpp;
		for ( size_t a = 0; a < Ch->n_artifacts; ++a ) {
			if ( (Ch->artifacts[a*2  ] > cur_page_start_s && Ch->artifacts[a*2  ] < cur_page_end_s) ||
			     (Ch->artifacts[a*2+1] > cur_page_start_s && Ch->artifacts[a*2+1] < cur_page_end_s) ) {
				size_t	aa = Ch->artifacts[a*2  ] < cur_page_start_s ? cur_page_start_s : Ch->artifacts[a*2  ],
					az = Ch->artifacts[a*2+1] > cur_page_end_s   ? cur_page_end_s   : Ch->artifacts[a*2+1];
				cairo_rectangle( cr,
						 (float)( aa       % lpp) / lpp * wd, ht*1./3,
						 (float)((az - aa) % lpp) / lpp * wd, ht*1./3);
				cairo_fill( cr);
				cairo_stroke( cr);
			}
			if ( Ch->artifacts[a*2+1] > cur_page_end_s )
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

      // uV scale
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

      // ticks
	cairo_set_font_size( cr, 9);
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, .3);
	for ( i = 0; i < __pagesize_ticks[__pagesize_item]; ++i ) {
		guint tick_pos = i * APSZ / __pagesize_ticks[__pagesize_item];
		cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item], 0);
		cairo_line_to( cr, i * wd / __pagesize_ticks[__pagesize_item], ht);

		cairo_move_to( cr, i * wd / __pagesize_ticks[__pagesize_item] + 5, ht-2);
		snprintf_buf( "%2d", tick_pos);
		cairo_show_text( cr, __buf__);
	}
	cairo_stroke( cr);


      // samples per pixel
	{
		cairo_set_source_rgb( cr,
				      (double)__fg1__[cLABELS_SF].red/65536,
				      (double)__fg1__[cLABELS_SF].green/65536,
				      (double)__fg1__[cLABELS_SF].blue/65536);
		snprintf_buf( "%4.2f spp", (float)Ch->samplerate * APSZ / wd);
		cairo_move_to( cr, wd-60, 15);
		cairo_show_text( cr, __buf__);
	}
}



static void
draw_page_to_widget( GtkWidget *wid, SChannelPresentation *Ch)
{
	cairo_t *cr = gdk_cairo_create( wid->window);

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	__draw_page( cr, Ch, wd, ht);

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


      // marquee
	if ( __marking_in_widget == wid ) {
		float vstart = (__marquee_start < __marquee_virtual_end) ? __marquee_start : __marquee_virtual_end,
			vend = (__marquee_start < __marquee_virtual_end) ? __marquee_virtual_end : __marquee_start;
		cairo_set_source_rgba( cr, .7, .7, .7, .4);
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
draw_page_to_file( const char *fname, SChannelPresentation *Ch,
		   guint width, guint height)
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs = cairo_svg_surface_create( fname, width, height);
	cairo_t *cr = cairo_create( cs);

	__draw_page( cr, Ch, width, height);

	cairo_stroke( cr);
	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif

}


gboolean
daScoringFacPageView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( !CH_IS_EXPANDED || !Ch->n_samples )
		return TRUE;

	draw_page_to_widget( wid, Ch);

	return TRUE;
}






gboolean
daScoringFacPageView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
										    (float*)Ch->power->data);
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
											    (float*)Ch->power->data);
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
				for ( h = 0; h < __n_visible; ++h )
					Ai (HH, SChannelPresentation, h).signal_display_scale = __sane_signal_display_scale;
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
}





gboolean
daScoringFacPageView_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
		for ( guint h = 0; h < __n_visible; ++h ) {
			SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
			if ( CH_IS_EXPANDED && Ch->da_page )
				gtk_widget_queue_draw( Ch->da_page);
		}
	} else if ( __marking_in_widget == wid )
		gtk_widget_queue_draw( wid);

	return TRUE;
}



gboolean
daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
		for ( guint h = 0; h < __n_visible; ++h )
			Ai (HH, SChannelPresentation, h).signal_display_scale =
				Ch->signal_display_scale;
		gtk_widget_queue_draw( cScoringFacPageViews);
	} else
		gtk_widget_queue_draw( wid);

	return TRUE;
}










// -------------------- PSD profile


gboolean
daScoringFacPSDProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
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

	guint i;

      // profile
	if ( Ch->draw_bands ) {
		cairo_set_line_width( cr, 1.);
		cairo_set_font_size( cr, 9);
		for ( gushort b = 0; b <= Ch->uppermost_band; ++b ) {
			GArray *this_band = Ai (Ch->power_in_bands, GArray*, b);
			cairo_set_source_rgb( cr,
					      (double)__fg1__[cBAND_DELTA + b].red/65536,
					      (double)__fg1__[cBAND_DELTA + b].green/65536,
					      (double)__fg1__[cBAND_DELTA + b].blue/65536);
			cairo_move_to( cr, 0.5, -Ai (this_band, float, 0) * Ch->power_display_scale + ht);
			for ( i = 1; i < this_band->len; ++i )
				cairo_line_to( cr, (double)(i+.5) / this_band->len * wd,
					       - Ai (this_band, float, i) * Ch->power_display_scale + ht);
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
		cairo_move_to( cr, 0, Ai (Ch->power, float, 0));
		for ( i = 0; i < Ch->power->len; ++i )
			cairo_line_to( cr, (double)i / Ch->power->len * wd,
				       - Ai (Ch->power, float, i) * Ch->power_display_scale + ht);
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
	guint hours = __signal_length / 3600;
	for ( i = 1; i <= hours; ++i ) {
		guint tick_pos = (gfloat)(i * 3600) / __signal_length * wd;
		cairo_move_to( cr,
			       tick_pos, 0);
		cairo_line_to( cr,
			       tick_pos, 15);
		snprintf_buf( "%2uh", i);
		cairo_move_to( cr, tick_pos+5, 12);
		cairo_show_text( cr, __buf__);
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
}









void bScoringFacForward_clicked_cb();
void bScoringFacBack_clicked_cb();

gboolean
daScoringFacPSDProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
											    (float*)Ch->power->data);
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
											    (float*)Ch->power->data);
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

		for ( size_t h = 0; h < __n_visible; ++h ) {
			SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);;
			if ( Ch->da_power ) {
				gtk_widget_queue_draw( Ch->da_power);
				gtk_widget_queue_draw( Ch->da_spectrum);
			}
		}
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}









gboolean
daScoringFacPSDProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
		gtk_menu_popup( GTK_MENU (mSFPower),
				NULL, NULL, NULL, NULL, 3, event->time);
	    break;
	}

	return TRUE;
}






// ------------- Spectrum

gboolean
daScoringFacSpectrumView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	if ( !PS_IS_RIGHT )
		return TRUE;

	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
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
	cairo_set_line_width( cr, 1);
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
}





gboolean
daScoringFacSpectrumView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
}


gboolean
daScoringFacSpectrumView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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

	for ( size_t h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);;
		if ( Ch->da_power ) {
			gtk_widget_queue_draw( Ch->da_power);
			gtk_widget_queue_draw( Ch->da_spectrum);
		}
	}

	return TRUE;
}









// -------------------- EMG profile

gboolean
daScoringFacEMGProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
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
	for ( i = 0; i < Ch->emg_fabs_per_page->len; ++i ) {
		cairo_move_to( cr, (double)(i+.5) / Ch->emg_fabs_per_page->len * wd,
			       AghSFDAEMGProfileHeight/2
			       - Ai (Ch->emg_fabs_per_page, float, i) * Ch->emg_scale);
		cairo_line_to( cr, (double)(i+.5) / Ch->emg_fabs_per_page->len * wd,
			       AghSFDAEMGProfileHeight/2
			       + Ai (Ch->emg_fabs_per_page, float, i) * Ch->emg_scale);
	}
	cairo_stroke( cr);

      // hour ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, 1);

	cairo_set_font_size( cr, 7);
	guint hours = __signal_length / 3600;
	for ( i = 0; i <= hours; ++i ) {
		guint tick_pos = (gfloat)(i * 3600) / __signal_length * wd;
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
}








gboolean
daScoringFacEMGProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

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
	// these lines can be discontinuous: cannot use gdk_draw_lines in one swoop
	for ( i = 0; i < __hypnogram->len; ++i ) {
		gchar c;
		if ( (c = Ai (__hypnogram, gchar, i)) != AghScoreCodes[AGH_SCORE_NONE] ) {
			gint y = __score_hypn_depth[ SCOREID(c) ];
			cairo_move_to( cr, lroundf( (gfloat) i   /__hypnogram->len * wd), y);
			cairo_line_to( cr, lroundf( (gfloat)(i+1)/__hypnogram->len * wd), y);
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

// if ( !PS_IS_RIGHT )
// 	for ( guint h = 0; h < HH->len; ++h ) {
// 		SChannelPresentation *Ch = &Ai( HH, SChannelPresentation, h);
// 		if ( CH_IS_EXPANDED && Ch->da_power ) {
// 			g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
// 			gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg1__[cSIGNAL_SCORE_NONE]);
// 			gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg1__[cSIGNAL_SCORE_NONE]);
// 			g_signal_handler_unblock( Ch->da_page, Ch->expose_handler_id);
// 		}
// 	}
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

	__cur_stage = SCOREID( Ai (__hypnogram, gchar, __cur_pos / PSZ));

	for ( guint h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( CH_IS_EXPANDED && Ch->da_page ) {
			if ( PS_IS_RIGHT && Ch->da_power
			     && (__select_state == 0 || __clicked_channel != Ch) ) {
				g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
				gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg1__[__cur_stage]);
				gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg1__[__cur_stage]);
				g_signal_handler_unblock( Ch->da_page, Ch->expose_handler_id);
			}
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
__do_score_forward( gchar score_ch)
{
	__have_unsaved_scores = TRUE;
	if ( __cur_page < __total_pages ) {
		Ai (__hypnogram, gchar, __cur_page) = score_ch;
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
	if ( __cur_page_app < __total_pages ) {
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
	guint p = __cur_page - 1;
	while ( Ai (__hypnogram, gchar, p) != AghScoreCodes[AGH_SCORE_NONE] )
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
	while ( Ai (__hypnogram, gchar, p) != AghScoreCodes[AGH_SCORE_NONE] )
		if ( p < __total_pages )
			++p;
		else
			break;
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   (__cur_page_app = __cur_page = p)+1);
}





inline static gboolean
__page_has_artifacts( guint p)
{
	for ( guint h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai( HH, SChannelPresentation, h);
		for ( size_t a = 0; a < Ch->n_artifacts; ++a )
			if ( (p * APSZ * Ch->samplerate < Ch->artifacts[a*2  ] && Ch->artifacts[a*2  ] < (p+1) * APSZ * Ch->samplerate) ||
			     (p * APSZ * Ch->samplerate < Ch->artifacts[a*2+1] && Ch->artifacts[a*2+1] < (p+1) * APSZ * Ch->samplerate)
			   ||
			     (Ch->artifacts[a*2  ] < p * APSZ * Ch->samplerate && (p+1) * APSZ * Ch->samplerate < Ch->artifacts[a*2+1]) ) {
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
	for ( guint h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->power ) {
			g_object_set( G_OBJECT (Ch->da_power),
				      "visible", __draw_power ? TRUE : FALSE,
				      NULL);
			g_object_set( G_OBJECT (Ch->da_spectrum),
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
				0, __signal_length * __clicked_channel->samplerate);
	__clicked_channel->n_artifacts = agh_edf_get_artifacts( __source_ref, __clicked_channel->name,
								&__clicked_channel->artifacts);
	agh_msmt_get_signal_filtered_as_float( __clicked_channel->rec_ref,
					       &__clicked_channel->signal_filtered, NULL, NULL);
	if ( __clicked_channel->power ) {
		agh_msmt_get_power_course_in_range_as_float_direct( __clicked_channel->rec_ref,
								    __clicked_channel->from, __clicked_channel->upto,
								    (float*)__clicked_channel->power->data);
		for ( gushort b = 0; b <= __clicked_channel->uppermost_band; ++b ) {
			GArray *this_band = Ai (__clicked_channel->power_in_bands, GArray*, b);
			agh_msmt_get_power_course_in_range_as_float_direct( __clicked_channel->rec_ref,
									    AghFreqBands[b][0], AghFreqBands[b][1],
									    (float*)this_band->data);
		}

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
	for ( guint h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		Ch->signal_display_scale = __sane_signal_display_scale;
	}
	REDRAW_ALL;
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
	free( fname_base);
	agh_msmt_export_power_in_range( __clicked_channel->rec_ref, __clicked_channel->from, __clicked_channel->upto,
					__buf__);
	gtk_statusbar_pop( GTK_STATUSBAR (sbSF), agh_sb_context_id_General);
	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      fname_base, __clicked_channel->from, __clicked_channel->upto);
	gtk_statusbar_push( GTK_STATUSBAR (sbSF), agh_sb_context_id_General, __buf__);
}

void
iSFPowerExportAll_activate_cb()
{
	char *fname_base = agh_msmt_fname_base( __clicked_channel->rec_ref);
	snprintf_buf( "%s.tsv", fname_base);
	free( fname_base);
	agh_msmt_export_power( __clicked_channel->rec_ref,
			       __buf__);
	gtk_statusbar_pop( GTK_STATUSBAR (sbSF), agh_sb_context_id_General);
	snprintf_buf( "Wrote %s.tsv", fname_base);
	gtk_statusbar_push( GTK_STATUSBAR (sbSF), agh_sb_context_id_General, __buf__);
}






// -- Score

void
iSFScoreAssist_activate_cb()
{
	if ( agh_episode_assisted_score_by_jde( __our_j->name, __our_d, __our_e) == 0 ) {
		__have_unsaved_scores = TRUE;
		agh_edf_get_scores_as_garray( __source_ref, __hypnogram, NULL);

		__repaint_score_stats();
		REDRAW_ALL;
	}
}



void
bSFScore_clicked_cb()
{
	agh_edf_put_scores_as_garray( __source_ref,
				      __hypnogram);

      // update power profile in measurements view
	gtk_widget_queue_draw( cMeasurements);

	gtk_widget_hide( wScoringFacility);
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
		agh_edf_import_scores( __source_ref,
				       fname);
	}
	gtk_widget_destroy( f_chooser);
	agh_edf_get_scores_as_garray( __source_ref,
				      __hypnogram, NULL);
	REDRAW_ALL;
	__repaint_score_stats();
}

void
iSFScoreExport_activate_cb()
{
	agh_edf_put_scores_as_garray( __source_ref,
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
	agh_edf_get_scores_as_garray( __source_ref, __hypnogram, NULL);

	REDRAW_ALL;
}

void
iSFScoreClear_activate_cb()
{
	g_array_set_size( __hypnogram, 0);
	g_array_set_size( __hypnogram, __signal_length / PSZ);

	REDRAW_ALL;

	snprintf_buf( "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);
}







// ------- cleanup

void
wScoringFacility_delete_event_cb()
{
	if ( __have_unsaved_scores &&
	     pop_question( GTK_WINDOW (wScoringFacility), "Save your scorings?") == GTK_RESPONSE_YES )
		agh_edf_put_scores_as_garray( __source_ref, __hypnogram);

	free( __pattern_snippet);
	__pattern_snippet = NULL;
	gtk_widget_queue_draw( cMeasurements);
	gtk_widget_hide( wPattern);
	gtk_widget_hide( wScoringFacility);
}






// -------- colours

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
