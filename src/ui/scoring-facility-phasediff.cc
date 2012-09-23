// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-phasediff.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-01-29
 *
 *         Purpose:  scoring facility phase diff dialog
 *
 *         License:  GPL
 */




#include "../common/lang.hh"
#include "misc.hh"
#include "scoring-facility.hh"
#include "scoring-facility_cb.hh"


using namespace std;


aghui::SScoringFacility::SPhasediffDialog::
SPhasediffDialog( aghui::SScoringFacility& parent)
      : channel1 (nullptr),
	channel2 (nullptr),
	use_original_signal (false),
	from (1.), upto (2.),
	bwf_order (1),
	scope (0),
	display_scale (1.),
	course (0), // have no total_pages() known yet
	smooth_side (1),
	_p (parent)
{
}

aghui::SScoringFacility::SPhasediffDialog::
~SPhasediffDialog()
{
	gtk_widget_destroy( (GtkWidget*)wSFPD);
}


int
aghui::SScoringFacility::SPhasediffDialog::
construct_widgets()
{
	GtkCellRenderer *renderer;

      // ------- wPhaseDiff
	if ( !(AGH_GBGETOBJ3 (_p.builder, GtkDialog, wSFPD)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkDrawingArea, daSFPD)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox, eSFPDChannelA)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkComboBox, eSFPDChannelB)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eSFPDFreqFrom)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkSpinButton, eSFPDBandwidth)) ||
	     !(AGH_GBGETOBJ3 (_p.builder, GtkScaleButton, eSFPDSmooth)) )
		return -1;

	gtk_combo_box_set_model( eSFPDChannelA,
				 (GtkTreeModel*)_p._p.mEEGChannels);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPDChannelA, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPDChannelA, renderer,
					"text", 0,
					NULL);
	eSFPDChannelA_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelA, "changed",
				  G_CALLBACK (eSFPDChannelA_changed_cb),
				  this);

	gtk_combo_box_set_model( eSFPDChannelB,
				 (GtkTreeModel*)_p._p.mEEGChannels);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start( (GtkCellLayout*)eSFPDChannelB, renderer, FALSE);
	gtk_cell_layout_set_attributes( (GtkCellLayout*)eSFPDChannelB, renderer,
					"text", 0,
					NULL);
	eSFPDChannelB_changed_cb_handler_id =
		g_signal_connect( eSFPDChannelB, "changed",
				  G_CALLBACK (eSFPDChannelB_changed_cb),
				  this);

	g_signal_connect( daSFPD, "draw",
			  G_CALLBACK (daSFPD_draw_cb),
			  this);
	g_signal_connect( daSFPD, "scroll-event",
			  G_CALLBACK (daSFPD_scroll_event_cb),
			  this);
	g_signal_connect( eSFPDChannelA, "changed",
			  G_CALLBACK (eSFPDChannelA_changed_cb),
			  this);
	g_signal_connect( eSFPDChannelB, "changed",
			  G_CALLBACK (eSFPDChannelB_changed_cb),
			  this);
	g_signal_connect( eSFPDFreqFrom, "value-changed",
			  G_CALLBACK (eSFPDFreqFrom_value_changed_cb),
			  this);
	g_signal_connect( eSFPDBandwidth, "value-changed",
			  G_CALLBACK (eSFPDBandwidth_value_changed_cb),
			  this);
	g_signal_connect( eSFPDSmooth, "value-changed",
			  G_CALLBACK (eSFPDSmooth_value_changed_cb),
			  this);
	g_signal_connect( wSFPD, "show",
			  G_CALLBACK (wSFPD_show_cb),
			  this);
	g_signal_connect( wSFPD, "hide",
			  G_CALLBACK (wSFPD_hide_cb),
			  this);
	return 0;
}




void
aghui::SScoringFacility::SPhasediffDialog::
update_course()
{
	if ( channel1->samplerate() != channel2->samplerate() )
		return;

      // set scope, now that we know the samplerate
	scope = channel1->samplerate() * .5;

	if ( course.size() == 0 )
		course.resize( _p.total_pages());
	for ( size_t p = 0; p < course.size()-1; ++p ) {
		size_t	pa = _p.pagesize() * channel1->samplerate() *  p,
			pz = _p.pagesize() * channel1->samplerate() * (p+1);
		course[p] =
			(channel1->artifacts.region_dirty_fraction(pa, pz) > .2 ||
			 channel2->artifacts.region_dirty_fraction(pa, pz) > .2)
			? NAN
			: sigproc::phase_diff(
				use_original_signal ? channel1->signal_original : channel1->signal_filtered,
				use_original_signal ? channel2->signal_original : channel2->signal_filtered,
				channel1 -> samplerate(),
				pa, pz,
				from, upto,
				bwf_order,
				scope);
	}
}

const aghui::SScoringFacility::SChannel*
aghui::SScoringFacility::SPhasediffDialog::
channel_from_cbox( GtkComboBox *cbox)
{
	GtkTreeIter iter;
	if ( gtk_combo_box_get_active_iter( cbox, &iter) == FALSE )
		return nullptr;

	char *entry;
	gtk_tree_model_get( gtk_combo_box_get_model( cbox), &iter,
			    0, &entry,
			    -1);
	for ( auto &H : _p.channels )
		if ( strcmp( entry, H.name) == 0 )
			return &H;
	return nullptr;
}


void
aghui::SScoringFacility::SPhasediffDialog::
preselect_channel( GtkComboBox *cbox, const char *ch)
{
	GtkTreeModel *model = gtk_combo_box_get_model( cbox);
	GtkTreeIter iter;
	gboolean valid;
	valid = gtk_tree_model_get_iter_first( model, &iter);
	while ( valid ) {
		DEF_UNIQUE_CHARP (entry);

		gtk_tree_model_get( model, &iter,
				    0, &entry,
				    -1);
		if ( strcmp( entry, ch) == 0 ) {
			gtk_combo_box_set_active_iter( cbox, &iter);
			return;
		}
		valid = gtk_tree_model_iter_next( model, &iter);
	}
}





void
aghui::SScoringFacility::SPhasediffDialog::
draw( cairo_t* cr, int wd, int ht)
{
	auto& SF = _p;
	auto& ED = SF._p;

	ED.CwB[SExpDesignUI::TColour::hypnogram].set_source_rgb( cr);
	cairo_rectangle( cr, 0, 0, wd, ht);
	cairo_fill( cr);
	cairo_stroke( cr);

	if ( channel1 == channel2 ) {
		cairo_move_to( cr, 5, 15);
		cairo_show_text( cr, "Same channel");
		cairo_stroke( cr);
		return;
	}

	if ( course.size() == 0 ) {
		cairo_move_to( cr, 5, 15);
		cairo_show_text( cr, "Huh?");
		cairo_stroke( cr);
		return;
	}

	if ( channel1->samplerate() != channel2->samplerate() ) {
		cairo_move_to( cr, 5, 15);
		cairo_show_text( cr, "Incompatible channels (different samplerate)");
		cairo_stroke( cr);
		return;
	}

      // psd course in selected freq range
	{
		auto	C1 = channel1->crecording.sigfile::CBinnedPower::course<TFloat>( from, upto);
//			C2 = channel2->crecording.sigfile::CBinnedPower::course<TFloat>( from, upto) * display_scale + ht/2;

		ED.CwB[SExpDesignUI::TColour::profile_psd_sf].set_source_rgba( cr, .5);
		auto	scale =
			agh::alg::calibrate_display_scale( C1, C1.size(), ht/2.);
		cairo_move_to( cr, 0, ht - C1[0]);
		for ( size_t i = 1; i < C1.size(); ++i )
			cairo_line_to( cr, ((double)i+.5) / C1.size() * wd, ht - C1[i] * scale);
		cairo_line_to( cr, wd, ht);
		cairo_line_to( cr, 0, ht);
		cairo_line_to( cr, 0, C1[0]);
		cairo_fill( cr);
		cairo_stroke( cr);
	}

      // zeroline and hour ticks
	{
		ED.CwB[SExpDesignUI::TColour::ticks_sf].set_source_rgb( cr);
		cairo_set_line_width( cr, 1);
		cairo_move_to( cr, 0,  ht/2);
		cairo_line_to( cr, wd, ht/2);
		cairo_stroke( cr);

		cairo_set_font_size( cr, 10);
		float	hours4 = channel1->crecording.F().length_in_seconds() / 3600. * 4;
		for ( size_t i = 1; i < hours4; ++i ) {
			unsigned tick_pos = (float)i / hours4 * wd;
			cairo_move_to( cr, tick_pos, 0);
			if ( i % 4 == 0 )
				cairo_set_line_width( cr, 1.);
			else if ( i % 2 == 0 )
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
	}

      // course
	{
		auto tmp = course;
		sigproc::smooth( tmp, smooth_side);

		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_set_line_width( cr, .8);
		for ( size_t p = 1; p < tmp.size()-1; ++p )
			if ( isfinite(tmp[p-1]) && isfinite(tmp[p]) ) {
				cairo_move_to( cr,
					       (float)(p-1)/(tmp.size()-1) * wd, tmp[p-1] * 1000 * display_scale + ht/2);
				cairo_line_to( cr,
					       (float)(p  )/(tmp.size()-1) * wd, tmp[p  ] * 1000 * display_scale + ht/2);
				cairo_stroke( cr);
			}
	}

      // scale bar
	{
		cairo_set_source_rgb( cr, 0., 0., 0.);
		cairo_set_line_width( cr, 1.5);
		double dpuf =
			agh::alg::sensible_scale_reduction_factor(
				1e3 * display_scale, ht);
		int x = 10;
		cairo_move_to( cr, x, 5);
		cairo_rel_line_to( cr, 0, dpuf * (1e3 * display_scale));
		cairo_stroke( cr);

		cairo_set_font_size( cr, 9);
		cairo_select_font_face( cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_move_to( cr, x + 5, 10);
		snprintf_buf( "%g ms", dpuf);
		cairo_show_text( cr, __buf__);
		cairo_stroke( cr);
	}
}


// eof
