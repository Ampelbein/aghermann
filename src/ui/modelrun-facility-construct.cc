// ;-*-C++-*-
/*
 *       File name:  ui/modelrun-facility-construct.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-11
 *
 *         Purpose:  modelrun facility construct_widgets
 *
 *         License:  GPL
 */

#include "modelrun-facility.hh"


int
aghui::SModelrunFacility::construct_widgets()
{
	if ( !(AGH_GBGETOBJ3 (builder, GtkWindow,	wModelrunFacility)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkDrawingArea,	daMFProfile)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkTextView,	lMFLog)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFLiveUpdate)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkHBox,		cMFControls)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkLabel,	lMFCostFunction)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,	bMFRun)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkButton,	bMFReset)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkToolButton,	bMFAccept)) )
		return -1;

	g_signal_connect_after( wModelrunFacility, "delete-event",
				(GCallback)wModelrunFacility_delete_event_cb,
				this);

	if ( !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFDB1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFDB2)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFAZ1)) ||
	     !(AGH_GBGETOBJ3 (builder, GtkCheckButton,	eMFAZ2)) )
		return -1;

	g_signal_connect_after( eMFDB1, "toggled",
				(GCallback)eMFDB1_toggled_cb,
				this);
	g_signal_connect_after( eMFDB2, "toggled",
				(GCallback)eMFDB2_toggled_cb,
				this);
	g_signal_connect_after( eMFAZ1, "toggled",
				(GCallback)eMFAZ1_toggled_cb,
				this);
	g_signal_connect_after( eMFAZ2, "toggled",
				(GCallback)eMFAZ2_toggled_cb,
				this);

	using namespace agh;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrs" )] = TTunable::rs ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVrc" )] = TTunable::rc ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcR")] = TTunable::fcR;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVfcW")] = TTunable::fcW;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVS0" )] = TTunable::S0 ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVSU" )] = TTunable::SU ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVta" )] = TTunable::ta ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVtp" )] = TTunable::tp ;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc1")] = TTunable::gc1;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc2")] = TTunable::gc2;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc3")] = TTunable::gc3;
	eMFVx[(GtkSpinButton*)gtk_builder_get_object( builder, "eMFVgc4")] = TTunable::gc4;
	for ( auto &tuple : eMFVx )
		if ( tuple.first == NULL )
			return -1;
	if ( not csimulation.ctl_params.AZAmendment1 ) // disable gcx unless AZAmendment is in effect
		for ( auto &tuple : eMFVx )
			if ( tuple.second > TTunable::gc )
				gtk_widget_set_sensitive( (GtkWidget*)tuple.first, FALSE);

	for ( auto &tuple : eMFVx ) {
		g_signal_connect_after( tuple.first, "value-changed",
					(GCallback)eMFVx_value_changed_cb,
					this);
		auto	jdst = gtk_spin_button_get_adjustment( tuple.first);
		auto	t = min((size_t)tuple.second, (size_t)TTunable::_basic_tunables - 1);
		gtk_adjustment_configure(
			jdst,
			_p.ED->tunables0.value[t] * agh::STunableSet::stock[t].display_scale_factor,
			_p.ED->tunables0.lo[t]    * agh::STunableSet::stock[t].display_scale_factor,
			_p.ED->tunables0.hi[t]    * agh::STunableSet::stock[t].display_scale_factor,
			agh::STunableSet::stock[t].adj_step,
			agh::STunableSet::stock[t].adj_step * 10,
			0.);
	}

	g_object_set( (GObject*)lMFLog,
		      "tabs", pango_tab_array_new_with_positions( 6, TRUE,
								  PANGO_TAB_LEFT, 50,
								  PANGO_TAB_LEFT, 150,
								  PANGO_TAB_LEFT, 240,
								  PANGO_TAB_LEFT, 330,
								  PANGO_TAB_LEFT, 420,
								  PANGO_TAB_LEFT, 510),
		      NULL);


	auto font_desc = pango_font_description_from_string( "Sans 10");
	gtk_widget_modify_font( (GtkWidget*)lMFLog, font_desc);
	pango_font_description_free( font_desc);

	log_text_buffer = gtk_text_view_get_buffer( lMFLog);

	g_signal_connect_after( daMFProfile, "configure-event",
				(GCallback)daMFProfile_configure_event_cb,
				this);
	g_signal_connect_after( daMFProfile, "draw",
				(GCallback)daMFProfile_draw_cb,
				this);
	g_signal_connect_after( daMFProfile, "button-press-event",
				(GCallback)daMFProfile_button_press_event_cb,
				this);
	g_signal_connect_after( daMFProfile, "scroll-event",
				(GCallback)daMFProfile_scroll_event_cb,
				this);

	g_signal_connect_after( bMFRun, "clicked",
				(GCallback)bMFRun_clicked_cb,
				this);
	g_signal_connect_after( bMFReset, "clicked",
				(GCallback)bMFReset_clicked_cb,
				this);
	g_signal_connect_after( bMFAccept, "clicked",
				(GCallback)bMFAccept_clicked_cb,
				this);

//		jTunable[t][d] = gtk_spin_button_get_adjustment( eTunable[t][d]);
	return 0;
}

// eof

