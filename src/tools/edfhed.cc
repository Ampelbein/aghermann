// ;-*-C++-*-
/*
 *       File name:  tools/edfed.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-07-20
 *
 *         Purpose:  EDF header viewer/editor
 *
 *         License:  GPL
 */


#include <argp.h>
#include <iostream>
#include "../libsigfile/edf.hh"
#include "../libsigfile/source.hh"

#include "config.h"

const char
	*argp_program_version = "edfhed " VERSION,
	*argp_program_bug_address = "<" PACKAGE_BUGREPORT ">";

static char doc[] =
       "edfhed -- EDF file header viewer and non-interctive editor";

static struct argp_option options[] = {
       {"no-channels",	'b', 0,	0, "Only dump general header fields (no channel details)"},
       {"from-tree",	'R', 0,	0, "Set 'RecordingID' field to Subject/Session/Episode given current file location"},
       {"set",		's', "[CH:]FIELD:VALUE",
				0, "Set FIELD to VALUE" },
       { 0 }
     };

static char args_doc[] = "FILE.edf ...";

extern "C" error_t
parse_opt (int, char*, struct argp_state*);

static struct argp argp = {
	options,
	parse_opt,
	args_doc,
	doc
};

struct SSettable {
	enum TField {
		// note that some fields are not not included
		version_number,		//  [ 8],
		patient_id,		//  [80],  // maps to subject name
		recording_id,		//  [80],  // maps to episode_name (session_name)
		recording_date,		//  [ 8],
		recording_time,		//  [ 8],
		reserved,		//  [44],

		ch_label,		//  [16],  // maps to channel
		ch_transducer_type,	//  [80],
		ch_physical_dim,	//  [ 8],
		ch_physical_min,	//  [ 8],
		ch_physical_max,	//  [ 8],
		ch_digital_min,		//  [ 8],
		ch_digital_max,		//  [ 8],
		ch_filtering_info,	//  [80],
		ch_reserved,		//  [32];
	};
	TField which;
	int channel;

	SSettable( const char* pv)
		{
			if ( from_string( pv) != 0 )
				throw invalid_argument( pv);
		}
	SSettable() = delete;

	string value;
	int from_string( const char *pv)
		{
			int h = -1;
			channel = h;
			char p[21], v[81];  // make it 20
			if ( 3 == sscanf( pv, "%u:%20[a-z_]:%80s", &h, p, v) && h != -1 ) {
				channel = h - 1;  // base 0
				if ( strcmp( p, "label") == 0 ) {
					if ( not sigfile::SChannel::channel_follows_system1020( v) )
						printf( "Note: Channel label \"%s\" does not follow System 10-20\n", v);
					which = ch_label;
				} else if ( strcmp( p, "transducer_type") == 0 ) {
					which = ch_transducer_type;
				} else if ( strcmp( p, "physical_dim") == 0 ) {
					which = ch_physical_dim;
				} else if ( strcmp( p, "physical_min") == 0 ) {
					stod(v);
					which = ch_physical_min;
				} else if ( strcmp( p, "physical_max") == 0 ) {
					stod(v);
					which = ch_physical_max;
				} else if ( strcmp( p, "digital_min") == 0 ) {
					stoi(v);
					which = ch_digital_min;
				} else if ( strcmp( p, "digital_max") == 0 ) {
					stoi(v);
					which = ch_digital_max;
				} else if ( strcmp( p, "filtering_info") == 0 ) {
					which = ch_filtering_info;
				} else if ( strcmp( p, "reserved") == 0 ) {
					which = ch_reserved;
				} else
					return -3;
			} else if ( 2 == sscanf( pv, "%20[a-z_]:%80s", p, v) ) {
				if      ( strcmp( p, "version_number") == 0 )
					which = version_number;
				else if ( strcmp( p, "patient_id") == 0 )
					which = patient_id;
				else if ( strcmp( p, "recording_id") == 0 )
					which = recording_id;
				else if ( strcmp( p, "recording_date") == 0 )
					which = recording_date;
				else if ( strcmp( p, "recording_time") == 0 )
					which = recording_time;
				else if ( strcmp( p, "reserved") == 0 )
					which = reserved;
				else
					return -2;
			} else {
				return -1;
			}

			value.assign( v);
			return 0;
		}
};

struct SArguments {
	std::vector<const char*>
		files;
	std::vector<SSettable>
		settables;
	bool	header_only:1;
	SArguments()
	      : header_only (false)
		{}
};



error_t
parse_opt( int key, char *arg, struct argp_state *state)
{
	auto& Q = *(SArguments*)state->input;

	switch ( key ) {
	case 'b':
		Q.header_only = true;
		break;
	// case 'R':
	// 	Q.from_tree = true;
	// 	break;
	case 's':
		try {
			// screw google C++ guidelines
			Q.settables.emplace_back( arg);
		} catch (...) {
			cerr << "Bad field or value: " << arg << endl
			     << "Valid fields are:\n"
				"\tversion_number, patient_id, recording_id,\n"
				"\trecording_date, recording_time, reserved;\n"
				"(with prepended <ch>:)\n"
				"\tlabel, transducer_type, physical_dim,\n"
				"\tphysical_min, physical_max, digital_min, digital_max,\n"
				"\tfiltering_info, reserved\n";
			exit(1);
		}
		break;

	case ARGP_KEY_ARG:
		Q.files.push_back( arg);
		break;

	case ARGP_KEY_END:
		if ( state->arg_num < 1 )
			argp_usage( state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
		return 0;
	}
	return 0;
}

int
main( int argc, char **argv)
{
	SArguments Opts;
	argp_parse( &argp, argc, argv, 0, NULL, (void*)&Opts);

	for ( auto &fname : Opts.files )
		try {
			auto F = sigfile::CEDFFile (fname,
						    sigfile::CSource::no_ancillary_files |
						    sigfile::CEDFFile::no_field_consistency_check);
			F.no_save_extra_files = true;
			if ( Opts.settables.empty() ) {
				cout << F.details( not Opts.header_only) << endl;
			} else {
				for ( auto& S : Opts.settables ) {
					switch ( S.which ) {
					case SSettable::TField::version_number:
						memcpy( F.header.version_number,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::patient_id:
						memcpy( F.header.patient_id,
							strpad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::recording_id:
						memcpy( F.header.recording_id,
							strpad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::recording_date:
						memcpy( F.header.recording_date,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::recording_time:
						memcpy( F.header.recording_time,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::reserved:
						memcpy( F.header.reserved,
							strpad( S.value.c_str(), 44).c_str(), 44);
						break;

					case SSettable::TField::ch_label:
						memcpy( F[S.channel].header.label,
							strpad( S.value.c_str(), 16).c_str(), 16);
						break;
					case SSettable::TField::ch_transducer_type:
						memcpy( F[S.channel].header.transducer_type,
							strpad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::ch_physical_dim:
						memcpy( F[S.channel].header.physical_dim,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_physical_min:
						memcpy( F[S.channel].header.physical_min,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_physical_max:
						memcpy( F[S.channel].header.physical_max,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_digital_min:
						memcpy( F[S.channel].header.digital_min,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_digital_max:
						memcpy( F[S.channel].header.digital_max,
							strpad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_filtering_info:
						memcpy( F[S.channel].header.filtering_info,
							strpad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::ch_reserved:
						memcpy( F[S.channel].header.reserved,
							strpad( S.value.c_str(), 32).c_str(), 32);
						break;
					}
				}
			}

		} catch (invalid_argument ex) {
			cerr << ex.what() << endl;
			return -2;
		} catch (out_of_range ex) {
			cerr << ex.what() << endl;
			return -2;
		}

	return 0;
}


// eof
