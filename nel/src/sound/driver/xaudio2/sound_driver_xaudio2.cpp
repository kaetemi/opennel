/**
 * \file sound_driver_xaudio2.cpp
 * \brief CSoundDriverXAudio2
 * \date 2008-08-20 10:52GMT
 * \author Jan Boon (Kaetemi)
 * CSoundDriverXAudio2
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2008  Jan Boon (Kaetemi)
 * 
 * This file is part of NLSOUND XAudio2 Driver.
 * NLSOUND XAudio2 Driver is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * NLSOUND XAudio2 Driver is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NLSOUND XAudio2 Driver; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include "stdxaudio2.h"
#include "sound_driver_xaudio2.h"

// Project includes
#include "sample_voice_xaudio2.h"
#include "listener_xaudio2.h"
#include "source_xaudio2.h"
#include "music_channel_xaudio2.h"

// NeL includes
#include "../sound_driver.h"
#include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/dynloadlib.h>

// STL includes
#include <cmath>

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

// ******************************************************************

#ifndef NL_STATIC
class CSoundDriverXAudio2NelLibrary : public NLMISC::INelLibrary { 
	void onLibraryLoaded(bool firstTime) { } 
	void onLibraryUnloaded(bool lastTime) { }  
};
NLMISC_DECL_PURE_LIB(CSoundDriverXAudio2NelLibrary)

HINSTANCE CSoundDriverXAudio2DllHandle = NULL;
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	CSoundDriverXAudio2DllHandle = (HINSTANCE)hModule;
	return TRUE;
}
#endif /* #ifndef NL_STATIC */

// ***************************************************************************

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstance
#else
__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance
#endif
	(bool useEax, ISoundDriver::IStringMapperProvider *stringMapper, bool forceSoftwareBuffer)
{
	CSoundDriverXAudio2 *driver = new CSoundDriverXAudio2(useEax, stringMapper, forceSoftwareBuffer);
	// driver->init(stringMapper, forceSoftwareBuffer); // init in constructor lol ^^'
	return driver;
}

// ******************************************************************

#ifdef NL_STATIC
uint32 interfaceVersion()
#else
__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
#endif
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

#ifdef NL_STATIC
void outputProfile
#else
__declspec(dllexport) void NLSOUND_outputProfile
#endif
	(string &out)
{
	// -- CSoundDriverXAudio2::instance()->writeProfile(out);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver::TDriver getDriverType()
#else
__declspec(dllexport) ISoundDriver::TDriver NLSOUND_getDriverType()
#endif
{
	return ISoundDriver::DriverXAudio2;
}

// ******************************************************************

//static XAUDIO2_DEBUG_CONFIGURATION NLSOUND_XAUDIO2_DEBUG_CONFIGURATION = {
//  ~XAUDIO2_LOG_FUNC_CALLS & ~XAUDIO2_LOG_LOCKS & ~XAUDIO2_LOG_MEMORY, 0, true, true, true, true
//};

CSoundDriverXAudio2::CSoundDriverXAudio2(bool useEax, 
	ISoundDriver::IStringMapperProvider *stringMapper, bool forceSoftwareBuffer) 
	: _StringMapper(stringMapper), _XAudio2(NULL), _MasteringVoice(NULL), 
	_SoundDriverOk(false), _CoInitOk(false), _Listener(NULL)
{
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Initializing CSoundDriverXAudio2");
	HRESULT hr;

	memset(&_X3DAudioHandle, 0, sizeof(_X3DAudioHandle));
	memset(&_DSPSettings, 0, sizeof(_DSPSettings));

	// Windows
#ifdef NL_OS_WINDOWS // CoInitializeEx not on xbox, lol
	if (FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		{ release(); nlerror(NLSOUND_XAUDIO2_PREFIX "FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) HRESULT: %u", (uint32)hr); return; }
	_CoInitOk = true;
#endif

	UINT32 flags = 0;
	//flags |= XAUDIO2_DEBUG_ENGINE; // comment when done using this :)

	// XAudio2
	if (FAILED(hr = XAudio2Create(&_XAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR)))
		{ release(); nlerror(NLSOUND_XAUDIO2_PREFIX "XAudio2 failed to initialize. Please install the latest version of the DirectX End-User Runtimes. HRESULT: %u", (uint32)hr); return; }
	
	//_XAudio2->SetDebugConfiguration(&NLSOUND_XAUDIO2_DEBUG_CONFIGURATION);

	if (FAILED(hr = _XAudio2->CreateMasteringVoice(&_MasteringVoice, 0, 44100, 0, 0, NULL)))
		{ release(); nlerror(NLSOUND_XAUDIO2_PREFIX "FAILED(_XAudio2->CreateMasteringVoice(&_MasteringVoice)) HRESULT: %u", (uint32)hr); return; }
	
	// X3DAudio
	// speed of sound in meters per second for dry air at approximately 20C, used with X3DAudioInitialize
	// #define X3DAUDIO_SPEED_OF_SOUND 343.5f
	// todo replace SPEAKER_STEREO
	X3DAudioInitialize(SPEAKER_STEREO, X3DAUDIO_SPEED_OF_SOUND, _X3DAudioHandle);
	XAUDIO2_VOICE_DETAILS voice_details;
	_MasteringVoice->GetVoiceDetails(&voice_details);
	_DSPSettings.SrcChannelCount = 1; // ?! // number of channels in the emitter
	_DSPSettings.DstChannelCount = voice_details.InputChannels; // ?! // how many speakers do you have
	_DSPSettings.pMatrixCoefficients = new FLOAT32[_DSPSettings.SrcChannelCount * _DSPSettings.DstChannelCount];

	// Music Channels
	_MusicChannels.push_back(new CMusicChannelXAudio2(this));
	_MusicChannels.push_back(new CMusicChannelXAudio2(this));

	_SoundDriverOk = true;
}

CSoundDriverXAudio2::~CSoundDriverXAudio2()
{
	release();
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Destroying CSoundDriverXAudio2");
}

#define NLSOUND_XAUDIO2_RELEASE(pointer) if (_SoundDriverOk) nlassert(pointer) \
	/*if (pointer) {*/ delete pointer; pointer = NULL; /*}*/
#define NLSOUND_XAUDIO2_RELEASE_EX(pointer, command) if (_SoundDriverOk) nlassert(pointer) \
	if (pointer) { command; pointer = NULL; }
void CSoundDriverXAudio2::release()
{
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Releasing CSoundDriverXAudio2");

	// Driver (listeners etc todo)
	// Stop any played music
	{
		vector<CMusicChannelXAudio2*>::iterator it(_MusicChannels.begin()), end(_MusicChannels.end());
		for (; it != end; ++it)
		{
			delete *it;
		}
		_MusicChannels.clear();
	}
	// Assure that the remaining sources have released all their channels before closing
	{
		set<CSourceXAudio2*>::iterator it(_Sources.begin()), end(_Sources.end());
		for (; it != end; ++it)
		{
			(*it)->release(); // WARNING: The sources are NOT DELETED automagically!
		}
	}
	// Stop the listener
	_Listener->release(); // LISTENER AND SOURCES DELETED AT NLSOUND USER LEVEL

	// X3DAudio
	NLSOUND_XAUDIO2_RELEASE(_DSPSettings.pMatrixCoefficients);

	// XAudio2
	NLSOUND_XAUDIO2_RELEASE_EX(_MasteringVoice, _MasteringVoice->DestroyVoice());
	NLSOUND_XAUDIO2_RELEASE_EX(_XAudio2, _XAudio2->Release());
	_SoundDriverOk = false;

	// Windows
#ifdef NL_OS_WINDOWS
	if (_CoInitOk) CoUninitialize();
	_CoInitOk = false;
#else
	nlassert(!_CoInitOk);
#endif
}

CSampleVoiceXAudio2 *CSoundDriverXAudio2::createSampleVoice(CSourceXAudio2 *owner, TSampleFormat format) // checks pool
{
	nlassert(owner);
	std::set<CSampleVoiceXAudio2 *>::iterator it(_SampleVoices.begin()), next, end(_SampleVoices.end());
	while (it != end) if ((*it)->getFormat() == format)
	{
		next = it; ++next;
		XAUDIO2_VOICE_STATE voice_state;
		CSampleVoiceXAudio2 *sample_voice = *it;
		if (!sample_voice->getSourceVoice())
		{
			// If sample voice broke while it was in the pool.
			nlwarning("Erasing broken sample voice.");
			_SampleVoices.erase(it);
			delete sample_voice;
		}
		else
		{
			sample_voice->getSourceVoice()->GetState(&voice_state);
			if (!voice_state.pCurrentBufferContext) // make sure this buffer is done completely
			{
				_SampleVoices.erase(it);
				sample_voice->setOwner(owner);
				return sample_voice;
			}
		}
		it = next;
	}
	return new CSampleVoiceXAudio2(owner, format);;
}

// note: if something's wrong with the sample voice while it has an owner, just delete it instead of using this
void CSoundDriverXAudio2::destroySampleVoice(CSampleVoiceXAudio2 *sampleVoice, bool flush) // doesn't really destroy
{
	if (sampleVoice)
	{
		nlassert(sampleVoice->getSourceVoice()); // can also just delete if this fails, but keep this for now
		sampleVoice->getSourceVoice()->Stop(0);
		// cannot flush buffer when from callback! (also no need if from endbuffer callback)
		if (flush) { sampleVoice->getSourceVoice()->FlushSourceBuffers(); }
		// todo: nlassert check that no buffers exist anymore // maybe
		sampleVoice->setOwner(NULL);
		_SampleVoices.insert(sampleVoice);
	}
}

///** Set the gain (volume value inside [0 , 1]). (default: 1)
// * 0.0 -> silence
// * 0.5 -> -6dB
// * 1.0 -> no attenuation
// * values > 1 (amplification) not supported by most drivers
// */
//void CSoundDriverXAudio2::setGain(float gain)
//{
//	_MasteringVoice->SetVolume(gain);
//}
//
///// Get the gain
//float CSoundDriverXAudio2::getGain() const
//{
//	float gain;
//	_MasteringVoice->GetVolume(&gain);
//	return gain;
//}

/// Create a sound buffer
IBuffer *CSoundDriverXAudio2::createBuffer()
{
	return new CBufferXAudio2();
}

/// Create the listener instance
IListener *CSoundDriverXAudio2::createListener()
{
	if (!_Listener) _Listener = new CListenerXAudio2(this);
	return _Listener;
}

/// Return the maximum number of sources that can created
uint CSoundDriverXAudio2::countMaxSources()
{
	return 0xC0FFEE;
	// return I_HAVE_NO_FUCKING_IDEA;
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return NULL;
}

/// Create a source
ISource *CSoundDriverXAudio2::createSource()
{
	CSourceXAudio2* src = new CSourceXAudio2(this);
	_Sources.insert(src);
	return src;
}

/// Read a WAV data in a buffer (format supported: Mono16, Mono8, Stereo16, Stereo8)
bool CSoundDriverXAudio2::readWavBuffer(IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize)
{
	return ((CBufferXAudio2 *)destbuffer)->readWavBuffer(name, wavData, dataSize);
}

/// FMod driver Note: ADPCM format are converted and stored internally in Mono16 format (hence IBuffer::getFormat() return Mono16)
bool CSoundDriverXAudio2::readRawBuffer(IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency)
{
	return ((CBufferXAudio2 *)destbuffer)->readRawBuffer(name, rawData, dataSize, format, frequency);
}

/// Commit all the changes made to 3D settings of listener and sources
void CSoundDriverXAudio2::commit3DChanges()
{
	/*nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;*/
	std::set<CSourceXAudio2 *>::iterator it(_Sources.begin()), end(_Sources.end());
	for (; it != end; ++it) (*it)->commit3DChanges();
}

/// Write information about the driver to the output stream.
void CSoundDriverXAudio2::writeProfile(std::string& out)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

// Does not create a sound loader
void CSoundDriverXAudio2::startBench()
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

void CSoundDriverXAudio2::endBench()
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

void CSoundDriverXAudio2::displayBench(NLMISC::CLog *log)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

/** Play some music syncrhonously (.mp3 etc...) (implemented in fmod only)
 *	FMOD: The File is loaded synchronously in memory, but decompressed by FMod in a thread
 *	Hence if the mp3 fileSize is 5 Mb, it will take only 5 Mb in memory (not the decompressed 40 Mb size)
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 *	\param CIFile opened file (must use a CIFile if for instance you want to load from a BNP, and CBigFile is static....)
 *	\param xFadeTime if not 0 the old music played is not stoped imediatly but a cross-fade of xFadeTime (in ms) is made between the 2.
 */
bool CSoundDriverXAudio2::playMusic(uint channel, NLMISC::CIFile &file, uint xFadeTime, bool loop)
{
	_MusicChannels[channel]->play(file, xFadeTime, loop);
	return true; // todo
}

/** Play some music asynchronously (.mp3 etc...) (implemented in fmod only)
 *	FMOD: the file is load asynchronously
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently	 *	\param path full file path (no CPath::lookup is done since static)
 *	\param xFadeTime if not 0 the old music played is not stoped imediatly but a cross-fade of xFadeTime (in ms) is made between the 2.
 *	\param fileOffset and fileSize: if not 0, use it to load a .mp3 that reside in a BNP. 
 *		the offset and size have to be retrieved with CBigFile methods. 
 *		e.g.: use either 
 *			playMusicAsync("C:/test/mymusic.mp3");
 *		or
 * 			playMusicAsync("C:/test/mydata.bnp", offsetOfMp3InBnp, sizeOfMp3InBnp);
 *		Notice that you must give the full path of the bnp (eg: "C:/test/mydata.bnp") in path.
 *	\param loop must be true to play the music in loop. 
 */
bool CSoundDriverXAudio2::playMusicAsync(uint channel, const std::string &path, uint xFadeTime, uint fileOffset, uint fileSize, bool loop)
{
	_MusicChannels[channel]->play(path, xFadeTime, fileOffset, fileSize, loop);
	return true;
}

/** Stop the music previously loaded and played (the Memory is also freed)
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 *	\param xFadeTime if not 0 the old music played is not stoped but faded out of xFadeTime (in ms)
 */
void CSoundDriverXAudio2::stopMusic(uint channel, uint xFadeTime)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

/** Pause the music previously loaded and played (the Memory is not freed)
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 */
void CSoundDriverXAudio2::pauseMusic(uint channel)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

/** Resume the music previously paused
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 */
void CSoundDriverXAudio2::resumeMusic(uint channel)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

/** Get the song title. Returns false if the song is not found or the function is not implemented. 
 * If the song as no name, result is filled with the filename.
 */
bool CSoundDriverXAudio2::getSongTitle(const std::string &filename, std::string &result, uint fileOffset, uint fileSize)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return false;
}

/** Return true if a song is finished.
 *	NB: in case of cross fading, the channel is considered "ended" if all fading are done
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 */
bool CSoundDriverXAudio2::isMusicEnded(uint channel)
{
	nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return false;
}

/** Return the total length (in second) of the music currently played
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 */
float CSoundDriverXAudio2::getMusicLength(uint channel)
{
	// -- nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return 0.0;
}

/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
 *	NB: the volume of music is NOT affected by IListener::setGain()
 *	\param channel up to 2 channels are available for now 0 and 1 (any other value will fail), so that 2 music can be played concurently
 */
void CSoundDriverXAudio2::setMusicVolume(uint channel, float gain)
{
	// -- nlerror(NLSOUND_XAUDIO2_PREFIX "not implemented");
	return;
}

/// Remove a buffer (should be called by the friend destructor of the buffer class)
void CSoundDriverXAudio2::removeBuffer(IBuffer *buffer)
{
	// does nothing here .. see createBuffer
}

/// Remove a source (should be called by the friend destructor of the source class)
void CSoundDriverXAudio2::removeSource(ISource *source)
{
	_Sources.erase((CSourceXAudio2*)source); // note: IT DOESN'T DELETE
	// SOURCE ARE DELETED IN CTrack / mixing_track.h ...
	// -> use delete to call this function ^^'
}

} /* namespace NLSOUND */

/* end of file */
