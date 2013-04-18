/*
 *       File name:  libsigfile/channel.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-10
 *
 *         Purpose:  a string-based class representing a biosig channel
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_CHANNEL_H
#define _SIGFILE_CHANNEL_H

#include <cstring>
#include <array>
#include <string>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace sigfile {

struct SChannel
  : public string {
	template <typename T>
	SChannel (const T& h)
	      : string (h)
		{}
	SChannel ()
		{}

	bool follows_system1020() const
		{
			return channel_follows_system1020( c_str());
		}
	bool operator<( const SChannel& rv) const;

      // static members
	enum TType : int {
		embedded_annotation,
		eeg, eog, emg, ecg, erg,
		nc, meg, mcg, ep, temp, resp, sao2, light, sound, event, freq,
		other
	};
	static const size_t n_channels = 78;
	static const size_t last_eeg_no = 74;
	static const size_t last_eog_no = 76;
	static const size_t last_emg_no = 77;
	static const size_t n_kemp_signal_types = 18;
	static const char* system1020_channels[n_channels];
	static const char* kemp_signal_types[n_kemp_signal_types];
	static bool channel_follows_system1020( const char* channel)
		{
			for ( auto &I : system1020_channels )
				if ( strcmp( channel, I) == 0 )
					return true;
			return false;
		}
	static TType figure_signal_type( const char* s)
		{
			for ( int i = 0; i < (int)n_kemp_signal_types; ++i )
				if ( strcasecmp( kemp_signal_types[i], s) == 0 )
					return (TType)i;
			return TType::other;
		}
	static TType signal_type_of_channel( const string&);
	static bool signal_type_is_fftable( const string& signal_type)
		{
			return signal_type == "EEG";
		}
	static bool signal_type_is_fftable( TType signal_type)
		{
			return signal_type == TType::eeg;
		}
	static bool channel_is_fftable( const string& H)
		{
			return signal_type_is_fftable(
				signal_type_of_channel(H));
		}
};


} // namespace sigfile

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

