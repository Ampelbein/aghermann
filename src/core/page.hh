// ;-*-C++-*- *  Time-stamp: "2010-11-14 03:37:09 hmmr"

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <functional>
#include <algorithm>
#include <stdexcept>

#include "../common.h"


using namespace std;



struct SPage {
	float	NREM, REM, Wake;
	char	p2score() const
		{
			return	 (NREM >  3./4) ? AghScoreCodes[AGH_SCORE_NREM4]
				:(NREM >  1./2) ? AghScoreCodes[AGH_SCORE_NREM3]
				:(REM  >= 1./3) ? AghScoreCodes[AGH_SCORE_REM  ]
				:(Wake >= 1./3) ? AghScoreCodes[AGH_SCORE_WAKE ]
				:(NREM >  1./4) ? AghScoreCodes[AGH_SCORE_NREM2]
				:(NREM >   .1 ) ? AghScoreCodes[AGH_SCORE_NREM1]
				:(Wake == AGH_MVT_WAKE_VALUE) ? AghScoreCodes[AGH_SCORE_MVT]
				: AghScoreCodes[AGH_SCORE_NONE];
		}
	bool has_swa() const
		{
			return (NREM + REM > .2);
		}  // excludes NREM1, in fact
	bool is_scored() const
		{
			return p2score() != AghScoreCodes[AGH_SCORE_NONE];
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
};

struct SPageSimulated : public SPageWithSWA {
	double	S,
		SWA_sim;
	SPageSimulated( float nrem = 0., float rem = 0., float wake = 0.,
			float swa = 0.)
	      : SPageWithSWA (nrem, rem, wake, swa),
		S (0), SWA_sim (swa)
		{}
};






class CHypnogram {

    protected:
	size_t	_pagesize;
	vector<SPage>
		_pages;
    public:
	CHypnogram( size_t psize = 30,
		    const char* fname = NULL)
	      : _pagesize (psize)
		{
			if ( fname )
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
				throw out_of_range("page index out of range");
			return _pages[i];
		}
	const SPage& nth_page( size_t i) const
		{
			if ( i >= _pages.size() )
				throw out_of_range("page index out of range");
			return _pages[i];
		}
	SPage& operator[]( size_t i)
		{
			return nth_page(i);
		}

	size_t pagesize() const		{ return _pagesize; }
	size_t length() const		{ return _pages.size(); }
	float percent_scored() const
		{
			return (float)count_if( _pages.begin(), _pages.end(),
						mem_fun_ref (&SPage::is_scored)) / _pages.size();
		}

	int save( const char* fname) const;
	int load( const char* fname);

	int save_canonical( const char* fname) const;
	int load_canonical( const char* fname);
};





#endif

// EOF
