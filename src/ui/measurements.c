// ;-*-C-*- *  Time-stamp: "2011-02-26 00:37:21 hmmr"
/*
 *       File name:  ui/measurements.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  measurements overview view
 *
 *         License:  GPL
 */


#include <stdio.h>
//#include <math.h>
#include <string.h>
#include <time.h>
#include <glade/glade.h>
#include <cairo.h>
#include <cairo-svg.h>
#include "../libagh/iface.h"
#include "../libagh/iface-glib.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"


float	AghPPuV2 = 1e-5;
static guint
	AghTimelinePPH = 20;


static GtkWidget
	*lMsmtHint;

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

	*wEdfImport,
	*eEdfImportGroup,
	*eEdfImportSession,
	*eEdfImportEpisode,
	*lEdfImportSubject,
	*lEdfImportCaption,
	*lEdfImportFileInfo,
	*bEdfImportAdmit,
	*bEdfImportScoreSeparately,
	*bEdfImportAttachCopy,
	*bEdfImportAttachMove,
//	*bEdfImportAttachLink,

	*bColourPowerMT,
	*bColourTicksMT,
	*bColourLabelsMT;

static GtkTextBuffer
	*textbuf2,
	*textbuf3;

#define AGH_TIP_GENERAL 0
static const char*
	__tooltips[] = {
"<b>Subject timeline:</b>\n"
"	Ctrl+Wheel:	change scale;\n"
"	Click1:		view/score episode;\n"
"	Click3:		show edf file info;\n"
"	Alt+Click3:	save timeline as svg.",
};


GdkColor
	__fg0__[cTOTAL_MT],
	__bg0__[cTOTAL_MT];



static void
change_fg_colour( guint c, GtkColorButton *cb)
{
	gdk_colormap_free_colors( agh_cmap, &__fg0__[c], 1);
	gtk_color_button_get_color( cb, &__fg0__[c]);
	gdk_colormap_alloc_color( agh_cmap, &__fg0__[c], FALSE, TRUE);
}
// static void
// change_bg_colour( guint c, GtkColorButton *cb)
// {
// 	gdk_colormap_free_colors( __cmap, &__bg0__[c], 1);
// 	gtk_color_button_get_color( cb, &__bg0__[c]);
// 	gdk_colormap_alloc_color( __cmap, &__bg0__[c], FALSE, TRUE);
// }


void eMsmtSession_changed_cb();
void eMsmtChannel_changed_cb();
gulong	eMsmtSession_changed_cb_handler_id,
	eMsmtChannel_changed_cb_handler_id;

gboolean
check_gtk_entry_nonempty( GtkWidget *ignored,
			  GdkEventKey *event,
			  gpointer  user_data)
{
	gtk_widget_set_sensitive( bEdfImportAdmit, TRUE);
	gtk_widget_set_sensitive( bEdfImportScoreSeparately, TRUE);

	const gchar *e;
	gchar *ee;

	ee = NULL;
	e = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportGroup))));
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( bEdfImportAdmit, FALSE);
		gtk_widget_set_sensitive( bEdfImportScoreSeparately, FALSE);
	}
	g_free( ee);

	ee = NULL;
	e = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportSession))));
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( bEdfImportAdmit, FALSE);
		gtk_widget_set_sensitive( bEdfImportScoreSeparately, FALSE);
	}
	g_free( ee);

	ee = NULL;
	e = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportEpisode))));
	if ( !e || !*g_strchug( g_strchomp( ee = g_strdup( e))) ) {
		gtk_widget_set_sensitive( bEdfImportAdmit, FALSE);
		gtk_widget_set_sensitive( bEdfImportScoreSeparately, FALSE);
	}
	g_free( ee);

	gtk_widget_queue_draw( bEdfImportAdmit);
	gtk_widget_queue_draw( bEdfImportScoreSeparately);

	return FALSE;
}





int
agh_ui_construct_Measurements( GladeXML *xml)
{
	GtkCellRenderer *renderer;

     // ------------- cMeasurements
	if ( !(cMeasurements = glade_xml_get_widget( xml, "cMeasurements")) ||
	     !(lMsmtHint = glade_xml_get_widget( xml, "lMsmtHint")) ||
	     !(lMsmtInfo = glade_xml_get_widget( xml, "lMsmtInfo")) )
		return -1;

	gtk_drag_dest_set( cMeasurements, GTK_DEST_DEFAULT_ALL,
			   NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets( cMeasurements);


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


      // ------- wEdfImport
	if ( !(wEdfImport	  = glade_xml_get_widget( xml, "wEdfImport")) ||
	     !(eEdfImportGroup	  = glade_xml_get_widget( xml, "eEdfImportGroup")) ||
	     !(eEdfImportSession  = glade_xml_get_widget( xml, "eEdfImportSession")) ||
	     !(eEdfImportEpisode  = glade_xml_get_widget( xml, "eEdfImportEpisode")) ||
	     !(lEdfImportSubject  = glade_xml_get_widget( xml, "lEdfImportSubject")) ||
	     !(lEdfImportCaption  = glade_xml_get_widget( xml, "lEdfImportCaption")) ||
	     !(lEdfImportFileInfo = glade_xml_get_widget( xml, "lEdfImportFileInfo")) ||
	     !(bEdfImportAttachCopy = glade_xml_get_widget( xml, "bEdfImportAttachCopy")) ||
	     !(bEdfImportAttachMove = glade_xml_get_widget( xml, "bEdfImportAttachMove")) ||
	     !(bEdfImportAdmit		 = glade_xml_get_widget( xml, "bEdfImportAdmit")) ||
	     !(bEdfImportScoreSeparately = glade_xml_get_widget( xml, "bEdfImportScoreSeparately")))
		return -1;

	g_object_set( lEdfImportFileInfo,
		      "tabs", pango_tab_array_new_with_positions( 2, TRUE,
								  PANGO_TAB_LEFT, 130,
								  PANGO_TAB_LEFT, 190),
		      NULL);
	textbuf3 = gtk_text_view_get_buffer( GTK_TEXT_VIEW (lEdfImportFileInfo));

	g_signal_connect_after( gtk_bin_get_child( GTK_BIN (eEdfImportGroup)),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);
	g_signal_connect_after( gtk_bin_get_child( GTK_BIN (eEdfImportSession)),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);
	g_signal_connect_after( gtk_bin_get_child( GTK_BIN (eEdfImportEpisode)),
			  "key-release-event", G_CALLBACK (check_gtk_entry_nonempty),
			  NULL);

      // --- assorted static objects
	gtk_widget_set_tooltip_markup( lMsmtHint, __tooltips[AGH_TIP_GENERAL]);

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





//static gboolean
//	AghMsmtViewDrawDetails = TRUE;

static time_t
	__timeline_start,
	__timeline_end;

static size_t
	__tl_left_margin = 45,
	__tl_right_margin = 20,
	__timeline_pixels,
	__timeline_pages,
	__pagesize;

static inline guint
T2P( time_t t)
{
	return difftime( t, __timeline_start) / 3600 * AghTimelinePPH;
}

static inline time_t
P2T( guint p)
{
	return (double)p * 3600 / AghTimelinePPH + __timeline_start;
}



typedef struct {
	struct SSubject
		*subject;
	GtkWidget
		*da;
	float	*power;
	size_t	 total_pages;
	gint     episode_focused;
	gboolean is_focused;
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
		// agh_SSubject_destruct( &J->subject);  // this is done every agh_expdesign_snapshot
		free( J->power);
		J->power = NULL;
		// gtk_widget_destroy( J->da);  // this is done in gtk_container_foreach( cMeasurements, gtk_widget_destroy)
	}
	g_array_free( g->subjects, TRUE);
}

static GArray	*GG;








#define JTLDA_HEIGHT 60

gboolean cMeasurements_drag_data_received_cb( GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, gpointer);
gboolean cMeasurements_drag_drop_cb( GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);

gboolean daSubjectTimeline_button_press_event_cb( GtkWidget*, GdkEventButton*, gpointer);
gboolean daSubjectTimeline_scroll_event_cb( GtkWidget*, GdkEventScroll*, gpointer);
gboolean daSubjectTimeline_expose_event_cb( GtkWidget*, GdkEventExpose*, gpointer);
gboolean daSubjectTimeline_enter_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_leave_notify_event_cb( GtkWidget*, GdkEventCrossing*, gpointer);
gboolean daSubjectTimeline_motion_notify_event_cb( GtkWidget*, GdkEventMotion*, gpointer);


int
virtual_d( struct SSubject *_j)
{
	int d = _j->n_sessions-1;
	while ( d > -1 )
		if ( strcmp( _j->sessions[d].name, AghD) == 0 )
			break;
		else
			--d;
	return d;
}

static SSubjectPresentation *J_paintable;

static void
__collect_one_subject_episodes()
{
	struct SSubject* _j = J_paintable->subject;

	int d = virtual_d( _j);
	if ( d == -1 ) {
//		J->power = NULL;
		return;
	}

	float *episode_signal;
	size_t episode_pages;

      // check for an impossible pagesize mismatch
	if ( __pagesize != agh_msmt_get_pagesize( _j->sessions[d].episodes[0].recordings[0]) ) {
		fprintf( stderr, "__collect_one_subject_episodes(): pagesize mismatch\n");
		return;
	}

//	printf( "__collect_one_subject_episodes( %s, %s, %s)\n", _j->name, _j->sessions[d].name, AghT);
	time_t	j_timeline_start = _j->sessions[d].episodes[0].start_rel;

	for ( guint e = 0; e < _j->sessions[d].n_episodes; ++e ) {
		// ...( J->subject->sessions[AghDi].episodes[e].; // rather use agh_msmt_find_by_jdeh than look up the matching channel
//		printf( "agh_msmt_find_by_jdeh( %s, %s, %s, %s)\n", _j->name, AghD, _j->sessions[AghDi].episodes[e].name, AghT);
		TRecRef recref = agh_msmt_find_by_jdeh( _j->name, _j->sessions[d].name, _j->sessions[d].episodes[e].name, AghT);
		if ( recref == NULL ) { // this can happen, too
			continue;
		}
		episode_pages = agh_msmt_get_power_course_in_range_as_float( recref,
									     AghOperatingRangeFrom, AghOperatingRangeUpto,
									     &episode_signal);
		time_t	j_e_start = _j->sessions[d].episodes[e].start_rel;
		//j_e_end = _j->sessions[AghDi].episodes[e].end_rel;
		// put power on the global timeline

		memcpy( &J_paintable->power[(j_e_start - j_timeline_start) / __pagesize],
			episode_signal,
			episode_pages * sizeof(float));

		free( episode_signal);
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
	guint g, j;

	for ( g = 0; g < GG->len; ++g )
		free_group_presentation( &Ai (GG, SGroupPresentation, g));
	g_array_set_size( GG, agh_cc.n_groups);

      // others should match
	__pagesize = agh_msmt_get_pagesize( agh_cc.groups[0].subjects[0].sessions[0].episodes[0].recordings[0]);

	time_t	earliest_start = (time_t)-1,
		latest_end = (time_t)-1;

      // first pass: determine common timeline and collect episodes' power
	for ( g = 0; g < agh_cc.n_groups; ++g ) {
		SGroupPresentation* G = &Ai (GG, SGroupPresentation, g);
		G->group = &agh_cc.groups[g];
		G->subjects = g_array_new( FALSE, FALSE, sizeof(SSubjectPresentation));
		g_array_set_size( G->subjects, G->group->n_subjects);

		for ( j = 0; j < G->group->n_subjects; ++j ) {
			SSubjectPresentation* J = &Ai (G->subjects, SSubjectPresentation, j);
			J->subject = &G->group->subjects[j];
			struct SSubject* _j = J->subject;

			int d = virtual_d( _j);
			if ( d != -1 ) {
				guint	j_n_episodes = _j->sessions[d].n_episodes;
				time_t	j_timeline_start = _j->sessions[d].episodes[0].start_rel,
					j_timeline_end   = _j->sessions[d].episodes[ j_n_episodes-1 ].end_rel;

			      // determine timeline global start and end
				if ( earliest_start == (time_t)-1) {
					earliest_start = j_timeline_start;
					latest_end = j_timeline_end;
				}
				if ( earliest_start > j_timeline_start )
					earliest_start = j_timeline_start;
				if ( latest_end < j_timeline_end )
					latest_end = j_timeline_end;

				// allocate subject timeline: will mostly be shorter than that common for all subjects
				J->total_pages = (j_timeline_end - j_timeline_start) / __pagesize;
				J->power = calloc( J->total_pages, sizeof(float));
				J_paintable = J;
				__collect_one_subject_episodes();
			} else
				J->power = NULL;
		}
	}

	__timeline_start = earliest_start;
	__timeline_end   = latest_end;
	__timeline_pixels = (__timeline_end - __timeline_start) / 3600 * AghTimelinePPH;
	__timeline_pages  = (__timeline_end - __timeline_start) / __pagesize;

	fprintf( stderr, "agh_populate_cMeasurements(): common timeline:\n");
	fputs (asctime (localtime(&earliest_start)), stderr);
	fputs (asctime (localtime(&latest_end)), stderr);

	__tl_left_margin = 0;
      // walk again thoroughly, set timeline drawing area length
	for ( g = 0; g < GG->len; ++g ) {
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
		gchar *g_escaped = g_markup_escape_text( G->group->name, -1);
		snprintf_buf( "<b>%s</b> (%u) %s", g_escaped, G->subjects->len, episodes_ext->str);
		g_free( g_escaped);
		g_string_free( episodes_ext, TRUE);
		G->expander = gtk_expander_new( __buf__);
		gtk_expander_set_use_markup( GTK_EXPANDER (G->expander), TRUE);
		g_object_set( G_OBJECT (G->expander),
			      "visible", TRUE,
			      "expanded", TRUE,
			      "height-request", -1,
			      NULL);
		gtk_box_pack_start( GTK_BOX (cMeasurements),
				    G->expander, TRUE, TRUE, 3);
		gtk_container_add( GTK_CONTAINER (G->expander),
				   G->vbox = gtk_vbox_new( TRUE, 1));
		g_object_set( G_OBJECT (G->vbox),
			      "height-request", -1,
			      NULL);
		for ( j = 0; j < G->subjects->len; ++j ) {
			SSubjectPresentation* J = &Ai (G->subjects, SSubjectPresentation, j);
			gtk_box_pack_start( GTK_BOX (G->vbox),
					    J->da = gtk_drawing_area_new(), TRUE, TRUE, 2);

			// determine __tl_left_margin
			cairo_t *cr = gdk_cairo_create( J->da->window);
			cairo_text_extents_t extents;
			cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size( cr, 11);
			cairo_text_extents( cr, J->subject->name, &extents);
			if ( __tl_left_margin < extents.width )
				__tl_left_margin = extents.width;
			cairo_destroy( cr);

			// set it later
//			g_object_set( G_OBJECT (J->da),
//				      "app-paintable", TRUE,
//				      "double-buffered", TRUE,
//				      "height-request", JTLDA_HEIGHT,
//				      "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
//				      NULL);
			J->episode_focused = -1;
			J->is_focused = FALSE;

			gtk_widget_add_events( J->da,
					       (GdkEventMask)
					       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
					       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
					       GDK_POINTER_MOTION_MASK);
			g_signal_connect_after( J->da, "expose-event",
						G_CALLBACK (daSubjectTimeline_expose_event_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "enter-notify-event",
						G_CALLBACK (daSubjectTimeline_enter_notify_event_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "leave-notify-event",
						G_CALLBACK (daSubjectTimeline_leave_notify_event_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "scroll-event",
						G_CALLBACK (daSubjectTimeline_scroll_event_cb),
						(gpointer)J);
			if ( J->power ) {
				g_signal_connect_after( J->da, "button-press-event",
							G_CALLBACK (daSubjectTimeline_button_press_event_cb),
							(gpointer)J);
				g_signal_connect_after( J->da, "motion-notify-event",
							G_CALLBACK (daSubjectTimeline_motion_notify_event_cb),
							(gpointer)J);
			}
			g_signal_connect_after( J->da, "drag-data-received",
						G_CALLBACK (cMeasurements_drag_data_received_cb),
						(gpointer)J);
			g_signal_connect_after( J->da, "drag-drop",
						G_CALLBACK (cMeasurements_drag_drop_cb),
						(gpointer)J);
			gtk_drag_dest_set( J->da, GTK_DEST_DEFAULT_ALL,
					   NULL, 0, GDK_ACTION_COPY);
			gtk_drag_dest_add_uri_targets( J->da);
		}
	}

      // walk quickly one last time to set __tl_left_margin
	__tl_left_margin += 10;
	for ( g = 0; g < GG->len; ++g ) {
		SGroupPresentation* G = &Ai (GG, SGroupPresentation, g);
		for ( j = 0; j < G->subjects->len; ++j ) {
			SSubjectPresentation* J = &Ai (G->subjects, SSubjectPresentation, j);
			g_object_set( G_OBJECT (J->da),
				      "app-paintable", TRUE,
				      "double-buffered", TRUE,
				      "height-request", JTLDA_HEIGHT,
				      "width-request", __timeline_pixels + __tl_left_margin + __tl_right_margin,
				      NULL);
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






static void
__draw_subject_timeline( cairo_t *cr, SSubjectPresentation *J)
{
      // draw subject name
	cairo_move_to( cr, 2, 15);
	cairo_select_font_face( cr, "serif", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size( cr, 11);
	cairo_show_text( cr, J->subject->name);

	if ( J->power == NULL ) {
		cairo_stroke( cr);
		cairo_move_to( cr, 50, JTLDA_HEIGHT/2+9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 18);
		cairo_set_source_rgba( cr, 0., 0., 0., .13);
		cairo_show_text( cr, "(no episodes)");
		return;
	}

	gint d = virtual_d( J->subject);
      // draw day and night
	{
		cairo_pattern_t *cp = cairo_pattern_create_linear( __tl_left_margin, 0., __timeline_pixels-__tl_right_margin, 0.);
		struct tm clock_time;
		memcpy( &clock_time, localtime( &__timeline_start), sizeof(clock_time));
		clock_time.tm_hour = 4;
		clock_time.tm_min = clock_time.tm_sec = 0;
		time_t	dawn = mktime( &clock_time),
			t;
		gboolean up = TRUE;
		for ( t = dawn; t < __timeline_end; t += 3600 * 12, up = !up )
			if ( t > __timeline_start ) {
//				printf( "part %lg %d\n", (double)T2P(t) / __timeline_pixels, up);
				cairo_pattern_add_color_stop_rgb( cp, (double)T2P(t) / __timeline_pixels, up?.5:.8, up?.4:.8, 1.);
			}
		cairo_set_source( cr, cp);
		cairo_rectangle( cr, __tl_left_margin, 0., __tl_left_margin+__timeline_pixels, JTLDA_HEIGHT);
		cairo_fill( cr);
		cairo_pattern_destroy( cp);
	}

	struct tm tl_start_fixed_tm;
	memcpy( &tl_start_fixed_tm, localtime( &__timeline_start), sizeof(struct tm));
	// determine the latest full hour before __timeline_start
	tl_start_fixed_tm.tm_min = 0;
	time_t tl_start_fixed = mktime( &tl_start_fixed_tm);

	guint	j_n_episodes;
	if ( (j_n_episodes = J->subject->sessions[d].n_episodes) == 0 ) {
		cairo_stroke( cr);
		return;
	}
	gulong	j_tl_pixel_start = difftime( J->subject->sessions[d].episodes[0].start_rel, __timeline_start) / 3600 * AghTimelinePPH,
		j_tl_pixel_end   = difftime( J->subject->sessions[d].episodes[j_n_episodes-1].end_rel, __timeline_start) / 3600 * AghTimelinePPH,
		j_tl_pixels = j_tl_pixel_end - j_tl_pixel_start;

      // boundaries, with scored percentage bars
	cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size( cr, 11);
	for ( guint e = 0; e < J->subject->sessions[d].n_episodes; ++e ) {
		struct SEpisode *_e = &J->subject->sessions[d].episodes[e];
		guint	e_pix_start = T2P( _e->start_rel),
			e_pix_end   = T2P( _e->end_rel);

		// episode start timestamp
		cairo_move_to( cr, __tl_left_margin + e_pix_start + 2, 12);
		cairo_set_source_rgb( cr, 1., 1., 1.);
		strftime( __buf__, 79, "%F %T",
			  localtime( &J->subject->sessions[d].episodes[e].start));
		g_string_printf( __ss__, "%s | %s",
				 __buf__, J->subject->sessions[d].episodes[e].name);
		cairo_show_text( cr, __ss__->str);
		cairo_stroke( cr);

		// percentage bar graph
 		float pc_scored, pc_nrem, pc_rem, pc_wake;
 		pc_scored =
 			agh_edf_get_scored_stages_breakdown(
 				agh_msmt_get_source( J->subject->sessions[d].episodes[e].recordings[0]),
 				&pc_nrem, &pc_rem, &pc_wake);

// 		printf( "N:%4.1f%% R:%4.1f%% W:%4.1f%% | %4.1f%%\n",
//			100 * pc_nrem, 100 * pc_rem, 100 * pc_wake,
 //					 pc_scored > agh_modelrun_get_req_percent_scored() ? "white" : "yellow",
// 				 100 * pc_scored);
// 		cairo_show_text( cr, __ss__->str);

		pc_scored *= (e_pix_end-e_pix_start);
		pc_nrem   *= (e_pix_end-e_pix_start);
		pc_rem    *= (e_pix_end-e_pix_start);
		pc_wake   *= (e_pix_end-e_pix_start);

		cairo_set_line_width( cr, 4);

		cairo_set_source_rgb( cr, 0., .1, .9);
		cairo_move_to( cr, __tl_left_margin + e_pix_start + 2, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_nrem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, .9, .0, .5);
		cairo_move_to( cr, __tl_left_margin + e_pix_start + 2 + pc_nrem, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_rem, 0);
		cairo_stroke( cr);

		cairo_set_source_rgb( cr, 0., .9, .1);
		cairo_move_to( cr, __tl_left_margin + e_pix_start + 2 + pc_nrem + pc_rem, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_wake, 0);
		cairo_stroke( cr);

		cairo_set_line_width( cr, 10);
		cairo_set_source_rgba( cr, 1., 1., 1., .5);
		cairo_move_to( cr, __tl_left_margin + e_pix_start + 2, JTLDA_HEIGHT-5);
		cairo_rel_line_to( cr, pc_scored, 0);
		cairo_stroke( cr);

		// highlight
		if ( J->episode_focused == e && J->is_focused ) {
			cairo_set_source_rgba( cr, 1., 1., 1., .5);
			cairo_rectangle( cr,
					 __tl_left_margin + e_pix_start - 5, 0,
					 e_pix_end-e_pix_start + 5, JTLDA_HEIGHT - 0);
			cairo_fill( cr);
		}
		cairo_stroke( cr);
	}

      // power
	cairo_set_source_rgb( cr,
			      (double)__fg0__[cPOWER_MT].red/65536,
			      (double)__fg0__[cPOWER_MT].green/65536,
			      (double)__fg0__[cPOWER_MT].blue/65536);
	cairo_set_line_width( cr, .3);
	cairo_move_to( cr, j_tl_pixel_start + __tl_left_margin, JTLDA_HEIGHT-12);
	for ( guint i = 0; i < J->total_pages; ++i )
		cairo_line_to( cr, j_tl_pixel_start + __tl_left_margin + ((float)i/J->total_pages * j_tl_pixels),
			       -J->power[i] * AghPPuV2 + JTLDA_HEIGHT-12);
	cairo_line_to( cr, j_tl_pixel_start + __tl_left_margin + j_tl_pixels, JTLDA_HEIGHT-12);
	cairo_fill( cr);

      // ticks
	if ( J->is_focused ) {
		cairo_set_line_width( cr, .5);
		cairo_set_source_rgb( cr,
				      (double)__fg0__[cTICKS_MT].red/65535,
				      (double)__fg0__[cTICKS_MT].green/65535,
				      (double)__fg0__[cTICKS_MT].blue/65535);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size( cr, 8);
		unsigned clock_d0 = localtime(&tl_start_fixed)->tm_mday;
		for ( time_t t = tl_start_fixed; t <= __timeline_end; t += 3600 ) {
			size_t x = T2P(t);
			unsigned
				clock_h  = localtime(&t)->tm_hour,
				clock_d  = localtime(&t)->tm_mday;
			if ( clock_h % 6 == 0 ) {
				cairo_move_to( cr, __tl_left_margin + x, ( clock_h % 24 == 0 ) ? 0 : (JTLDA_HEIGHT - 16));
				cairo_line_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 10);

				snprintf_buf_ts_h( (clock_d - clock_d0) * 24 + clock_h);
				cairo_text_extents_t extents;
				cairo_text_extents( cr, __buf__, &extents);
				cairo_move_to( cr, __tl_left_margin + x - extents.width/2, JTLDA_HEIGHT-1);
				cairo_show_text( cr, __buf__);

			} else {
				cairo_move_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 14);
				cairo_line_to( cr, __tl_left_margin + x, JTLDA_HEIGHT - 7);
			}
		}
	}
	cairo_stroke( cr);
}

static void
draw_subject_timeline_to_widget( GtkWidget *wid, SSubjectPresentation *J)
{
	cairo_t *cr = gdk_cairo_create( wid->window);

	__draw_subject_timeline( cr, J);

	cairo_destroy( cr);
}

static void
draw_subject_timeline_to_file( const char *fname, SSubjectPresentation *J)
{
#ifdef CAIRO_HAS_SVG_SURFACE
	cairo_surface_t *cs = cairo_svg_surface_create( fname,
							__timeline_pixels + __tl_left_margin + __tl_right_margin,
							JTLDA_HEIGHT);
	cairo_t *cr = cairo_create( cs);

	__draw_subject_timeline( cr, J);

	cairo_destroy( cr);
	cairo_surface_destroy( cs);
#endif
}



gboolean
daSubjectTimeline_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	if ( AghGs == 0 )
		return TRUE;

	draw_subject_timeline_to_widget( wid, (SSubjectPresentation*)userdata);

	return TRUE;
}






static gint
get_episode_from_timeline_click( struct SSubject* _j, guint along, const char **which_e)
{
	int d = virtual_d( _j);
	if ( d == -1 )
		return -1;

	along -= __tl_left_margin;
	for ( gint e = 0; e < _j->sessions[d].n_episodes; ++e ) {
		struct SEpisode *_e = &_j->sessions[d].episodes[e];
		if ( along >= T2P(_e->start_rel) && along <= T2P(_e->end_rel) ) {
			if ( which_e )
				(*which_e) = _e->name;
			return e;
		}
	}
	return -1;
}

gboolean
daSubjectTimeline_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	SSubjectPresentation *J = (SSubjectPresentation*)userdata;
	J->episode_focused = get_episode_from_timeline_click( J->subject, event->x, NULL);
	gtk_widget_queue_draw( wid);
	return TRUE;
}
gboolean
daSubjectTimeline_leave_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
{
	SSubjectPresentation *J = (SSubjectPresentation*)userdata;
	J->is_focused = FALSE;
	gtk_widget_queue_draw( wid);
	return TRUE;
}
gboolean
daSubjectTimeline_enter_notify_event_cb( GtkWidget *wid, GdkEventCrossing *event, gpointer userdata)
{
	SSubjectPresentation *J = (SSubjectPresentation*)userdata;
	J->is_focused = TRUE;
	gtk_widget_queue_draw( wid);
	return TRUE;
}





gboolean
daSubjectTimeline_button_press_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer userdata)
{
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
			snprintf_buf( "Scoring: %s\342\200\231s %s",
				      _j->name, AghE);
			gtk_window_set_title( GTK_WINDOW (wScoringFacility),
					      __buf__);
			gtk_window_set_default_size( GTK_WINDOW (wScoringFacility),
						     gdk_screen_get_width( gdk_screen_get_default()) * .93,
						     gdk_screen_get_height( gdk_screen_get_default()) * .92);
			gtk_widget_show_all( wScoringFacility);

			J_paintable = J;
			__collect_one_subject_episodes();
			gtk_widget_queue_draw( J->da);
		}
	    break;
	case 2:
	case 3:
		if ( event->state & GDK_MOD1_MASK ) {
			char *p;
			agh_subject_get_path( _j->name, &p);
			snprintf_buf( "%s/%s/%s.svg", p, AghD, AghT);
			free( p);
			p = g_strdup( __buf__);
			draw_subject_timeline_to_file( __buf__, J);
			snprintf_buf( "Wrote \"%s\"", p);
			g_free( p);
			gtk_statusbar_pop( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General);
			gtk_statusbar_push( GTK_STATUSBAR (sbMainStatusBar), agh_sb_context_id_General,
					    __buf__);
		} else if ( clicked_episode != -1 ) {
			TEDFRef edfref;
			agh_edf_find_by_jdeh( _j->name, AghD, AghEE[clicked_episode], AghT,
					      &edfref);
			char *tmp;
			const struct SEDFFile *edfstruct = agh_edf_get_info_from_sourceref( edfref, &tmp);
			gtk_text_buffer_set_text( textbuf2, tmp, -1);
			free( tmp);
			snprintf_buf( "%s header",
				      edfstruct->filename);
			gtk_window_set_title( GTK_WINDOW (wEDFFileDetails),
					      __buf__);
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









// --- drag-and-drop

static int
maybe_admit_one( char* fname)
{
	char *info;
	struct SEDFFile *edf = agh_edf_get_info_from_file( fname, &info);
	if ( edf == NULL || (edf->status & AGH_EDFCHK_INOPERABLE) ) {
		pop_ok_message( GTK_WINDOW (wMainWindow), "Could not read edf header in \"%s\"", fname);
		return 0;
	}
	gtk_text_buffer_set_text( textbuf3, info, -1);

	snprintf_buf( "File: <i>%s</i>", fname);
	gtk_label_set_markup( GTK_LABEL (lEdfImportCaption), __buf__);
	snprintf_buf( "<b>%s</b>", edf->PatientID);
	gtk_label_set_markup( GTK_LABEL (lEdfImportSubject), __buf__);

      // populate and attach models
	GtkListStore
		*m_groups = gtk_list_store_new( 1, G_TYPE_STRING),
		*m_episodes = gtk_list_store_new( 1, G_TYPE_STRING),
		*m_sessions = gtk_list_store_new( 1, G_TYPE_STRING);
	GtkTreeIter iter;
	guint i;
	for ( i = 0; i < AghGs; ++i ) {
		gtk_list_store_append( m_groups, &iter);
		gtk_list_store_set( m_groups, &iter, 0, AghGG[i], -1);
	}
	gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportGroup),
				 GTK_TREE_MODEL (m_groups));
	gtk_combo_box_entry_set_text_column( GTK_COMBO_BOX_ENTRY (eEdfImportGroup), 0);

	for ( i = 0; i < AghEs; ++i ) {
		gtk_list_store_append( m_episodes, &iter);
		gtk_list_store_set( m_episodes, &iter, 0, AghEE[i], -1);
	}
	gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportEpisode),
				 GTK_TREE_MODEL (m_episodes));
	gtk_combo_box_entry_set_text_column( GTK_COMBO_BOX_ENTRY (eEdfImportEpisode), 0);

	for ( i = 0; i < AghDs; ++i ) {
		gtk_list_store_append( m_sessions, &iter);
		gtk_list_store_set( m_sessions, &iter, 0, AghDD[i], -1);
	}
	gtk_combo_box_set_model( GTK_COMBO_BOX (eEdfImportSession),
				 GTK_TREE_MODEL (m_sessions));
	gtk_combo_box_entry_set_text_column( GTK_COMBO_BOX_ENTRY (eEdfImportSession), 0);

      // guess episode from fname
	char *fname2 = g_strdup( fname), *episode = strrchr( fname2, '/')+1;
	if ( g_str_has_suffix( episode, ".edf") || g_str_has_suffix( episode, ".EDF") )
		*strrchr( episode, '.') = '\0';
	gtk_entry_set_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportEpisode))),
			    episode);

      // display
	gtk_widget_set_sensitive( bEdfImportAdmit, FALSE);
	gtk_widget_set_sensitive( bEdfImportScoreSeparately, FALSE);
	gint response = gtk_dialog_run( GTK_DIALOG (wEdfImport));
	const gchar
		*selected_group   = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportGroup)))),
		*selected_session = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportSession)))),
		*selected_episode = gtk_entry_get_text( GTK_ENTRY (gtk_bin_get_child( GTK_BIN (eEdfImportEpisode))));
	switch ( response ) {
	case -5: // GTK_RESPONSE_OK:  Admit
	{	char *dest_path, *dest, *cmd;
		dest_path = g_strdup_printf( "%s/%s/%s/%s",
					     agh_cc.session_dir,
					     selected_group,
					     edf->PatientID,
					     selected_session);
		dest = g_strdup_printf( "%s/%s.edf",
					dest_path,
					selected_episode);
		if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (bEdfImportAttachCopy)) )
			cmd = g_strdup_printf( "mkdir -p '%s' && cp -n '%s' '%s'", dest_path, fname, dest);
		else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (bEdfImportAttachMove)) )
			cmd = g_strdup_printf( "mkdir -p '%s' && mv -n '%s' '%s'", dest_path, fname, dest);
		else
			cmd = g_strdup_printf( "mkdir -p '%s' && ln -s '%s' '%s'", dest_path, fname, dest);

		int cmd_exit = system( cmd);
		if ( cmd_exit )
			pop_ok_message( GTK_WINDOW (wMainWindow), "Command\n %s\nexited with code %d", cmd_exit);

		g_free( cmd);
		g_free( dest);
		g_free( dest_path);

		agh_ui_depopulate( 0);
		agh_ui_populate( 0);
	}
	    break;
	case -6: // GTK_RESPONSE_CANCEL:  Drop
		break;
	case -7: // GTK_RESPONSE_CLOSE:  View separately
		break;
	}

      // finalise
	g_free( fname2);

	g_object_unref( m_groups);
	g_object_unref( m_sessions);
	g_object_unref( m_episodes);

	agh_SEDFFile_destruct( edf);
	free( (void*)edf);

	free( info);

	return 0;
}


gboolean
cMeasurements_drag_data_received_cb( GtkWidget        *widget,
				     GdkDragContext   *context,
				     gint              x,
				     gint              y,
				     GtkSelectionData *selection_data,
				     guint             info,
				     guint             time,
				     gpointer          user_data)
{
        if ( (selection_data != NULL) && (selection_data->length >= 0) ) {
		gchar **uris = gtk_selection_data_get_uris( selection_data);

		guint i = 0;
		while ( uris[i] ) {
			if ( strncmp( uris[i], "file://", 7) == 0 ) {
				char *fname = g_filename_from_uri( uris[i], NULL, NULL);
				int retval = maybe_admit_one( fname);
				g_free( fname);
				if ( retval )
					break;
			}
			++i;
		}

		// fear no shortcuts
		do_rescan_tree();

		g_strfreev( uris);
        }

        gtk_drag_finish (context, TRUE, FALSE, time);
	return TRUE;
}


gboolean
cMeasurements_drag_drop_cb( GtkWidget      *widget,
			    GdkDragContext *context,
			    gint            x,
			    gint            y,
			    guint           time,
			    gpointer        user_data)
{
//	GdkAtom         target_type;
//
//      if ( context->targets ) {
//              // Choose the best target type
//              target_type = GDK_POINTER_TO_ATOM
//                      (g_list_nth_data( context->targets, 0));
//		unsigned i = g_list_length(context->targets);
//		while ( i-- )
//			printf( "%zu: %s\n", i, gdk_atom_name( GDK_POINTER_TO_ATOM (g_list_nth_data( context->targets, i))));
//
//		//Request the data from the source.
//              gtk_drag_get_data(
//                      widget,         // will receive 'drag-data-received' signal
//                      context,        // represents the current state of the DnD
//                      target_type,    // the target type we want
//                      time);          // time stamp
//
//	} else { // No target offered by source => error
//              return FALSE;
//	}
//
	return  TRUE;
}





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
