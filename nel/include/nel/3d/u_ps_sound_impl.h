/** \file ps_sound_impl.h
 * <File description>
 *
 * $Id: u_ps_sound_impl.h,v 1.3 2001/09/04 16:16:55 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_PS_SOUND_IMPL_H
#define NL_PS_SOUND_IMPL_H

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/rgba.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/3d/u_particle_system_sound.h"


// WARNING : this file is not intended to be directly included by the client. 
// It is just used to avoid a dependencie between NL3D and NLSOUND



namespace NL3D 
{


inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam);


/// This class implement a sound instance (a sound source)
class CPSSoundInstanceImpl : public UPSSoundInstance
{
public:
	/// construct this object from a nel sound source
	/** The system will call this method to set the parameters of the sound	  
	  */
	CPSSoundInstanceImpl() 
		: _Source(NULL), _AudioMixer(NULL), _Spawned(false)
	{			
	}

	/// init this sound instance parameters
	void init(NLSOUND::USource *source, NLSOUND::UAudioMixer *am, bool spawned)
	{
		nlassert(source);
		nlassert(am);
		_Source = source;
		_AudioMixer = am;
		_Spawned    = spawned;
	}

	/// change this sound source paramerters
	virtual void setSoundParams(float gain
						   , const NLMISC::CVector &pos
						   , const NLMISC::CVector &velocity
						   , float pitch
						  )
	{
		if (!_Source) return;		
		if (gain < 0) gain = 0;
		if (gain > 1) gain = 1;
		if (pitch > 1) pitch = 1;
		if (pitch < 0.0001f) pitch = 0.0001f;
		_Source->setPos(pos);
		_Source->setVelocity(velocity);
		_Source->setGain(gain);
		_Source->setPitch(pitch);
	}

	/// start to play the sound
	virtual void play(void)
	{
		if (!_Source) return;
		_Source->play();
	}


	virtual bool isPlaying(void) const
	{
		if (!_Source) return false;
		return _Source->isPlaying();
	}

	/// stop the sound
	virtual void stop(void)
	{
		if (!_Source) return;
		_Source->stop();
	}

	/// release the sound source
	virtual void release(void)
	{	
		if (!_Spawned) // remove this source from the audio mixer if it hasn't been spawned
		{
			nlassert(_AudioMixer);
			_AudioMixer->removeSource(_Source);
		}
		else
		{
			if (_Source) // tells this spawned source not to notify us when it ends
			{
				_Source->unregisterSpawnCallBack();
			}
		}
		delete this;
	}

protected:
	friend inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam);
	NLSOUND::USource *_Source;
	bool			 _Spawned;
	NLSOUND::UAudioMixer *_AudioMixer;
};







/**
 * This class implements PS sound server. It warps the calls to NEL sound. Everything is in a .h file to avoid dependency
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSoundServImpl : public UPSSoundServer
{
public:	
	/// construct this sound server; You must init it then
	CPSSoundServImpl() : _AudioMixer(NULL)
	{
	}

	

	/// init this particle system sound server, using the given audio mixer
	void init(NLSOUND::UAudioMixer *audioMixer)
	{
		nlassert(audioMixer);
		_AudioMixer = audioMixer;
	}


	/// get the audio mixer associated with that server
	NLSOUND::UAudioMixer *getAudioMixer(void) { return _AudioMixer;}
	const NLSOUND::UAudioMixer *getAudioMixer(void) const { return _AudioMixer;}


	/// inherited from IPSSoundServer
	UPSSoundInstance *createSound(const std::string &soundName, bool spawned = true)
	{		
		nlassert(_AudioMixer);
		CPSSoundInstanceImpl *sound = new CPSSoundInstanceImpl;
		NLSOUND::USource *source = _AudioMixer->createSource(soundName.c_str(), spawned, SpawnedSourceEndedCallback, sound );
		if (source)
		{			
			if (spawned)
			{
				source->setLooping(false);
			}
			sound->init(source, _AudioMixer, spawned);
			return sound;
		}
		else
		{
			// should usually not happen
			delete sound;
			return NULL;
		}
	}

protected:

	NLSOUND::UAudioMixer  *_AudioMixer;

};


/// this callback is called when a spawned source has ended, so that we know that the pointer to it is invalid...
inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam)
{
	nlassert(((CPSSoundInstanceImpl *) userParam)->_Source == source);
	((CPSSoundInstanceImpl *) userParam)->_Source = NULL;
}




} // NL3D


#endif // NL_PS_SOUND_IMPL_H

/* End of ps_sound_impl.h */
