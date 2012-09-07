// ;-*-C++-*-
/*
 *       File name:  expdesign/recording.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-26
 *
 *         Purpose:  experimental design primary classes: CRecording,
 *
 *         License:  GPL
 */


#ifndef _AGH_EXPDESIGN_RECORDING_H
#define _AGH_EXPDESIGN_RECORDING_H

#include "../libsigfile/psd.hh"
#include "../libsigfile/mc.hh"
#include "../libsigfile/source.hh"
#include "../expdesign/forward-decls.hh"

namespace agh {

using namespace std;

class CRecording
  : public sigfile::CBinnedPower,
    public sigfile::CBinnedMC {

    friend class CExpDesign;

    protected:
	int	_status;

	sigfile::CSource&
		_source;
	int	_sig_no;

	CRecording() = delete;
	void operator=( const CRecording&) = delete;

    public:
	const sigfile::CSource&
	F() const
		{
			return _source;
		}
	sigfile::CSource&
	F()  // although we shouldn't want to access CEDFFile writably from CRecording,
		{      // this shortcut saves us the trouble of AghCC->subject_by_x(,,,).measurements...
			return _source;  // on behalf of aghui::SChannelPresentation
		}
	int h() const
		{
			return _sig_no;
		}

	CRecording (sigfile::CSource& F, int sig_no,
		    const sigfile::SFFTParamSet&,
		    const sigfile::SMCParamSet&);

	const char* subject() const      {  return _source.subject(); }
	const char* session() const      {  return _source.session(); }
	const char* episode() const      {  return _source.episode(); }
	const char* channel() const      {  return _source.channel_by_id(_sig_no); }
	sigfile::SChannel::TType signal_type() const
		{  return _source.signal_type(_sig_no); }

	bool operator<( const CRecording &o) const
		{
			return _source.end_time() < o._source.start_time();
		}

	time_t start() const
		{
			return _source.start_time();
		}
	time_t end() const
		{
			return _source.end_time();
		}

	// this one is damn identical in two bases
	size_t
	pagesize() const
		{
			((sigfile::CBinnedPower*)this) -> pagesize();
		}
	// cut through, and cache it please
	template <typename T>
	valarray<T>
	cached_course( sigfile::TMetricType metric, float freq_from, float freq_upto)
		{
			if ( metric    == _cached_metric &&
			     freq_from == _cached_freq_from &&
			     freq_upto == _cached_freq_upto &&
			     not _cached_course.empty() )
				return _cached_course;
			else {
				metric    = _cached_metric;
				freq_from = _cached_freq_from;
				freq_upto = _cached_freq_upto;
				switch ( _cached_metric = metric ) {
				case sigfile::TMetricType::Psd:
					return _cached_course =
						CBinnedPower::course<T>( freq_from, freq_upto);
				case sigfile::TMetricType::Mc:
					return _cached_course =
						CBinnedMC::course<T>(
							min( (size_t)((freq_from) / bandwidth),
							     CBinnedMC::bins()-1));
				default:
					return _cached_course;
				}
			}
    private:
	valarray<TFloat>
		_cached_course;
	sigfile::TMetricType
		_cached_metric;
	float	_cached_freq_from,
		_cached_freq_upto;
};






struct SSCourseParamSet {
	sigfile::TMetricType
		_profile_type;
	float	_freq_from,
		_freq_upto;
	float	_req_percent_scored;
	size_t	_swa_laden_pages_before_SWA_0;
	bool	_ScoreUnscoredAsWake:1;
};


class CSCourse
  : private SSCourseParamSet {

    public:
	CSCourse (const CRecording&,
		  const SSCourseParamSet& params);
	CSCourse (const CSubject&, const string& d, const sigfile::SChannel& h,
		  const SSCourseParamSet& params);
	void create_timeline( const SSCourseParamSet& params)
		{
			*(SSCourseParamSet*)this = params;
			create_timeline();
		}
	void create_timeline();

	sigfile::TMetricType profile_type() const
					{ return _profile_type; }
	float freq_from() const		{ return _freq_from; }
	float freq_upto() const		{ return _freq_upto; }
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

	const vector<const CRecording*>&
	mm_list() const			{ return _mm_list; }

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

	CSCourse (const CSCourse&) = delete;
	CSCourse ()
		{} // easier than the default; not used anyway
	CSCourse (CSCourse&& rv);

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

	vector<const CRecording*>
		_mm_list;
    private:
	size_t	_pagesize;  // since power is binned each time it is
			    // collected in layout_measurements() and
			    // then detached, we keep it here
			    // privately
};


inline const char* CSCourse::subject() const { return _mm_list.front()->subject(); }
inline const char* CSCourse::session() const { return _mm_list.front()->session(); }
inline const char* CSCourse::channel() const { return _mm_list.front()->channel(); }



inline time_t
CSCourse::nth_episode_start_time( size_t n) const
{
	return _0at + _mm_bounds[n].first * _pagesize;
}

inline time_t
CSCourse::nth_episode_end_time( size_t n) const
{
	return _0at + _mm_bounds[n].second * _pagesize;
}

inline size_t
CSCourse::nth_episode_start_page( size_t n) const
{
	return _mm_bounds[n].first;
}

inline size_t
CSCourse::nth_episode_end_page( size_t n) const
{
	return _mm_bounds[n].second;
}




} // namespace agh

#endif

// eof
