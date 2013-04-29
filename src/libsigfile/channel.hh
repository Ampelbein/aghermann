/*
 *       File name:  libsigfile/channel.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-10
 *
 *         Purpose:  representation of a biosig channel
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_CHANNEL_H
#define _SIGFILE_CHANNEL_H

#include <cstring>
#include <tuple>
#include <string>
#include <sstream>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace sigfile {


// we want scoped enums with basic arith support, so:
namespace EEG {
enum E : int {
	invalid = -1,
	custom = 0,
	first,

	Nz = first,
	Fp1, Fpz, Fp2,
	AF7, AF3, AFz, AF4, AF8,
	F9,  F7, F5, F3, F1, Fz, F2, F4, F6, F8, F10,
	FT9, FT7, FC5, FC3, FC1, FCz, FC2, FC4, FC6, FCT8, FT10,
	A1, T9, T7, C5, C3, C1, Cz, C2, C4, C6, T8, T10, A2,
	TP9, TP7, CP5, CP3, CP1, CPz, CP2, CP4, CP6, TP8, TP10,
	P9, P7, P5, P3, P1, Pz, P2, P4, P6, P8, P10,
	PO7, PO3, POz, PO4, PO8,
	O1, Oz, O2,
	Iz,

	last = Iz,
	total
};
}

namespace EOG {
enum E : int {
	invalid = -1,
	custom = 0,
	first,
	left = first, right,
	last = right,
	total
};
}

namespace EMG {
enum E : int {
	invalid = -1,
	custom = 0,
	first,
	chin = first,
	last = chin,
	total
};
}

namespace ECG {
enum E : int {
	invalid = -1,
	custom = 0,
	total
};
}

namespace ERG {
enum E : int {
	invalid = -1,
	custom = 0,
	total
};
}
// moar types ...


extern const char* edf_annotations_label;


struct SChannel {

	enum class TType {
		invalid,
		embedded_annotation,
		eeg, eog, emg, ecg, erg,
		nc, meg, mcg, ep, temp, resp, sao2, light, sound, event, freq,
		other
	};

	static const char* type_s( TType t);

	template <TType T>
	static const char* channel_s( int);

	static tuple<TType, int> figure_type_and_name( const string&);

	static bool is_fftable( TType type)
		{
			return type == TType::eeg;
		}

      // ctor
	SChannel (const string& h)
		{
			tie(_type, _idx) = figure_type_and_name(h);
			if ( _type == TType::invalid ) {
				_type = TType::other;
				_idx = 0; // custom
				_custom_name = h;
			}
		}

	SChannel (TType type_, int idx_)
	      : _type (type_),
		_idx (idx_)
		{}
	SChannel (TType type_, const string& custom_name_)
	      : _type (type_),
		_idx (0),
		_custom_name (custom_name_)
		{}
	SChannel () = default;

	TType type() const
		{ return _type; }
	const char* type_s() const
		{ return type_s(_type); }

	const char* name() const
		{
			if ( _custom_name.empty() )
				switch ( _type ) {
				case TType::eeg: return channel_s<TType::eeg>( _idx);
				case TType::eog: return channel_s<TType::eog>( _idx);
				case TType::emg: return channel_s<TType::emg>( _idx);
				case TType::ecg: return channel_s<TType::ecg>( _idx);
				case TType::erg: return channel_s<TType::erg>( _idx);
				default: return "(unknown)";
				}
			else
				return _custom_name.c_str();
		}
	const char* c_str() const
		{ return name(); }
	int idx() const
		{ return _idx; }

	bool is_fftable() const
		{
			return is_fftable( _type);
		}
    private:
	TType	_type;
	int	_idx;
	string	_custom_name;

    public:
	// compares by channel actual locations, antero-posteriorly
	bool operator<( const SChannel& rv) const
		{
			if ( _type == rv._type ) {
				if ( _idx > 0 && rv._idx > 0 )
					return _idx < rv._idx;
				else if ( _idx > 0 )
					return true;
				else
					return _custom_name < rv._custom_name;
			} else
				return _type < rv._type;
		}
	bool operator==( const SChannel& rv) const
		{
			return _type == rv._type && _idx == rv._idx;
		}
	bool operator==( const char* rv) const
		{
			return 0 == strcasecmp( name(), rv);
		}
};

template <typename C>
string
join_channel_names( const C& l, const char* sep)
{
	if ( l.empty() )
		return "";
	ostringstream recv;
	auto I = l.begin();
	for ( ; next(I) != l.end(); ++I )
		recv << I->name() << sep;
	recv << I->name();
	return recv.str();
}


} // namespace sigfile

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

