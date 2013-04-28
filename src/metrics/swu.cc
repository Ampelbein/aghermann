/*
 *       File name:  metrics/swu.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 *
 * Initial version:  2012-11-10
 *
 *         Purpose:  CBinnedSWU methods
 *
 *         License:  GPL
 */


#include <cassert>
#include <unistd.h>

#include "common/lang.hh"
#include "common/fs.hh"
#include "sigproc/sigproc.hh"
#include "libsigfile/source.hh"
#include "swu.hh"

using namespace std;



void
metrics::swu::SPPack::
check() const
{
	metrics::SPPack::check();
}

void
metrics::swu::SPPack::
reset()
{
	metrics::SPPack::reset();
	min_upswing_duration = .3;
}







metrics::swu::CProfile::
CProfile (const sigfile::CTypedSource& F, int sig_no,
	  const SPPack &params)
      : metrics::CProfile (F, sig_no,
			   params.pagesize,
			   params.compute_n_bins(F().samplerate(sig_no))),
	Pp (params)
{
	Pp.check();
}



string
metrics::swu::CProfile::
fname_base() const
{
	DEF_UNIQUE_CHARP (_);
	ASPRINTF( &_,
		  "%s.%s-%lu"
		  ":%zu-%g",
		  _using_F().filename(), _using_F().channel_by_id(_using_sig_no).name(),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.min_upswing_duration);
	string ret {_};
	return ret;
}


string
metrics::swu::CProfile::
mirror_fname() const
{
	DEF_UNIQUE_CHARP (_);
	string basename_dot = agh::fs::make_fname_base (_using_F().filename(), "", true);
	ASPRINTF( &_,
		  "%s.%s-%lu"
		  ":%zu-%g@%zu"
		  ".swu",
		  basename_dot.c_str(), _using_F().channel_by_id(_using_sig_no).name(),
		  _using_F().dirty_signature( _using_sig_no),
		  Pp.pagesize, Pp.min_upswing_duration,
		  sizeof(TFloat));
	string ret {_};
	return ret;
}


int
metrics::swu::CProfile::
go_compute()
{
	_data.resize( pages() * _bins);

	auto dS = sigproc::derivative(
		_using_F().get_signal_filtered( _using_sig_no));

	for ( size_t p = 0; p < pages(); ++p ) {
		auto	a =  p    * (samplerate() * Pp.pagesize),
			z = (p+1) * (samplerate() * Pp.pagesize);
		auto	la = a, lz = a;
		double	Q = 0.;
	      // 1. upswing proper
		// find a stretch of uninterrupted positive values
		for ( auto i = a; i < z; ++i ) {
			double q = 0.;
			auto j = i;
			while ( dS[j] > 0 ) {
				q += dS[j];
				if ( not (j < z) )
					break;
				++j;
			}
			la = i; lz = j;
			double upswing_duration = (lz - la) * samplerate();
			if ( upswing_duration > Pp.min_upswing_duration )
				Q += q;
		}
	      // 2. clean peaks
		

		nmth_bin(p, 0) =
			Q / Pp.pagesize;
	}


	return 0;
}









int
metrics::swu::CProfile::
export_tsv( const string& fname) const
{
	FILE *f = fopen( fname.c_str(), "w");
	if ( !f )
		return -1;

	auto sttm = _using_F().start_time();
	char *asctime_ = asctime( localtime( &sttm));
	fprintf( f, "## Subject: %s;  Session: %s, Episode: %s recorded %.*s;  Channel: %s\n"
		 "## SWU course (%zu %zu-sec pages)\n"
		 "#Page\tSWU\n",
		 _using_F().subject().name.c_str(), _using_F().session(), _using_F().episode(),
		 (int)strlen(asctime_)-1, asctime_,
		 _using_F().channel_by_id(_using_sig_no).name(),
		 pages(), Pp.pagesize);

	for ( size_t p = 0; p < pages(); ++p )
		fprintf( f, "%zu\t%g\n", p, nmth_bin( p, 0));

	fclose( f);
	return 0;
}




// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
