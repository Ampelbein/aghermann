// ;-*-C++-*- *  Time-stamp: "2011-05-10 22:37:45 hmmr"
/*
 *       File name:  common.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  Common enums, defines and structures used in core and ui
 *
 *         License:  GPL
 */


#ifndef AGH_ENUMS_H
#define AGH_ENUMS_H

#include <stdexcept>

#if HAVE_CONFIG_H
#  include "config.h"
#endif


#define FAFA printf( __FILE__ ":%d (%s): fafa\n", __LINE__, __FUNCTION__);


namespace agh {

enum class TGender : char {
	neuter = 'o',
	male   = 'M',
	female = 'F'
};


enum class THypnogramError : int {
	ok            = 0,
	nofile        = -1,
	baddata       = -2,
	wrongpagesize = -3,
	shortread     = -4
};


enum TEdfStatus : int {
	ok			= 0,
	bad_header		= (1 <<  0),
	bad_version		= (1 <<  1),
	bad_numfld		= (1 <<  2),
	bad_recording		= (1 <<  3),
	date_unparsable		= (1 <<  4),
	time_unparsable		= (1 <<  5),
	nosession		= (1 <<  6),
	noepisode		= (1 <<  7),
	nonkemp_signaltype	= (1 <<  8),
	non1020_channel		= (1 <<  9),
	dup_channels		= (1 << 11),
	nogain			= (1 << 12),
	sysfail			= (1 << 13),
	inoperable		= (bad_header
				   | bad_version
				   | bad_numfld
				   | bad_recording
				   | date_unparsable | time_unparsable
				   | dup_channels
				   | nogain
				   | sysfail)
};


typedef unsigned short TBand_underlying_type;
enum class TBand : TBand_underlying_type {
	delta,
	theta,
	alpha,
	beta,
	gamma,
	_total,
};

inline TBand
next( TBand& b)
{
	if ( b == TBand::gamma )
		throw std::out_of_range ("gamma is the last band");
	return b = (TBand) ((TBand_underlying_type)b+1);
}
inline TBand
prev( TBand& b)
{
	if ( b == TBand::delta )
		throw std::out_of_range ("delta is the first band");
	return b = (TBand) ((TBand_underlying_type)b-1);
}



typedef unsigned short TScore_underlying_type;
enum class TScore : TScore_underlying_type {
	none,
	nrem1,
	nrem2,
	nrem3,
	nrem4,
	rem,
	wake,
	mvt,
	_total
};
inline TScore
next( TScore& b)
{
	if ( b == TScore::mvt )
		throw std::out_of_range ("mvt is the last score");
	return b = (TScore) ((TScore_underlying_type)b+1);
}


enum class TFFTWinType : unsigned short {
	bartlett,
	blackman,
	blackman_harris,
	hamming,
	hanning,
	parzen,
	square,
	welch,
	_total
};


// template <class IntT>
// inline TFFTWinType
// operator=( TFFTWinType lv, IntT rv)
// {
// 	return lv = (TFFTWinType)rv;
// }

typedef unsigned short TTunable_underlying_type;
enum class TTunable : TTunable_underlying_type {
	rs,	rc,
	fcR,	fcW,
	S0,	SU,
	ta,	tp,
	gc,
	_basic_tunables,
	gc1 = gc,
	gc2,
	gc3,
	gc4,
	_all_tunables
};





struct STunableDescription {
	double	def_val, def_min, def_max, def_step;
	float	display_scale_factor;
	int	is_required;
	int	time_adj;
	const char
		*name,
		*fmt,
		*unit;
};

enum class TTIdx : unsigned {
	val,
	min,
	max,
	step
};



enum class TSimPrepError : int {
	ok			= 0,
	enoscore		= -5,
	efarapart		= -6,
	esigtype		= -7,
	etoomanymsmt		= -8,
	enoswa			= -9,
	eamendments_ineffective	= -10,
	ers_nonsensical		= -11,
	enegoffset		= -12,
	euneq_pagesize		= -13
};


inline const char*
simprep_perror( TSimPrepError code)
{
	switch ( code ) {
	case TSimPrepError::enoscore:
		return "Insufficiently scored";
	case TSimPrepError::efarapart:
		return "Measurements too far apart";
	case TSimPrepError::esigtype:
		return "Signal is not an EEG";
	case TSimPrepError::etoomanymsmt:
		return "Too many measurements";
	case TSimPrepError::enoswa:
		return "Measurements have no SWA";
	case TSimPrepError::eamendments_ineffective:
		return "Inappropriate amendments";
	case TSimPrepError::ers_nonsensical:
		return "Must have more measurements to estimate rs";
	case TSimPrepError::enegoffset:
		return "Negative offset";
	case TSimPrepError::euneq_pagesize:
		return "Wrong page size";
	default:
		return "(Not a valid simprep code)";
	}
}

#define AGH_MODRUN_TRIED 1


#define AGH_MVT_WAKE_VALUE	.001

// enum {
// 	AGH_BATCHRUN_REDOSKIP,
// 	AGH_BATCHRUN_REDOFAILED,
// 	AGH_BATCHRUN_REDOSUCCEEDED,
// 	AGH_BATCHRUN_REDOALWAYS,
// 	AGH_BATCHRUN_N_REDO_OPTIONS
// };


typedef int TExpDesignState_underlying_type;
enum class TExpDesignState : TExpDesignState_underlying_type {
	ok = 0,
	init_fail = 1,
	load_fail = 2,
};

template <class T>
TExpDesignState operator|( TExpDesignState lv, T rv)
{
	return (TExpDesignState) ((TExpDesignState_underlying_type)lv | (TExpDesignState_underlying_type)rv);
}


}

#endif

// EOF
