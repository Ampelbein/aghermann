// ;-*-C++-*-
/*
 *       File name:  libica/ica.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-10-13
 *
 *         Purpose:  Implementation (header) of ICA as described in Bell & Sejnowski (1995)
 *
 *         License:  GPL
 */

#ifndef _ICA_HH
#define _ICA_HH

#include <valarray>
#include <vector>
#include <list>
#include <string>
#include <cmath>
#include <functional>

#include "itpp/signal/fastica.h"

using namespace std;

namespace ica {

template <class T>
class CICA {
    public:
	int extended;
// % 'extended'  = [N] perform tanh() "extended-ICA" with sign estimation
// %               N training blocks. If N > 0, automatically estimate the
// %               number of sub-Gaussian sources. If N < 0, fix number of
// %               sub-Gaussian comps to -N [faster than N>0] (default|0 -> off)
	int pca;
// % 'pca'       = [N] decompose a principal component     (default -> 0=off)
// %               subspace of the data. Value is the number of PCs to retain.
	bool sphering;
// % 'sphering'  = ['on'/'off'] flag sphering of data      (default -> 'on')
	valarray<T> weights;
// % 'weights'   = [W] initial weight matrix               (default -> eye())
// %                            (Note: if 'sphering' 'off', default -> spher())
	double lrate;
// % 'lrate'     = [rate] initial ICA learning rate (<< 1) (default -> heuristic)
	unsigned block;
// % 'block'     = [N] ICA block size (<< datalength)      (default -> heuristic)
	double anneal;
// % 'anneal'    = annealing constant (0,1] (defaults -> 0.90, or 0.98, extended)
// %                         controls speed of convergence
	unsigned annealdeg;
// % 'annealdeg' = [N] degrees weight change for annealing (default -> 70)
	double stop;
// % 'stop'      = [f] stop training when weight-change < this (default -> 1e-6
// %               if less than 33 channel and 1E-7 otherwise)
	unsigned maxsteps;
// % 'maxsteps'  = [N] max number of ICA training steps    (default -> 512)
	bool do_bias;
// % 'bias'      = ['on'/'off'] perform bias adjustment    (default -> 'on')
	double momentum;
// % 'momentum'  = [0<f<1] training momentum               (default -> 0)
	struct SSpecgram {
		unsigned srate;
		double loHz, hiHz;
		unsigned frames, winframes;
	};
	SSpecgram specgram;
// % 'specgram'  = [srate loHz hiHz frames winframes] decompose a complex time/frequency
// %               transform of the data - though not optimally. (Note: winframes must
// %               divide frames) (defaults [srate 0 srate/2 size(data,2) size(data,2)])
	bool posact;
// % 'posact'    = make all component activations net-positive(default 'off'}
// %               Requires time and memory; posact() may be applied separately.
	list<string> log;
// % 'verbose'   = give ascii messages ('on'/'off')        (default -> 'on')
// % 'logfile'   = [filename] save all message in a log file in addition to showing them
// %               on screen (default -> none)
// % 'interput'  = ['on'|'off'] draw interupt figure. Default is off.

      // ctor
	CICA( unsigned srate_,
	      const vector<valarray<T>>& data_)
	      : data (data_),
		srate (srate_),
		extended (0),
		pca (0),
		sphering (true),
		//weights
		lrate (INFINITY), // means 'heuristic'
		block (0),   // ditto
		anneal (0.90),
		annealdeg (70),
		stop (data.size() < 33 ? 1e-6 : 1e-7),
		maxsteps (512),
		do_bias (true),
		momentum (0.),
		specgram (srate, 0., srate/2., data.size(), data.size()),
		posact (false)
		{
			// set up weights
		}
	const vector<valarray<T>>& data;
	unsigned srate;

	CICA() = delete;
	CICA( const CICA&) = delete;

      // methods
	int train();
	int run();

      // outputs
	vector<valarray<T>> compvars;
	valarray<T> bias;
	valarray<T> signs;
	valarray<T> lrates;
	vector<valarray<T>> activations;
};




template <class T>
class CFastICA {
    public:
      // ctor
	CFastICA( const vector<valarray<T> >& source)
		{
			itpp::Mat<T>
				_source_mat (source.size(), source.front().size());
			for ( size_t r = 0; r < source.size(); ++r ) {
				auto& row = source[r];
				_source_mat.set_row( r, itpp::Vec<T> (&row[0], row.size()));
			}
			_obj = new itpp::Fast_ICA (_source_mat);
		}
	CFastICA( const vector<function<valarray<T>()> >& source, size_t cols)
	// avoid creating a third temporary, specially for use with agh::CEDFFile::get_signal
		{
			itpp::Mat<T>
				_source_mat (source.size(), cols);
			for ( size_t r = 0; r < source.size(); ++r ) {
				_source_mat.set_row( r, itpp::Vec<T> (&source[r]()[0], cols));
			}
			_obj = new itpp::Fast_ICA (_source_mat);
		}
       ~CFastICA()
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

#endif // _ICA_HH

// eof