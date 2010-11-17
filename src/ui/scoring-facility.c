// ;-*-C-*- *  Time-stamp: "2010-11-17 00:27:26 hmmr"
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



#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include "../core/iface.h"
#include "../core/iface-glib.h"
#include "misc.h"
#include "ui.h"



void __collect_and_paint_one_subject_episodes();  // in measurements view, that is


GtkListStore
	*agh_mScoringPageSize;

GtkWidget
	*wScoringFacility,
	*eScoringFacPageSize;

static GtkWidget
//	*daScoringFacProfileView,
	*cScoringFacPageViews,
	*daScoringFacHypnogram,
	*bScoringFacBack,
	*bScoringFacForward,
	*lScoringFacTotalPages,
	*lScoringFacPercentScored,
	*eScoringFacCurrentPage,
	*lScoringFacCurrentPos,
	*lScoreStatsNREMPercent,
	*lScoreStatsREMPercent,
	*lScoreStatsWakePercent,
	*lScoringFacCurrentStage,
	*bScoreClear, *bScoreNREM1, *bScoreNREM2, *bScoreNREM3, *bScoreNREM4,
	*bScoreREM,   *bScoreWake,  *bScoreMVT,
	*bScoreGotoPrevUnscored, *bScoreGotoNextUnscored;

static GtkWidget
	*mSFArtifacts, *mSFPower, *mSFScore, *mSFSpectrum,
	*iSFArtifactsShowOriginal, *iSFArtifactsShowProcessed;


#define AGH_DA_PAGE_HEIGHT 130
#define AGH_DA_SPECTRUM_WIDTH 100
#define AGH_DA_PSD_PROFILE_HEIGHT 65
#define AGH_DA_EMG_PROFILE_HEIGHT 26

enum {
	cSIGNAL_SCORE_NONE,
	cSIGNAL_SCORE_NREM1,
	cSIGNAL_SCORE_NREM2,
	cSIGNAL_SCORE_NREM3,
	cSIGNAL_SCORE_NREM4,
	cSIGNAL_SCORE_WAKE,
	cSIGNAL_SCORE_REM,
	cSIGNAL_SCORE_MVT,

	cSIGNAL_UNFAZER,

	cARTIFACT,
	cARTIFACT_VOLATILE,

	cLABELS,
	cTICKS,

	cPOWER,
	cHYPNOGRAM,
	cHYPNOGRAM_SCORELINE,
	cCURSOR,

	cSPECTRUM,
	cSPECTRUM_AXES,
	cSPECTRUM_GRID,

	cEMG,

	cTOTAL
};  // colours

static const gchar* const __bg_rgb[] = {
	"white",
	"#2D2D8B", "#000064", "#000046", "#00002a",
	"#610863", "#8D987F", "#4F3D02",

	"#110000",  // unfazer

	"#550001",  // artifact
	"#BB1116",

	"#000000", "#000000", // labels, ticks

	"#FFFFFF",
	"#FAFAD2", "#FFFFFF",
	"#FFFFFF",

	"white",
	"#440000",
	"#440000",

	"#EEEEFF",
};

static const gchar* const __fg_rgb[] = {
	"#000000",
	"#FFEDB5", "#FFEDB5", "#FFEDB5", "#FFF7DF",
	"#E2E2E2", "#BDBDBD", "#BDBDBD",

	"#EEEEFF",

	"#890001",
	"#661116",

	"#12FFFF", "#00FE1E",

	"#4682B4",
	"#FFD3FF", "BBFFBB",
	"#FF1121",

	"#FF1123",
	"#DDDDFF",
	"#DEDEFF",

	"#119901",
};

static gshort __line_widths[] = {
	1,  1, 1, 1, 1,  1, 1, 1,
	1,
	1, 1,
	1, 1,
	2, 1, 2, 1,
	1, 1, 1,
	1
};
//static gshort __cap_styles[] = {
//};

static	GdkColor
	__fg__[cTOTAL],
	__bg__[cTOTAL];

static GdkGC
	*__gc__[cTOTAL];






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
	     !(lScoringFacCurrentStage  = glade_xml_get_widget( xml, "lScoringFacCurrentStage")) ||
	     !(lScoringFacCurrentPos    = glade_xml_get_widget( xml, "lScoringFacCurrentPos")) ||
	     !(lScoringFacPercentScored = glade_xml_get_widget( xml, "lScoringFacPercentScored")) ||
	     !(lScoreStatsNREMPercent   = glade_xml_get_widget( xml, "lScoreStatsNREMPercent")) ||
	     !(lScoreStatsREMPercent    = glade_xml_get_widget( xml, "lScoreStatsREMPercent")) ||
	     !(lScoreStatsWakePercent   = glade_xml_get_widget( xml, "lScoreStatsWakePercent")) ||
	     !(bScoreClear  = glade_xml_get_widget( xml, "bScoreClear")) ||
	     !(bScoreNREM1  = glade_xml_get_widget( xml, "bScoreNREM1")) ||
	     !(bScoreNREM2  = glade_xml_get_widget( xml, "bScoreNREM2")) ||
	     !(bScoreNREM3  = glade_xml_get_widget( xml, "bScoreNREM3")) ||
	     !(bScoreNREM4  = glade_xml_get_widget( xml, "bScoreNREM4")) ||
	     !(bScoreREM    = glade_xml_get_widget( xml, "bScoreREM"))   ||
	     !(bScoreWake   = glade_xml_get_widget( xml, "bScoreWake"))  ||
	     !(bScoreMVT    = glade_xml_get_widget( xml, "bScoreMVT"))   ||
	     !(bScoreGotoPrevUnscored    = glade_xml_get_widget( xml, "bScoreGotoPrevUnscored"))   ||
	     !(bScoreGotoNextUnscored    = glade_xml_get_widget( xml, "bScoreGotoNextUnscored")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eScoringFacPageSize),
				 GTK_TREE_MODEL (agh_mScoringPageSize));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer,
					"text", 0,
					NULL);

	GdkGCValues xx;
	GdkColormap *cmap = gtk_widget_get_colormap( daScoringFacHypnogram);
	for ( gushort i = 0; i < cTOTAL; ++i ) {
		gdk_color_parse( __fg_rgb[i], &__fg__[i]),
			gdk_colormap_alloc_color( cmap, &__fg__[i], FALSE, TRUE);
		gdk_color_parse( __bg_rgb[i], &__bg__[i]),
			gdk_colormap_alloc_color( cmap, &__bg__[i], FALSE, TRUE);

		xx.foreground = __fg__[i], xx.background = __bg__[i];  // bg <-> fg // why?
		xx.line_width = __line_widths[i],
			xx.line_style = GDK_LINE_SOLID, xx.cap_style = GDK_CAP_ROUND;

		__gc__[i] = gtk_gc_get( agh_visual->depth, cmap,
					&xx, GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);
	}

	gtk_widget_modify_bg( daScoringFacHypnogram, GTK_STATE_NORMAL, &__bg__[cHYPNOGRAM]);

      // ------- menus
	if ( !(mSFArtifacts       = glade_xml_get_widget( xml, "mSFArtifacts")) ||
	     !(mSFPower    	  = glade_xml_get_widget( xml, "mSFPower")) ||
	     !(mSFScore    	  = glade_xml_get_widget( xml, "mSFScore")) ||
	     !(mSFSpectrum    	  = glade_xml_get_widget( xml, "mSFSpectrum")) ||
	     !(iSFArtifactsShowOriginal  = glade_xml_get_widget( xml, "iSFArtifactsShowOriginal")) ||
	     !(iSFArtifactsShowProcessed = glade_xml_get_widget( xml, "iSFArtifactsShowProcessed")) )
		return -1;


	return 0;
}

void
agh_ui_destruct_ScoringFacility()
{
	// __gc_...
}











// ---------- data structures

static gsize	__signal_length; // in sec
static gsize	__total_pages, __fft_pagesize;
static guint	__cur_page,
		__cur_page_app,
		__cur_pos_hr, __cur_pos_min, __cur_pos_sec;

typedef struct {
	const gchar
	          *name,
		  *type;
	TRecRef    rec_ref;

	double	  *signal_filtered,
		  *signal_original;
	gsize	   n_samples;
	gsize	   samplerate;
	gfloat	   signal_display_scale;
	GtkWidget *da_page;

	struct SUnfazer
		  *unfazers;
	guint	   n_unfazers;

	GArray	  *power;
	gfloat	   from, upto;
	GtkWidget *da_powercourse;

	double	  *spectrum;  // per page, is volatile
	double	   spectrum_max;
	guint	   spectrum_upper_freq;
	guint	   n_bins;
	GtkWidget *da_spectrum;
	gfloat	   spectrum_display_scale;

	GArray	  *af_track;
	gboolean   af_marks_updated;
	gfloat	   dirty_percent;

	GArray    *emg_precomputed;
	GtkWidget *da_emg_profile;
	gfloat     emg_scale;

	gboolean   visible,
		show_original_signal,
		show_processed_signal,
		show_spectrum_absolute;

	GtkWidget *expander,
		  *vbox;
	gulong     expose_handler_id;

} SChannelPresentation;

static void
__destroy_ch( SChannelPresentation *Ch)
{
	if ( Ch->signal_filtered )  { free( Ch->signal_filtered);  Ch->signal_filtered = NULL; }
	if ( Ch->signal_original )  { free( Ch->signal_original);  Ch->signal_original = NULL; }
	if ( Ch->spectrum )         { free( Ch->spectrum);         Ch->spectrum        = NULL; }
	if ( Ch->unfazers )  	    { free( Ch->unfazers);         Ch->unfazers        = NULL; }
	if ( Ch->power )           { g_array_free( Ch->power,           TRUE);   Ch->power           = NULL; }
	if ( Ch->af_track )        { g_array_free( Ch->af_track,        TRUE);   Ch->af_track        = NULL; }
	if ( Ch->emg_precomputed ) { g_array_free( Ch->emg_precomputed, TRUE);   Ch->emg_precomputed = NULL; }
}


static GArray	*HH;
static TEDFRef	__source_ref;  // the core structures allow for multiple edf
                               // sources providing signals for a single episode;
                               // keeping only one __source_ref here will, then,
                               // read/write scores in this source's histogram;
// -- but it's essentially not a problem since all edf sources will still converge
//    to the same .histogram file
static GArray	*__hypnogram;







guint	AghDisplayPageSizeValues[] = { 5, 10, 15, 20, 30, 60, 120, 300, -1 };
guint	AghDisplayPageSizeItem = 4;  // the one used to obtain FFTs

static guint	__pagesize_item = 4;  // pagesize as currently displayed



static guint __pagesize_ticks[] = {
	5, 5, 3, 4, 6, 12, 24, 30
};


#define PSZ  AghDisplayPageSizeValues[AghDisplayPageSizeItem]
#define APSZ AghDisplayPageSizeValues[__pagesize_item]

#define P2AP(p)  (guint)((p) * (float)PSZ / (float)APSZ)
#define AP2P(p)  (guint)((p) * (float)APSZ / (float)PSZ)





static void
__calculate_dirty_percent( SChannelPresentation *Ch)
{
	guint	dirty_secs = 0,
		p;
	for ( p = 0; p < Ch->af_track->len; ++p )
		dirty_secs += (Ai (Ch->af_track, gchar, p) == 'x');
	Ch->dirty_percent = (gfloat) dirty_secs / p * 100;
}




static void __repaint_score_stats();

gboolean cScoringFacPageViewExpander_activate_cb( GtkExpander*, gpointer);
gboolean daScoringFacPageView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
gboolean daScoringFacPageView_key_press_event_cb( GtkWidget*, GdkEventKey*, gpointer);
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

gboolean
agh_prepare_scoring_facility()
{
      // prepare structures for the first viewing
	if ( !HH ) {
		HH  = g_array_new( FALSE, FALSE, sizeof(SChannelPresentation));
		__hypnogram = g_array_new( FALSE,  TRUE, sizeof(gchar));
	}

      // clean up after previous viewing
	gtk_container_foreach( GTK_CONTAINER (cScoringFacPageViews),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	guint h;
	for ( h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		__destroy_ch(Ch);
	}
	__cur_page = 0;
	__signal_length = 0; // is set to the longest signal, below


      // set up channel representations
	g_array_set_size( HH, AghHs);

	set_cursor_busy( TRUE, wMainWindow);

	guint n_visible = 0;
	for ( h = 0; h < AghHs; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		Ch->name = AghHH[h];

		Ch->rec_ref = agh_msmt_find_by_jdeh( AghJ->name, AghD, AghE, Ch->name);
		if ( Ch->rec_ref == NULL ) {
			fprintf( stderr, "agh_prepare_scoring_facility(): no measurement matching (%s, %s, %s, %s)\n",
				 AghJ->name, AghD, AghE, Ch->name);
			Ch->signal_filtered = Ch->signal_original = NULL, Ch->power = Ch->af_track = NULL;
			continue;
		}
		Ch->type = agh_msmt_get_signal_type( Ch->rec_ref);

		if ( h == 0 )
			__source_ref = agh_msmt_get_source( Ch->rec_ref);

	      // get signal data
		Ch->n_samples = agh_msmt_get_signal_original_as_double( Ch->rec_ref,
									&Ch->signal_original, &Ch->samplerate, NULL);
		agh_msmt_get_signal_filtered_as_double( Ch->rec_ref,
							&Ch->signal_filtered, NULL, NULL);

		Ch->from = AghQuickViewFreqFrom, Ch->upto = AghQuickViewFreqUpto;

		if ( agh_signal_type_is_fftable( agh_msmt_get_signal_type( Ch->rec_ref)) ) {
			Ch->power = g_array_new( FALSE, FALSE, sizeof(double));
			Ch->af_track = g_array_new( TRUE, FALSE, sizeof(char));  // zero-terminate for strxxx() to work
			// the first call to get power course is *_as_garray; others will use *_direct
			agh_msmt_get_power_course_in_range_as_double_garray( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     Ch->power);
			Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
							       Ch->name,
							       &Ch->unfazers);
			Ch->show_processed_signal = TRUE;
			Ch->show_original_signal = FALSE;
			Ch->show_spectrum_absolute = TRUE;
			__calculate_dirty_percent( Ch);

			Ch->n_bins = agh_msmt_get_power_spectrum_as_double( Ch->rec_ref, 0,
									    &Ch->spectrum, &Ch->spectrum_max);
			Ch->spectrum_display_scale = Ch->spectrum_max;
			Ch->spectrum_upper_freq = Ch->n_bins *.66;

			agh_edf_get_artifacts_as_garray( __source_ref, Ch->name,
							 Ch->af_track);
		} else {
			Ch->show_original_signal = TRUE;
			Ch->show_processed_signal = FALSE;
			Ch->power = Ch->af_track = NULL;
			Ch->spectrum = NULL;
			Ch->unfazers = NULL, Ch->n_unfazers = 0;
		}

		Ch->af_marks_updated = FALSE;

		if ( Ch->n_samples == 0 )
			Ch->visible = FALSE;
		else {
			Ch->visible = TRUE;
			n_visible++;

			snprintf_buf( "%s <b>%s</b>", Ch->type, Ch->name);
			Ch->expander = gtk_expander_new( __buf__);
			gtk_expander_set_use_markup( GTK_EXPANDER (Ch->expander), TRUE);

			gtk_box_pack_start( GTK_BOX (cScoringFacPageViews),
					    Ch->expander, TRUE, TRUE, 0);
			gtk_expander_set_expanded( GTK_EXPANDER (Ch->expander),
						   TRUE /*h == AghHi*/);
			g_signal_connect_after( Ch->expander, "activate",
					  G_CALLBACK (cScoringFacPageViewExpander_activate_cb),
					  (gpointer)Ch);

			gtk_container_add( GTK_CONTAINER (Ch->expander),
					   Ch->vbox = gtk_vbox_new( FALSE, 0));

		      // set up page view
			if ( Ch->n_samples ) {
				__signal_length = MAX( __signal_length, Ch->n_samples / Ch->samplerate);
				Ch->signal_display_scale = 20;

				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   Ch->da_page = gtk_drawing_area_new());
				g_object_set( G_OBJECT (Ch->da_page),
					      "app-paintable", TRUE,
					      "height-request", AGH_DA_PAGE_HEIGHT,
					      NULL);
				Ch->expose_handler_id = g_signal_connect_after( Ch->da_page, "expose-event",
										G_CALLBACK (daScoringFacPageView_expose_event_cb),
										(gpointer)Ch);
				g_signal_connect_after( Ch->da_page, "key-press-event",
							G_CALLBACK (daScoringFacPageView_key_press_event_cb),
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
			} else
				Ch->da_page = NULL;

		      // set up PSD profile view
			if ( Ch->power ) {
				__signal_length = MAX( __signal_length,
						       Ch->power->len * PSZ);

				GtkWidget *hbox;
				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   hbox = gtk_hbox_new( FALSE, 0));
				gtk_container_add( GTK_CONTAINER (hbox),
						   Ch->da_powercourse = gtk_drawing_area_new());
				gtk_container_add_with_properties( GTK_CONTAINER (hbox),
								   Ch->da_spectrum = gtk_drawing_area_new(),
								   "expand", FALSE,
								   NULL);
			      // profile pane
				g_object_set( G_OBJECT (Ch->da_powercourse),
					      "app-paintable", TRUE,
					      "height-request", AGH_DA_PSD_PROFILE_HEIGHT,
					      NULL);
				g_signal_connect_after( Ch->da_powercourse, "expose-event",
							G_CALLBACK (daScoringFacPSDProfileView_expose_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_powercourse, "button-press-event",
							G_CALLBACK (daScoringFacPSDProfileView_button_press_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_powercourse, "scroll-event",
							G_CALLBACK (daScoringFacPSDProfileView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_powercourse,
						       (GdkEventMask) GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

				gtk_widget_modify_fg( Ch->da_powercourse, GTK_STATE_NORMAL, &__fg__[cPOWER]);
				gtk_widget_modify_bg( Ch->da_powercourse, GTK_STATE_NORMAL, &__bg__[cPOWER]);

			      // spectrum pane
				g_object_set( G_OBJECT (Ch->da_spectrum),
					      "app-paintable", TRUE,
					      "width-request", AGH_DA_SPECTRUM_WIDTH,
					      NULL);
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

				gtk_widget_modify_fg( Ch->da_spectrum, GTK_STATE_NORMAL, &__fg__[cSPECTRUM]);
				gtk_widget_modify_bg( Ch->da_spectrum, GTK_STATE_NORMAL, &__bg__[cSPECTRUM]);
			} else
				Ch->da_powercourse = Ch->da_spectrum = NULL;

		      // set up EMG profile
			if ( strcmp( Ch->type, "EMG") == 0 ) {
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
					      "height-request", AGH_DA_EMG_PROFILE_HEIGHT,
					      NULL);
				g_object_set( G_OBJECT (da_void),
					      "width-request", AGH_DA_SPECTRUM_WIDTH,
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

				gtk_widget_modify_fg( Ch->da_emg_profile, GTK_STATE_NORMAL, &__fg__[cEMG]);
				gtk_widget_modify_bg( Ch->da_emg_profile, GTK_STATE_NORMAL, &__bg__[cEMG]);

				Ch->emg_precomputed = g_array_sized_new( FALSE, FALSE, sizeof(float), 0);  // to be filled on first expose
				Ch->emg_scale = 1.;
			} else {
				Ch->emg_precomputed = NULL;
				Ch->da_emg_profile = NULL;
			}
		}

	}

	set_cursor_busy( FALSE, wMainWindow);

	if ( !n_visible || !Ai (HH, SChannelPresentation, AghHi).expander )
		return FALSE;

      // get scores
	__total_pages = agh_edf_get_scores_as_garray( __source_ref,
						      __hypnogram, &__fft_pagesize);

      // set up other controls
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   __cur_page = __cur_page_app = 1);
	gtk_combo_box_set_active( GTK_COMBO_BOX (eScoringFacPageSize),
				  __pagesize_item = AghDisplayPageSizeItem);

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	g_signal_emit_by_name( eScoringFacPageSize, "changed");

	__repaint_score_stats();

	gtk_widget_queue_draw( cMeasurements);

	return TRUE;
}












// -------------------- Page


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




static GtkWidget *__af_marking_in_widget;
static guint __af_mark_start, __af_mark_virtual_end;
static gboolean __af_mark_on;
static void __af_mark_region( guint, guint, SChannelPresentation*, gchar);


static SChannelPresentation  // for menus
	*__clicked_channel;


enum {
	UNF_SEL_CHANNEL = 1,
	UNF_SEL_CALIBRATE = 2
};
static gint __unfazer_sel_state = 0;
static SChannelPresentation
	*__unfazer_offending_channel;
static float
	__unfazer_factor = 0.1;




gboolean
cScoringFacPageViewExpander_activate_cb( GtkExpander *expander, gpointer userdata)
{
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	return TRUE;
}





static guint
__draw_signal( double *signal, guint width, guint height, SChannelPresentation *Ch, GdkGC *gc)
{
	guint i;
	for ( i = 0; i < width; ++i ) {
		LL(i).x = i;
		LL(i).y =
			- signal[ lroundf(((gfloat)i / width + __cur_page_app) * Ch->samplerate * APSZ) ]
			* (height / Ch->signal_display_scale)
			+ height/2;
	}
	gdk_draw_lines( Ch->da_page->window, gc,
			(GdkPoint*)__ll__->data, i);
	return i;
}

gboolean
daScoringFacPageView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( !Ch->visible || !gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) || !Ch->n_samples )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);
	__ensure_enough_lines( wd);

	guint i;

      // artifacts (changed bg)
	if ( Ch->af_track ) {
		guint	cur_page_start_s =  __cur_page_app      * APSZ,
			cur_page_end_s   = (__cur_page_app + 1) * APSZ;
		for ( i = cur_page_start_s; i < cur_page_end_s; ++i ) {
			if ( Ai (Ch->af_track, gchar, i) == 'x' ) {
				gdk_draw_rectangle( wid->window, __gc__[cARTIFACT],
						    TRUE,
						    (gfloat)(i % APSZ) / APSZ * wd, 20,
						    (gfloat)(1)        / APSZ * wd, ht-40);
/*
				snprintf( __buf__, 20, "<b>\342\234\230</b>");
				pango_layout_set_markup( __pp__, __buf__, -1);
				gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
						 (i % APSZ + .5) / APSZ * wd,
						 15,
						 __pp__);
*/
			}
		}
		snprintf_buf( "<small><i>%4.2f %% dirty</i></small>", Ch->dirty_percent);
		pango_layout_set_markup( __pp__, __buf__, -1);
		gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
				 wd - 70,
				 ht - 15,
				 __pp__);
	}

      // volatile artifacts (while marking)
	if ( __af_marking_in_widget == wid ) {
		guint vstart = (__af_mark_start < __af_mark_virtual_end) ? __af_mark_start : __af_mark_virtual_end,
			vend = (__af_mark_start < __af_mark_virtual_end) ? __af_mark_virtual_end : __af_mark_start;
		gdk_draw_rectangle( wid->window, __gc__[cARTIFACT_VOLATILE],
				    TRUE,
				    (gfloat)(vstart % APSZ) / APSZ * wd, 20,
				    (gfloat)(vend-vstart)   / APSZ * wd, ht-40);
	}


      // uV scale
	guint dpuV = 1 * (ht / Ch->signal_display_scale);
	gdk_draw_line( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
		       10, 10,
		       10, 10 + dpuV);
	snprintf_buf( "<b><small>1 \302\265V</small></b>");
	pango_layout_set_markup( __pp__, __buf__, -1);
	gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			 15, 10,
			 __pp__);

      // ticks
	for ( i = 0; i < __pagesize_ticks[__pagesize_item]; ++i ) {
		guint tick_pos = i * APSZ / __pagesize_ticks[__pagesize_item];
		snprintf_buf( "<small>%2d</small>", tick_pos);
		pango_layout_set_markup( __pp__, __buf__, -1);
		gdk_draw_layout( wid->window, __gc__[cTICKS],
				 i * wd / __pagesize_ticks[__pagesize_item] + 5,
				 120 - 15,
				 __pp__);
		gdk_draw_line( wid->window, __gc__[cTICKS],
			       i * wd / __pagesize_ticks[__pagesize_item], 0,
			       i * wd / __pagesize_ticks[__pagesize_item], ht);
	}

      // waveform: signal_filtered
	if ( Ch->show_processed_signal && Ch->af_track
	     && __unfazer_sel_state == 0 ) {  // only show processed signal when done with unfazing
		__draw_signal( Ch->signal_filtered, wd, ht, Ch,
			       wid->style->fg_gc[GTK_STATE_NORMAL]);
		if ( Ch->power ) {
			snprintf_buf( "<i>filt</i>");
			pango_layout_set_markup( __pp__, __buf__, -1);
			gdk_draw_layout( wid->window, __gc__[cLABELS],
					 wd - 80,
					 5,
					 __pp__);
		}
	}

      // waveform: signal_original
	if ( Ch->show_original_signal || !Ch->af_track ) {
		__draw_signal( Ch->signal_original, wd, ht, Ch,
			       wid->style->fg_gc[GTK_STATE_INSENSITIVE]);
		if ( Ch->power ) {
			snprintf_buf( "<i>orig</i>");
			pango_layout_set_markup( __pp__, __buf__, -1);
			gdk_draw_layout( wid->window, __gc__[cLABELS],
					 wd - 80,
					 22,
					 __pp__);
		}
	}

      // unfazer
	if ( __unfazer_sel_state ) {
		PangoRectangle extents;
		if ( Ch == __clicked_channel ) {
			switch ( __unfazer_sel_state ) {
			case UNF_SEL_CHANNEL:
				snprintf_buf( "<big><b>Unfaze this channel from...</b></big>");
			    break;
			case UNF_SEL_CALIBRATE:
				snprintf_buf( "<big><b>Unfaze this channel from %s</b></big>",
					  __unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				guint subscript;
				for ( i = 0; i < wd; ++i ) {
					Ai (__ll__, GdkPoint, i).x = i;
					Ai (__ll__, GdkPoint, i).y =
						(subscript = lroundf(((gfloat)i / wd + __cur_page_app) * Ch->samplerate * APSZ),
						 - (Ch->signal_original[ subscript ]
						    - __unfazer_offending_channel->signal_original[ subscript ] * __unfazer_factor)
						 * (ht / Ch->signal_display_scale)
						 + ht/2);
				}
				gdk_draw_lines( wid->window, __gc__[cSIGNAL_UNFAZER],
						(GdkPoint*)__ll__->data, i);

			    break;
			}
			pango_layout_set_markup( __pp__, __buf__, -1);
			pango_layout_get_pixel_extents( __pp__, &extents, NULL);
			gdk_draw_layout( wid->window, __gc__[cLABELS],
					 wd/2 - extents.width/2,
					 ht - 35,
					 __pp__);

		} else if ( Ch == __unfazer_offending_channel ) {
			switch ( __unfazer_sel_state ) {
			case UNF_SEL_CHANNEL:
			    break;
			case UNF_SEL_CALIBRATE:
				snprintf_buf( "<big><b>Calibrating unfaze factor:</b> %4.2f</big>",
					  __unfazer_factor);
				break;
			}
			pango_layout_set_markup( __pp__, __buf__, -1);
			pango_layout_get_pixel_extents( __pp__, &extents, NULL);
			gdk_draw_layout( wid->window, __gc__[cLABELS],
					 wd/2 - extents.width/2,
					 ht - 35,
					 __pp__);
		}
	}

      // unfazer info
	if ( Ch->n_unfazers ) {
		GString *unf_buf = g_string_sized_new( 128);
		g_string_assign( unf_buf, "Unf: ");
		for ( i = 0; i < Ch->n_unfazers; ++i ) {
			g_string_append_printf( unf_buf, "<small><b>%s: %5.3f%c</b></small>",
						Ch->unfazers[i].channel, Ch->unfazers[i].factor,
						(i+1 == Ch->n_unfazers) ? ' ' : ';');
		}
		pango_layout_set_markup( __pp__, unf_buf->str, -1);
		gdk_draw_layout( wid->window, __gc__[cLABELS],
				 10,
				 ht - 35,
				 __pp__);
		g_string_free( unf_buf, TRUE);
	}

	return TRUE;
}


static int __chaining_next_key = -1;

gboolean
daScoringFacPageView_key_press_event_cb( GtkWidget *wid, GdkEventKey *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( event->type == GDK_KEY_PRESS ) {
		switch ( event->keyval ) {
		case GDK_F1:
			Ch->show_original_signal = !Ch->show_original_signal;
		      break;
		case GDK_F2:
			Ch->show_processed_signal = !Ch->show_processed_signal;
		      break;
		case GDK_J:
			if ( event->state & GDK_SHIFT_MASK ) {
				;
			}
		    break;
		case GDK_F:
			if ( event->state & GDK_SHIFT_MASK ) {
				;
			}
		    break;
		case GDK_1:
		case GDK_2:
		case GDK_3:
		case GDK_4:
		case GDK_5:
		case GDK_6:
		case GDK_7:
		case GDK_8:
		case GDK_9:
		case GDK_0:
			if ( __chaining_next_key != -1 ) {
				guint h = event->keyval - GDK_0;
				Ai ( HH, SChannelPresentation, h).da_page = Ch->da_page;
				__chaining_next_key = -1;
			}
			break;
		}

		gtk_widget_queue_draw( wid);
	}
	return TRUE;
}



gboolean
daScoringFacPageView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	gint wd, ht;
	guint h;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	if ( __unfazer_sel_state ) {
		switch ( __unfazer_sel_state ) {
		case UNF_SEL_CHANNEL:
			if ( event->button == 1 )
				if ( strcmp( Ch->name, __clicked_channel->name) != 0 ) {
					__unfazer_offending_channel = Ch;
					double f = agh_edf_get_unfazer_factor( __source_ref,
									       __clicked_channel->name,
									       __unfazer_offending_channel->name);
					__unfazer_factor = ( isnan(f) ) ? 0. : f;
					__unfazer_sel_state = UNF_SEL_CALIBRATE;
				} else {
					__unfazer_sel_state = 0;
				}
			else
				__unfazer_sel_state = 0;
		    break;
		case UNF_SEL_CALIBRATE:
			if ( event->button == 1 && __clicked_channel == Ch ) {
				agh_edf_add_or_mod_unfazer( __source_ref,  // apply
							    __clicked_channel->name,
							    __unfazer_offending_channel->name,
							    __unfazer_factor);
				Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
								       Ch->name,
								       &Ch->unfazers);
				agh_msmt_get_signal_filtered_as_double( Ch->rec_ref,
									&Ch->signal_filtered, NULL, NULL);
				agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
										     Ch->from, Ch->upto,
										     (double*)Ch->power->data);
				__unfazer_sel_state = 0;

			} else if ( event->button == 2 )
				if ( event->state & GDK_CONTROL_MASK ) { // remove unfazer
					agh_edf_remove_unfazer( __source_ref,
								__clicked_channel->name,
								__unfazer_offending_channel->name);
					Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
									       Ch->name,
									       &Ch->unfazers);
					agh_msmt_get_signal_filtered_as_double( Ch->rec_ref,
										&Ch->signal_filtered, NULL, NULL);
					agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
											     Ch->from, Ch->upto,
											     (double*)Ch->power->data);
					__unfazer_sel_state = 0;
				} else
					__unfazer_factor = 0.;
			else
				__unfazer_sel_state = 0;
		    break;
		}
		g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
		return TRUE;
	}

	switch ( event->button ) {
	case 2:
		if ( event->state & GDK_CONTROL_MASK )
			for ( h = 0; h < HH->len; ++h )
				Ai (HH, SChannelPresentation, h).signal_display_scale = 20;
		else
			Ch->signal_display_scale = 20;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
		if ( Ch->af_track && event->y > ht/2 ) {
			__clicked_channel = Ch;  // no other way to mark this channel even though user may not select Unfaze
			gtk_menu_popup( GTK_MENU (mSFArtifacts),
					NULL, NULL, NULL, NULL, 3, event->time);
			break;
		}
		if ( Ch->af_track ) {
			__af_marking_in_widget = wid;
			__af_mark_start = (__cur_page_app + event->x / wd) * APSZ;
			__af_mark_on = FALSE;
		}
	case 1:
		if ( Ch->af_track ) {
			__af_marking_in_widget = wid;
			__af_mark_start = (__cur_page_app + event->x / wd) * APSZ;
			__af_mark_on = TRUE;
		}
	    break;
	}

	return TRUE;
}



gboolean
daScoringFacPageView_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	gint wd, ht;
	gdk_drawable_get_size( wid->window, &wd, &ht);

	if ( wid != __af_marking_in_widget || !Ch->af_track )
		return TRUE;
	switch ( event->button ) {
	case 1:
		__af_mark_region( __af_mark_start, (__cur_page_app + event->x / wd) * APSZ, Ch, 'x');
	    break;
	case 3:
		__af_mark_region( __af_mark_start, (__cur_page_app + event->x / wd) * APSZ, Ch, ' ');
	    break;
	}

	__af_marking_in_widget = NULL;

	// if ( event->state & GDK_MOD1_MASK ) {
	// 	agh_msmt_get_af_track_as_garray( AghJ->name, AghD->str, AghE->str, Ch->name, Ch->af_track);
	// 	gtk_widget_queue_draw( Ch->da_powercourse);
	// 	Ch->af_marks_updated = FALSE;
	// }

	return TRUE;
}




static void
__af_mark_region( guint x1, guint x2, SChannelPresentation* Ch, gchar value)
{
	if ( x1 > x2 ) {
		guint _ = x1;
		x1 = x2, x2 = _;
	}
	if ( x1 < 0 )
		x1 = 0;
	if ( x2 >= __signal_length )
		x2 = __signal_length-1;

	guint s;
	for ( s = x1; s <= x2; s++ )
		Ai (Ch->af_track, gchar, s) = value;

	Ch->af_marks_updated = TRUE;
	__calculate_dirty_percent( Ch);

	agh_edf_put_artifacts_as_garray( __source_ref, Ch->name,
					 Ch->af_track);
	agh_msmt_get_signal_filtered_as_double( Ch->rec_ref,
						&Ch->signal_filtered, NULL, NULL);
	agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
							     Ch->from, Ch->upto,
							     (double*)Ch->power->data);
	agh_msmt_get_power_spectrum_as_double( Ch->rec_ref, __cur_page,
					       &Ch->spectrum, &Ch->spectrum_max);
	gtk_widget_queue_draw( Ch->da_page);
	gtk_widget_queue_draw( Ch->da_powercourse);
	gtk_widget_queue_draw( Ch->da_spectrum);
}



gboolean
daScoringFacPageView_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	if ( __af_marking_in_widget != wid )
		return TRUE;

	gint wd;
	gdk_drawable_get_size( wid->window, &wd, NULL);
	__af_mark_virtual_end = (__cur_page_app + ((event->x > 0. ) ? event->x : 0) / wd) * APSZ;

	gtk_widget_queue_draw( wid);
	return TRUE;
}



gboolean
daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	if ( __unfazer_sel_state == UNF_SEL_CALIBRATE && Ch == __clicked_channel ) {
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
		if ( Ch->signal_display_scale < 250 )
			Ch->signal_display_scale *= 1.3;
	    break;
	case GDK_SCROLL_UP:
		if ( Ch->signal_display_scale > .001 )
			Ch->signal_display_scale /= 1.3;
	    break;
	default:
	    break;
	}

	if ( event->state & GDK_CONTROL_MASK ) {
		for ( guint h = 0; h < HH->len; ++h )
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
	if ( !Ch->visible || !gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);
	__ensure_enough_lines( wd*2);

	guint i, m;

      // hour ticks
	guint hours = __signal_length / 3600;
	for ( i = 0; i <= hours; ++i ) {
		guint tick_pos = (gfloat)(i * 3600) / __signal_length * wd;
		snprintf_buf( "<small>%2uh</small>", i);
		pango_layout_set_markup( __pp__, __buf__, -1);
		gdk_draw_layout( wid->window, __gc__[cTICKS],
				 tick_pos + 5,
				 5,
				 __pp__);
		gdk_draw_line( wid->window, __gc__[cTICKS],
			       tick_pos, 0,
			       tick_pos, 15);
	}

      // profile
	for ( i = 0, m = 0; i < wd; ++i, m += 2 ) {
		guint i_real = (gfloat)i / wd * Ch->power->len;
		LL(m).x = i;
		LL(m).y = - (Ai (Ch->power, double, i_real)
			     * AghPPuV2)
			+ ht;
		LL(m+1).x = i+1;
		LL(m+1).y = ht;
	}

	gdk_draw_lines( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			(GdkPoint*)__ll__->data, m);

      // cursor
	gdk_draw_rectangle( wid->window, __gc__[cCURSOR],
			    FALSE,
			    (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			    ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht);


	gdk_draw_line( wid->window, __gc__[cPOWER],
		       10, 10,
		       10, 10 + AghPPuV2/10);
	snprintf_buf( "<b><small>0.1 \302\265V\302\262</small></b>");
	pango_layout_set_markup( __pp__, __buf__, -1);
	gdk_draw_layout( wid->window, __gc__[cLABELS],
			 15, 15,
			 __pp__);

	snprintf_buf( "<b>%g - %g</b> Hz", Ch->from, Ch->upto);
	pango_layout_set_markup( __pp__, __buf__, -1);
	gdk_draw_layout( wid->window, __gc__[cLABELS],
			 wd - 50, 10,
			 __pp__);

	return TRUE;
}










gboolean
daScoringFacPSDProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( Ch->from > 0 )
			Ch->from--, Ch->upto--;
	    break;
	case GDK_SCROLL_UP:
		if ( Ch->upto < 10 )
			Ch->from++, Ch->upto++;
	    break;
	default:
	    break;
	}

	agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
							     Ch->from, Ch->upto,
							     (double*)Ch->power->data);

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
	{
		guint i_real = (guint)((gdouble)event->x / wd * Ch->power->len);
		AghPPuV2 = ht * 0.75 / Ai (Ch->power, double, i_real);
		gtk_widget_queue_draw( wid);
	}
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
	if ( __pagesize_item != AghDisplayPageSizeItem )
		return TRUE;

	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( !Ch->visible || !gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) )
		return TRUE;

      // spectrum
	guint	graph_height = AGH_DA_PSD_PROFILE_HEIGHT - 4,
		graph_width  = AGH_DA_SPECTRUM_WIDTH - 4;
	PangoRectangle extents;
	for ( gushort i = 1; i <= Ch->spectrum_upper_freq; ++i ) {
		gdk_draw_line( wid->window, __gc__[cSPECTRUM_GRID],
			       2 + (float)i/Ch->spectrum_upper_freq * graph_width, AGH_DA_PSD_PROFILE_HEIGHT - 2,
			       2 + (float)i/Ch->spectrum_upper_freq * graph_width, 2);
	}
	snprintf_buf( "<small><b>%u Hz</b></small>", Ch->spectrum_upper_freq);
	pango_layout_set_markup( __pp__, __buf__, -1);
	pango_layout_get_pixel_extents( __pp__, &extents, NULL);
	gdk_draw_layout( wid->window, __gc__[cLABELS],
			 AGH_DA_SPECTRUM_WIDTH - extents.width - 3, AGH_DA_PSD_PROFILE_HEIGHT - 2 - extents.height - 3,
			 __pp__);

	snprintf_buf( "<small><b>%c</b></small>", Ch->show_spectrum_absolute ? 'A' : 'R');
	pango_layout_set_markup( __pp__, __buf__, -1);
	pango_layout_get_pixel_extents( __pp__, &extents, NULL);
	gdk_draw_layout( wid->window, __gc__[cLABELS],
			 AGH_DA_SPECTRUM_WIDTH - extents.width - 3, 3,
			 __pp__);

	__ensure_enough_lines( Ch->n_bins);

	guint m;
	gfloat	factor = Ch->show_spectrum_absolute ? Ch->spectrum_display_scale : Ch->spectrum_max;
	for ( m = 0; m < Ch->spectrum_upper_freq; ++m ) {
		LL(m).x = 2 + (float)(graph_width) / (Ch->spectrum_upper_freq) * m;
		LL(m).y = AGH_DA_PSD_PROFILE_HEIGHT
			- (2 + (float)(graph_height) / factor * Ch->spectrum[m]);
	}
	gdk_draw_lines( wid->window, __gc__[cSPECTRUM],
			(GdkPoint*)__ll__->data, m);

	gdk_draw_line( wid->window, __gc__[cSPECTRUM_AXES],
		       2, 2,
		       2, graph_height + 2);
	gdk_draw_line( wid->window, __gc__[cSPECTRUM_AXES],
		       2, AGH_DA_PSD_PROFILE_HEIGHT - 2,
		       2 + graph_width, AGH_DA_PSD_PROFILE_HEIGHT - 2);

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
		Ch->show_spectrum_absolute = !Ch->show_spectrum_absolute;
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
			if ( Ch->spectrum_upper_freq > 2 )
				Ch->spectrum_upper_freq -= 1;
		} else
			if ( Ch->spectrum_display_scale < 1e7 )
				Ch->spectrum_display_scale *= 1.3;
	    break;
	case GDK_SCROLL_UP:
		if ( event->state & GDK_SHIFT_MASK ) {
			if ( Ch->spectrum_upper_freq < Ch->n_bins )
				Ch->spectrum_upper_freq += 1;
		} else
			if ( Ch->spectrum_display_scale > .001 )
				Ch->spectrum_display_scale /= 1.3;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}









// -------------------- EMG profile

inline
double avg( double* signal, guint x1, guint x2)
{
	double acc = 0.;
	for ( guint x = x1; x <= x2; ++x )
		acc += fabs( signal[x]);
	return acc / (x2-x1+1);
}

gboolean
daScoringFacEMGProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( !Ch->visible || !gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) )
		return TRUE;

	gint ht, wd;
	gdk_drawable_get_size( /*Ai(HH, SChannelPresentation, 0).da_ */ wid->window,
			       &wd, &ht);
	__ensure_enough_lines( wd*2);

	guint i, m;

      // hour ticks
	guint hours = __signal_length / 3600;
	for ( i = 0; i <= hours; ++i ) {
		guint tick_pos = (gfloat)(i * 3600) / __signal_length * wd;
		snprintf_buf( "<small>%2uh</small>", i);
		pango_layout_set_markup( __pp__, __buf__, -1);
		gdk_draw_layout( wid->window, __gc__[cTICKS],
				 tick_pos + 3,
				 3,
				 __pp__);
		gdk_draw_line( wid->window, __gc__[cTICKS],
			       tick_pos, 0,
			       tick_pos, 8);
	}

      // precompute
	if ( Ch->emg_precomputed->len != wd ) {
		GArray *tmp = g_array_sized_new( FALSE, FALSE, sizeof(double), wd);
		double largest = 0.;
		for ( i = 0; i < wd; ++i ) {
			guint	i_real      = (gfloat)i     / wd * Ch->n_samples,
				i_real_next = (gfloat)(i+1) / wd * Ch->n_samples;
			double	current = Ai (tmp, double, i)
				= avg( Ch->signal_original, i_real, i_real_next);
			if ( largest < current )
				largest = current;
		}

		g_array_set_size( Ch->emg_precomputed, wd);
		for ( i = 0; i < wd; ++i )
			Ai (Ch->emg_precomputed, float, i) =
				Ai (tmp, double, i) / largest * AGH_DA_EMG_PROFILE_HEIGHT / 2;

		g_array_free( tmp, TRUE);
	}


	for ( i = 0, m = 0; i < wd; ++i, m += 2 ) {
		LL(m).x   = i;
		LL(m).y   = AGH_DA_EMG_PROFILE_HEIGHT/2 - Ai (Ch->emg_precomputed, float, i) * Ch->emg_scale;
		LL(m+1).x = i+1;
		LL(m+1).y = AGH_DA_EMG_PROFILE_HEIGHT/2 + Ai (Ch->emg_precomputed, float, i) * Ch->emg_scale;
	}

	gdk_draw_lines( wid->window, __gc__[cEMG],
			(GdkPoint*)__ll__->data, m);

      // cursor
	gdk_draw_rectangle( wid->window, __gc__[cCURSOR],
			    FALSE,
			    (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			    ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht);

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

	guint i;
	for ( i = 1; i < 8; ++i )
		gdk_draw_line( wid->window, __gc__[i],  // AGH_SCORE_* coincides here with cSCORE_*
			       0,   __score_hypn_depth[i],
			       wd,  __score_hypn_depth[i]);


	// these lines can be discontinuous: cannot use gdk_draw_lines in one swoop
	for ( i = 0; i < __hypnogram->len; ++i ) {
		gchar c;
		if ( (c = Ai (__hypnogram, gchar, i)) != AghScoreCodes[AGH_SCORE_NONE] ) {
			gint y = __score_hypn_depth[ SCOREID(c) ];
			gdk_draw_line( wid->window, __gc__[cHYPNOGRAM_SCORELINE],
				       lroundf( (gfloat) i   /__hypnogram->len * wd), y,
				       lroundf( (gfloat)(i+1)/__hypnogram->len * wd), y);
		}
	}

	gdk_draw_rectangle( wid->window, __gc__[cCURSOR],
			    FALSE,
			    (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			    ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht);

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
	if ( __cur_page < __total_pages ) {
		Ai (__hypnogram, gchar, __cur_page) = score_ch;
		__cur_page++;
		__cur_page_app++; //  = P2AP (__cur_page);
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
	if ( __cur_page_app * APSZ < __signal_length ) {
		__cur_page_app++;
		__cur_page = AP2P (__cur_page_app);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	}
}

void
bScoringFacBack_clicked_cb()
{
	if ( __cur_page_app ) {
		__cur_page_app--;
		__cur_page = AP2P (__cur_page_app);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	}
}



void
eScoringFacPageSize_changed_cb()
{
	guint cur_pos = __cur_page_app * APSZ;
	__pagesize_item = gtk_combo_box_get_active( GTK_COMBO_BOX (eScoringFacPageSize));
	__cur_page_app = cur_pos / APSZ;
//	__cur_page = AP2P (__cur_page_app); // shouldn't change

	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
	gtk_spin_button_set_range( GTK_SPIN_BUTTON (eScoringFacCurrentPage), 1, P2AP (__total_pages));

	snprintf_buf( "<small>of</small> %d", P2AP (__total_pages));
	gtk_label_set_markup( GTK_LABEL (lScoringFacTotalPages), __buf__);

	gboolean pagesize_is_right = (__pagesize_item == AghDisplayPageSizeItem);
	gtk_widget_set_sensitive( bScoreClear, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM1, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM2, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM3, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM4, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreREM,   pagesize_is_right);
	gtk_widget_set_sensitive( bScoreWake,  pagesize_is_right);
	gtk_widget_set_sensitive( bScoreMVT,   pagesize_is_right);
	gtk_widget_set_sensitive( bScoreGotoPrevUnscored, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreGotoNextUnscored, pagesize_is_right);

	if ( !pagesize_is_right )
		for ( guint h = 0; h < HH->len; ++h ) {
			SChannelPresentation *Ch = &Ai( HH, SChannelPresentation, h);
			if ( Ch->visible && Ch->da_powercourse ) {
				g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
				gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg__[cSIGNAL_SCORE_NONE]);
				gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg__[cSIGNAL_SCORE_NONE]);
				g_signal_handler_unblock( Ch->da_page, Ch->expose_handler_id);
			}
		}

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
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

	gboolean pagesize_is_right = (APSZ == PSZ);
	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai( HH, SChannelPresentation, h);
		if ( Ch->visible && gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) &&
		     Ch->da_page ) {

			if ( pagesize_is_right && Ch->da_powercourse
			     && (__unfazer_sel_state == 0 || __clicked_channel != Ch) ) {
				g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
				gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg__[__cur_stage]);
				gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg__[__cur_stage]);
				g_signal_handler_unblock( Ch->da_page, Ch->expose_handler_id);
			}
			gtk_widget_queue_draw( Ch->da_page);

			if ( Ch->da_powercourse ) {
				if ( Ch->spectrum ) {
					free( Ch->spectrum);
					agh_msmt_get_power_spectrum_as_double( Ch->rec_ref, __cur_page,
									       &Ch->spectrum, &Ch->spectrum_max);
					gtk_widget_queue_draw( Ch->da_spectrum);
				}
				gtk_widget_queue_draw( Ch->da_powercourse);
			}
			if ( Ch->da_emg_profile )
				gtk_widget_queue_draw( Ch->da_emg_profile);
		}
	}
	snprintf_buf( "<b><big>%s</big></b>", __score_names[ __cur_stage ]);
	gtk_label_set_markup( GTK_LABEL (lScoringFacCurrentStage), __buf__);

	snprintf_buf( "<b>%2d:%02d:%02d</b>", __cur_pos_hr, __cur_pos_min, __cur_pos_sec);
	gtk_label_set_markup( GTK_LABEL (lScoringFacCurrentPos), __buf__);

	gtk_widget_queue_draw( daScoringFacHypnogram);
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
	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai( HH, SChannelPresentation, h);
		if ( Ch->af_track ) {
			char	*pa = &Ai (Ch->af_track, gchar, p*__fft_pagesize),
				*pz = strchr( pa, 'x');
			if ( pz && pz - pa < __fft_pagesize )
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







// ------ menu callbacks

void
iSFPowerExportRange_activate_cb()
{
	GString *fname_buf = g_string_sized_new(120),
		*messages = g_string_sized_new(200);
	g_string_assign( messages, "Wrote the following files:\n");
	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->power ) {
			char *fname_base = agh_msmt_fname_base( Ch->rec_ref);
			g_string_printf( fname_buf, "%s_%g-%g.tsv",
					 fname_base, Ch->from, Ch->upto);
			free( (void*)fname_base);
			agh_msmt_export_power_in_range( Ch->rec_ref, Ch->from, Ch->upto,
							fname_buf->str);
			g_string_append_printf( messages, "* %s\n", fname_buf->str);
		}
	}
	g_string_free( fname_buf, TRUE);
	pop_ok_message( GTK_WINDOW (wScoringFacility), messages->str);
	g_string_free( messages, TRUE);
}

void
iSFPowerExportAll_activate_cb()
{
	GString *fname_buf = g_string_sized_new(120),
		*messages = g_string_sized_new(200);
	g_string_assign( messages, "Wrote the following files:\n");
	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->power ) {
			char *fname_base = agh_msmt_fname_base( Ch->rec_ref);
			g_string_printf( fname_buf, "%s.tsv",
					 fname_base);
			free( (void*)fname_base);
			agh_msmt_export_power( Ch->rec_ref,
					       fname_buf->str);
			g_string_append_printf( messages, "* %s\n", fname_buf->str);
		}
	}
	g_string_free( fname_buf, TRUE);
	pop_ok_message( GTK_WINDOW (wScoringFacility), messages->str);
	g_string_free( messages, TRUE);
}





void
iSFArtifactsApply_clicked_cb()
{
	set_cursor_busy( TRUE, wScoringFacility);

	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->af_track && Ch->af_marks_updated ) {
//			printf( "%s has changed artifacts:\n%s\n", Ch->name, Ch->af_track->data);
			agh_edf_put_artifacts_as_garray( __source_ref, Ch->name,
							 Ch->af_track);
			agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     (double*)Ch->power->data);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);

			Ch->af_marks_updated = FALSE;
		}
	}

      // update power profile in measurements view
	__collect_and_paint_one_subject_episodes();

	set_cursor_busy( FALSE, wScoringFacility);
}



void
iSFArtifactsClear_activate_cb()
{
	if ( pop_question( GTK_WINDOW (wScoringFacility),
			   "All marked artifacts will be lost in all channels. Continue?") != GTK_RESPONSE_YES )
		return;

	guint h;
	for ( h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->power )
			memset( Ch->af_track->data, (int)' ', Ch->af_track->len);
	}

	set_cursor_busy( TRUE, wScoringFacility);

	for ( h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->power ) {
			agh_edf_put_artifacts_as_garray( __source_ref, Ch->name,
							 Ch->af_track);
			agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     (double*)Ch->power->data);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);
		}
	}

      // update power profile in measurements view
	__collect_and_paint_one_subject_episodes();

	set_cursor_busy( FALSE, wScoringFacility);
}


void
iSFArtifactsShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->show_original_signal = gtk_check_menu_item_get_active( checkmenuitem);
      // prevent both being switched off
	if ( !__clicked_channel->show_original_signal && !__clicked_channel->show_processed_signal )
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFArtifactsShowProcessed),
						__clicked_channel->show_processed_signal = TRUE);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}


void
iSFArtifactsShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer unused)
{
	__clicked_channel->show_processed_signal = gtk_check_menu_item_get_active( checkmenuitem);
	if ( !__clicked_channel->show_processed_signal && !__clicked_channel->show_original_signal )
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (iSFArtifactsShowOriginal),
						__clicked_channel->show_original_signal = TRUE);
	gtk_widget_queue_draw( __clicked_channel->da_page);
}


void
iSFArtifactsMarkMVT_activate_cb()
{
	double *spectrum;
	for ( guint p = 0; p < __total_pages; ++p ) {
		agh_msmt_get_power_spectrum_as_double( __clicked_channel->rec_ref, p, &spectrum, NULL);
		if ( spectrum[4] * 5 < spectrum[5] )
			Ai (__hypnogram, gchar, p) = AghScoreCodes[AGH_SCORE_MVT];
		free( spectrum);
	}
	__repaint_score_stats();

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
}


void
iSFArtifactsUnfazer_activate_cb()
{
	__unfazer_sel_state = UNF_SEL_CHANNEL;

	g_signal_handler_block( __clicked_channel->da_page, __clicked_channel->expose_handler_id);
	gtk_widget_modify_fg( __clicked_channel->da_page, GTK_STATE_NORMAL, &__fg__[cSIGNAL_UNFAZER]);
	gtk_widget_modify_bg( __clicked_channel->da_page, GTK_STATE_NORMAL, &__bg__[cSIGNAL_UNFAZER]);
	g_signal_handler_unblock( __clicked_channel->da_page, __clicked_channel->expose_handler_id);

	gtk_widget_queue_draw( __clicked_channel->da_page);
}






void
iSFScoreAssist_activate_cb()
{
	agh_edf_run_scoring_assistant( __source_ref);
	agh_edf_get_scores_as_garray( __source_ref, __hypnogram, NULL);

	__repaint_score_stats();
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
}



void
bSFScore_clicked_cb()
{
	for ( guint h = 0; h < HH->len; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( Ch->af_track && Ch->af_marks_updated ) {
			agh_edf_put_artifacts_as_garray( __source_ref, Ch->name,
							 Ch->af_track);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);
		}
	}

	agh_edf_put_scores_as_garray( __source_ref,
				      __hypnogram);
      // update power profile in measurements view
	__collect_and_paint_one_subject_episodes();

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
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
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

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
}

void
iSFScoreClear_activate_cb()
{
	g_array_set_size( __hypnogram, 0);
	g_array_set_size( __hypnogram, __signal_length / PSZ);

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");

	snprintf_buf( "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);
}


// EOF
