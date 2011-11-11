// ;-*-C++-*-
/*
 *       File name:  libsigfile/source.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-08
 *
 *         Purpose:  generic signal source
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_SOURCE_H
#define _SIGFILE_SOURCE_H

//#include "other.hh"
#include "edf.hh"
#include "page.hh"
#include "psd.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace sigfile {


class CSource
  : public CHypnogram {
	enum class TType : int {
		bin, ascii,
		edf, edfplus,
	};

	TType _type;
	ISource	*_obj;

    public:
      // ctor
	CSource() = delete;
	CSource( const CSource&)
	      : CHypnogram (-1, "")
		{
			throw invalid_argument("nono");
		}
	CSource( CSource&& rv)
	      : CHypnogram ((CHypnogram&&)rv)
		{
			switch ( _type = rv._type ) {
			case TType::bin:
				throw invalid_argument ("Source type 'bin' not yet supported");
			case TType::ascii:
				throw invalid_argument ("Source type 'ascii' not yet supported");
			case TType::edf:
				_obj = new CEDFFile( *rv._obj);
				break;
			case TType::edfplus:
				_obj = new CEDFPlusFile( *rv._obj);
				break;
			}
			delete rv._obj;
			rv._obj = nullptr;
		}

	CSource( const char* fname,
		 int pagesize)
	      : CHypnogram (make_fname_hypnogram(fname, pagesize))
		{
			switch ( source_file_type(file) ) {
			case TType::bin:
				throw invalid_argument ("Source type 'bin' not yet supported");
			case TType::ascii:
				throw invalid_argument ("Source type 'ascii' not yet supported");
			case TType::edf:
				_obj = new CFittedSource<edf::CEDFFile>( fname);
				break;
			case TType::edfplus:
				_obj = new CFittedSource<edf::CEDFPlusFile>( fname);
				break;
			}
		      // CHypnogram::
			size_t scorable_pages = _obj->recording_time() / pagesize;  // with implicit floor
			if ( CHypnogram::length() != scorable_pages ) {
				if ( CHypnogram::length() > 0 )
					fprintf( stderr, "CEDFFile(\"%s\"): number of scorable pages @pagesize=%zu (%zu) differs from the number read from hypnogram file (%zu); discarding hypnogram\n",
						 fname, pagesize, scorable_pages, CHypnogram::length());
				CHypnogram::_pages.resize( scorable_pages);
			}
		}

       ~CSource()
		{
			if ( _obj )
				delete _obj;
		}

	TType type() const
		{
			return _type;
		}

	const char *filename() const
		{
			return _obj->filename();
		}

      // filenames
	string make_fname_hypnogram() const
		{
			return ::make_fname_hypnogram( filename(), pagesize());
		}
	string make_fname_artifacts( const string& channel) const
		{
			return ::make_fname_artifacts( _filename, channel);
		}
	string make_fname_annotations( const string& channel) const
		{
			return ::make_fname_annotations( _filename, channel);
		}


};



// inline methods of CBinnedPower
inline size_t
sigfile::CBinnedPower::n_bins() const
{
	if ( !_using_F )
		return 0;
	auto smplrt = _using_F->samplerate(_using_sig_no);
	return (smplrt * pagesize() + 1) / 2 / smplrt / bin_size;
}





} // namespace sigfile

#endif // _AGH_SOURCE_H

// eof
