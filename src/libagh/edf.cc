// ;-*-C++-*-
/*
 *       File name:  libagh/edf.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2008-07-01
 *
 *         Purpose:  EDF class methods
 *
 *         License:  GPL
 */


#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cerrno>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iterator>

#include "misc.hh"
#include "edf.hh"

using namespace std;
using namespace agh;

#define AGH_KNOWN_CHANNELS_TOTAL 78
#define AGH_LAST_EEG 74
#define AGH_LAST_EOG 76
#define AGH_LAST_EMG 77
array<const char*, 78> agh::SChannel::system1020_channels = {{  // counted 'em all!
	"Nz",
	"Fp1", "Fpz", "Fp2",
	"AF7", "AF3", "AFz", "AF4", "AF8",
	"F9",  "F7", "F5", "F3", "F1", "Fz", "F2", "F4", "F6", "F8", "F10",
	"FT9", "FT7", "FC5", "FC3", "FC1", "FCz", "FC2", "FC4", "FC6", "FCT8", "FT10",
	"A1", "T9", "T7", "C5", "C3", "C1", "Cz", "C2", "C4", "C6", "T8", "T10", "A2",
	"TP9", "TP7", "CP5", "CP3", "CP1", "CPz", "CP2", "CP4", "CP6", "TP8", "TP10",
	"P9", "P7", "P5", "P3", "P1", "Pz", "P2", "P4", "P6", "P8", "P10",
	"PO7", "PO3", "POz", "PO4", "PO8",
	"O1", "Oz", "O2",
	"Iz",
	// plus channels of other signal types
	"Left", "Right",
	"Chin",
}};


#define AGH_KNOWN_SIGNAL_TYPES 16

array<const char*, 16> agh::SChannel::kemp_signal_types = {{
	"EEG", "EOG", "EMG", "ECG", "ERG",
	"NC",  "MEG", "MCG", "EP",
	"Temp", "Resp", "SaO2",
	"Light", "Sound", "Event", "Freq",
}};


const char*
agh::SChannel::signal_type_following_kemp( const string& signal)
{
	size_t h = 0;
	for ( ; h <= AGH_LAST_EEG; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[0];  // we
	for ( ; h <= AGH_LAST_EOG; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[1];  // love
	for ( ; h <= AGH_LAST_EMG; ++h )
		if ( signal == system1020_channels[h] )
			return kemp_signal_types[2];  // plain C
	return NULL;
}

bool
agh::SChannel::channel_follows_system1020( const string& channel)
{
	for ( size_t h = 0; h < system1020_channels.size(); ++h )
		if ( channel == system1020_channels[h] )
			return true;
	return false;
}



int
agh::SChannel::compare( const char *a, const char *b)
{
	size_t ai = 0, bi = 0;
	while ( ai < system1020_channels.size() && strcmp( a, system1020_channels[ai]) )
		++ai;
	while ( bi < system1020_channels.size() && strcmp( b, system1020_channels[bi]) )
		++bi;
	return (ai < bi) ? -1 : ((ai > bi) ? 1 : strcmp( a, b));
}






size_t
agh::CEDFFile::SSignal::dirty_signature() const
{
	string sig ("a");
	for ( auto &A : artifacts )
		sig += (to_string((long long int)A.first) + ':' + to_string((long long int)A.second));
	return HASHKEY(sig);
}


void
agh::CEDFFile::SSignal::mark_artifact( size_t aa, size_t az)
{
	artifacts.emplace_back( aa, az);
	artifacts.sort();
startover:
	for ( auto A = artifacts.begin(); A != artifacts.end(); ++A )
		if ( next(A) != artifacts.end()
		     && A->second >= next(A)->first ) {
			A->second = max( A->second, next(A)->second);
			artifacts.erase( next(A));
			goto startover;
		 }
 }


void
agh::CEDFFile::SSignal::clear_artifact( size_t aa, size_t az)
{
startover:
	for ( auto A = artifacts.begin(); A != artifacts.end(); ++A ) {
		if ( aa < A->first && A->second < az ) {
			artifacts.erase( A);
			goto startover;
		}
		if ( A->first < aa && az < A->second ) {
			artifacts.emplace( next(A), az, A->second);
			A->second = aa;
			break;
		}
		if ( A->first < aa && aa < A->second ) {
			A->second = aa;
		}
		if ( A->first < az && az < A->second ) {
			A->first = az;
		}
	}
}


size_t
agh::CEDFFile::SSignal::mark_annotation( size_t aa, size_t az, const char *label)
{
	annotations.emplace_back( aa, az, label, SAnnotation::TOrigin::file);
	return annotations.size()-1;
}








#define EOA '$'


agh::CEDFFile::CEDFFile( const char *fname,
			 size_t scoring_pagesize,
			 SFFTParamSet::TWinType _af_dampen_window_type)
      : CHypnogram (scoring_pagesize, agh::make_fname_hypnogram(fname, scoring_pagesize)),
	af_dampen_window_type (_af_dampen_window_type),
	no_save_extra_files (false),
	_status (TStatus::ok)
{
	_filename = fname;
	{
		struct stat stat0;
		int stst = stat( filename(), &stat0);
		if ( stst == -1 ) {
			DEF_UNIQUE_CHARP(_);
			if ( asprintf( &_, "No such file: \"%s\"", fname) ) ;
			throw invalid_argument (_);
		}
		_fsize = stat0.st_size;
	}
	int filedes = open( fname, O_RDWR);
	if ( filedes == -1 ) {
		_status |= TStatus::sysfail;
		throw invalid_argument (string ("Failed to open: ") + fname);
	}
	if ( (_mmapping = mmap( NULL,
				_fsize,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				filedes,
				0)) == (void*)-1 ) {
		close( filedes);
		DEF_UNIQUE_CHARP(_);
		if ( asprintf( &_, "Failed to mmap %zu bytes", _fsize) ) ;
		throw length_error (_);
	}

	if ( _parse_header() ) {  // creates signals list
		string st = explain_edf_status(_status);
		fprintf( stderr, "CEDFFile(\"%s\"): errors found while parsing:\n%s\n",
			 fname, st.c_str());
		throw invalid_argument (
			string ("Failed to parse edf header of \"") + fname
			+ "\": " + st);
	}

	_data_offset = 256 + (signals.size() * 256);

      // CHypnogram::
	size_t scorable_pages = n_data_records * data_record_size / scoring_pagesize;  // with implicit floor
	if ( CHypnogram::length() != scorable_pages ) {
		if ( CHypnogram::length() > 0 )
			fprintf( stderr, "CEDFFile(\"%s\"): number of scorable pages @pagesize=%zu (%zu) differs from the number read from hypnogram file (%zu); discarding hypnogram\n",
				 fname, scoring_pagesize, scorable_pages, CHypnogram::length());
		CHypnogram::_pages.resize( scorable_pages);
	}

	//fprintf( stderr, "CEDFFile(\"%s\"): added, with details:\n", fname);
	// fprintf( stderr, "%s\n", o.c_str());

      // artifacts, per signal
	for ( size_t h = 0; h < signals.size(); ++h ) {
		ifstream thomas (make_fname_artifacts( signals[h].channel));
		if ( not thomas.good() )
			continue;
		int wt = -1;
		float fac = 0.;
		thomas >> wt >> fac;
		if ( thomas.eof()
		     || wt < 0 || wt > (int)SFFTParamSet::TWinType::welch
		     || fac == 0. ) {
			continue;
		}
		signals[h].af_dampen_window_type = (SFFTParamSet::TWinType)wt;
		signals[h].af_factor = fac;

		while ( !thomas.eof() ) {
			size_t aa = (size_t)-1, az = (size_t)-1;
			thomas >> aa >> az;
			if ( aa == (size_t)-1 || az == (size_t)-1 )
				break;
			signals[h].artifacts.emplace_back( aa, az);
		}
	}

      // annotations, per signal
	for ( size_t h = 0; h < signals.size(); ++h ) {
		ifstream fd (make_fname_annotations( signals[h].channel));
		if ( not fd.good() )
			continue;
		size_t aa, az;
		string an;
		while ( true ) {
			fd >> aa >> az;
			getline( fd, an, EOA);
			if ( fd.good() && !fd.eof() ) {
				signals[h].annotations.emplace_back(
					aa, az,
					strtrim(an),
					SSignal::SAnnotation::TOrigin::file);
			} else
				break;
		}
	}

      // filters
	{
		ifstream thomas (make_fname_filters(fname));
		if ( !thomas.fail() )
			for ( size_t h = 0; h < signals.size(); ++h ) {
				int ol = -1, oh = -1;
				float fl = 0., fh = 0.;
				thomas >> fl >> ol
				       >> fh >> oh;
				if ( thomas.bad() || thomas.eof() )
					break;
				if ( ol > 0 && oh > 0 && ol < 5 && oh < 5
				     && fl > 0. && fh > 0. ) {
					signals[h].low_pass_cutoff = fl, signals[h].low_pass_order = ol;
					signals[h].high_pass_cutoff = fh, signals[h].high_pass_order = oh;
				}
			}
	}
}




agh::CEDFFile::CEDFFile( CEDFFile&& rv)
      : CHypnogram ((CEDFFile&&)rv)
{
	swap( _filename, rv._filename);

	header = rv.header;
	n_data_records   = rv.n_data_records;
	data_record_size = rv.data_record_size;

	start_time = rv.start_time;
	end_time   = rv.end_time;

	swap( patient, rv.patient);
	swap( episode, rv.episode);
	swap( session, rv.session);

	swap( signals, rv.signals);

	_data_offset = rv._data_offset;
	_fsize       = rv._fsize;
	_fld_pos     = rv._fld_pos;
	_total_samples_per_record = rv._total_samples_per_record;
	_mmapping    = rv._mmapping;

	_status = rv._status;

	rv._mmapping = (void*)-1;  // will prevent munmap in ~CEDFFile()
}


agh::CEDFFile::~CEDFFile()
{
	if ( _mmapping != (void*)-1 ) {
		munmap( _mmapping, _fsize);

		if ( not no_save_extra_files )
			write_ancillry_files();
	}
}

void
agh::CEDFFile::write_ancillry_files() const
{
	CHypnogram::save( agh::make_fname_hypnogram( filename(), pagesize()));

	for ( size_t h = 0; h < signals.size(); ++h ) {
		if ( signals[h].artifacts.size() ) {
			ofstream thomas (make_fname_artifacts( signals[h].channel), ios_base::trunc);
			if ( thomas.good() ) {
				thomas << (unsigned short)signals[h].af_dampen_window_type << ' ' << signals[h].af_factor << endl;
				for ( auto &A : signals[h].artifacts )
					thomas << A.first << ' ' << A.second << endl;
			}
		} else
			if ( unlink( make_fname_artifacts( signals[h].channel).c_str()) )
				;
	}

	{
		ofstream thomas (make_fname_filters( filename()), ios_base::trunc);
		if ( thomas.good() )
			for ( size_t h = 0; h < signals.size(); ++h ) {
				const SSignal& sig = signals[h];
				thomas << sig.low_pass_cutoff << ' ' << sig.low_pass_order << ' '
				       << sig.high_pass_cutoff << ' ' << sig.high_pass_order << endl;
			}
	}

	for_each( signals.begin(), signals.end(),
		  [this]( const SSignal& H)
		  {
			  if ( H.annotations.size() ) {
				  ofstream thomas (make_fname_annotations( H.channel), ios_base::trunc);
				  for ( auto &A : H.annotations )
					  thomas << A.span.first << ' ' << A.span.second << ' ' << A.label << EOA << endl;
			  } else
				  if ( unlink( make_fname_annotations( H.channel).c_str()) )
					  ;
		  });
}















char*
agh::CEDFFile::_get_next_field( char *&field, size_t fld_size) throw (TStatus)
{
	if ( _fld_pos + fld_size > _fsize ) {
		_status |= bad_header;
		throw bad_header;
	}

	field = (char*)_mmapping + _fld_pos;
	_fld_pos += fld_size;

	return field;
}

size_t
	agh::CEDFFile::max_signals = 128;

int
agh::CEDFFile::_parse_header()
{
	size_t	n_signals,
		i;
	try {
		_fld_pos = 0;
		_get_next_field( header.version_number,   8);
		_get_next_field( header.patient_id,      80);
		_get_next_field( header.recording_id,    80);
		_get_next_field( header.recording_date,   8);
		_get_next_field( header.recording_time,   8);
		_get_next_field( header.header_length,    8);
		_get_next_field( header.reserved,        44);
		_get_next_field( header.n_data_records,   8);
		_get_next_field( header.data_record_size, 8);
		_get_next_field( header.n_signals,        4);

		if ( strncmp( header.version_number, "0       ", 8) ) {
			_status |= bad_version;
			return -2;
		}

		size_t	header_length;

		header_length = n_data_records = data_record_size = n_signals = 0;
		sscanf( header.header_length,    "%8zu", &header_length);
		sscanf( header.n_data_records,   "%8zu", &n_data_records);
		sscanf( header.data_record_size, "%8zu", &data_record_size);
		sscanf( header.n_signals,        "%4zu", &n_signals);

		if ( !header_length || !n_data_records || !data_record_size || !n_signals ) {
			_status |= bad_numfld;
			return -2;
		}

		patient = strtrim( string (header.patient_id, 80));

	      // deal with episode and session
		{
		      // (a) parsed from RecordingID_raw
			char int_session[81], int_episode[81];
			string rec_id_isolated (strtrim( string (header.recording_id, 80)));
#define T "%80[-a-zA-Z0-9 _]"
			if ( sscanf( rec_id_isolated.c_str(), T", "T,    int_episode, int_session) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T": "T,    int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T" / "T,   int_session, int_episode) == 2 ||
			     sscanf( rec_id_isolated.c_str(), T" ("T")", int_session, int_episode) == 2 )
				;
			else
				_status |= (nosession | noepisode);
#undef T
		      // (b) identified from file name
			string fn_episode;
			size_t basename_start = _filename.rfind( '/');
			fn_episode =
				_filename.substr(
					basename_start + 1,
					_filename.size() - basename_start - 4 /* strlen(".edf") */ - 1);
			// chip away '-1' if present
			if ( fn_episode.size() >= 3 /* strlen("a-1") */ ) {
				size_t sz = fn_episode.size();
				if ( fn_episode[sz-2] == '-' && isdigit(fn_episode[sz-1]) )
					fn_episode.erase( sz-2, 2);
			}

			if ( _status & noepisode ) { // (a) failed
				episode.assign( fn_episode);    // use RecordingID_raw as Session
				session.assign( rec_id_isolated.c_str());
			} else {
				episode.assign( int_episode);
				session.assign( int_session);
			}
		}

		{
			struct tm ts;
			char *p;
			//memset( &ts, 0, sizeof(struct tm));
			ts.tm_isdst = 0;  // importantly
			p = strptime( string (header.recording_date, 8).c_str(), "%d.%m.%y", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= date_unparsable;
				return -2;
			}
			p = strptime( string (header.recording_time, 8).c_str(), "%H.%M.%S", &ts);
			if ( p == NULL || *p != '\0' ) {
				_status |= time_unparsable;
				return -2;
			}

			// if ( ts.tm_year < 50 )
			// 	ts.tm_year += 100;
			start_time = mktime( &ts);
			if ( start_time == (time_t)-1 )
				_status |= (date_unparsable|time_unparsable);
			else
				end_time = start_time + n_data_records * data_record_size;
		}

		if ( n_signals > max_signals ) {
			_status |= bad_numfld;
			return -2;
		} else {
			signals.resize( n_signals);

			for ( i = 0; i < n_signals; ++i )
				signals[i].channel = strtrim( string (_get_next_field( signals[i].header.label, 16), 16));
			        // to be parsed again wrt SignalType:Channel format

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.transducer_type, 80);

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.physical_dim, 8);

			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.physical_min, 8);
				if ( sscanf( signals[i].header.physical_min, "%8g",
					     &signals[i].physical_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.physical_max, 8);
				if ( sscanf( signals[i].header.physical_max, "%8g",
					     &signals[i].physical_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.digital_min, 8);
				if ( sscanf( signals[i].header.digital_min, "%8d",
					     &signals[i].digital_min) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}
			for ( i = 0; i < n_signals; ++i ) {
				_get_next_field( signals[i].header.digital_max, 8);
				if ( sscanf( signals[i].header.digital_max, "%8d",
					     &signals[i].digital_max) != 1 ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.filtering_info, 80);

			for ( i = 0; i < n_signals; ++i ) {
				char *tail;
				signals[i].samples_per_record =
					strtoul( strtrim( string (_get_next_field( signals[i].header.samples_per_record, 8), 8)).c_str(),
						 &tail, 10);
				if ( *tail != '\0' ) {
					_status |= bad_numfld;
					return -2;
				}
			}

			for ( i = 0; i < n_signals; ++i )
				_get_next_field( signals[i].header.reserved, 32);
		}
	} catch (TStatus ex) {
		return -1;
	} catch (invalid_argument ex) {
		_status |= bad_numfld;
		return -3;
	}

      // calculate gain
	for ( i = 0; i < n_signals; ++i )
		if ( signals[i].physical_max <= signals[i].physical_min ||
		     signals[i].digital_max  <= signals[i].digital_min  ) {
			_status |= nogain;
			return -2;
		} else
			signals[i].scale =
				(signals[i].physical_max - signals[i].physical_min) /
				(signals[i].digital_max  - signals[i].digital_min );


      // determine signal type
	for ( i = 0; i < n_signals; ++i ) {
	      // try parsing as "type channel" first
		string parsable (signals[i].channel);
		char	*_1 = strtok( &parsable[0], " :,./"),
			*_2 = strtok( NULL, " :,./");
		if ( _2 ) {
			signals[i].signal_type = _1;
			signals[i].channel = _2;  // .channel overwritten
	      // it only has a channel name
		} else {
			const char* _signal_type = SChannel::signal_type_following_kemp( signals[i].signal_type);
			if ( _signal_type )
				signals[i].signal_type = _signal_type;
			else {
				signals[i].signal_type = "(unknown type)";
				_status |= nonkemp_signaltype;
			}

			if ( not signals[i].channel.follows_system1020() ) {  // in case there are duplicate labels, rewrite
				DEF_UNIQUE_CHARP (_);
				if ( asprintf( &_, "%zu:<%s>", i, signals[i].channel.c_str()) )
					;
				signals[i].channel = _;
				_status |= non1020_channel;
			}
		}
	}

      // convenience field
	_total_samples_per_record = 0;
	for ( i = 0; i < n_signals; ++i ) {
		signals[i]._at = _total_samples_per_record;
		_total_samples_per_record += signals[i].samples_per_record;
	}

      // are channels unique?
	for ( i = 0; i < n_signals; ++i )
		for ( size_t j = 0; j < n_signals; ++j )
			if ( j != i && signals[j].channel == signals[i].channel ) {
				_status |= dup_channels;
				break;
			}
	return 0;
}







// int
// agh::CEDFFile::_put_next_field( char* field, size_t fld_size)
// {
// 	if ( _fld_pos + fld_size > _fsize )
// 		return -1;

// 	memset( (void*)_fld_pos, '\0', fld_size);

// 	memcpy( (char*)_mmapping + _fld_pos, field, fld_size);
// 	_fld_pos += fld_size;

// 	return 0;
// }


string
agh::CEDFFile::details() const
{
	ostringstream recv;
	if ( _status & bad_header )
		recv << "Bad header, or no file\n";
	else {
		char *outp;
		if ( asprintf( &outp,
			       "File\t: %s\n"
			       "PatientID\t: %s\n"
			       "Session\t: %s\n"
			       "Episode\t: %s\n"
			       "(RecordingID: \"%s\")\n"
			       "Timestamp\t: %s"
			       "# of signals\t: %zu\n"
			       "# of records\t: %zu\n"
			       "Record length\t: %zu sec\n",
			       filename(),
			       patient.c_str(),
			       session.c_str(), episode.c_str(),
			       strtrim( string (header.recording_id, 80)).c_str(),
			       asctime( localtime( &start_time)),
			       signals.size(),
			       n_data_records,
			       data_record_size) )
			;
		recv << outp;
		free( outp);

		for ( size_t i = 0; i < signals.size(); ++i ) {
			if ( asprintf( &outp,
				       "Signal %zu: Type: %s Channel: %s\n"
				       " (label: \"%s\")\n"
				       "  Transducer type\t: %s\n"
				       "  Physical dimension\t: %s\n"
				       "  Physical min\t: %g\n"
				       "  Physical max\t: %g\n"
				       "  Digital min\t: %d\n"
				       "  Digital max\t: %d\n"
				       "  Filtering info\t: %s\n"
				       "  Samples/rec\t: %zu\n"
				       "  Scale\t: %g\n"
				       "  (reserved)\t: %s\n",
				       i,
				       signals[i].signal_type.c_str(),
				       signals[i].channel.c_str(),
				       strtrim( string (signals[i].header.label, 16)).c_str(),
				       signals[i].transducer_type.c_str(),
				       signals[i].physical_dim.c_str(),
				       signals[i].physical_min,
				       signals[i].physical_max,
				       signals[i].digital_min,
				       signals[i].digital_max,
				       signals[i].filtering_info.c_str(),
				       signals[i].samples_per_record,
				       signals[i].scale,
				       signals[i].reserved.c_str()) )
				;
			recv << outp;
			free( outp);
		}
	}

	return recv.str();
}







string
agh::CEDFFile::explain_edf_status( int status)
{
	ostringstream recv;
	if ( status & bad_header )
		recv << "* Ill-formed header\n";
	if ( status & bad_version )
		recv << "* Bad Version signature (i.e., not an EDF file)\n";
	if ( status & bad_numfld )
		recv << "* Garbage in numerical fields\n";
	if ( status & date_unparsable )
		recv << "* Date field ill-formed\n";
	if ( status & time_unparsable )
		recv << "* Time field ill-formed\n";
	if ( status & nosession )
		recv << "* No session information in field RecordingID "
			"(expecting this to appear after "
			"episode designation followed by a comma)\n";
	if ( status & non1020_channel )
		recv << "* Channel designation not following the 10-20 system\n";
	if ( status & nonkemp_signaltype )
		recv << "* Signal type not listed in Kemp et al\n";
	if ( status & dup_channels )
		recv << "* Duplicate channel names\n";
	if ( status & nogain )
		recv << "* Physical or Digital Min value greater than Max\n";
	if ( status & too_many_signals )
		recv << "* Number of signals grearter than " << max_signals << "\n";

	return recv.str();
}






// eof
