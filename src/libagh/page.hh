// ;-*-C++-*- *  Time-stamp: "2011-04-27 02:42:26 hmmr"

/*
 * Author: Andrei Zavada (johnhommer@gmail.com)
 *
 * License: GPL
 *
 * Initial version: 2010-04-28
 */


#ifndef _AGH_PAGE_H
#define _AGH_PAGE_H


#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>

#include "enums.hh"

#if HAVE_CONFIG_H
#  include <config.h>
#endif

using namespace std;

namespace agh {


struct SPage {
	static const char score_codes[(size_t)TScore::_total];
	static const char score_code( TScore i)
		{
			if ( (TScore_underlying_type)i >= (TScore_underlying_type)TScore::_total )
				throw out_of_range ("Score index out of range");
			return score_codes[(TScore_underlying_type)i];
		}
	static const char* const score_names[(size_t)TScore::_total];
	static const char* score_name( TScore i)
		{
			if ( (TScore_underlying_type)i >= (TScore_underlying_type)TScore::_total )
				throw out_of_range ("Score index out of range");
			return score_names[(TScore_underlying_type)i];
		}

	static TScore char2score( char c)
		{
			auto i = (TScore_underlying_type)TScore::_total;
			while ( i && c != score_codes[i] )
				--i;
			return (TScore)i;
		}
	static char score2char( TScore i)
		{
			if ( (TScore_underlying_type)i >= (TScore_underlying_type)TScore::_total )
				throw invalid_argument ("Bad score");
			return score_codes[(size_t)i];
		}
      // class proper
	float	NREM, REM, Wake;
	TScore	score() const
		{
			return	 (NREM >  3./4) ? TScore::nrem4
				:(NREM >  1./2) ? TScore::nrem3
				:(REM  >= 1./3) ? TScore::rem
				:(Wake >= 1./3) ? TScore::wake
				:(NREM >  1./4) ? TScore::nrem2
				:(NREM >   .1 ) ? TScore::nrem1
				:(Wake == AGH_MVT_WAKE_VALUE) ? TScore::mvt
				: TScore::none;
		}
	char	score_code() const
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
			case TScore::wake:   NREM = 0., REM = 0., Wake = 0.; break;
			case TScore::none:
			default:             NREM = 0., REM = 0., Wake = 0.; break;
			}
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
	CHypnogram( size_t psize)
	      : _pagesize (psize)
		{}
	CHypnogram( size_t psize,
		    const string& fname)
	      : _pagesize (psize)
		{
			load( fname);
		}
	CHypnogram( CHypnogram&& rv)
		{
			_pagesize = rv._pagesize;
			swap( _pages, rv._pages);
		}

	SPage& nth_page( size_t i)
		{
			if ( i >= _pages.size() )
				throw out_of_range ("page index out of range");
			return _pages[i];
		}
	const SPage& nth_page( size_t i) const
		{
			if ( i >= _pages.size() )
				throw out_of_range ("page index out of range");
			return _pages[i];
		}
	SPage& operator[]( size_t i)
		{
			return nth_page(i);
		}

	size_t pagesize() const		{ return _pagesize; }
	size_t length() const		{ return _pages.size(); }
	float percent_scored( float *nrem_p = NULL, float *rem_p = NULL, float *wake_p = NULL) const
		{
			if ( nrem_p )
				*nrem_p = (float)count_if( _pages.begin(), _pages.end(),
							   mem_fun_ref (&SPage::is_nrem)) / _pages.size();
			if ( rem_p )
				*rem_p = (float)count_if( _pages.begin(), _pages.end(),
							   mem_fun_ref (&SPage::is_rem)) / _pages.size();
			if ( wake_p )
				*wake_p = (float)count_if( _pages.begin(), _pages.end(),
							   mem_fun_ref (&SPage::is_wake)) / _pages.size();

			return (float)count_if( _pages.begin(), _pages.end(),
						mem_fun_ref (&SPage::is_scored)) / _pages.size() * 100;
		}

	THypnogramError save( const string& fname) const
		{
			return save( fname.c_str());
		}
	THypnogramError load( const string& fname)
		{
			return load( fname.c_str());
		}
	THypnogramError save( const char *fname) const;
	THypnogramError load( const char *fname);

	int save_canonical( const char* fname) const;
	int load_canonical( const char* fname)
		{
			const char *custom_score_codes[] =
				{ " -0", "1", "2", "3", "4", "6Rr8", "Ww5", "mM"};
			return load_canonical( fname, custom_score_codes);
		}
	int load_canonical( const char* fname, const char *custom_score_codes[8]);
};


}


#endif

// EOF
