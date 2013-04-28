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

#include <cstdarg>
#include "libsigfile/source.hh"
#include "metrics/psd.hh"
#include "metrics/swu.hh"
#include "metrics/mc.hh"
#include "model/forward-decls.hh"
#include "forward-decls.hh"

namespace agh {

using namespace std;


struct SProfileParamSet {
	metrics::TType
		metric;
	const char*
	metric_name() const
		{
			return metrics::name(metric);
		}

	struct PSD {
		double	freq_from,
			freq_upto;
	};
	struct MC {
		double	f0;
	};
	struct SWU {
		double	f0;
	};

	union {
		PSD psd;
		MC  mc;
		SWU swu;
	} P;

	double	req_percent_scored;
	size_t	swa_laden_pages_before_SWA_0;
	bool	score_unscored_as_wake;

	SProfileParamSet (const SProfileParamSet::PSD& psd_,
			  double req_percent_scored_ = 90.,
			  size_t swa_laden_pages_before_SWA_0_ = 3,
			  bool	score_unscored_as_wake_ = true)
	      : metric (metrics::TType::psd),
		req_percent_scored (req_percent_scored_),
		swa_laden_pages_before_SWA_0 (swa_laden_pages_before_SWA_0_),
		score_unscored_as_wake (score_unscored_as_wake_)
		{
			P.psd = psd_;
		}
	SProfileParamSet (const SProfileParamSet::SWU& swu_,
			  double req_percent_scored_ = 90.,
			  size_t swa_laden_pages_before_SWA_0_ = 3,
			  bool	score_unscored_as_wake_ = true)
	      : metric (metrics::TType::swu),
		req_percent_scored (req_percent_scored_),
		swa_laden_pages_before_SWA_0 (swa_laden_pages_before_SWA_0_),
		score_unscored_as_wake (score_unscored_as_wake_)
		{
			P.swu = swu_;
		}
	SProfileParamSet (const SProfileParamSet::MC& mc_,
			  double req_percent_scored_ = 90.,
			  size_t swa_laden_pages_before_SWA_0_ = 3,
			  bool	score_unscored_as_wake_ = true)
	      : metric (metrics::TType::mc),
		req_percent_scored (req_percent_scored_),
		swa_laden_pages_before_SWA_0 (swa_laden_pages_before_SWA_0_),
		score_unscored_as_wake (score_unscored_as_wake_)
		{
			P.mc = mc_;
		}

	string display_name() const;

	// silly stl requirements
	SProfileParamSet ()
		{} // if initialised as part of a class with us as base, exception already thrown by those
	bool operator<( const SProfileParamSet&) const
		{
			return false;
		}
};

template<metrics::TType t>
SProfileParamSet
make_profile_paramset(double, ...);

template<>
inline SProfileParamSet
make_profile_paramset<metrics::TType::psd>(double freq_from, ...)
{
	va_list ap;
	va_start (ap, freq_from);
	double freq_upto = va_arg (ap, double);
	va_end (ap);
	return SProfileParamSet (SProfileParamSet::PSD {freq_from, freq_upto});
}

template<>
inline SProfileParamSet
make_profile_paramset<metrics::TType::swu>(double f0, ...)
{
	return SProfileParamSet (SProfileParamSet::SWU {f0});
}

template<>
inline SProfileParamSet
make_profile_paramset<metrics::TType::mc>(double f0, ...)
{
	return SProfileParamSet (SProfileParamSet::MC {f0});
}



class CRecording {

	CRecording () = delete;
	void operator=( const CRecording&) = delete;

    public:
	CRecording (const CRecording& rv) // needed for map
	      : psd_profile (rv.psd_profile),
		swu_profile (rv.swu_profile),
		mc_profile  (rv.mc_profile),
		uc_params (nullptr),
		_status (rv._status),
		_source (rv._source),
		_sig_no (rv._sig_no)
		{}
	CRecording (sigfile::CTypedSource& F, int sig_no,
		    const metrics::psd::SPPack&,
		    const metrics::swu::SPPack&,
		    const metrics::mc::SPPack&);
       ~CRecording ();

	const char* subject() const      {  return _source().subject().name.c_str(); }
	const char* session() const      {  return _source().session(); }
	const char* episode() const      {  return _source().episode(); }
	const char* channel() const      {  return _source().channel_by_id(_sig_no).name(); }

	sigfile::SChannel::TType signal_type() const
		{
			return _source().signal_type(_sig_no);
		}

	const sigfile::CSource& F() const
		{
			return _source();
		}
	sigfile::CSource& F()  // although we shouldn't want to access CEDFFile writably from CRecording,
		{      // this shortcut saves us the trouble of AghCC->subject_by_x(,,,).measurements...
			return _source();  // on behalf of aghui::SChannelPresentation
		}
	const sigfile::CHypnogram&
	hypnogram() const
		{
			return *(sigfile::CHypnogram*)&_source;
		}

	int h() const
		{
			return _sig_no;
		}

	bool operator<( const CRecording &o) const
		{
			return _source().end_time() < o._source().start_time();
		}

	time_t start() const
		{
			return _source().start_time();
		}
	time_t end() const
		{
			return _source().end_time();
		}

	// this one damn identical in all bases
	size_t pagesize() const
		{
			return ((metrics::psd::CProfile*)this) -> Pp.pagesize;
		}

	// actual page counts based on actual edf samples
	size_t total_pages() const
		{
			return _source().recording_time() / pagesize();
		}
	size_t full_pages() const
		{
			return round(_source().recording_time() / pagesize());
		}
	size_t total_samples() const
		{
			return _source().recording_time() * _source().samplerate(_sig_no);
		}

	valarray<TFloat>
	course( const SProfileParamSet::PSD&);

	valarray<TFloat>
	course( const SProfileParamSet::SWU&);

	valarray<TFloat>
	course( const SProfileParamSet::MC&);

	valarray<TFloat>
	course( const SProfileParamSet& P)
		{
			switch ( P.metric ) {
			case metrics::TType::psd:
				return course( P.P.psd);
			case metrics::TType::swu:
				return course( P.P.swu);
			case metrics::TType::mc:
				return course( P.P.mc);
			default:
				throw runtime_error ("What metric?");
			}
		}

	bool
	need_compute( const SProfileParamSet& P)
		{
			switch ( P.metric ) {
			case metrics::TType::psd:
				return psd_profile.need_compute();
			case metrics::TType::swu:
				return swu_profile.need_compute();
			case metrics::TType::mc:
				return mc_profile.need_compute();
			default:
				throw runtime_error ("What metric?");
			}
		}

	metrics::psd::CProfile psd_profile;
	metrics::swu::CProfile swu_profile;
	metrics::mc::CProfile	mc_profile;

	bool have_uc_determined() const
		{
			return uc_params;
		}
	agh::beersma::SUltradianCycle
		*uc_params;

    protected:
	int	_status;

	sigfile::CTypedSource&
		_source;
	int	_sig_no;
};






inline valarray<TFloat>
CRecording::
course( const SProfileParamSet::PSD& p)
{
	return (psd_profile.compute(),
		psd_profile.course( p.freq_from, p.freq_upto));
}

inline valarray<TFloat>
CRecording::
course( const SProfileParamSet::SWU& p)
{
	return (swu_profile.compute(),
		swu_profile.course( p.f0));
}

inline valarray<TFloat>
CRecording::
course( const SProfileParamSet::MC& p)
{
	return (mc_profile.compute(),
		mc_profile.course( p.f0));
}


} // namespace agh

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
