// ;-*-C++-*- *  Time-stamp: "2011-05-08 02:04:37 hmmr"
/*
 *       File name:  ui/ui.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  common ui variables, GTK+ widgets, and models
 *
 *         License:  GPL
 */



#include <cstring>
#include <algorithm>
#include <initializer_list>

#include <cairo.h>

#include "ui.hh"
#include "settings.hh"
#include "misc.hh"


#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace aghui {



const array<unsigned, 4>
	FFTPageSizeValues = {{15, 20, 30, 60}};
const array<unsigned, 8>
	DisplayPageSizeValues = {{5, 10, 15, 20, 30, 60, 120, 300}};


const char* const FreqBandNames[(size_t)TBand::_total] = {
	       "Delta", "Theta", "Alpha", "Beta", "Gamma",
};

GtkBuilder
	*__builder;

GtkWindow
	*wMainWindow;

GtkListStore
	*mSessions,
	*mEEGChannels,
	*mAllChannels;

GtkTreeStore
	*mSimulations;




GdkVisual
	*__visual;

const GdkColor*
contrasting_to( const GdkColor* c)
{
	static GdkColor cc;
	if ( c->red + c->green + c->blue < 65535*3/2 )
		cc.red = cc.green = cc.blue = 65535;
	else
		cc.red = cc.green = cc.blue = 0;
	return &cc;
}




#define AGH_UI_FILE "agh-ui.glade"
#define AGH_BG_IMAGE_FNAME "idle-bg.svg"

inline namespace {
	GString *__pkg_data_path = NULL;

	const char* const scoring_pagesize_values_s[9] = {
		       "5 sec", "10 sec", "15 sec", "20 sec", "30 sec", "1 min", "2 min", "5 min", NULL
	};
	const char* const fft_pagesize_values_s[5] = {
		       "15 sec", "20 sec", "30 sec", "1 min", NULL
	};

	void
	populate_static_models()
	{
		GtkTreeIter iter;
		size_t i;
		for ( i = 0; scoring_pagesize_values_s[i]; ++i ) {
			gtk_list_store_append( settings::mScoringPageSize, &iter);
			gtk_list_store_set( settings::mScoringPageSize, &iter, 0, scoring_pagesize_values_s[i], -1);
		}

		// must match FFTPageSizeValues
		for ( i = 0; fft_pagesize_values_s[i]; ++i ) {
			gtk_list_store_append( settings::mFFTParamsPageSize, &iter);
			gtk_list_store_set( settings::mFFTParamsPageSize, &iter, 0, fft_pagesize_values_s[i], -1);
		}

		for( i = 0; i < (size_t)agh::TFFTWinType::_total; ++i ) {
			gtk_list_store_append( settings::mFFTParamsWindowType, &iter);
			gtk_list_store_set( settings::mFFTParamsWindowType, &iter, 0, agh::SFFTParamSet::welch_window_type_names[i], -1);
		}
		for( i = 0; i < (size_t)agh::TFFTWinType::_total; ++i ) {
			gtk_list_store_append( settings::mAfDampingWindowType, &iter);
			gtk_list_store_set( settings::mAfDampingWindowType, &iter, 0, agh::SFFTParamSet::welch_window_type_names[i], -1);
		}
	}
}

int
construct_once()
{
      // load glade
	__builder = gtk_builder_new();
	if ( !gtk_builder_add_from_file( __builder, PACKAGE_DATADIR "/" AGH_UI_FILE, NULL) ||
	     !AGH_GBGETOBJ (GtkWindow, wMainWindow) ) {
		pop_ok_message( NULL, "Failed to load UI description file.");
		return -1;
	}

	// glade_xml_signal_autoconnect( xml); // done automatically? how nice

      // construct list and tree stores
	mSessions =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mEEGChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	mAllChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	sf::patterns::mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	mSimulations =
		gtk_tree_store_new( 16,
				    G_TYPE_STRING,	// group, subject, channel, from-upto
				    G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,	// tunables
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

	settings::mScoringPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	settings::mFFTParamsPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	settings::mFFTParamsWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);
	settings::mAfDampingWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);

	sb::mExpDesignList =
		gtk_list_store_new( 1, G_TYPE_STRING);

	populate_static_models();

      // now construct treeviews which glade failed to, and set up all facilities
	if ( misc::construct()		||
	     msmtview::construct()	||
	     settings::construct()	||
	     sf::construct()		||
	     sf::filter::construct()	||
	     sf::patterns::construct()	||
	     sf::phasediff::construct()	||
	     simview::construct()	||
	     mf::construct()		||
	     sb::construct() ) {
		pop_ok_message( NULL, "Failed to construct some widgets.  It was you who messed things up.");
		return -1;
	}

	return 0;
}




inline namespace {
	template <class T>
	void
	print_xx( const char *pre, const list<T>& ss)
	{
		printf( "%s", pre);
		for ( auto S = ss.begin(); S != ss.end(); ++S )
			printf( " %s;", S->c_str());
		printf("\n");
	}
}

int
populate( int do_load)
{
	AghDD = AghCC->enumerate_sessions();
	print_xx( "Sessions:", AghDD);
	AghGG = AghCC->enumerate_groups();
	print_xx( "Groups:", AghGG);
	AghHH = AghCC->enumerate_all_channels();
	print_xx( "All Channels:", AghHH);
	AghTT = AghCC->enumerate_eeg_channels();
	print_xx( "EEG channels:", AghTT);
	AghEE = AghCC->enumerate_episodes();
	print_xx( "Episodes:", AghEE);

	if ( do_load ) {
		if ( settings::load() )
			;
		else
			if ( GeometryMain.w > 0 ) // implies the rest are, too
				gdk_window_move_resize( gtk_widget_get_window( (GtkWidget*)wMainWindow),
							GeometryMain.x, GeometryMain.y,
							GeometryMain.w, GeometryMain.h);
	}

	if ( AghGG.empty() ) {
		gtk_container_foreach( (GtkContainer*)cMeasurements,
				       (GtkCallback) gtk_widget_destroy,
				       NULL);
		const char *briefly =
			"<b><big>Empty experiment\n</big></b>\n"
			"When you have your recordings ready as a set of .edf files,\n"
			"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
			"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
			"• Drop EDF sources onto here and identify and place them individually.\n\n"
			"Once set up, either:\n"
			"• click <b>⎇</b> and select the top directory of the (newly created) experiment tree, or\n"
			"• click <b>Rescan</b> if this is the tree you have just populated.";
		GtkLabel *text = (GtkLabel*)gtk_label_new( "");
		gtk_label_set_markup( text, briefly);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)text,
				    TRUE, TRUE, 0);

		snprintf_buf( "%s%s", __pkg_data_path->str, AGH_BG_IMAGE_FNAME);
		gtk_box_pack_start( (GtkBox*)cMeasurements,
				    (GtkWidget*)gtk_image_new_from_file( __buf__),
				    TRUE, FALSE, 0);
		gtk_widget_show_all( (GtkWidget*)cMeasurements);
	} else {
		populate_mChannels();
		populate_mSessions();
		msmtview::populate();
//		populate_mSimulations( FALSE);
	}

	return 0;
}


void
depopulate( int do_save)
{
	if ( do_save )
		settings::save();

	sf::destruct();
	msmtview::destruct();

	// these are freed on demand immediately before reuse; leave them alone
	AghGG.clear();
	AghDD.clear();
	AghEE.clear();
	AghHH.clear();
	AghTT.clear();

	gtk_list_store_clear( mSessions);

	gtk_list_store_clear( mEEGChannels);
}








void
populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
	GtkTreeIter iter;
	for ( auto D = AghDD.begin(); D != AghDD.end(); ++D ) {
		gtk_list_store_append( mSessions, &iter);
		gtk_list_store_set( mSessions, &iter,
				    0, D->c_str(),
				    -1);
	}
	__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, msmt::eMsmtSession_changed_cb_handler_id);
}






void
populate_mChannels()
{
	g_signal_handler_block( ePatternChannel, sf::ePatternChannel_changed_cb_handler_id);
	g_signal_handler_block( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
	g_signal_handler_block( ePhaseDiffChannelA, sf::ePhaseDiffChannelA_changed_cb_handler_id);
	g_signal_handler_block( ePhaseDiffChannelB, sf::ePhaseDiffChannelB_changed_cb_handler_id);

	// for ( auto H = AghTT.begin(); H != AghTT.end(); ++H ) {
	// 	gtk_list_store_append( agh_mEEGChannels, &iter);
	// 	gtk_list_store_set( agh_mEEGChannels, &iter,
	// 			    0, H->c_str(),
	// 			    -1);
	// }
	for_each( AghTT.begin(), AghTT.end(),
		  [&] ( const SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mEEGChannels, &iter);
			  gtk_list_store_set( mEEGChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	for_each( AghHH.begin(), AghHH.end(),
		  [&] ( const SChannel& H) {
			  GtkTreeIter iter;
			  gtk_list_store_append( mAllChannels, &iter);
			  gtk_list_store_set( mAllChannels, &iter,
					      0, H.c_str(),
					      -1);
		  });

	__reconnect_channels_combo();

	g_signal_handler_unblock( ePatternChannel, sf::ePatternChannel_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtChannel, msmt::eMsmtChannel_changed_cb_handler_id);
	g_signal_handler_unblock( ePhaseDiffChannelA, sf::ePhaseDiffChannelA_changed_cb_handler_id);
	g_signal_handler_unblock( ePhaseDiffChannelB, sf::ePhaseDiffChannelB_changed_cb_handler_id);
}






void
__reconnect_channels_combo()
{
	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtChannel),		GTK_TREE_MODEL (mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePatternChannel),	GTK_TREE_MODEL (mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelA),	GTK_TREE_MODEL (mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelB),	GTK_TREE_MODEL (mEEGChannels));

	if ( !AghTT.empty() ) {
		int Ti = AghTi();
		if ( Ti != -1 ) {
			gtk_combo_box_set_active( GTK_COMBO_BOX (ePatternChannel), Ti);
			gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtChannel),    Ti);
			gtk_combo_box_set_active( GTK_COMBO_BOX (ePhaseDiffChannelA), Ti);
			gtk_combo_box_set_active( GTK_COMBO_BOX (ePhaseDiffChannelB), Ti);
		}
	}
}


void
__reconnect_sessions_combo()
{
	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtSession),   GTK_TREE_MODEL (mSessions));

	if ( !AghDD.empty() ) {
		int Di = AghDi();
		if ( Di != -1 )
			gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtSession),   Di);
	}
}




// colours

unordered_map<TColour, SManagedColor>
	CwB;

}


// EOF
