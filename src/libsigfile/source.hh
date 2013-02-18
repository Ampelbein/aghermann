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

#include "source-base.hh"
#include "edf.hh"
//#include "other.hh"
#include "page.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace sigfile {



class CSource
  : public CHypnogram {

	void operator=( const CSource&) = delete;
	CSource () = delete;
    public:
	enum class TType : int {
		unrecognised,
		bin, ascii,
		edf, edfplus,
	};

    private:
	TType	_type;  // rtti is evil
	CSource_base
		*_obj;
    public:
	CSource (const CSource&)
	      : CHypnogram (-1)
		{
			throw invalid_argument("nono");
		}
      // ctor
	enum { no_ancillary_files = 1 };
	CSource (const char* fname, size_t pagesize, int flags = 0);
	CSource (CSource&& rv);
       ~CSource ();

	TType type() const { return _type; }

      // passthrough to obj
	CSource_base& operator()()
		{ return *_obj; }

      // filenames
	string make_fname_hypnogram() const
		{
			return sigfile::make_fname_hypnogram( _obj->filename(), pagesize());
		}

	static TType source_file_type( const char* fname) __attribute__ ((pure));
};


template <typename T = int>
struct SNamedChannel {
	CSource& source;
	T sig_no;
	SNamedChannel (CSource& source_, T sig_no_)
	      : source (source_),
		sig_no (sig_no_)
		{}
	SNamedChannel (const SNamedChannel&) = default;
};



} // namespace sigfile

#endif // _AGH_SOURCE_H

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:

