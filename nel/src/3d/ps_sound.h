/** \file ps_sound.h
 * <File description>
 *
 * $Id: ps_sound.h,v 1.12 2004/05/14 15:38:54 vizerie Exp $
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

#ifndef NL_PS_SOUND_H
#define NL_PS_SOUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "3d/ps_located.h"
#include "3d/ps_attrib.h"



namespace NL3D 
{


template <class T>
class CPSAttribMaker;
struct UPSSoundInstance;

/**
 * This is a goup of sound instances sound instance. sounds are produced during the motion pass
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSound : public CPSLocatedBindable
{
public:
	NLMISC_DECLARE_CLASS(CPSSound);
	///\name object
	//@{
	/// ctor
								CPSSound();
	/// dtor
								~CPSSound();
	/// serialisation. Derivers must override this, and call their parent version
	virtual void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}

	/// return this bindable type
	uint32						getType(void) const;
	/// return priority	
	virtual uint32				getPriority(void) const { return 500; }

	/**
	  * process one pass for the sound. This is usually done during the motion pass
	  */
	virtual void				step(TPSProcessPass pass);
	
	/// set the name of the sound
	void						setSoundName(const NLMISC::TStringId &soundName)
	{
		_SoundName = soundName;
	}

	/// get the name of the sound
	const NLMISC::TStringId		&getSoundName(void) const
	{
		return _SoundName;
	}

	/// set a constant gain. Any previous gain scheme is discarded.
	void						setGain(float gain);

	/// get the current gain. meningful only if a scheme is not used
	float						getGain(void) const
	{
		return _Gain;
	}

	/** Set a gain scheme. It must have been allocated by new, and is then owned by this object
	  */	  
	void						setGainScheme(CPSAttribMaker<float> *gain);

	/// get the current gain scheme
	CPSAttribMaker<float>		*getGainScheme(void)
	{
		return _GainScheme;
	}

	/// get the current gain scheme (const version)
	const CPSAttribMaker<float> *getGainScheme(void) const
	{
		return _GainScheme;
	}

	/// set a constant pitch. Any previous pitch scheme is discarded.
	void						setPitch(float pitch);

	/// get the current pitch. meaningful only if a scheme is not used
	float						getPitch(void) const
	{
		return _Pitch;
	}

	/** Set a pitch scheme. It must have been allocated by new, and is then owned by this object
	  */	  
	void						setPitchScheme(CPSAttribMaker<float> *pitch);

	/// get the current pitch scheme
	CPSAttribMaker<float>		*getPitchScheme(void)
	{
		return _PitchScheme;
	}

	/// get the current pitch scheme (const version)
	const CPSAttribMaker<float> *getPitchScheme(void) const
	{
		return _PitchScheme;
	}

	/// set a percent of sound emission. If set to 1, it try to emit all sounds
	void						setEmissionPercent(float percent) { _EmissionPercent = percent; }


	/// get the percent of sound emission.
	float						getEmissionPercent(void) const { return _EmissionPercent; }

	/// tells wether sound emissions are spawned or not
	void						setSpawn(bool enabled = true) { _SpawnSounds = enabled; }

	/// test wether sound spawning is enabled
	bool						getSpawn(void) const		  { return _SpawnSounds; }

	///set mute on/off. The default is off. WARNING : It is not saved by this object
	void						setMute(bool enabled = true)  { _Mute = enabled; }

	/// test wether mute has been activated
	bool						getMute(void) const { return _Mute; }

	
	void						stopSound();

	void						reactivateSound();

	/** force the sound to use original pitch that has been entered in the .sound sheet
	  * This remove any existing pitchScheme
	  */
	void						setUseOriginalPitchFlag(bool useOriginalPitch);
	bool						getUseOriginalPitchFlag() const { return _UseOriginalPitch; }
	
protected:
	virtual void			newElement(const CPSEmitterInfo &info);
	virtual void			deleteElement(uint32 index);
	virtual void			resize(uint32 size);
	void					removeAllSources();

	CPSAttrib<UPSSoundInstance *>	_Sounds;
	NLMISC::TStringId				_SoundName;
	float							_Gain;
	CPSAttribMaker<float> *			_GainScheme;
	float							_Pitch;
	CPSAttribMaker<float> *			_PitchScheme;	
	float							_EmissionPercent;
	bool							_SpawnSounds;
	bool							_Mute;
	bool							_SoundStopped;
	bool							_SoundReactivated;
	bool							_UseOriginalPitch;
};


} // NL3D


#endif // NL_PS_SOUND_H

/* End of ps_sound.h */
