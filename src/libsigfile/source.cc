/*
 *       File name:  libsigfile/source.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-14
 *
 *         Purpose:  wrapper class for various biosignals (edf, edf+ etc)
 *
 *         License:  GPL
 */


#include "source.hh"
#include "edf.hh"
#include "tsv.hh"

using namespace std;

using sigfile::CSource;
using sigfile::CTypedSource;
using sigfile::CTSVFile;
using sigfile::CEDFFile;


CTypedSource::
CTypedSource (const string& fname,
	      const size_t pagesize,
	      const int flags)
      : CHypnogram (pagesize)
{
	switch ( _type = source_file_type(fname) ) {
	case TType::ascii:
		_obj = new CTSVFile( fname, flags);
		break;
	case TType::edf:
		_obj = new CEDFFile( fname, flags);
		break;

	case TType::bin:
		throw invalid_argument ("Source type 'bin' not supported");
	case TType::unrecognised:
		throw invalid_argument ("Unrecognised source type");
	}

	if ( flags | ~CSource::no_ancillary_files ) {
		// CHypnogram::
		CHypnogram::load( sigfile::make_fname_hypnogram(fname, pagesize));
		size_t scorable_pages = ceil( _obj->recording_time() / pagesize);
		if ( CHypnogram::pages() != scorable_pages ) {
			if ( CHypnogram::pages() > 0 )
				fprintf( stderr, "CSource(\"%s\"): number of scorable pages @pagesize=%zu (%g / %zu = %zu) "
					 "differs from the number read from hypnogram file (%zu); adjusting hypnogram size\n",
					 fname.c_str(), pagesize, _obj->recording_time(), pagesize, scorable_pages, CHypnogram::pages());
			CHypnogram::_pages.resize( scorable_pages);
		}
	}
}



CTypedSource::
CTypedSource (CTypedSource&& rv)
      : CHypnogram (move(rv))
{
	_type   = rv._type;
	_obj    = rv._obj;
	rv._obj = nullptr;
}


CTypedSource::
~CTypedSource ()
{
	if ( _obj ) {
		if ( not (_obj->_flags & CSource::no_ancillary_files) )
			CHypnogram::save( make_fname_hypnogram());
		delete _obj;
	}
}



CTypedSource::TType
CTypedSource::
source_file_type( const string& fname)
{
	if ( fname.size() > 4 &&
	     strcasecmp( &fname[fname.size()-4], ".edf") == 0 )
		return TType::edf;

	if ( fname.size() > 4 &&
	     (strcasecmp( &fname[fname.size()-4], ".tsv") == 0 ||
	      strcasecmp( &fname[fname.size()-4], ".csv") == 0 ) )
		return TType::ascii;

	return TType::unrecognised;
}


// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
