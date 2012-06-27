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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <stdlib.h>

#include <iostream>
#include "../libsigfile/edf.hh"
#include "../libsigfile/source.hh"
#include "../common/fs.hh"

#include "config.h"

const char
	*argp_program_version = "edfhed " VERSION,
	*argp_program_bug_address = "<" PACKAGE_BUGREPORT ">";

static char doc[] =
       "edfhed -- EDF file header viewer and non-interctive editor";

static struct argp_option options[] = {
       {"no-channels",		'b', 0,	0, "Only dump general header fields (no channel details)"},
       {"set",			's', "[CH:]FIELD:VALUE", 0, "Set FIELD to VALUE (possibly in channel CH)" },
       {"id-from-tree",		'R', 0,	0, "Set 'recording_id' field to Subject/Session/Episode given current file location"},
       {"from-mtime",		'T', 0,	0, "Set 'recording_date' and 'recording_time' fields to file modification date/time"},
       {"touch-mtime",		't', 0,	0, "Parse 'recording_date' and 'recording_time' and set file modification timestamp"},
       { 0 }
     };

static char args_doc[] = "FILE.edf ...";

static error_t parse_opt (int, char*, struct argp_state*);

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
	bool	header_only:1,
		from_tree:1,
		from_timestamp:1,
		to_timestamp:1;
	SArguments()
	      : header_only (false),
		from_tree (false),
		from_timestamp (false),
		to_timestamp (false)
		{}
};



static error_t
parse_opt( int key, char *arg, struct argp_state *state)
{
	auto& Q = *(SArguments*)state->input;

	switch ( key ) {
	case 'b':
		Q.header_only = true;
		break;
	case 'R':
		Q.from_tree = true;
		break;
	case 'T':
		Q.from_timestamp = true;
		break;
	case 't':
		Q.to_timestamp = true;
		break;
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
		return (error_t)ARGP_ERR_UNKNOWN;
	}
	return (error_t)0;
}



static int
set_recording_datetime_from_mtime( sigfile::CEDFFile& F)
{
	struct stat ss;
	if ( stat( F.filename(), &ss) != 0 ) {
		perror( (string ("Error statting ") + F.filename()).c_str());
		return errno;
	} else {
		struct tm *t = localtime( &ss.st_mtime);
		if ( t ) {
			char date_buf[9], time_buf[9];
			int fixed_year = t->tm_year;
			if ( fixed_year > 99 )
				fixed_year -= 100;
			snprintf( date_buf, 9, "%02d.%02d.%02d",
				  t->tm_mday, t->tm_mon+1, fixed_year);
			snprintf( time_buf, 9, "%02d.%02d.%02d",
				  t->tm_hour, t->tm_min, t->tm_sec);
			memcpy( F.header.recording_date,
				date_buf, 8);
			memcpy( F.header.recording_time,
				time_buf, 8);
			printf( "%s %s\n", date_buf, time_buf);
			return 0;
		} else {
			fprintf( stderr, "Could not parse statted mtime of %s\n", F.filename());
			return -1;
		}
	}
}


static int
set_mtime_from_recording_datetime( sigfile::CEDFFile& F)
{
	if ( F.status() & sigfile::CEDFFile::date_unparsable ||
	     F.status() & sigfile::CEDFFile::time_unparsable ) {
		fprintf( stderr, "Error: Bad recording_date or _time fields; not setting file mtime");
		return -1;
	}

	struct utimbuf tmb;
	tmb.modtime = tmb.actime = F.start_time();
	if ( utime( F.filename(), &tmb) ) {
		perror("Error setting mtime:");
		return errno;
	}
	return 0;
}


static int
set_session_and_episode_from_tree( sigfile::CEDFFile& F)
{
	// filename can be anything, including a symlink
	bool	is_path_absolute = (F.filename()[0] == '/');
	list<string> pe = agh::fs::path_elements( string (is_path_absolute ? "" : "./") + F.filename());
	string	episode = agh::fs::make_fname_base( pe.back(), ".edf", false);

	string	in_dir = string (is_path_absolute ? "/" : "") + agh::str::join( list<string> (pe.begin(), prev(pe.end())), "/") + "/.";
	// a symlink from ./filename.edf would resolve somewhere else,
	// losing the right path elements, so only canonicalize_file_name
	// on the dir it is in
	char *c = canonicalize_file_name( in_dir.c_str());
	pe = agh::fs::path_elements( c);
	free(c);
	if ( pe.size() < 2 ) {
		fprintf( stderr, "Too few path elements (expecting Subject/Session/Episode.edf)\n");
		return -1;
	}
	string	session = pe.back(),
		subject = (pe.pop_back(), pe.back());
	// assign
	if ( subject.size() > 80 ) {
		fprintf( stderr, "Refuse to set patient_id as path element \"%s\" is longer than 80 characters\n",
			 subject.c_str());
		return -2;
	}
	if ( session.size() + 1 + episode.size() > 80 ) {
		fprintf( stderr, "Refuse to set recording_id as path elements \"%s/%s\" combined are longer than 80 characters\n",
			 session.c_str(), episode.c_str());
		return -2;
	}
	memcpy( F.header.patient_id,
		agh::str::pad( subject.c_str(), 80).c_str(), 80);
	memcpy( F.header.recording_id,
		agh::str::pad( (session + '/' + episode).c_str(), 80).c_str(), 80);
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
			if ( Opts.settables.empty() &&
			     not Opts.from_timestamp && not Opts.from_tree && not Opts.to_timestamp ) {
				cout << F.details( not Opts.header_only) << endl;
			} else {
				if ( Opts.to_timestamp ) {
					set_mtime_from_recording_datetime( F);
				}
				if ( Opts.from_timestamp ) {
					set_recording_datetime_from_mtime( F);
				}
				if ( Opts.from_tree ) {
					set_session_and_episode_from_tree( F);
				}
				for ( auto& S : Opts.settables ) {
					switch ( S.which ) {
					case SSettable::TField::version_number:
						memcpy( F.header.version_number,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::patient_id:
						memcpy( F.header.patient_id,
							agh::str::pad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::recording_id:
						memcpy( F.header.recording_id,
							agh::str::pad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::recording_date:
						memcpy( F.header.recording_date,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::recording_time:
						memcpy( F.header.recording_time,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::reserved:
						memcpy( F.header.reserved,
							agh::str::pad( S.value.c_str(), 44).c_str(), 44);
						break;

					case SSettable::TField::ch_label:
						memcpy( F[S.channel].header.label,
							agh::str::pad( S.value.c_str(), 16).c_str(), 16);
						break;
					case SSettable::TField::ch_transducer_type:
						memcpy( F[S.channel].header.transducer_type,
							agh::str::pad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::ch_physical_dim:
						memcpy( F[S.channel].header.physical_dim,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_physical_min:
						memcpy( F[S.channel].header.physical_min,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_physical_max:
						memcpy( F[S.channel].header.physical_max,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_digital_min:
						memcpy( F[S.channel].header.digital_min,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_digital_max:
						memcpy( F[S.channel].header.digital_max,
							agh::str::pad( S.value.c_str(),  8).c_str(),  8);
						break;
					case SSettable::TField::ch_filtering_info:
						memcpy( F[S.channel].header.filtering_info,
							agh::str::pad( S.value.c_str(), 80).c_str(), 80);
						break;
					case SSettable::TField::ch_reserved:
						memcpy( F[S.channel].header.reserved,
							agh::str::pad( S.value.c_str(), 32).c_str(), 32);
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
