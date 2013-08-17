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


#include <map>
#include <vector>
#include <algorithm>
#include "common/string.hh"
#include "channel.hh"

using namespace std;
using sigfile::SChannel;

const char* sigfile::edf_annotations_label =
	"EDF Annotations";


namespace {

const map<SChannel::TType, vector<const char*>> _CT_ = {
	{SChannel::TType::eeg,
	 {"(custom)",	  // counted 'em all!
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
	  "Iz",}
	},
	{SChannel::TType::eog,
	 {"(invalid)",
	  "Left", "Right",}
	},
	{SChannel::TType::emg,
	 {"(invalid)",
	  "Chin",}
	},
};

const map<SChannel::TType, const char*> _ST_ = {
	{SChannel::TType::embedded_annotation, sigfile::edf_annotations_label},
	{SChannel::TType::eeg, "EEG"},
	{SChannel::TType::eog, "EOG"},
	{SChannel::TType::emg, "EMG"},
	{SChannel::TType::ecg, "ECG"},
	{SChannel::TType::erg, "ERG"},
	{SChannel::TType::nc , "NC" },
	{SChannel::TType::meg, "MEG"},
	{SChannel::TType::mcg, "MCG"},
	{SChannel::TType::ep , "EP" },
	{SChannel::TType::temp, "Temp"},
	{SChannel::TType::resp, "Resp"},
	{SChannel::TType::sao2, "SaO2"},
	{SChannel::TType::light, "Light"},
	{SChannel::TType::sound, "Sound"},
	{SChannel::TType::event, "Event"},
	{SChannel::TType::freq, "Freq"},
	{SChannel::TType::other, "(other)"},
};

} // anonymous namespace


const char*
SChannel::
type_s( SChannel::TType t)
{
	return _ST_.at(t);
}


template <SChannel::TType t>
const char*
SChannel::
channel_s( int idx)
{
	return _CT_.at(t)[idx];
}

namespace sigfile {
template <>
const char*
SChannel::
channel_s<SChannel::TType::invalid>(int)
{
	return "(invalid)";
}
}

template const char* SChannel::channel_s<SChannel::TType::eeg>( int);
template const char* SChannel::channel_s<SChannel::TType::eog>( int);
template const char* SChannel::channel_s<SChannel::TType::emg>( int);
template const char* SChannel::channel_s<SChannel::TType::ecg>( int);
template const char* SChannel::channel_s<SChannel::TType::erg>( int);



tuple<SChannel::TType, int>
SChannel::
figure_type_and_name( const string& h_)
{
	auto tt = agh::str::tokens( h_, "-");
	if ( tt.size() == 2 ) {
		for ( auto& T : _CT_ )
			if ( all_of( tt.begin(), tt.end(),
				     [&]( const string& t)
				     {
					     return any_of( T.second.begin(), T.second.end(),
							    [&]( const string& h)
							    {
								    return 0 == strcasecmp( h.c_str(), t.c_str());
							    });
				     }) )
				return make_tuple(T.first, 0); // always custom, because it's a compound (EEG) channel like Fpz-Oz
		return make_tuple(TType::other, 0);

	} else if ( tt.size() == 1 ) {
		for ( auto& T : _CT_ )
			for ( size_t i = 0; i < T.second.size(); ++i )
				if ( strcasecmp( T.second[i], h_.c_str()) == 0 ) {
					return make_tuple(T.first, (int)i);
				}
	}

	return make_tuple(TType::other, 0);
}




// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
