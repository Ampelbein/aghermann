/*
 *       File name:  aghermann/ui/misc.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-08-10
 *
 *         Purpose:  general-purpose non-GTK+ functions
 *
 *         License:  GPL
 */

#include <math.h>
#include "misc.hh"

using namespace std;
using namespace agh::ui;

// these are intended for durations, not timestamps
const char*
agh::ui::
snprintf_buf_ts_d( const double d_)
{
	if ( d_ < 1. )
		snprintf_buf_ts_h( d_ * 24);
	else {
		unsigned m_ = lroundf(d_*24*60*60) / 60,
			m = (m_ % 60),
			h = (m_ / 60) % 24,
			d = (m_ / 60 / 24);
		if ( h % 24 == 0 && m % 60 == 0 )
			snprintf_buf( "%ud", d);
		else if ( m % 60 == 0 )
			snprintf_buf( "%ud%uh", d, h);
		else
			snprintf_buf( "%ud%uh%um", d, h, m);
	}
	return global::buf;
}

const char*
agh::ui::
snprintf_buf_ts_h( const double h_)
{
	if ( h_ < 1. )
		snprintf_buf_ts_m( h_ * 60);
	else if ( h_ >= 24. )
		snprintf_buf_ts_d( h_ / 24);
	else {
		unsigned m_ = lroundf( h_*60*60) / 60,
			m = (m_ % 60),
			h = (m_ / 60);
		if ( m % 60 == 0 )
			snprintf_buf( "%uh", h);
		else
			snprintf_buf( "%uh%um", h, m);
	}
	return global::buf;
}

const char*
agh::ui::
snprintf_buf_ts_m( const double m_)
{
	if ( m_ < 1. )
		snprintf_buf_ts_s( m_ * 60);
	else if ( m_ >= 60. )
		snprintf_buf_ts_h( m_ / 60);
	else {
		unsigned s_ = lroundf( m_*60) / 60,
			s = (s_ % 60),
			m = (s_ / 60);
		if ( s % 60 == 0 )
			snprintf_buf( "%um", m);
		else
			snprintf_buf( "%um%us", m, s);
	}
	return global::buf;
}

const char*
agh::ui::
snprintf_buf_ts_s( const double s_)
{
	if ( s_ >= 60. )
		snprintf_buf_ts_m( s_/60);
	else
		snprintf_buf( "%.2gs", s_);
	return global::buf;
}

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:

