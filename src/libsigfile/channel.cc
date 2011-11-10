// ;-*-C++-*-
/*
 *       File name:  libsigfile/channel.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-10
 *
 *         Purpose:  a string-based class representing a biosig channel
 *
 *         License:  GPL
 */


#include "channel.hh"

using namespace std;

#if HAVE_CONFIG_H
#  include "config.h"
#endif


array<const char*, sigfile::SChannel::n_known_channels>
sigfile::SChannel::system1020_channels = {{  // counted 'em all!
	"Nz",
	"Fp1", "Fpz", "Fp2",
	"AF7", "AF3", "AFz", "AF4", "AF8",
	"F9",  "F7", "F5", "F3", "F1", "Fz", "F2", "F4", "F6", "F8", "F10",
	"FT9", "FT7", "FC5", "FC3", "FC1", "FCz", "FC2", "FC4", "FC6", "FCT8", "FT10",
	"A1", "T9", "T7", "C5", "C3", "C1", "Cz", "C2", "C4", "C6", "T8", "T10", "A2",
	"TP9", "TP7", "CP5", "CP3", "CP1", "CPz", "CP2", "CP4", "CP6", "TP8", "TP10",
	"P9", "P7", "P5", "P3", "P1", "Pz", "P2", "P4", "P6", "P8", "P10",
	"PO7", "PO3", "POz", "PO4", "PO8",
	"O1", "Oz", "O2",
	"Iz",
	// plus channels of other signal types
	"Left", "Right",
	"Chin",
}};




array<const char*, agh::SChannel::n_known_signal_types>
sigfile::SChannel::kemp_signal_types = {{
	"EEG", "EOG", "EMG", "ECG", "ERG",
	"NC",  "MEG", "MCG", "EP",
	"Temp", "Resp", "SaO2",
	"Light", "Sound", "Event", "Freq",
}};


const char*
sigfile::SChannel::signal_type_following_kemp( const string& signal)
{
	size_t h = 0;
	for ( ; h <= last_eeg_no; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[0];  // we
	for ( ; h <= last_eog_no; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[1];  // love
	for ( ; h <= last_emg_no; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[2];  // plain C
	return NULL;
}

bool
sigfile::SChannel::channel_follows_system1020( const string& channel)
{
	for ( size_t h = 0; h < system1020_channels.size(); ++h )
		if ( channel == system1020_channels[h] )
			return true;
	return false;
}



int
sigfile::SChannel::compare( const char *a, const char *b)
{
	size_t ai = 0, bi = 0;
	while ( ai < system1020_channels.size() && strcmp( a, system1020_channels[ai]) )
		++ai;
	while ( bi < system1020_channels.size() && strcmp( b, system1020_channels[bi]) )
		++bi;
	return (ai < bi) ? -1 : ((ai > bi) ? 1 : strcmp( a, b));
}


// eof
