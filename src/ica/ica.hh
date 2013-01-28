/*
 *       File name:  ica/ica.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-10-13
 *
 *         Purpose:  Implementation (header) of ICA as described in Bell & Sejnowski (1995)
 *
 *         License:  GPL
 */

#ifndef _AGH_ICA_HH
#define _AGH_ICA_HH

#include <valarray>
#include <vector>
#include <list>
#include <string>
#include <cmath>
#include <functional>

#include <itpp/base/vec.h>
#include <itpp/signal/fastica.h>


#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


namespace itpp {

template <class T>
inline vector<valarray<T>>
to_vecva( const itpp::Mat<T>& rv)
{
	vector<valarray<T>> ret;
	for ( int r = 0; r < rv.rows(); ++r ) {
		itpp::Vec<T> v = rv.get_row(r);
		ret.emplace_back( rv.cols());
		memcpy( &ret.back()[0], &v(0), sizeof(T) * rv.cols());
	}
	return ret;
}

template <class Tl, class Tr>
inline valarray<Tl>
to_va( const itpp::Mat<Tr>& rv, int row)
{
	valarray<Tl> ret;
	itpp::Vec<Tr> v = rv.get_row(row);
	ret.resize( v.size());
	if ( sizeof(Tl) == sizeof(Tr) )
		memcpy( &ret[0], &v(0), sizeof(Tr) * rv.cols());
	else
		for ( int c = 0; c < rv.cols(); ++c )
			ret[c] = rv(row, c);
	return ret;
}

template <class Titpp, class T>
inline void
make_mat_from_vecva( itpp::Mat<Titpp>& lv, const vector<valarray<T>>& rv)
{
	if ( rv.empty() )
		lv.set_size( 0, 0, false);
	else {
		lv.set_size( rv.size(), rv.front().size());
		for ( size_t r = 0; r < rv.size(); ++r ) {
			auto& row = rv[r];
			itpp::Vec<Titpp> tmp;
			if ( sizeof(Titpp) == sizeof(T) )
				tmp = itpp::Vec<Titpp> (&row[0], row.size());
			else {
				tmp.set_size( row.size());
				for ( size_t c = 0; c < row.size(); ++c )
					tmp[c] = row[c];
			}
			lv.set_row( r, tmp);
		}
	}
}

} // namespace itpp



namespace ica {

class CFastICA {
    public:
      // ctor
	template <class T>
	CFastICA (const vector<valarray<T> >& source)
		{
			itpp::Mat<double>
				_source_mat;
			itpp::make_mat_from_vecva<double, T>( _source_mat, source);
			_obj = new itpp::Fast_ICA (_source_mat);
		}
	CFastICA (const vector<function<valarray<TFloat>()>>& source, size_t cols)
	// avoid creating a third temporary, specially for use with agh::CEDFFile::get_signal
		{
			itpp::Mat<double>
				_source_mat (source.size(), cols);
			for ( int r = 0; r < (int)source.size(); ++r ) {
				auto tmp = source[r]();
				tmp -= tmp.sum() / tmp.size();
				for ( int c = 0; c < (int)cols; ++c )
					_source_mat( r, c) = tmp[c];
			}
			_obj = new itpp::Fast_ICA (_source_mat);
		}
       ~CFastICA ()
		{
			delete _obj;
		}

      // do all ops via this proxy
	itpp::Fast_ICA&
	obj()
		{
			return *_obj;
		};
    private:
	itpp::Fast_ICA*
		_obj;
};


}

#endif // _AGH_ICA_HH

// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
