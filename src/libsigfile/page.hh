// ;-*-C++-*-
/*
 *       File name:  libsigfile/page.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2010-04-28
 *
 *         Purpose:  page and hypnogram classes
 *
 *         License:  GPL
 */


#ifndef _AGH_PAGE_H
#define _AGH_PAGE_H


#include <vector>
#include <array>
#include <stdexcept>

#if HAVE_CONFIG_H && !defined(VERSION)
#  include <config.h>
#endif

using namespace std;

namespace sigfile {


struct SPage {
	enum TScore : unsigned short {
		none,
		nrem1,	nrem2,	nrem3,	nrem4,
		rem,	wake,	mvt,
		_total
	};
	static const char score_codes[TScore::_total];
	static char score_code( TScore i)
		{
			if ( i >= TScore::_total )
				return '?';
			return score_codes[i];
		}
	static const char* const score_names[TScore::_total];
	static const char* score_name( TScore i)
		{
			if ( i >= TScore::_total )
				return "(invalid)";
			return score_names[i];
		}

	static TScore
	__attribute__ ((const))
	char2score( char c)
		{
			size_t i = TScore::none;
			while ( i != TScore::_total && c != score_codes[i] )
				++i;
			return (TScore)i;
		}
	static char score2char( TScore i)
		{
			if ( i >= TScore::_total )
				return '?';
			return score_codes[i];
		}
	static constexpr float mvt_wake_value = .001;

      // class proper
	float	NREM, REM, Wake;
	TScore score() const
		{
			return	 (NREM >  3./4) ? TScore::nrem4
				:(NREM >  1./2) ? TScore::nrem3
				:(REM  >= 1./3) ? TScore::rem
				:(Wake >= 1./3) ? TScore::wake
				:(NREM >  1./4) ? TScore::nrem2
				:(NREM >   .1 ) ? TScore::nrem1
				:(Wake == mvt_wake_value) ? TScore::mvt
				: TScore::none;
		}
	char score_code() const
		{
			return score_codes[(size_t)score()];
		}

	bool has_swa() const
		{
			return (NREM + REM > .2);
		}  // excludes NREM1, in fact
	bool is_nrem() const
		{
			return NREM >= .1;
		}
	bool is_rem() const
		{
			return REM >= 1./3;
		}
	bool is_wake() const
		{
			return Wake >= 1./3;
		}
	bool is_scored() const
		{
			return score() != TScore::none;
		}

	void mark( TScore as)
		{
			switch ( as ) {
			case TScore::nrem1:  NREM = .2, REM = 0., Wake = 0.; break;
			case TScore::nrem2:  NREM = .4, REM = 0., Wake = 0.; break;
			case TScore::nrem3:  NREM = .6, REM = 0., Wake = 0.; break;
			case TScore::nrem4:  NREM = .9, REM = 0., Wake = 0.; break;
			case TScore::rem:    NREM = 0., REM = 1., Wake = 0.; break;
			case TScore::wake:   NREM = 0., REM = 0., Wake = 1.; break;
			case TScore::none:
			default:             NREM = 0., REM = 0., Wake = 0.; break;
			}
		}
	void mark( char as)
		{
			mark( char2score(as));
		}


	SPage( float nrem = 0., float rem = 0., float wake = 0.)
	      : NREM (nrem), REM (rem), Wake (wake)
		{}
};


struct SPageWithSWA : public SPage {
	double	SWA;
	SPageWithSWA( float nrem = 0., float rem = 0., float wake = 0., float swa = 0.)
	      : SPage (nrem, rem, wake),
		SWA (swa)
		{}
	SPageWithSWA( const SPage& p,
		      float swa = 0.)
	      : SPage (p),
		SWA (swa)
		{}
};

struct SPageSimulated : public SPageWithSWA {
	double	S,
		SWA_sim;
	SPageSimulated( float nrem = 0., float rem = 0., float wake = 0.,
			float swa = 0.)
	      : SPageWithSWA (nrem, rem, wake, swa),
		S (0), SWA_sim (swa)
		{}
	SPageSimulated( const SPageWithSWA& p,
			float swa = 0.)
	      : SPageWithSWA (p),
		S (0), SWA_sim (swa)
		{}
};










class CHypnogram {

    protected:
	size_t	_pagesize;
	vector<SPage>
		_pages;
    public:
	CHypnogram() = delete;
	CHypnogram( const CHypnogram&) = default;

	CHypnogram( size_t psize)
	      : _pagesize (psize)
		{}
	CHypnogram( CHypnogram&& rv)
		{
			_pagesize = rv._pagesize;
			swap( _pages, rv._pages);
		}

	SPage& operator[]( size_t i)
		{
			if ( i >= _pages.size() )
				throw out_of_range ("page index out of range");
			return _pages[i];
		}
	const SPage& operator[]( size_t i) const
		{
			if ( i >= _pages.size() )
				throw out_of_range ("page index out of range");
			return _pages[i];
		}

	size_t pagesize() const		{ return _pagesize; }
	size_t length() const		{ return _pages.size(); }
	float percent_scored( float *nrem_p = NULL, float *rem_p = NULL, float *wake_p = NULL) const;

	enum TError : int {
		ok            = 0,
		nofile        = -1,
		baddata       = -2,
		wrongpagesize = -3,
		shortread     = -4
	};
	CHypnogram::TError save( const char*) const;
	CHypnogram::TError load( const char*);
	CHypnogram::TError save( const string& s) const
		{
			return save(s.c_str());
		}
	CHypnogram::TError load( const string& s)
		{
			return load(s.c_str());
		}

	int save_canonical( const char* fname) const;
	typedef array<string, (size_t)SPage::TScore::_total> TCustomScoreCodes;
	int load_canonical( const char* fname)
		{
			return load_canonical( fname,
					       TCustomScoreCodes {{" -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"}});
		}
	int load_canonical( const char* fname,
			    const TCustomScoreCodes&);
};


}


#endif

// EOF
