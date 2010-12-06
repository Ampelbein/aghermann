// ;-*-C-*- *  Time-stamp: "2010-12-05 16:03:14 hmmr"
/*
 *       File name:  ui/measurements.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <glade/glade.h>
#include "../libagh/iface.h"
#include "../libagh/iface-glib.h"
#include "misc.h"
#include "ui.h"


GtkWidget
	*wEDFFileDetails,
	*lMsmtInfo,
	*eMsmtChannel,
	*eMsmtSession,
	*cMeasurements,
	*lEDFFileDetailsReport,
	*eMsmtPSDFreqFrom,
	*eMsmtPSDFreqWidth,
	*bMsmtDetails,

	*bColourPowerMT,
	*bColourTicksMT,
	*bColourLabelsMT;

static GtkTextBuffer
	*textbuf2;


static const gchar* const __bg_rgb[] = {
	"black",
	"white",
	"white",
	"white",
	"white",
};

static const gchar* const __fg_rgb[] = {
	"#FF4500", // OrangeRed1
	"grey", // TICKS: spring green
	"#CAFF70",
	"#2F4F4F", // dark slate grey
	"#191970", // midnight blue
};

static gshort __line_widths[] = {
	1, 1,
	1, 1,
	1,
};

GdkColor
	__fg0__[cTOTAL_MT],
	__bg0__[cTOTAL_MT];

static GdkGC
	*__gc__[cTOTAL_MT];

static GdkColormap *__cmap;



static void
change_fg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( __cmap, &__fg0__[c], 1);
	gtk_color_button_get_color( cb, &__fg0__[c]);
	gdk_colormap_alloc_color( __cmap, &__fg0__[c], FALSE, TRUE);
	GdkGCValues xx;
	xx.foreground = __fg0__[c];
	__gc__[c] = gtk_gc_get( agh_visual->depth, __cmap,
				&xx, GDK_GC_FOREGROUND);
}
static void
change_bg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( __cmap, &__bg0__[c], 1);
	gtk_color_button_get_color( cb, &__bg0__[c]);
	gdk_colormap_alloc_color( __cmap, &__bg0__[c], FALSE, TRUE);
	GdkGCValues xx;
	xx.background = __bg0__[c];
	__gc__[c] = gtk_gc_get( agh_visual->depth, __cmap,
				&xx, GDK_GC_BACKGROUND);
}




void eMsmtSession_changed_cb(void);
void eMsmtChannel_changed_cb(void);
gulong	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id;

gint
agh_ui_construct_Measurements( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- cMeasurements
	if ( !(cMeasurements = glade_xml_get_widget( xml, "cMeasurements")) ||
	     !(lMsmtInfo = glade_xml_get_widget( xml, "lMsmtInfo")) )
		return -1;


     // ------------- eMsmtSession
	if ( !(eMsmtSession = glade_xml_get_widget( xml, "eMsmtSession")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtSession),
				 GTK_TREE_MODEL (agh_mSessions));
	eMsmtSession_changed_cb_handler_id =
		g_signal_connect( eMsmtSession, "changed", eMsmtSession_changed_cb, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eMsmtSession), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eMsmtSession), renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtChannel
	if ( !(eMsmtChannel = glade_xml_get_widget( xml, "eMsmtChannel")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtChannel),
				 GTK_TREE_MODEL (agh_mEEGChannels));
	eMsmtChannel_changed_cb_handler_id =
		g_signal_connect( eMsmtChannel, "changed", eMsmtChannel_changed_cb, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (eMsmtChannel), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (eMsmtChannel), renderer,
					"text", 0,
					NULL);

     // ------------- eMsmtPSDFreq
	if ( !(eMsmtPSDFreqFrom = glade_xml_get_widget( xml, "eMsmtPSDFreqFrom")) ||
	     !(eMsmtPSDFreqWidth = glade_xml_get_widget( xml, "eMsmtPSDFreqWidth")) )
		return -1;

     // ------------- wEDFFileDetails
	if ( !(wEDFFileDetails = glade_xml_get_widget( xml, "wEDFFileDetails")) ||
	     !(lEDFFileDetailsReport = glade_xml_get_widget( xml, "lEDFFileDetailsReport")) )
		return -1;

	g_object_set( lEDFFileDetailsReport,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);
	textbuf2 = gtk_text_view_get_buffer( GTK_TEXT_VIEW (lEDFFileDetailsReport));


      // --- assorted static objects
	GdkGCValues xx;
	__cmap = gtk_widget_get_colormap( cMeasurements);
	for ( gushort i = 0; i < cTOTAL_MT; ++i ) {
		gdk_color_parse( __fg_rgb[i], &__fg0__[i]),
			gdk_colormap_alloc_color( __cmap, &__fg0__[i], FALSE, TRUE);
		gdk_color_parse( __bg_rgb[i], &__bg0__[i]),
			gdk_colormap_alloc_color( __cmap, &__bg0__[i], FALSE, TRUE);

		xx.foreground = __fg0__[i], xx.background = __bg0__[i];  // bg <-> fg // why?
		xx.line_width = __line_widths[i],
			xx.line_style = GDK_LINE_SOLID, xx.cap_style = GDK_CAP_ROUND;

		__gc__[i] = gtk_gc_get( agh_visual->depth, __cmap,
					&xx, GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_LINE_WIDTH | GDK_GC_LINE_STYLE);
	}

      // ------ colours
	if ( !(bColourPowerMT	= glade_xml_get_widget( xml, "bColourPowerMT")) ||
	     !(bColourTicksMT	= glade_xml_get_widget( xml, "bColourTicksMT")) ||
	     !(bColourLabelsMT	= glade_xml_get_widget( xml, "bColourLabelsMT")) )
		return -1;

	return 0;
}


void
agh_ui_destruct_Measurements()
{
/*
	gtk_gc_release(__gc_power);
	gtk_gc_release(__gc_ticks);
	gtk_gc_release(__gc_labels);
	gtk_gc_release(__gc_hours);
*/
}




void
eMsmtSession_changed_cb()
{
	gint oldval = AghDi;
	AghDi = gtk_combo_box_get_active( GTK_COMBO_BOX (eMsmtSession));

	if ( oldval != AghDi ) {
		gtk_combo_box_set_active( GTK_COMBO_BOX (eSimulationsSession), AghDi);
		agh_populate_cMeasurements();
	}
//	gtk_widget_queue_draw( cMeasurements);
}

void
eMsmtChannel_changed_cb()
{
	gint oldval = AghTi;
	AghTi = gtk_combo_box_get_active( GTK_COMBO_BOX (eMsmtChannel));

	if ( oldval != AghTi ) {
		gtk_combo_box_set_active( GTK_COMBO_BOX (eSimulationsChannel), AghTi);
		agh_populate_cMeasurements();
	}
//	gtk_widget_queue_draw( cMeasurements);
}




void
eMsmtPSDFreqFrom_value_changed_cb()
{
	AghOperatingRangeFrom = gtk_spin_button_get_value( GTK_SPIN_BUTTON (eMsmtPSDFreqFrom));
	AghOperatingRangeUpto = AghOperatingRangeFrom + gtk_spin_button_get_value( GTK_SPIN_BUTTON (eMsmtPSDFreqWidth));
	agh_populate_cMeasurements();
}

void
eMsmtPSDFreqWidth_value_changed_cb()
{
	AghOperatingRangeUpto = AghOperatingRangeFrom + gtk_spin_button_get_value( GTK_SPIN_BUTTON (eMsmtPSDFreqWidth));
	agh_populate_cMeasurements();
}

void
cMsmtPSDFreq_map_cb()
{
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eMsmtPSDFreqWidth), AghOperatingRangeUpto - AghOperatingRangeFrom);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (eMsmtPSDFreqFrom), AghOperatingRangeFrom);
	g_signal_connect( eMsmtPSDFreqFrom, "value-changed", G_CALLBACK (eMsmtPSDFreqFrom_value_changed_cb), NULL);
	g_signal_connect( eMsmtPSDFreqWidth, "value-changed", G_CALLBACK (eMsmtPSDFreqWidth_value_changed_cb), NULL);
}






guint	AghTimelinePPH = 20;
gboolean
	AghMsmtViewDrawPowerSolid = TRUE,
	AghMsmtViewDrawDetails = TRUE;

static time_t
	AghTimelineStart,
	AghTimelineEnd;

static guint
	__tl_left_margin = 50,
	__tl_right_margin = 20,
	__timeline_length;

static inline guint
T2P( time_t t)
{
	return difftime( t, AghTimelineStart) / 3600 * AghTimelinePPH;
}

static inline time_t
P2T( guint p)
{
	return (double)p * 3600 / AghTimelinePPH + AghTimelineStart;
}



typedef struct {
	struct SSubject
		*subject;
	GtkWidget
		*da;
	GArray	*power;
} SSubjectPresentation;

typedef struct {
	struct SGroup
		*group;
	GArray	*subjects;
	gboolean
		visible;
	GtkWidget
		*expander,
		*vbox;
} SGroupPresentation;

static void
free_group_presentation( SGroupPresentation* g)
{
	for ( guint j = 0; j < g->subjects->len; ++j ) {
		SSubjectPresentation *J = &Ai (g->subjects, SSubjectPresentation, j);
		// agh_SSubject_destruct( &J->subject);
		g_array_free( J->power, TRUE);
		// gtk_widget_destroy( J->da);  // this is done in gtk_container_foreach( cMeasurements, gtk_widget_destroy)
	}
	g_array_free( g->subjects, TRUE);
}

static GArray	*GG;








#define JTLDA_HEIGHT 60

gboolean xExpander_expose_event_cb( GtkWidget* un, GdkEventExpose* us, gpointer ed)
{
	return TRUE;
}

gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSubjectTimeline_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);




static SSubjectPresentation *J_paintable;

static void
__collect_one_subject_episodes()
{
	struct SSubject* _j = J_paintable->subject;
	static GArray *episode_signal = NULL;
	if ( episode_signal == NULL )
		episode_signal = g_array_new( FALSE, FALSE, sizeof(double));

	time_t	j_timeline_start = _j->sessions[AghDi].episodes[0].start_rel;

	for ( guint e = 0; e < _j->sessions[AghDi].n_episodes; ++e ) {
		// ...( J->subject->sessions[AghDi].episodes[e].; // rather use agh_msmt_find_by_jdeh than look up the matching channel
		TRecRef recref = agh_msmt_find_by_jdeh( _j->name, AghD, _j->sessions[AghDi].episodes[e].name, AghT);
		if ( recref == NULL ) { // this can happen, too
			continue;
		}
		agh_msmt_get_power_course_in_range_as_double_garray( recref,
								     AghOperatingRangeFrom, AghOperatingRangeUpto,
								     episode_signal);

		time_t	j_e_start = _j->sessions[AghDi].episodes[e].start_rel;
		//j_e_end = _j->sessions[AghDi].episodes[e].end_rel;
		// put power on the global timeline

		guint	pagesize = agh_msmt_get_pagesize( _j->sessions[AghDi].episodes[0].recordings[0]);

		memcpy( &Ai (J_paintable->power, double, (j_e_start - j_timeline_start) / pagesize),
			&Ai (episode_signal, double, 0),
			episode_signal->len * sizeof(double));
	}
}

void
agh_populate_cMeasurements()
{
	if ( AghGs == 0 )
		return;

      // get current expdesign snapshot
	agh_expdesign_snapshot( &agh_cc);

      // prepare own module-private structures for the first run
	if ( !GG )
		GG = g_array_new( FALSE, FALSE, sizeof(SGroupPresentation));

	gtk_container_foreach( GTK_CONTAINER (cMeasurements),
			       (GtkCallback) gtk_widget_destroy,
			       NULL);
	for ( guint g = 0; g < GG->len; ++g )
		free_group_presentation( &Ai (GG, SGroupPresentation, g));
	g_array_set_size( GG, agh_cc.n_groups);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

      // first pass: determine common timeline and collect episodes' power
	for ( guint g = 0; g < agh_cc.n_groups; ++g ) {
		SGroupPresentation* G = &Ai (GG, SGroupPresentation, g);
		G->group = &agh_cc.groups[g];
		G->subjects = g_array_new( FALSE, FALSE, sizeof(SSubjectPresentation));
		g_array_set_size( G->subjects, G->group->n_subjects);

		for ( guint j = 0; j < G->group->n_subjects; ++j ) {
			SSubjectPresentation* J = &Ai (G->subjects, SSubjectPresentation, j);
			J->subject = &G->group->subjects[j];
			struct SSubject* _j = J->subject;

			J->power = g_array_new( FALSE, TRUE, sizeof(double));

			if ( AghDi < _j->n_sessions ) {
				guint	j_n_episodes = _j->sessions[AghDi].n_episodes;
				time_t	j_timeline_start = _j->sessions[AghDi].episodes[0].start_rel,
					j_timeline_end   = _j->sessions[AghDi].episodes[ j_n_episodes-1 ].end_rel;

			      // determine timeline global start and end
				if ( earliest_start == (time_t)-1) {
					earliest_start = j_timeline_start;
					latest_end = j_timeline_end;
				}
				if ( earliest_start > j_timeline_start )
					earliest_start = j_timeline_start;
				if ( latest_end < j_timeline_end )
					latest_end = j_timeline_end;

				// allocate subject timeline
				guint	pagesize = agh_msmt_get_pagesize( _j->sessions[AghDi].episodes[0].recordings[0]),
					total_pages = (j_timeline_end - j_timeline_start) / pagesize;

				g_array_set_size( J->power, total_pages);

				J_paintable = J;
				__collect_one_subject_episodes();
			}
		}
	}

	AghTimelineStart = earliest_start;
	AghTimelineEnd   = latest_end;
	__timeline_length = (double)(AghTimelineEnd - AghTimelineStart) / 3600 * AghTimelinePPH;

//	fprintf( stderr, "agh_populate_cMeasurements(): common timeline:\n");
//	fputs (asctime (localtime(&earliest_start)), stderr);
//	fputs (asctime (localtime(&latest_end)), stderr);

      // walk again, set timeline drawing area length
	for ( guint g = 0; g < GG->len; ++g ) {
		SGroupPresentation* G = &Ai (GG, SGroupPresentation, g);

		GString *episodes_ext = g_string_new("");
		for ( gushort e = 0; e < AghEs; ++e ) {
			struct SEpisodeTimes e_times;
			agh_group_avg_episode_times( AghGG[g], AghD, AghEE[e], &e_times);
			g_string_append_printf( episodes_ext,
						"       <i>%s</i> %02d:%02d:%02d ~ %02d:%02d:%02d",
						AghEE[e],
						e_times.start_hour % 24,
						e_times.start_min,
						e_times.start_sec,
						e_times.end_hour % 24,
						e_times.end_min,
						e_times.end_sec);
		}
		snprintf_buf( "<b>%s</b> (%u) %s", G->group->name, G->subjects->len, episodes_ext->str);
		g_string_free( episodes_ext, TRUE);
		G->expander = gtk_expander_new( __buf__);
		gtk_expander_set_use_markup( GTK_EXPANDER (G->expander), TRUE);
		g_object_set( G_OBJECT (G->expander),
			      "visible", TRUE,
			      "expanded", TRUE,
			      "height-request", -1,
//			      "fill", FALSE,
//			      "expand", FALSE,
			      NULL);
		gtk_box_pack_start( GTK_BOX (cMeasurements),
				    G->expander, TRUE, TRUE, 3);
		gtk_container_add( GTK_CONTAINER (G->expander),
				   G->vbox = gtk_vbox_new( TRUE, 1));
		g_object_set( G_OBJECT (G->vbox),
//			      "fill", FALSE,
//			      "expand", FALSE,
			      "height-request", -1,
			      NULL);
		guint j = 0;
		while ( j < G->subjects->len ) {
			SSubjectPresentation* J = &Ai (G->subjects, SSubjectPresentation, j);
			gtk_box_pack_start( GTK_BOX (G->vbox),
					    J->da = gtk_drawing_area_new(), TRUE, TRUE, 2);

			g_signal_connect_after( J->da, "expose-event",
						G_CALLBACK (daSubjectTimeline_expose_event_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "scroll-event",
						G_CALLBACK (daSubjectTimeline_scroll_event_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "button-press-event",
						G_CALLBACK (daSubjectTimeline_button_press_event_cb),
						(gpointer)J);
			g_object_set( G_OBJECT (J->da),
				      "app-paintable", TRUE,
				      "double-buffered", TRUE,
//				      "fill", TRUE,
//				      "expand", FALSE,
				      "height-request", JTLDA_HEIGHT,
				      "width-request", __timeline_length + __tl_left_margin + __tl_right_margin,
				      NULL);

			gtk_widget_add_events( J->da,
					       (GdkEventMask) GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

//			gtk_widget_modify_fg( J->da, GTK_STATE_NORMAL, &__fg_power);
//			gtk_widget_modify_bg( J->da, GTK_STATE_NORMAL, &__bg);

			++j;
		}
	}

	snprintf_buf( "<b><small>page: %s  bin: %g Hz  %s</small></b>",
		      // agh_fft_get_pagesize(), // please the user
		      agh_fft_pagesize_values_s[AghFFTPageSizeCurrent],
		      agh_fft_get_binsize(),
		      agh_fft_window_types_s[agh_fft_get_window_type()]);
	gtk_label_set_markup( GTK_LABEL (lMsmtInfo), __buf__);

	gtk_widget_show_all( cMeasurements);
}






#define M_PI 3.14159


gboolean
daSubjectTimeline_expose_event_cb( GtkWidget *container, GdkEventExpose *event, gpointer userdata)
{
	if ( AghGs == 0 )
		return TRUE;

	static GdkGC *circadian_gc = NULL;
	if ( !circadian_gc )
		circadian_gc = gdk_gc_new( cMeasurements->window);

	__ensure_enough_lines( __timeline_length * 2);  // in case AghMsmtViewDrawPowerSolid is TRUE

	GdkColor circadian_color = { 0, 65535, 65535, 65535 };

	SSubjectPresentation* J = (SSubjectPresentation*)userdata;
	struct SSubject *_j = J->subject;

      // draw subject
	g_string_printf( __ss__, "<big>%s</big>", _j->name);
	pango_layout_set_markup( __pp__, __ss__->str, -1);
	gdk_draw_layout( J->da->window, __gc__[cJINFO],
			 3, 5,
			 __pp__);

      // draw day and night
	for ( guint c = 0; c < __timeline_length; ++c ) {
		time_t t = P2T(c);
		struct tm clock_time;
		memcpy( &clock_time, localtime(&t), sizeof(clock_time));
		float clock_h = clock_time.tm_hour + (float)clock_time.tm_min/60 + (float)clock_time.tm_sec/3600;

		float day_fraction = (1 + sinf( (clock_h - 5)/ 24 * 2.*M_PI))/2;
		circadian_color.red = circadian_color.green = day_fraction*65535/1.6;
		gdk_gc_set_rgb_fg_color( circadian_gc, &circadian_color);
		gdk_draw_line( J->da->window, circadian_gc,
			       c + __tl_left_margin, 0,
			       c + __tl_left_margin, JTLDA_HEIGHT - 12);
	}

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &AghTimelineStart), sizeof(struct tm));
	// determine the latest full hour before AghTimelineStart
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

	gsize	j_n_episodes = _j->sessions[AghDi].n_episodes;
	gulong	j_tl_pixel_start = difftime( _j->sessions[AghDi].episodes[0].start_rel, AghTimelineStart) / 3600 * AghTimelinePPH,
		j_tl_pixel_end   = difftime( _j->sessions[AghDi].episodes[j_n_episodes-1].end_rel, AghTimelineStart) / 3600 * AghTimelinePPH,
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;


	// power
#define X(x) Ai (J->power, double, x)
	guint i, k, m;
	if ( AghMsmtViewDrawPowerSolid ) {
		for ( i = j_tl_pixel_start + __tl_left_margin, k = 0, m = 0;
		      k < j_tl_pixels; ++i, ++k, m += 2 ) {
			LL(m).x = i;
			LL(m).y = -X( (gsize)((double)k / j_tl_pixels * J->power->len) )
				* AghPPuV2
				+ JTLDA_HEIGHT-12;
			LL(m+1).x = i+1;
			LL(m+1).y = AghPPuV2 + JTLDA_HEIGHT-12;
		}
		gdk_draw_lines( J->da->window, __gc__[cPOWER_MT],
				(GdkPoint*)__ll__->data, m);
	} else {
		for ( i = j_tl_pixel_start + __tl_left_margin, k = 0;
		      k < j_tl_pixels; ++i, ++k ) {
			LL(k).x = i;
			LL(k).y = -X( (gsize)((double)k / j_tl_pixels * J->power->len) )
				* AghPPuV2
				+ JTLDA_HEIGHT-12;
		}
		gdk_draw_lines( J->da->window, __gc__[cPOWER_MT],
				(GdkPoint*)__ll__->data, k);
	}
#undef X


	// boundaries
	for ( guint e = 0; e < _j->sessions[AghDi].n_episodes; ++e ) {
		struct SEpisode *_e = &_j->sessions[AghDi].episodes[e];
		guint	e_pix_start = T2P( _e->start_rel),
			e_pix_end   = T2P( _e->end_rel);
//				printf( "%s: %d: %d-%d\n", _j->name, e, e_pix_start, e_pix_end);
		gdk_draw_rectangle( J->da->window, __gc__[cBOUNDS], TRUE,
				    __tl_left_margin + e_pix_start, JTLDA_HEIGHT-12,
				    e_pix_end-e_pix_start, 12);

		if ( AghMsmtViewDrawDetails ) {
			// episode start timestamp
			strftime( __buf__, 79, "%F %T",
				  localtime( &_j->sessions[AghDi].episodes[e].start));
			g_string_printf( __ss__, "<small><b>%s\n%s</b></small>",
					 __buf__, _j->sessions[AghDi].episodes[e].name);
			pango_layout_set_markup( __pp__,
						 __ss__->str,
						 -1);
			gdk_draw_layout( J->da->window, __gc__[cLABELS_MT],
					 __tl_left_margin + e_pix_start + 2, 2,
					 __pp__);
		} else {
			float pc_scored, pc_nrem, pc_rem, pc_wake;
			pc_scored =
				agh_edf_get_scored_stages_breakdown(
					agh_msmt_get_source( _j->sessions[AghDi].episodes[e].recordings[0]),
					&pc_nrem, &pc_rem, &pc_wake);
			g_string_printf( __ss__,
					 "<small>"
					 "N:%4.1f%% R:%4.1f%% W:%4.1f%%"
					 "</small>\n"
					 "<span foreground=\"%s\" weight=\"bold\">%4.1f%%</span>",
					 100 * pc_nrem, 100 * pc_rem, 100 * pc_wake,
					 pc_scored > agh_modelrun_get_req_percent_scored() ? "white" : "yellow",
					 100 * pc_scored);
			pango_layout_set_markup( __pp__,
						 __ss__->str,
						 -1);
//			PangoRectangle extents;
//			pango_layout_get_pixel_extents( __pp__, &extents, NULL);
			gdk_draw_layout( J->da->window, __gc__[cLABELS_MT],
					 __tl_left_margin + e_pix_start /*- extents.width */, 2,
					 __pp__);
		}
	}

      // draw hours
	for ( time_t t = tl_start_fixed; t < AghTimelineEnd + 3600; t += 3600 ) {
		guint x = T2P(t);
		guint clock_h = localtime(&t)->tm_hour;
		if ( clock_h % 6 == 0 ) {
			gdk_draw_line( J->da->window, __gc__[cTICKS_MT],
				       __tl_left_margin + x, ( clock_h % 24 == 0 ) ? 0 : (JTLDA_HEIGHT - 16),
				       __tl_left_margin + x, JTLDA_HEIGHT - 10);
			g_string_printf( __ss__, "<small><b>%d</b></small>", clock_h);
			pango_layout_set_markup( __pp__, __ss__->str, -1);
			PangoRectangle extents;
			pango_layout_get_pixel_extents( __pp__, &extents, NULL);
			gdk_draw_layout( J->da->window, __gc__[cTICKS_MT],
					 __tl_left_margin + x - extents.width/2, JTLDA_HEIGHT-11,
					 __pp__);
		} else
			gdk_draw_line( J->da->window, __gc__[cTICKS_MT],
				       __tl_left_margin + x, JTLDA_HEIGHT - 14,
				       __tl_left_margin + x, JTLDA_HEIGHT - 7);
	}

	return TRUE;
}








static gint
get_episode_from_timeline_click( struct SSubject* _j, guint along, const char **which_e)
{
	along -= __tl_left_margin;
	for ( gint e = 0; e < _j->sessions[AghDi].n_episodes; ++e ) {
		struct SEpisode *_e = &_j->sessions[AghDi].episodes[e];
		if ( along >= T2P(_e->start_rel) && along <= T2P(_e->end_rel) ) {
			(*which_e) = _e->name;
			return e;
		}
	}
	return -1;
}

gboolean
daSubjectTimeline_button_press_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer userdata)
{
	static GString *window_title = NULL;
	if ( !window_title )
		window_title = g_string_sized_new( 120);

	SSubjectPresentation *J = (SSubjectPresentation*)userdata;
	struct SSubject *_j = J->subject;

	gint wd, ht;
	gdk_drawable_get_size( widget->window, &wd, &ht);
	const char *real_episode_name;
	gint clicked_episode = get_episode_from_timeline_click( _j, event->x, &real_episode_name);
	if ( clicked_episode != -1 ) {
		// should some episodes be missing, we make sure the correct one gets identified by number
		guint i = 0;
		while ( i < AghEs && strcmp( AghEE[i], real_episode_name) )
			++i;
		AghEi = i;
	}
	AghJ = _j;

	switch ( event->button ) {
	case 1:
		if ( clicked_episode != -1 && agh_prepare_scoring_facility( _j, AghD, AghE) ) {
			g_string_printf( window_title, "Scoring: %s\342\200\231s %s",
					 _j->name, AghE);
			gtk_window_set_title( GTK_WINDOW (wScoringFacility),
					      window_title->str);
			gtk_window_set_default_size( GTK_WINDOW (wScoringFacility),
						     gdk_screen_get_width( gdk_screen_get_default()) * .9,
						     -1);
			gtk_widget_show_all( wScoringFacility);

			J_paintable = J;
			__collect_one_subject_episodes();
			gtk_widget_queue_draw( J->da);
		}
	    break;
	case 2:
	case 3:
		if ( clicked_episode != -1 ) {
			TEDFRef edfref;
			agh_edf_find_by_jdeh( _j->name, AghD, AghEE[clicked_episode], AghT,
					      &edfref);
			char *tmp;
			const struct SEDFFile *edfstruct = agh_edf_get_info_from_sourceref( edfref, &tmp);
			gtk_text_buffer_set_text( textbuf2, tmp, -1);
			free( tmp);
			g_string_printf( window_title, "%s header",
					 edfstruct->filename);
			gtk_window_set_title( GTK_WINDOW (wEDFFileDetails),
					      window_title->str);
			gtk_widget_show_all( wEDFFileDetails);
		}
	    break;
	}

	return TRUE;
}




gboolean
daSubjectTimeline_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		if ( event->state & GDK_CONTROL_MASK ) {
			AghPPuV2 /= 1.3;
			gtk_widget_queue_draw( cMeasurements);
			return TRUE;
		}
	    break;
	case GDK_SCROLL_UP:
		if ( event->state & GDK_CONTROL_MASK ) {
			AghPPuV2 *= 1.3;
			gtk_widget_queue_draw( cMeasurements);
			return TRUE;
		}
	    break;
	default:
	    break;
	}

	return FALSE;
}








void
bViewMsmtDetails_toggled_cb( GtkToggleButton *item,
			     gpointer         user_data)
{
	gint is_on = gtk_toggle_button_get_active( item);
//	AghMsmtViewDrawPowerSolid = !is_on;
	AghMsmtViewDrawDetails    =  is_on;

	gtk_widget_queue_draw( cMeasurements);
}

/*
static guint __single_channel_wScoringFacility_height = 0;

gboolean
wScoringFacility_configure_event_cb( GtkWidget *widget,
				     GdkEventConfigure *event,
				     gpointer unused)
{
	if ( __single_channel_wScoringFacility_height == 0 )
		__single_channel_wScoringFacility_height = event->height;
	return FALSE;
}

*/




// -------- colours

void
bColourPowerMT_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_fg_colour( cPOWER_MT, widget);
}

void
bColourTicksMT_color_set_cb( GtkColorButton *widget,
			     gpointer        user_data)
{
	change_fg_colour( cTICKS_MT, widget);
}

void
bColourLabelsMT_color_set_cb( GtkColorButton *widget,
			      gpointer        user_data)
{
	change_fg_colour( cLABELS_MT, widget);
}


// EOF
