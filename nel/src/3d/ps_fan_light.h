/** \file ps_particle.h
 * FanLight particles
 *
 * $Id: ps_fan_light.h,v 1.1 2002/02/15 17:03:29 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_PS_FAN_LIGHT_H
#define NL_PS_FAN_LIGHT_H

#include "3d/ps_particle_basic.h"
#include "3d/vertex_buffer.h"


namespace NL3D 
{


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A fan light particle
 */
class CPSFanLight : public CPSParticle, public CPSColoredParticle
				  , public CPSSizedParticle, public CPSRotated2DParticle
				  , public CPSMaterial, public CPSTexturedParticleNoAnim
{
public:	
	NLMISC_DECLARE_CLASS(CPSFanLight);
	virtual bool		completeBBox(NLMISC::CAABBox &box) const;
	///\name Object
	//@{
	/// Ctor, with the numbers of fans to draw (minimum is 3, maximum is 128)
	CPSFanLight(uint32 nbFans = 7);
	/// Dtor
	~CPSFanLight();
	void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}

	// Set the number of fans used for drawing (minimum is 3, maximum is 128)
	void				setNbFans(uint32 nbFans);

	/** Set the smoothness of phases. The default is 0 which means no smoothness.
	  * n mean that the phase will be linearly interpolated between each n + 1 fans
	  * It ranges from 0 to 31
	  */
	void				setPhaseSmoothness(uint32 smoothNess) 
	{ 
		nlassert(smoothNess < 32);
		_PhaseSmoothness = smoothNess; 
	}

	/// retrieve the phase smoothness
	uint32				getPhaseSmoothness(void) const { return _PhaseSmoothness;}

	/// set the intensity of fan movement. Default is 1.5
	void				setMoveIntensity(float intensity) { _MoveIntensity = intensity; }

	/// get the intensity of fans movement
	float				getMoveIntensity(void) const      { return _MoveIntensity; }

	// Get the number of fans used for drawing
	uint32				getNbFans(void) const
	{
		return _NbFans;
	}

	/** Set the speed for phase
	 *	If the located holding this particle as a limited lifetime, it gives how many 0-2Pi cycle it'll do during its life
	 *  Otherwise it gives how many cycle there are in a second
	 */
	void				setPhaseSpeed(float multiplier);

	/// get the speed for phase
	float				getPhaseSpeed(void) const { return _PhaseSpeed / 256.0f; }

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void		updateMatAndVbForColor(void);


	/// must call this at least if you intend to use fanlight
	static void			initFanLightPrecalc(void);
	
	/// return true if there are transparent faces in the object
	virtual bool		hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool		hasOpaqueFaces(void);


	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32		getMaxNumFaces(void) const;

	/// Set a texture. NULL remove it
	void setTexture(CSmartPtr<ITexture> tex)
	{
		_Tex = tex;		
		updateMatAndVbForColor();
	}
	
	/// get the texture used
	ITexture *getTexture(void)
	{
		return _Tex; 
	}
	const ITexture *getTexture(void) const
	{
		return _Tex; 
	}
protected:

	friend class CPSFanLightHelper;
	
	/// initialisations
	virtual void init(void);

	uint32						_NbFans;
	uint32						_PhaseSmoothness;
	CVertexBuffer				_Vb;
	float						_MoveIntensity;	
	uint32						*_IndexBuffer;
	NLMISC::CSmartPtr<ITexture> _Tex;
	static uint8				_RandomPhaseTab[32][128];		
	float						_PhaseSpeed;

	//#ifdef NL_DEBUG		
		static bool _RandomPhaseTabInitialized;
	//#endif
	
	virtual void		draw(bool opaque);
	void				newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	void				deleteElement(uint32);

	/// Set the max number of fanlights		
	virtual void resize(uint32 size); 

	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
};


} // NL3D


#endif // NL_PS_FAN_LIGHT_H

/* End of ps_particle.h */
