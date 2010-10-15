// ;-*-C-*- *  Time-stamp: "2010-10-15 20:44:08 hmmr"
/*
 *       File name:  scoring-facility.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  scoring facility
 *
 *         License:  GPL
 */



#include <glade/glade.h>
#include <string.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include "../core/iface.h"
#include "../core/iface-glib.h"
#include "misc.h"
#include "ui.h"


static gchar	__buf__[256];


void __paint_one_subject_episodes();  // in measurements view, that is


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
	*bScoreREM,   *bScoreWake,  *bScoreMVT;

static GtkWidget
	*mSFArtifacts,	*mSFPower, *mSFScore;


#define AGH_SCORE__UNFAZER (AGH_SCORE_MVT+1)

static const gchar* const __bg_rgb_score[] = {
	"#2A0952", "#2D2D8B", "#000064", "#000046", // sleep stages
	"#00002a", "#610863", "#8D987F", "#4F3D02",
	"#000000", "#000000",
	"#000000",  // unfazer
};

static const gchar* const __fg_rgb_score[] = {
	"#dddddd", "#FFEDB5", "#FFEDB5", "#FFEDB5",
	"#FFF7DF", "#E2E2E2", "#BDBDBD", "#BDBDBD",
	"#FFFFFF", "#00FE1E",
	"#EEEEFF",
};

static const gchar __bg_rgb_power[] = "#FFFFFF";
static const gchar __fg_rgb_power[] = "#2222FF";
static const gchar __bg_rgb_hypno[] = "#2E8B57";

static const gchar __fg_rgb_highlight[] = "#FFEEFF";
static const gchar __bg_rgb_highlight[] = "#000000";


static GdkColor	__fg_score[11], __bg_score[11],
		__fg_power,     __bg_power,
		__bg_hypno,
		__fg_highlight, __bg_highlight;

static GdkGC
	*__gc_score[11],
	*__gc_score_line,
	*__gc_score_cursor,
	*__gc_power,
	*__gc_highlight;






static guint		AghPagesizeValues[] = { 5, 10, 15, 20, 30, 60, 120, 300 };
static const guint	AghPagesizeNItems = 8;
guint			AghPagesizeItem = 4;  // the one used to obtain FFTs

static guint	__pagesize_item = 6;  // pagesize as currently displayed



static guint __pagesize_ticks[] = {
	5, 5, 3, 4, 6, 12, 24, 30
};





gboolean daScoringFacPageView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
gboolean daScoringFacPageView_key_press_event_cb( GtkWidget*, GdkEventKey*, gpointer);
gboolean daScoringFacPageView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daScoringFacPageView_button_release_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daScoringFacPageView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean cScoringFacPageViewExpander_activate_cb( GtkExpander*, gpointer);

gboolean daScoringFacProfileView_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
gboolean daScoringFacProfileView_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daScoringFacProfileView_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);








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
	     !(bScoreMVT    = glade_xml_get_widget( xml, "bScoreMVT")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eScoringFacPageSize),
				 GTK_TREE_MODEL (agh_mScoringPageSize));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eScoringFacPageSize), renderer,
					"text", 0,
					NULL);

	GdkGCValues xx;
	guint i;
	for ( i = 0; i < 11; i++ ) {
		gdk_color_parse( __fg_rgb_score[i], &__fg_score[i]), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__fg_score[i], FALSE, TRUE);
		gdk_color_parse( __bg_rgb_score[i], &__bg_score[i]), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__bg_score[i], FALSE, TRUE);

		xx.foreground = __bg_score[i], xx.background = __fg_score[0];  // bg <-> fg
		xx.line_width = 1, xx.line_style = GDK_LINE_SOLID;

		__gc_score[i] = gtk_gc_get( agh_visual->depth, gtk_widget_get_colormap( daScoringFacHypnogram),
					    &xx, GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);
	}
	gdk_color_parse( __bg_rgb_hypno, &__bg_hypno), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__bg_hypno, FALSE, TRUE);
	gtk_widget_modify_bg( daScoringFacHypnogram, GTK_STATE_NORMAL, &__bg_hypno);

	gdk_color_parse( __fg_rgb_score[8], &__fg_score[8]), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__fg_score[8], FALSE, TRUE);
	xx.foreground = __fg_score[8];
	xx.line_width = 2, xx.line_style = GDK_LINE_SOLID, xx.join_style = GDK_JOIN_ROUND, xx.cap_style = GDK_CAP_BUTT;
	__gc_score_line = gtk_gc_get( agh_visual->depth, gtk_widget_get_colormap( daScoringFacHypnogram),
				      &xx, GDK_GC_FOREGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE | GDK_GC_JOIN_STYLE | GDK_GC_CAP_STYLE);

	gdk_color_parse( __fg_rgb_score[9], &__fg_score[9]), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__fg_score[9], FALSE, TRUE);
	xx.foreground = __fg_score[9];
	xx.line_width = 1, xx.line_style = GDK_LINE_SOLID;
	__gc_score_cursor = gtk_gc_get( agh_visual->depth, gtk_widget_get_colormap( daScoringFacHypnogram),
					&xx, GDK_GC_FOREGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);

	gdk_color_parse( __fg_rgb_power, &__fg_power), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__fg_power, FALSE, TRUE);
	gdk_color_parse( __bg_rgb_power, &__bg_power), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__bg_power, FALSE, TRUE);
	xx.foreground = __fg_power, xx.background = __bg_power;
	xx.line_width = 1, xx.line_style = GDK_LINE_SOLID;
	__gc_power = gtk_gc_get( agh_visual->depth, gtk_widget_get_colormap( daScoringFacHypnogram),
				 &xx, GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);

	gdk_color_parse( __fg_rgb_highlight, &__fg_highlight), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__fg_highlight, FALSE, TRUE);
	gdk_color_parse( __bg_rgb_highlight, &__bg_highlight), gdk_colormap_alloc_color( gtk_widget_get_colormap( daScoringFacHypnogram), &__bg_highlight, FALSE, TRUE);
	xx.foreground = __fg_highlight, xx.background = __bg_highlight;
	xx.line_width = 1, xx.line_style = GDK_LINE_SOLID;
	__gc_highlight = gtk_gc_get( agh_visual->depth, gtk_widget_get_colormap( daScoringFacHypnogram),
				     &xx, GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);

      // ------- menus and Android Notify (TM)
	if ( !(mSFArtifacts       = glade_xml_get_widget( xml, "mSFArtifacts")) ||
	     !(mSFPower    	  = glade_xml_get_widget( xml, "mSFPower")) ||
	     !(mSFScore    	  = glade_xml_get_widget( xml, "mSFScore")) )
		return -1;

	return 0;
}

void
agh_ui_destruct_ScoringFacility()
{
	// __gc_...
}








static gsize	__signal_length; // in sec
static gsize	__total_pages, __fft_pagesize;
static guint	__cur_page,
		__cur_page_app,
		__cur_pos_hr, __cur_pos_min, __cur_pos_sec;

typedef struct {
	gchar	  *name;
	guint	   slot;
	TRecRef    rec_ref;

	double	  *signal_data,
		  *signal_data_orig;
	gsize	   n_samples;
	gsize	   samplerate;
	gfloat	   display_scale;
	GtkWidget *da_page;

	struct SUnfazer
		  *unfazers;
	guint	   n_unfazers;

	GArray	  *power;
	gfloat	   from,
		   upto;
	gdouble	   max_value;
	GtkWidget *da_powercourse;

	GArray	  *track;
	gboolean   new_marks;
	gfloat	   dirty_percent;

	gboolean   visible;
	GtkWidget *expander,
		  *vbox;
	gulong     expose_handler_id;

} SChannelPresentation;

static void
__destroy_ch( SChannelPresentation *Ch)
{
	if ( Ch->signal_data )      { free( Ch->signal_data);       Ch->signal_data      = NULL; }
	if ( Ch->signal_data_orig ) { free( Ch->signal_data_orig);  Ch->signal_data_orig = NULL; }
	if ( Ch->unfazers )  	    { free( Ch->unfazers);          Ch->unfazers  = NULL; }
	if ( Ch->power )     { g_array_free( Ch->power,    TRUE);   Ch->power     = NULL; }
	if ( Ch->track )     { g_array_free( Ch->track,    TRUE);   Ch->track     = NULL; }
}


static GArray	*__channels;
static TEDFRef	__source_ref;  // the core structures allow for multiple edf
                               // sources providing signals for a single episode;
                               // keeping only one __source_ref here will, then,
                               // read/write scores in this source's histogram;
// -- but it's essentially not a problem since all edf sources will still converge
//    to the same .histogram file
static GArray	*__hypnogram;






#define PSZ  AghPagesizeValues[AghPagesizeItem]
#define APSZ AghPagesizeValues[__pagesize_item]

#define P2AP(p)  (guint)((p) * (float)PSZ / (float)APSZ)
#define AP2P(p)  (guint)((p) * (float)APSZ / (float)PSZ)





static void
__calculate_dirty_percent( SChannelPresentation *Ch)
{
	guint	dirty_secs = 0,
		p;
	for ( p = 0; p < Ch->track->len; p++ )
		dirty_secs += (Ai (Ch->track, gchar, p) == 'x');
	Ch->dirty_percent = (gfloat) dirty_secs / p * 100;
}




static void __repaint_score_stats();


gboolean
agh_prepare_scoring_facility()
{
      // prepare structures for the first viewing
	if ( !__channels ) {
		__channels  = g_array_new( FALSE, FALSE, sizeof(SChannelPresentation));
		__hypnogram = g_array_new( FALSE,  TRUE, sizeof(gchar));
	}

      // clean up after previous viewing
	gtk_container_foreach( GTK_CONTAINER (cScoringFacPageViews),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	guint h;
	for ( h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		__destroy_ch(Ch);
	}
	__cur_page = 0;
	__signal_length = 0; // is set to the longest signal, below


      // set up channel representations
	g_array_set_size( __channels, AghHs);

	guint n_visible = 0;
	for ( h = 0; h < AghHs; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		Ch->name = AghHH[h];
		Ch->slot = h;

		Ch->rec_ref = agh_msmt_find_by_jdeh( AghJ->name, AghD, AghE, Ch->name);
		if ( Ch->rec_ref == NULL ) {
			fprintf( stderr, "agh_prepare_scoring_facility(): no measurement matching (%s, %s, %s, %s)\n",
				 AghJ->name, AghD, AghE, Ch->name);
			Ch->signal_data = Ch->signal_data_orig = NULL, Ch->power = Ch->track = NULL;
			continue;
		}

		if ( h == 0 )
			__source_ref = agh_msmt_get_source( Ch->rec_ref);

	      // get signal data
		Ch->n_samples = agh_msmt_get_signal_data_as_double( Ch->rec_ref,
								    &Ch->signal_data_orig, &Ch->samplerate, NULL);
		agh_msmt_get_signal_data_unfazed_as_double( Ch->rec_ref,
							    &Ch->signal_data, NULL, NULL);

		Ch->from = AghQuickViewFreqFrom, Ch->upto = AghQuickViewFreqUpto;
		if ( agh_signal_type_is_fftable(
			     agh_msmt_get_signal_type( Ch->rec_ref)) ) {
			Ch->power = g_array_new( FALSE, FALSE, sizeof(double));
			Ch->track = g_array_new( TRUE, FALSE, sizeof(char));  // zero-terminate for strxxx() to work
			// the first call to get power course is *_as_garray; others will use *_direct
			agh_msmt_get_power_course_in_range_as_double_garray( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     Ch->power);
			FAFA;
			Ch->n_unfazers = agh_edf_get_unfazers( __source_ref,
							       Ch->name,
							       &Ch->unfazers);
			FAFA;
			agh_msmt_get_track_as_garray( Ch->rec_ref,
						      Ch->track);
			__calculate_dirty_percent( Ch);
		} else
			Ch->power = Ch->track = NULL;

		Ch->new_marks = FALSE;

		if ( Ch->n_samples == 0 )
			Ch->visible = FALSE;
		else {
			Ch->visible = TRUE;
			n_visible++;

			Ch->expander = gtk_expander_new( Ch->name);
			gtk_box_pack_start( GTK_BOX (cScoringFacPageViews),
					    Ch->expander, TRUE, TRUE, 3);
			gtk_expander_set_expanded( GTK_EXPANDER (Ch->expander),
						   TRUE /*h == AghHi*/);
			g_signal_connect_after( Ch->expander, "activate",
					  G_CALLBACK (cScoringFacPageViewExpander_activate_cb),
					  (gpointer)Ch);

			gtk_container_add( GTK_CONTAINER (Ch->expander),
					   Ch->vbox = gtk_vbox_new( FALSE, 3));

		      // set up page view
			if ( Ch->n_samples ) {
				__signal_length = MAX( __signal_length, Ch->n_samples / Ch->samplerate);
				Ch->display_scale = 20;

				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   Ch->da_page = gtk_drawing_area_new());
				g_object_set( G_OBJECT (Ch->da_page),
					      "app-paintable", TRUE,
					      "height-request", 120,
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
				g_signal_connect_after( Ch->da_page, "scroll-event",
							G_CALLBACK (daScoringFacPageView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_page, (GdkEventMask) GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
			} else
				Ch->da_page = NULL;

		      // set up profile view
			if ( Ch->power && Ch->power->len ) {
				__signal_length = MAX( __signal_length,
						       Ch->power->len * PSZ);

				gtk_container_add( GTK_CONTAINER (Ch->vbox),
						   Ch->da_powercourse = gtk_drawing_area_new());
				g_object_set( G_OBJECT (Ch->da_powercourse),
					      "app-paintable", TRUE,
					      "height-request", 60,
					      NULL);
				g_signal_connect_after( Ch->da_powercourse, "expose-event",
							G_CALLBACK (daScoringFacProfileView_expose_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_powercourse, "button-press-event",
							G_CALLBACK (daScoringFacProfileView_button_press_event_cb),
							(gpointer)Ch);
				g_signal_connect_after( Ch->da_powercourse, "scroll-event",
							G_CALLBACK (daScoringFacProfileView_scroll_event_cb),
							(gpointer)Ch);
				gtk_widget_add_events( Ch->da_powercourse, (GdkEventMask) GDK_BUTTON_PRESS_MASK);

				gtk_widget_modify_fg( Ch->da_powercourse, GTK_STATE_NORMAL, &__fg_power);
				gtk_widget_modify_bg( Ch->da_powercourse, GTK_STATE_NORMAL, &__bg_power);

			} else
				Ch->da_powercourse = NULL;
		}

	}

	set_cursor_busy( FALSE, wMainWindow);

	if ( !n_visible || !Ai (__channels, SChannelPresentation, AghHi).expander )
		return FALSE;

      // get scores
	g_array_set_size( __hypnogram, 0);
	__total_pages = agh_edf_get_scores_as_garray( __source_ref,
						      __hypnogram, &__fft_pagesize);

      // set up other controls
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage),
				   __cur_page = __cur_page_app = 1);
	gtk_combo_box_set_active( GTK_COMBO_BOX (eScoringFacPageSize),
				  __pagesize_item = AghPagesizeItem);

	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	g_signal_emit_by_name( eScoringFacPageSize, "changed");

	__repaint_score_stats();

	gtk_widget_queue_draw( cMeasurements);

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




static GtkWidget *__marking_in_widget;
static guint __mark_start;
static void __mark_region( guint, guint, SChannelPresentation*, gchar);


enum {
	UNF_SEL_CHANNEL = 1,
	UNF_SEL_CALIBRATE = 2
};
static gint __unfazer_sel_state = 0;
static SChannelPresentation
	*__unfazer_affected_channel,
	*__unfazer_offending_channel;
static float
	__unfazer_factor = 0.1;


static gboolean
	__show_processed_signal = TRUE,
	__show_original_signal = FALSE;

// -------------------- Page

gboolean
cScoringFacPageViewExpander_activate_cb( GtkExpander *expander, gpointer userdata)
{
	g_signal_emit_by_name( eScoringFacCurrentPage, "value-changed");
	return TRUE;
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
	static PangoLayout *layout = NULL;
	static GArray *lines = NULL;
	if ( !lines ) {
		lines = g_array_sized_new( FALSE, FALSE, wd, sizeof(GdkPoint));
		layout = gtk_widget_create_pango_layout( wid, "");
	}

	if ( lines->len < wd )
		g_array_set_size( lines, wd);

	guint i;

      // uV scale
	guint dpuV = 1 * (ht / Ch->display_scale);
	gdk_draw_line( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
		       10, 10,
		       10, 10 + dpuV);

	snprintf( __buf__, 50, "<b><small>1 \302\265V</small></b>");
	pango_layout_set_markup( layout, __buf__, -1);
	gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			 15, 10,
			 layout);

      // ticks
	for ( i = 0; i < __pagesize_ticks[__pagesize_item]; ++i ) {
		guint tick_pos = i * APSZ / __pagesize_ticks[__pagesize_item];
		snprintf( __buf__, 23, "<small>%2d</small>", tick_pos);
		pango_layout_set_markup( layout, __buf__, -1);
		gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
				 i * wd / __pagesize_ticks[__pagesize_item] + 5,
				 120 - 15,
				 layout);
		gdk_draw_line( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			       i * wd / __pagesize_ticks[__pagesize_item], 110,
			       i * wd / __pagesize_ticks[__pagesize_item], 100);
	}

      // waveform: signal_data
	if ( __show_processed_signal && Ch->track
	     && __unfazer_sel_state == 0 ) {  // only show processed signal when done with unfazing
		for ( i = 0; i < wd; ++i ) {
			Ai (lines, GdkPoint, i).x = i;
			Ai (lines, GdkPoint, i).y =
				- Ch->signal_data[ lroundf(((gfloat)i / wd + __cur_page_app) * Ch->samplerate * APSZ) ]
				* (ht / Ch->display_scale)
				+ ht/2;
		}

		gdk_draw_lines( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
				(GdkPoint*)lines->data, lines->len);
	}

      // waveform: signal_data_orig
	if ( __show_original_signal || !Ch->track ) {
		for ( i = 0; i < wd; ++i ) {
			Ai (lines, GdkPoint, i).x = i;
			Ai (lines, GdkPoint, i).y =
				- Ch->signal_data_orig[ lroundf(((gfloat)i / wd + __cur_page_app) * Ch->samplerate * APSZ) ]
				* (ht / Ch->display_scale)
				+ ht/2;
		}

		gdk_draw_lines( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
				(GdkPoint*)lines->data, lines->len);

		snprintf( __buf__, 220, "<b>orig</b>");
		pango_layout_set_markup( layout, __buf__, -1);
		gdk_draw_layout( wid->window, __gc_highlight,
				 20,
				 ht - 5,
				 layout);
	}

      // unfazer
	if ( __unfazer_sel_state ) {
		PangoRectangle extents;
		if ( Ch == __unfazer_affected_channel ) {
			switch ( __unfazer_sel_state ) {
			case UNF_SEL_CHANNEL:
				snprintf( __buf__, 220, "<big><b>Unfaze this channel from...</b></big>");
			    break;
			case UNF_SEL_CALIBRATE:
				snprintf( __buf__, 220, "<big><b>Unfaze this channel from %s</b></big>",
					  __unfazer_offending_channel->name);
				// show the signal being set up for unfazer live
				guint subscript;
				for ( i = 0; i < wd; ++i ) {
					Ai (lines, GdkPoint, i).x = i;
					Ai (lines, GdkPoint, i).y =
						(subscript = lroundf(((gfloat)i / wd + __cur_page_app) * Ch->samplerate * APSZ),
						 - (Ch->signal_data_orig[ subscript ]
						    - __unfazer_offending_channel->signal_data_orig[ subscript ] * __unfazer_factor)
						 * (ht / Ch->display_scale)
						 + ht/2);
				}

				gdk_draw_lines( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
						(GdkPoint*)lines->data, lines->len);

			    break;
			}
			pango_layout_set_markup( layout, __buf__, -1);
			pango_layout_get_pixel_extents( layout, &extents, NULL);
			gdk_draw_layout( wid->window, __gc_highlight,
					 wd/2 - extents.width/2,
					 ht - 35,
					 layout);

		} else if ( Ch == __unfazer_offending_channel ) {
			switch ( __unfazer_sel_state ) {
			case UNF_SEL_CHANNEL:
			    break;
			case UNF_SEL_CALIBRATE:
				snprintf( __buf__, 220, "<big><b>Calibrating unfaze factor:</b> %4.2f</big>",
					  __unfazer_factor);
				break;
			}
			pango_layout_set_markup( layout, __buf__, -1);
			pango_layout_get_pixel_extents( layout, &extents, NULL);
			gdk_draw_layout( wid->window, __gc_highlight,
					 wd/2 - extents.width/2,
					 ht - 35,
					 layout);
		}
	}

      // unfazer info
	if ( Ch->unfazers ) {
		static GString *unf_buf = NULL;
		if ( unf_buf == NULL )
			unf_buf = g_string_sized_new( 128);
		g_string_assign( unf_buf, "Unf: ");
		for ( i = 0; i < Ch->n_unfazers; ++i ) {
			g_string_append_printf( unf_buf, "<small><b>%s: %5.3f%c</b></small>",
						Ch->unfazers[i].channel, Ch->unfazers[i].factor,
						(i+1 == Ch->n_unfazers) ? ' ' : ';');
		}
		pango_layout_set_markup( layout, unf_buf->str, -1);
		gdk_draw_layout( wid->window, __gc_highlight,
				 10,
				 ht - 35,
				 layout);
	}

      // artifacts
	if ( Ch->track ) {
		guint	cur_page_start_s =  __cur_page_app      * APSZ,
			cur_page_end_s   = (__cur_page_app + 1) * APSZ;
		for ( i = cur_page_start_s; i < cur_page_end_s; i++ ) {
			if ( Ai (Ch->track, gchar, i) == 'x' ) {
				snprintf( __buf__, 20, "<b>\342\234\230</b>");
				pango_layout_set_markup( layout, __buf__, -1);
				gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
						 (i % APSZ + .5)
						 / APSZ * wd,
						 15,
						 layout);
			}
		}
		snprintf( __buf__, 40, "<small><i>%4.2f %% dirty</i></small>", Ch->dirty_percent);
		pango_layout_set_markup( layout, __buf__, -1);
		gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
				 wd - 70,
				 ht - 15,
				 layout);
	}


	return TRUE;
}



static int __chaining_next_key = -1;

gboolean
daScoringFacPageView_key_press_event_cb( GtkWidget *wid, GdkEventKey *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	if ( event->type == GDK_KEY_PRESS )
		switch ( event->keyval ) {
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
				Ai ( __channels, SChannelPresentation, h).da_page = Ch->da_page;
				__chaining_next_key = -1;
			}
			break;
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
				if ( strcmp( Ch->name, __unfazer_affected_channel->name) != 0 ) {
					__unfazer_offending_channel = Ch;
					__unfazer_sel_state = UNF_SEL_CALIBRATE;
				} else {
					__unfazer_sel_state = 0;
				}
			else
				__unfazer_sel_state = 0;
			gtk_widget_queue_draw( wid);
		    break;
		case UNF_SEL_CALIBRATE:
			if ( event->button == 1 && __unfazer_affected_channel == Ch ) {
				agh_edf_add_or_mod_unfazer( agh_msmt_get_source( Ch->rec_ref),  // apply
							    __unfazer_affected_channel->name,
							    __unfazer_offending_channel->name,
							    __unfazer_factor);
				agh_msmt_get_signal_data_unfazed_as_double( Ch->rec_ref,
									    &Ch->signal_data, NULL, NULL);
			} else
				; // cancel
			__unfazer_sel_state = 0;
		    break;
		}
		gtk_widget_queue_draw( wid);
		return TRUE;
	}

	switch ( event->button ) {
	case 2:
		if ( event->state & GDK_CONTROL_MASK )
			for ( h = 0; h < __channels->len; ++h )
				Ai (__channels, SChannelPresentation, h).display_scale = 20;
		else
			Ch->display_scale = 20;
		gtk_widget_queue_draw( wid);
	    break;
	case 3:
		if ( Ch->track && event->y > ht/2 ) {
			__unfazer_affected_channel = Ch;  // no other way to mark this channel even though user may not select Unfaze
			gtk_menu_popup( GTK_MENU (mSFArtifacts),
					NULL, NULL, NULL, NULL, 3, event->time);
			break;
		}
	case 1:
		if ( Ch->track ) {
			__marking_in_widget = wid;
			__mark_start = (__cur_page_app + event->x / wd) * APSZ;
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

	if ( wid != __marking_in_widget || !Ch->track )
		return TRUE;
	switch ( event->button ) {
	case 1:
		__mark_region( __mark_start, (__cur_page_app + event->x / wd) * APSZ, Ch, 'x');
	    break;
	case 3:
		__mark_region( __mark_start, (__cur_page_app + event->x / wd) * APSZ, Ch, ' ');
	    break;
	}

	// if ( event->state & GDK_MOD1_MASK ) {
	// 	agh_msmt_get_track_as_garray( AghJ->name, AghD->str, AghE->str, Ch->name, Ch->track);
	// 	gtk_widget_queue_draw( Ch->da_powercourse);
	// 	Ch->new_marks = FALSE;
	// }

	return TRUE;
}




static void
__mark_region( guint x1, guint x2, SChannelPresentation* Ch, gchar value)
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
		Ai (Ch->track, gchar, s) = value;

	Ch->new_marks = TRUE;
	__calculate_dirty_percent( Ch);

	gtk_widget_queue_draw( Ch->da_page);
}






gboolean
daScoringFacPageView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;

	if ( __unfazer_sel_state == UNF_SEL_CALIBRATE && Ch == __unfazer_affected_channel ) {
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( __unfazer_factor > .01 )
				__unfazer_factor /= 1.2;
		    break;
		case GDK_SCROLL_UP:
			if ( __unfazer_factor < 2. )
				__unfazer_factor *= 1.2;
		    break;
		default:
		    break;
		}
		gtk_widget_queue_draw( __unfazer_affected_channel->da_page);
		gtk_widget_queue_draw( __unfazer_offending_channel->da_page);
		return TRUE;
	}

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( Ch->display_scale < 250 )
			Ch->display_scale *= 1.5;
	    break;
	case GDK_SCROLL_UP:
		if ( Ch->display_scale > .001 )
			Ch->display_scale /= 1.5;
	    break;
	default:
	    break;
	}

	if ( event->state & GDK_CONTROL_MASK ) {
		for ( guint h = 0; h < __channels->len; ++h )
			Ai (__channels, SChannelPresentation, h).display_scale =
				Ch->display_scale;
		gtk_widget_queue_draw( cScoringFacPageViews);
	} else
		gtk_widget_queue_draw( wid);

	return TRUE;
}










// -------------------- Profile

gboolean
daScoringFacProfileView_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	SChannelPresentation *Ch = (SChannelPresentation*) userdata;
	if ( !Ch->visible || !gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) )
		return TRUE;

	static GArray *lines = NULL;
	static PangoLayout *layout = NULL;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);
	if ( !lines ) {
		lines = g_array_sized_new( FALSE, FALSE, wd, sizeof(GdkPoint));
		layout = gtk_widget_create_pango_layout( wid, "");
	}
	if ( lines->len < wd*2 )
		g_array_set_size( lines, wd*2);

#define L(x) Ai (lines, GdkPoint, x)
	guint i, m;
	for ( i = 0, m = 0; i < wd; ++i, m += 2 ) {
		guint i_real = (gfloat)i / wd * Ch->power->len;
		L(m).x = i;
		L(m).y = - (Ai (Ch->power, double, i_real)
			    * AghPPuV2)
			+ ht;
		L(m+1).x = i+1;
		L(m+1).y = ht;
	}
#undef L
	gdk_draw_lines( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			(GdkPoint*)lines->data, m);

      // cursor
	gdk_draw_rectangle( wid->window, __gc_score_cursor,
			    FALSE,
			    (float) __cur_page_app / P2AP (__total_pages) * wd,  0,
			    ceil( (gfloat)  1 / P2AP (__total_pages) * wd), ht);


	gdk_draw_line( wid->window, __gc_power,
		       10, 10,
		       10, 10 + AghPPuV2/10);
	snprintf( __buf__, 50, "<b><small>0.1 \302\265V\302\262</small></b>");
	pango_layout_set_markup( layout, __buf__, -1);
	gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			 15, 15,
			 layout);

	snprintf( __buf__, 23, "<b>%g - %g</b> Hz", Ch->from, Ch->upto);
	pango_layout_set_markup( layout, __buf__, -1);
	gdk_draw_layout( wid->window, wid->style->fg_gc[GTK_STATE_NORMAL],
			 wd - 50, 10,
			 layout);

	return TRUE;
}






gboolean
daScoringFacProfileView_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
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
	agh_msmt_get_track_as_garray( Ch->rec_ref,
				      Ch->track);

	gtk_widget_queue_draw( wid);

	return TRUE;
}









gboolean
daScoringFacProfileView_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
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
	case 3:
	{
		guint i_real = (guint)((gdouble)event->x / wd * Ch->power->len);
		AghPPuV2 = ht * 0.75 / Ai (Ch->power, double, i_real);
		gtk_widget_queue_draw( wid);
	}
	    break;
	}

	return TRUE;
}














// -------------------- Hypnogram

gboolean
daScoringFacHypnogram_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer unused)
{
	static GArray *lines = NULL;
	if ( !lines )
		lines = g_array_new( FALSE, FALSE, sizeof(GdkPoint));

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	guint i;
	for ( i = 1; i < 8; ++i )
		gdk_draw_line( wid->window, __gc_score[i],
			       0,   __score_hypn_depth[i],
			       wd,  __score_hypn_depth[i]);


	for ( i = 0; i < __hypnogram->len; ++i ) {
		gchar c;
		if ( (c = Ai (__hypnogram, gchar, i)) != AghScoreCodes[AGH_SCORE_NONE] ) {
			gint y = __score_hypn_depth[ SCOREID(c) ];
			gdk_draw_line( wid->window, __gc_score_line,
				       lroundf( (gfloat) i   /__hypnogram->len * wd), y,
				       lroundf( (gfloat)(i+1)/__hypnogram->len * wd), y);
//		gdk_draw_lines( wid->window, __gc_score_line,
//			       (GdkPoint*)lines->data, lines->len);
		}
	}

	gdk_draw_rectangle( wid->window, __gc_score_cursor,
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





static void
__repaint_score_stats()
{
	snprintf( __buf__, 29, "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);

	snprintf( __buf__, 29, "<small>%3.1f</small> %%", __percent_NREM());
	gtk_label_set_markup( GTK_LABEL (lScoreStatsNREMPercent), __buf__);

	snprintf( __buf__, 29, "<small>%3.1f</small> %%", __percent_REM());
	gtk_label_set_markup( GTK_LABEL (lScoreStatsREMPercent), __buf__);

	snprintf( __buf__, 29, "<small>%3.1f</small> %%", __percent_Wake());
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

	snprintf( __buf__, 19, "of %d", P2AP (__total_pages));
	gtk_label_set_text( GTK_LABEL (lScoringFacTotalPages), __buf__);

	gboolean pagesize_is_right = (APSZ == PSZ);
	gtk_widget_set_sensitive( bScoreClear, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM1, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM2, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM3, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreNREM4, pagesize_is_right);
	gtk_widget_set_sensitive( bScoreREM,   pagesize_is_right);
	gtk_widget_set_sensitive( bScoreWake,  pagesize_is_right);
	gtk_widget_set_sensitive( bScoreMVT,   pagesize_is_right);

	if ( !pagesize_is_right )
		for ( guint h = 0; h < __channels->len; ++h ) {
			SChannelPresentation *Ch = &Ai( __channels, SChannelPresentation, h);
			if ( Ch->visible ) {
				g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
				gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg_score[AGH_SCORE_NONE]);
				gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg_score[AGH_SCORE_NONE]);
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

	for ( guint h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai( __channels, SChannelPresentation, h);
		if ( Ch->visible && gtk_expander_get_expanded( GTK_EXPANDER (Ch->expander)) )
			if ( AghPagesizeItem == __pagesize_item && Ch->da_page ) {
				if ( __unfazer_sel_state == 0 || __unfazer_affected_channel != Ch ) {
					g_signal_handler_block( Ch->da_page, Ch->expose_handler_id);
					gtk_widget_modify_fg( Ch->da_page, GTK_STATE_NORMAL, &__fg_score[__cur_stage]);
					gtk_widget_modify_bg( Ch->da_page, GTK_STATE_NORMAL, &__bg_score[__cur_stage]);
					g_signal_handler_unblock( Ch->da_page, Ch->expose_handler_id);
				}
				gtk_widget_queue_draw( Ch->da_page);
				gtk_widget_queue_draw( Ch->da_powercourse);
			}
	}
	snprintf( __buf__, 29, "<b><big>%s</big></b>", __score_names[ __cur_stage ]);
	gtk_label_set_markup( GTK_LABEL (lScoringFacCurrentStage), __buf__);

	snprintf( __buf__, 39, "%2d:%02d:%02d", __cur_pos_hr, __cur_pos_min, __cur_pos_sec);
	gtk_label_set_text( GTK_LABEL (lScoringFacCurrentPos), __buf__);

	gtk_widget_queue_draw( daScoringFacHypnogram);
}












// ------ menu callbacks

void
iSFPowerExportRange_activate_cb()
{
	GString *fname_buf = g_string_sized_new(120),
		*messages = g_string_sized_new(200);
	g_string_assign( messages, "Wrote the following files:\n");
	for ( guint h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
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
	for ( guint h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
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

	for ( guint h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		if ( Ch->track && Ch->new_marks ) {
//			printf( "%s has changed artifacts:\n%s\n", Ch->name, Ch->track->data);
			agh_msmt_put_track_as_garray( Ch->rec_ref,
						      Ch->track);
			agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     (double*)Ch->power->data);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);

			Ch->new_marks = FALSE;
		}
	}

      // update power profile in measurements view
	__paint_one_subject_episodes();

	set_cursor_busy( FALSE, wScoringFacility);
}



void
iSFArtifactsClear_activate_cb()
{
	if ( pop_question( GTK_WINDOW (wScoringFacility),
			   "All marked artifacts will be lost in all channels. Continue?") != GTK_RESPONSE_YES )
		return;

	guint h;
	for ( h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		if ( Ch->power )
			memset( Ch->track->data, (int)' ', Ch->track->len);
	}

	set_cursor_busy( TRUE, wScoringFacility);

	for ( h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		if ( Ch->power ) {
			agh_msmt_put_track_as_garray( Ch->rec_ref,
						      Ch->track);
			agh_msmt_get_power_course_in_range_as_double_direct( Ch->rec_ref,
									     Ch->from, Ch->upto,
									     (double*)Ch->power->data);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);
		}
	}

      // update power profile in measurements view
	__paint_one_subject_episodes();

	set_cursor_busy( FALSE, wScoringFacility);
}


void
iSFArtifactsMarkMVT_activate_cb()
{
}


void
iSFArtifactsUnfazer_activate_cb()
{
	__unfazer_sel_state = UNF_SEL_CHANNEL;

	g_signal_handler_block( __unfazer_affected_channel->da_page, __unfazer_affected_channel->expose_handler_id);
	gtk_widget_modify_fg( __unfazer_affected_channel->da_page, GTK_STATE_NORMAL, &__fg_score[AGH_SCORE__UNFAZER]);
	gtk_widget_modify_bg( __unfazer_affected_channel->da_page, GTK_STATE_NORMAL, &__bg_score[AGH_SCORE__UNFAZER]);
	g_signal_handler_unblock( __unfazer_affected_channel->da_page, __unfazer_affected_channel->expose_handler_id);

	gtk_widget_queue_draw( __unfazer_affected_channel->da_page);
}





void
bSFScore_clicked_cb()
{
	set_cursor_busy( TRUE, wScoringFacility);
	for ( guint h = 0; h < __channels->len; ++h ) {
		SChannelPresentation *Ch = &Ai (__channels, SChannelPresentation, h);
		if ( Ch->track && Ch->new_marks ) {
			agh_msmt_put_track_as_garray( Ch->rec_ref,
						      Ch->track);
			gtk_widget_queue_draw( Ch->da_page);
			gtk_widget_queue_draw( Ch->da_powercourse);
		}
	}
	set_cursor_busy( FALSE, wScoringFacility);

	agh_edf_put_scores_as_garray( __source_ref,
				      __hypnogram);
      // update power profile in measurements view
	__paint_one_subject_episodes();

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

	snprintf( __buf__, 29, "<b>%3.1f</b> %% scored", __percent_scored());
	gtk_label_set_markup( GTK_LABEL (lScoringFacPercentScored), __buf__);
}



// EOF
