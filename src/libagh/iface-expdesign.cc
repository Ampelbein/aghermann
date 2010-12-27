// ;-*-C++-*- *  Time-stamp: "2010-12-27 02:50:22 hmmr"
/*
 *       File name:  core/iface-expdesign.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada (johnhommer@gmail.com)
 * Initial version:  2008-04-28
 *
 *         Purpose:  wrappers to access, manipulate core structures
 *
 *         License:  GPL
 */


#include <sys/stat.h>

#include <iterator>
#include <stdexcept>
#include <sstream>

#include "iface.h"

#include "edf.hh"
#include "primaries.hh"
#include "model.hh"



CExpDesign *AghCC;


#ifdef __cplusplus
extern "C" {
#endif





#define ENUMERATE_THIS(WHAT)						\
	list<string> recp0;						\
	size_t count = AghCC -> WHAT( recp0);				\
	if ( recp ) {							\
		(*recp) = (char**)malloc( (recp0.size()+1) * sizeof(char*)); \
		size_t n;						\
		auto I = recp0.begin();					\
		for ( n = 0; n < recp0.size(); ++n, ++I )		\
			(*recp)[n] = strdup( I->c_str());		\
		(*recp)[n] = NULL;					\
	}								\
	return count;

#define ENUMERATE_THIS_TOO(WHAT)					\
	list<SChannel> recp0;						\
	size_t count = AghCC -> WHAT( recp0);				\
	if ( recp ) {							\
		(*recp) = (char**)malloc( (recp0.size()+1) * sizeof(char*)); \
		size_t n;						\
		auto I = recp0.begin();					\
		for ( n = 0; n < recp0.size(); ++n, ++I )		\
			(*recp)[n] = strdup( I->c_str());		\
		(*recp)[n] = NULL;					\
	}								\
	return count;

size_t
agh_enumerate_groups( char*** recp)
{
	ENUMERATE_THIS (enumerate_groups);
}

size_t
agh_enumerate_subjects( char*** recp)
{
	ENUMERATE_THIS (enumerate_subjects);
}

size_t
agh_enumerate_sessions( char*** recp)
{
	ENUMERATE_THIS (enumerate_sessions);
}

size_t
agh_enumerate_episodes( char*** recp)
{
	ENUMERATE_THIS (enumerate_episodes);
}

size_t
agh_enumerate_all_channels( char*** recp)
{
	ENUMERATE_THIS_TOO (enumerate_all_channels);
}

size_t
agh_enumerate_eeg_channels( char*** recp)
{
	ENUMERATE_THIS_TOO (enumerate_eeg_channels);
}


void
agh_free_enumerated_array( char** what)
{
	size_t i = 0;
	while ( what[i] )
		free( what[i++]);
	free( what);
}





// --- expdesign

int
agh_expdesign_init( const char* dir, TProgressIndicatorFun fun)
{
	try {
		AghCC = new CExpDesign (dir, fun);

		if ( !AghCC ) {
			fprintf( stderr, "agh_expdesign_init(): AghCC is NULL\n");
			return -1;
		} else
			return 0;
	} catch (invalid_argument ex) {
		fprintf( stderr, "agh_expdesign_init(\"%s\"): %s\n",
			 dir, AghCC->error_log());
		return -1;
	}
}


void
agh_expdesign_shutdown()
{
	delete AghCC;
	AghCC = NULL;
}



int
agh_expdesign_status()
{
	return AghCC ? AghCC->status() : -1;
}

const char*
agh_expdesign_messages()
{
	return AghCC ? AghCC->error_log() : "CExpDesign structure is NULL";
}

const char*
agh_expdesign_last_message()
{
	return AghCC ? AghCC->last_error() : "CExpDesign structure is NULL";
}



void
agh_expdesign_scan_tree( TProgressIndicatorFun fun)
{
	AghCC -> scan_tree( fun);
}


static void __copy_subject_class_to_struct( struct SSubject* _j, const CSubject& J);

void
agh_expdesign_snapshot( SExpDesign* ed)
{
	fprintf( stderr, "agh_expdesign_snapshot ");
	agh_SExpDesign_destruct( ed);
	fprintf( stderr, "(agh_SExpDesign_destruct) ");

	ed->session_dir = AghCC->session_dir();
	ed->groups = (SGroup*)malloc( sizeof(SGroup) * (ed->n_groups = AghCC -> n_groups()));
	size_t g = 0;
	for ( auto G = AghCC->groups_begin(); G != AghCC->groups_end(); ++G, ++g ) {
		struct SGroup& __g = ed->groups[g];
		__g.name = G->first.c_str();
		__g.subjects = (SSubject*)malloc( sizeof(SSubject) * (__g.n_subjects = G->second.size()));
		size_t j = 0;
		for ( auto J = G->second.begin(); J != G->second.end(); ++J, ++j )
			__copy_subject_class_to_struct( &__g.subjects[j], *J);
	}
	fprintf( stderr, "done\n");
}

void
agh_SExpDesign_destruct( SExpDesign* ed)
{
	for ( size_t g = 0; g < ed->n_groups; ++g ) {
		struct SGroup& __g = ed->groups[g];
		for ( size_t j = 0; j < __g.n_subjects; ++j )
			agh_SSubject_destruct( &__g.subjects[j]);
		free( __g.subjects);
	}
	free( ed->groups);
}



static struct SSubject __subject_consumable;
static void
__copy_subject_class_to_struct( struct SSubject* _j, const CSubject& J)
{
	_j->name    = J.name();
	_j->gender  = J.gender();
	_j->age     = J.age();
	_j->comment = J.comment();

	_j->sessions = (SSession*)malloc( sizeof(SSession) * (_j->n_sessions = J.measurements.size()));
	size_t d = 0;
	for ( auto Di = J.measurements.begin(); Di != J.measurements.end(); ++Di, ++d ) {
		SSession& __d = _j->sessions[d];
		__d.name = Di->first.c_str();

		// part one: recordings
		__d.episodes = (SEpisode*)malloc( sizeof(SEpisode) * (__d.n_episodes = Di->second.episodes.size()));
		size_t e = 0;
		long shift = 0;
		for ( auto Ei = Di->second.episodes.begin(); Ei != Di->second.episodes.end(); ++Ei, ++e ) {
			SEpisode& __e = __d.episodes[e];
			const CEDFFile& F = *Ei->sources.begin();
			__e.name   = Ei->name();
			__e.length = F.length();
			__e.start  = F.start_time;
			__e.end    = F.end_time;

			// shifting and aligning episode sequences is done here
			if ( Ei == Di->second.episodes.begin() ) {
				struct tm a_start;
				memcpy( &a_start, localtime( &__e.start), sizeof(struct tm));

				a_start.tm_year = 109;
				a_start.tm_mon = 1;

				// take care of larks going to bed before midnight
				int early_hours_start = (a_start.tm_hour < 12);
				a_start.tm_mday = 1 + early_hours_start;
				__e.start_rel	= mktime(&a_start);

				shift = (long)difftime( __e.start, __e.start_rel);
				__e.end_rel	= __e.end - shift;

			} else {
				__e.start_rel	= __e.start - shift;
				__e.end_rel	= __e.end - shift;
			}

			__e.recordings = (TRecRef*)malloc( sizeof(TRecRef) * (__e.n_recordings = Ei->recordings.size()));
			size_t h = 0;
			for ( auto Hi = Ei->recordings.begin(); Hi != Ei->recordings.end(); ++Hi, ++h )
				__e.recordings[h] = static_cast<TRecRef>(const_cast<CRecording*>(&Hi->second));
		}

		// part two: simulations
		__d.modrun_sets = (SModelRunSet*)malloc( sizeof(SModelRunSet) * (__d.n_modrun_sets = Di->second.modrun_sets.size()));
		size_t rs = 0;
		for ( auto RS = Di->second.modrun_sets.begin(); RS != Di->second.modrun_sets.end(); ++RS, ++rs ) {
			auto &__rs = __d.modrun_sets[rs];
			__rs.channel = RS->first.c_str();  // channel
			__rs.modruns = (SModelRun*)malloc( sizeof(SModelRun) * (__rs.n_modruns = RS->second.size()));
			size_t r = 0;
			for ( auto R = RS->second.begin(); R != RS->second.end(); ++R, ++r ) {
				auto &__r = __rs.modruns[r];
				__r.from   = R->first.first;
				__r.upto   = R->first.second;
				__r.modref = static_cast <TModelRef> ( const_cast<CSimulation*>(&R->second) );
			}
		}
	}
}







// edf ------------

void
agh_SEDFFile_destruct( struct SEDFFile *f)
{
	free( const_cast<char*>(f->filename));
	free( const_cast<char*>(f->PatientID));
	free( const_cast<char*>(f->Session));
	free( const_cast<char*>(f->Episode));
	for ( size_t h = 0; h < f->NSignals; ++h ) {
		free( const_cast<char*>(f->signals[h].Channel));
		free( const_cast<char*>(f->signals[h].SignalType));
	}
	free( f->signals);
}

static void
__dup_edf_class_to_struct( struct SEDFFile* _F, const CEDFFile& F)
{
	_F->status           = F.status();
	_F->filename         = strdup( F.filename());
	_F->PatientID        = strdup( F.PatientID_raw);
	_F->Session          = strdup( F.Session.c_str());
	_F->Episode          = strdup( F.Episode.c_str());
	_F->NDataRecords     = F.NDataRecords;
	_F->DataRecordSize   = F.DataRecordSize;
	_F->NSignals         = F.NSignals;
	_F->timestamp_struct = F.timestamp_struct;
	_F->start_time	     = F.start_time;
	_F->end_time	     = F.end_time;
	_F->signals          = (SSignal_lite*)malloc( _F->NSignals * sizeof(SSignal_lite));
	for ( size_t h = 0; h < _F->NSignals; ++h ) {
		_F->signals[h].Channel          = strdup( F[h].Channel.c_str());
		_F->signals[h].SignalType       = strdup( F[h].SignalType.c_str());
		_F->signals[h].SamplesPerRecord = F[h].SamplesPerRecord;
	}
}

struct SEDFFile*
agh_edf_get_info_from_file( const char *fname, char **out_p)
{
	try {
		CEDFFile F (fname, AghCC->fft_params.page_size);
		if ( out_p )
			(*out_p) = strdup( F.details().c_str());

		struct SEDFFile *o = (struct SEDFFile*)malloc( sizeof(struct SEDFFile));
		__dup_edf_class_to_struct( o, F);
		return o;

	} catch ( invalid_argument ex) {
		return NULL;
	}
}


static struct SEDFFile
	__edf_consumer_struct;

static void
__copy_edf_class_to_struct( struct SEDFFile* _F, const CEDFFile& F)
{
	_F->status           = F.status();
	_F->filename         = F.filename();
	_F->PatientID        = F.PatientID_raw;
	_F->Session          = F.Session.c_str();
	_F->Episode          = F.Episode.c_str();
	_F->NDataRecords     = F.NDataRecords;
	_F->DataRecordSize   = F.DataRecordSize;
	_F->NSignals         = F.NSignals;
	_F->timestamp_struct = F.timestamp_struct;
	_F->start_time	     = F.start_time;
	_F->end_time	     = F.end_time;
	_F->signals          = (SSignal_lite*)realloc( _F->signals,
						      _F->NSignals * sizeof(SSignal_lite));
	for ( size_t h = 0; h < _F->NSignals; ++h ) {
		_F->signals[h].Channel          = F[h].Channel.c_str();
		_F->signals[h].SignalType       = F[h].SignalType.c_str();
		_F->signals[h].SamplesPerRecord = F[h].SamplesPerRecord;
	}
}
const struct SEDFFile*
agh_edf_get_info_from_sourceref( TEDFRef Fp, char **out_p)
{
	CEDFFile& F = *static_cast<CEDFFile*>(Fp);
	if ( out_p )
		(*out_p) = strdup( F.details().c_str());
	__copy_edf_class_to_struct( &__edf_consumer_struct, F);
	return &__edf_consumer_struct;
}


// because multiple edf's can supply multiple channels for the same
// episode, we also search by channel
const struct SEDFFile*
agh_edf_find_by_jdeh( const char* j, const char* d, const char* e, const char* h,
		     TEDFRef* Fp)
{
	try {
		CSubject& J = AghCC -> subject_by_x(j);
		list<CEDFFile>& FF = J.measurements.at(d)[e].sources;
		auto F = FF.begin();
		for ( ; F != FF.end(); ++F )
			if ( F->have_channel(h) )
				break;
		if ( F == FF.end() )
			return NULL;
		if ( Fp )
			*Fp = &*F;
		__copy_edf_class_to_struct( &__edf_consumer_struct, *F);
		return &__edf_consumer_struct;
	} catch (invalid_argument ex) {
		return NULL;
	}
}



// int
// agh_edf_write_header( TEDFRef Fp)
// {
// 	CEDFFile& F = *static_cast<CEDFFile*>(Fp);
// 	return F.write_header();
// }




size_t
agh_edf_get_scores( TEDFRef _F,
		    char **scores, size_t *pagesize_p)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);

	*scores = (char*)malloc( F.CHypnogram::length() * sizeof(char));

	for ( size_t p = 0; p < F.CHypnogram::length(); ++p )
		(*scores)[p] = F.nth_page(p).p2score();

	if ( pagesize_p)
		*pagesize_p = F.pagesize();

	return F.CHypnogram::length();
}



int
agh_edf_put_scores( TEDFRef _F,
		    char *scores)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);

	for ( size_t p = 0; p < F.CHypnogram::length() && scores[p]; ++p )
		if ( scores[p] == AghScoreCodes[AGH_SCORE_NREM1] )
			F.nth_page(p).NREM =  .25;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_NREM2] )
			F.nth_page(p).NREM =  .50;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_NREM3] )
			F.nth_page(p).NREM =  .75;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_NREM4] )
			F.nth_page(p).NREM = 1.;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_REM] )
			F.nth_page(p).REM  = 1.;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_WAKE] )
			F.nth_page(p).Wake = 1.;
		else if ( scores[p] == AghScoreCodes[AGH_SCORE_MVT] )
			F.nth_page(p).Wake = AGH_MVT_WAKE_VALUE; // .01 is specially reserved for MVT
	return 0;
}


float
agh_edf_get_percent_scored( TEDFRef _F)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	return F.percent_scored();
}

float
agh_edf_get_scored_stages_breakdown( TEDFRef _F, float *nrem_p, float *rem_p, float *wake_p)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	return F.percent_scored( nrem_p, rem_p, wake_p);
}



int
agh_edf_import_scores( TEDFRef _F,
		       const char *fname)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	return F.load_canonical( fname);
}


int
agh_edf_export_scores( TEDFRef _F,
		       const char *fname)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	return F.save_canonical( fname);
}






size_t
agh_edf_get_unfazers( TEDFRef _F, const char *affected_channel, struct SUnfazer **recp)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	int	h = F.which_channel( affected_channel);
	if ( h == -1 ) {
		fprintf( stderr, "agh_edf_get_unfazers(): no such channel: \"%s\"\n",
			 affected_channel);
		*recp = NULL;
		return 0;
	}

	auto &unfs = F[h].interferences;
	size_t i = 0;
	if ( unfs.size() ) {
		*recp = (struct SUnfazer*)malloc( unfs.size() * sizeof(struct SUnfazer));
		for ( auto U = unfs.begin(); U != unfs.end(); ++U, ++i ) {
			(*recp)[i].channel = F.signals[U->h].Channel.c_str();
			(*recp)[i].factor = U->fac;
		}
	} else
		*recp = NULL;

	return unfs.size();
}


int
agh_edf_add_or_mod_unfazer( TEDFRef _F,
			    const char *target_channel,
			    const char *offending_channel, double factor)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	int	ho = F.which_channel( offending_channel),
		ht = F.which_channel( target_channel);
	if ( ho == -1 || ht == -1 || ho == ht ) {
		fprintf( stderr, "agh_edf_add_or_mod_unfazer(): target (\"%s\") or offending (%s\") channel do not exist in this source, or the are the same\n",
			 target_channel, offending_channel);
		return -1;
	}
	auto &unfs = F[ht].interferences;
	auto U = find( unfs.begin(), unfs.end(), CEDFFile::SSignal::SUnfazer (ho));
	if ( U == unfs.end() )
		unfs.emplace_back( ho, factor);
	else {
		U->fac = factor;
	}

	return 0;
}

void
agh_edf_remove_unfazer( TEDFRef _F,
			const char *target_channel,
			const char *offending_channel)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	int	ho = F.which_channel( offending_channel),
		ht = F.which_channel( target_channel);
	if ( ho == -1 || ht == -1 || ho == ht ) {
		fprintf( stderr, "agh_edf_remove_unfazer(): target (\"%s\") or offending (%s\") channel do not exist in this source, or the are the same\n",
			 target_channel, offending_channel);
		return;
	}

	auto &unfs = F[ht].interferences;
	auto U = find( unfs.begin(), unfs.end(), CEDFFile::SSignal::SUnfazer (ho));
	if ( U != unfs.end() )
		unfs.erase(U);
}


double
agh_edf_get_unfazer_factor( TEDFRef _F,
			    const char *target_channel,
			    const char *offending_channel)
{
	CEDFFile& F = *static_cast<CEDFFile*>(_F);
	int	ho = F.which_channel( offending_channel),
		ht = F.which_channel( target_channel);
	if ( ho == -1 || ht == -1 || ho == ht ) {
		fprintf( stderr, "agh_edf_remove_unfazer(): target (\"%s\") or offending (%s\") channel do not exist in this source, or the are the same\n",
			 target_channel, offending_channel);
		return NAN;
	}

	auto &unfs = F[ht].interferences;
	auto U = find( unfs.begin(), unfs.end(), CEDFFile::SSignal::SUnfazer (ho));
	return ( U != unfs.end() ) ? U->fac : NAN;
}





size_t
agh_edf_get_artifacts( TEDFRef ref, const char *channel,
		       size_t **outp)
{
	CEDFFile& F = *static_cast<CEDFFile*>(ref);

	int h = F.which_channel(channel);
	if ( h != -1 ) {
		auto &AA = F.signals[h].artifacts;
		assert( (*outp) = (size_t*)malloc( AA.size() * sizeof(size_t) * 2));
		size_t a = 0;
		for ( auto A = AA.begin(); A != AA.end(); ++A ) {
			(*outp)[a++] = A->first;
			(*outp)[a++] = A->second;
		}
		return AA.size();
	} else
		return 0;
}


void
agh_edf_mark_artifact( TEDFRef ref, const char *channel,
		       size_t aa, size_t az)
{
	CEDFFile& F = *static_cast<CEDFFile*>(ref);

	int h = F.which_channel(channel);
	if ( h != -1 )
		F.signals[h].mark_artifact( aa, az);
}

void
agh_edf_clear_artifact( TEDFRef ref, const char *channel,
			size_t aa, size_t az)
{
	CEDFFile& F = *static_cast<CEDFFile*>(ref);

	int h = F.which_channel(channel);
	if ( h != -1 )
		F.signals[h].clear_artifact( aa, az);
}





void
agh_explain_edf_status( int status, char **out_p)
{
	string st (explain_edf_status( status));
	(*out_p) = strdup( st.c_str());
}







// --- group

static map<string, CJGroup>::iterator __agh_group_iter;
static bool __agh_group_iter_valid = false;

const char*
agh_group_find_first()
{
	if ( AghCC->groups_begin() == AghCC->groups_end() )
		return NULL;
	__agh_group_iter_valid = true;
	return ((__agh_group_iter = AghCC->groups_begin())++) -> first.c_str();
}

const char*
agh_group_find_next()
{
	if ( !__agh_group_iter_valid )
		return agh_group_find_first();
	else
		if ( __agh_group_iter != AghCC->groups_end() )
			return (__agh_group_iter++) -> first.c_str();
		else
			return NULL;
}





// --- subject

size_t
agh_subject_get_n_of()
{
	size_t cnt = 0;
	for ( auto G = AghCC->groups_begin(); G != AghCC->groups_end(); ++G )
		cnt += G->second.size();
	return cnt;
}

size_t
agh_subject_get_n_of_in_group( const char *g)
{
	return AghCC->have_group(g) ? AghCC->group_by_name(g).size() : (size_t)-1;
}


size_t
agh_subject_get_path( const char *j, char **outp)
{
	try {
		UNIQUE_CHARP(_);
		if ( asprintf( &_, "%s/%s/%s", AghCC->session_dir(), AghCC->group_of( j), j) )
			;
		*outp = strdup( _);
		return strlen( _);
	} catch ( invalid_argument ex) {
		return 0;
	}
}


static CJGroup::iterator __agh_subject_iter;
static CJGroup *__agh_subject_iter_in_group = NULL;

static const struct SSubject*
agh_get_subject_in_group_first_or_next( const char* g, struct SSubject* recp)
{
	if ( g )
		if ( AghCC -> have_group(g) ) {
			__agh_subject_iter_in_group = &(AghCC -> group_by_name(g));
			__agh_subject_iter = __agh_subject_iter_in_group -> begin();
		} else
			return NULL;
	else
		if ( __agh_subject_iter == __agh_subject_iter_in_group->end() )
			return NULL;

	agh_SSubject_destruct( &__subject_consumable);
	__copy_subject_class_to_struct( &__subject_consumable, *__agh_subject_iter);
	if ( recp )
		__copy_subject_class_to_struct( recp, *__agh_subject_iter);
	++__agh_subject_iter;

	return &__subject_consumable;
}

const struct SSubject*
agh_subject_find_first_in_group( const char* g_name, struct SSubject* recp)
{
	return agh_get_subject_in_group_first_or_next( g_name, recp);
}

const struct SSubject*
agh_subject_find_next_in_group( struct SSubject* recp)
{
	return agh_get_subject_in_group_first_or_next( NULL, recp);
}

const struct SSubject*
agh_subject_find_by_name( const char* j_name, struct SSubject* recp)
{
	try {
		CSubject& J = AghCC -> subject_by_x( j_name);
		agh_SSubject_destruct( &__subject_consumable);
		__copy_subject_class_to_struct( &__subject_consumable, J);
		if ( recp )
			__copy_subject_class_to_struct( recp, J);
		return &__subject_consumable;
	} catch (invalid_argument ex) {
		return NULL;
	}
}

void
agh_SSubject_destruct( struct SSubject* _j)
{
	for ( size_t d = 0; d < _j->n_sessions; ++d ) {
		SSession& __d = _j->sessions[d];
		for ( size_t e = 0; e < __d.n_episodes; ++e ) {
			SEpisode& __e = __d.episodes[e];
			free( __e.recordings);
		}
		free( __d.episodes);

		for ( size_t rs = 0; rs < __d.n_modrun_sets; ++rs ) {
			SModelRunSet& __rs = __d.modrun_sets[rs];
			free( __rs.modruns);
		}
		free( __d.modrun_sets);
	}
	free( _j->sessions);
}








float
agh_group_avg_episode_times( const char *g_name, const char *d_name, const char *e_name,
			     struct SEpisodeTimes *recp)
{
	try {
		CJGroup &G = AghCC -> group_by_name( g_name);
		pair<float, float> &avge = G.avg_episode_times[d_name][e_name];
		if ( recp ) {
			unsigned seconds = avge.first * 24 * 60 * 60;
			recp->start_hour = seconds / 60 / 60;
			recp->start_min  = seconds % 3600 / 60;
			recp->start_sec  = seconds % 60;
			seconds = avge.second * 24 * 60 * 60;
			recp->end_hour = seconds / 60 / 60;
			recp->end_min  = seconds % 3600 / 60;
			recp->end_sec  = seconds % 60;
		}
		return avge.second - avge.first;

	} catch (invalid_argument ex) {
		fprintf( stderr, "agh_group_avg_episode_times(\"%s\", \"%s\", \"%s\"): %s\n",
			 g_name, d_name, e_name, ex.what());
		return -1;
	} catch (out_of_range ex) {
		fprintf( stderr, "agh_group_avg_episode_times(\"%s\", \"%s\", \"%s\"): %s\n",
			 g_name, d_name, e_name, ex.what());
		return -1;
	}
	return -1.;
}




// --- measurements

TRecRef
agh_msmt_find_by_jdeh( const char *j_name,
		       const char *d_name,
		       const char *e_name,
		       const char *h_name)
{
	try {
		// CSubject &J = AghCC -> subject_by_x(j_name);
		// CSubject::SEpisodeSequence &D = J.measurements.at(d_name);
		// CSubject::SEpisode &E = D[e_name];
		// CRecording &R = E.recordings.at(h_name);
		return static_cast<TRecRef> (&(AghCC -> subject_by_x(j_name)
					       . measurements.at(d_name)[e_name]
					       . recordings.at(h_name)));
	} catch (invalid_argument ex) {
		fprintf( stderr, "agh_msmt_find_by_jdeh(\"%s\", \"%s\", \"%s\", \"%s\"): %s\n",
			 j_name, d_name, e_name, h_name, ex.what());
		return (TRecRef)NULL;
	} catch (out_of_range ex) {
		fprintf( stderr, "agh_msmt_find_by_jdeh(\"%s\", \"%s\", \"%s\", \"%s\"): %s\n",
			 j_name, d_name, e_name, h_name, ex.what());
		return (TRecRef)NULL;
	}
}

TEDFRef
agh_msmt_get_source( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return static_cast<TEDFRef> (const_cast<CEDFFile*> (&K.F()));
}


const char*
agh_msmt_get_signal_type( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return K.signal_type();
}

const char*
agh_msmt_get_signal_name( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return K.channel();
}

size_t
agh_msmt_get_pagesize( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return K.pagesize();
}

float
agh_msmt_get_binsize( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return K.binsize();
}

size_t
agh_msmt_get_n_bins( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	return K.n_bins();
}


size_t
agh_msmt_get_signal_original_as_double( TRecRef ref,
					double** buffer_p,
					size_t *samplerate, float *signal_scale)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<double> tmp;
	size_t n_samples = F.NDataRecords * F[K.h()].SamplesPerRecord;
	// printf( "get_signal_data( %d(=%s), 0, %zu); n_samples = %zu DataRecords x %zu RecordSize\n",
	// 	K.h(), F[K.h()].Label, n_samples, F.NDataRecords, F[K.h()].SamplesPerRecord);
	F.get_signal_original( K.h(), tmp);

	(*buffer_p) = (double*)malloc( n_samples * sizeof(double));
	assert (*buffer_p != NULL );

	memcpy( *buffer_p, &tmp[0], sizeof(double) * n_samples);

	if ( samplerate )
		*samplerate = F[K.h()].SamplesPerRecord / F.DataRecordSize;
	if ( signal_scale )
		*signal_scale = F[K.h()].Scale;

	return n_samples;
}

size_t
agh_msmt_get_signal_original_as_float( TRecRef ref,
				       float** buffer_p,
				       size_t *samplerate, float *signal_scale)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<float> tmp;
	size_t n_samples = F.NDataRecords * F[K.h()].SamplesPerRecord;
	F.get_signal_original( K.h(), tmp);

	(*buffer_p) = (float*)malloc( n_samples * sizeof(float));
	assert (*buffer_p != NULL);

	memcpy( *buffer_p, &tmp[0], sizeof(float) * n_samples);

	if ( samplerate )
		*samplerate = F[K.h()].SamplesPerRecord / F.DataRecordSize;
	if ( signal_scale )
		*signal_scale = F[K.h()].Scale;

	return n_samples;
}


size_t
agh_msmt_get_signal_filtered_as_double( TRecRef ref,
					double** buffer_p,
					size_t *samplerate, float *signal_scale)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<double> tmp;
	size_t n_samples = F.NDataRecords * F[K.h()].SamplesPerRecord;
	F.get_signal_filtered( K.h(), tmp);

	(*buffer_p) = (double*)malloc( n_samples * sizeof(double));
	assert (*buffer_p != NULL );

	memcpy( *buffer_p, &tmp[0], sizeof(double) * n_samples);

	if ( samplerate )
		*samplerate = F[K.h()].SamplesPerRecord / F.DataRecordSize;
	if ( signal_scale )
		*signal_scale = F[K.h()].Scale;

	return n_samples;
}

size_t
agh_msmt_get_signal_filtered_as_float( TRecRef ref,
				       float** buffer_p,
				       size_t *samplerate, float *signal_scale)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<float> tmp;
	size_t n_samples = F.NDataRecords * F[K.h()].SamplesPerRecord;
	F.get_signal_filtered( K.h(), tmp);
	(*buffer_p) = (float*)malloc( n_samples * sizeof(float));
	assert (*buffer_p != NULL);

	memcpy( *buffer_p, &tmp[0], sizeof(float) * n_samples);

	if ( samplerate )
		*samplerate = F[K.h()].SamplesPerRecord / F.DataRecordSize;
	if ( signal_scale )
		*signal_scale = F[K.h()].Scale;

	return n_samples;
}



size_t
agh_msmt_get_signal_dzcdf( TRecRef ref,
			   float **buffer_p,
			   float dt, float sigma, float window, size_t smooth)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<float> tmp;
	if ( F.get_dzcdf( K.h(), tmp, dt, sigma, window, smooth) == 0 )
		return 0;

	(*buffer_p) = (float*)malloc( tmp.size() * sizeof(float));
	assert (*buffer_p != NULL);

	memcpy( *buffer_p, &tmp[0], sizeof(float) * tmp.size());

	return tmp.size();
}

size_t
agh_msmt_get_signal_shape( TRecRef ref,
			   size_t **buffer_l, size_t *buffer_l_size_p,
			   size_t **buffer_u, size_t *buffer_u_size_p,
			   size_t over)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	vector<size_t> tmp_l, tmp_u;
	if ( F.get_shape( K.h(), tmp_l, tmp_u, over) == 0 )
		return 0;

	(*buffer_l) = (size_t*)malloc( (*buffer_l_size_p = tmp_l.size()) * sizeof(size_t));
	(*buffer_u) = (size_t*)malloc( (*buffer_u_size_p = tmp_u.size()) * sizeof(size_t));
	assert (*buffer_l && *buffer_u);

	memcpy( *buffer_l, &tmp_l[0], sizeof(size_t) * tmp_l.size());
	memcpy( *buffer_u, &tmp_u[0], sizeof(size_t) * tmp_u.size());

	return tmp_u.size();
}



size_t
agh_msmt_find_pattern( TRecRef ref,
		       size_t pa, size_t pz, size_t start,
		       float cutoff,
		       float sigma,
		       float tolerance,
		       size_t tightness)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	const CEDFFile& F = K.F();

	valarray<float> sought;
	F.get_region_filtered( K.h(), pa, pz, sought);

	CSignalPattern<float> pattern (sought, F.samplerate(K.h()), cutoff, sigma, tightness);
	return F.find_pattern( K.h(), pattern, start, tolerance);
}



char*
agh_msmt_fname_base( TRecRef ref)
{
	CRecording& K = *static_cast<CRecording*>(ref);

	return strdup( K.fname_base().c_str());
}

int
agh_msmt_export_power( TRecRef ref, const char *fname)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	return K.export_tsv( fname);
}
int
agh_msmt_export_power_in_range( TRecRef ref,
				float from, float upto,
				const char *fname)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	return K.export_tsv( from, upto, fname);
}





size_t
agh_msmt_get_power_spectrum_as_double( TRecRef ref, size_t p,
				       double **out, double *max_p)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<double> power_acc = K.power_spectrum(p);
	*out = (double*)malloc( K.n_bins() * sizeof(double));
	memcpy( *out, &power_acc[0], K.n_bins() * sizeof(double));
	if ( max_p )
		*max_p = power_acc.max();

	return K.n_bins();
}

size_t
agh_msmt_get_power_spectrum_as_float( TRecRef ref, size_t p,
				      float **out, float *max_p)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<float> power_acc = K.power_spectrumf(p);
	*out = (float*)malloc( K.n_bins() * sizeof(float));
	memcpy( *out, &power_acc[0], K.n_bins() * sizeof(float));
	if ( max_p )
		*max_p = power_acc.max();

	return K.n_bins();
}


size_t
agh_msmt_get_power_course_as_double( TRecRef ref,
				     double **out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<double> power_acc = K.power_course();
	size_t n_pages_by_bins = power_acc.size();
	*out = (double*)malloc( n_pages_by_bins * sizeof(double));
	memcpy( *out, &power_acc[0], n_pages_by_bins * sizeof(double));

	return n_pages_by_bins;
}

size_t
agh_msmt_get_power_course_as_float( TRecRef ref,
				    float **out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<float> power_acc = K.power_coursef();
	size_t n_pages_by_bins = power_acc.size();
	*out = (float*)malloc( n_pages_by_bins * sizeof(float));
	memcpy( *out, &power_acc[0], n_pages_by_bins * sizeof(float));

	return n_pages_by_bins;
}


size_t
agh_msmt_get_power_course_as_double_direct( TRecRef ref,
					    double *out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<double> power_acc = K.power_course();
	size_t n_pages_by_bins = power_acc.size();
	memcpy( out, &power_acc[0], n_pages_by_bins * sizeof(double));

	return n_pages_by_bins;
}

size_t
agh_msmt_get_power_course_as_float_direct( TRecRef ref,
					   float *out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<float> power_acc = K.power_coursef();
	size_t n_pages_by_bins = power_acc.size();
	memcpy( out, &power_acc[0], n_pages_by_bins * sizeof(float));

	return n_pages_by_bins;
}




size_t
agh_msmt_get_power_course_in_range_as_double( TRecRef ref,
					      float from, float upto,
					      double **out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<double> power_acc = K.power_course( from, upto);
	size_t n_pages = power_acc.size();
	*out = (double*)malloc( n_pages * sizeof(double));
	memcpy( *out, &power_acc[0], n_pages * sizeof(double));

	return n_pages;
}

size_t
agh_msmt_get_power_course_in_range_as_float( TRecRef ref,
					     float from, float upto,
					     float **out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<float> power_acc = K.power_coursef( from, upto);
	size_t n_pages = power_acc.size();
	*out = (float*)malloc( n_pages * sizeof(float));
	memcpy( *out, &power_acc[0], n_pages * sizeof(float));

	return n_pages;
}


size_t
agh_msmt_get_power_course_in_range_as_double_direct( TRecRef ref,
						     float from, float upto,
						     double *out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<double> power_acc (K.power_course( from, upto));
	size_t n_pages = power_acc.size();
	memcpy( out, &power_acc[0], n_pages * sizeof(double));

	return n_pages;
}

size_t
agh_msmt_get_power_course_in_range_as_float_direct( TRecRef ref,
						    float from, float upto,
						    float *out)
{
	CRecording& K = *static_cast<CRecording*>(ref);
	K.obtain_power();

	valarray<float> power_acc (K.power_coursef( from, upto));
	size_t n_pages = power_acc.size();
	memcpy( out, &power_acc[0], n_pages * sizeof(float));

	return n_pages;
}





int
agh_episode_assisted_score_by_jde( const char *j_name, const char *d_name, const char *e_name)
{
	try {
		return AghCC -> subject_by_x(j_name)
			. measurements.at(d_name)[e_name]
			. assisted_score();
	} catch (invalid_argument ex) {
		fprintf( stderr, "agh_episode_assisted_score_by_jde(\"%s\", \"%s\", \"%s\"): %s\n",
			 j_name, d_name, e_name, ex.what());
		return -1;
	} catch (out_of_range ex) {
		fprintf( stderr, "agh_episode_assisted_score_by_jde(\"%s\", \"%s\", \"%s\"): %s\n",
			 j_name, d_name, e_name, ex.what());
		return -1;
	}
}




// --- fft

size_t
agh_fft_get_pagesize()
{
	return AghCC -> fft_params.page_size;
}

void
agh_fft_set_pagesize( size_t val)
{
	AghCC->fft_params.page_size = val;
}

float
agh_fft_get_binsize()
{
	return AghCC -> fft_params.bin_size;
}

void
agh_fft_set_binsize( float val)
{
	AghCC->fft_params.bin_size = val;
}


TFFTWinType
agh_fft_get_window_type()
{
	return AghCC->fft_params.welch_window_type;
}

void
agh_fft_set_window_type( TFFTWinType value)
{
	AghCC->fft_params.welch_window_type = value;
}



TFFTWinType
agh_af_get_window_type()
{
	return AghCC->af_dampen_window_type;
}

void
agh_af_set_window_type( TFFTWinType value)
{
	AghCC->af_dampen_window_type = value;
}

size_t
agh_af_get_smoothover()
{
	return AghCC->fft_params.smoothover;
}

void
agh_af_set_smoothover( size_t value)
{
	AghCC->fft_params.smoothover = value;
}









// --- expdesign



// standalone

int
agh_signal_type_is_fftable( const char* signal)
{
	return signal_type_is_fftable(signal);
}

const char*
agh_signal_type_following_Kemp( const char* h)
{
	return signal_type_following_Kemp(h);
}

int
agh_channel_follows_1020( const char* h)
{
	return channel_follows_1020(h);
}



#ifdef __cplusplus
}
#endif

// EOF
