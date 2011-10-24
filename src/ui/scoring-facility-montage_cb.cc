// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-montage_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-02
 *
 *         Purpose:  scoring facility: montage drawing area callbacks
 *
 *         License:  GPL
 */




#include <cairo/cairo.h>

#include "misc.hh"
#include "scoring-facility.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif




using namespace std;
using namespace aghui;

extern "C" {

gboolean
daScoringFacMontage_configure_event_cb( GtkWidget *widget,
					GdkEventConfigure *event,
					gpointer userdata)
{
	 if ( event->type == GDK_CONFIGURE ) {
		 auto& SF = *(SScoringFacility*)userdata;
		 SF.da_wd = event->width;
		 // don't care about height: it's our own calculation
	 }
	 return FALSE;
}




// -------------------- Page

gboolean
daScoringFacMontage_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.draw_montage( cr);
	return TRUE;
}

gboolean
daScoringFacMontage_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == aghui::SScoringFacility::TMode::doing_ica )
		return TRUE;

	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( strcmp( Ch->type, "EEG") == 0 && Ch->draw_power && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			if ( event->state & GDK_MODIFIER_MASK )
				;
			else
				gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
							   (event->x / SF.da_wd) * SF.total_vpages() + 1);
			// will eventually call set_cur_vpage(), which will do redraw
		    break;
		case 2:
			Ch->draw_bands = !Ch->draw_bands;
			gtk_widget_queue_draw( wid);
		    break;
		case 3:
			gtk_menu_popup( SF.mSFPower,
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		}

	} else if ( strcmp( Ch->type, "EMG") == 0 && Ch->draw_emg && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
						   (event->x / SF.da_wd) * SF.total_vpages() + 1);
		    break;
		default:
		    break;
		}

	} else {
		switch ( event->button ) {
		case 2:
			if ( event->state & GDK_CONTROL_MASK )
				for_each( SF.channels.begin(), SF.channels.end(),
					  [&SF] ( SScoringFacility::SChannel& H)
					  {
						  H.signal_display_scale = SF.sane_signal_display_scale;
					  });
			else
				Ch->signal_display_scale = SF.sane_signal_display_scale;
			gtk_widget_queue_draw( wid);
		    break;
		case 3:
			if ( event->state & GDK_MOD1_MASK && SF.n_hidden > 0 )
				gtk_menu_popup( SF.mSFPageHidden,
						NULL, NULL, NULL, NULL, 3, event->time);
			else {
				double cpos = SF.time_at_click( event->x);
				// hide ineffective items
				SF.using_channel->update_channel_check_menu_items();
				gtk_widget_set_visible( (GtkWidget*)SF.iSFPageHidden, SF.n_hidden > 0);
				bool over_any =
					not (SF.over_annotations = Ch->in_annotations( cpos)) . empty();
				gtk_widget_set_visible( (GtkWidget*)SF.mSFPageAnnotation, over_any);
				gtk_widget_set_visible( (GtkWidget*)SF.iSFPageAnnotationSeparator, over_any);
				gtk_menu_popup( overlap( Ch->selection_start_time, Ch->selection_end_time,
							 cpos, cpos) ? SF.mSFPageSelection : SF.mSFPage,
						NULL, NULL, NULL, NULL, 3, event->time);
			}
		    break;
		case 1:
			if ( event->state & GDK_MOD1_MASK ) {
				SF.event_y_when_shuffling = event->y;
				SF.zeroy_before_shuffling = Ch->zeroy;
				SF.mode = aghui::SScoringFacility::TMode::shuffling_channels;
			} else {
				SF.mode = aghui::SScoringFacility::TMode::marking;
				Ch->marquee_mstart = Ch->marquee_mend = event->x;
			}
			gtk_widget_queue_draw( wid);
		    break;
		}
	}
	return TRUE;
}





gboolean
daScoringFacMontage_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	// update marquee boundaries
	if ( SF.mode == aghui::SScoringFacility::TMode::shuffling_channels ) {
		SF.using_channel->zeroy = SF.zeroy_before_shuffling + (event->y - SF.event_y_when_shuffling);
		gtk_widget_queue_draw( wid);

	} else if ( SF.mode == aghui::SScoringFacility::TMode::marking ) {
		if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
			return TRUE;
		SF.using_channel->marquee_mend = event->x;
		SF.using_channel->marquee_to_selection(); // to be sure, also do it on button_release
		if ( event->state & GDK_SHIFT_MASK )
			for_each( SF.channels.begin(), SF.channels.end(),
				  [&] (SScoringFacility::SChannel& H)
				  {
					  H.marquee_mstart = SF.using_channel->marquee_mstart;
					  H.marquee_mend = event->x;
					  H.marquee_to_selection(); // to be sure, also do it on button_release
				  });
		gtk_widget_queue_draw( wid);

	} else if ( SF.draw_crosshair ) {
		SF.crosshair_at = event->x;
		SF.crosshair_at_time = SF.time_at_click( event->x);
		gtk_widget_queue_draw( wid);
	}

	return TRUE;
}



gboolean
daScoringFacMontage_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = SF.using_channel;

	if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
		 return TRUE;

	switch ( event->button ) {
	case 1:
		if ( SF.mode == aghui::SScoringFacility::TMode::marking
		     && fabs(SF.using_channel->marquee_mstart - SF.using_channel->marquee_mend) > 5 ) {
			gtk_widget_queue_draw( wid);
			gtk_menu_popup( SF.mSFPageSelection,
					NULL, NULL, NULL, NULL, 3, event->time);
		} else if ( strcmp( Ch->type, "EEG") == 0 && Ch->draw_power && event->y > Ch->zeroy )
			gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
						   (event->x / SF.da_wd) * SF.total_vpages()+1);
		else {
			SF.using_channel->marquee_to_selection();
			gtk_widget_queue_draw( wid);
		}
		SF.mode = aghui::SScoringFacility::TMode::scoring;
	    break;
	case 3:
	    break;
	}

	return TRUE;
}







gboolean
daScoringFacMontage_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( event->state & GDK_MOD1_MASK ) {
		auto da_ht0 = SF.da_ht;
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( SF.da_ht > (int)(SF.channels.size() - SF.n_hidden) * 20 ) {
				gtk_widget_set_size_request( (GtkWidget*)SF.daScoringFacMontage,
							     -1, SF.da_ht -= 20);
				SF.expand_by_factor( (double)SF.da_ht / da_ht0);
				gtk_widget_queue_draw( wid);
			}
		    break;
		case GDK_SCROLL_UP:
			gtk_widget_set_size_request( (GtkWidget*)SF.daScoringFacMontage,
						     -1, SF.da_ht += 20);
			SF.expand_by_factor( (double)SF.da_ht / da_ht0);
		    gtk_widget_queue_draw( wid);
		default:
		    break;
		}
		return TRUE;
	}

	if ( strcmp( Ch->type, "EEG") == 0 && Ch->draw_power && event->y > Ch->zeroy ) {
		if ( event->state & GDK_SHIFT_MASK ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->draw_bands ) {
					if ( Ch->focused_band != agh::TBand::delta ) {
						prev( Ch->focused_band);
						gtk_widget_queue_draw( wid);
					}
				} else
					if ( Ch->from > 0 ) {
						Ch->from = Ch->from - .5;
						Ch->upto = Ch->upto - .5;
						Ch->get_power();
						gtk_widget_queue_draw( wid);
					}
			    break;
			case GDK_SCROLL_UP:
				if ( Ch->draw_bands ) {
					if ( Ch->focused_band != Ch->uppermost_band ) {
						next( Ch->focused_band);
						gtk_widget_queue_draw( wid);
					}
				} else
					if ( Ch->upto < 18. ) {
						Ch->from = Ch->from + .5;
						Ch->upto = Ch->upto + .5;
						Ch->get_power();
						gtk_widget_queue_draw( wid);
					}
			    break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
			    break;
			}

		} else
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				Ch->power_display_scale /= 1.1;
				gtk_widget_queue_draw( wid);
			    break;
			case GDK_SCROLL_UP:
				Ch->power_display_scale *= 1.1;
				gtk_widget_queue_draw( wid);
			    break;
			case GDK_SCROLL_LEFT:
				if ( SF.cur_vpage() > 0 )
					gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
								   SF.cur_vpage() - 1);
			    break;
			case GDK_SCROLL_RIGHT:
				if ( SF.cur_vpage() < SF.total_vpages() )
					gtk_spin_button_set_value( SF.eScoringFacCurrentPage,
								   SF.cur_vpage() + 1);
			    break;
			}

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

	if ( event->state & GDK_CONTROL_MASK )
		for_each( SF.channels.begin(), SF.channels.end(),
			  [&] ( SScoringFacility::SChannel& H)
			  {
				  H.signal_display_scale = Ch->signal_display_scale;
			  });

	gtk_widget_queue_draw( wid);
	return TRUE;
}






// ------ menu callbacks

// -- Page

void
iSFPageShowOriginal_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_original_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	// prevent both being switched off
	if ( !SF.using_channel->draw_original_signal && !SF.using_channel->draw_filtered_signal )
		gtk_check_menu_item_set_active( SF.iSFPageShowProcessed,
						(gboolean)(SF.using_channel->draw_filtered_signal = true));
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_filtered_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	if ( !SF.using_channel->draw_filtered_signal && !SF.using_channel->draw_original_signal )
		gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
						(gboolean)(SF.using_channel->draw_original_signal = true));
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageUseResample_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->use_resample = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}

void
iSFPageDrawZeroline_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_zeroline = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}

void
iSFPageHide_activate_cb( GtkMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->hidden = true;
	// add an item to iSFPageHidden
	auto item = (GtkWidget*)(SF.using_channel->menu_item_when_hidden =
				 (GtkMenuItem*)gtk_menu_item_new_with_label( SF.using_channel->name));
	g_object_set( (GObject*)item,
		      "visible", TRUE,
		      NULL);
	g_signal_connect( (GObject*)item,
			  "activate", G_CALLBACK (iSFPageShowHidden_activate_cb),
			  &SF);
	gtk_container_add( (GtkContainer*)SF.mSFPageHidden,
			   item);
	++SF.n_hidden;
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageShowHidden_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = &SF[gtk_menu_item_get_label(menuitem)];
	Ch->hidden = false;

	SF.using_channel = Ch;
	gtk_widget_get_pointer( (GtkWidget*)SF.daScoringFacMontage,
				NULL, (int*)&Ch->zeroy); //SF.find_free_space();
	SF.zeroy_before_shuffling = Ch->zeroy;
	SF.event_y_when_shuffling = (double)Ch->zeroy;
	SF.mode = aghui::SScoringFacility::TMode::shuffling_channels;

	gtk_widget_destroy( (GtkWidget*)Ch->menu_item_when_hidden);
	Ch->menu_item_when_hidden = NULL;

	--SF.n_hidden;
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}

void
iSFPageSpaceEvenly_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.space_evenly();
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageDrawPSDProfile_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_power = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}
void
iSFPageDrawEMGProfile_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_emg = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( GTK_RESPONSE_YES != pop_question(
		     SF.wScoringFacility,
		     "All marked artifacts will be lost in this channel.  Continue?") )
		return;

	SF.using_channel->ssignal().artifacts.clear();
	SF.using_channel->get_signal_filtered();

	if ( strcmp( SF.using_channel->type, "EEG") == 0 ) {
		SF.using_channel->get_power();
		SF.using_channel->get_power_in_bands();
	}

	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}




void
iSFPageSaveAs_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& ED = SF._p;
	string j_dir = ED.ED->subject_dir( SF.using_channel->crecording.subject());
	snprintf_buf( "%s/%s/%s-p%zu@%zu.svg", j_dir.c_str(), ED.AghD(), ED.AghT(), SF.cur_vpage(), SF.vpagesize());
	string fname {__buf__};

	SF.using_channel->draw_page( fname.c_str(), SF.da_wd, SF.interchannel_gap);
	snprintf_buf( "Wrote \"%s\"",
		      homedir2tilda(fname).c_str());
	gtk_statusbar_pop( SF.sbSF, SF._p.sbContextIdGeneral);
	gtk_statusbar_push( SF.sbSF, SF._p.sbContextIdGeneral, __buf__);
}


void
iSFPageExportSignal_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& r = SF.using_channel->crecording;
	string fname_base = r.fname_base();
	snprintf_buf( "%s-orig.tsv", fname_base.c_str());
	r.F().export_original( SF.using_channel->name, __buf__);
	snprintf_buf( "%s-filt.tsv", fname_base.c_str());
	r.F().export_filtered( SF.using_channel->name, __buf__);
	snprintf_buf( "Wrote \"%s-{filt,orig}.tsv\"",
		      fname_base.c_str());
	gtk_statusbar_pop( SF.sbSF, SF._p.sbContextIdGeneral);
	gtk_statusbar_push( SF.sbSF, SF._p.sbContextIdGeneral, __buf__);
}



void
iSFPageUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.sane_signal_display_scale = SF.using_channel->signal_display_scale;
	for_each( SF.channels.begin(), SF.channels.end(),
		  [&] ( SScoringFacility::SChannel& H)
		  {
			  H.signal_display_scale = SF.sane_signal_display_scale;
		  });
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageAnnotationDelete_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.over_annotations.size() == 1 ) {
		if ( GTK_RESPONSE_YES
		     == pop_question( SF.wScoringFacility,
				      "Sure you want to delete annotation\n <b>%s</b>?",
				      SF.over_annotations.front()->label.c_str()) )
			SF.using_channel->crecording.F()[SF.using_channel->name].annotations.remove(
				*SF.over_annotations.front());
	} else {
		if ( GTK_RESPONSE_YES
		     == pop_question( SF.wScoringFacility,
				      "Sure you want to delete <b>%zu annotations</b>?",
				      SF.over_annotations.size()) )
			for_each( SF.over_annotations.begin(), SF.over_annotations.end(),
				  [&] ( agh::CEDFFile::SSignal::SAnnotation*& rm)
				  {
					  SF.using_channel->crecording.F()[SF.using_channel->name].annotations.remove( *rm);
				  });
	}
	SF._p.populate_mGlobalAnnotations();
	gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
}


void
iSFPageAnnotationEdit_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	agh::CEDFFile::SSignal::SAnnotation *which =
		(SF.over_annotations.size() == 1)
		? SF.over_annotations.front()
		: SF.interactively_choose_annotation();
	if ( which == NULL )
		return;

	gtk_entry_set_text( SF.eAnnotationLabel, which->label.c_str());
	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( SF.wAnnotationLabel) ) {
		const char* new_label = gtk_entry_get_text( SF.eAnnotationLabel);
		if ( strlen(new_label) > 0 ) {
			which->label = new_label;
			SF._p.populate_mGlobalAnnotations();
			gtk_widget_queue_draw( (GtkWidget*)SF.daScoringFacMontage);
		}
	}
}



// page selection
void
iSFPageSelectionMarkArtifact_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.using_channel->selection_size() > 5 )
		SF.using_channel->mark_region_as_artifact( true);
}

void
iSFPageSelectionClearArtifact_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.using_channel->selection_size() > 5 )
		SF.using_channel->mark_region_as_artifact( false);
}

void
iSFPageSelectionFindPattern_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.using_channel->selection_size() > 5 )
		SF.using_channel->mark_region_as_pattern();
}

void
iSFPageSelectionAnnotate_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gtk_entry_set_text( SF.eAnnotationLabel, "");
	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( (GtkDialog*)SF.wAnnotationLabel) ) {
		SF.using_channel->mark_region_as_annotation(
			gtk_entry_get_text( SF.eAnnotationLabel));
		SF._p.populate_mGlobalAnnotations();
	}
}






// power
void
iSFPowerExportRange_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	string fname_base = SF.using_channel->crecording.fname_base();
	snprintf_buf( "%s_%g-%g.tsv",
		      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
	SF.using_channel->crecording.export_tsv( SF.using_channel->from, SF.using_channel->upto,
						__buf__);
	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      homedir2tilda(fname_base).c_str(), SF.using_channel->from, SF.using_channel->upto);
	SF._p.buf_on_status_bar();
}

void
iSFPowerExportAll_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	string fname_base = SF.using_channel->crecording.fname_base();
	snprintf_buf( "%s_%g-%g.tsv",
		      fname_base.c_str(), SF.using_channel->from, SF.using_channel->upto);
	SF.using_channel->crecording.export_tsv( __buf__);
	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      homedir2tilda(fname_base).c_str(), SF.using_channel->from, SF.using_channel->upto);
	SF._p.buf_on_status_bar();
}

void
iSFPowerUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.sane_power_display_scale = SF.using_channel->power_display_scale;
	for_each( SF.channels.begin(), SF.channels.end(),
		  [&] ( SScoringFacility::SChannel& H)
		  {
			  H.power_display_scale = SF.sane_power_display_scale;
		  });
	SF.queue_redraw_all();
}


} // extern "C"

// eof
