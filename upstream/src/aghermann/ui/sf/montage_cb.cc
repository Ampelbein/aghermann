/*
 *       File name:  aghermann/ui/sf/montage_cb.cc
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

#include "aghermann/ui/misc.hh"

#include "sf.hh"
#include "sf_cb.hh"
#include "d/artifacts.hh"
#include "d/artifacts-simple.hh"
#include "d/filters.hh"


using namespace std;
using namespace agh;
using namespace agh::ui;

extern "C" {

gboolean
daSFMontage_configure_event_cb(
	GtkWidget*,
	GdkEventConfigure *event,
	const gpointer userdata)
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
daSFMontage_draw_cb(
	GtkWidget*,
	cairo_t *cr,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.draw_montage( cr);
	return TRUE;
}


namespace {
void
radio_item_setter( GtkWidget *i, const gpointer u)
{
	const char *label = gtk_menu_item_get_label( (GtkMenuItem*)i);
	if ( strcmp(label, (const char*)u) == 0 )
		gtk_check_menu_item_set_active( (GtkCheckMenuItem*)i, TRUE);
}
} // namespace


gboolean
daSFMontage_button_press_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( SF.mode == SScoringFacility::TMode::showing_ics ) {
		if ( SF.ica_components.size() == 0 )
			return TRUE;

		SF.using_ic = SF.ic_near( event->y);

		if ( event->button == 1 &&
		     (SF.remix_mode == SScoringFacility::TICARemixMode::punch ||
		      SF.remix_mode == SScoringFacility::TICARemixMode::zero) ) {
			SF.ica_map[SF.using_ic].m =
				(SF.ica_map[SF.using_ic].m == -1) ? 0 : -1;
			gtk_widget_queue_draw( wid);
		} else if ( SF.remix_mode == SScoringFacility::TICARemixMode::map ) {
			const char *mapped =
				(SF.ica_map[SF.using_ic].m != -1)
				? SF.channel_by_idx( SF.ica_map[SF.using_ic].m) . name()
				: SScoringFacility::ica_unmapped_menu_item_label;
			SF.suppress_redraw = true;
			gtk_container_foreach(
				(GtkContainer*)SF.iiSFICAPage,
				radio_item_setter, (gpointer)mapped);
			SF.suppress_redraw = false;
			gtk_menu_popup( SF.iiSFICAPage,
					NULL, NULL, NULL, NULL, 3, event->time);
		}
		return TRUE;
	}

	if ( SF.mode == SScoringFacility::TMode::showing_remixed ) {
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

	if ( SF.mode == SScoringFacility::TMode::shuffling_channels ) {
		SF.mode = SScoringFacility::TMode::scoring;
		return TRUE;
	}

	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( Ch->schannel().type() == sigfile::SChannel::TType::eeg &&
	     (Ch->draw_psd || Ch->draw_mc) && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			if ( event->state & GDK_MODIFIER_MASK )
				;
			else
				SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
			// will eventually call set_cur_vpage(), which will do redraw
		    break;
		case 2:
			Ch->draw_psd_bands = !Ch->draw_psd_bands;
			gtk_widget_queue_draw( wid);
		    break;
		case 3:
			Ch->update_power_menu_items();
			gtk_menu_popup( SF.iiSFPower,
					NULL, NULL, NULL, NULL, 3, event->time);
		    break;
		}

	} else if ( Ch->schannel().type() == sigfile::SChannel::TType::emg &&
		    Ch->draw_emg && event->y > Ch->zeroy ) {
		switch ( event->button ) {
		case 1:
			SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());
		    break;
		default:
		    break;
		}

	} else {
		double cpos = SF.time_at_click( event->x);
		bool in_selection =
			agh::alg::overlap(
				Ch->selection_start_time, Ch->selection_end_time,
				cpos, cpos);

		switch ( event->button ) {
		case 2:
			Ch->signal_display_scale =
				agh::alg::calibrate_display_scale(
					Ch->draw_filtered_signal ? Ch->signal_filtered : Ch->signal_original,
					SF.vpagesize() * Ch->samplerate() * min (Ch->crecording.total_pages(), (size_t)10),
					SF.interchannel_gap / 2);
			if ( event->state & GDK_CONTROL_MASK )
				for ( auto& H : SF.channels )
					H.signal_display_scale = Ch->signal_display_scale;

			gtk_widget_queue_draw( wid);
		    break;

		case 3:
			if ( (event->state & GDK_MOD1_MASK && SF.n_hidden > 0) ||
			     !(SF.n_hidden < (int)SF.channels.size()) )
				gtk_menu_popup( SF.iiSFPageHidden,
						NULL, NULL, NULL, NULL, 3, event->time);
			else {
				Ch->update_channel_menu_items( event->x);
				Ch->update_power_menu_items();
				gtk_menu_popup(
					in_selection ? SF.iiSFPageSelection : SF.iiSFPage,
					NULL, NULL, NULL, NULL, 3, event->time);
			}
		    break;

		case 1:
			if ( event->state & GDK_MOD1_MASK ) {
				if ( in_selection ) {
					SF.moving_selection_handle_offset =
						cpos - Ch->selection_start_time;
					SF.mode = SScoringFacility::TMode::moving_selection;
				} else {
					SF.event_y_when_shuffling = event->y;
					SF.zeroy_before_shuffling = Ch->zeroy;
					SF.mode = SScoringFacility::TMode::shuffling_channels;
				}
			} else {
				SF.mode = SScoringFacility::TMode::marking;
				Ch->marquee_mstart = Ch->marquee_mend = event->x;
			}
			gtk_widget_queue_draw( wid);
		    break;
		}
	}
	return TRUE;
}



namespace {
inline double
timeval_elapsed( const struct timeval &x, const struct timeval &y)
{
	return y.tv_sec - x.tv_sec
		+ 1e-6 * (y.tv_usec - x.tv_usec);
}
}

gboolean
daSFMontage_motion_notify_event_cb(
	GtkWidget *wid,
	GdkEventMotion *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == SScoringFacility::TMode::showing_ics )
		return TRUE;

	static struct timeval last_page_flip = {0, 0};
	if ( last_page_flip.tv_sec == 0 )
		gettimeofday( &last_page_flip, NULL);

	switch ( SF.mode ) {

	case SScoringFacility::TMode::shuffling_channels:
	{
		SF.using_channel->zeroy = SF.zeroy_before_shuffling + (event->y - SF.event_y_when_shuffling);
		gtk_widget_queue_draw( wid);
	}
	break;

	case SScoringFacility::TMode::marking:
	{
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
			for( auto &H : SF.channels )
				if ( &H != SF.using_channel ) {
					H.marquee_mstart = SF.using_channel->marquee_mstart;
					H.marquee_mend = event->x;
					H.marquee_to_selection();
				}
		gtk_widget_queue_draw( wid);
	}
	break;

	case SScoringFacility::TMode::moving_selection:
	{
		auto	new_start_time = SF.time_at_click( event->x) - SF.moving_selection_handle_offset,
			new_end_time = new_start_time + (SF.using_channel->selection_end_time - SF.using_channel->selection_start_time);
		auto& H = *SF.using_channel;
		// reposition marquee
		H.marquee_mstart =
			(new_start_time - SF.cur_xvpage_start()) / SF.xvpagesize() * SF.da_wd;
		H.marquee_mend =
			(new_end_time - SF.cur_xvpage_start()) / SF.xvpagesize() * SF.da_wd;

		H.marquee_to_selection(); // to be sure, also do it on button_release
		H.put_selection( H.selection_start, H.selection_end);

		gtk_widget_queue_draw( wid);
	}
	break;

	default:
	break;
	}

	if ( SF.draw_crosshair ) {
		SF.crosshair_at = event->x;
		SF.crosshair_at_time = SF.time_at_click( event->x);
		gtk_widget_queue_draw( wid);
	}

	if ( SF.mode == SScoringFacility::TMode::scoring ) {
		gtk_label_set_text(
			SF.lSFOverChannel,
			SF.channel_near( event->y) -> name());
	} else
		gtk_label_set_text( SF.lSFOverChannel, "");

      // current pos
	SF.draw_current_pos( event->x);

	return TRUE;
}


gboolean
daSFMontage_leave_notify_event_cb(
	GtkWidget*,
	GdkEventMotion *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	gtk_label_set_text( SF.lSFOverChannel, "");
	SF.draw_current_pos( NAN);
	return TRUE;
}


gboolean
daSFMontage_button_release_event_cb(
	GtkWidget *wid,
	GdkEventButton *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.mode == SScoringFacility::TMode::showing_ics ||
	     SF.mode == SScoringFacility::TMode::showing_remixed )
		return TRUE;

	auto Ch = SF.using_channel;

	if ( SF.channel_near( event->y) != SF.using_channel ) // user has dragged too much vertically
		 return TRUE;

	switch ( event->button ) {
	case 1:
		if ( SF.mode == SScoringFacility::TMode::marking ) {
			SF.mode = SScoringFacility::TMode::scoring;
			Ch->put_selection( Ch->selection_start, Ch->selection_end);
			Ch->selectively_enable_selection_menu_items();
			Ch->update_channel_menu_items( event->x);
			if ( fabs(SF.using_channel->marquee_mstart - SF.using_channel->marquee_mend) > 5 ) {
				gtk_menu_popup( SF.iiSFPageSelection,
						NULL, NULL, NULL, NULL, 3, event->time);
			}
			gtk_widget_queue_draw( wid);

		} else if ( Ch->schannel().type() == sigfile::SChannel::TType::eeg &&
			    (Ch->draw_psd || Ch->draw_mc) && event->y > Ch->zeroy )
			SF.set_cur_vpage( (event->x / SF.da_wd) * SF.total_vpages());

		else {
			SF.using_channel->marquee_to_selection();
			SF.mode = SScoringFacility::TMode::scoring;
			gtk_widget_queue_draw( wid);
		}
	    break;
	case 3:
	    break;
	}

	return TRUE;
}





gboolean
daSFMontage_scroll_event_cb(
	GtkWidget *wid,
	GdkEventScroll *event,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = SF.using_channel = SF.channel_near( event->y);

	if ( (event->state & GDK_MOD1_MASK) and
	     not (event->state & GDK_SHIFT_MASK) ) {
		switch ( event->direction ) {
		case GDK_SCROLL_UP:
			if ( SF.da_ht > (int)(SF.channels.size() - SF.n_hidden) * 20 ) {
				SF.expand_by_factor( (double)(SF.da_ht - 10)/ SF.da_ht);
				gtk_widget_queue_draw( wid);
			}
		    break;
		case GDK_SCROLL_DOWN:
			SF.expand_by_factor( (double)(SF.da_ht + 10) / SF.da_ht);
			gtk_widget_queue_draw( wid);
		    break;
		default:
		    break;
		}

	} else if ( event->y > Ch->zeroy ) {
		if ( event->state & GDK_SHIFT_MASK && Ch->draw_psd ) {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->draw_psd_bands ) {
					if ( Ch->psd.focused_band > metrics::TBand::delta ) {
						--Ch->psd.focused_band;
						if ( Ch->autoscale_profile )
							Ch->update_profile_display_scales();
						gtk_widget_queue_draw( wid);
					}
				} else
					if ( Ch->psd.from > 0 ) {
						Ch->psd.from -= .5;
						Ch->psd.upto -= .5;
						Ch->get_psd_course();
						if ( Ch->autoscale_profile )
							Ch->update_profile_display_scales();
						gtk_widget_queue_draw( wid);
					}
				break;
			case GDK_SCROLL_UP:
				if ( Ch->draw_psd_bands ) {
					if ( Ch->psd.focused_band < Ch->psd.uppermost_band ) {
						++Ch->psd.focused_band;
						if ( Ch->autoscale_profile )
							Ch->update_profile_display_scales();
						gtk_widget_queue_draw( wid);
					}
				} else {
					auto& R = Ch->crecording;
					if ( Ch->psd.upto < R.psd_profile.Pp.binsize * R.psd_profile.bins() ) {
						Ch->psd.from += .5;
						Ch->psd.upto += .5;
						Ch->get_psd_course();
						if ( Ch->autoscale_profile )
							Ch->update_profile_display_scales();
						gtk_widget_queue_draw( wid);
					}
				}
				break;
			case GDK_SCROLL_LEFT:
			case GDK_SCROLL_RIGHT:
			default:
				break;
			}
		} else {
			switch ( event->direction ) {
			case GDK_SCROLL_DOWN:
				if ( Ch->draw_psd )
					Ch->psd.display_scale /= SF._p.scroll_factor;
				if ( Ch->draw_swu )
					Ch->swu.display_scale /= SF._p.scroll_factor;
				if ( Ch->draw_mc )
					Ch->mc.display_scale  /= SF._p.scroll_factor;
			    break;
			case GDK_SCROLL_UP:
				if ( Ch->draw_psd )
					Ch->psd.display_scale *= SF._p.scroll_factor;
				if ( Ch->draw_swu )
					Ch->swu.display_scale *= SF._p.scroll_factor;
				if ( Ch->draw_mc )
					Ch->mc.display_scale  *= SF._p.scroll_factor;
			    break;
			case GDK_SCROLL_LEFT:
				if ( SF.cur_vpage() > 0 )
					SF.set_cur_vpage( SF.cur_vpage() - 1);
			case GDK_SCROLL_RIGHT:
				if ( SF.cur_vpage() < SF.total_vpages() )
					SF.set_cur_vpage( SF.cur_vpage() + 1);
			    break;
			default:
			    break;
			}
			if ( event->state & GDK_CONTROL_MASK )
				for ( auto& H : SF.channels ) {
					if ( Ch->schannel().type() == sigfile::SChannel::TType::eeg &&
					     H.schannel().type() == sigfile::SChannel::TType::eeg ) {
						H.psd.display_scale = Ch->psd.display_scale;
						H.mc.display_scale  = Ch->mc.display_scale;
						H.swu.display_scale = Ch->swu.display_scale;
					} else if ( Ch->schannel().type() == sigfile::SChannel::TType::emg &&
					     H.schannel().type() == sigfile::SChannel::TType::emg )
						H.signal_display_scale = Ch->signal_display_scale;
				}
			gtk_widget_queue_draw( wid);
		}
	} else {
		switch ( event->direction ) {
		case GDK_SCROLL_DOWN:
			Ch->signal_display_scale /= SF._p.scroll_factor;
			break;
		case GDK_SCROLL_UP:
			Ch->signal_display_scale *= SF._p.scroll_factor;
			break;
		default:
			break;
		}

		if ( event->state & GDK_CONTROL_MASK )
			for ( auto& H : SF.channels )
				H.signal_display_scale = Ch->signal_display_scale;
		gtk_widget_queue_draw( wid);
	}

	return TRUE;
}





// ------ menu callbacks

// -- Page

void
iSFPageShowOriginal_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_original_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	// prevent both being switched off
	if ( !SF.using_channel->draw_original_signal && !SF.using_channel->draw_filtered_signal )
		gtk_check_menu_item_set_active( SF.iSFPageShowProcessed,
						(gboolean)(SF.using_channel->draw_filtered_signal = true));
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageShowProcessed_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_filtered_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	if ( !SF.using_channel->draw_filtered_signal && !SF.using_channel->draw_original_signal )
		gtk_check_menu_item_set_active( SF.iSFPageShowOriginal,
						(gboolean)(SF.using_channel->draw_original_signal = true));
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageUseResample_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->resample_signal = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageDrawZeroline_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_zeroline = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageHide_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->hidden = true;
	// add an item to iSFPageHidden
	auto item = (GtkWidget*)(SF.using_channel->menu_item_when_hidden =
				 (GtkMenuItem*)gtk_menu_item_new_with_label( SF.using_channel->name()));
	g_object_set( (GObject*)item,
		      "visible", TRUE,
		      NULL);
	g_signal_connect( (GObject*)item,
			  "activate", (GCallback)iSFPageShowHidden_activate_cb,
			  &SF);
	gtk_container_add( (GtkContainer*)SF.iiSFPageHidden,
			   item);
	++SF.n_hidden;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageShowHidden_activate_cb(
	GtkMenuItem *menuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto Ch = &SF[gtk_menu_item_get_label(menuitem)];
	Ch->hidden = false;

	SF.using_channel = Ch;
	gdk_window_get_device_position(
		gtk_widget_get_window( (GtkWidget*)SF.daSFMontage),
		global::client_pointer,
		NULL, (int*)&Ch->zeroy, NULL); //SF.find_free_space();
	SF.zeroy_before_shuffling = Ch->zeroy;
	SF.event_y_when_shuffling = (double)Ch->zeroy;
	SF.mode = SScoringFacility::TMode::shuffling_channels;

	gtk_widget_destroy( (GtkWidget*)Ch->menu_item_when_hidden);
	Ch->menu_item_when_hidden = NULL;

	--SF.n_hidden;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageSpaceEvenly_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.space_evenly();
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageLocateSelection_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.using_channel->selection_start == SF.using_channel->selection_end ) {
		SF.sb_message( "There is no selection in this channel");
	} else
		SF.set_cur_vpage(
			SF.using_channel->selection_start_time / SF.vpagesize());
}


void
iSFPageDrawPSDProfile_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_psd = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawPSDSpectrum_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_spectrum = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawMCProfile_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_mc = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawSWUProfile_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_swu = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawEMGProfile_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_emg = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawPhasicSpindles_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_phasic_spindle = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawPhasicKComplexes_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_phasic_Kcomplex = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageDrawPhasicEyeBlinks_toggled_cb(
	GtkCheckMenuItem *checkmenuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_phasic_eyeblink = (bool)gtk_check_menu_item_get_active( checkmenuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageFilter_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& FD =  SF.filters_d();
	auto& H  = *SF.using_channel;
	FD.P = H.filters;
	FD.W_V.up();

	gtk_label_set_markup(
		FD.lSFFilterCaption,
		snprintf_buf(
			"<big>Filters for channel <b>%s</b></big>",
			SF.using_channel->name()));

	if ( gtk_dialog_run( FD.wSFFilters) == GTK_RESPONSE_OK ) {
		FD.W_V.down();
		H.filters = FD.P;
		H.get_signal_filtered();

		if ( H.schannel().type() == sigfile::SChannel::TType::eeg ) {
			H.get_psd_course();
			H.get_psd_in_bands();
			H.get_spectrum( SF.cur_page());
			H.get_mc_course();
		}
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);

		if ( strcmp( SF.using_channel->name(), SF._p.AghH()) == 0 )
			SF.redraw_ssubject_timeline();
	}
}


void
iSFPageArtifactsDetect_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AD = SF.artifacts_d();

	gtk_widget_show( (GtkWidget*)AD.wSFAD);
}

void
iSFPageArtifactsMarkFlat_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& AS = SF.artifacts_simple_d();
	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( (GtkDialog*)AS.wSFADS) ) {
		AS.W_V.down();

		auto marked = SF.using_channel->mark_flat_regions_as_artifacts( AS.min_size, AS.pad);

		SF.sb_message(
			snprintf_buf(
				"Detected %.2g sec of flat regions, adding %.2g sec to already marked",
				marked.first, marked.second));

		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
	}
}



void
iSFPageArtifactsClear_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	char* chnamee = g_markup_escape_text( SF.using_channel->name(), -1);

	if ( SF.using_channel->artifacts().empty() ) {
		pop_ok_message( SF.wSF, "No artifacts to clear", "Channel <b>%s</b> is already clean.", chnamee);

	} else
		if ( GTK_RESPONSE_YES ==
		     pop_question(
			     SF.wSF,
			     "<b>All marked artifacts will be lost</b>",
			     "Sure to clean all artifacts in channel <b>%s</b>?",
			     chnamee) ) {

			SF.using_channel->artifacts().clear();
			SF.using_channel->get_signal_filtered();

			if ( SF.using_channel->schannel().type() == sigfile::SChannel::TType::eeg ) {
				SF.using_channel->get_psd_course();
				SF.using_channel->get_psd_in_bands();
				SF.using_channel->get_spectrum();

				SF.redraw_ssubject_timeline();
			}

			gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
			gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
		}

	g_free( chnamee);
}




void
iSFPageSaveChannelAsSVG_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& ED = SF._p;
	string j_dir = ED.ED->subject_dir( SF.using_channel->crecording.subject());
	string fname = str::sasprintf(
		"%s/%s/%s-p%zu@%zu.svg",
		j_dir.c_str(), ED.AghD(), ED.AghT(), SF.cur_vpage(), SF.vpagesize());

	SF.using_channel->draw_for_montage( fname, SF.da_wd, SF.interchannel_gap);
	ED.sb_message(
		str::sasprintf(
			"Wrote \"%s\"",
			agh::str::homedir2tilda(fname).c_str()));
}


void
iSFPageSaveMontageAsSVG_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& ED = SF._p;
	string j_dir = ED.ED->subject_dir( SF.using_channel->crecording.subject());
	string fname = str::sasprintf(
			"%s/%s/montage-p%zu@%zu.svg",
			j_dir.c_str(), ED.AghD(), SF.cur_vpage(), SF.vpagesize());

	SF.draw_montage( fname);
	ED.sb_message(
		str::sasprintf( "Wrote \"%s\"", agh::str::homedir2tilda(fname).c_str()));
}


void
iSFPageExportSignal_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& r = SF.using_channel->crecording;
	string fname_base = r.F().filename();
	r.F().export_filtered(
		SF.using_channel->h(),
		str::sasprintf( "%s-filt.tsv", fname_base.c_str()));
	r.F().export_original(
		SF.using_channel->h(),
		str::sasprintf( "%s-filt.tsv", fname_base.c_str()));
	SF.sb_message(
		str::sasprintf( "Wrote \"%s-{filt,orig}.tsv\"", fname_base.c_str()));
}



void
iSFPageUseThisScale_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto sane_signal_display_scale = SF.using_channel->signal_display_scale;
	for_each( SF.channels.begin(), SF.channels.end(),
		  [&] ( SScoringFacility::SChannel& H)
		  {
			  H.signal_display_scale = sane_signal_display_scale;
		  });
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}





void
iSFPageAnnotationDelete_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.over_annotations.size() == 1 ) {
		if ( GTK_RESPONSE_YES
		     == pop_question( SF.wSF,
				      "<b>Deleting annotation</b>",
				      "Sure you want to delete annotation\n <b>%s</b>?",
				      SF.over_annotations.front()->label.c_str()) )
			SF.using_channel->annotations.remove(
				*SF.over_annotations.front());
	} else {
		if ( GTK_RESPONSE_YES
		     == pop_question( SF.wSF,
				      "<b>Deleting annotations</b>",
				      "Sure you want to delete <b>%zu annotations</b>?",
				      SF.over_annotations.size()) )
			for ( auto &rm : SF.over_annotations )
				SF.using_channel->annotations.remove( *rm);
	}
	SF._p.populate_mGlobalAnnotations();
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}


void
iSFPageAnnotationEdit_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	sigfile::SAnnotation *which =
		(SF.over_annotations.size() == 1)
		? SF.over_annotations.front()
		: SF.interactively_choose_annotation();
	if ( not which )
		return;

	gtk_entry_set_text( SF.eSFAnnotationLabel, which->label.c_str());
	switch ( which->type ) {
	case sigfile::SAnnotation::TType::phasic_event_spindle:
		gtk_toggle_button_set_active( (GtkToggleButton*)SF.eSFAnnotationTypeSpindle, TRUE);
		break;
	case sigfile::SAnnotation::TType::phasic_event_K_complex:
		gtk_toggle_button_set_active( (GtkToggleButton*)SF.eSFAnnotationTypeKComplex, TRUE);
		break;
	case sigfile::SAnnotation::TType::eyeblink:
		gtk_toggle_button_set_active( (GtkToggleButton*)SF.eSFAnnotationTypeBlink, TRUE);
		break;
	case sigfile::SAnnotation::TType::plain:
	default:
		gtk_toggle_button_set_active( (GtkToggleButton*)SF.eSFAnnotationTypePlain, TRUE);
		break;
	}

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( SF.wSFAnnotationLabel) ) {
		const char* new_label = gtk_entry_get_text( SF.eSFAnnotationLabel);
		auto new_type =
			gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeSpindle)
			? sigfile::SAnnotation::TType::phasic_event_spindle
			: gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeKComplex)
			? sigfile::SAnnotation::TType::phasic_event_K_complex
			: gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeBlink)
			? sigfile::SAnnotation::TType::eyeblink
			: sigfile::SAnnotation::TType::plain;

		if ( strlen(new_label) > 0 ) {
			which->label = new_label;
			which->type = new_type;
			SF._p.populate_mGlobalAnnotations();
			gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		}
	}
}


void
iSFPageAnnotationClearAll_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	char* chnamee = g_markup_escape_text( SF.using_channel->name(), -1);
	if ( GTK_RESPONSE_YES
	     == pop_question(
		     SF.wSF,
		     "<b>Deleting annotations</b>",
		     "Sure you want to delete all annotations in channel <b>%s</b>?",
		     chnamee) ) {

		SF.using_channel->annotations.clear();

		SF._p.populate_mGlobalAnnotations();
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	}
	g_free( chnamee);
}


void
iSFPageAnnotationGotoNext_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( SF.cur_vpage() == SF.total_vpages()-1 )
		return;
	size_t p = SF.cur_vpage();
	while ( ++p < SF.total_vpages() )
		if ( SF.page_has_annotations( p, *SF.using_channel)) {
			SF.sb_clear();
			SF.set_cur_vpage( p);
			return;
		}
	SF.sb_message( "No more annotations after this");
}

void
iSFPageAnnotationGotoPrev_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	if ( SF.cur_vpage() == 0 )
		return;
	size_t p = SF.cur_vpage();
	while ( --p != (size_t)-1 )
		if ( SF.page_has_annotations( p, *SF.using_channel)) {
			SF.sb_clear();
			SF.set_cur_vpage( p);
			return;
		}
	SF.sb_message( "No more annotations before this");
}






void
iSFICAPageMapIC_activate_cb(
	GtkRadioMenuItem* i,
	const gpointer u)
{
	auto& SF = *(SScoringFacility*)u;
	if ( SF.suppress_redraw )
		return;
	const char *label = gtk_menu_item_get_label( (GtkMenuItem*)i);

      // find target h
	int target = -1;
	int h = 0;
	for ( auto H = SF.channels.begin(); H != SF.channels.end(); ++H, ++h )
		if ( strcmp( H->name(), label) == 0 ) {
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
iSFPageSelectionMarkArtifact_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& H = SF.using_channel;
	SBusyBlock bb (SF.wSF);

	H->mark_region_as_artifact( true);

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}

void
iSFPageSelectionClearArtifact_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& H = SF.using_channel;
	SBusyBlock bb (SF.wSF);

	H->mark_region_as_artifact( false);

	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);
}

void
iSFPageSelectionFindPattern_activate_cb(
	GtkMenuItem*,
	gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& H = SF.using_channel;
	H->mark_region_as_pattern();
}

void
iSFPageSelectionAnnotate_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	gtk_entry_set_text( SF.eSFAnnotationLabel, "");

	if ( GTK_RESPONSE_OK ==
	     gtk_dialog_run( (GtkDialog*)SF.wSFAnnotationLabel) ) {
		auto new_ann = gtk_entry_get_text( SF.eSFAnnotationLabel);

		using sigfile::SAnnotation;
		auto type =
			gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeSpindle)
			? SAnnotation::TType::phasic_event_spindle
			: gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeKComplex)
			? SAnnotation::TType::phasic_event_K_complex
			: gtk_toggle_button_get_active( (GtkToggleButton*)SF.eSFAnnotationTypeBlink)
			? SAnnotation::TType::eyeblink
			: SAnnotation::TType::plain;

		if ( strlen( new_ann) == 0 && type == SAnnotation::TType::plain ) {
			pop_ok_message( SF.wSF, "Give a plain annotation a name", "and try again.");
			return;
		}

		SF.using_channel->mark_region_as_annotation( new_ann, type);

		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFHypnogram);

		SF._p.populate_mGlobalAnnotations();
	}
}


void
iSFPageSelectionDrawCourse_toggled_cb(
	GtkCheckMenuItem *cb,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_selection_course = gtk_check_menu_item_get_active( cb);
	if ( SF.suppress_redraw )
		return;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageSelectionDrawEnvelope_toggled_cb(
	GtkCheckMenuItem *cb,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_selection_envelope = gtk_check_menu_item_get_active( cb);
	if ( SF.suppress_redraw )
		return;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPageSelectionDrawDzxdf_toggled_cb(
	GtkCheckMenuItem *cb,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	SF.using_channel->draw_selection_dzcdf = gtk_check_menu_item_get_active( cb);
	if ( SF.suppress_redraw )
		return;
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}



// power

void
iSFPowerExportRange_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	auto& R = SF.using_channel->crecording;

	if ( SF.using_channel->draw_psd ) {
		string fname = str::sasprintf(
			"%s-psd_%g-%g.tsv",
			R.psd_profile.fname_base().c_str(), SF.using_channel->psd.from, SF.using_channel->psd.upto);
		R.psd_profile.export_tsv(
			SF.using_channel->psd.from, SF.using_channel->psd.upto,
			fname);
		SF.sb_message( str::sasprintf( "Wrote \"%s\"", str::homedir2tilda(fname).c_str()));
	}
	// if ( SF.using_channel->draw_swu ) {
	// 	fname_base = R.swu_profile.fname_base();
	// 	snprintf_buf( "%s-swu_%g-%g.tsv",
	// 		      fname_base.c_str(), SF.using_channel->swu.from, SF.using_channel->swu.upto);
	// 	R.swu_profile.export_tsv(
	// 		SF.using_channel->swu.from, SF.using_channel->swu.upto,
	// 		global::buf);
	// 	fname_base = global::buf; // recycle
	// }
	// if ( SF.using_channel->draw_mc ) {
	// 	fname_base = R.mc_profile.fname_base();
	// 	snprintf_buf( "%s-mc_%g-%g.tsv",
	// 		      fname_base.c_str(),
	// 		      R.freq_from + R.bandwidth*(SF.using_channel->mc.bin),
	// 		      R.freq_from + R.bandwidth*(SF.using_channel->mc.bin+1));
	// 	R.mc_profile.export_tsv(
	// 		SF.using_channel->mc.bin,
	// 		global::buf);
	// 	fname_base = global::buf;
	// }
}

void
iSFPowerExportAll_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	const auto& SF = *(SScoringFacility*)userdata;
	auto& R = SF.using_channel->crecording;

	string fname_base;
	if ( SF.using_channel->draw_psd ) {
		string fname = str::sasprintf(
			"%s-psd.tsv",
			SF.using_channel->crecording.psd_profile.fname_base().c_str());
		R.psd_profile.export_tsv( fname);
		SF.sb_message( str::sasprintf( "Wrote \"%s\"", agh::str::homedir2tilda(fname).c_str()));
	}
	if ( SF.using_channel->draw_swu ) {
		string fname = str::sasprintf(
			"%s-swu.tsv",
			SF.using_channel->crecording.swu_profile.fname_base().c_str());
		R.swu_profile.export_tsv( fname);
		SF.sb_message( str::sasprintf( "Wrote \"%s\"", agh::str::homedir2tilda(fname).c_str()));
	}
	if ( SF.using_channel->draw_mc ) {
		string fname = str::sasprintf(
			"%s-psd.tsv",
			SF.using_channel->crecording.psd_profile.fname_base().c_str());
		R.psd_profile.export_tsv( fname);
		SF.sb_message( str::sasprintf( "Wrote \"%s\"", agh::str::homedir2tilda(fname).c_str()));
	}
}

void
iSFPowerSmooth_toggled_cb(
	GtkCheckMenuItem *menuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	if ( likely (SF.using_channel->schannel().type() == sigfile::SChannel::TType::eeg ) ) {
		SF.using_channel->resample_power = (bool)gtk_check_menu_item_get_active( menuitem);
		SF.using_channel->get_psd_course();
		SF.using_channel->get_psd_in_bands();
		SF.using_channel->get_swu_course();
		SF.using_channel->get_mc_course();
		gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
	}
}

void
iSFPowerDrawBands_toggled_cb(
	GtkCheckMenuItem *menuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	SF.using_channel->draw_psd_bands = (bool)gtk_check_menu_item_get_active( menuitem);
	gtk_widget_queue_draw( (GtkWidget*)SF.daSFMontage);
}

void
iSFPowerUseThisScale_activate_cb(
	GtkMenuItem*,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;

	auto	sane_psd_display_scale = SF.using_channel->psd.display_scale,
		sane_swu_display_scale = SF.using_channel->swu.display_scale,
		sane_mc_display_scale  = SF.using_channel->mc.display_scale;
	for ( auto& H : SF.channels ) {
		H.psd.display_scale = sane_psd_display_scale;
		H.swu.display_scale = sane_swu_display_scale;
		H.mc.display_scale  = sane_mc_display_scale;
	}
	SF.queue_redraw_all();
}

void
iSFPowerAutoscale_toggled_cb(
	GtkCheckMenuItem *menuitem,
	const gpointer userdata)
{
	auto& SF = *(SScoringFacility*)userdata;
	if ( SF.suppress_redraw )
		return;
	auto& H = *SF.using_channel;

	H.autoscale_profile = (bool)gtk_check_menu_item_get_active( menuitem);

	SF.queue_redraw_all();
}


} // extern "C"

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
