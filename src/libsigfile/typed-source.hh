/*
 *       File name:  libsigfile/typed-source.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-11-11
 *
 *         Purpose:  generic signal source wrapper/selector
 *
 *         License:  GPL
 */

#ifndef AGH_SIGFILE_SOURCE_H_
#define AGH_SIGFILE_SOURCE_H_

#include "source-base.hh"
#include "page.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;

namespace sigfile {



class CTypedSource
  : public CHypnogram {

	void operator=( const CTypedSource&) = delete;
	CTypedSource () = delete;
    public:
	enum class TType : int {
		unrecognised,
		bin, ascii,
		edf,
	};

    private:
	TType	_type;  // rtti is evil
	CSource
		*_obj;
    public:
	CTypedSource (const CTypedSource&)
	      : CHypnogram (-1)
		{
			throw invalid_argument("nono");
		}
      // ctor
	CTypedSource (const string& fname, size_t pagesize, int flags = 0);
	CTypedSource (CTypedSource&& rv);
       ~CTypedSource ();

	TType type() const { return _type; }

      // passthrough to obj
	CSource& operator()()
		{ return *_obj; }
	const CSource& operator()() const
		{ return *_obj; }

      // filenames
	string make_fname_hypnogram() const
		{
			return sigfile::make_fname_hypnogram( _obj->filename(), pagesize());
		}

	static TType source_file_type( const string& fname) __attribute__ ((pure));
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

#endif

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// tab-width: 8
// End:
