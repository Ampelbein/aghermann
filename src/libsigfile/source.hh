// ;-*-C++-*-
/*
 *       File name:  libsigfile/source.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-11
 *
 *         Purpose:  generic signal source
 *
 *         License:  GPL
 */

#ifndef _SIGFILE_SOURCE_H
#define _SIGFILE_SOURCE_H

#include "edf.hh"
//#include "other.hh"

#if HAVE_CONFIG_H
#  include "config.h"
#endif


using namespace std;

namespace sigfile {

template<class T>
string
make_fname__common( const T& _filename, bool hidden)
{
	string	fname_ (_filename);
	if ( fname_.size() > 4 && strcasecmp( &fname_[fname_.size()-4], ".edf") == 0 )
		fname_.erase( fname_.size()-4, 4);
	if ( hidden ) {
		size_t slash_at = fname_.rfind('/');
		if ( slash_at < fname_.size() )
			fname_.insert( slash_at+1, ".");
	}
	return fname_;
}

template<class T>
string
make_fname_hypnogram( const T& _filename, size_t pagesize)
{
	return make_fname__common( _filename, true)
		+ "-" + to_string( (long long unsigned)pagesize) + ".hypnogram";
}

template<class T>
string
make_fname_artifacts( const T& _filename, const string& channel)
{
	return make_fname__common( _filename, true)
		+ "-" + channel + ".af";
}

template<class T>
string
make_fname_annotations( const T& _filename, const T& channel)
{
	return make_fname__common( _filename, true)
		+ "-" + channel + ".annotations";
}

template<class T>
string
make_fname_filters( const T& _filename)
{
	return make_fname__common( _filename, true)
		+ ".filters";
}





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
		{
			throw invalid_argument("nono");
		}
	CSource( CSource&& rv)
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
	      : CHypnogram (make_fname_hypnogram(fname, pagesize)) (
		{
			switch ( source_file_type(file) ) {
			case TType::bin:
				throw invalid_argument ("Source type 'bin' not yet supported");
			case TType::ascii:
				throw invalid_argument ("Source type 'ascii' not yet supported");
			case TType::edf:
				_obj = new CEDFFile( fname);
				break;
			case TType::edfplus:
				_obj = new CEDFPlusFile( fname);
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

	TSourceType type() const
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

} // namespace sigfile

#endif // _AGH_SOURCE_H

// eof
