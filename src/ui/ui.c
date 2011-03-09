// ;-*-C-*- *  Time-stamp: "2011-03-07 19:20:35 hmmr"
/*
 *       File name:  ui/ui.c
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  common ui variables, GTK+ widgets, and models
 *
 *         License:  GPL
 */



#include <unistd.h>
#include <string.h>
#include "cairo.h"
#include "../libagh/iface.h"
#include "ui.h"
#include "settings.h"
#include "misc.h"

int	AghHi,
	AghTi,
	AghGi,
	AghDi,
	AghEi;

const struct SSubject
	*AghJ;

const gchar
	*AghFreqBandsNames[AGH_BAND__TOTAL] = {
	"Delta", "Theta", "Alpha", "Beta", "Gamma",
};

const guint	AghFFTPageSizeValues[]		= { 15, 20, 30, 60, (guint)-1 };
const guint	AghDisplayPageSizeValues[]	= { 5, 10, 15, 20, 30, 60, 120, 300, (guint)-1 };




GtkWidget *wMainWindow;

GtkListStore
	*agh_mScoringPageSize,
	*agh_mFFTParamsPageSize,
	*agh_mFFTParamsWindowType,
	*agh_mAfDampingWindowType;

GtkListStore
	*agh_mSessions,
	*agh_mEEGChannels,
	*agh_mAllChannels;

GtkTreeStore
	*agh_mSimulations;


const gchar* const agh_scoring_pagesize_values_s[] = {
	"5 sec", "10 sec", "15 sec", "20 sec", "30 sec", "1 min", "2 min", "5 min", NULL
};
const gchar* const agh_fft_pagesize_values_s[] = {
	"15 sec", "20 sec", "30 sec", "1 min", NULL
};
const gchar* const agh_fft_window_types_s[] = {
	"Bartlett", "Blackman", "Blackman-Harris",
	"Hamming",  "Hanning",  "Parzen",
	"Square",   "Welch", NULL
};



GdkVisual
	*agh_visual;

const GdkColor*
contrasting_to( GdkColor* c)
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
static GString *__pkg_data_path = NULL;

static void populate_static_models(void);

int
agh_ui_construct()
{
	__pkg_data_path = g_string_sized_new( 120);

      // load glade
	gint retval = 0;
	GladeXML *xml = NULL;
	GString *ui_file = g_string_sized_new( 180);
	g_string_printf( __pkg_data_path, "%s/share/%s/ui/", getenv("HOME"), PACKAGE);
	g_string_printf( ui_file, "%s/%s", __pkg_data_path->str, AGH_UI_FILE);
	if ( access( ui_file->str, R_OK) ) {
		g_string_assign( __pkg_data_path, "/usr/local/share/" PACKAGE "/ui/");
		g_string_printf( ui_file, "%s%s", __pkg_data_path->str, AGH_UI_FILE);
		if ( access( ui_file->str, R_OK) ) {
			g_string_assign( __pkg_data_path, "/usr/share/" PACKAGE "/ui/");
			g_string_printf( ui_file, "%s%s", __pkg_data_path->str, AGH_UI_FILE);
			if ( access( ui_file->str, R_OK) ) {
				fprintf( stderr, "agh_ui_construct(): failed to locate %s in ~/share/"PACKAGE"/ui:/usr/local/share/"PACKAGE"/ui:/usr/share/"PACKAGE"/ui.\n", AGH_UI_FILE);
				retval = -2;
				goto fail;
			}
		}
	}
	if ( !(xml = glade_xml_new( ui_file->str, NULL, NULL)) ||
	     !(wMainWindow = glade_xml_get_widget( xml, "wMainWindow")) ) {
		fprintf( stderr, "UI Init: Failed to construct ui from %s\n", ui_file->str);
		retval = -1;
		goto fail;
	}

	glade_xml_signal_autoconnect( xml);

      // construct list and tree stores
	agh_mSessions =
		gtk_list_store_new( 1, G_TYPE_STRING);
	agh_mEEGChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	agh_mAllChannels =
		gtk_list_store_new( 1, G_TYPE_STRING);
	agh_mPatterns =
		gtk_list_store_new( 1, G_TYPE_STRING);

	agh_mSimulations =
		gtk_tree_store_new( 16,
				    G_TYPE_STRING,	// group, subject, channel, from-upto
				    G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,	// tunables
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_BOOLEAN,
				    G_TYPE_POINTER);

	agh_mScoringPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	agh_mFFTParamsPageSize =
		gtk_list_store_new( 1, G_TYPE_STRING);
	agh_mFFTParamsWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);

	agh_mAfDampingWindowType =
		gtk_list_store_new( 1, G_TYPE_STRING);

	agh_mExpDesignList =
		gtk_list_store_new( 1, G_TYPE_STRING);

	populate_static_models();

      // now construct treeviews which glade failed to, and set up all facilities
	if ( agh_ui_construct_misc( xml)                      ||
	     agh_ui_construct_Measurements( xml)	      ||
	     agh_ui_construct_Settings( xml)		      ||
	     agh_ui_construct_ScoringFacility( xml)	      ||
	     agh_ui_construct_ScoringFacility_Filter( xml)    ||
	     agh_ui_construct_ScoringFacility_Patterns( xml)  ||
	     agh_ui_construct_ScoringFacility_PhaseDiff( xml) ||
	     agh_ui_construct_Simulations( xml)		      ||
	     agh_ui_construct_ModelRun( xml)		      ||
	     agh_ui_construct_StatusBar( xml) ) {
		fprintf( stderr, "agh_ui_construct(): Failed to construct (some) widgets\n");
		retval = -2;
		goto fail;
	}

      // specially `construct' this item
	for ( gushort i = 0; i < AGH_SCORE__TOTAL; ++i )
		AghExtScoreCodes[i] = strdup( AghExtScoreCodes_defaults[i]);

fail:
	g_string_free( ui_file, TRUE);

	if ( xml )
		g_object_unref( xml);

	return retval;
}




static void
print_xx( const char* pre, char** ss)
{
	printf( "%s", pre);
	size_t h = 0;
	while ( ss[h] )
		printf( " %s;", ss[h++]);
	printf("\n");
}

int
agh_ui_populate( int do_load)
{
	AghDs = agh_enumerate_sessions(     &AghDD);
	print_xx( "Sessions:", AghDD);
	AghGs = agh_enumerate_groups(       &AghGG);
	print_xx( "Groups:", AghGG);
	AghHs = agh_enumerate_all_channels( &AghHH);
	print_xx( "All Channels:", AghHH);
	AghTs = agh_enumerate_eeg_channels( &AghTT);
	print_xx( "EEG channels:", AghTT);
	AghEs = agh_enumerate_episodes(     &AghEE);
	print_xx( "Episodes:", AghEE);

	agh_expdesign_snapshot( &agh_cc);
      // this would be done in:
      // (a) agh_populate_cMeasurements, right before local-static
      //     SSubjectPresentation are filled out with pointers into agh_cc members
      // (b) agh_populate_mSimulations, as simulations are volatile
      // At this point, enumerating expdesign entities is enough

	if ( do_load )
		if ( agh_ui_settings_load() )
			;

	if ( AghGs == 0 ) {
		gtk_container_foreach( GTK_CONTAINER (cMeasurements),
				       (GtkCallback) gtk_widget_destroy,
				       NULL);
		const gchar *briefly =
			"<b><big>Empty experiment\n</big></b>\n"
			"When you have your recordings ready as a set of .edf files,\n"
			"• Create your experiment tree as follows: <i>Experiment/Group/Subject/Session</i>;\n"
			"• Have your EDF sources named <i>Episode</i>.edf, and placed in the corresponding <i>Session</i> directory, or\n"
			"• Drop EDF sources onto here and identify and place them individually.\n\n"
			"Once set up, either:\n"
			"• click <b>♻</b> and select the top directory of the (newly created) experiment tree, or\n"
			"• click <b>R</b> if this is the tree you have just populated.";
		GtkWidget *text = GTK_WIDGET (gtk_label_new( ""));
		gtk_label_set_markup( GTK_LABEL (text), briefly);
		gtk_box_pack_start( GTK_BOX (cMeasurements),
				    text,
				    TRUE, TRUE, 0);

		snprintf_buf( "%s%s", __pkg_data_path->str, AGH_BG_IMAGE_FNAME);
		gtk_box_pack_start( GTK_BOX (cMeasurements),
				    GTK_WIDGET (gtk_image_new_from_file( __buf__)),
				    TRUE, FALSE, 0);
//		gtk_widget_show_all( cMeasurements);
	} else {
		agh_populate_mChannels();
		agh_populate_mSessions();
		agh_populate_cMeasurements();
//		agh_populate_mSimulations( FALSE);
	}

	return 0;
}


void
agh_ui_depopulate( int do_save)
{
	if ( do_save )
		agh_ui_settings_save();

	agh_ui_destruct_ScoringFacility();
	agh_ui_destruct_Measurements();

	free_charp_array( AghGG);
	free_charp_array( AghDD);
	free_charp_array( AghEE);
	free_charp_array( AghHH);
	free_charp_array( AghTT);
	AghGG = AghDD = AghEE = AghHH = AghTT = NULL;
	AghGi = AghDi = AghEi = AghHi = AghTi = -1;

//	__agh__disconnect_sessions_combo();
	gtk_list_store_clear( agh_mSessions);

//	__agh__disconnect_channels_combo();
	gtk_list_store_clear( agh_mEEGChannels);
}







static void
populate_static_models()
{
	GtkTreeIter iter;
	size_t i;
	for ( i = 0; agh_scoring_pagesize_values_s[i]; ++i ) {
		gtk_list_store_append( agh_mScoringPageSize, &iter);
		gtk_list_store_set( agh_mScoringPageSize, &iter, 0, agh_scoring_pagesize_values_s[i], -1);
	}

	// must match AghFFTPageSizeValues
	for ( i = 0; agh_fft_pagesize_values_s[i]; ++i ) {
		gtk_list_store_append( agh_mFFTParamsPageSize, &iter);
		gtk_list_store_set( agh_mFFTParamsPageSize, &iter, 0, agh_fft_pagesize_values_s[i], -1);
	}

	for( i = 0; agh_fft_window_types_s[i]; ++i ) {
		gtk_list_store_append( agh_mFFTParamsWindowType, &iter);
		gtk_list_store_set( agh_mFFTParamsWindowType, &iter, 0, agh_fft_window_types_s[i], -1);
	}
	for( i = 0; agh_fft_window_types_s[i]; ++i ) {
		gtk_list_store_append( agh_mAfDampingWindowType, &iter);
		gtk_list_store_set( agh_mAfDampingWindowType, &iter, 0, agh_fft_window_types_s[i], -1);
	}
}








void
agh_populate_mSessions()
{
	g_signal_handler_block( eMsmtSession, eMsmtSession_changed_cb_handler_id);
	GtkTreeIter iter;
	for ( size_t i = 0; i < AghDs; ++i ) {
		gtk_list_store_append( agh_mSessions, &iter);
		gtk_list_store_set( agh_mSessions, &iter,
				    0, AghDD[i],
				    -1);
	}
	__agh__reconnect_sessions_combo();
	g_signal_handler_unblock( eMsmtSession, eMsmtSession_changed_cb_handler_id);
}






void
agh_populate_mChannels()
{
	g_signal_handler_block( ePatternChannel, ePatternChannel_changed_cb_handler_id);
	g_signal_handler_block( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
	g_signal_handler_block( ePhaseDiffChannelA, ePhaseDiffChannelA_changed_cb_handler_id);
	g_signal_handler_block( ePhaseDiffChannelB, ePhaseDiffChannelB_changed_cb_handler_id);

	GtkTreeIter iter;
	size_t h;
	for ( h = 0; h < AghTs; ++h ) {
		gtk_list_store_append( agh_mEEGChannels, &iter);
		gtk_list_store_set( agh_mEEGChannels, &iter,
				    0, AghTT[h],
				    -1);
	}

	for ( h = 0; h < AghHs; ++h ) {
		gtk_list_store_append( agh_mAllChannels, &iter);
		gtk_list_store_set( agh_mAllChannels, &iter,
				    0, AghHH[h],
				    -1);
	}

	__agh__reconnect_channels_combo();

	g_signal_handler_unblock( ePatternChannel, ePatternChannel_changed_cb_handler_id);
	g_signal_handler_unblock( eMsmtChannel, eMsmtChannel_changed_cb_handler_id);
	g_signal_handler_unblock( ePhaseDiffChannelA, ePhaseDiffChannelA_changed_cb_handler_id);
	g_signal_handler_unblock( ePhaseDiffChannelB, ePhaseDiffChannelB_changed_cb_handler_id);
}






void
__agh__reconnect_channels_combo()
{
	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtChannel),		GTK_TREE_MODEL (agh_mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (eSimulationsChannel),	GTK_TREE_MODEL (agh_mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePatternChannel),	GTK_TREE_MODEL (agh_mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelA),	GTK_TREE_MODEL (agh_mEEGChannels));
	gtk_combo_box_set_model( GTK_COMBO_BOX (ePhaseDiffChannelB),	GTK_TREE_MODEL (agh_mEEGChannels));

	if ( AghTs == 0 )
		AghTi = -1;
	else {
		if ( AghTi < 0 )
			AghTi = 0;
		else if ( AghTi >= AghTs )
			AghTi = AghTs-1;
		gtk_combo_box_set_active( GTK_COMBO_BOX (ePatternChannel), AghTi);
		gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtChannel),    AghTi);
		gtk_combo_box_set_active( GTK_COMBO_BOX (eSimulationsChannel),  AghTi);
		gtk_combo_box_set_active( GTK_COMBO_BOX (ePhaseDiffChannelA), AghTi);
		gtk_combo_box_set_active( GTK_COMBO_BOX (ePhaseDiffChannelB), AghTi);
	}
}


void
__agh__reconnect_sessions_combo()
{
	gtk_combo_box_set_model( GTK_COMBO_BOX (eMsmtSession),   GTK_TREE_MODEL (agh_mSessions));
	gtk_combo_box_set_model( GTK_COMBO_BOX (eSimulationsSession),  GTK_TREE_MODEL (agh_mSessions));

	if ( AghDs == 0 )
		AghDi = -1;
	else {
		if ( AghDi < 0 )
			AghDi = 0;
		else if ( AghDi >= AghDs )
			AghDi = AghDs-1;
		gtk_combo_box_set_active( GTK_COMBO_BOX (eMsmtSession),   AghDi);
		gtk_combo_box_set_active( GTK_COMBO_BOX (eSimulationsSession),  AghDi);
	}
}






// EOF
