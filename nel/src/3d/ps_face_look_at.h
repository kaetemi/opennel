/** \file ps_face_look_at.h
 * Face look at particles.
 *
 * $Id: ps_face_look_at.h,v 1.5 2004/02/19 09:49:44 vizerie Exp $
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

#ifndef NL_PS_FACE_LOOK_AT_H
#define NL_PS_FACE_LOOK_AT_H

#include "3d/ps_particle_basic.h"
#include "3d/ps_quad.h"
#include "3d/ps_iterator.h"

namespace NL3D 
{


class CPSFaceLookAt;


/**
 * A FaceLookAt particle
 *  These particles can have 2 different size (width and height) when activated
 */
class CPSFaceLookAt :   public CPSQuad, public CPSRotated2DParticle
{
public:
	/** create the face look at by giving a texture. This can't be a CTextureGrouped (for animation)
     * animation must be set later by using setTextureScheme
	 */
	CPSFaceLookAt(CSmartPtr<ITexture> tex = NULL);
	
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt);
			
	/** activate 'fake' motion blur (its done by deforming the quad)
	 *  This is slower, however. This has no effect with rotated particles.
	 *  \param coeff a coefficient for motion blur (too high value may give unrealistic result)
	 *         0 deactivate the motion blur
	 *  \param threshold : speed will be clamped below this value	 
	 */
	void activateMotionBlur(float coeff = 1.f, float threshold = 1.f)
	{
		_MotionBlurCoeff = coeff;
		_Threshold = threshold;
	}

	/// set the motion blur coeff (0 = none)
	void setMotionBlurCoeff(float coeff) { _MotionBlurCoeff = coeff; }

	/// set the motion blur threshold
	void setMotionBlurThreshold(float threshold) { _Threshold = threshold; }



	/** return the motion blur coeff (0.f means none)
	 *  \see  activateMotionBlur()
	 */
	float getMotionBlurCoeff(void) const { return _MotionBlurCoeff; }

	/// get the motion blur threshold
	float getMotionBlurThreshold(void) const { return _Threshold; }

	/** Setting this to true allows to have independant height and width for these particles.
	  * The interface to manage the second size can be obtained from getSecondSize(), which correspond to the height of particles.
	  * The default is to not have independant sizes
	  */
	void setIndependantSizes(bool enable  = true) { _IndependantSizes = enable; }

	/// test wether independant sizes are activated
	bool hasIndependantSizes(void) const { return _IndependantSizes; }

	/// retrieve an interface to set the second size
	CPSSizedParticle &getSecondSize(void) 
	{ 
		nlassert(_IndependantSizes);
		return _SecondSize;
	}

	/// retrieve an interface to set the second size const version
	const CPSSizedParticle &getSecondSize(void) const
	{ 
		nlassert(_IndependantSizes);
		return _SecondSize;
	}

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool			hasLightableFaces() { 	return false; }

	// Force faces to be aligned on motion. This bypass motion blur
	void					setAlignOnMotion(bool align) { _AlignOnMotion = align; }
	bool					getAlignOnMotion() const { return _AlignOnMotion; }
	
protected:
	friend class			CPSFaceLookAtHelper;	/// used for implementation only
	float					_MotionBlurCoeff;	
	float					_Threshold; // motion blur theshold
	// in this struct we defines the getSizeOwner method, which is abstract in the CPSSizedParticle clas
	struct					CSecondSize : public CPSSizedParticle
	{
		CPSFaceLookAt *Owner;
		virtual CPSLocated *getSizeOwner(void) { return Owner->getOwner(); }
	} _SecondSize;
	bool					_IndependantSizes;
	bool                    _AlignOnMotion;
	virtual void			draw(bool opaque);
	void					newElement(CPSLocated *emitterLocated, uint32 emitterIndex);	
	void					deleteElement(uint32);
	void					resize(uint32);
	virtual CPSLocated		*getAngle2DOwner(void) { return _Owner; }

};


} // NL3D

#endif // NL_PS_FACE_LOOK_AT_H

/* End of ps_face_look_at.h */
