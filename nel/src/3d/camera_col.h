/** \file camera_col.h
 * TODO: File description
 *
 * $Id: camera_col.h,v 1.3.32.1 2006/01/11 15:02:10 boucher Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_CAMERA_COL_H
#define NL_CAMERA_COL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"


namespace NL3D {


// ***************************************************************************
/**
 * A tool class used to compute differents info for camera collision
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CCameraCol
{
public:
	CCameraCol();

	/** build the camera collision as a cone or a cylinder
	 */
	void			build(const CVector &start, const CVector &end, float radius, bool cone);

	/** build the camera collision as a simple ray
	 */
	void			buildRay(const CVector &start, const CVector &end);

	/** compute the intersection of the Camera Volume against the triangle, and minimize 
	 *	minDist (actual square of distance) with min sqr distance of the poly.
	 */
	void			minimizeDistanceAgainstTri(const CVector &p0, const CVector &p1, const CVector &p2, float &sqrMinDist);

	/** Compute into this the camera collision 'other' mul by 'matrix'
	 *	NB: for cone Radius, suppose uniform scale, else will have strange result (a uniform scale is deduced)
	 */
	void			setApplyMatrix(const CCameraCol &other, const NLMISC::CMatrix &matrix);
	
	/** Get The World Bbox enclosing the camera collision volume
	 */
	const NLMISC::CAABBox		&getBBox() const {return _BBox;}
	
	/** Get the length of the ray built
	 */
	float			getRayLen() const {return _RayLen;}

private:
	enum	{MaxNPlanes=6};
	
	// The start of the camera raycast
	CVector		_Start;
	// The end of the camera raycast
	CVector		_End;
	// The radius (at end only if cone)
	float		_Radius;
	// cone or cylinder?
	bool		_Cone;
	// Simple Ray?
	bool		_SimpleRay;
	
	// The World Bbox enclosing the camera collision volume
	NLMISC::CAABBox		_BBox;
	
	// Temp Data for minimizeDistanceAgainstTri
	CVector		_ArrayIn[3+MaxNPlanes];
	CVector		_ArrayOut[3+MaxNPlanes];

	// The pyramid representing the camera collision volume. Nb: local to start for precision problems
	NLMISC::CPlane		_Pyramid[MaxNPlanes];
	uint	_NPlanes;
	
	// For Camera smoothing. => the pyramid is bigger
	float		_MaxRadius;
	// projection of the radius at 1 meter
	float		_MinRadiusProj;
	float		_MaxRadiusProj;
	float		_OODeltaRadiusProj;
	float		_RayLen;
	CVector		_RayNorm;

	// simpler method for simple ray
	void			intersectRay(const CVector &p0, const CVector &p1, const CVector &p2, float &sqrMinDist);
	
};


} // NL3D


#endif // NL_CAMERA_COL_H

/* End of camera_col.h */
