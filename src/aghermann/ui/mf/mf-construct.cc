/*
 *       File name:  aghermann/ui/mf/construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-11
 *
 *         Purpose:  modelrun facility construct_widgets
 *
 *         License:  GPL
 */

#include "mf.hh"
#include "mf_cb.hh"


int
aghui::SModelrunFacility::
construct_widgets()
{
	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,	wModelrunFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,	daMFProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTextView,	lMFLog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFLiveUpdate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFHighlightWake)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFHighlightNREM)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFHighlightREM)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFLiveUpdate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkScaleButton,	eMFSmooth)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,		cMFControls)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,	lMFCostFunction)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFClassicFit)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,	lMFClassicFit)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,	bMFRun)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,	bMFReset)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,	bMFAccept)) )
		return -1;

	g_signal_connect( wModelrunFacility, "delete-event",
			  (GCallback)wModelrunFacility_delete_event_cb,
			  this);
	g_signal_connect( eMFSmooth, "value-changed",
			  (GCallback)eMFSmooth_value_changed_cb,
			  this);

	g_signal_connect( eMFHighlightNREM, "toggled",
			  (GCallback)eMFHighlightNREM_toggled_cb,
			  this);
	g_signal_connect( eMFHighlightREM, "toggled",
			  (GCallback)eMFHighlightREM_toggled_cb,
			  this);
	g_signal_connect( eMFHighlightWake, "toggled",
			  (GCallback)eMFHighlightWake_toggled_cb,
			  this);
	g_signal_connect( eMFClassicFit, "toggled",
			  (GCallback)eMFClassicFit_toggled_cb,
			  this);

	if ( !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFDB1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFDB2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFAZ1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFAZ2)) )
		return -1;

	g_signal_connect( eMFDB1, "toggled",
			  (GCallback)eMFDB1_toggled_cb,
			  this);
	g_signal_connect( eMFDB2, "toggled",
			  (GCallback)eMFDB2_toggled_cb,
			  this);
	g_signal_connect( eMFAZ1, "toggled",
			  (GCallback)eMFAZ1_toggled_cb,
			  this);
	g_signal_connect( eMFAZ2, "toggled",
			  (GCallback)eMFAZ2_toggled_cb,
			  this);

	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrs" )] = agh::ach::TTunable::rs ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrc" )] = agh::ach::TTunable::rc ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcR")] = agh::ach::TTunable::fcR;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcW")] = agh::ach::TTunable::fcW;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVS0" )] = agh::ach::TTunable::S0 ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVSU" )] = agh::ach::TTunable::SU ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVta" )] = agh::ach::TTunable::ta ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVtp" )] = agh::ach::TTunable::tp ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc1")] = agh::ach::TTunable::gc1;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc2")] = agh::ach::TTunable::gc2;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc3")] = agh::ach::TTunable::gc3;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc4")] = agh::ach::TTunable::gc4;
	for ( auto &tuple : eMFVx )
		if ( tuple.first == nullptr )
			return -1;
	if ( not csimulation.ctl_params.AZAmendment1 ) // disable gcx unless AZAmendment is in effect
		for ( auto &tuple : eMFVx )
			if ( tuple.second > agh::ach::TTunable::gc )
				gtk_widget_set_sensitive( (GtkWidget*)tuple.first, FALSE);

	for ( auto &tuple : eMFVx )
		g_signal_connect( tuple.first, "value-changed",
				  (GCallback)eMFVx_value_changed_cb,
				  this);

	g_object_set( (GObject*)lMFLog,
		      "tabs", pango_tab_array_new_with_positions(
			      6, TRUE,
			      PANGO_TAB_LEFT, 50,
			      PANGO_TAB_LEFT, 150,
			      PANGO_TAB_LEFT, 240,
			      PANGO_TAB_LEFT, 330,
			      PANGO_TAB_LEFT, 420,
			      PANGO_TAB_LEFT, 510),
		      NULL);


	auto font_desc = pango_font_description_from_string( "Mono 9");
	gtk_widget_override_font( (GtkWidget*)lMFLog, font_desc);
	pango_font_description_free( font_desc);

	log_text_buffer = gtk_text_view_get_buffer( lMFLog);

	g_signal_connect( daMFProfile, "configure-event",
			  (GCallback)daMFProfile_configure_event_cb,
			  this);
	g_signal_connect( daMFProfile, "draw",
			  (GCallback)daMFProfile_draw_cb,
			  this);
	g_signal_connect( daMFProfile, "button-press-event",
			  (GCallback)daMFProfile_button_press_event_cb,
			  this);
	g_signal_connect( daMFProfile, "scroll-event",
			  (GCallback)daMFProfile_scroll_event_cb,
			  this);

	g_signal_connect( bMFRun, "clicked",
			  (GCallback)bMFRun_clicked_cb,
			  this);
	g_signal_connect( bMFReset, "clicked",
			  (GCallback)bMFReset_clicked_cb,
			  this);
	g_signal_connect( bMFAccept, "clicked",
			  (GCallback)bMFAccept_clicked_cb,
			  this);

	return 0;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

