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


const array<const char*, sigfile::SChannel::n_channels>
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
	// plus a few channels of other signal types
	"Left", "Right",
	"Chin",
}};




const array<const char*, sigfile::SChannel::n_kemp_signal_types>
sigfile::SChannel::kemp_signal_types = {{
	"EEG", "EOG", "EMG", "ECG", "ERG",
	"NC",  "MEG", "MCG", "EP",
	"Temp", "Resp", "SaO2",
	"Light", "Sound", "Event", "Freq",
	"(unknown)"
}};


sigfile::SChannel::TType
sigfile::SChannel::signal_type_of_channel( const string& signal)
{
	size_t h = 0;
	for ( ; h <= last_eeg_no; ++h )
		if ( signal == system1020_channels[h] )
			return TType::eeg;
	for ( ; h <= last_eog_no; ++h )
		if ( signal == system1020_channels[h] )
			return TType::eog;
	for ( ; h <= last_emg_no; ++h )
		if ( signal == system1020_channels[h] )
			return TType::emg;
	return TType::other;
}


bool
sigfile::SChannel::operator<( const SChannel& rv) const
{
	size_t ai = 0, bi = 0;
	while ( ai < n_channels && strcmp(    c_str(), system1020_channels[ai]) )
		++ai;
	while ( bi < n_channels && strcmp( rv.c_str(), system1020_channels[bi]) )
		++bi;
	if ( ai < n_channels && bi < n_channels ) // both are vlaid 10-20 ones: compare by index
		return (ai < bi);
	else if ( ai < n_channels )  // whichever is good, wins
		return false;
	else if ( bi < n_channels )
		return true;
	else
		return strcmp( c_str(), rv.c_str()) < 0;
}


// eof
