// ;-*-C-*- *  Time-stamp: "2011-03-06 12:55:43 hmmr"
/*
 *       File name:  ui/scoring-facility-phasediff.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-29
 *
 *         Purpose:  scoring facility phase diff dialog
 *
 *         License:  GPL
 */




#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include <glade/glade.h>
#include <cairo-svg.h>

#include "../libexstrom/iface.h"
#include "misc.h"
#include "ui.h"
#include "settings.h"
#include "scoring-facility.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif



GtkWidget
	*wPhaseDiff,
	*ePhaseDiffChannelA,
	*ePhaseDiffChannelB;

static GtkWidget
	*daPhaseDiff,
	*ePhaseDiffFreqFrom,
	*ePhaseDiffFreqUpto,
	*bPhaseDiffApply;


void ePhaseDiffChannelA_changed_cb( GtkComboBox*, gpointer);
void ePhaseDiffChannelB_changed_cb( GtkComboBox*, gpointer);
gulong
	ePhaseDiffChannelA_changed_cb_handler_id,
	ePhaseDiffChannelB_changed_cb_handler_id;

gint
agh_ui_construct_ScoringFacility_PhaseDiff( GladeXML *xml)
{
	GtkCellRenderer *renderer;

      // ------- wPhaseDiff
	if ( !(wPhaseDiff		= glade_xml_get_widget( xml, "wPhaseDiff")) ||
	     !(daPhaseDiff		= glade_xml_get_widget( xml, "daPhaseDiff")) ||
	     !(ePhaseDiffChannelA	= glade_xml_get_widget( xml, "ePhaseDiffChannelA")) ||
	     !(ePhaseDiffChannelB	= glade_xml_get_widget( xml, "ePhaseDiffChannelB")) ||
	     !(ePhaseDiffFreqFrom	= glade_xml_get_widget( xml, "ePhaseDiffFreqFrom")) ||
	     !(ePhaseDiffFreqUpto	= glade_xml_get_widget( xml, "ePhaseDiffFreqUpto")) ||
	     !(bPhaseDiffApply		= glade_xml_get_widget( xml, "bPhaseDiffApply")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelA),
				 GTK_TREE_MODEL (agh_mEEGChannels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT (ePhaseDiffChannelA), renderer, FALSE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT (ePhaseDiffChannelA), renderer,
					"text", 0,
					NULL);
	ePhaseDiffChannelA_changed_cb_handler_id =
		g_signal_connect( ePhaseDiffChannelA, "changed",
				  G_CALLBACK (ePhaseDiffChannelA_changed_cb), NULL);

	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelB),
				 GTK_TREE_MODEL (agh_mEEGChannels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( GTK_CELL_LAYOUT (ePhaseDiffChannelB), renderer, FALSE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT (ePhaseDiffChannelB), renderer,
					"text", 0,
					NULL);
	ePhaseDiffChannelB_changed_cb_handler_id =
		g_signal_connect( ePhaseDiffChannelB, "changed",
				  G_CALLBACK (ePhaseDiffChannelB_changed_cb), NULL);

	return 0;
}



static struct SChannelPresentation
	*__phasediff_channel1,
	*__phasediff_channel2;
static gboolean
	__use_original_signal = TRUE;
static float
	__phasediff_freq_from = 0.,
	__phasediff_freq_upto = 1.;

static unsigned
	__phasediff_order = 1,
	__phasediff_scope = 10;
static float
	__phasediff_display_scale = 1.;



static float
	*__phasediff_course;
static size_t
	__phasediff_course_size;

void
__update_course()
{
	if ( __phasediff_course == NULL )
		__phasediff_course = (float*)malloc( (__phasediff_course_size = __total_pages - 1) * sizeof(float));
	if ( __phasediff_course_size != __total_pages - 1 )
		__phasediff_course = (float*)realloc( __phasediff_course,
						      (__phasediff_course_size = __total_pages - 1) * sizeof(float));
//	printf( "ch1 = %s, ch2 = %s, f1 = %g, f2 = %g, order = %u\n", __phasediff_channel1->name, __phasediff_channel2->name, __phasediff_freq_from, __phasediff_freq_upto, __phasediff_order);
	for ( size_t p = 0; p < __total_pages-1; ++p )
		__phasediff_course[p] =
			signal_phasediff( __use_original_signal ? __phasediff_channel1->signal_original : __phasediff_channel1->signal_filtered,
					  __use_original_signal ? __phasediff_channel2->signal_original : __phasediff_channel2->signal_filtered,
					  __phasediff_channel1 -> samplerate,
					  PSZ * __phasediff_channel1->samplerate *  p,
					  PSZ * __phasediff_channel1->samplerate * (p+1),
					  __phasediff_freq_from,
					  __phasediff_freq_upto,
					  __phasediff_order,
					  __phasediff_scope);
}

gboolean
daPhaseDiff_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	cairo_t *cr = gdk_cairo_create( wid->window);

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	if ( __phasediff_course == NULL ) {
		return TRUE;
	}

      // zeroline and hour ticks
	cairo_set_source_rgb( cr,
			      (double)__fg1__[cTICKS_SF].red/65536,
			      (double)__fg1__[cTICKS_SF].green/65536,
			      (double)__fg1__[cTICKS_SF].blue/65536);
	cairo_set_line_width( cr, 1);
	cairo_move_to( cr, 0,  ht/2);
	cairo_line_to( cr, wd, ht/2);
	cairo_stroke( cr);

	cairo_set_font_size( cr, 10);
	float	hours4 = __total_pages * PSZ / 3600. * 4;
	for ( size_t i = 1; i < hours4; ++i ) {
		unsigned tick_pos = (float)i / hours4 * wd;
		cairo_move_to( cr, tick_pos, 0);
		if ( i % 4 == 0 ) {
			cairo_set_line_width( cr, 1.);
		} else if ( i % 2 == 0 )
			cairo_set_line_width( cr, .5);
		else
			cairo_set_line_width( cr, .25);
		cairo_line_to( cr, tick_pos, ht);
		if ( i % 4 == 0 ) {
			snprintf_buf( "%2zuh", i / 4);
			cairo_move_to( cr, tick_pos+5, 12);
			cairo_show_text( cr, __buf__);
		}
		cairo_stroke( cr);
	}

      // course
	cairo_set_source_rgb( cr, 0., 0., 0.);
	cairo_set_line_width( cr, .5);
	cairo_move_to( cr, 0., __phasediff_course[0] * 1000 * __phasediff_display_scale + ht/2);
	for ( size_t p = 0; p < __total_pages-1; ++p )
		cairo_line_to( cr,
			       (float)p/(__total_pages-1) * wd,
			       __phasediff_course[p] * 1000 * __phasediff_display_scale + ht/2);
	cairo_stroke( cr);

      // scale
	cairo_set_line_width( cr, 3.);
	cairo_move_to( cr, 10., 10.);
	cairo_rel_line_to( cr, 0., 1000 * __phasediff_display_scale);
	cairo_move_to( cr, 15, 12);
	cairo_show_text( cr, "1 ms");
	cairo_stroke( cr);

	cairo_destroy( cr);

	return TRUE;
}


gboolean
daPhaseDiff_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	switch ( event->direction ) {
	case GDK_SCROLL_UP:
		__phasediff_display_scale *= 1.1;
	    break;
	case GDK_SCROLL_DOWN:
		__phasediff_display_scale /= 1.1;
	default:
	    break;
	}

	gtk_widget_queue_draw( wid);

	return TRUE;
}




static struct SChannelPresentation*
__set_channel_from_cbox( GtkComboBox *cbox)
{
	GtkTreeIter iter;
	if ( gtk_combo_box_get_active_iter( cbox, &iter) == FALSE )
		return NULL;
	char *entry;
	gtk_tree_model_get( gtk_combo_box_get_model( cbox), &iter,
			    0, &entry,
			    -1);
	for ( size_t h = 0; h < __n_all_channels; ++h )
		if ( strcmp( entry, HH[h].name) == 0 )
			return &HH[h];
	return NULL;
}

void
ePhaseDiffChannelA_changed_cb( GtkComboBox *cbox, gpointer unused)
{
	__phasediff_channel1 = __set_channel_from_cbox( cbox);
	gtk_widget_set_sensitive( bPhaseDiffApply,
				  __phasediff_channel1 != __phasediff_channel2);
}

void
ePhaseDiffChannelB_changed_cb( GtkComboBox *cbox, gpointer unused)
{
	__phasediff_channel2 = __set_channel_from_cbox( cbox);
	gtk_widget_set_sensitive( bPhaseDiffApply,
				  __phasediff_channel1 != __phasediff_channel2);
}




void
ePhaseDiffFreqFrom_value_changed_cb( GtkSpinButton *spinbutton,
				     gpointer       user_data)
{
	__phasediff_freq_from = fabs( gtk_spin_button_get_value( spinbutton) * 10) / 10;
	gtk_widget_set_sensitive( bPhaseDiffApply,
				  __phasediff_freq_from < __phasediff_freq_upto);
}

void
ePhaseDiffFreqUpto_value_changed_cb( GtkSpinButton *spinbutton,
				     gpointer       user_data)
{
	__phasediff_freq_upto = fabs( gtk_spin_button_get_value( spinbutton) * 10) / 10;
	gtk_widget_set_sensitive( bPhaseDiffApply,
				  __phasediff_freq_from < __phasediff_freq_upto);
}



static void
__preselect_channel( GtkComboBox *cbox, const char *ch)
{
	GtkTreeModel *model = gtk_combo_box_get_model( cbox);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		char *entry;
		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( cbox, &iter);
			free( entry);
			return;
		}
		free( entry);
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}




void
bPhaseDiffApply_clicked_cb( GtkButton *button,
			    gpointer   user_data)
{
	__update_course();
	gtk_widget_queue_draw( daPhaseDiff);
}



void
wPhaseDiff_show_cb()
{
	if ( gtk_combo_box_get_active( GTK_COMBO_BOX (ePhaseDiffChannelA)) == -1 ||
	     gtk_combo_box_get_active( GTK_COMBO_BOX (ePhaseDiffChannelB)) == -1 ) {
		__phasediff_channel1 = &HH[0];  // following channel sort order in core, EEG channels come first
		__phasediff_channel2 = &HH[1];
		__preselect_channel( GTK_COMBO_BOX (ePhaseDiffChannelA), __phasediff_channel1->name);
		__preselect_channel( GTK_COMBO_BOX (ePhaseDiffChannelB), __phasediff_channel2->name);
	} else {
		__phasediff_channel1 = __set_channel_from_cbox( GTK_COMBO_BOX (ePhaseDiffChannelA));
		__phasediff_channel2 = __set_channel_from_cbox( GTK_COMBO_BOX (ePhaseDiffChannelB));
	}

	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePhaseDiffFreqFrom), __phasediff_freq_from);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePhaseDiffFreqUpto), __phasediff_freq_upto);

	gtk_widget_set_sensitive( bPhaseDiffApply,
				  __phasediff_channel1 != __phasediff_channel2 &&
				  __phasediff_freq_from < __phasediff_freq_upto);
}

void
wPhaseDiff_hide_cb()
{
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (bScoringFacShowPhaseDiffDialog), FALSE);
}


// eof
