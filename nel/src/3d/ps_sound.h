/** \file ps_sound.h
 * <File description>
 *
 * $Id: ps_sound.h,v 1.2 2001/08/16 17:12:37 vizerie Exp $
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
#include "3d/ps_located.h"
#include "3d/ps_attrib.h"



namespace NL3D 
{


template <class T>
class CPSAttribMaker;
struct IPSSoundInstance;

/**
 * This is a goup of sound instances sound instance. sounds are produced during the motion pass
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSound : public CPSLocatedBindable
{
public:

	///\name object
		//@{
		/// ctor
									CPSSound();
		/// dtor
									~CPSSound();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);

		NLMISC_DECLARE_CLASS(CPSSound);
		//@}


	/// return this bindable type
		uint32						getType(void) const;
	/// return priority	
	virtual uint32					getPriority(void) const { return 500; }

	/**
	* process one pass for the sound. This is usually done during the motion pass
	*/
	virtual void					step(TPSProcessPass pass, CAnimationTime ellapsedTime);
	
	/// set the name of the sound
	void							setSoundName(const std::string &soundName)
	{
		_SoundName = soundName;
	}

	/// get the name of the sound
	const std::string &				getSoundName(void) const
	{
		return _SoundName;
	}

	/// set a constant gain. Any previous gain scheme is discarded.
	void							setGain(float gain);

	/// get the current gain. meaningful only if a scheme is not used
	float							getGain(void) const
	{
		return _Gain;
	}

	/** Set a gain scheme. It must have been allocated by new, and is then owned by this object
	  */	  
	void							setGainScheme(CPSAttribMaker<float> *gain);

	/// get the current gain scheme
	CPSAttribMaker<float> *			getGainScheme(void)
	{
		return _GainScheme;
	}

	/// get the current gain scheme (const version)
	const CPSAttribMaker<float> *	getGainScheme(void) const
	{
		return _GainScheme;
	}

	/// set a constant pitch. Any previous pitch scheme is discarded.
	void							setPitch(float pitch);

	/// get the current pitch. meaningful only if a scheme is not used
	float							getPitch(void) const
	{
		return _Pitch;
	}

	/** Set a pitch scheme. It must have been allocated by new, and is then owned by this object
	  */	  
	void							setPitchScheme(CPSAttribMaker<float> *pitch);

	/// get the current pitch scheme
	CPSAttribMaker<float> *			getPitchScheme(void)
	{
		return _PitchScheme;
	}

	/// get the current pitch scheme (const version)
	const CPSAttribMaker<float> *  getPitchScheme(void) const
	{
		return _PitchScheme;
	}

	
protected:
	virtual void			newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	virtual void			deleteElement(uint32 index);
	virtual void			resize(uint32 size);

	CPSAttrib<IPSSoundInstance *>	_Sounds;
	std::string						_SoundName;
	float							_Gain;
	CPSAttribMaker<float> *			_GainScheme;
	float							_Pitch;
	CPSAttribMaker<float> *			_PitchScheme;

};


} // NL3D


#endif // NL_PS_SOUND_H

/* End of ps_sound.h */
