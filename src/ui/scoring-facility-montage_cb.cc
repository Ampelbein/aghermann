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


#include <sys/time.h>
#include <cairo/cairo.h>

#include "misc.hh"
#include "scoring-facility.hh"


using namespace std;
using namespace aghui;

extern "C" {

gboolean
daSFMontage_configure_event_cb( GtkWidget *widget,
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
daSFMontage_draw_cb( GtkWidget *wid, cairo_t *cr, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.draw_montage( cr);
	return TRUE;
}


static void
radio_item_setter( GtkWidget *i, gpointer u)
{
	const char *label = gtk_menu_item_get_label( (GtkMenuItem*)i);
	if ( strcmp(label, (const char*)u) == 0 )
		gtk_check_menu_item_set_active( (GtkCheckMenuItem*)i, TRUE);
}

gboolean
daSFMontage_button_press_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == aghui::SScoringFacility::TMode::showing_ics ) {
		if ( SF.ica_components.size() == 0 )
			return TRUE;

		SF.using_ic = SF.ic_near( event->y);

		if ( event->button == 1 &&
		     (SF.remix_mode == aghui::SScoringFacility::TICARemixMode::punch ||
		      SF.remix_mode == aghui::SScoringFacility::TICARemixMode::zero) ) {
			SF.ica_map[SF.using_ic].m =
				(SF.ica_map[SF.using_ic].m == -1) ? 0 : -1;
			gtk_widget_queue_draw( wid);
		} else if ( SF.remix_mode == aghui::SScoringFacility::TICARemixMode::map ) {
			const char *mapped =
				(SF.ica_map[SF.using_ic].m != -1)
				? SF.channel_by_idx( SF.ica_map[SF.using_ic].m) . name
				: aghui::SScoringFacility::ica_unmapped_menu_item_label;
			SF.suppress_redraw = true;
			gtk_container_foreach(
				(GtkContainer*)SF.mSFICAPage,
				radio_item_setter, (gpointer)mapped);
			SF.suppress_redraw = false;
			gtk_menu_popup( SF.mSFICAPage,
					NULL, NULL, NULL, NULL, 3, event->time);
		}
		return TRUE;
	}
	if ( SF.mode == aghui::SScoringFacility::TMode::showing_remixed ) {
		if ( SF.ica_components.size() == 0 )
			return TRUE;

		SF.using_channel = SF.channel_near( event->y);
		//SF.using_ic = SF.ic_of( SF.using_channel);

		if ( event->button == 1 ) {
			SF.using_channel->apply_reconstituted =
				!SF.using_channel->apply_reconstituted;
			gtk_widget_queue_draw( wid);
		}
		return TRUE;
	}

	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( Ch->type == sigfile::SChannel::TType::eeg &&
	     (Ch->draw_psd || Ch->draw_mc) && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			if ( event->state & GDK_MODIFIER_MASK )
				;
			else
				gtk_spin_button_set_value( SF.eSFCurrentPage,
							   (event->x / SF.da_wd) * SF.total_vpages() + 1);
			// will eventually call set_cur_vpage(), which will do redraw
		    break;
		case 2:
			Ch->draw_bands = !Ch->draw_bands;
			gtk_widget_queue_draw( wid);
		    break;
		case 3:
			Ch->update_power_check_menu_items();
			gtk_menu_popup( SF.mSFPower,
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		}

	} else if ( Ch->type == sigfile::SChannel::TType::emg &&
		    Ch->draw_emg && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			gtk_spin_button_set_value( SF.eSFCurrentPage,
						   (event->x / SF.da_wd) * SF.total_vpages() + 1);
		    break;
		default:
		    break;
		}

	} else {
		switch ( event->button ) {
		case 2:
			if ( event->state & GDK_CONTROL_MASK )
				for ( auto& H : SF.channels )
					H.signal_display_scale = SF.sane_signal_display_scale;
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
				gtk_menu_popup( agh::overlap( Ch->selection_start_time, Ch->selection_end_time,
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



inline double
timeval_elapsed( const struct timeval &x, const struct timeval &y)
{
	return y.tv_sec - x.tv_sec
		+ 1e-6 * (y.tv_usec - x.tv_usec);
}

gboolean
daSFMontage_motion_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == aghui::SScoringFacility::TMode::showing_ics )
		return TRUE;

	static struct timeval last_page_flip = {0, 0};
	if ( last_page_flip.tv_sec == 0 )
		gettimeofday( &last_page_flip, NULL);

	// update marquee boundaries
	if ( SF.mode == aghui::SScoringFacility::TMode::shuffling_channels ) {
		SF.using_channel->zeroy = SF.zeroy_before_shuffling + (event->y - SF.event_y_when_shuffling);
		gtk_widget_queue_draw( wid);

	} else if ( SF.mode == aghui::SScoringFacility::TMode::marking ) {
		if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
			return TRUE;
		SF.using_channel->marquee_mend = event->x;

		struct timeval currently;
		gettimeofday( &currently, NULL);
		if ( (int)event->x > SF.da_wd && SF.cur_vpage() < SF.total_vpages()-1 ) {
			if ( timeval_elapsed( last_page_flip, currently) > .4 ) {
				// x (1+2a) = y
				SF.using_channel->marquee_mstart -= SF.da_wd / (1. + 2*SF.skirting_run_per1);
				SF.set_cur_vpage( SF.cur_vpage()+1);
				gettimeofday( &last_page_flip, NULL);
			}
		} else if ( (int)event->x < 0 && SF.cur_vpage() > 0 ) {
			if ( timeval_elapsed( last_page_flip, currently) > .4 ) {
				SF.using_channel->marquee_mstart += SF.da_wd / (1. + 2*SF.skirting_run_per1);
				SF.set_cur_vpage( SF.cur_vpage()-1);
				gettimeofday( &last_page_flip, NULL);
			}
		}
		SF.using_channel->marquee_to_selection(); // to be sure, also do it on button_release
		if ( event->state & GDK_SHIFT_MASK )
			for( auto &H : SF.channels ) {
				H.marquee_mstart = SF.using_channel->marquee_mstart;
				H.marquee_mend = event->x;
				H.marquee_to_selection(); // to be sure, also do it on button_release
			}
		gtk_widget_queue_draw( wid);

	} else if ( SF.draw_crosshair ) {
		SF.crosshair_at = event->x;
		SF.crosshair_at_time = SF.time_at_click( event->x);
		gtk_widget_queue_draw( wid);
	}

	if ( SF.mode == aghui::SScoringFacility::TMode::scoring )
		gtk_label_set_text(
			SF.lSFOverChannel,
			SF.channel_near( event->y) -> name);
	else
		gtk_label_set_text( SF.lSFOverChannel, "");

	return TRUE;
}

gboolean
daSFMontage_leave_notify_event_cb( GtkWidget *wid, GdkEventMotion *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gtk_label_set_text( SF.lSFOverChannel, "");
	return TRUE;
}


gboolean
daSFMontage_button_release_event_cb( GtkWidget *wid, GdkEventButton *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == aghui::SScoringFacility::TMode::showing_ics ||
	     SF.mode == aghui::SScoringFacility::TMode::showing_remixed )
		return TRUE;

	auto Ch = SF.using_channel;

	if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
		 return TRUE;

	switch ( event->button ) {
	case 1:
		if ( SF.mode == aghui::SScoringFacility::TMode::marking ) {
			SF.mode = aghui::SScoringFacility::TMode::scoring;
			gtk_widget_queue_draw( wid);
			if ( fabs(SF.using_channel->marquee_mstart - SF.using_channel->marquee_mend) > 5 ) {
				gtk_menu_popup( SF.mSFPageSelection,
						NULL, NULL, NULL, NULL, 3, event->time);
			}
		} else if ( Ch->type == sigfile::SChannel::TType::eeg &&
			    (Ch->draw_psd || Ch->draw_mc) && event->y > Ch->zeroy )
			gtk_spin_button_set_value( SF.eSFCurrentPage,
						   (event->x / SF.da_wd) * SF.total_vpages()+1);
		else {
			SF.using_channel->marquee_to_selection();
			SF.mode = aghui::SScoringFacility::TMode::scoring;
			gtk_widget_queue_draw( wid);
		}
	    break;
	case 3:
	    break;
	}

	return TRUE;
}







gboolean
daSFMontage_scroll_event_cb( GtkWidget *wid, GdkEventScroll *event, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( (event->state & GDK_MOD1_MASK) and
	     not (event->state & GDK_SHIFT_MASK) ) {
		auto da_ht0 = SF.da_ht;
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			if ( SF.da_ht > (int)(SF.channels.size() - SF.n_hidden) * 20 ) {
				gtk_widget_set_size_request( (GtkWidget*)SF.daSFMontage,
							     -1, SF.da_ht -= 20);
				SF.expand_by_factor( (double)SF.da_ht / da_ht0);
				gtk_widget_queue_draw( wid);
			}
		    break;
		case GDK_SCROLL_UP:
			gtk_widget_set_size_request( (GtkWidget*)SF.daSFMontage,
						     -1, SF.da_ht += 20);
			SF.expand_by_factor( (double)SF.da_ht / da_ht0);
		    gtk_widget_queue_draw( wid);
		default:
		    break;
		}

	} else if ( Ch->type == sigfile::SChannel::TType::eeg
	     && event->y > Ch->zeroy
	     && (Ch->draw_psd || Ch->draw_mc) ) {
		if ( event->state & GDK_SHIFT_MASK && Ch->draw_psd )
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->draw_bands ) {
					if ( Ch->psd.focused_band != sigfile::TBand::delta ) {
						--Ch->psd.focused_band;
						gtk_widget_queue_draw( wid);
					}
				} else
					if ( Ch->psd.from > 0 ) {
						Ch->psd.from -= .5;
						Ch->psd.upto -= .5;
						Ch->get_psd_course( false);
						gtk_widget_queue_draw( wid);
					}
				break;
			case GDK_SCROLL_UP:
				if ( Ch->draw_bands ) {
					if ( Ch->psd.focused_band != Ch->psd.uppermost_band ) {
						++Ch->psd.focused_band;
						gtk_widget_queue_draw( wid);
					}
				} else
					if ( Ch->psd.upto < 18. ) {
						Ch->psd.from += .5;
						Ch->psd.upto += .5;
						Ch->get_psd_course( false);
						gtk_widget_queue_draw( wid);
					}
				break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
				break;
			}
		else if ( event->state & (GDK_SHIFT_MASK|GDK_MOD1_MASK) && Ch->draw_mc )
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->mc.bin > 0 ) {
					--Ch->mc.bin;
					Ch->get_mc_course( false);
					gtk_widget_queue_draw( wid);
				}
				break;
			case GDK_SCROLL_UP:
				if ( Ch->mc.bin < Ch->crecording.sigfile::SMCParamSet::compute_n_bins(
					     Ch->crecording.sigfile::CBinnedMC::samplerate()) - 1 ) {
					++Ch->mc.bin;
					Ch->get_mc_course( false);
					gtk_widget_queue_draw( wid);
				}
				break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
				break;
			}

		else
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				Ch->psd.display_scale /= 1.1;
				Ch->mc.display_scale /= 1.1;
				gtk_widget_queue_draw( wid);
			    break;
			case GDK_SCROLL_UP:
				Ch->psd.display_scale *= 1.1;
				Ch->mc.display_scale *= 1.1;
				gtk_widget_queue_draw( wid);
			    break;
			case GDK_SCROLL_LEFT:
				if ( SF.cur_vpage() > 0 )
					gtk_spin_button_set_value( SF.eSFCurrentPage,
								   SF.cur_vpage() - 1);
			    break;
			case GDK_SCROLL_RIGHT:
				if ( SF.cur_vpage() < SF.total_vpages() )
					gtk_spin_button_set_value( SF.eSFCurrentPage,
								   SF.cur_vpage() + 1);
			    break;
			}
	} else {
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
	}

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
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageShowProcessed_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_filtered_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	if ( !SF.using_channel->draw_filtered_signal && !SF.using_channel->draw_original_signal )
		gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
						(gboolean)(SF.using_channel->draw_original_signal = true));
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageUseResample_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->resample_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageDrawZeroline_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_zeroline = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
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
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageShowHidden_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = &SF[gtk_menu_item_get_label(menuitem)];
	Ch->hidden = false;

	SF.using_channel = Ch;
	gdk_window_get_device_position(
		gtk_widget_get_window( (GtkWidget*)SF.daSFMontage),
		__pointer,
		NULL, (int*)&Ch->zeroy, NULL); //SF.find_free_space();
	SF.zeroy_before_shuffling = Ch->zeroy;
	SF.event_y_when_shuffling = (double)Ch->zeroy;
	SF.mode = aghui::SScoringFacility::TMode::shuffling_channels;

	gtk_widget_destroy( (GtkWidget*)Ch->menu_item_when_hidden);
	Ch->menu_item_when_hidden = NULL;

	--SF.n_hidden;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageSpaceEvenly_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.space_evenly();
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageDrawPSDProfile_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_psd = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}
void
iSFPageDrawMCProfile_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_mc = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}
void
iSFPageDrawEMGProfile_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_emg = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageClearArtifacts_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( GTK_RESPONSE_YES != pop_question(
		     SF.wScoringFacility,
		     "All marked artifacts will be lost in channel <b>%s</b>.\n\n"
		     "Continue?",
		     SF.using_channel->name) )
		return;

	SF.using_channel->artifacts().clear();
	SF.using_channel->get_signal_filtered();

	if ( SF.using_channel->type == sigfile::SChannel::TType::eeg ) {
		SF.using_channel->get_psd_course( false);
		SF.using_channel->get_psd_in_bands( false);
	}

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
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
	string fname_base = r.F().filename();
	snprintf_buf( "%s-filt.tsv", fname_base.c_str());
	r.F().export_filtered( SF.using_channel->name, __buf__);
	snprintf_buf( "%s-orig.tsv", fname_base.c_str());
	r.F().export_original( SF.using_channel->h(), __buf__);
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
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
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
			SF.using_channel->annotations.remove(
				*SF.over_annotations.front());
	} else {
		if ( GTK_RESPONSE_YES
		     == pop_question( SF.wScoringFacility,
				      "Sure you want to delete <b>%zu annotations</b>?",
				      SF.over_annotations.size()) )
			for ( auto &rm : SF.over_annotations )
				SF.using_channel->annotations.remove( *rm);
	}
	SF._p.populate_mGlobalAnnotations();
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageAnnotationEdit_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	sigfile::SAnnotation *which =
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
			gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		}
	}
}



void
iSFICAPageMapIC_activate_cb( GtkRadioMenuItem* i, gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	if ( SF.suppress_redraw )
		return;
	const char *label = gtk_menu_item_get_label( (GtkMenuItem*)i);

      // find target h
	int target = -1;
	int h = 0;
	for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H, ++h )
		if ( strcmp( H->name, label) == 0 ) {
			target = h;
			break;
		}
	SF.ica_map[SF.using_ic].m = target;

      // remove any previous mapping of the same target
	h = 0;
	for ( h = 0; h < (int)SF.ica_map.size(); ++h )
		if ( SF.ica_map[h].m == target && h != SF.using_ic )
			SF.ica_map[h].m = -1;

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
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

	string fname_base;
	switch ( SF.using_channel->display_profile_type ) {
	case sigfile::TProfileType::Psd:
		fname_base = SF.using_channel->crecording.CBinnedPower::fname_base();
		snprintf_buf( "%s_%g-%g.tsv",
			      fname_base.c_str(), SF.using_channel->psd.from, SF.using_channel->psd.upto);
		SF.using_channel->crecording.CBinnedPower::export_tsv(
			SF.using_channel->psd.from, SF.using_channel->psd.upto,
			__buf__);
	    break;
	case sigfile::TProfileType::Mc:
		fname_base = SF.using_channel->crecording.CBinnedMC::fname_base();
		snprintf_buf( "%s.tsv",
			      fname_base.c_str());
		SF.using_channel->crecording.CBinnedMC::export_tsv(
			__buf__);
	    break;
	}

	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      homedir2tilda(fname_base).c_str(), SF.using_channel->psd.from, SF.using_channel->psd.upto);
	SF._p.buf_on_status_bar();
}

void
iSFPowerExportAll_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	string fname_base;
	switch ( SF.using_channel->display_profile_type ) {
	case sigfile::TProfileType::Psd:
		fname_base = SF.using_channel->crecording.CBinnedPower::fname_base();
		snprintf_buf( "%s.tsv",
			      fname_base.c_str());
		SF.using_channel->crecording.CBinnedPower::export_tsv(
			__buf__);
	    break;
	case sigfile::TProfileType::Mc:
		fname_base = SF.using_channel->crecording.CBinnedMC::fname_base();
		snprintf_buf( "%s.tsv",
			      fname_base.c_str());
		SF.using_channel->crecording.CBinnedMC::export_tsv(
			__buf__);
	    break;
	}

	snprintf_buf( "Wrote %s_%g-%g.tsv",
		      homedir2tilda(fname_base).c_str(), SF.using_channel->psd.from, SF.using_channel->psd.upto);
	SF._p.buf_on_status_bar();
}

void
iSFPowerSmooth_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->resample_power = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	SF.using_channel->get_psd_course(false);
	SF.using_channel->get_psd_in_bands(false);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPowerDrawBands_toggled_cb( GtkCheckMenuItem *checkmenuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_bands = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPowerUseThisScale_activate_cb( GtkMenuItem *menuitem, gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	SF.sane_psd_display_scale = SF.using_channel->psd.display_scale;
	SF.sane_mc_display_scale  = SF.using_channel->mc.display_scale;
	for ( auto& H : SF.channels ) {
		H.psd.display_scale = SF.sane_psd_display_scale;
		H.mc.display_scale = SF.sane_mc_display_scale;
	}
	SF.queue_redraw_all();
}


} // extern "C"

// eof
