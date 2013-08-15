/*
 *       File name:  aghermann/expdesign/subject.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2013-08-14
 *
 *         Purpose:  experimental design primary classes: CSubject
 *
 *         License:  GPL
 */


#include "primaries.hh"


using namespace std;

using namespace agh;


float
CSubject::
age( const string& d) const // age when recordings in this session were made
{
	if ( alg::member(d, measurements) &&
	     measurements.at(d).episodes.size() > 0 )
		return age_rel(
			measurements.at(d).episodes.front().start_time());
	else
		return -1.;
}



float
CSubject::
age() const
{
	time_t now = time(NULL);
	if ( unlikely (now == -1) ) {
		perror( "What's wrong with localtime? ");
		return 21.;
	}
	return age_rel(now);
}

float
CSubject::
age_rel( time_t rel) const
{
	return (difftime(rel, dob))/365.25/24/60/60;
}






CSubject::SEpisode::
SEpisode (sigfile::CTypedSource&& F_,
	  const metrics::psd::SPPack& fft_params,
	  const metrics::swu::SPPack& swu_params,
	  const metrics::mc::SPPack& mc_params)
{
      // move it in place
	sources.emplace_back( move(F_));
	auto& F = sources.back();
	auto HH = F().channel_list();
	printf( "CSubject::SEpisode::SEpisode( \"%s\"): %s\n",
		F().filename(), sigfile::join_channel_names(HH, ", ").c_str());
	int h = 0;
	for ( auto& H : HH )
		recordings.insert( {H, {F, h++, fft_params, swu_params, mc_params}});
}


list<CSubject::SEpisode::SAnnotation>
CSubject::SEpisode::
get_annotations() const
{
	list<agh::CSubject::SEpisode::SAnnotation>
		ret;
	for ( auto &F : sources ) {
		auto HH = F().channel_list();
		for ( size_t h = 0; h < HH.size(); ++h ) {
			auto &AA = F().annotations(h);
			for ( auto &A : AA )
				ret.emplace_back( F(), h, A);
		}
		for ( auto& A : F().annotations() )
			ret.emplace_back( F(), -1, A);
	}
	ret.sort();
	return ret;
}



const CSubject::SEpisode&
CSubject::SEpisodeSequence::
operator[]( const string& e) const
{
	auto E = find( episodes.begin(), episodes.end(), e);
	if ( E != episodes.end() )
		return *E;
	else
		throw invalid_argument( string("no such episode: ") + e);
}

CSubject::SEpisode&
CSubject::SEpisodeSequence::
operator[]( const string& e)
{
	auto E = find( episodes.begin(), episodes.end(), e);
	if ( E != episodes.end() )
		return *E;
	else // or don't throw, go and make one?
		throw invalid_argument( string("no such episode: ") + e);
	// no, let it be created in
	// CExpDesign::add_measurement, when
	// episode start/end times are known
}




// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// c-basic-offset: 8
// End:
