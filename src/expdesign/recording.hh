// ;-*-C++-*-
/*
 *       File name:  expdesign/recording.hh
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2012-08-26
 *
 *         Purpose:  experimental design primary classes: CRecording,
 *
 *         License:  GPL
 */


#ifndef _AGH_EXPDESIGN_RECORDING_H
#define _AGH_EXPDESIGN_RECORDING_H

#include "../libsigfile/psd.hh"
#include "../libsigfile/mc.hh"
#include "../libsigfile/source.hh"

namespace agh {

using namespace std;

class CRecording
  : public sigfile::CBinnedPower,
    public sigfile::CBinnedMC {

    friend class CExpDesign;

    protected:
	int	_status;

	sigfile::CSource&
		_source;
	int	_sig_no;

	CRecording() = delete;
	void operator=( const CRecording&) = delete;

    public:
	const sigfile::CSource&
	F() const
		{
			return _source;
		}
	sigfile::CSource&
	F()  // although we shouldn't want to access CEDFFile writably from CRecording,
		{      // this shortcut saves us the trouble of AghCC->subject_by_x(,,,).measurements...
			return _source;  // on behalf of aghui::SChannelPresentation
		}
	int h() const
		{
			return _sig_no;
		}

	CRecording( sigfile::CSource& F, int sig_no,
		    const sigfile::SFFTParamSet&,
		    const sigfile::SMCParamSet&);

	const char* subject() const      {  return _source.subject(); }
	const char* session() const      {  return _source.session(); }
	const char* episode() const      {  return _source.episode(); }
	const char* channel() const      {  return _source.channel_by_id(_sig_no); }
	sigfile::SChannel::TType signal_type() const
		{  return _source.signal_type(_sig_no); }

	bool operator<( const CRecording &o) const
		{
			return _source.end_time() < o._source.start_time();
		}

	time_t start() const
		{
			return _source.start_time();
		}
	time_t end() const
		{
			return _source.end_time();
		}
};


} // namespace agh

#endif

// EOF
