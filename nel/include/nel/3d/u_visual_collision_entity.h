/** \file u_visual_collision_entity.h
 * <File description>
 *
 * $Id: u_visual_collision_entity.h,v 1.7 2002/02/18 13:23:34 berenguier Exp $
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

#ifndef NL_U_VISUAL_COLLISION_ENTITY_H
#define NL_U_VISUAL_COLLISION_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/3d/point_light_influence.h"


namespace NL3D 
{


using	NLMISC::CVector;


/**
 * Interface to visual collision entity.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UVisualCollisionEntity
{
protected:
	UVisualCollisionEntity() {}
	virtual ~UVisualCollisionEntity() {}

public:


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 *	and meshes setuped into the visual collision manager.
	 *	pos is checked with polygons that are at least (cache dependent) at +- 10m in altitude.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	virtual bool	snapToGround(CVector &pos) =0;


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 *	and meshes setuped into the visual collision manager.
	 *	pos is checked with polygons that are at least (cache dependent) at +- 10m in altitude.
	 * \param normal the ret normal of where it is snapped. NB: if return false, not modified.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	virtual bool	snapToGround(CVector &pos, CVector &normal) =0;



	/** If groundMode is true, the entity is snapped on faces with normal.z > 0. Default is true.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	virtual void	setGroundMode(bool groundMode) =0;


	/** If ceilMode is true, the entity is snapped on faces with normal.z < 0. Default is false.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	virtual void	setCeilMode(bool ceilMode) =0;


	virtual bool	getGroundMode() const =0;
	virtual bool	getCeilMode() const =0;

	/** By default, the visual collision entity is snapped on rendered/geomorphed tesselation (true).
	 *  Use this method to change this behavior. if false, the entity is snapped to the tile level tesselation
	 *	according to noise etc...
	 */
	virtual void	setSnapToRenderedTesselation(bool snapMode) =0;
	virtual bool	getSnapToRenderedTesselation() const =0;


	/// \name Static Lighting
	// @{
	/** Get the static Light Setup, using landscape under us. append lights to pointLightList.
	 *	NB: if find no landscape faces, don't modify pointLightList, set sunContribution=255, and return false
	 *	Else, use CPatch::TileLightInfluences to get lights, and use CPatch::Lumels to get sunContribution.
	 *	NB: because CPatch::Lumels encode the gouraud shading on the surface, return lumelValue*2 so
	 *	the object won't be too darken.
	 */
	virtual bool	getStaticLightSetup(const CVector &pos, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient) =0;
	// @}


};


} // NL3D


#endif // NL_U_VISUAL_COLLISION_ENTITY_H

/* End of u_visual_collision_entity.h */
