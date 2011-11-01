// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-patterns_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-03
 *
 *         Purpose:  scoring facility patterns
 *
 *         License:  GPL
 */




#include "misc.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;


using namespace aghui;

extern "C" {

	gboolean
	daPatternSelection_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		FD.acquire_parameters();

		FD.draw( cr);

		return TRUE;
	}




	gboolean
	daPatternSelection_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;

		switch ( event->direction ) {
		case GDK_SCROLL_UP:
			FD.display_scale *= 1.1;
		    break;
		case GDK_SCROLL_DOWN:
			FD.display_scale /= 1.1;
		    break;
		default:
		    break;
		}

		gtk_widget_queue_draw( wid);

		return TRUE;
	}



	void
	bPatternFind_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		gboolean
			go_forward = button == FD.bPatternFindNext;

		size_t from;
		if ( FD.last_find == (size_t)-1 )
			from = go_forward
				? FD.context_before
				: FD.field_channel->n_samples() - FD.pattern.size();
		else
			from = FD.last_find + (go_forward ? 10 : -10);

		set_cursor_busy( true, (GtkWidget*)FD.wPattern);
		FD.search( from);
		if ( FD.last_find == (size_t)-1 )
			pop_ok_message( (GtkWindow*)FD.wPattern, "Not found");
		else { // reach up and out
			auto& SF = FD.field_channel->_p;
			SF.set_cur_vpage(
				FD.last_find / FD.samplerate / SF.vpagesize());
			SF.using_channel = FD.field_channel;
			SF.using_channel->put_selection( FD.last_find, FD.last_find + FD.pattern_size_essential());
			SF.queue_redraw_all();
			//SF.using_channel = NULL;

			snprintf_buf( "at p. %zu (a = %4.2f, b = %4.2f, c = %4.2f)\n",
				      SF.cur_vpage()+1, FD.match_a, FD.match_b, FD.match_c);
			gtk_label_set_markup( FD.lPatternSimilarity, __buf__);

			gtk_widget_queue_draw( (GtkWidget*)FD.lPatternSimilarity);
			gtk_widget_queue_draw( (GtkWidget*)FD.daPatternSelection);
		}

		set_cursor_busy( false, (GtkWidget*)FD.wPattern);
	}





	void
	bPatternSave_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		const char *label = gtk_combo_box_get_active_id( FD.ePatternList);
		if ( label ) {
			if ( strncmp( label, FD.globally_marker, strlen( FD.globally_marker)) == 0 )
				label += strlen( FD.globally_marker);
			gtk_entry_set_text( FD.ePatternNameName, label);
		}
		if ( gtk_dialog_run( FD.wPatternName) == GTK_RESPONSE_OK ) {
			const char *label = gtk_entry_get_text( FD.ePatternNameName);
			gboolean do_globally = gtk_toggle_button_get_active( (GtkToggleButton*)FD.ePatternNameSaveGlobally);
			FD.save_pattern( label, do_globally);

			// add to dropdown list & select the newly added entry
			FD.enumerate_patterns_to_combo();
			g_signal_handler_block( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
			FD.preselect_entry( label, do_globally);
			g_signal_handler_unblock( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
		}
	}


	void
	bPatternDiscard_clicked_cb( GtkButton *button, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( FD.ePatternList, &iter) == FALSE )
			return;
		char *label;
		gtk_tree_model_get( (GtkTreeModel*)FD.mPatterns, &iter,
				    0, &label,
				    -1);
		gboolean do_globally = strncmp( label, FD.globally_marker,
						strlen( FD.globally_marker)) == 0;
		char *fname = do_globally
			? label + strlen( FD.globally_marker)
			: label;
		FD.discard_pattern( fname, do_globally);
		free( label);
		g_signal_handler_block( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
		FD.preselect_entry( NULL, do_globally);
		g_signal_handler_unblock( FD.ePatternList, FD.ePatternList_changed_cb_handler_id);
	}


	void
	ePatternList_changed_cb( GtkComboBox *combo, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( combo, &iter) == FALSE )
			return;
		char *label;
		gtk_tree_model_get( (GtkTreeModel*)FD.mPatterns, &iter,
				    0, &label,
				    -1);
		gboolean do_globally = strncmp( label, FD.globally_marker, strlen( FD.globally_marker)) == 0;
		char *fname = do_globally
			? label + strlen( FD.globally_marker)
			: label;
		FD.load_pattern( fname, do_globally);
		free( label);

		gtk_label_set_markup( FD.lPatternSimilarity, "");

		gtk_widget_queue_draw( (GtkWidget*)FD.daPatternSelection);
	}


	void
	ePatternChannel_changed_cb( GtkComboBox *combo, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		GtkTreeIter iter;
		if ( gtk_combo_box_get_active_iter( combo, &iter) == FALSE )
			return;

		char *label;
		gtk_tree_model_get( gtk_combo_box_get_model( combo), &iter,
				    0, &label,
				    -1);
		auto& SF = FD.field_channel->_p;
		for ( auto &H : SF.channels ) {
			if ( strcmp( H.name, label) == 0 ) {
				FD.field_channel = SF.using_channel = &H;
				break;
			}
		}
		free( label);
	}

	void
	ePattern_any_value_changed_cb( GtkSpinButton *spinbutton, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
//		FD.acquire_parameters();
		gtk_widget_queue_draw( (GtkWidget*)FD.daPatternSelection);
	}


	void
	wPattern_show_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		FD.update_displayed_parameters();
		FD.enumerate_patterns_to_combo();

		if ( FD._p.using_channel == NULL ) // not invoked for a preselected signal via a menu
			FD._p.using_channel = &FD._p.channels.front();
		FD.field_channel = FD._p.using_channel;
		FD.samplerate = FD.field_channel->samplerate();
		FD.preselect_channel( FD.field_channel->name);
	}

	void
	wPattern_hide_cb( GtkWidget *widget, gpointer userdata)
	{
		auto& FD = *(SScoringFacility::SFindDialog*)userdata;
		gtk_toggle_button_set_active( (GtkToggleButton*)FD.field_channel->_p.bSFShowFindDialog, FALSE);
	}


} // extern "C"
