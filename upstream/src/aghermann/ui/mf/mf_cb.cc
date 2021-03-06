/*
 *       File name:  aghermann/ui/mf/mf_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  modelrun facility callbacks
 *
 *         License:  GPL
 */

#include <sstream>
#include <cairo-svg.h>

#include "aghermann/model/beersma.hh"
#include "aghermann/ui/misc.hh"
#include "mf.hh"

using namespace std;
using namespace agh::ui;

extern "C" {

gboolean
daMFProfile_configure_event_cb(
	GtkWidget*,
	GdkEventConfigure *event,
	const gpointer userdata)
{
	if ( event->type == GDK_CONFIGURE ) {
		auto& MF = *(SModelrunFacility*)userdata;
		MF.da_ht = event->height;
		MF.da_wd = event->width;
	}
	return FALSE;
}


gboolean
daMFProfile_draw_cb(
	GtkWidget*,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;
	MF.draw_timeline( cr);

	MF.update_infobar();

	return TRUE;
}




gboolean
daMFProfile_button_press_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	switch ( event->button ) {
	case 1:
		if ( event->state & GDK_MOD1_MASK ) {
			GtkWidget *f_chooser
				= gtk_file_chooser_dialog_new(
					"Export Profile Snapshot",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
			g_object_ref_sink( f_chooser);
			GtkFileFilter *file_filter = gtk_file_filter_new();
			g_object_ref_sink( file_filter);

			gtk_file_filter_set_name( file_filter, "SVG images");
			gtk_file_filter_add_pattern( file_filter, "*.svg");
			gtk_file_chooser_add_filter( (GtkFileChooser*)f_chooser, file_filter);
			if ( gtk_dialog_run( (GtkDialog*)f_chooser) == GTK_RESPONSE_ACCEPT ) {
				char *fname_ = gtk_file_chooser_get_filename( (GtkFileChooser*)f_chooser);
				snprintf_buf( "%s%s", fname_,
					      g_str_has_suffix( fname_, ".svg") ? "" : ".svg");
				g_free( fname_);
				cairo_surface_t *cs = cairo_svg_surface_create( global::buf, MF.da_wd, MF.da_ht);
				cairo_t *cr = cairo_create( cs);
				MF.draw_timeline( cr);
				cairo_destroy( cr);
				cairo_surface_destroy( cs);
			}
			g_object_unref( file_filter);
			g_object_unref( f_chooser);
			gtk_widget_destroy( f_chooser);
		} else {
			if ( MF.zoomed_episode == -1 ) {
				for ( int ep = MF.csimulation.mm_list().size()-1; ep > -1; --ep )
					if ( event->x/MF.da_wd * MF.csimulation.timeline().size() >
					     MF.csimulation.nth_episode_start_page( ep) ) {
						MF.zoomed_episode = ep;
						break;
					}
			} else
				MF.zoomed_episode = -1;
			gtk_widget_queue_draw( wid);
		}
		break;
	}

	return TRUE;
}





gboolean
daMFProfile_scroll_event_cb(
	GtkWidget *wid,
	GdkEventScroll *event,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	switch ( event->direction ) {
	case GDK_SCROLL_DOWN:
		MF.display_factor /= MF._p.scroll_factor;
	    break;
	case GDK_SCROLL_UP:
		MF.display_factor *= MF._p.scroll_factor;
	    break;
	case GDK_SCROLL_LEFT:
	    break;
	case GDK_SCROLL_RIGHT:
	    break;
	case GDK_SCROLL_SMOOTH: // wow
	    break;
	}

	// if ( event->state & GDK_CONTROL_MASK ) {
	// 	;
	// } else
	// 	;

	gtk_widget_queue_draw( wid);

	return TRUE;
}





namespace {
SModelrunFacility *this_mf = nullptr;
void this_mf_siman_param_printer(void *xp)
{
	this_mf -> siman_param_printer(xp);
}
}

void
bMFRun_clicked_cb(
	GtkButton*,
	gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	SBusyBlock bb (MF.wModelrunFacility);

	void (*fun)(void*) = (this_mf == nullptr)
		? (this_mf = &MF, this_mf_siman_param_printer)
		: nullptr;
	MF.csimulation.watch_simplex_move(
		gtk_toggle_button_get_active( (GtkToggleButton*)MF.eMFLiveUpdate)
		? fun : nullptr);
	this_mf = nullptr;

	MF.snapshot();
	MF.update_infobar();

	GtkTextIter iter;
	if ( not MF._tunables_header_printed ) {
		ostringstream ss;
		ss << "#";
		for ( size_t t = 0; t < MF.csimulation.tx.size(); ++t )
			ss << agh::str::sasprintf(
				"%s%s",
				agh::ach::tunable_name(t).c_str(),
				t < MF.csimulation.tx.size()-1 ? "\t" : "\n");
		gtk_text_buffer_insert(
			MF.log_text_buffer,
			(gtk_text_buffer_get_end_iter( MF.log_text_buffer, &iter), &iter),
			ss.str().c_str(), -1);
		MF._tunables_header_printed = true;
	}

	for ( size_t t = 0; t < MF.csimulation.tx.size(); ++t ) {
		auto tg = min( t, (size_t)agh::ach::TTunable::_basic_tunables-1);
		snprintf_buf(
			"%s%s",
			agh::str::sasprintf(
				agh::ach::stock[tg].fmt,
				MF.csimulation.tx[t] * agh::ach::stock[tg].display_scale_factor).c_str(),
			t < MF.csimulation.tx.size()-1 ? "\t" : "\n");
		gtk_text_buffer_insert(
			MF.log_text_buffer,
  			(gtk_text_buffer_get_end_iter( MF.log_text_buffer, &iter), &iter),
			global::buf, -1);
	}
	gtk_text_view_scroll_to_iter(
		MF.lMFLog,
		(gtk_text_buffer_get_end_iter( MF.log_text_buffer, &iter), &iter),
		0.1, FALSE,
		0., 1.);

	gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
}





void
eMFSmooth_value_changed_cb(
	GtkScaleButton *b,
	const gdouble v,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;
	MF.swa_smoothover = v;
	gtk_button_set_label(
		(GtkButton*)b,
		snprintf_buf( "Smooth: %zu", MF.swa_smoothover));
	if ( !MF._suppress_Vx_value_changed )
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
}



void
eMFHighlightNREM_toggled_cb(
	GtkCheckButton *e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	MF.highlight_nrem = gtk_toggle_button_get_active( (GtkToggleButton*)e);
	if ( !MF._suppress_Vx_value_changed )
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
}

void
eMFHighlightREM_toggled_cb(
	GtkCheckButton *e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	MF.highlight_rem = gtk_toggle_button_get_active( (GtkToggleButton*)e);
	if ( !MF._suppress_Vx_value_changed )
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
}

void
eMFHighlightWake_toggled_cb(
	GtkCheckButton *e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	MF.highlight_wake = gtk_toggle_button_get_active( (GtkToggleButton*)e);
	if ( !MF._suppress_Vx_value_changed )
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
}


void
bMFReset_clicked_cb(
	GtkButton*,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	MF.csimulation.tx.set_defaults();
	MF.update_infobar();

	gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
	gtk_text_buffer_set_text( MF.log_text_buffer, "", -1);
	MF._tunables_header_printed = false;
}



void
eMFClassicFit_toggled_cb(
	GtkCheckButton *b,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	if ( gtk_toggle_button_get_active( (GtkToggleButton*)b) == TRUE ) {
		const auto mm = MF.csimulation.mm_list().size();
		valarray<double>
			rr (mm);

		size_t i = 0;
		for ( auto& M : MF.csimulation.mm_list() ) {
			agh::beersma::SClassicFit borbely =
				agh::beersma::classic_fit(
					*M, { MF.csimulation.P(), .1, 40 });
			rr[i] = borbely.r;

			++i;
		}
		snprintf_buf(
			"avg r = %4g",
			rr.sum()/rr.size());
	} else
		snprintf_buf(
			"N/A");

	gtk_label_set_markup(
		MF.lMFClassicFit,
		global::buf);
}



void
bMFAccept_clicked_cb(
	GtkToolButton*,
	const gpointer userdata)
{
	auto& MF = *(SModelrunFacility*)userdata;

	if ( MF.csimulation.status & agh::ach::CModelRun::modrun_tried )
		MF._p.populate_2();

	delete &MF;
}




void
eMFVx_value_changed_cb(
	GtkSpinButton* e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	if ( !MF._suppress_Vx_value_changed ) {
		agh::ach::TTunable t = MF.eMFVx[e];
		if ( (size_t)t < MF.csimulation.tx.size() ) {
			MF.csimulation.tx[t] =
				gtk_spin_button_get_value(e)
				/ agh::ach::stock[min(t, agh::ach::TTunable::gc)].display_scale_factor;
			MF.snapshot();
			gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
		}
	}
}


void
eMFDB1_toggled_cb(
	GtkCheckButton* e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	if ( !MF._suppress_Vx_value_changed ) {
		MF.csimulation.ctl_params.DBAmendment1 =
			gtk_toggle_button_get_active( (GtkToggleButton*)e);
		MF.snapshot();
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
	}
}
void
eMFDB2_toggled_cb(
	GtkCheckButton* e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	if ( !MF._suppress_Vx_value_changed ) {
		MF.csimulation.ctl_params.DBAmendment2 =
			gtk_toggle_button_get_active( (GtkToggleButton*)e);
		MF.snapshot();
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
	}
}
void
eMFAZ1_toggled_cb(
	GtkCheckButton* e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	if ( !MF._suppress_Vx_value_changed ) {
		MF.csimulation.ctl_params.AZAmendment1 =
			gtk_toggle_button_get_active( (GtkToggleButton*)e);
		MF.snapshot();
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
	}
}
void
eMFAZ2_toggled_cb(
	GtkCheckButton* e,
	const gpointer u)
{
	auto& MF = *(SModelrunFacility*)u;
	if ( !MF._suppress_Vx_value_changed ) {
		MF.csimulation.ctl_params.AZAmendment2 =
			gtk_toggle_button_get_active( (GtkToggleButton*)e);
		MF.snapshot();
		gtk_widget_queue_draw( (GtkWidget*)MF.daMFProfile);
	}
}



gboolean
wModelrunFacility_delete_event_cb(
	GtkWidget*,
	GdkEvent*,
	const gpointer userdata)
{
	auto MFp = (SModelrunFacility*)userdata;
	delete MFp;
	return TRUE;
}

}  // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
