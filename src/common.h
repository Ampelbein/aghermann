// ;-*-C-*- *  Time-stamp: "2010-10-21 02:57:01 hmmr"
/*
 *       File name:  core/common.h
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-07-01
 *
 *         Purpose:  Common enums, defines and structures used in core and ui
 *
 *         License:  GPL
 */


#ifndef AGH_ENUMS_H
#define AGH_ENUMS_H

#define FAFA printf( __FILE__ " %d: fafa\n", __LINE__)

inline size_t
min( size_t a, size_t b)
{
	if ( a < b )
		return a;
	else
		return b;
}

typedef enum {
	AGH_J_GENDER_NEUTER = 'o',
	AGH_J_GENDER_MALE   = 'M',
	AGH_J_GENDER_FEMALE = 'F'
} TGender;


typedef enum {
	AGH_HYP_NOFILE        = -1,
	AGH_HYP_BADDATA       = -2,
	AGH_HYP_WRONGPAGESIZE = -3,
	AGH_HYP_SHORTREAD     = -4,
} THypnogramRetval;



#define AGH_EDFCHK_BAD_HEADER		(1 <<  0)
#define AGH_EDFCHK_BAD_VERSION		(1 <<  1)
#define AGH_EDFCHK_BAD_NUMFLD		(1 <<  2)
#define AGH_EDFCHK_BAD_RECORDING	(1 <<  3)
#define AGH_EDFCHK_DATE_UNPARSABLE	(1 <<  4)
#define AGH_EDFCHK_TIME_UNPARSABLE	(1 <<  5)
#define AGH_EDFCHK_NOSESSION		(1 <<  6)
#define AGH_EDFCHK_NOEPISODE		(1 <<  7)
#define AGH_EDFCHK_NONKEMP_SIGNALTYPE	(1 <<  8)
#define AGH_EDFCHK_NON1020_CHANNEL	(1 <<  9)
#define AGH_EDFCHK_NOCHANNEL		(1 << 10)
#define AGH_EDFCHK_DUP_CHANNELS		(1 << 11)
#define AGH_EDFCHK_NOGAIN		(1 << 12)
#define AGH_EDFCHK_SYSFAIL		(1 << 13)



typedef enum {
	AGH_SCORE_NONE,
	AGH_SCORE_NREM1,
	AGH_SCORE_NREM2,
	AGH_SCORE_NREM3,
	AGH_SCORE_NREM4,
	AGH_SCORE_REM,
	AGH_SCORE_WAKE,
	AGH_SCORE_MVT
} TScores;

extern const char AghScoreCodes[];


typedef enum {
	AGH_WT_BARTLETT,
	AGH_WT_BLACKMAN,
	AGH_WT_BLACKMAN_HARRIS,
	AGH_WT_HAMMING,
	AGH_WT_HANNING,
	AGH_WT_PARZEN,
	AGH_WT_SQUARE,
	AGH_WT_WELCH,
	AGH_WT_N_TYPES
} TFFTWinType;



typedef enum {
	_rs_,	_rc_,
	_fcR_,	_fcW_,
	_S0_,	_SU_,
	_ta_,	_tp_,
	_gc_,
	_agh_basic_tunables_,
	_gc1_ = _gc_,
	_gc2_,
	_gc3_,
	_gc4_,
	_agh_n_tunables_
} TTunable;


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



enum {
	AGH_SIMPREP_ENOSCORE		= -5,
	AGH_SIMPREP_EFARAPART		= -6,
	AGH_SIMPREP_ESIGTYPE		= -7,
	AGH_SIMPREP_ETOOMANYMSMT	= -8,
	AGH_SIMPREP_ENOSWA		= -9,
	AGH_SIMPREP_EAMENDMENTS_INEFFECTIVE	= -10,
	AGH_SIMPREP_ERS_NONSENSICAL		= -11,
	AGH_SIMPREP_ENEGOFFSET			= -12,
	AGH_SIMPREP_EUNEQ_PAGESIZE		= -13
};




#define AGH_MVT_WAKE_VALUE	.001

// enum {
// 	AGH_BATCHRUN_REDOSKIP,
// 	AGH_BATCHRUN_REDOFAILED,
// 	AGH_BATCHRUN_REDOSUCCEEDED,
// 	AGH_BATCHRUN_REDOALWAYS,
// 	AGH_BATCHRUN_N_REDO_OPTIONS
// };


#endif

// EOF