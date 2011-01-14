// ;-*-C-*- *  Time-stamp: "2011-01-14 03:27:48 hmmr"
/*
 *       File name:  ui/scoring-facility-patterns.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2011-01-14
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */




#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include <glade/glade.h>

#include "misc.h"
#include "ui.h"
#include "settings.h"
#include "scoring-facility.h"

#if HAVE_CONFIG_H
#  include <config.h>
#endif


unsigned
	AghBWFOrder	= 1;
float	AghBWFCutoff	= 3.0;

//int	AghDZCDFStepFine = 1;
float	AghDZCDFStep	= .125,
	AghDZCDFSigma	= 2;
size_t	AghDZCDFSmooth  = 3;

size_t	AghEnvTightness = 4;


GtkWidget
	*wPattern,
	*ePatternChannel;
static GtkWidget
	*daPatternSelection,
//	*lPatternName,
	*bPatternFindNext,
	*bPatternFindPrevious,
	*ePatternFilterOrder,
	*ePatternFilterCutoff,
	*ePatternEnvTightness,
	*ePatternDZCDFStep,
	*ePatternDZCDFSigma,
	*ePatternDZCDFSmooth,
	*cPatternLabelBox,
	*ePatternList = NULL,  // this one is not in glade
	*lPatternSimilarity,
	*ePatternParameterA,
	*ePatternParameterB,
	*ePatternParameterC,

	*wPatternName,
	*ePatternNameName,
	*ePatternNameSaveGlobally;



void ePatternList_changed_cb( GtkComboBox*, gpointer);

gulong	ePatternChannel_changed_cb_handler_id;
void ePatternChannel_changed_cb( GtkComboBox *combo, gpointer unused);

gint
agh_ui_construct_ScoringFacilityPatterns( GladeXML *xml)
{
	GtkCellRenderer *renderer;

      // ----- wPattern
	if ( !(wPattern			= glade_xml_get_widget( xml, "wPattern")) ||
	     !(daPatternSelection	= glade_xml_get_widget( xml, "daPatternSelection")) ||
	     !(bPatternFindPrevious	= glade_xml_get_widget( xml, "bPatternFindPrevious")) ||
	     !(bPatternFindNext		= glade_xml_get_widget( xml, "bPatternFindNext")) ||
	     !(ePatternFilterOrder	= glade_xml_get_widget( xml, "ePatternFilterOrder")) ||
	     !(ePatternFilterCutoff	= glade_xml_get_widget( xml, "ePatternFilterCutoff")) ||
	     !(ePatternDZCDFStep	= glade_xml_get_widget( xml, "ePatternDZCDFStep")) ||
	     !(ePatternDZCDFSigma	= glade_xml_get_widget( xml, "ePatternDZCDFSigma")) ||
	     !(ePatternDZCDFSmooth	= glade_xml_get_widget( xml, "ePatternDZCDFSmooth")) ||
	     !(lPatternSimilarity	= glade_xml_get_widget( xml, "lPatternSimilarity")) ||
	     !(ePatternEnvTightness	= glade_xml_get_widget( xml, "ePatternEnvTightness")) ||
	     !(ePatternParameterA	= glade_xml_get_widget( xml, "ePatternParameterA")) ||
	     !(ePatternParameterB	= glade_xml_get_widget( xml, "ePatternParameterB")) ||
	     !(ePatternParameterC	= glade_xml_get_widget( xml, "ePatternParameterC")) ||
//	     !(lPatternName		= glade_xml_get_widget( xml, "lPatternName")) ||
	     !(cPatternLabelBox		= glade_xml_get_widget( xml, "cPatternLabelBox")) ||
	     !(ePatternChannel		= glade_xml_get_widget( xml, "ePatternChannel")) ||
	     !(wPatternName		= glade_xml_get_widget( xml, "wPatternName")) ||
	     !(ePatternNameName		= glade_xml_get_widget( xml, "ePatternNameName")) ||
	     !(ePatternNameSaveGlobally	= glade_xml_get_widget( xml, "ePatternNameSaveGlobally")) )
		return -1;

	gtk_combo_box_set_model( GTK_COMBO_BOX (ePatternChannel),
				 GTK_TREE_MODEL (agh_mAllChannels));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (ePatternChannel), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (ePatternChannel), renderer,
					"text", 0,
					NULL);

	ePatternChannel_changed_cb_handler_id =
		g_signal_connect( ePatternChannel, "changed",
				  G_CALLBACK (ePatternChannel_changed_cb), NULL);

	return 0;
}


#define GLOBALLY_MARKER "[global] "


static const size_t
	__context_pad = 100;


struct SSignalPatternPrimer
	__pattern = { .data = NULL, };

size_t
	__pattern_ia, __pattern_iz;
size_t
	__pattern_wd;


static size_t
	__last_find = (size_t)-1;


static int
scandir_filter( const struct dirent *e)
{
	return strcmp( e->d_name, ".") && strcmp( e->d_name, "..");
}

void
__enumerate_patterns_to_combo()
{
	if ( ePatternList )
		gtk_widget_destroy( ePatternList);
	gtk_container_add( GTK_CONTAINER (cPatternLabelBox),
			   ePatternList = gtk_combo_box_new_text());
	g_signal_connect_after( ePatternList, "changed",
				G_CALLBACK (ePatternList_changed_cb),
				NULL);

	struct dirent **eps;
	int n;

	snprintf_buf( "%s/.patterns", agh_cc.session_dir);
	n = scandir( __buf__, &eps, scandir_filter, alphasort);
//	printf( "n = %d in %s\n", n, __buf__);
	if ( n >= 0 ) {
		for ( size_t cnt = 0; cnt < n; ++cnt ) {
			snprintf_buf( "%s%s", GLOBALLY_MARKER, eps[cnt]->d_name);
			gtk_combo_box_append_text( GTK_COMBO_BOX (ePatternList), __buf__);
		}
		free( (void*)eps);
	}

	char *j_path;
	agh_subject_get_path( __our_j->name, &j_path);
	snprintf_buf( "%s/.patterns", j_path);
	free( j_path);
	n = scandir (__buf__, &eps, scandir_filter, alphasort);
	if ( n >= 0 ) {
		for ( size_t cnt = 0; cnt < n; ++cnt )
			gtk_combo_box_append_text( GTK_COMBO_BOX (ePatternList), eps[cnt]->d_name);
		free( (void*)eps);
	}
}




static int
	__da_PatternSelection_height;
static float
	__pattern_display_scale = 1.2;

void
__mark_region_as_pattern()
{
	const SChannelPresentation *Ch = __clicked_channel;

	size_t	run = __marquee_to_az();
	if ( run == 0 )
		return;
	__pattern.context_before = (__pattern_ia < __context_pad) ? __pattern_ia : __context_pad;
	__pattern.context_after  = (__pattern_iz + __context_pad > Ch->n_samples) ? Ch->n_samples - __pattern_iz : __context_pad;

	size_t	full_sample = __pattern.context_before + run + __pattern.context_after;

	if ( !__pattern.data ) { // first run
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternEnvTightness), __pattern.env_tightness = AghEnvTightness);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterCutoff), __pattern.bwf_cutoff    = AghBWFCutoff);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterOrder),  __pattern.bwf_order     = AghBWFOrder);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFStep),    __pattern.dzcdf_step    = AghDZCDFStep);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma),   __pattern.dzcdf_sigma   = AghDZCDFSigma);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth),  __pattern.dzcdf_smooth  = AghDZCDFSmooth);
	}

	if ( __pattern.data )
		free( (void*)__pattern.data);
	assert (__pattern.data = malloc( (__pattern.n_samples = full_sample) * sizeof(float)));
	memcpy( __pattern.data,
		&Ch->signal_filtered[__pattern_ia - __pattern.context_before],
		full_sample * sizeof(float));

	g_object_set( G_OBJECT (daPatternSelection),
		      "height-request", (int)(__da_PatternSelection_height = AghSFDAPageHeight * __pattern_display_scale),
		      "width-request", (int)(__pattern_wd * __pattern_display_scale),
		      NULL);

	__last_find = (size_t)-1;

	gtk_widget_show_all( wPattern);
}







static void
__save_pattern( const char *label, gboolean do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns", agh_cc.session_dir);
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", agh_cc.session_dir, label);
	} else {
		char *j_path;
		agh_subject_get_path( __our_j->name, &j_path);
		snprintf_buf( "%s/.patterns", j_path);
		if ( mkdir( __buf__, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
			;
		snprintf_buf( "%s/.patterns/%s", j_path, label);
		free( j_path);
	}
	FILE *fd = fopen( __buf__, "w");
	if ( fd ) {
		size_t	tightness = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightness));
		unsigned
			order = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrder));
		float	cutoff = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoff));
		float	step   = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStep)),
			sigma  = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma));
		size_t	smooth = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth));
		float	a = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterA)),
			b = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterB)),
			c = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterC));
		size_t	full_sample = __pattern.context_before + (__pattern_iz - __pattern_ia) + __pattern.context_after;
		fprintf( fd,
			 "%zu  %u %g  %g %g %zu  %g %g %g\n"
			 "%zu %zu %zu\n",
			 tightness, order, cutoff, step, sigma, smooth, a, b, c,
			 full_sample, __pattern.context_before, __pattern.context_after);
		for ( size_t i = 0; i < full_sample; ++i )
			fprintf( fd, "%a\n", __pattern.data[i]);
		fclose( fd);
	}
}


static void
__load_pattern( const char *label, gboolean do_globally)
{
	if ( do_globally ) {
		snprintf_buf( "%s/.patterns/%s", agh_cc.session_dir, label);
	} else {
		char *j_path;
		agh_subject_get_path( __our_j->name, &j_path);
		snprintf_buf( "%s/.patterns/%s", j_path, label);
		free( j_path);
	}

	if ( __pattern.data )
		free( (void*)__pattern.data);

	FILE *fd = fopen( __buf__, "r");
	if ( fd ) {
		if ( fscanf( fd,
			     "%zu  %u %g  %g %g %zu  %g %g %g\n"
			     "%zu %zu %zu\n",
			     &__pattern.env_tightness,
			     &__pattern.bwf_order, &__pattern.bwf_cutoff,
			     &__pattern.dzcdf_step, &__pattern.dzcdf_sigma, &__pattern.dzcdf_smooth,
			     &__pattern.a, &__pattern.b, &__pattern.c,
			     &__pattern.n_samples, &__pattern.context_before, &__pattern.context_after) == 12 ) {
			assert (__pattern.data = (float*)malloc( __pattern.n_samples * sizeof(float)));
			for ( size_t i = 0; i < __pattern.n_samples; ++i )
				if ( fscanf( fd, "%a", &__pattern.data[i]) != 1 ) {
					fprintf( stderr, "__load_pattern(): short read at sample %zu from %s\n", i, __buf__);
					free( (void*)__pattern.data);
					__pattern.data = NULL;
					__pattern.n_samples = 0;
					break;
				}
		} else {
			__pattern.data = NULL;
			__pattern.n_samples = 0;
		}
		fclose( fd);
	}
}








gboolean
daPatternSelection_expose_event_cb( GtkWidget *wid, GdkEventExpose *event, gpointer userdata)
{
	if ( __pattern.data == NULL ) {
		cairo_t *cr = gdk_cairo_create( wid->window);
		cairo_set_source_rgba( cr, 1., 1., 1., .2);
		cairo_set_font_size( cr, 18);
		cairo_show_text( cr, "fafa");
		cairo_stroke( cr);
		cairo_destroy( cr);
		return FALSE;
	}

	SChannelPresentation *Ch = __clicked_channel;

	gint ht, wd;
	gdk_drawable_get_size( wid->window,
			       &wd, &ht);

	size_t	run = (__pattern_iz - __pattern_ia),
		full_sample = __pattern.context_before + run + __pattern.context_after;

	float	display_scale = Ch->signal_display_scale* __pattern_display_scale;

	cairo_t *cr = gdk_cairo_create( wid->window);
	cairo_set_source_rgb( cr, 1., 1., 1.);
	cairo_paint( cr);
	cairo_stroke( cr);

      // snippet
	cairo_set_source_rgb( cr, 0.1, 0.1, 0.1);
	cairo_set_line_width( cr, .8);
	__draw_signal( &__pattern.data[__pattern.context_before], run, display_scale,
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
		if ( agh_signal_get_envelope( __pattern.data, full_sample, Ch->samplerate,
					      &env_l, &env_u,
					      tightness,
					      &breadth) == 0 ) {
			gtk_widget_set_sensitive( bPatternFindNext, FALSE);
			gtk_widget_set_sensitive( bPatternFindPrevious, FALSE);
			cairo_set_source_rgba( cr, 0., 0., 0., .6);
			cairo_set_font_size( cr, 15);
			cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_text_extents_t extents;
			snprintf_buf( "Selection is too short");
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
		__draw_signal( &env_u[__pattern.context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		__draw_signal( &env_l[__pattern.context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		cairo_stroke( cr);
	}

      // low-pass filter
	{
		unsigned order = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrder));
		float cutoff = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoff));
		agh_signal_get_course( __pattern.data, full_sample, Ch->samplerate,
				       order, cutoff, 1,
				       &course);

		cairo_set_source_rgba( cr, 0.3, 0.3, 0.3, .5);
		cairo_set_line_width( cr, 3.);
		__draw_signal( &course[__pattern.context_before], run, display_scale,
			       wd, ht/3, cr, FALSE);
		cairo_stroke( cr);
	}

      // dzcdf
	{
		float	step   = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStep)),
			sigma  = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma)),
			smooth = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth));
		agh_signal_get_dzcdf( __pattern.data, full_sample, Ch->samplerate,
				      step, sigma, smooth,
				      &dzcdf);
		float	dzcdf_display_scale,
			avg = 0.;
		for ( size_t i = __pattern.context_before; i < __pattern.context_before + run; ++i )
			avg += dzcdf[i];
		avg /= run;
		dzcdf_display_scale = ht/5 / avg;

		cairo_set_source_rgba( cr, 0.3, 0.3, 0.99, .8);
		cairo_set_line_width( cr, 1.);
		__draw_signal( &dzcdf[__pattern.context_before], run, dzcdf_display_scale,
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





void
bPatternFind_clicked_cb( GtkButton *button, gpointer unused)
{
	set_cursor_busy( TRUE, wPattern);

	SChannelPresentation *Ch = __clicked_channel;

	size_t	run = __pattern_iz - __pattern_ia;
	__pattern.n_samples	= __pattern.context_before + run + __pattern.context_after;
	__pattern.samplerate	= Ch->samplerate;
	__pattern.bwf_order	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterOrder));
	__pattern.bwf_cutoff	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternFilterCutoff));
	__pattern.bwf_scale	= 1;
	__pattern.dzcdf_step	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFStep));
	__pattern.dzcdf_sigma	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma));
	__pattern.dzcdf_smooth	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth));
	__pattern.env_tightness	= gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternEnvTightness));
	__pattern.a = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterA));
	__pattern.b = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterB));
	__pattern.c = gtk_spin_button_get_value( GTK_SPIN_BUTTON (ePatternParameterC));


      // reprocess the field if the pattern is set up with different tightness etc
	if ( __pattern.bwf_order  != Ch->bwf_order ||
	     __pattern.bwf_cutoff != Ch->bwf_cutoff ) {
		free( Ch->signal_course);
		agh_signal_get_course( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
				       Ch->bwf_cutoff = __pattern.bwf_cutoff, Ch->bwf_order = __pattern.bwf_order, 1,
				       &Ch->signal_course);
	}
	if ( __pattern.env_tightness != Ch->env_tightness ) {
		free( Ch->envelope_lower);
		free( Ch->envelope_upper);
		free( Ch->signal_breadth);
		agh_signal_get_envelope( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
					 &Ch->envelope_lower,
					 &Ch->envelope_upper,
					 __pattern.env_tightness,
					 &Ch->signal_breadth);
	}
	if ( __pattern.dzcdf_sigma  != Ch->dzcdf_sigma ||
	     __pattern.dzcdf_step   != Ch->dzcdf_step ||
	     __pattern.dzcdf_smooth != Ch->dzcdf_smooth ) {
		free( Ch->signal_dzcdf);
		agh_signal_get_dzcdf( Ch->signal_filtered, Ch->n_samples, Ch->samplerate,
				      Ch->dzcdf_step = __pattern.dzcdf_step,
				      Ch->dzcdf_sigma = __pattern.dzcdf_sigma,
				      Ch->dzcdf_smooth = __pattern.dzcdf_smooth,
				      &Ch->signal_dzcdf);
	}

	gboolean
		go_forward = strcmp( gtk_widget_get_name( GTK_WIDGET (button)), "bPatternFindNext") == 0;
	size_t from;
	if ( __last_find == (size_t)-1 )
		from = go_forward
			? __pattern.context_before
			: Ch->n_samples - __pattern.context_before - __pattern.context_after - (__pattern_iz - __pattern_ia);
	else
		from = __last_find + (go_forward ? 10 : -10);
	__last_find =
		agh_signal_find_pattern_( &__pattern,
					  Ch->signal_course,
					  Ch->signal_breadth,
					  Ch->signal_dzcdf,
					  Ch->n_samples,
					  from,
					  go_forward ? 1 : -1);

	if ( __last_find == (size_t)-1 )
		pop_ok_message( GTK_WINDOW (wPattern), "Not found");
	else {
		__cur_page_app = (__last_find / Ch->samplerate / APSZ);
		gint	wd;
		gdk_drawable_get_size( Ch->da_page->window, &wd, NULL);
		size_t	lpp = Ch->samplerate * APSZ;
		__marking_in_widget = Ch->da_page;
		__marquee_start = (float)(__last_find % lpp) / lpp * wd;
		__marquee_virtual_end = __marquee_start + (float)run / lpp * wd;
		gtk_spin_button_set_value( GTK_SPIN_BUTTON (eScoringFacCurrentPage), __cur_page_app+1);
		REDRAW_ALL;
		while ( gtk_events_pending() )
			gtk_main_iteration();
		__marking_in_widget = NULL;

/*		gint wd;
		gdk_drawable_get_size( Ch->da_page->window, &wd, NULL);
*/
		snprintf_buf( "at p. %zu (a = %4.2f, b = %4.2f, c = %4.2f)\n",
			      __cur_page_app+1, __pattern.match_a, __pattern.match_b, __pattern.match_c);
		gtk_label_set_markup( GTK_LABEL (lPatternSimilarity), __buf__);

		gtk_widget_queue_draw( lPatternSimilarity);
		gtk_widget_queue_draw( daPatternSelection);
	}

	set_cursor_busy( FALSE, wPattern);
}





void
bPatternSave_clicked_cb()
{
	if ( gtk_dialog_run( GTK_DIALOG (wPatternName)) == GTK_RESPONSE_OK ) {
		const char *label = gtk_entry_get_text( GTK_ENTRY (ePatternNameName));
		gboolean do_globally = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON (ePatternNameSaveGlobally));
		snprintf_buf( "%s%s", do_globally ? GLOBALLY_MARKER : "", label);
		__save_pattern( __buf__, do_globally);
		//snprintf_buf( "<b>%s</b>", label);
		//gtk_label_set_markup( GTK_LABEL (lPatternName), __buf__);

		// add to dropdown list
		gtk_combo_box_append_text( GTK_COMBO_BOX (ePatternList), label);
	}
}


void
ePatternList_changed_cb( GtkComboBox *combo, gpointer unused)
{
	char *label = gtk_combo_box_get_active_text( combo);
	gboolean do_globally = strncmp( label, GLOBALLY_MARKER, strlen( GLOBALLY_MARKER)) == 0;
	char *fname = do_globally
		? label + strlen(GLOBALLY_MARKER)
		: label;
	printf( "%d %s, %s\n", do_globally, label, fname);
	__load_pattern( fname, do_globally);
	free( label);

	__pattern_iz = __pattern.n_samples - __pattern.context_after;
	__pattern_ia = __pattern.context_before;

	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternEnvTightness), AghEnvTightness = __pattern.env_tightness);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterCutoff), AghBWFCutoff    = __pattern.bwf_cutoff   );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternFilterOrder),  AghBWFOrder     = __pattern.bwf_order    );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFStep),    AghDZCDFStep    = __pattern.dzcdf_step   );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSigma),   AghDZCDFSigma   = __pattern.dzcdf_sigma  );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON (ePatternDZCDFSmooth),  AghDZCDFSmooth  = __pattern.dzcdf_smooth );

	gtk_label_set_markup( GTK_LABEL (lPatternSimilarity), "");

	if ( __clicked_channel == NULL )
		__clicked_channel = &Ai (HH, SChannelPresentation, 0);
	FAFA;
	gtk_widget_queue_draw( daPatternSelection);
}

gboolean
wPattern_delete_event_cb()
{
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (bScoringFacShowFindDialog), FALSE);
	return TRUE;
}



void
ePatternChannel_changed_cb( GtkComboBox *combo, gpointer unused)
{
	gint h_sel = gtk_combo_box_get_active( combo);
	if ( h_sel == -1 )
		return;

	for ( size_t h = 0; h < __n_visible; ++h ) {
		SChannelPresentation *Ch = &Ai (HH, SChannelPresentation, h);
		if ( strcmp( Ch->name, AghHH[h_sel]) == 0 )
			__clicked_channel = Ch;
	}
}



// EOF
