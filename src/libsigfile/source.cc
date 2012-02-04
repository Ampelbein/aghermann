// ;-*-C++-*-
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

using namespace std;

sigfile::CSource::CSource( const char* fname,
			   size_t pagesize,
			   int flags)
      : CHypnogram (pagesize)
{
	switch ( _type = source_file_type(fname) ) {
	case TType::bin:
		throw invalid_argument ("Source type 'bin' not yet supported");
	case TType::ascii:
		throw invalid_argument ("Source type 'ascii' not yet supported");
	case TType::edf:
		_obj = new CEDFFile( fname, flags);
		break;
	case TType::edfplus:
		//_obj = new CEDFPlusFile( fname);
		break;
	case TType::unrecognised:
		throw invalid_argument ("Unrecognised source type");
	}

	if ( flags | ~no_ancillary_files ) {
		// CHypnogram::
		CHypnogram::load( sigfile::make_fname_hypnogram(fname, pagesize));
		size_t scorable_pages = _obj->recording_time() / pagesize;  // implicit floor
		if ( CHypnogram::length() != scorable_pages ) {
			// if ( CHypnogram::length() > 0 )
			// 	fprintf( stderr, "CEDFFile(\"%s\"): number of scorable pages @pagesize=%zu (%zu) "
			// 		 "differs from the number read from hypnogram file (%zu); discarding hypnogram\n",
			// 		 fname, pagesize, scorable_pages, CHypnogram::length());
			CHypnogram::_pages.resize( scorable_pages);
		}
	}
}



sigfile::CSource::CSource( CSource&& rv)
      : CHypnogram (rv)
{
	switch ( _type = rv._type ) {
	case TType::bin:
		throw invalid_argument ("Source type 'bin' not yet supported");
	case TType::ascii:
		throw invalid_argument ("Source type 'ascii' not yet supported");
	case TType::edf:
		_obj = new CEDFFile( static_cast<CEDFFile&&>(*rv._obj));
		break;
	case TType::edfplus:
		//_obj = new CEDFPlusFile( *static_cast<CEDFPlusFile*>(rv._obj);
		break;
	case TType::unrecognised:
		throw invalid_argument ("Unrecognised source type");
	default:
		throw invalid_argument (string("Bad source type: ")+to_string( (long long int)rv._type)+'\n');
	}
	delete rv._obj;
	rv._obj = nullptr;
}


sigfile::CSource::~CSource()
{
	if ( _obj ) {
		if ( not (flags() & no_ancillary_files) )
			CHypnogram::save( make_fname_hypnogram());
		delete _obj;
	}
}



sigfile::CSource::TType
sigfile::CSource::source_file_type( const char* fname)
{
	if ( strlen(fname) > 4 && strcasecmp( &fname[strlen(fname)-4], ".edf") == 0 )
		return TType::edf;
	return TType::unrecognised;
}




// eof
