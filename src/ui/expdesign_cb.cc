// ;-*-C++-*-
/*
 *       File name:  ui/expdesign_cb.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-06-29
 *
 *         Purpose:  SExpDesignUI widget callbacks
 *
 *         License:  GPL
 */

#include "misc.hh"
#include "expdesign.hh"
#include "scoring-facility.hh"


using namespace aghui;

extern "C" {

void
iExpChange_activate_cb( GtkMenuItem *item, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	gtk_window_get_position( ED.wMainWindow, &ED.geometry.x, &ED.geometry.y);
	gtk_window_get_size( ED.wMainWindow, &ED.geometry.w, &ED.geometry.h);

	gtk_widget_show( (GtkWidget*)ED.wExpDesignChooser);
	gtk_widget_hide( (GtkWidget*)ED.wMainWindow);
	// if ( gtk_widget_get_visible( (GtkWidget*)wScoringFacility) )
	// 	gtk_widget_hide( (GtkWidget*)wScoringFacility);
	// better make sure bExpChange is greyed out on opening any child windows
}


void
iExpRefresh_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.do_rescan_tree( false);
}




void
iExpAnnotations_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( gtk_dialog_run( ED.wGlobalAnnotations) == -1 )
		;
}


void
iExpQuit_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	ED.shutdown();
}


void
iMontageResetAll_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;

	snprintf_buf( "find '%s' -name '.*.montage' -delete",
		      ED.ED->session_dir());
	if ( system( __buf__) )
		;
}


inline namespace {
void
set_all_filters( agh::CExpDesign& ED, sigfile::SFilterPack::TNotchFilter value)
{
	for ( auto &G : ED.groups )
		for ( auto &J : G.second )
			for ( auto &D : J.measurements )
				for ( auto &E : D.second.episodes )
					for ( auto &F : E.sources )
						for ( auto &H : F.channel_list() )
							F.filters(H.c_str()).notch_filter = value;
	ED.sync();
}
} // namespace

void
iMontageNotchNone_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::none);
}

void
iMontageNotch50Hz_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::at50Hz);
}

void
iMontageNotch60Hz_activate_cb( GtkMenuItem*, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	set_all_filters( *ED.ED, sigfile::SFilterPack::TNotchFilter::at60Hz);
}



gboolean
wMainWindow_configure_event_cb( GtkWidget *wid, GdkEvent *event, gpointer userdata)
{
	auto EDp = (SExpDesignUI*)userdata;
	if ( event->type == GDK_CONFIGURE ) {
		EDp->geometry.x = ((GdkEventConfigure*)event) -> x;
		EDp->geometry.y = ((GdkEventConfigure*)event) -> y;
		EDp->geometry.w = ((GdkEventConfigure*)event) -> width;
		EDp->geometry.h = ((GdkEventConfigure*)event) -> height;
	}
	return FALSE; // whatever
}

gboolean
wMainWindow_delete_event_cb( GtkWidget *wid, GdkEvent *event, gpointer userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( ED.nodestroy_by_cb )
		return TRUE;

	ED.shutdown();

	return TRUE; // whatever
}


// tab switch
void
tTaskSelector_switch_page_cb( GtkNotebook     *notebook,
			      gpointer	       unused,
			      guint            page_num,
			      gpointer         userdata)
{
	auto& ED = *(SExpDesignUI*)userdata;
	if ( page_num == 1 ) {
		//ED.populate( false);
		snprintf_buf( "Session: <b>%s</b>", ED.AghD());
		gtk_label_set_markup( ED.lSimulationsSession, __buf__);
		snprintf_buf( "Channel: <b>%s</b>", ED.AghT());
		gtk_label_set_markup( ED.lSimulationsChannel, __buf__);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpChange, FALSE);
		ED.populate_2();
	} else if ( page_num == 0 ) {
		// ED.ED->remove_untried_modruns(); // done in populate_2
		// ED.populate( false);
		gtk_widget_set_sensitive( (GtkWidget*)ED.iExpChange, TRUE);
	}
}


// -------- colours
void
bColourX_color_set_cb( GtkColorButton *widget,
		       gpointer        userdata)
{
	auto& mc = *(SManagedColor*)userdata;
	mc.acquire();
}

} // extern "C"

// eof
