/*
 *       File name:  tools/edfcat.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-06-28
 *
 *         Purpose:  EDF signal converter
 *
 *         License:  GPL
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <utime.h>
#include <stdlib.h>

#include <stdexcept>
#include <fstream>
#include "libsigfile/edf.hh"
#include "libsigfile/source.hh"
#include "common/alg.hh"
#include "common/fs.hh"
#include "common/string.hh"

#if HAVE_CONFIG_H && !defined(VERSION)
#  include "config.h"
#endif


using namespace std;


struct SOperation {

	enum TKind {
		Noop,
		Split, Cat, // length-wise
		Convert,
		Prune, Merge,
		Generate
	};
	TKind	operation;

	struct SObject : public string {
		list<size_t>
			channels;  // if any
		list<double>
			timepoints_g;
		list<int>
			timepoints_d;
		size_t	samplerate,
			record_size;
		void figure_timepoints( const list<string>&) throw (invalid_argument);
		void figure_channels( const list<string>&) throw (invalid_argument);

		SObject (const char* rv)
		      : string (rv)
			{}
	};
	list<SObject>
		operands;
	int parse_op( int argc, const char* argv[]) throw (invalid_argument);
	int exec();

	SOperation( int argc, const char* argv[]) throw (invalid_argument)
		{
			parse_op( argc, argv);
		}
};



int
SOperation::
parse_op( int argc, const char* argv[]) throw (invalid_argument)
{
	if ( argc < 2 ) {
		operation = TKind::Noop;
		return 0;
	}

	const char* p = argv[1];

	if        ( strcmp( p, "split") == 0 ) {
		operation = TKind::Split;
		if ( argc != 4 )
			throw invalid_argument ("Usage: split FILE POS1[,POS2,...]");
		operands.emplace_back( argv[2]);
		operands.back().figure_timepoints( agh::str::tokens( argv[3], ","));

	} else if ( strcmp( p, "cat")   == 0 ) {
		operation = TKind::Split;
		if ( argc < 5 )
			throw invalid_argument ("Usage: cat FILE1 FILE2 [FILE3 ...] RESULT");
		for ( int k = 2; k < argc; ++k )
			operands.emplace_back( argv[k]);

	} else if ( strcmp( p, "conv")  == 0 ) {
		operation = TKind::Convert;
		if ( argc != 5 )
			throw invalid_argument ("Usage: conv FILE SAMPLERATE RECORD_SIZE");
		operands.emplace_back( argv[2]);
		char *tail;
		operands.back().samplerate = strtoul( argv[3], &tail, 10);
		if ( *tail )
			throw invalid_argument ("Unparsable samplerate");
		operands.back().record_size = strtoul( argv[4], &tail, 10);
		if ( *tail )
			throw invalid_argument ("Unparsable record_size");

	} else if ( strcmp( p, "prune")  == 0 ) {
		operation = TKind::Prune;
		if ( argc != 4 )
			throw invalid_argument ("Usage: prune FILE N1[N2,...]");
		operands.emplace_back( argv[2]);
		operands.back().figure_channels( agh::str::tokens( argv[3], ","));

	} else if ( strcmp( p, "merge") == 0 ) {
		operation = TKind::Merge;
		if ( argc < 4 )
			throw invalid_argument ("Usage: merge FILE FILE1 [FILE2 ...]");
		for ( int k = 2; k < argc; ++k )
			operands.emplace_back( argv[k]);

	} else
		throw invalid_argument ("Unrecognised operation");

	return 0;
}



void
SOperation::SObject::
figure_timepoints( const list<string>& argv) throw (invalid_argument)
{
	for ( auto& A : argv ) {
		char *tail;
		long Int = strtol( A.c_str(), &tail, 10);
		if ( *tail ) {
			double Double = strtod( A.c_str(), &tail);
			if ( *tail )
				throw invalid_argument ("Unparsable timepoint value");
			else if ( Double < 0. || Double > 1. )
				throw invalid_argument ("A floating-point timepoint value must be in [0..1] range");
			else {
				if ( not timepoints_d.empty() )
					throw invalid_argument ("Expecting all timepoint values to be either of type int or double");
				timepoints_g.push_back( Double);
			}
		} else {
			if ( not timepoints_g.empty() )
				throw invalid_argument ("Expecting all timepoint values to be either of type int or double");
			timepoints_d.push_back( Int);
		}
	}
	if ( timepoints_d.empty() && timepoints_g.empty() )
		throw invalid_argument ("Expecting one or more comma-separated timepoints (integers for absolute time in seconds, or doubles in range [0..1] for relative timepoints)");

	timepoints_g.sort(); timepoints_g.unique();
	timepoints_d.sort(); timepoints_d.unique();
}

void
SOperation::SObject::
figure_channels( const list<string>& argv) throw (invalid_argument)
{
	for ( auto& A : argv ) {
		char *tail;
		long Int = strtol( A.c_str(), &tail, 10) - 1;
		if ( *tail )
			throw invalid_argument ("Expecting a list of non-neg integers");
		else
			if ( Int < 1 )
				throw invalid_argument ("Expecting a list of non-neg integers");
		channels.push_back( Int);
	}
	if ( channels.empty() )
		throw invalid_argument ("Expecting one or more channel comma-separated indices");
}







list<pair<string, size_t>>
make_channel_headers_for_CEDFFile( size_t n, const char *fmt, size_t samplerate)
{
	list<pair<string, size_t>> ret;
	for ( size_t i = 0; i < n; ++i ) {
		DEF_UNIQUE_CHARP (_);
		assert (asprintf( &_, fmt, i) > 0 );
		ret.emplace_back( _, samplerate);
	}
	return ret;
}


inline void
sscanf_n_fields( string& linebuf, size_t columns, vector<valarray<TFloat>>& recp, size_t i)
{
	char *p = &linebuf[0];
	for ( size_t f = 0; f < columns; ++f ) {
		recp[f][i] = strtod( p, &p);
		if ( (f == columns-1 && *p) || (f < columns-1 && !isspace(*p)) ) {
			fprintf( stderr, "Bad data (row %zu, field %zu) at: \"%s\"\n", i, f, p);
			throw runtime_error ("Bad data");
		}
		p += strspn( p, " ,\t");
	}
}

pair<TFloat, TFloat>
determine_ranges( const valarray<TFloat>& x)
{
	pair<TFloat, TFloat> ranges = {x.min(), x.max()};

	return ranges;
}


valarray<TFloat>
preprocess( const valarray<TFloat>& x_, size_t samplerate,
	    TFloat* avgmin_p, TFloat* avgmax_p)
{
	valarray<TFloat>
		x = x_ - x_.sum()/x_.size(); // zeromean

	vector<size_t>
		mini, maxi;
	sigproc::envelope(
		{x, samplerate}, samplerate/4, .25,
		(valarray<TFloat>*)nullptr,  // wow, just wow
		(valarray<TFloat>*)nullptr,
		&mini, &maxi);
	// compute avg min and max
	TFloat avgmin = 0.;
	for ( size_t i = 0; i < mini.size(); ++i )
		avgmin += x[mini[i]];
	avgmin /= mini.size();
	TFloat avgmax = 0.;
	for ( size_t i = 0; i < maxi.size(); ++i )
		avgmax += x[maxi[i]];
	avgmax /= maxi.size();
	printf( "avg min/max: %g/%g\n", avgmin, avgmax);
	*avgmin_p = avgmin;
	*avgmax_p = avgmax;

	// find outliers
	for ( size_t i = 0; i < mini.size(); ++i )
		if ( x[mini[i]] < avgmin * 10 )
			printf( "outlier (-) at %s:\t%g\n",
				agh::str::dhms_colon((double)mini[i] / samplerate, 2).c_str(),
				x[mini[i]]);
	for ( size_t i = 0; i < maxi.size(); ++i )
		if ( x[maxi[i]] > avgmax * 10 )
			printf( "outlier (+) at %s:\t%g\n",
				agh::str::dhms_colon((double)maxi[i] / samplerate, 2).c_str(),
				x[maxi[i]]);

	return x;
}

int
exec_convert( const SOperation::SObject& obj)
{
	vector<valarray<TFloat>> Hh;
	size_t total_samples;
	double duration;

      // read data
	{
		ifstream ifs (obj.c_str());
		if ( not ifs.good() ) {
			DEF_UNIQUE_CHARP (_);
			if ( asprintf( &_, "Convert: Couldn't open file %s", obj.c_str()) )
				;
			throw runtime_error (_);
		}

		string linebuf;
		// figure # of fields
		while ( (getline( ifs, linebuf, '\n'), linebuf[0] == '#') )
			;
		Hh.resize( agh::str::tokens( linebuf, " \t,").size());

		size_t i = 0, p = 0;
#define chunk 1000000
		while ( true ) {
			if ( i >= p*chunk ) {
				++p;
				for ( size_t f = 0; f < Hh.size(); ++f ) {
					auto tmp = Hh[f];
					Hh[f].resize(p * chunk);
					Hh[f][slice (0, tmp.size(), 1)] = tmp;
				}
			}

			sscanf_n_fields( linebuf, Hh.size(), Hh, i); // throws
			++i;

			while ( (getline( ifs, linebuf, '\n'),
				 linebuf.empty() || linebuf[0] == '#') )
				if ( ifs.eof() )
					goto out;
		}
	out:
		total_samples = i;

		duration = (double)total_samples/obj.samplerate;
		printf( "Read %'zu samples (%s) in %zu channel(s)\n",
			total_samples, agh::str::dhms(duration).c_str(), Hh.size());
	}

      // zeromean, report glitches, get ranges
	vector<pair<TFloat, TFloat>>
		ranges (Hh.size());
	for ( size_t i = 0; i < Hh.size(); ++i ) {
		Hh[i] = preprocess( Hh[i], obj.samplerate,
				    &ranges[i].first, &ranges[i].second);
		printf( "physical_min/max in channel %zu: %g:%g\n",
			i, ranges[i].first, ranges[i].second);
	}

      // unify ranges
	TFloat grand_min = INFINITY, grand_max = -INFINITY;
	for ( size_t i = 0; i < Hh.size(); ++i ) {
		if ( grand_min > ranges[i].first )
			grand_min = ranges[i].first;
		if ( grand_max < ranges[i].second )
			grand_max = ranges[i].second;
	}
	grand_min = floor(grand_min) * 1.5; // extend a little
	grand_max =  ceil(grand_max) * 1.5;
	printf( "Setting common physical_min/max to %g:%g\n",
		grand_min, grand_max);

	sigfile::CEDFFile F ((obj + ".edf").c_str(),
			     sigfile::CEDFFile::TSubtype::edf,
			     sigfile::CTypedSource::no_ancillary_files,
			     make_channel_headers_for_CEDFFile( Hh.size(), "channel%zu", obj.samplerate),
			     obj.record_size,
			     ceilf(duration / obj.record_size));
	for ( size_t f = 0; f < Hh.size(); ++f ) {
		F[f].set_physical_range( grand_min, grand_max);
		F.put_signal( f, valarray<TFloat> {Hh[f][slice (0, total_samples, 1)]});
	}

	printf( "Created edf:\n%s\n"
		"You may now want to fill out the header of the newly created EDF file.\n"
		"Use edfhed --set ... to do so, or run edfhed-gtk.\n", F.details().c_str());

	return 0;
}





int
exec_prune( const SOperation::SObject& obj)
{
	sigfile::CEDFFile F (obj.c_str(), sigfile::CTypedSource::no_ancillary_files);

	list<pair<string, size_t>> selected_channels;
	for ( auto& select_this : obj.channels ) {
		if ( select_this >= F.n_channels() ) {
			DEF_UNIQUE_CHARP (_);
			assert (asprintf( &_, "Prune: Requested channel #%zu (1-based) in file %s which only has %zu channel(s)",
					  select_this, F.filename(), F.n_channels()) > 0 );
			throw invalid_argument (_);
		}
		string label (F[select_this].header.label, 16);
//		strncpy( &label[0], F[select_this].header.label, 16);
		selected_channels.emplace_back(
			agh::str::trim( label), // F.channel_by_id( select_this), // that gives a cooked string like "1:<channel1>"
			F.samplerate( select_this));
	}
	printf( "Keeping %zu channel(s)\n", selected_channels.size());

	sigfile::CEDFFile G ((agh::fs::make_fname_base( obj, ".edf", false) + "-mod.edf").c_str(),
			     sigfile::CEDFFile::TSubtype::edf,
			     sigfile::CTypedSource::no_ancillary_files,
			     selected_channels,
			     F.data_record_size,
			     F.n_data_records);

	G.set_patient_id( F.patient_id());
	string tmp = F.recording_id();
	G.set_recording_id( tmp.c_str());
	tmp = F.comment();
	G.set_comment( tmp.c_str());
	G.set_start_time( F.start_time());
	printf( "Created edf:\n%s\n", G.details().c_str());

//	F.resize( data.size() / obj.samplerate / obj.record_size);
	size_t h = 0;
	for ( auto& hs : obj.channels ) {
		G[h].set_physical_range(
			F[hs].physical_min, F[hs].physical_max);
		G[h].set_digital_range(
			F[hs].digital_min, F[hs].digital_max);
		G[h].scale =
			F[hs].scale;
		G.put_signal(
			h,
			F.get_signal_original( hs));
		++h;
	}

	return 0;
}





int
SOperation::
exec()
{
	switch ( operation ) {
	// case TKind::Split:
	// 	printf( "Pretend exec split (%s)\n", agh::str::join( operands, ", ").c_str());
	//     break;
	// case TKind::Cat:
	// 	printf( "Pretend exec cat (%s)\n", agh::str::join( operands, ", ").c_str());
	//     break;
	case TKind::Convert:
		return exec_convert( operands.front());
	    break;
	case TKind::Prune:
		return exec_prune( operands.front());
	    break;
	// case TKind::Merge:
	// 	printf( "Pretend exec merge (%s)\n", agh::str::join( operands, ", ").c_str());
	//     break;
	// case TKind::Generate:
	// 	printf( "Generate Not supported\n");
	//     break;
	case TKind::Noop:
	default:
		printf( "edfcat %s <%s>\n"
			"Usage: conv|prune OPERANDS\n", VERSION, PACKAGE_BUGREPORT);
	    break;
	}

	return 0;
}



int
main( int argc, const char **argv)
{
	try {
		SOperation Op (argc, argv);

		Op.exec();

	} catch (exception& ex) {
		printf( "Error: %s\n", ex.what());
		return 1;
	}

	return 0;
}



// Local Variables:
// Mode: c++
// indent-tabs-mode: 8
// End:
