/** \file light_contribution.h
 * <File description>
 *
 * $Id: light_contribution.h,v 1.3 2002/02/18 13:21:55 berenguier Exp $
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

#ifndef NL_LIGHT_CONTRIBUTION_H
#define NL_LIGHT_CONTRIBUTION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "3d/point_light.h"


namespace NL3D 
{


// ***************************************************************************
/** This is the maximum possible contribution of light. NB: actual max may be less 
 *	because of setup in CLightingManager.
 */
#define	NL3D_MAX_LIGHT_CONTRIBUTION		6


// ***************************************************************************
/**
 * light contribution on a model. Owned by a CTransform
 *	computed by lighting manager. result CLight is computed at render.
 *
 * if FrozenStaticLightSetup, then the SunContribution won't never be recomputed, and 
 *	the first NumFrozenStaticLight PointLight are considered always valid and their setup
 *	won't be recomputed too. It's means also that CTransform::resetLighting() do not affect those
 *	NumFrozenStaticLight. 
 *
 *	Typically, FrozenStaticLightSetup is setup for models and lights THAT ARE IN SAME IG, and 
 *	are deleted together. This last point is important because the first NumFrozenStaticLight PointLight
 *	pointers are never updated, so delete a light which is in this setup will cause memory failure.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLightContribution
{
public:
	/// This is the list of Light which influence us. The first NULL means end_of_list.
	CPointLight			*PointLight[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// An iterator on the list of model in the pointLight which owns our transform.
	CPointLight::ItTransformList	TransformIterator[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the factor of influence to apply to each point light.
	uint8				Factor[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the Attenuation factor of influence to apply to each point light. Used if the model
	uint8				AttFactor[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the Dynamic Local Ambient. If A==0 then full SunAmbient is taken. If A==255, take full LocalAmbient color.
	NLMISC::CRGBA		LocalAmbient;


	/// Tells if there is some frozen static light setup.
	bool				FrozenStaticLightSetup;
	/** if FrozenStaticLightSetup, tells the number of point light setup which are static.
	 *	NB: it is possible that FrozenStaticLightSetup==true, and NumFrozenStaticLight==0. it means
	 *	that the model is not touched by any static pointLight.
	 */
	uint8				NumFrozenStaticLight;
	// The contribution of the sun (directionnal light) on this model. This not apply to ambient part of the sun
	uint8				SunContribution;
	/** if FrozenStaticLightSetup, this is the frozen AmbientLight in ig. 
	 *	can't be stored as RGBA, because the ambient color may change. 
	 *	NULL means take full Sun ambient
	 */
	CPointLight			*FrozenAmbientLight;

public:

	/// Constructor
	CLightContribution();

};


} // NL3D


#endif // NL_LIGHT_CONTRIBUTION_H

/* End of light_contribution.h */
