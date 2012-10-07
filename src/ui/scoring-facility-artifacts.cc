// ;-*-C++-*-
/*
 *       File name:  ui/scoring-facility-artifacts.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-10-05
 *
 *         Purpose:  scoring facility: artifact detection dialog
 *
 *         License:  GPL
 */


#include "scoring-facility.hh"
#include "scoring-facility-widgets.hh"

using namespace std;


aghui::SScoringFacility::SChannel::SDetectArtifactsParams
aghui::SScoringFacility::get_mc_params_from_SFAD_widgets() const
{
	return SChannel::SDetectArtifactsParams {
		(float)gtk_spin_button_get_value( eSFADScope),
		(float)gtk_spin_button_get_value( eSFADUpperThr),
		(float)gtk_spin_button_get_value( eSFADLowerThr),
		(float)gtk_spin_button_get_value( eSFADF0),
		(float)gtk_spin_button_get_value( eSFADFc),
		(float)gtk_spin_button_get_value( eSFADBandwidth),
		(float)gtk_spin_button_get_value( eSFADMCGain),
		(float)gtk_spin_button_get_value( eSFADBackpolate),

		gtk_toggle_button_get_active( (GtkToggleButton*)eSFADEstimateE)
			? INFINITY
			: (float)gtk_spin_button_get_value( eSFADEValue),

		(float)gtk_spin_button_get_value( eSFADHistRangeMin),
		(float)gtk_spin_button_get_value( eSFADHistRangeMax),
		(size_t)round(gtk_spin_button_get_value( eSFADHistBins)),

		(size_t)round(gtk_spin_button_get_value( eSFADSmoothSide)),

		(bool)gtk_toggle_button_get_active( (GtkToggleButton*)eSFADUseThisRange)
	};
}

// eof
