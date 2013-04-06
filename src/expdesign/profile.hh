/*
 *       File name:  expdesign/profile.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-11-24
 *
 *         Purpose:  A list of CRecording's
 *
 *         License:  GPL
 */


#ifndef _AGH_EXPDESIGN_PROFILE_H
#define _AGH_EXPDESIGN_PROFILE_H

#include "recording.hh"

namespace agh {

using namespace std;




class CProfile
  : private SProfileParamSet {

    public:
	CProfile (CRecording&,
		  const SProfileParamSet&);
	CProfile (CSubject&, const string& d, const sigfile::SChannel& h,
		  const SProfileParamSet&);
	void create_timeline( const SProfileParamSet& params)
		{
			*(SProfileParamSet*)this = params;
			create_timeline();
		}
	void create_timeline();
	bool need_compute( const SProfileParamSet&);

	const SProfileParamSet& P() const
					{ return *this; }
	size_t sim_start() const	{ return _sim_start; }
	size_t sim_end() const		{ return _sim_end; }
	size_t baseline_end() const	{ return _baseline_end; }
	size_t pages_with_swa() const	{ return _pages_with_SWA; }
	size_t pages_non_wake() const	{ return _pages_non_wake; }
	size_t pages_in_bed() const	{ return _pages_in_bed; }
	double SWA_L() const		{ return _SWA_L; }
	double SWA_0() const		{ return _SWA_0; }
	double SWA_100() const		{ return _SWA_100; }
	double metric_avg() const	{ return _metric_avg; }

	const vector<sigfile::SPageSimulated>&
	timeline() const		{ return _timeline; }

	typedef pair<size_t, size_t> TBounds;
	const vector<TBounds>&
	mm_bounds() const		{ return _mm_bounds; }

	const vector<CRecording*>&
	mm_list() 			{ return _mm_list; }

	const sigfile::SPageSimulated&
	operator[]( size_t p) const
		{
			return _timeline[p];
		}

	time_t nth_episode_start_time( size_t n) const;
	time_t nth_episode_end_time( size_t n) const;
	size_t nth_episode_start_page( size_t n) const;
	size_t nth_episode_end_page( size_t n) const;

	size_t pagesize() const
		{
			return _pagesize;
		}

	const char* subject() const;
	const char* session() const;
	const char* channel() const;

	enum TFlags {
		ok			= 0,
		enoscore		= 1,
		efarapart		= 2,
		esigtype		= 4,
		etoomanymsmt		= 8,
		enoswa			= 16,
		eamendments_ineffective	= 32,
		ers_nonsensical		= 64,
		enegoffset		= 128,
		euneq_pagesize		= 256
	};

	static string explain_status( int);

    protected:
	int	_status;

	CProfile (const CProfile&) = delete;
	CProfile ()
		{
			throw runtime_error ("nono");
		}
	CProfile (CProfile&& rv);

	size_t	_sim_start,
		_sim_end,
		_baseline_end,
		_pages_with_SWA,
		_pages_non_wake,
		_pages_in_bed;
	double	_SWA_L,
		_SWA_0,	_SWA_100,
		_metric_avg;

	time_t	_0at;
	vector<sigfile::SPageSimulated>
		_timeline;
	vector<TBounds>  // in pages
		_mm_bounds;

	vector<CRecording*>
		_mm_list;
    private:
	size_t	_pagesize;  // since power is binned each time it is
			    // collected in layout_measurements() and
			    // then detached, we keep it here
			    // privately
};




inline const char* CProfile::subject() const { return _mm_list.front()->subject(); }
inline const char* CProfile::session() const { return _mm_list.front()->session(); }
inline const char* CProfile::channel() const { return _mm_list.front()->channel(); }


inline time_t
CProfile::nth_episode_start_time( size_t n) const
{
	return _0at + _mm_bounds[n].first * _pagesize;
}

inline time_t
CProfile::nth_episode_end_time( size_t n) const
{
	return _0at + _mm_bounds[n].second * _pagesize;
}

inline size_t
CProfile::nth_episode_start_page( size_t n) const
{
	return _mm_bounds[n].first;
}

inline size_t
CProfile::nth_episode_end_page( size_t n) const
{
	return _mm_bounds[n].second;
}




} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
